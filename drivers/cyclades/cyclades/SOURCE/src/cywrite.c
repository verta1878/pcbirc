/* ====================================================================
 * cywrite.c — IRP_MJ_WRITE Dispatch and Write DPC Completion
 * ====================================================================
 * Handles WriteFile requests from applications.
 *
 * Data flow:
 *   1. Write dispatch copies data from IRP → TxBuf ring buffer
 *   2. Write dispatch enables TxRdy interrupt (kicks transmitter)
 *   3. ISR reads bytes from TxBuf → writes to CD1400 TDR (hardware)
 *   4. When ISR drains TxBuf and space opens, WriteDpc fires
 *   5. WriteDpc dequeues pending write IRPs and fills TxBuf again
 *
 * Fast path: If TxBuf has enough space for the entire write, we
 *            copy all bytes, kick the transmitter, and complete
 *            the IRP immediately.
 *
 * Slow path: If TxBuf can't hold all the data, we copy what fits,
 *            kick the transmitter, and queue the remainder as a
 *            pending IRP. The WriteDpc will continue copying when
 *            the ISR makes space.
 *
 * All TxBuf access from dispatch level uses KeSynchronizeExecution
 * to prevent races with the ISR reading TxBuf.
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cywrite.tmh"
#endif
/* csq.h — provided by owcompat.h for OW, by WDK for MSVC */
#ifndef __WATCOMC__
#include <csq.h>
#endif


/* ====================================================================
 * CySyncWriteToTxBuf — Copy data to TxBuf at DIRQL
 * ====================================================================
 * Runs via KeSynchronizeExecution. Copies up to 'length' bytes
 * from the caller's buffer into the TX ring buffer. Returns the
 * actual number of bytes copied.
 *
 * After copying, if TxBuf was previously empty and now has data,
 * we enable the TxRdy interrupt so the ISR starts draining. */

/* ====================================================================
 * CySyncWriteToTxBuf — Copy data to TxBuf at DIRQL
 * ====================================================================
 * KeSynchronizeExecution callback. Copies up to 'Length' bytes
 * from the caller's buffer into the TX ring buffer. After copying,
 * if the buffer was previously empty, enables the TxRdy interrupt
 * to kick the transmitter.
 *
 * Context: CY_WRITE_CONTEXT
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

typedef struct _CY_WRITE_CONTEXT {
    PCY_PDO_EXT Extension;
    PUCHAR      Buffer;         /* Source buffer (system buffer)     */
    ULONG       Length;         /* How many bytes to write           */
    ULONG       Written;        /* How many bytes actually written   */
} CY_WRITE_CONTEXT, *PCY_WRITE_CONTEXT;

static BOOLEAN NTAPI CySyncWriteToTxBuf(PVOID Context)
{
    PCY_WRITE_CONTEXT ctx = (PCY_WRITE_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    ULONG       written = 0;
    BOOLEAN     wasEmpty;
    UCHAR       srer;              /* Service Request Enable Register  */

    /* Remember if TxBuf was empty before our write.
     * If it was, we need to kick the transmitter by enabling
     * TxRdy after we add data. If it wasn't empty, the ISR is
     * already draining and TxRdy is already enabled. */
    wasEmpty = (ext->TxCount == 0);

    /* Copy bytes into the ring buffer until:
     *   - We've copied all the bytes the caller wants to send, or
     *   - The ring buffer is full */
    while (written < ctx->Length && ext->TxCount < CY_RING_BUF_SIZE) {
        ext->TxBuf[ext->TxHead] = ctx->Buffer[written];
        ext->TxHead++;
        if (ext->TxHead >= CY_RING_BUF_SIZE)
            ext->TxHead = 0;   /* Wrap around                      */
        ext->TxCount++;
        written++;
    }

    ctx->Written = written;

    /* If the TX buffer was empty and we just added data, enable
     * the TxRdy interrupt. This tells the CD1400 to generate a
     * transmit service request when the hardware FIFO has space.
     * The ISR will then drain TxBuf into the FIFO.
     *
     * We do this HERE (at DIRQL) because SRER is a per-channel
     * register that requires CAR to be set correctly. We're
     * synchronized with the ISR, so CAR is safe to touch. */
    if (wasEmpty && ext->TxCount > 0) {
        CySelectChannel(ext->ChipBase, ext->Channel);
        srer = CyReadReg(ext->ChipBase, CySRER);
        srer |= CyTxRdy;       /* Enable transmit ready interrupt  */
        CyWriteReg(ext->ChipBase, CySRER, srer);
    }

    return TRUE;
}


/* ====================================================================
 * CyDispatchWrite — Handle IRP_MJ_WRITE (WriteFile)
 * ====================================================================
 * The application called WriteFile on the COM port. We need to
 * transmit the data bytes over the serial line.
 *
 * We copy data into TxBuf (the TX ring buffer) and let the ISR
 * drain it into the CD1400's hardware FIFO. The ISR handles the
 * actual byte-by-byte transmission.
 *
 * Fast path: If TxBuf can hold all the data, copy everything,
 *            kick the transmitter, complete immediately.
 *
 * Slow path: Copy what fits, queue the IRP for the remainder.
 *            The WriteDpc will continue when ISR makes space.
 * ==================================================================== */

NTSTATUS CyDispatchWrite(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT      common = CyGetCommon(DeviceObject);
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            status;

    /* FDOs don't handle Write */
    if (common->IsFDO) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    {
        PCY_PDO_EXT pdoExt = CyGetPdo(DeviceObject);

        /* Acquire remove lock */
        status = IoAcquireRemoveLock(&pdoExt->Common.RemoveLock, Irp);
        if (!NT_SUCCESS(status)) {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
        }

        /* Verify the port is open */
        if (!pdoExt->IsOpen) {
            status = STATUS_INVALID_DEVICE_STATE;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
            return status;
        }

        irpSp = IoGetCurrentIrpStackLocation(Irp);

        /* ---- Try to copy all data to TxBuf ---- */
        if (pdoExt->InterruptConnected) {
            CY_WRITE_CONTEXT writeCtx;
            writeCtx.Extension = pdoExt;
            writeCtx.Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
            writeCtx.Length = irpSp->Parameters.Write.Length;
            writeCtx.Written = 0;

            /* Copy to TxBuf at DIRQL — synchronized with ISR */
            KeSynchronizeExecution(pdoExt->Interrupt,
                                   CySyncWriteToTxBuf, &writeCtx);

            if (writeCtx.Written == irpSp->Parameters.Write.Length) {
                /* ---- Fast path: all bytes fit in TxBuf ----
                 * Complete immediately. The ISR will drain TxBuf
                 * into the hardware FIFO asynchronously. */
                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = writeCtx.Written;

                TraceEvents(CYPORT_LEVEL_VERBOSE, CYPORT_WRITE,
                        "WRITE port %lu fast %lu bytes", pdoExt->PortIndex, writeCtx.Written);
                CyTrace("WRITE: port %lu fast-path %lu bytes\n",
                        pdoExt->PortIndex, writeCtx.Written);
                IoCompleteRequest(Irp, IO_SERIAL_INCREMENT);
                IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
                return STATUS_SUCCESS;
            }

            /* ---- Partial write: some bytes fit, rest must wait ----
             * Store the write offset in the IRP's driver context area
             * so the WriteDpc knows where to continue. Queue the IRP
             * and the DPC will feed remaining bytes into TxBuf as
             * the ISR drains it.
             *
             * IRP driver context: we use the DriverContext field in
             * Tail.Overlay to store the byte offset. This is safe
             * because the IRP is pending (we own it until completion).
             *
             * (Fix for TODO #4 — partial write queueing) */
            if (writeCtx.Written > 0) {
                /* Store how many bytes we've already written.
                 * The WriteDpc reads this to know where to continue
                 * in the user buffer. */
                Irp->Tail.Overlay.DriverContext[0] =
                    (PVOID)(ULONG_PTR)writeCtx.Written;

                CyTrace("WRITE: port %lu partial %lu/%lu bytes, queueing rest\n",
                        pdoExt->PortIndex, writeCtx.Written,
                        irpSp->Parameters.Write.Length);

                /* Queue the IRP — WriteDpc will continue from offset */
                IoMarkIrpPending(Irp);
                IoCsqInsertIrp(&pdoExt->WriteQueue, Irp, NULL);

                IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
                return STATUS_PENDING;
            }
        }

        /* ---- TxBuf completely full or ISR not connected ----
         * Queue the IRP. The WriteDpc will process it when the ISR
         * drains some bytes and makes space in TxBuf. */
        IoMarkIrpPending(Irp);
        IoCsqInsertIrp(&pdoExt->WriteQueue, Irp, NULL);

        CyTrace("WRITE: port %lu queued (TxCount=%lu, buffer full)\n",
                pdoExt->PortIndex, pdoExt->TxCount);

        IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
        return STATUS_PENDING;
    }
}


/* ====================================================================
 * CyWriteDpcComplete — Write DPC implementation
 * ====================================================================
 * Called from the WriteDpc (queued by the ISR when TX space opens).
 * Dequeues pending write IRPs and fills TxBuf.
 *
 * This replaces the stub CyWriteDpcRoutine in cyisr.c.
 *
 * Runs at DISPATCH_LEVEL.
 * ==================================================================== */

VOID CyWriteDpcComplete(PCY_PDO_EXT pdoExt)
{
    PIRP    irp;

    /* Loop: dequeue pending write IRPs and fill TxBuf.
     * Stop when the queue is empty or TxBuf is full. */
    while (pdoExt->TxCount < CY_RING_BUF_SIZE) {

        irp = IoCsqRemoveNextIrp(&pdoExt->WriteQueue, NULL);
        if (!irp)
            break;              /* No pending write IRPs             */

        {
            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
            CY_WRITE_CONTEXT writeCtx;
            ULONG totalLength;
            ULONG offset;
            ULONG remaining;

            totalLength = irpSp->Parameters.Write.Length;

            /* Check if this IRP has a partial write offset stored
             * from a previous CyDispatchWrite that couldn't fit all
             * bytes. DriverContext[0] holds the byte offset.
             * (Fix for TODO #4 — partial write queueing) */
            offset = (ULONG)(ULONG_PTR)irp->Tail.Overlay.DriverContext[0];
            if (offset >= totalLength)
                offset = 0;    /* Safety: reset if corrupt */

            remaining = totalLength - offset;

            writeCtx.Extension = pdoExt;
            writeCtx.Buffer = (PUCHAR)irp->AssociatedIrp.SystemBuffer + offset;
            writeCtx.Length = remaining;
            writeCtx.Written = 0;

            /* Copy data to TxBuf at DIRQL */
            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncWriteToTxBuf, &writeCtx);
            }

            if (writeCtx.Written < remaining) {
                /* Still more bytes to write. Update offset and
                 * re-queue the IRP for the next DPC cycle. */
                irp->Tail.Overlay.DriverContext[0] =
                    (PVOID)(ULONG_PTR)(offset + writeCtx.Written);

                IoCsqInsertIrp(&pdoExt->WriteQueue, irp, NULL);

                CyTrace("WRITE DPC: port %lu partial %lu/%lu (offset %lu), requeued\n",
                        pdoExt->PortIndex, writeCtx.Written,
                        remaining, offset);
                break;          /* TxBuf is full — wait for next DPC */
            }

            /* All bytes from this IRP are now in TxBuf.
             * Complete with the TOTAL byte count (offset + this batch). */
            irp->IoStatus.Status = STATUS_SUCCESS;
            irp->IoStatus.Information = totalLength;

            CyTrace("WRITE DPC: port %lu completed IRP %lu bytes total\n",
                    pdoExt->PortIndex, totalLength);

            IoCompleteRequest(irp, IO_SERIAL_INCREMENT);
        }
    }
}
