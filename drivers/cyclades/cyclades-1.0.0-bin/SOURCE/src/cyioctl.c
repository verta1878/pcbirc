/* ====================================================================
 * cyioctl.c — Serial Port IOCTL Dispatch
 * ====================================================================
 * Handles IRP_MJ_DEVICE_CONTROL for all IOCTL_SERIAL_* functions.
 *
 * This is what makes real applications work. When an app calls
 * SetCommState, GetCommModemStatus, SetCommTimeouts, WaitCommEvent,
 * etc., the Win32 API translates these into IOCTL_SERIAL_* calls
 * which arrive here as IRP_MJ_DEVICE_CONTROL IRPs.
 *
 * Every IOCTL follows the same pattern:
 *   1. Validate input/output buffer sizes
 *   2. Acquire appropriate lock (PortLock, KeSyncExec, or EventLock)
 *   3. Read/write CD1400 registers or port state
 *   4. Set Irp->IoStatus.Information = bytes returned
 *   5. Complete the IRP
 *
 * Debug output: every IOCTL entry and exit is logged with CyDbgPrint
 * at CY_TRACE level. Critical state changes (baud, line control,
 * modem signals) are logged at CY_INFO level.
 *
 * Sources:
 *   - ntddser.h (IOCTL definitions and structures)
 *   - Original cyyport.sys import analysis (68 kernel imports)
 *   - WIN_LINUX_COMPARISON.md (IOCTL coverage checklist)
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cyioctl.tmh"
#endif

/* ====================================================================
 * Baud Rate Tables
 * ====================================================================
 * Two tables: 25 MHz (Rev G, CD1400) and 60 MHz (Rev J, CD1400).
 * Each entry has a clock option byte (written to TCOR/RCOR) and
 * a baud period register byte (written to TBPR/RBPR).
 *
 * The CD1400 generates baud as:
 *   baud = clock / (2 × prescaler × BPR)
 * where prescaler is selected by the clock option bits.
 *
 * These tables are identical to those in the Linux cyclades.c driver
 * and the DOS FOSSIL driver (cyfossil.asm). All three platforms use
 * the same hardware, so the register values are identical.
 * ==================================================================== */

typedef struct _CY_BAUD_ENTRY {
    ULONG   Baud;               /* Baud rate (e.g., 9600)           */
    UCHAR   ClockOpt;           /* Clock option (TCOR/RCOR value)   */
    UCHAR   BaudPeriod;         /* Baud period (TBPR/RBPR value)    */
} CY_BAUD_ENTRY;

static const CY_BAUD_ENTRY CyBaudTable25MHz[] = {
    {     50, 0x04, 0xF5 }, {     75, 0x04, 0xA3 },
    {    110, 0x04, 0x6F }, {    134, 0x04, 0x5C },
    {    150, 0x04, 0x51 }, {    200, 0x04, 0x41 },
    {    300, 0x03, 0xF5 }, {    600, 0x03, 0xA3 },
    {   1200, 0x03, 0x51 }, {   1800, 0x02, 0xA3 },
    {   2400, 0x02, 0x6D }, {   4800, 0x02, 0x51 },
    {   9600, 0x01, 0xA3 }, {  19200, 0x01, 0x51 },
    {  38400, 0x00, 0xA3 }, {  57600, 0x00, 0x51 },
    {  76800, 0x00, 0x36 }, { 115200, 0x00, 0x1B },
    { 0, 0, 0 }                 /* Sentinel — end of table          */
};

static const CY_BAUD_ENTRY CyBaudTable60MHz[] = {
    {     50, 0x00, 0x00 }, {     75, 0x00, 0x82 },
    {    110, 0x00, 0x21 }, {    134, 0x04, 0xFF },
    {    150, 0x04, 0xDB }, {    200, 0x04, 0xC3 },
    {    300, 0x04, 0x92 }, {    600, 0x04, 0x62 },
    {   1200, 0x03, 0xC3 }, {   1800, 0x03, 0x62 },
    {   2400, 0x03, 0x41 }, {   4800, 0x02, 0xC3 },
    {   9600, 0x02, 0x62 }, {  19200, 0x01, 0xC3 },
    {  38400, 0x01, 0x62 }, {  57600, 0x00, 0xC3 },
    {  76800, 0x00, 0x82 }, { 115200, 0x00, 0x62 },
    { 150000, 0x00, 0x41 }, { 230400, 0x00, 0x32 },
    { 0, 0, 0 }                 /* Sentinel                         */
};


/* ====================================================================
 * CyLookupBaud — Find baud table entry for a given rate
 * ====================================================================
 * Returns pointer to the matching table entry, or NULL if the
 * requested baud rate isn't supported by this chip revision.
 * ==================================================================== */

static const CY_BAUD_ENTRY *CyLookupBaud(
    BOOLEAN is60MHz, ULONG baudRate)
{
    const CY_BAUD_ENTRY *table;
    ULONG i;

    table = is60MHz ? CyBaudTable60MHz : CyBaudTable25MHz;

    for (i = 0; table[i].Baud != 0; i++) {
        if (table[i].Baud == baudRate)
            return &table[i];
    }

    return NULL;                /* Unsupported baud rate             */
}


/* ====================================================================
 * KeSynchronizeExecution callback: Set baud rate
 * ====================================================================
 * Writes the clock option and baud period registers for both
 * transmit (TCOR/TBPR) and receive (RCOR/RBPR). Both directions
 * must match for full-duplex communication.
 * ==================================================================== */

typedef struct _CY_BAUD_CONTEXT {
    PCY_PDO_EXT         Extension;
    const CY_BAUD_ENTRY *Entry;     /* Baud table entry to apply    */
    NTSTATUS            Status;
} CY_BAUD_CONTEXT;

/* ====================================================================
 * CySyncSetBaud — KeSynchronizeExecution callback: set baud rate
 * ====================================================================
 * Runs at DIRQL, synchronized with the ISR. Writes the clock option
 * and baud period registers for both TX (TCOR/TBPR) and RX
 * (RCOR/RBPR). Both directions must match for full-duplex.
 *
 * Context: CY_BAUD_CONTEXT with the baud table entry to apply.
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

static BOOLEAN NTAPI CySyncSetBaud(PVOID Context)
{
    CY_BAUD_CONTEXT *ctx = (CY_BAUD_CONTEXT *)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    const CY_BAUD_ENTRY *entry = ctx->Entry;

    /* Select our channel — safe at DIRQL, ISR is blocked */
    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Set transmit clock option and baud period.
     * TCOR controls the clock divider for TX.
     * TBPR is the baud period counter for TX. */
    CyWriteReg(ext->ChipBase, CyTCOR, entry->ClockOpt);
    CyWriteReg(ext->ChipBase, CyTBPR, entry->BaudPeriod);

    /* Set receive clock option and baud period.
     * Must match TX for proper full-duplex operation.
     * Some protocols use split baud (different TX/RX rates)
     * but we don't support that — it's extremely rare. */
    CyWriteReg(ext->ChipBase, CyRCOR, entry->ClockOpt);
    CyWriteReg(ext->ChipBase, CyRBPR, entry->BaudPeriod);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * KeSynchronizeExecution callback: Set line control (COR1)
 * ====================================================================
 * Writes COR1 with the new data width, parity, and stop bit
 * settings. Then issues a COR_CHANGE command via CCR so the
 * chip applies the new settings.
 * ==================================================================== */

typedef struct _CY_LINE_CONTEXT {
    PCY_PDO_EXT         Extension;
    UCHAR               Cor1Value;  /* Pre-computed COR1 value      */
    NTSTATUS            Status;
} CY_LINE_CONTEXT;

/* ====================================================================
 * CySyncSetLine — KeSynchronizeExecution callback: set line control
 * ====================================================================
 * Writes COR1 with new data width, parity, and stop bit settings,
 * then issues COR_CHANGE via CCR so the chip applies them.
 *
 * Context: CY_LINE_CONTEXT with pre-computed COR1 value.
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

static BOOLEAN NTAPI CySyncSetLine(PVOID Context)
{
    CY_LINE_CONTEXT *ctx = (CY_LINE_CONTEXT *)Context;
    PCY_PDO_EXT ext = ctx->Extension;

    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Write the new COR1 value */
    CyWriteReg(ext->ChipBase, CyCOR1, ctx->Cor1Value);

    /* Tell the chip to re-read COR1. Without this CCR command,
     * the chip ignores the COR1 change. */
    CyWriteReg(ext->ChipBase, CyCCR, CyCOR_CHANGE | CyCOR1ch);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * KeSynchronizeExecution callback: Set/Clear DTR or RTS
 * ====================================================================
 * Writes to MSVR1 to change modem output signals.
 * ==================================================================== */

typedef struct _CY_MODEM_CONTEXT {
    PCY_PDO_EXT Extension;
    UCHAR       MsvrValue;      /* New MSVR1 value to write         */
    NTSTATUS    Status;
} CY_MODEM_CONTEXT;

/* ====================================================================
 * CySyncSetModem — KeSynchronizeExecution callback: set/clear DTR/RTS
 * ====================================================================
 * Writes to MSVR1 and MSVR2 to change modem output signals.
 * Both registers are written for compatibility across CD1400 revisions.
 *
 * Context: CY_MODEM_CONTEXT with the new MSVR value.
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

static BOOLEAN NTAPI CySyncSetModem(PVOID Context)
{
    CY_MODEM_CONTEXT *ctx = (CY_MODEM_CONTEXT *)Context;
    PCY_PDO_EXT ext = ctx->Extension;

    CySelectChannel(ext->ChipBase, ext->Channel);
    CyWriteReg(ext->ChipBase, CyMSVR1, ctx->MsvrValue);
    CyWriteReg(ext->ChipBase, CyMSVR2, ctx->MsvrValue);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * KeSynchronizeExecution callback: Set/Clear BREAK
 * ==================================================================== */

typedef struct _CY_BREAK_CONTEXT {
    PCY_PDO_EXT Extension;
    BOOLEAN     BreakOn;        /* TRUE = start break, FALSE = stop */
    NTSTATUS    Status;
} CY_BREAK_CONTEXT;

/* ====================================================================
 * CySyncSetBreak — KeSynchronizeExecution callback: set/clear BREAK
 * ====================================================================
 * Sends CCR commands to start or stop a break condition on the TX line.
 * Break = TX line held low for longer than one character time.
 *
 * Context: CY_BREAK_CONTEXT with BreakOn flag.
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

static BOOLEAN NTAPI CySyncSetBreak(PVOID Context)
{
    CY_BREAK_CONTEXT *ctx = (CY_BREAK_CONTEXT *)Context;
    PCY_PDO_EXT ext = ctx->Extension;

    CySelectChannel(ext->ChipBase, ext->Channel);

    if (ctx->BreakOn) {
        /* Start sending break — hold TX line low.
         * CySEND_SPEC_1 (0x21) tells the chip to begin a break
         * condition. The line stays low until we send STOP. */
        CyWriteReg(ext->ChipBase, CyCCR, CySEND_SPEC_1);
    } else {
        /* Stop sending break — resume normal transmission.
         * CySEND_SPEC_2 (0x22) ends the break condition. */
        CyWriteReg(ext->ChipBase, CyCCR, CySEND_SPEC_2);
    }

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * Helper: Build COR1 value from SERIAL_LINE_CONTROL
 * ====================================================================
 * Translates the Win32 SERIAL_LINE_CONTROL structure (WordLength,
 * StopBits, Parity) into a CD1400 COR1 register value.
 *
 * Returns STATUS_INVALID_PARAMETER if any field is invalid.
 * ==================================================================== */

static NTSTATUS CyBuildCor1(
    PSERIAL_LINE_CONTROL lineCtl,
    PUCHAR               cor1Out)
{
    UCHAR cor1 = 0;

    /* ---- Data width (bits 1:0) ----
     * CD1400 COR1 bits 1:0: 00=5, 01=6, 10=7, 11=8 */
    switch (lineCtl->WordLength) {
    case 5: cor1 |= Cy_5_BITS; break;
    case 6: cor1 |= Cy_6_BITS; break;
    case 7: cor1 |= Cy_7_BITS; break;
    case 8: cor1 |= Cy_8_BITS; break;
    default:
        CyWarn("IOCTL: invalid word length %u\n",
                 lineCtl->WordLength);
        return STATUS_INVALID_PARAMETER;
    }

    /* ---- Stop bits (bits 3:2) ----
     * STOP_BIT_1   = 0 → COR1 bits 3:2 = 00 (1 stop bit)
     * STOP_BITS_1_5 = 1 → COR1 bits 3:2 = 01 (1.5 stop bits, 5-bit only)
     * STOP_BITS_2   = 2 → COR1 bits 3:2 = 10 (2 stop bits) */
    switch (lineCtl->StopBits) {
    case STOP_BIT_1:    cor1 |= Cy_1_STOP;   break;
    case STOP_BITS_1_5: cor1 |= Cy_1_5_STOP; break;
    case STOP_BITS_2:   cor1 |= Cy_2_STOP;   break;
    default:
        CyWarn("IOCTL: invalid stop bits %u\n",
                 lineCtl->StopBits);
        return STATUS_INVALID_PARAMETER;
    }

    /* ---- Parity (bits 7:5) ----
     * NO_PARITY    = 0 → disabled
     * ODD_PARITY   = 1 → odd
     * EVEN_PARITY  = 2 → even
     * MARK_PARITY  = 3 → mark (force 1)
     * SPACE_PARITY = 4 → space (force 0)
     *
     * cd1400.h names: CyPARITY_O (odd), CyPARITY_E (even),
     * CyPARITY_1 (mark/force 1), CyPARITY_0 (space/force 0).
     * (Audit fix: was using CyPARITY_ODD etc which don't exist) */
    switch (lineCtl->Parity) {
    case NO_PARITY:    cor1 |= CyPARITY_NONE;  break;
    case ODD_PARITY:   cor1 |= CyPARITY_O;     break;  /* 0xC0 */
    case EVEN_PARITY:  cor1 |= CyPARITY_E;     break;  /* 0x40 */
    case MARK_PARITY:  cor1 |= CyPARITY_1;     break;  /* 0xA0 force 1 */
    case SPACE_PARITY: cor1 |= CyPARITY_0;     break;  /* 0x20 force 0 */
    default:
        CyWarn("IOCTL: invalid parity %u\n",
                 lineCtl->Parity);
        return STATUS_INVALID_PARAMETER;
    }

    *cor1Out = cor1;
    return STATUS_SUCCESS;
}


/* ====================================================================
 * Helper: Map CD1400 MSVR to Win32 modem status bits
 * ====================================================================
 * The CD1400 MSVR1 register has different bit positions than the
 * Win32 MODEM_STATUS_REGISTER format. We translate.
 *
 * CD1400 MSVR1:
 *   Bit 0: RTS output state (we set this)
 *   Bit 1: DTR output state (we set this)
 *   Bit 4: CTS input
 *   Bit 5: DSR input
 *   Bit 6: RI input
 *   Bit 7: DCD input
 *
 * Win32 SERIAL_MSR (returned by GET_MODEMSTATUS):
 *   Bit 4: CTS
 *   Bit 5: DSR
 *   Bit 6: RI
 *   Bit 7: DCD
 * ==================================================================== */

static ULONG CyMsvrToModemStatus(UCHAR msvr, BOOLEAN rtsdtrInv)
{
    ULONG status = 0;

    /* Map CD1400 MSVR INPUT signal bits to Win32 SERIAL_MSR.
     * These are input signals from the remote end — not affected
     * by pin inversion (inversion only affects OUTPUT signals). */
    if (msvr & CyCTS) status |= SERIAL_MSR_CTS;
    if (msvr & CyDSR) status |= SERIAL_MSR_DSR;
    if (msvr & CyRI)  status |= SERIAL_MSR_RI;
    if (msvr & CyDCD) status |= SERIAL_MSR_DCD;

    /* DTR/RTS are OUTPUT signals we control. The Win32 API doesn't
     * report these via GET_MODEMSTATUS (it's input only). But some
     * applications check them. On boards with rtsdtr_inv, the pins
     * are swapped — CyDTR controls the physical RTS pin and vice
     * versa. We report the LOGICAL state (what we think we set),
     * not the physical pin state, so apps see consistent behavior
     * regardless of inversion. (Fix for known issue 5.2) */
    UNREFERENCED_PARAMETER(rtsdtrInv);

    return status;
}


/* ====================================================================
 * CySyncSetSpecialChars — Write XON/XOFF chars to SCHR1/SCHR2
 * ====================================================================
 * KeSynchronizeExecution callback. Writes the XON character to
 * SCHR1 and XOFF character to SCHR2 on the CD1400. These are used
 * for in-band software flow control when COR2 IXM is enabled.
 *
 * Context: CY_SYNC_CONTEXT with Data pointing to SERIAL_CHARS.
 * IRQL: DIRQL
 * ==================================================================== */

static BOOLEAN NTAPI CySyncSetSpecialChars(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    PSERIAL_CHARS chars = (PSERIAL_CHARS)ctx->Data;

    CySelectChannel(ext->ChipBase, ext->Channel);

    CyWriteReg(ext->ChipBase, CySCHR1, chars->XonChar);
    CyWriteReg(ext->ChipBase, CySCHR2, chars->XoffChar);

    CyTrace("SCHR: port %lu XON=0x%02X XOFF=0x%02X\n",
            ext->PortIndex, chars->XonChar, chars->XoffChar);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * CySyncSetCor2 — KeSynchronizeExecution callback: set COR2 flow control
 * ====================================================================
 * Writes COR2 with hardware flow control bits (CtsAE, RtsAO, IXM)
 * and issues COR_CHANGE via CCR to apply.
 *
 * Context: CY_SYNC_CONTEXT with COR2 value in Data (cast from UCHAR).
 * IRQL: DIRQL (called via KeSynchronizeExecution)
 * ==================================================================== */

static BOOLEAN NTAPI CySyncSetCor2(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    UCHAR cor2 = (UCHAR)(ULONG_PTR)ctx->Data;

    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Write the new COR2 value */
    CyWriteReg(ext->ChipBase, CyCOR2, cor2);

    /* Tell the chip to apply the COR2 change.
     * COR_CHANGE with COR2ch bit set in CCR. */
    CyWriteReg(ext->ChipBase, CyCCR, CyCOR_CHANGE | CyCOR2ch);
    KeStallExecutionProcessor(10);

    CyTrace("COR2: port %lu set to 0x%02X (CtsAE=%u RtsAO=%u IXM=%u)\n",
            ext->PortIndex, cor2,
            (cor2 & CyCtsAE) ? 1 : 0,
            (cor2 & CyRtsAO) ? 1 : 0,
            (cor2 & 0x40) ? 1 : 0);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * CyDispatchIoCtl — Main IOCTL Dispatch
 * ====================================================================
 * Routes each IOCTL code to its handler. Every IOCTL:
 *   1. Validates buffer sizes
 *   2. Logs entry with CyTrace
 *   3. Does its work
 *   4. Logs result with CyTrace
 *   5. Completes the IRP
 *
 * Buffer access:
 *   Input:  Irp->AssociatedIrp.SystemBuffer (buffered I/O)
 *   Output: Irp->AssociatedIrp.SystemBuffer (same pointer)
 *   Input size:  irpSp->Parameters.DeviceIoControl.InputBufferLength
 *   Output size: irpSp->Parameters.DeviceIoControl.OutputBufferLength
 * ==================================================================== */

NTSTATUS CyDispatchIoCtl(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT      common = CyGetCommon(DeviceObject);
    PIO_STACK_LOCATION  irpSp;
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG_PTR            info = 0;  /* Bytes returned to caller      */
    ULONG               ioctlCode;
    PVOID               sysBuf;
    ULONG               inLen;
    ULONG               outLen;

    /* FDOs don't handle serial IOCTLs */
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

        /* Verify port is open */
        if (!pdoExt->IsOpen) {
            status = STATUS_INVALID_DEVICE_STATE;
            goto IoctlDone;
        }

        irpSp     = IoGetCurrentIrpStackLocation(Irp);
        ioctlCode = irpSp->Parameters.DeviceIoControl.IoControlCode;
        sysBuf    = Irp->AssociatedIrp.SystemBuffer;
        inLen     = irpSp->Parameters.DeviceIoControl.InputBufferLength;
        outLen    = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

        switch (ioctlCode) {

        /* ============================================================
         * IOCTL_SERIAL_SET_BAUD_RATE
         * ============================================================
         * Input: SERIAL_BAUD_RATE (ULONG BaudRate)
         * Output: none
         *
         * Sets the baud rate for both TX and RX. The baud rate must
         * match an entry in the baud table for this chip revision.
         * ============================================================ */
        case IOCTL_SERIAL_SET_BAUD_RATE:
        {
            PSERIAL_BAUD_RATE baudReq;
            const CY_BAUD_ENTRY *entry;
            CY_BAUD_CONTEXT baudCtx;

            if (inLen < sizeof(SERIAL_BAUD_RATE)) {
            CyTrace("SET_BAUD: buffer too small (%lu)\n", inLen);
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            baudReq = (PSERIAL_BAUD_RATE)sysBuf;

            CyTrace("SET_BAUD: port %lu → %lu baud\n",
                     pdoExt->PortIndex, baudReq->BaudRate);

            /* Look up the baud rate in our table */
            entry = CyLookupBaud(pdoExt->Is60MHz, baudReq->BaudRate);
            if (!entry) {
                CyWarn("SET_BAUD: unsupported rate %lu\n",
                         baudReq->BaudRate);
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            /* Apply via KeSynchronizeExecution */
            baudCtx.Extension = pdoExt;
            baudCtx.Entry = entry;

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncSetBaud, &baudCtx);
            } else {
                CySyncSetBaud(&baudCtx);
            }

            pdoExt->BaudRate = baudReq->BaudRate;
            status = baudCtx.Status;

            CyTrace("SET_BAUD: port %lu now %lu baud (CO=0x%02X BPR=0x%02X)\n",
                     pdoExt->PortIndex, baudReq->BaudRate,
                     entry->ClockOpt, entry->BaudPeriod);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_GET_BAUD_RATE
         * ============================================================
         * Input: none
         * Output: SERIAL_BAUD_RATE (ULONG BaudRate)
         * ============================================================ */
        case IOCTL_SERIAL_GET_BAUD_RATE:
        {
            PSERIAL_BAUD_RATE baudOut;

            if (outLen < sizeof(SERIAL_BAUD_RATE)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            baudOut = (PSERIAL_BAUD_RATE)sysBuf;
            baudOut->BaudRate = pdoExt->BaudRate;
            info = sizeof(SERIAL_BAUD_RATE);

            CyTrace("GET_BAUD: port %lu = %lu\n",
                     pdoExt->PortIndex, pdoExt->BaudRate);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_LINE_CONTROL
         * ============================================================
         * Input: SERIAL_LINE_CONTROL (WordLength, StopBits, Parity)
         * Output: none
         * ============================================================ */
        case IOCTL_SERIAL_SET_LINE_CONTROL:
        {
            PSERIAL_LINE_CONTROL lineReq;
            UCHAR cor1;
            CY_LINE_CONTEXT lineCtx;

            if (inLen < sizeof(SERIAL_LINE_CONTROL)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            lineReq = (PSERIAL_LINE_CONTROL)sysBuf;

            TraceEvents(CYPORT_LEVEL_INFO, CYPORT_IOCTL,
                     "SET_LINE port %lu", pdoExt->PortIndex);
            CyTrace("SET_LINE: port %lu → %u%c%s\n",
                     pdoExt->PortIndex,
                     lineReq->WordLength,
                     "NOEMS"[lineReq->Parity],
                     lineReq->StopBits == STOP_BIT_1 ? "1" :
                     lineReq->StopBits == STOP_BITS_1_5 ? "1.5" : "2");

            /* Build COR1 value — validates all fields */
            status = CyBuildCor1(lineReq, &cor1);
            if (!NT_SUCCESS(status))
                break;

            /* Apply via KeSynchronizeExecution */
            lineCtx.Extension = pdoExt;
            lineCtx.Cor1Value = cor1;

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncSetLine, &lineCtx);
            } else {
                CySyncSetLine(&lineCtx);
            }

            /* Save in extension for GET_LINE_CONTROL */
            pdoExt->LineControl = *lineReq;
            status = lineCtx.Status;
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_GET_LINE_CONTROL
         * ============================================================ */
        case IOCTL_SERIAL_GET_LINE_CONTROL:
        {
            if (outLen < sizeof(SERIAL_LINE_CONTROL)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PSERIAL_LINE_CONTROL)sysBuf = pdoExt->LineControl;
            info = sizeof(SERIAL_LINE_CONTROL);

            CyTrace("GET_LINE: port %lu = %u%c%s\n",
                     pdoExt->PortIndex,
                     pdoExt->LineControl.WordLength,
                     "NOEMS"[pdoExt->LineControl.Parity],
                     pdoExt->LineControl.StopBits == STOP_BIT_1 ? "1" : "2");
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_DTR / CLR_DTR / SET_RTS / CLR_RTS
         * ============================================================
         * These four IOCTLs control the modem output signals.
         * No input/output buffers — just the IOCTL code itself
         * tells us what to do.
         * ============================================================ */
        case IOCTL_SERIAL_SET_DTR:
        case IOCTL_SERIAL_CLR_DTR:
        case IOCTL_SERIAL_SET_RTS:
        case IOCTL_SERIAL_CLR_RTS:
        {
            CY_MODEM_CONTEXT modemCtx;
            UCHAR msvrValue;

            /* Build the new MSVR value from current state + change */
            msvrValue = 0;
            if (ioctlCode == IOCTL_SERIAL_SET_DTR) {
                pdoExt->ModemControl |= SERIAL_DTR_STATE;
                CyTrace("SET_DTR: port %lu\n", pdoExt->PortIndex);
            } else if (ioctlCode == IOCTL_SERIAL_CLR_DTR) {
                pdoExt->ModemControl &= ~SERIAL_DTR_STATE;
                CyTrace("CLR_DTR: port %lu\n", pdoExt->PortIndex);
            } else if (ioctlCode == IOCTL_SERIAL_SET_RTS) {
                pdoExt->ModemControl |= SERIAL_RTS_STATE;
                CyTrace("SET_RTS: port %lu\n", pdoExt->PortIndex);
            } else {
                pdoExt->ModemControl &= ~SERIAL_RTS_STATE;
                CyTrace("CLR_RTS: port %lu\n", pdoExt->PortIndex);
            }

            /* Map our modem control flags to CD1400 MSVR bits.
             * CyDTR = bit 1, CyRTS = bit 0 in MSVR */
            if (pdoExt->ModemControl & SERIAL_DTR_STATE) msvrValue |= CyDTR;
            if (pdoExt->ModemControl & SERIAL_RTS_STATE) msvrValue |= CyRTS;

            modemCtx.Extension = pdoExt;
            modemCtx.MsvrValue = msvrValue;

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncSetModem, &modemCtx);
            } else {
                CySyncSetModem(&modemCtx);
            }

            status = modemCtx.Status;

            CyTrace("MODEM: port %lu DTR=%u RTS=%u MSVR=0x%02X\n",
                     pdoExt->PortIndex,
                     (pdoExt->ModemControl & SERIAL_DTR_STATE) ? 1 : 0,
                     (pdoExt->ModemControl & SERIAL_RTS_STATE) ? 1 : 0,
                     msvrValue);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_GET_MODEMSTATUS
         * ============================================================
         * Output: ULONG — modem input signal states
         *
         * Returns the current state of CTS, DSR, RI, DCD.
         * Reads the shadow MSVR (updated by the ISR on changes).
         * ============================================================ */
        case IOCTL_SERIAL_GET_MODEMSTATUS:
        {
            UCHAR msvr;
            CY_SYNC_CONTEXT syncCtx;

            if (outLen < sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            /* Read MSVR via KeSynchronizeExecution.
             * This ensures we get a consistent snapshot even if
             * a modem signal changes during the read. */
            syncCtx.Extension = pdoExt;
            syncCtx.Data = &msvr;

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncReadMSVR, &syncCtx);
            } else {
                CySyncReadMSVR(&syncCtx);
            }

            *(PULONG)sysBuf = CyMsvrToModemStatus(msvr, pdoExt->RtsDtrInv);
            info = sizeof(ULONG);

            CyTrace("GET_MODEM: port %lu MSVR=0x%02X → status=0x%08lX "
                     "(CTS=%u DSR=%u RI=%u DCD=%u)\n",
                     pdoExt->PortIndex, msvr, *(PULONG)sysBuf,
                     (msvr & 0x10) ? 1 : 0, (msvr & 0x20) ? 1 : 0,
                     (msvr & 0x40) ? 1 : 0, (msvr & 0x80) ? 1 : 0);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_TIMEOUTS / GET_TIMEOUTS
         * ============================================================ */
        case IOCTL_SERIAL_SET_TIMEOUTS:
        {
            if (inLen < sizeof(SERIAL_TIMEOUTS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            pdoExt->Timeouts = *(PSERIAL_TIMEOUTS)sysBuf;

            CyTrace("SET_TIMEOUTS: port %lu RI=%lu RTM=%lu RTC=%lu WTM=%lu WTC=%lu\n",
                     pdoExt->PortIndex,
                     pdoExt->Timeouts.ReadIntervalTimeout,
                     pdoExt->Timeouts.ReadTotalTimeoutMultiplier,
                     pdoExt->Timeouts.ReadTotalTimeoutConstant,
                     pdoExt->Timeouts.WriteTotalTimeoutMultiplier,
                     pdoExt->Timeouts.WriteTotalTimeoutConstant);
            break;
        }

        case IOCTL_SERIAL_GET_TIMEOUTS:
        {
            if (outLen < sizeof(SERIAL_TIMEOUTS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PSERIAL_TIMEOUTS)sysBuf = pdoExt->Timeouts;
            info = sizeof(SERIAL_TIMEOUTS);

            CyTrace("GET_TIMEOUTS: port %lu\n", pdoExt->PortIndex);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_BREAK_ON / SET_BREAK_OFF
         * ============================================================ */
        case IOCTL_SERIAL_SET_BREAK_ON:
        case IOCTL_SERIAL_SET_BREAK_OFF:
        {
            CY_BREAK_CONTEXT breakCtx;
            breakCtx.Extension = pdoExt;
            breakCtx.BreakOn = (ioctlCode == IOCTL_SERIAL_SET_BREAK_ON);

            CyTrace("BREAK_%s: port %lu\n",
                     breakCtx.BreakOn ? "ON" : "OFF", pdoExt->PortIndex);

            if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                KeSynchronizeExecution(pdoExt->Interrupt,
                                       CySyncSetBreak, &breakCtx);
            } else {
                CySyncSetBreak(&breakCtx);
            }

            status = breakCtx.Status;
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_GET_COMMSTATUS
         * ============================================================
         * Output: SERIAL_STATUS — buffer counts, errors, hold reasons
         * ============================================================ */
        case IOCTL_SERIAL_GET_COMMSTATUS:
        {
            PSERIAL_STATUS serialStatus;

            if (outLen < sizeof(SERIAL_STATUS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            serialStatus = (PSERIAL_STATUS)sysBuf;
            RtlZeroMemory(serialStatus, sizeof(SERIAL_STATUS));

            /* Report ring buffer counts.
             * These are approximate — the ISR may be modifying them
             * right now. But GET_COMMSTATUS is advisory anyway. */
            serialStatus->AmountInInQueue  = pdoExt->RxCount;
            serialStatus->AmountInOutQueue = pdoExt->TxCount;
            serialStatus->EofReceived      = FALSE;
            serialStatus->WaitForImmediate = FALSE;

            info = sizeof(SERIAL_STATUS);

            CyTrace("GET_COMMSTATUS: port %lu RX=%lu TX=%lu\n",
                     pdoExt->PortIndex, pdoExt->RxCount, pdoExt->TxCount);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_PURGE
         * ============================================================
         * Input: ULONG — purge mask (SERIAL_PURGE_TXABORT, RXABORT,
         *                            TXCLEAR, RXCLEAR)
         * ============================================================ */
        case IOCTL_SERIAL_PURGE:
        {
            ULONG purgeMask;
            CY_SYNC_CONTEXT syncCtx;

            if (inLen < sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            purgeMask = *(PULONG)sysBuf;

            CyTrace("PURGE: port %lu mask=0x%08lX "
                     "(TxAbort=%u RxAbort=%u TxClear=%u RxClear=%u)\n",
                     pdoExt->PortIndex, purgeMask,
                     (purgeMask & SERIAL_PURGE_TXABORT) ? 1 : 0,
                     (purgeMask & SERIAL_PURGE_RXABORT) ? 1 : 0,
                     (purgeMask & SERIAL_PURGE_TXCLEAR) ? 1 : 0,
                     (purgeMask & SERIAL_PURGE_RXCLEAR) ? 1 : 0);

            /* Cancel pending IRPs if ABORT flags are set */
            if (purgeMask & SERIAL_PURGE_TXABORT) {
                PIRP pendingIrp;
                while ((pendingIrp = IoCsqRemoveNextIrp(
                            &pdoExt->WriteQueue, NULL)) != NULL) {
                    pendingIrp->IoStatus.Status = STATUS_CANCELLED;
                    pendingIrp->IoStatus.Information = 0;
                    IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
                }
            }

            if (purgeMask & SERIAL_PURGE_RXABORT) {
                PIRP pendingIrp;
                while ((pendingIrp = IoCsqRemoveNextIrp(
                            &pdoExt->ReadQueue, NULL)) != NULL) {
                    pendingIrp->IoStatus.Status = STATUS_CANCELLED;
                    pendingIrp->IoStatus.Information = 0;
                    IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
                }
            }

            /* Clear ring buffers if CLEAR flags are set.
             * Uses KeSynchronizeExecution to reset counts safely.
             * (Audit bug #9 fix — PURGE without spinlock) */
            if (purgeMask & (SERIAL_PURGE_TXCLEAR | SERIAL_PURGE_RXCLEAR)) {
                syncCtx.Extension = pdoExt;
                syncCtx.Data = &purgeMask;

                if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                    KeSynchronizeExecution(pdoExt->Interrupt,
                                           CySyncPurgeBuffers, &syncCtx);
                } else {
                    CySyncPurgeBuffers(&syncCtx);
                }
            }
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_WAIT_MASK / GET_WAIT_MASK
         * ============================================================
         * Controls which events WaitCommEvent waits for.
         * ============================================================ */
        case IOCTL_SERIAL_SET_WAIT_MASK:
        {
            KIRQL oldIrql;

            if (inLen < sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&pdoExt->EventLock, &oldIrql);

            /* If a WaitCommEvent IRP is pending and the mask changes,
             * complete the old wait with the current event history.
             * The app must call WaitCommEvent again with the new mask. */
            if (pdoExt->WaitIrp) {
                PIRP waitIrp = pdoExt->WaitIrp;
                pdoExt->WaitIrp = NULL;

                *(PULONG)waitIrp->AssociatedIrp.SystemBuffer = 0;
                waitIrp->IoStatus.Status = STATUS_SUCCESS;
                waitIrp->IoStatus.Information = sizeof(ULONG);
                IoCompleteRequest(waitIrp, IO_NO_INCREMENT);
            }

            pdoExt->WaitMask = *(PULONG)sysBuf;
            pdoExt->EventHistory = 0;   /* Reset history on mask change */

            KeReleaseSpinLock(&pdoExt->EventLock, oldIrql);

            CyTrace("SET_WAIT_MASK: port %lu mask=0x%08lX\n",
                     pdoExt->PortIndex, pdoExt->WaitMask);
            break;
        }

        case IOCTL_SERIAL_GET_WAIT_MASK:
        {
            if (outLen < sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PULONG)sysBuf = pdoExt->WaitMask;
            info = sizeof(ULONG);

            CyTrace("GET_WAIT_MASK: port %lu = 0x%08lX\n",
                     pdoExt->PortIndex, pdoExt->WaitMask);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_WAIT_ON_MASK
         * ============================================================
         * Blocks until one of the events in WaitMask fires.
         * Output: ULONG — bitmask of events that occurred.
         *
         * This is the hardest IOCTL to implement correctly.
         * Only one WaitCommEvent can be pending at a time.
         * ============================================================ */
        case IOCTL_SERIAL_WAIT_ON_MASK:
        {
            KIRQL oldIrql;

            if (outLen < sizeof(ULONG)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            KeAcquireSpinLock(&pdoExt->EventLock, &oldIrql);

            /* Only one wait can be pending at a time */
            if (pdoExt->WaitIrp) {
                KeReleaseSpinLock(&pdoExt->EventLock, oldIrql);
                CyWarn("WAIT_ON_MASK: port %lu REJECTED — already waiting\n",
                         pdoExt->PortIndex);
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            /* Check if events already occurred before we block */
            if (pdoExt->EventHistory & pdoExt->WaitMask) {
                ULONG events = pdoExt->EventHistory & pdoExt->WaitMask;
                pdoExt->EventHistory = 0;   /* Consume events        */
                KeReleaseSpinLock(&pdoExt->EventLock, oldIrql);

                *(PULONG)sysBuf = events;
                info = sizeof(ULONG);

                CyTrace("WAIT_ON_MASK: port %lu IMMEDIATE events=0x%08lX\n",
                         pdoExt->PortIndex, events);
                break;
            }

            /* No events yet — pend the IRP.
             * The modem DPC will complete it when events fire. */
            pdoExt->WaitIrp = Irp;
            KeReleaseSpinLock(&pdoExt->EventLock, oldIrql);

            IoMarkIrpPending(Irp);

            CyTrace("WAIT_ON_MASK: port %lu PENDING mask=0x%08lX\n",
                     pdoExt->PortIndex, pdoExt->WaitMask);

            /* Release remove lock — DPC will complete the IRP later */
            IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
            return STATUS_PENDING;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_HANDFLOW / GET_HANDFLOW
         * ============================================================ */
        case IOCTL_SERIAL_SET_HANDFLOW:
        {
            if (inLen < sizeof(SERIAL_HANDFLOW)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            pdoExt->HandFlow = *(PSERIAL_HANDFLOW)sysBuf;

            /* ---- Apply COR2 hardware flow control ----
             *
             * The CD1400 supports two hardware flow control modes:
             *
             * CtsAE (COR2 bit 1) — CTS Auto-Enable:
             *   When set, the transmitter pauses automatically when
             *   CTS is deasserted by the remote end. TX resumes when
             *   CTS is reasserted. No software intervention needed.
             *   This is the hardware equivalent of "pause when CTS low".
             *
             * RtsAO (COR2 bit 5) — RTS Auto-Output:
             *   When set, the chip deasserts RTS when the RX FIFO
             *   fills above the threshold (COR3), and reasserts RTS
             *   when the FIFO drains below the threshold. This tells
             *   the remote end to stop sending when we're full.
             *
             * We map Windows flow control flags to these bits:
             *   SERIAL_CTS_HANDSHAKE → CtsAE (pause TX on CTS low)
             *   SERIAL_RTS_HANDSHAKE → RtsAO (manage RTS from FIFO)
             *
             * (Fix for TODO #1 — COR2 hardware flow control) */
            {
                CY_SYNC_CONTEXT syncCtx;
                UCHAR cor2;

                syncCtx.Extension = pdoExt;

                /* Build COR2 value from HandFlow flags */
                cor2 = 0;

                /* CTS handshaking: pause TX when CTS is low */
                if (pdoExt->HandFlow.ControlHandShake &
                    SERIAL_CTS_HANDSHAKE) {
                    cor2 |= CyCtsAE;       /* COR2 bit 1 */
                }

                /* RTS handshaking: auto-manage RTS from RX FIFO level */
                if ((pdoExt->HandFlow.FlowReplace &
                     SERIAL_RTS_MASK) == SERIAL_RTS_HANDSHAKE) {
                    cor2 |= CyRtsAO;       /* COR2 bit 5 */
                }

                /* Software flow control: XON/XOFF */
                if (pdoExt->HandFlow.FlowReplace &
                    SERIAL_AUTO_TRANSMIT) {
                    /* Enable in-band XON/XOFF processing.
                     * The CD1400 can handle XON/XOFF in hardware via
                     * SCHR1 (XON char) and SCHR2 (XOFF char).
                     * COR2 bit 6 enables this. */
                    cor2 |= 0x40;           /* IXM: In-band Xon/Xoff Mode */
                }

                /* Store the new COR2 and apply via KeSynchronizeExecution */
                syncCtx.Data = (PVOID)(ULONG_PTR)cor2;

                if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                    KeSynchronizeExecution(pdoExt->Interrupt,
                                           CySyncSetCor2, &syncCtx);
                } else {
                    CySyncSetCor2(&syncCtx);
                }
            }

            CyTrace("SET_HANDFLOW: port %lu ctrl=0x%08lX flow=0x%08lX "
                     "XonLim=%lu XoffLim=%lu\n",
                     pdoExt->PortIndex,
                     pdoExt->HandFlow.ControlHandShake,
                     pdoExt->HandFlow.FlowReplace,
                     pdoExt->HandFlow.XonLimit,
                     pdoExt->HandFlow.XoffLimit);
            break;
        }

        case IOCTL_SERIAL_GET_HANDFLOW:
        {
            if (outLen < sizeof(SERIAL_HANDFLOW)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PSERIAL_HANDFLOW)sysBuf = pdoExt->HandFlow;
            info = sizeof(SERIAL_HANDFLOW);

            CyTrace("GET_HANDFLOW: port %lu\n", pdoExt->PortIndex);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_CHARS / GET_CHARS
         * ============================================================ */
        case IOCTL_SERIAL_SET_CHARS:
        {
            if (inLen < sizeof(SERIAL_CHARS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            pdoExt->SpecialChars = *(PSERIAL_CHARS)sysBuf;

            /* Write XON (SCHR1) and XOFF (SCHR2) chars to hardware.
             * The CD1400 uses these for in-band software flow control
             * when IXM mode is enabled in COR2. Without writing them,
             * the chip uses whatever power-on defaults are in the
             * registers — usually 0x00 — which won't match what the
             * application expects (typically 0x11/0x13 = DC1/DC3).
             * (sysop/0 missing feature fix — XON/XOFF) */
            {
                CY_SYNC_CONTEXT syncCtx;
                syncCtx.Extension = pdoExt;
                syncCtx.Data = (PVOID)&pdoExt->SpecialChars;

                if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                    KeSynchronizeExecution(pdoExt->Interrupt,
                                           CySyncSetSpecialChars, &syncCtx);
                } else {
                    CySyncSetSpecialChars(&syncCtx);
                }
            }

            CyTrace("SET_CHARS: port %lu XON=0x%02X XOFF=0x%02X\n",
                     pdoExt->PortIndex,
                     pdoExt->SpecialChars.XonChar,
                     pdoExt->SpecialChars.XoffChar);
            break;
        }

        case IOCTL_SERIAL_GET_CHARS:
        {
            if (outLen < sizeof(SERIAL_CHARS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PSERIAL_CHARS)sysBuf = pdoExt->SpecialChars;
            info = sizeof(SERIAL_CHARS);

            CyTrace("GET_CHARS: port %lu\n", pdoExt->PortIndex);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_GET_PROPERTIES
         * ============================================================
         * Output: SERIAL_COMMPROP — port capabilities
         *
         * HyperTerminal requires this IOCTL. Without it,
         * the port configuration dialog won't open.
         * ============================================================ */
        case IOCTL_SERIAL_GET_PROPERTIES:
        {
            PSERIAL_COMMPROP props;

            if (outLen < sizeof(SERIAL_COMMPROP)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            props = (PSERIAL_COMMPROP)sysBuf;
            RtlZeroMemory(props, sizeof(SERIAL_COMMPROP));

            props->PacketLength    = sizeof(SERIAL_COMMPROP);
            props->PacketVersion   = 2;
            props->ServiceMask     = SERIAL_SP_SERIALCOMM;
            props->MaxTxQueue      = CY_RING_BUF_SIZE;
            props->MaxRxQueue      = CY_RING_BUF_SIZE;

            /* Maximum baud rate depends on chip revision */
            props->MaxBaud = pdoExt->Is60MHz ?
                SERIAL_BAUD_USER : SERIAL_BAUD_115200;

            /* Supported baud rates */
            props->SettableBaud =
                SERIAL_BAUD_075 | SERIAL_BAUD_110 | SERIAL_BAUD_150 |
                SERIAL_BAUD_300 | SERIAL_BAUD_600 | SERIAL_BAUD_1200 |
                SERIAL_BAUD_1800 | SERIAL_BAUD_2400 | SERIAL_BAUD_4800 |
                SERIAL_BAUD_9600 | SERIAL_BAUD_19200 | SERIAL_BAUD_38400 |
                SERIAL_BAUD_57600 | SERIAL_BAUD_115200 | SERIAL_BAUD_USER;

            props->ProvSubType     = SERIAL_SP_RS232;

            /* Capabilities */
            props->ProvCapabilities =
                SERIAL_PCF_DTRDSR | SERIAL_PCF_RTSCTS |
                SERIAL_PCF_CD | SERIAL_PCF_PARITY_CHECK |
                SERIAL_PCF_XONXOFF | SERIAL_PCF_TOTALTIMEOUTS |
                SERIAL_PCF_INTTIMEOUTS;

            /* Settable parameters */
            props->SettableParams =
                SERIAL_SP_PARITY | SERIAL_SP_BAUD |
                SERIAL_SP_DATABITS | SERIAL_SP_STOPBITS |
                SERIAL_SP_PARITY_CHECK | SERIAL_SP_HANDSHAKING;

            props->SettableData =
                SERIAL_DATABITS_5 | SERIAL_DATABITS_6 |
                SERIAL_DATABITS_7 | SERIAL_DATABITS_8;

            props->SettableStopParity =
                SERIAL_STOPBITS_10 | SERIAL_STOPBITS_15 |
                SERIAL_STOPBITS_20 |
                SERIAL_PARITY_NONE | SERIAL_PARITY_ODD |
                SERIAL_PARITY_EVEN | SERIAL_PARITY_MARK |
                SERIAL_PARITY_SPACE;

            /* Current queue sizes */
            props->CurrentTxQueue = CY_RING_BUF_SIZE;
            props->CurrentRxQueue = CY_RING_BUF_SIZE;

            info = sizeof(SERIAL_COMMPROP);

            CyTrace("GET_PROPERTIES: port %lu maxBaud=%lu FIFO=%u\n",
                     pdoExt->PortIndex,
                     pdoExt->Is60MHz ? 230400UL : 115200UL,
                     CY_MAX_CHAR_FIFO);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_GET_STATS / CLR_STATS
         * ============================================================ */
        case IOCTL_SERIAL_GET_STATS:
        {
            if (outLen < sizeof(SERIALPERF_STATS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PSERIALPERF_STATS)sysBuf = pdoExt->PerfStats;
            info = sizeof(SERIALPERF_STATS);

            CyTrace("GET_STATS: port %lu RX=%lu TX=%lu "
                     "parity=%lu frame=%lu overrun=%lu buf_overrun=%lu\n",
                     pdoExt->PortIndex,
                     pdoExt->PerfStats.ReceivedCount,
                     pdoExt->PerfStats.TransmittedCount,
                     pdoExt->PerfStats.ParityErrorCount,
                     pdoExt->PerfStats.FrameErrorCount,
                     pdoExt->PerfStats.SerialOverrunErrorCount,
                     pdoExt->PerfStats.BufferOverrunErrorCount);
            break;
        }

        case IOCTL_SERIAL_CLEAR_STATS:
        {
            RtlZeroMemory(&pdoExt->PerfStats, sizeof(SERIALPERF_STATS));

            CyInfo("CLEAR_STATS: port %lu\n", pdoExt->PortIndex);
            break;
        }


        /* ============================================================
         * IOCTL_SERIAL_SET_QUEUE_SIZE
         * ============================================================
         * Input: SERIAL_QUEUE_SIZE — InSize, OutSize
         *
         * We use fixed-size ring buffers so we accept but ignore this.
         * Some apps call it during init; failing would cause them to
         * abort. Succeeding silently is the standard practice for
         * drivers with fixed-size buffers.
         * ============================================================ */
        case IOCTL_SERIAL_SET_QUEUE_SIZE:
        {
            CyTrace("SET_QUEUE_SIZE: port %lu (accepted, fixed at %u)\n",
                     pdoExt->PortIndex, CY_RING_BUF_SIZE);
            break;
        }


        /* ============================================================
         * Unknown IOCTL
         * ============================================================ */
        default:
            CyTrace("UNKNOWN IOCTL: port %lu code=0x%08lX\n",
                     pdoExt->PortIndex, ioctlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        } /* end switch */

    IoctlDone:
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = info;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
        return status;
    }
}
