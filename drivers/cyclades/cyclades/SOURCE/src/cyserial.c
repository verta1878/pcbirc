/* ====================================================================
 * cyserial.c — Serial Port Core (Open/Close/Cleanup/Channel Init)
 * ====================================================================
 * Handles IRP_MJ_CREATE, IRP_MJ_CLOSE, IRP_MJ_CLEANUP, and provides
 * the IoCsq cancel-safe queue infrastructure used by Read and Write.
 *
 * File organization:
 *   cyserial.c  — this file (open/close/cleanup, IoCsq, channel init)
 *   cyread.c    — IRP_MJ_READ dispatch + read DPC completion
 *   cywrite.c   — IRP_MJ_WRITE dispatch + write DPC completion
 *   cyisr.c     — ISR + DPC routines (ISR fills RxBuf, drains TxBuf)
 *
 * IoCsq (Cancel-Safe Queue) overview:
 * ====================================
 * When a Read or Write IRP can't be completed immediately (no data
 * available for read, or TX buffer full for write), we queue the IRP
 * in a cancel-safe queue. The IoCsq framework (linked via Csq.lib
 * for Win2K backward compat) handles the race condition between
 * IRP cancellation and queue insertion.
 *
 * We provide 6 callbacks per queue:
 *   CsqInsertIrp      — insert IRP into our linked list
 *   CsqRemoveIrp       — remove a specific IRP from the list
 *   CsqPeekNextIrp     — find the next IRP matching a criteria
 *   CsqAcquireLock     — acquire our queue spinlock
 *   CsqReleaseLock     — release our queue spinlock
 *   CsqCompleteCanceledIrp — complete a canceled IRP
 *
 * The framework calls these under the hood when we call
 * IoCsqInsertIrp (to queue) or IoCsqRemoveNextIrp (to dequeue).
 *
 * Sources:
 *   - WDK cancel sample (src/general/cancel/)
 *   - MSDN Cancel-Safe IRP Queues documentation
 *   - Original cyyport.sys: IoAcquireCancelSpinLock (manual pattern;
 *     we use IoCsq which is cleaner and available via Csq.lib on Win2K)
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cyserial.tmh"
#endif
/* Forward declarations for IoCsq callbacks.
 * OW needs these before IoCsqInitialize to match calling convention. */
static VOID NTAPI CyReadCsqInsert(PIO_CSQ, PIRP);
static VOID NTAPI CyReadCsqRemove(PIO_CSQ, PIRP);
static PIRP NTAPI CyReadCsqPeekNext(PIO_CSQ, PIRP, PVOID);
static VOID NTAPI CyReadCsqAcquireLock(PIO_CSQ, PKIRQL);
static VOID NTAPI CyReadCsqReleaseLock(PIO_CSQ, PKIRQL);
static VOID NTAPI CyReadCsqCompleteCanceled(PIO_CSQ, PIRP);
static VOID NTAPI CyWriteCsqInsert(PIO_CSQ, PIRP);
static VOID NTAPI CyWriteCsqRemove(PIO_CSQ, PIRP);
static PIRP NTAPI CyWriteCsqPeekNext(PIO_CSQ, PIRP, PVOID);
static VOID NTAPI CyWriteCsqAcquireLock(PIO_CSQ, PKIRQL);
static VOID NTAPI CyWriteCsqReleaseLock(PIO_CSQ, PKIRQL);
static VOID NTAPI CyWriteCsqCompleteCanceled(PIO_CSQ, PIRP);



/* IoCsq header — provides IoCsqInitialize and related functions.
 * On XP+, these are in wdm.h. On Win2K, we link against Csq.lib
 * which provides the implementation. The SOURCES file includes:
 *   TARGETLIBS=$(DDK_LIB_PATH)\csq.lib */
/* csq.h — provided by owcompat.h for OW, by WDK for MSVC */
#ifndef __WATCOMC__
#include <csq.h>
#endif


/* ====================================================================
 * Forward declarations for IoCsq callbacks
 * ==================================================================== */




/* ====================================================================
 * CyInitCancelSafeQueues — Initialize IoCsq for a port
 * ====================================================================
 * Called from PDO START_DEVICE. Sets up the cancel-safe IRP queues
 * for read and write operations. Must be called before any IRP
 * can be queued.
 *
 * This closes critical gaps G18 and G19 (no cancel-safe queues).
 * ==================================================================== */

NTSTATUS CyInitCancelSafeQueues(PCY_PDO_EXT pdoExt)
{
    NTSTATUS status;

    /* ---- Initialize the READ cancel-safe queue ----
     * IoCsqInitialize binds our 6 callback functions to the IO_CSQ
     * structure in the PDO extension. After this call, we can use
     * IoCsqInsertIrp and IoCsqRemoveNextIrp to safely manage
     * pending read IRPs.
     *
     * On Win2K, this function is provided by Csq.lib (statically
     * linked). On XP+, it's a kernel export. Either way, the API
     * is identical. */
    status = IoCsqInitialize(
        &pdoExt->ReadQueue,
        CyReadCsqInsert,
        CyReadCsqRemove,
        CyReadCsqPeekNext,
        CyReadCsqAcquireLock,
        CyReadCsqReleaseLock,
        CyReadCsqCompleteCanceled);

    if (!NT_SUCCESS(status)) {
        CyError("IoCsqInitialize(ReadQueue) failed: 0x%08X\n",
                 status);
        return status;
    }

    /* ---- Initialize the WRITE cancel-safe queue ---- */
    status = IoCsqInitialize(
        &pdoExt->WriteQueue,
        CyWriteCsqInsert,
        CyWriteCsqRemove,
        CyWriteCsqPeekNext,
        CyWriteCsqAcquireLock,
        CyWriteCsqReleaseLock,
        CyWriteCsqCompleteCanceled);

    if (!NT_SUCCESS(status)) {
        CyError("IoCsqInitialize(WriteQueue) failed: 0x%08X\n",
                 status);
        return status;
    }

    return STATUS_SUCCESS;
}


/* ====================================================================
 * IoCsq Callbacks — READ Queue
 * ====================================================================
 * These 6 functions are called by the IoCsq framework to manage
 * the read IRP queue. We provide the linked list and spinlock;
 * IoCsq handles the cancel-safety logic.
 *
 * The pattern is identical for read and write queues — only the
 * list head and spinlock differ. We could use a single set of
 * callbacks with CONTAINING_RECORD to find the PDO extension,
 * but separate callbacks are clearer for a driver that many
 * people will read.
 * ==================================================================== */

/* ---- Insert an IRP into the read queue ---- */
static VOID NTAPI CyReadCsqInsert(PIO_CSQ Csq, PIRP Irp)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, ReadQueue);

    /* Insert at the tail — FIFO order. First IRP queued is the
     * first one dequeued when data arrives. */
    InsertTailList(&pdoExt->ReadQueueHead,
                   &Irp->Tail.Overlay.ListEntry);

    CyVerbose("CSQ: Read IRP %p queued on port %lu\n",
              Irp, pdoExt->PortIndex);
}

/* ---- Remove a specific IRP from the read queue ----
 * Called by IoCsq when an IRP is being canceled or dequeued. */
static VOID NTAPI CyReadCsqRemove(PIO_CSQ Csq, PIRP Irp)
{
    UNREFERENCED_PARAMETER(Csq);

    /* RemoveEntryList unhooks the IRP from the doubly-linked list.
     * The IRP's ListEntry fields are set to point to themselves
     * after removal (standard NT list behavior). */
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}

/* ---- Find the next IRP in the read queue ----
 * If Irp is NULL, return the first IRP.
 * If Irp is non-NULL, return the IRP after it.
 * PeekContext can be used for filtering (we don't use it). */
static PIRP NTAPI CyReadCsqPeekNext(PIO_CSQ Csq, PIRP Irp, PVOID PeekContext)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, ReadQueue);
    PLIST_ENTRY head = &pdoExt->ReadQueueHead;
    PLIST_ENTRY entry;

    UNREFERENCED_PARAMETER(PeekContext);

    /* Start from the IRP after the given one, or from the head.
     * We don't use PeekContext filtering — return the first valid
     * entry we find. (Audit B1 fix: original code had a while loop
     * that returned on the first iteration, never advancing. Rewritten
     * as a clear if-check since we don't filter.) */
    if (Irp == NULL) {
        entry = head->Flink;    /* First entry in the list          */
    } else {
        entry = Irp->Tail.Overlay.ListEntry.Flink;  /* Next entry  */
    }

    if (entry != head) {
        return CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
    }

    return NULL;                /* Queue is empty                    */
}

/* ---- Acquire the read queue spinlock ----
 * Called by IoCsq before any insert/remove/peek operation.
 * We save the old IRQL so it can be restored on release. */
static VOID NTAPI CyReadCsqAcquireLock(PIO_CSQ Csq, PKIRQL Irql)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, ReadQueue);
    KeAcquireSpinLock(&pdoExt->ReadQueueLock, Irql);
}

/* ---- Release the read queue spinlock ---- */
static VOID NTAPI CyReadCsqReleaseLock(PIO_CSQ Csq, PKIRQL Irql)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, ReadQueue);
    KeReleaseSpinLock(&pdoExt->ReadQueueLock, *Irql);
}

/* ---- Complete a canceled read IRP ----
 * Called by IoCsq when an IRP in our queue has been canceled
 * (e.g., the application called CancelIo or closed the handle).
 * The IRP has already been removed from our queue by CsqRemove. */
static VOID NTAPI CyReadCsqCompleteCanceled(PIO_CSQ Csq, PIRP Irp)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, ReadQueue);
    UNREFERENCED_PARAMETER(pdoExt);

    CyTrace("CSQ: Read IRP %p CANCELLED on port %lu\n",
            Irp, pdoExt->PortIndex);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


/* ====================================================================
 * IoCsq Callbacks — WRITE Queue
 * ====================================================================
 * Identical pattern to read queue, using WriteQueueHead and
 * WriteQueueLock instead. See read callbacks above for detailed
 * explanations of each function's purpose.
 * ==================================================================== */

/* ---- Insert an IRP into the write queue (FIFO order) ---- */
static VOID NTAPI CyWriteCsqInsert(PIO_CSQ Csq, PIRP Irp)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, WriteQueue);
    InsertTailList(&pdoExt->WriteQueueHead,
                   &Irp->Tail.Overlay.ListEntry);
}

/* ---- Remove a specific IRP from the write queue ---- */
static VOID NTAPI CyWriteCsqRemove(PIO_CSQ Csq, PIRP Irp)
{
    UNREFERENCED_PARAMETER(Csq);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
}

/* ---- Find the next IRP in the write queue ---- */
static PIRP NTAPI CyWriteCsqPeekNext(PIO_CSQ Csq, PIRP Irp, PVOID PeekContext)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, WriteQueue);
    PLIST_ENTRY head = &pdoExt->WriteQueueHead;
    PLIST_ENTRY entry;

    UNREFERENCED_PARAMETER(PeekContext);

    /* Same pattern as read PeekNext — see audit B1 fix comment there */
    if (Irp == NULL) {
        entry = head->Flink;
    } else {
        entry = Irp->Tail.Overlay.ListEntry.Flink;
    }

    if (entry != head) {
        return CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
    }

    return NULL;
}

/* ---- Acquire the write queue spinlock ---- */
static VOID NTAPI CyWriteCsqAcquireLock(PIO_CSQ Csq, PKIRQL Irql)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, WriteQueue);
    KeAcquireSpinLock(&pdoExt->WriteQueueLock, Irql);
}

/* ---- Release the write queue spinlock ---- */
static VOID NTAPI CyWriteCsqReleaseLock(PIO_CSQ Csq, PKIRQL Irql)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, WriteQueue);
    KeReleaseSpinLock(&pdoExt->WriteQueueLock, *Irql);
}

static VOID NTAPI CyWriteCsqCompleteCanceled(PIO_CSQ Csq, PIRP Irp)
{
    PCY_PDO_EXT pdoExt = CONTAINING_RECORD(Csq, CY_PDO_EXT, WriteQueue);
    UNREFERENCED_PARAMETER(pdoExt);

    CyTrace("CSQ: Write IRP %p CANCELLED on port %lu\n",
            Irp, pdoExt->PortIndex);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}


/* ====================================================================
 * CyInitChannel — Initialize a CD1400 channel for serial I/O
 * ====================================================================
 * Resets the channel, sets 8N1 at the current baud rate, enables
 * the transmitter and receiver, and raises DTR + RTS.
 *
 * MUST be called via KeSynchronizeExecution if the ISR is connected,
 * because we write to per-channel registers (COR1, COR2, etc.)
 * which require CAR to be set correctly.
 *
 * If the ISR is not yet connected (first open before interrupt
 * connect), it's safe to call directly.
 * ==================================================================== */

BOOLEAN NTAPI CyInitChannelSync(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;

    /* Select this port's channel on the chip.
     * Safe here because we're either at DIRQL (KeSyncExec) or
     * the ISR isn't connected yet. */
    CySelectChannel(ext->ChipBase, ext->Channel);

    /* ---- Reset the channel ----
     * CyCHAN_RESET (0x80) resets the channel state machine, clears
     * FIFOs, and returns all per-channel registers to defaults.
     * We must wait for the chip to accept the command — CCR reads
     * as 0 when the command has been processed. */
    CyWriteReg(ext->ChipBase, CyCCR, CyCHAN_RESET);

    /* Brief delay for reset — KeStallExecutionProcessor is the
     * only safe delay at DIRQL (it busy-waits, doesn't block).
     * 100 microseconds is generous for a chip reset. */
    KeStallExecutionProcessor(100);

    /* ---- Set Channel Option Register 1 (COR1) ----
     * Default: 8 data bits, no parity, 1 stop bit.
     * Cy_8_BITS  = 0x03 (bits 1:0 = data width)
     * Cy_1_STOP  = 0x00 (bits 3:2 = stop bits)
     * CyPARITY_NONE = 0x00 (bits 7:5 = parity mode) */
    CyWriteReg(ext->ChipBase, CyCOR1,
               Cy_8_BITS | Cy_1_STOP | CyPARITY_NONE);

    /* ---- Set COR2 — flow control options ----
     * CyRtsAO (0x04): RTS automatic output. The chip asserts RTS
     * when the RX FIFO has space and deasserts when full. This
     * provides hardware flow control without software intervention. */
    CyWriteReg(ext->ChipBase, CyCOR2, CyRtsAO);

    /* ---- Set COR3 — RX FIFO threshold ----
     * When the RX FIFO reaches this level, the chip generates a
     * receive service request. Setting it to FIFO_SIZE - 2 (= 10)
     * gives us 2 bytes of headroom before overflow. */
    CyWriteReg(ext->ChipBase, CyCOR3, CY_MAX_CHAR_FIFO - 2);

    /* ---- Tell chip to re-read all COR registers ----
     * The CD1400 doesn't apply COR changes immediately. We must
     * write a COR_CHANGE command to CCR specifying which CORs
     * changed. 0x0E = CyCOR1ch | CyCOR2ch | CyCOR3ch (all three). */
    CyWriteReg(ext->ChipBase, CyCCR,
               CyCOR_CHANGE | CyCOR1ch | CyCOR2ch | CyCOR3ch);

    /* Brief delay for COR re-read */
    KeStallExecutionProcessor(10);

    /* ---- Enable transmitter and receiver ----
     * CyENB_RCVR (0x02) + CyENB_XMTR (0x08) = 0x0A.
     * After this, the channel is live and will generate interrupts
     * for received data (if SRER RxData bit is set). */
    CyWriteReg(ext->ChipBase, CyCCR, CyENB_RCVR | CyENB_XMTR);
    KeStallExecutionProcessor(10);

    /* ---- Set baud rate ----
     * Write the baud prescaler and clock option registers.
     * This uses the baud tables from cd1400.h.
     * Baud rate is set via IOCTL_SERIAL_SET_BAUD_RATE after open. */

    /* ---- Raise DTR and RTS ----
     * CyDTR (0x02) = Data Terminal Ready
     * CyRTS (0x01) = Request To Send
     * Both written to MSVR1 and MSVR2. The original driver writes
     * both registers — MSVR1 controls the physical output pins,
     * MSVR2 is a shadow/alias on some CD1400 revisions. */
    CyWriteReg(ext->ChipBase, CyMSVR1, CyDTR | CyRTS);
    CyWriteReg(ext->ChipBase, CyMSVR2, CyDTR | CyRTS);

    /* ---- Enable receive interrupts ----
     * Set RxData bit in SRER so the chip generates service requests
     * when data arrives. We DON'T enable TxRdy yet — that gets
     * enabled when the write path has data to send.
     * CyMdmCh enables modem change interrupts (CTS/DSR/DCD/RI). */
    CyWriteReg(ext->ChipBase, CySRER, CyRxData | CyMdmCh);

    /* ---- Clear ring buffers ----
     * Start fresh — no stale data from previous open. */
    ext->RxHead = ext->RxTail = ext->RxCount = 0;
    ext->TxHead = ext->TxTail = ext->TxCount = 0;

    /* ---- Read initial modem status ----
     * Read both MSVR1 and MSVR2 and OR them together.
     * (Audit B1 platform-difference fix) */
    ext->ShadowMSVR = CyReadReg(ext->ChipBase, CyMSVR1)
                    | CyReadReg(ext->ChipBase, CyMSVR2);
    ext->ModemControl = SERIAL_DTR_STATE | SERIAL_RTS_STATE;

    /* ---- Clear event history ---- */
    ext->EventHistory = 0;

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * CyShutdownChannel — Shut down a CD1400 channel
 * ====================================================================
 * Disables transmitter and receiver, drops DTR and RTS.
 * Called from Close or Remove. Must be synchronized with the ISR. */

BOOLEAN NTAPI CyShutdownChannelSync(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;

    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Disable all interrupts for this channel */
    CyWriteReg(ext->ChipBase, CySRER, 0);

    /* Disable transmitter and receiver.
     * CyDIS_RCVR (0x01) | CyDIS_XMTR (0x04) = 0x05 */
    CyWriteReg(ext->ChipBase, CyCCR, CyDIS_RCVR | CyDIS_XMTR);

    /* Drop DTR and RTS — writing 0 clears both signal bits.
     * This tells the remote end that we're no longer ready. */
    CyWriteReg(ext->ChipBase, CyMSVR1, 0);
    CyWriteReg(ext->ChipBase, CyMSVR2, 0);

    ext->ModemControl = 0;

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * CyDispatchCreate — Handle IRP_MJ_CREATE (OpenFile)
 * ====================================================================
 * Called when an application calls CreateFile("COM3").
 *
 * We initialize the CD1400 channel, set up cancel-safe queues, and
 * mark the port as open. If the port is already open, we return
 * STATUS_ACCESS_DENIED (serial ports are exclusive by convention).
 * ==================================================================== */

NTSTATUS CyDispatchCreate(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT  common = CyGetCommon(DeviceObject);
    NTSTATUS        status;

    /* FDOs don't handle serial I/O — only PDOs do */
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

        /* Check if device is started */
        if (pdoExt->Common.PnPState != CyStarted) {
            status = STATUS_DEVICE_NOT_READY;
            goto CreateDone;
        }

        /* Serial ports are exclusive — only one handle at a time.
         * If already open, reject with ACCESS_DENIED. Applications
         * expect this behavior from COM ports. */
        if (pdoExt->IsOpen) {
            status = STATUS_ACCESS_DENIED;
            goto CreateDone;
        }

        /* Initialize the CD1400 channel.
         * If the ISR is connected, use KeSynchronizeExecution.
         * If not (first open before interrupt connect), call directly. */
        {
            CY_SYNC_CONTEXT ctx;
            ctx.Extension = pdoExt;
            ctx.Data = NULL;

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CyInitChannelSync, &ctx);
            } else {
                /* ISR not connected yet — safe to call directly.
                 * This happens during initial setup before
                 * IoConnectInterrupt is called. */
                CyInitChannelSync(&ctx);
            }
            status = ctx.Status;
        }

        if (NT_SUCCESS(status)) {
            pdoExt->IsOpen = TRUE;

            CyInfo("Port %lu opened (chip %u chan %u)\n",
                     pdoExt->PortIndex, pdoExt->ChipIndex,
                     pdoExt->Channel);
        }

    CreateDone:
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
        return status;
    }
}


/* ====================================================================
 * CyDispatchClose — Handle IRP_MJ_CLOSE (CloseHandle)
 * ====================================================================
 * Called when the last handle to the device is closed AND all
 * pending I/O has completed (Cleanup runs first and cancels
 * pending IRPs).
 *
 * We shut down the CD1400 channel (drop DTR/RTS, disable TX/RX)
 * and mark the port as closed.
 * ==================================================================== */

NTSTATUS CyDispatchClose(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT common = CyGetCommon(DeviceObject);

    if (common->IsFDO) {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    {
        PCY_PDO_EXT pdoExt = CyGetPdo(DeviceObject);

        /* Don't need remove lock for close — the I/O manager
         * guarantees Close is only sent after all other IRPs
         * complete and Cleanup has finished. */

        if (pdoExt->IsOpen) {
            /* Shut down the channel — drop signals, disable TX/RX */
            CY_SYNC_CONTEXT ctx;
            ctx.Extension = pdoExt;
            ctx.Data = NULL;

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CyShutdownChannelSync, &ctx);
            } else {
                CyShutdownChannelSync(&ctx);
            }

            pdoExt->IsOpen = FALSE;

            CyInfo("Port %lu closed\n", pdoExt->PortIndex);
        }

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }
}


/* ====================================================================
 * CyDispatchCleanup — Handle IRP_MJ_CLEANUP
 * ====================================================================
 * Called when the last handle to the device is closed, BEFORE
 * IRP_MJ_CLOSE. Our job is to cancel all pending IRPs that belong
 * to the file object being closed.
 *
 * This is critical for correctness: if an application calls
 * CloseHandle while a ReadFile is pending on another thread,
 * Cleanup cancels the pending Read before Close runs. Without
 * Cleanup, the pending Read would hold the port open indefinitely.
 *
 * We drain both the read and write cancel-safe queues, completing
 * every IRP with STATUS_CANCELLED.
 * ==================================================================== */

NTSTATUS CyDispatchCleanup(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT common = CyGetCommon(DeviceObject);

    if (common->IsFDO) {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    {
        PCY_PDO_EXT pdoExt = CyGetPdo(DeviceObject);
        PIRP        pendingIrp;
        NTSTATUS    status;

        status = IoAcquireRemoveLock(&pdoExt->Common.RemoveLock, Irp);
        if (!NT_SUCCESS(status)) {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;
        }

        /* ---- Cancel all pending read IRPs ----
         * IoCsqRemoveNextIrp removes and returns the next IRP from
         * the queue. We loop until it returns NULL (queue empty).
         * Each removed IRP is completed with STATUS_CANCELLED. */
        while ((pendingIrp = IoCsqRemoveNextIrp(
                    &pdoExt->ReadQueue, NULL)) != NULL) {
            pendingIrp->IoStatus.Status = STATUS_CANCELLED;
            pendingIrp->IoStatus.Information = 0;
            IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
        }

        /* ---- Cancel all pending write IRPs ---- */
        while ((pendingIrp = IoCsqRemoveNextIrp(
                    &pdoExt->WriteQueue, NULL)) != NULL) {
            pendingIrp->IoStatus.Status = STATUS_CANCELLED;
            pendingIrp->IoStatus.Information = 0;
            IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
        }

        /* ---- Cancel pending WaitCommEvent IRP ---- */
        {
            KIRQL oldIrql;
            KeAcquireSpinLock(&pdoExt->EventLock, &oldIrql);
            if (pdoExt->WaitIrp) {
                PIRP waitIrp = pdoExt->WaitIrp;
                pdoExt->WaitIrp = NULL;
                KeReleaseSpinLock(&pdoExt->EventLock, oldIrql);

                waitIrp->IoStatus.Status = STATUS_CANCELLED;
                waitIrp->IoStatus.Information = 0;
                IoCompleteRequest(waitIrp, IO_NO_INCREMENT);
            } else {
                KeReleaseSpinLock(&pdoExt->EventLock, oldIrql);
            }
        }

        CyInfo("Port %lu cleanup — all pending IRPs canceled\n",
                 pdoExt->PortIndex);

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
        return STATUS_SUCCESS;
    }
}
