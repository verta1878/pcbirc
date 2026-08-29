/* ====================================================================
 * cyread.c — IRP_MJ_READ Dispatch and Read DPC Completion
 * ====================================================================
 * Handles ReadFile requests from applications.
 *
 * Data flow:
 *   1. ISR receives bytes from CD1400 RDSR → stores in RxBuf ring
 *   2. ISR queues ReadDpc via KeInsertQueueDpc
 *   3. ReadDpc dequeues pending read IRPs and fills them from RxBuf
 *
 * If data is already available in RxBuf when the Read IRP arrives,
 * we complete it immediately (fast path). If RxBuf is empty, we
 * queue the IRP in the cancel-safe read queue and return
 * STATUS_PENDING. The ReadDpc will complete it when data arrives.
 *
 * Ring buffer access from dispatch level uses
 * KeSynchronizeExecution to prevent races with the ISR.
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cyread.tmh"
#endif
/* csq.h — provided by owcompat.h for OW, by WDK for MSVC */
#ifndef __WATCOMC__
#include <csq.h>
#endif


/* ====================================================================
 * CySyncReadFromRxBuf — Copy data from RxBuf at DIRQL
 * ====================================================================
 * Runs via KeSynchronizeExecution. Copies up to 'requested' bytes
 * from the RX ring buffer to the caller's output buffer. Returns
 * the actual number of bytes copied in ctx->Data (reused as count).
 *
 * This is the SAFE way to read RxBuf from dispatch level. The ISR
 * writes to RxBuf at DIRQL; this callback runs at DIRQL too, so
 * they never overlap. Without KeSynchronizeExecution, the ISR could
 * modify RxHead/RxCount between our reads → torn data.
 * ==================================================================== */

/* ====================================================================
 * CySyncReadFromRxBuf — Copy data from RxBuf at DIRQL
 * ====================================================================
 * KeSynchronizeExecution callback. Copies up to 'Requested' bytes
 * from the RX ring buffer to the caller's output buffer. Returns
 * the actual count in ctx->Actual.
 *
 * This is the SAFE way to read RxBuf from dispatch level. The ISR
 * writes to RxBuf at DIRQL; this callback runs at DIRQL too, so
 * they never overlap.
 *
 * Context: CY_READ_CONTEXT
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

typedef struct _CY_READ_CONTEXT {
    PCY_PDO_EXT Extension;
    PUCHAR      Buffer;         /* Destination buffer (system buffer) */
    ULONG       Requested;      /* How many bytes the app wants       */
    ULONG       Actual;         /* How many bytes we actually copied  */
} CY_READ_CONTEXT, *PCY_READ_CONTEXT;

static BOOLEAN NTAPI CySyncReadFromRxBuf(PVOID Context)
{
    PCY_READ_CONTEXT ctx = (PCY_READ_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    ULONG copied = 0;

    /* Copy bytes from ring buffer to the caller's buffer.
     * We copy the minimum of: what the app requested, and what's
     * available in the ring buffer. */
    while (copied < ctx->Requested && ext->RxCount > 0) {
        ctx->Buffer[copied] = ext->RxBuf[ext->RxTail];
        ext->RxTail++;
        if (ext->RxTail >= CY_RING_BUF_SIZE)
            ext->RxTail = 0;   /* Wrap around                      */
        ext->RxCount--;
        copied++;
    }

    ctx->Actual = copied;
    return TRUE;
}


/* ====================================================================
 * CyDispatchRead — Handle IRP_MJ_READ (ReadFile)
 * ====================================================================
 * The application called ReadFile on the COM port. We need to
 * return received serial data.
 *
 * Fast path: If data is already in RxBuf, copy it and complete
 *            the IRP immediately. No queuing needed.
 *
 * Slow path: If RxBuf is empty, queue the IRP in the cancel-safe
 *            read queue. Return STATUS_PENDING. The ISR will fill
 *            RxBuf and queue the ReadDpc, which will dequeue this
 *            IRP and complete it with the data.
 *
 * The SystemBuffer (from DO_BUFFERED_IO) is the kernel-mode copy
 * of the user's buffer. We write to it, and the I/O manager copies
 * it back to user space when we complete the IRP.
 * ==================================================================== */

NTSTATUS CyDispatchRead(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT      common = CyGetCommon(DeviceObject);
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            status;

    /* FDOs don't handle Read — only PDOs */
    if (common->IsFDO) {
        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    {
        PCY_PDO_EXT pdoExt = CyGetPdo(DeviceObject);

        /* Acquire remove lock — prevents device removal during I/O */
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

        /* ---- Fast path: try to satisfy from ring buffer ----
         * If RxBuf has data, copy it directly and complete.
         * This avoids the overhead of queuing + DPC for the
         * common case where data is already waiting.
         *
         * NOTE (audit B2): We read RxCount here WITHOUT
         * KeSynchronizeExecution. This is an OPTIMIZATION HINT,
         * not a correctness requirement. RxCount is modified by
         * the ISR at DIRQL, so our read may be stale. Both races
         * are safe:
         *   - Stale zero: we take the slow path, queue the IRP,
         *     DPC completes it when data arrives. Correct.
         *   - Stale non-zero: KeSyncExec finds RxCount==0, copies
         *     0 bytes, we fall to the slow path. Correct.
         * The actual buffer access inside KeSyncExec is fully
         * synchronized. */
        if (pdoExt->RxCount > 0 && pdoExt->InterruptConnected) {
            CY_READ_CONTEXT readCtx;
            readCtx.Extension = pdoExt;
            readCtx.Buffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;
            readCtx.Requested = irpSp->Parameters.Read.Length;
            readCtx.Actual = 0;

            /* Read from RxBuf at DIRQL — synchronized with ISR */
            KeSynchronizeExecution(pdoExt->Interrupt,
                                   CySyncReadFromRxBuf, &readCtx);

            if (readCtx.Actual > 0) {
                /* Got data — complete immediately */
                Irp->IoStatus.Status = STATUS_SUCCESS;
                Irp->IoStatus.Information = readCtx.Actual;

                TraceEvents(CYPORT_LEVEL_VERBOSE, CYPORT_READ,
                        "READ port %lu fast %lu bytes", pdoExt->PortIndex, readCtx.Actual);
                CyTrace("READ: port %lu fast-path %lu bytes\n",
                        pdoExt->PortIndex, readCtx.Actual);

                IoCompleteRequest(Irp, IO_SERIAL_INCREMENT);
                IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
                return STATUS_SUCCESS;
            }
        }

        /* ---- Slow path: no data available — queue the IRP ----
         * Mark as pending FIRST (before inserting into queue).
         *
         * If the application set read timeouts via SetCommTimeouts,
         * we start a timer. When the timer fires, the DPC dequeues
         * the IRP and completes it with whatever data (if any) has
         * arrived, or with 0 bytes if no data came.
         *
         * SERIAL_TIMEOUTS rules (Win32 SetCommTimeouts):
         *   ReadIntervalTimeout = MAXULONG, Multiplier = 0, Constant = 0:
         *     → Return immediately with whatever is in the buffer
         *   ReadTotalTimeoutConstant > 0:
         *     → Total timeout = Constant + (Multiplier × BytesRequested)
         *   All zeros:
         *     → Wait forever until data arrives
         *
         * (sysop/0 missing feature fix — SERIAL_TIMEOUTS) */

        /* Check for "return immediately" timeout */
        if (pdoExt->Timeouts.ReadIntervalTimeout == (ULONG)(-1) &&
            pdoExt->Timeouts.ReadTotalTimeoutMultiplier == 0 &&
            pdoExt->Timeouts.ReadTotalTimeoutConstant == 0) {
            /* Return immediately with 0 bytes — no data available */
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0;
            CyTrace("READ: port %lu immediate return (MAXULONG timeout)\n",
                    pdoExt->PortIndex);
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
            return STATUS_SUCCESS;
        }

        IoMarkIrpPending(Irp);

        /* Calculate total timeout and store in IRP driver context.
         * The ReadDpc checks this to decide when to timeout. */
        {
            ULONG totalTimeoutMs = 0;
            ULONG bytesRequested = irpSp->Parameters.Read.Length;

            if (pdoExt->Timeouts.ReadTotalTimeoutConstant > 0 ||
                pdoExt->Timeouts.ReadTotalTimeoutMultiplier > 0) {
                totalTimeoutMs = pdoExt->Timeouts.ReadTotalTimeoutConstant +
                    (pdoExt->Timeouts.ReadTotalTimeoutMultiplier * bytesRequested);
            }
            /* Store timeout deadline (tick count) in DriverContext.
             * 0 = no timeout (wait forever). */
            if (totalTimeoutMs > 0) {
                LARGE_INTEGER tickCount;
                KeQueryTickCount(&tickCount);
                /* Convert ms to ticks: ticks = ms / KeQueryTimeIncrement()
                 * Approximate: 1 tick ≈ 15.6ms on most systems */
                Irp->Tail.Overlay.DriverContext[1] =
                    (PVOID)(ULONG_PTR)(tickCount.LowPart +
                        (totalTimeoutMs / 16) + 1);
            } else {
                Irp->Tail.Overlay.DriverContext[1] = NULL; /* No timeout */
            }
        }

        /* Insert into the cancel-safe read queue.
         * If the IRP is canceled between now and the DPC, the
         * IoCsq framework will call our CsqCompleteCanceled
         * callback to complete it with STATUS_CANCELLED. */
        IoCsqInsertIrp(&pdoExt->ReadQueue, Irp, NULL);

        CyTrace("READ: port %lu queued (RxCount=%lu, waiting for data)\n",
                pdoExt->PortIndex, pdoExt->RxCount);

        /* Release remove lock — but DON'T complete the IRP.
         * The DPC or cancel routine will complete it later.
         * We release the remove lock here because we're returning
         * STATUS_PENDING. The DPC will NOT re-acquire the remove
         * lock — the IRP's lifetime is now managed by the queue. */
        IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);

        return STATUS_PENDING;
    }
}


/* ====================================================================
 * CyReadDpcComplete — Read DPC implementation
 * ====================================================================
 * Called from the ReadDpc (queued by the ISR when data arrives).
 * Dequeues pending read IRPs and fills them from RxBuf.
 *
 * This replaces the stub CyReadDpcRoutine in cyisr.c. The actual
 * DPC routine in cyisr.c should call this function.
 *
 * Runs at DISPATCH_LEVEL. Can complete IRPs but CANNOT:
 *   - Access paged memory
 *   - Block or wait
 *   - Allocate paged pool
 * ==================================================================== */

VOID CyReadDpcComplete(PCY_PDO_EXT pdoExt)
{
    PIRP    irp;
    LARGE_INTEGER tickCount;

    KeQueryTickCount(&tickCount);

    /* Loop: dequeue pending read IRPs and fill from RxBuf.
     * Also check for timed-out IRPs and complete them with
     * whatever data is available (or 0 bytes if none).
     *
     * The ReadDpc fires both when data arrives AND periodically
     * (via the modem DPC which fires on signal changes). This
     * ensures timed-out reads get completed even if no data comes.
     * (sysop/0 missing feature fix — SERIAL_TIMEOUTS) */

    for (;;) {
        irp = IoCsqRemoveNextIrp(&pdoExt->ReadQueue, NULL);
        if (!irp)
            break;

        {
            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
            CY_READ_CONTEXT readCtx;
            ULONG_PTR deadline;
            BOOLEAN timedOut = FALSE;

            /* Check if this IRP has timed out */
            deadline = (ULONG_PTR)irp->Tail.Overlay.DriverContext[1];
            if (deadline != 0 && tickCount.LowPart >= (ULONG)deadline) {
                timedOut = TRUE;
            }

            /* Try to read data regardless of timeout —
             * return whatever is available */
            readCtx.Extension = pdoExt;
            readCtx.Buffer = (PUCHAR)irp->AssociatedIrp.SystemBuffer;
            readCtx.Requested = irpSp->Parameters.Read.Length;
            readCtx.Actual = 0;

            if (pdoExt->RxCount > 0 &&
                pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncReadFromRxBuf, &readCtx);
            }

            if (readCtx.Actual > 0 || timedOut) {
                /* Got data, or timed out — complete the IRP.
                 * STATUS_SUCCESS with 0 bytes = timeout (valid Win32). */
                irp->IoStatus.Status = STATUS_SUCCESS;
                irp->IoStatus.Information = readCtx.Actual;

                CyTrace("READ DPC: port %lu completed %lu bytes%s\n",
                        pdoExt->PortIndex, readCtx.Actual,
                        timedOut ? " (timeout)" : "");

                IoCompleteRequest(irp, IO_SERIAL_INCREMENT);
            } else {
                /* No data and not timed out — re-queue and wait */
                IoCsqInsertIrp(&pdoExt->ReadQueue, irp, NULL);
                break;  /* Don't spin — wait for next ISR/DPC */
            }
        }
    }
}
