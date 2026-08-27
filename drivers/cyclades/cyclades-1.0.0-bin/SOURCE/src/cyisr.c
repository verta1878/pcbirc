/* ====================================================================
 * cyisr.c — Interrupt Service Routine and DPC Bottom Halves
 * ====================================================================
 * This file is the heart of the driver. It contains:
 *
 *   1. CyInterruptService — the ISR that fires when the CD1400 chip
 *      raises an interrupt. Runs at DIRQL (Device IRQL), which is
 *      above DISPATCH_LEVEL. No paged memory, no blocking, no pool
 *      allocation allowed. Must be FAST.
 *
 *   2. DPC routines (CyReadDpcRoutine, CyWriteDpcRoutine,
 *      CyModemDpcRoutine) — "bottom halves" that run at
 *      DISPATCH_LEVEL after the ISR queues them. These complete
 *      pending IRPs, which can't be done at DIRQL.
 *
 *   3. KeSynchronizeExecution wrappers — allow dispatch-level code
 *      to safely access CD1400 registers by running a callback
 *      synchronized with the ISR at DIRQL. This prevents the ISR
 *      from changing the Channel Access Register (CAR) while
 *      dispatch code is mid-operation.
 *
 *   4. Interrupt connect/disconnect helpers called from PDO
 *      START_DEVICE and REMOVE_DEVICE.
 *
 * The CD1400 interrupt model:
 * ==========================
 * The CD1400 does NOT use the 8250 model (read IIR, check bits).
 * Instead it uses a SERVICE REQUEST model with three priority levels:
 *
 *   Priority 1 (highest): Receive data or receive exception
 *   Priority 2:           Transmitter ready for more data
 *   Priority 3 (lowest):  Modem signal change (CTS/DSR/DCD/RI)
 *
 * The ISR reads the Service Vector Request Register (SVRR) to see
 * WHICH type of service is pending, then reads the corresponding
 * interrupt vector register (RIVR/TIVR/MIVR) to determine WHICH
 * channel needs service.
 *
 * CRITICAL: The chip AUTO-SELECTS the channel when a service request
 * fires. The ISR does NOT need to write CAR before reading data
 * registers. The hardware loads CAR automatically with the requesting
 * channel number. This is different from the polling model used in
 * the IOCTL handlers, where we must manually write CAR.
 *
 * CRITICAL: After every service context (RX, TX, or modem), the ISR
 * MUST write to the End of Service Request Register (EOSRR). This
 * pops the chip's internal context stack. If EOSRR is not written,
 * the context stack fills and the chip stops generating interrupts
 * PERMANENTLY. Only a chip reset recovers from this. See G24
 * research in CRITICAL_GAPS_RESEARCH.md.
 *
 * Sources:
 *   - OpenBSD sys/dev/sbus/magma.c (working CD1400 ISR)
 *   - Linux drivers/char/cyclades.c (GPL reference)
 *   - Cirrus Logic CD1864 patent US5566352 (EOSRR documentation)
 *   - Original cyyport.sys import analysis (KeSynchronizeExecution,
 *     IoConnectInterrupt, KeInsertQueueDpc confirmed)
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cyisr.tmh"
#endif

/* ISR-level debug macro — gated by CY_DEBUG_REGS.
 * Must be defined before first use (OW requires this). */
#if defined(CY_DEBUG) && CY_DEBUG && defined(CY_DEBUG_REGS) && CY_DEBUG_REGS
#define CY_ISR_DBG(fmt, ...) DbgPrint("CYPORT ISR: " fmt, ##__VA_ARGS__)
#else
#define CY_ISR_DBG(fmt, ...) ((void)0)
#endif


/* ====================================================================
 * Constants
 * ==================================================================== */

/* Maximum iterations for the ISR service loop.
 * Prevents infinite looping if hardware is stuck or misconfigured.
 * (hexadecimal audit W-02 from SIO V1 project) */
#define CY_ISR_MAX_ITERATIONS   256

/* Maximum bytes to read from RX FIFO per service.
 * The CD1400 has a 12-byte FIFO, but RDCR can report up to 12.
 * We cap at 12 as a sanity check — if RDCR says more, hardware
 * is broken and we stop to avoid reading garbage. */
#define CY_ISR_MAX_RX_BYTES     12

/* ====================================================================
 * Ring Buffer Helpers (used at DIRQL inside the ISR)
 * ====================================================================
 * These operate on the ring buffer fields in CY_PDO_EXT.
 * The ISR is the ONLY writer to RxBuf and the ONLY reader of TxBuf.
 * Dispatch-level code is the ONLY reader of RxBuf and ONLY writer
 * of TxBuf. This single-producer/single-consumer model means we
 * don't need locks WITHIN the ISR — the ISR runs at DIRQL and can't
 * be preempted by dispatch code. The locks are only needed when
 * dispatch code accesses the buffers (via KeSynchronizeExecution).
 * ==================================================================== */

/* Store one byte in the RX ring buffer.
 * Called from ISR at DIRQL. Returns FALSE if buffer is full. */
static __inline BOOLEAN CyRxBufPut(PCY_PDO_EXT ext, UCHAR byte)
{
    if (ext->RxCount >= CY_RING_BUF_SIZE) {
        /* Buffer full — byte is lost. Increment overrun counter.
         * A real-world scenario: application stopped reading, ISR
         * kept receiving. The FIFO overflows into the ring buffer,
         * and now the ring buffer overflows too. */
        ext->PerfStats.BufferOverrunErrorCount++;
        return FALSE;
    }

    ext->RxBuf[ext->RxHead] = byte;
    ext->RxHead++;
    if (ext->RxHead >= CY_RING_BUF_SIZE)
        ext->RxHead = 0;       /* Wrap around to start of buffer    */
    ext->RxCount++;
    return TRUE;
}

/* Retrieve one byte from the TX ring buffer.
 * Called from ISR at DIRQL. Returns FALSE if buffer is empty. */
static __inline BOOLEAN CyTxBufGet(PCY_PDO_EXT ext, PUCHAR byte)
{
    if (ext->TxCount == 0)
        return FALSE;           /* Nothing to transmit              */

    *byte = ext->TxBuf[ext->TxTail];
    ext->TxTail++;
    if (ext->TxTail >= CY_RING_BUF_SIZE)
        ext->TxTail = 0;       /* Wrap around                      */
    ext->TxCount--;
    return TRUE;
}


/* ====================================================================
 * CyIsrServiceReceive — Handle RX service request
 * ====================================================================
 * Called when SVRR indicates a receive service is pending.
 *
 * The chip has auto-selected the requesting channel. We read RIVR
 * to determine if this is normal data or an exception (error/break).
 *
 * Normal data: Read RDCR for byte count, then read RDSR that many
 * times. Each RDSR read pops one byte from the hardware FIFO.
 *
 * Exception: Read RDSR once — it contains error status bits in the
 * upper nibble and the bad byte in the lower bits. Only one byte
 * is delivered on exception.
 *
 * After processing, write EOSRR to pop the service context.
 * ==================================================================== */

static VOID CyIsrServiceReceive(
    PCY_FDO_EXT fdoExt,
    PUCHAR      chipBase,
    PCY_PDO_EXT *portArray,     /* Array of PDO extensions by port   */
    PULONG      dpcFlags)       /* Bits indicating which DPCs to fire*/
{
    UCHAR   rivr;               /* Receive Interrupt Vector Register */
    UCHAR   channel;            /* Which channel (0-3) interrupted   */
    UCHAR   rdcr;               /* Receive Data Count Register       */
    UCHAR   data;               /* Data byte read from RDSR          */
    UCHAR   lineStatus;         /* Error bits from RDSR on exception */
    ULONG   count;
    ULONG   portIndex;
    PCY_PDO_EXT pdoExt;

    /* Read RIVR — the chip has already loaded this with the
     * requesting channel number and exception flag.
     * Bits [3:2] = channel number (0-3)
     * Bit [0]    = 1 if exception, 0 if normal receive */
    rivr = CyReadReg(chipBase, CyRIVR);
    channel = (rivr >> 2) & 0x03;

    /* Look up the PDO extension for this channel.
     * We need the chip index to compute the global port index.
     * chipBase - fdoExt->CardBase gives us the chip offset,
     * divided by CY_REG_SIZE gives the chip index. */
    portIndex = (ULONG)((ULONG_PTR)(chipBase - fdoExt->CardBase) / CY_REG_SIZE)
                * CY_PORTS_PER_CHIP + channel;

    if (portIndex >= fdoExt->NumPDOs) {
        /* Invalid port index — hardware returned garbage channel.
         * Write EOSRR anyway (MANDATORY) and bail. */
        CyWriteReg(chipBase, CyEOSRR, 0);
        return;
    }

    pdoExt = portArray[portIndex];
    if (!pdoExt || pdoExt->Common.PnPState != CyStarted) {
        /* Port not started — discard data, end service. */
        CyWriteReg(chipBase, CyEOSRR, 0);
        return;
    }

    if (rivr & 0x01) {
        /* ---- EXCEPTION (error or break) ----
         * RDSR contains error status + the problematic byte.
         * Only one byte is delivered during an exception.
         *
         * RDSR error bits:
         *   Bit 3: Break received (line held low for > 1 char time)
         *   Bit 2: Parity error
         *   Bit 1: Framing error (no valid stop bit)
         *   Bit 0: Overrun (FIFO overflow in hardware)
         *
         * We record the error in PerfStats and store the byte
         * in the ring buffer (marked with the error flag). Some
         * applications want to see the bad byte; others ignore it. */

        lineStatus = CyReadReg(chipBase, CyRDSR);

        /* SECOND read of RDSR gets the actual data byte.
         * On exception, the CD1400 delivers status on the first read
         * and the data byte on the second read. The OpenBSD magma.c
         * driver confirms this two-read pattern:
         *   line_stat = CD1400_READ_REG(cd, CD1400_RDSR);  // status
         *   *ptr++ = CD1400_READ_REG(cd, CD1400_RDSR);     // data
         * If we only read once, we'd store the status byte as data
         * and the actual received byte would be lost. (Audit B3 fix) */
        data = CyReadReg(chipBase, CyRDSR);

        /* Count each error type for IOCTL_SERIAL_GET_STATS */
        if (lineStatus & CyBREAK) {
            pdoExt->PerfStats.ParityErrorCount /* OW: no BreakErrorCount */++;
            /* Set event flag for WaitCommEvent(EV_BREAK) */
            pdoExt->EventHistory |= SERIAL_EV_BREAK;
        }
        if (lineStatus & CyPARITY) {
            pdoExt->PerfStats.ParityErrorCount++;
            pdoExt->EventHistory |= SERIAL_EV_ERR;
        }
        if (lineStatus & CyFRAME) {
            pdoExt->PerfStats.FrameErrorCount++;
            pdoExt->EventHistory |= SERIAL_EV_ERR;
        }
        if (lineStatus & CyOVERRUN) {
            pdoExt->PerfStats.SerialOverrunErrorCount++;
            pdoExt->EventHistory |= SERIAL_EV_ERR;
        }

        /* Store the byte in the ring buffer anyway — let the
         * application decide whether to use error-flagged data. */
        CyRxBufPut(pdoExt, data);

        /* Count this byte in total received (audit B4 fix).
         * Error bytes are still received bytes — the count should
         * reflect all bytes that arrived on the wire. */
        pdoExt->PerfStats.ReceivedCount++;

    } else {
        /* ---- NORMAL RECEIVE ----
         * RDCR tells us exactly how many bytes are in the FIFO.
         * We read RDSR that many times. Each read pops one byte.
         *
         * Unlike the 8250 where you poll LSR_DR until empty, the
         * CD1400 gives us the count directly. This is more
         * efficient — no wasted register reads checking status. */

        rdcr = CyReadReg(chipBase, CyRDCR);

        /* Sanity cap: FIFO is 12 bytes. If RDCR says more,
         * hardware is broken. Cap to prevent reading garbage. */
        if (rdcr > CY_ISR_MAX_RX_BYTES)
            rdcr = CY_ISR_MAX_RX_BYTES;

        for (count = 0; count < rdcr; count++) {
            data = CyReadReg(chipBase, CyRDSR);
            CyRxBufPut(pdoExt, data);
        }

        /* Track total received bytes */
        pdoExt->PerfStats.ReceivedCount += rdcr;
    }

    /* Set EV_RXCHAR event — data is now available in the ring buffer.
     * WaitCommEvent(EV_RXCHAR) will fire in the DPC. */
    pdoExt->EventHistory |= SERIAL_EV_RXCHAR;

        CY_ISR_DBG("RX: port %lu chan %u, %u bytes, RxCount=%lu\n",
               portIndex, channel,
               (rivr & 0x01) ? 1 : rdcr, pdoExt->RxCount);

    /* Flag that we need to fire the read DPC for this port.
     * The DPC will check the ring buffer and complete any pending
     * read IRPs. We set a bit per port so the DPC knows which
     * ports have new data. */
    *dpcFlags |= (1 << portIndex);

    /* ========================================
     * EOSRR — End of Service Request Register
     * ========================================
     * THIS WRITE IS MANDATORY. It pops the CD1400's internal
     * context stack. Without it, the chip jams and generates
     * NO MORE INTERRUPTS. This is the #1 cause of "dead serial
     * port" bugs in CD1400 drivers.
     *
     * Must be the LAST access to the chip for this service context.
     * Write value doesn't matter (any value clears). We write 0.
     *
     * Source: Cirrus Logic CD1864 patent US5566352, Section 2.4:
     *   "Writing to this register will generate an internal
     *    End-of-Service Signal, which 'pops' the context stack."
     * ======================================== */
    CyWriteReg(chipBase, CyEOSRR, 0);
}


/* ====================================================================
 * CyIsrServiceTransmit — Handle TX service request
 * ====================================================================
 * Called when SVRR indicates the transmitter is ready for more data.
 *
 * The chip has auto-selected the requesting channel. We read from
 * the TX ring buffer and write bytes to TDR (Transmit Data Register)
 * until either:
 *   - The ring buffer is empty (nothing more to send), or
 *   - We've written CY_MAX_CHAR_FIFO (12) bytes (FIFO is full)
 *
 * If the ring buffer is empty, we disable the TxRdy interrupt by
 * clearing the TxRdy bit in SRER. Otherwise, the chip will keep
 * interrupting us with "transmitter ready" when we have nothing
 * to send — wasting CPU cycles on empty interrupts.
 *
 * The dispatch-level write handler re-enables TxRdy when new data
 * is placed in the TX ring buffer (via KeSynchronizeExecution).
 * ==================================================================== */

static VOID CyIsrServiceTransmit(
    PCY_FDO_EXT fdoExt,
    PUCHAR      chipBase,
    PCY_PDO_EXT *portArray,
    PULONG      dpcFlags)
{
    UCHAR   tivr;               /* Transmit Interrupt Vector Reg    */
    UCHAR   channel;
    ULONG   portIndex;
    ULONG   bytesSent;
    UCHAR   byte;
    PCY_PDO_EXT pdoExt;

    /* Read TIVR for the channel that needs transmit service.
     * Bits [3:2] = channel number. */
    tivr = CyReadReg(chipBase, CyTIVR);
    channel = (tivr >> 2) & 0x03;

    portIndex = (ULONG)((ULONG_PTR)(chipBase - fdoExt->CardBase) / CY_REG_SIZE)
                * CY_PORTS_PER_CHIP + channel;

    if (portIndex >= fdoExt->NumPDOs) {
        CyWriteReg(chipBase, CyEOSRR, 0);  /* MANDATORY */
        return;
    }

    pdoExt = portArray[portIndex];
    if (!pdoExt || pdoExt->Common.PnPState != CyStarted) {
        CyWriteReg(chipBase, CyEOSRR, 0);  /* MANDATORY */
        return;
    }

    /* Load bytes from TX ring buffer into the hardware FIFO.
     * The CD1400 FIFO holds 12 bytes. We write up to 12 per
     * service request. The chip will interrupt us again when
     * the FIFO drains below the threshold. */
    bytesSent = 0;
    while (bytesSent < CY_MAX_CHAR_FIFO) {
        if (!CyTxBufGet(pdoExt, &byte))
            break;              /* Ring buffer empty — done          */

        CyWriteReg(chipBase, CyTDR, byte);
        bytesSent++;
    }

    pdoExt->PerfStats.TransmittedCount += bytesSent;

    if (pdoExt->TxCount == 0) {
        /* TX ring buffer is now empty. Disable TxRdy interrupt
         * to prevent empty "transmitter ready" interrupts.
         *
         * We read SRER, clear the TxRdy bit, and write it back.
         * The write handler will re-enable TxRdy when new data
         * arrives in the TX ring buffer.
         *
         * NOTE: We can safely read/write SRER here because the
         * chip has already auto-selected the correct channel.
         * CAR is loaded with our channel number by the service
         * request mechanism. */
        UCHAR srer_val = CyReadReg(chipBase, CySRER);
        srer_val &= ~CyTxRdy;      /* Clear TxRdy enable bit            */
        CyWriteReg(chipBase, CySRER, srer_val);

        /* Set EV_TXEMPTY event — all queued data has been sent.
         * WaitCommEvent(EV_TXEMPTY) will fire in the DPC. */
        pdoExt->EventHistory |= SERIAL_EV_TXEMPTY;
    }

    /* Flag the write DPC — there may now be space in the TX ring
     * buffer for more data from pending write IRPs. */
    *dpcFlags |= (1 << portIndex);

    /* EOSRR — MANDATORY. See CyIsrServiceReceive for explanation. */
    CyWriteReg(chipBase, CyEOSRR, 0);

        CY_ISR_DBG("TX: port %lu chan %u, %lu bytes sent, TxCount=%lu%s\n",
               portIndex, channel, bytesSent, pdoExt->TxCount,
               pdoExt->TxCount == 0 ? " (TxRdy disabled)" : "");
}


/* ====================================================================
 * CyIsrServiceModem — Handle modem signal change
 * ====================================================================
 * Called when SVRR indicates a modem signal has changed.
 *
 * Reads MISR (Modem Interrupt Status Register) for which signals
 * changed, then reads MSVR1 for the current signal levels.
 * Updates the shadow register in the PDO extension and sets event
 * flags for WaitCommEvent.
 *
 * Modem signals on the CD1400:
 *   CTS (Clear To Send)    — hardware flow control from remote
 *   DSR (Data Set Ready)   — remote device is powered on
 *   DCD (Data Carrier Detect) — modem has carrier (connection up)
 *   RI  (Ring Indicator)   — phone is ringing
 * ==================================================================== */

static VOID CyIsrServiceModem(
    PCY_FDO_EXT fdoExt,
    PUCHAR      chipBase,
    PCY_PDO_EXT *portArray,
    PULONG      dpcFlags)
{
    UCHAR   mivr;               /* Modem Interrupt Vector Register  */
    UCHAR   channel;
    UCHAR   misr;               /* Modem Interrupt Status Register  */
    UCHAR   msvr;               /* Current modem signal values      */
    ULONG   portIndex;
    PCY_PDO_EXT pdoExt;

    /* Read MIVR for the channel with the modem change.
     * Bits [3:2] = channel number. */
    mivr = CyReadReg(chipBase, CyMIVR);
    channel = (mivr >> 2) & 0x03;

    portIndex = (ULONG)((ULONG_PTR)(chipBase - fdoExt->CardBase) / CY_REG_SIZE)
                * CY_PORTS_PER_CHIP + channel;

    if (portIndex >= fdoExt->NumPDOs) {
        CyWriteReg(chipBase, CyEOSRR, 0);  /* MANDATORY */
        return;
    }

    pdoExt = portArray[portIndex];
    if (!pdoExt || pdoExt->Common.PnPState != CyStarted) {
        CyWriteReg(chipBase, CyEOSRR, 0);  /* MANDATORY */
        return;
    }

    /* Read MISR to see WHICH signals changed (delta bits).
     * Then read MSVR1 and MSVR2 for the CURRENT signal levels.
     * We must read both MSVRs — on some CD1400 revisions they
     * report different subsets. Linux ORs them together.
     * (Audit B1 platform-difference fix) */
    misr = CyReadReg(chipBase, CyMISR);
    msvr = CyReadReg(chipBase, CyMSVR1);
    msvr |= CyReadReg(chipBase, CyMSVR2);

    /* Update the shadow register.
     * Dispatch-level code reads ShadowMSVR for GET_MODEMSTATUS.
     * The ISR is the only writer, so no lock needed here. */
    pdoExt->ShadowMSVR = msvr;

    /* Set event flags for WaitCommEvent.
     * Applications use WaitCommEvent to detect signal changes.
     * We map CD1400 signal bits to Win32 EV_* constants. */
    if (misr & CyCTS)
        pdoExt->EventHistory |= SERIAL_EV_CTS;
    if (misr & CyDSR)
        pdoExt->EventHistory |= SERIAL_EV_DSR;
    if (misr & CyDCD)
        pdoExt->EventHistory |= SERIAL_EV_RLSD;    /* RLSD = DCD   */
    if (misr & CyRI)
        pdoExt->EventHistory |= SERIAL_EV_RING;

    /* Flag the modem DPC */
    *dpcFlags |= (1 << portIndex);

    /* EOSRR — MANDATORY. See CyIsrServiceReceive for explanation. */
    CyWriteReg(chipBase, CyEOSRR, 0);

        CY_ISR_DBG("MDM: port %lu chan %u, MISR=0x%02X MSVR=0x%02X "
               "(CTS=%u DSR=%u RI=%u DCD=%u)\n",
               portIndex, channel, misr, msvr,
               (msvr & CyCTS) ? 1 : 0, (msvr & CyDSR) ? 1 : 0,
               (msvr & CyRI) ? 1 : 0, (msvr & CyDCD) ? 1 : 0);
}


/* ====================================================================
 * CyInterruptService — Main Interrupt Service Routine
 * ====================================================================
 * Called by Windows when the hardware interrupt fires.
 *
 * Runs at DIRQL — above DISPATCH_LEVEL. Rules:
 *   - No paged memory access (everything must be non-paged)
 *   - No pool allocation (ExAllocatePool is forbidden)
 *   - No IRP completion (must defer to DPC)
 *   - No KeWait* (can't block)
 *   - No KdPrint (too slow for ISR)
 *   - Must be FAST — other interrupts are masked
 *
 * Algorithm:
 *   For each chip on the card:
 *     Read SVRR (Service Vector Request Register)
 *     If SVRR == 0, this chip didn't interrupt → skip
 *     Service the highest-priority pending request:
 *       Bit 0 set → receive service (CyIsrServiceReceive)
 *       Bit 1 set → transmit service (CyIsrServiceTransmit)
 *       Bit 2 set → modem service (CyIsrServiceModem)
 *     Loop until SVRR == 0 or max iterations reached
 *
 * Returns TRUE if we serviced the interrupt (it was ours).
 * Returns FALSE if no chip had pending service (shared IRQ,
 * interrupt belongs to another device on the same IRQ line).
 * ==================================================================== */

BOOLEAN CyInterruptService(
    IN PKINTERRUPT  Interrupt,
    IN PVOID        ServiceContext)
{
    PCY_FDO_EXT fdoExt = (PCY_FDO_EXT)ServiceContext;
    ULONG       chip;
    UCHAR       svrr;
    BOOLEAN     serviced = FALSE;
    ULONG       iterations = 0;
    ULONG       rxDpcFlags = 0;     /* Bitmask: which ports need RX DPC */
    ULONG       txDpcFlags = 0;     /* Bitmask: which ports need TX DPC */
    ULONG       mdmDpcFlags = 0;    /* Bitmask: which ports need modem DPC */

    /* Build a quick-access array of PDO extensions.
     * We do this once at ISR entry instead of looking it up for
     * every service request. The ChildPDOs array is stable during
     * the ISR because PnP operations happen at PASSIVE_LEVEL. */
    PCY_PDO_EXT portArray[CY_MAX_CHIPS * CY_PORTS_PER_CHIP];
    ULONG       i;

    UNREFERENCED_PARAMETER(Interrupt);

    TraceEvents(CYPORT_LEVEL_VERBOSE, CYPORT_ISR,
                "ISR entry: %lu chips", fdoExt->NumChips);

    for (i = 0; i < fdoExt->NumPDOs; i++) {
        if (fdoExt->ChildPDOs[i])
            portArray[i] = CyGetPdo(fdoExt->ChildPDOs[i]);
        else
            portArray[i] = NULL;
    }

    /* ================================================================
     * Main ISR loop — scan all chips on the card
     * ================================================================
     * We scan chips in order. For each chip, we read SVRR to check
     * if it has a pending service request. If so, we service it.
     *
     * We loop back to the first chip after servicing because a new
     * service request may have arrived while we were processing.
     * The iteration cap (CY_ISR_MAX_ITERATIONS) prevents infinite
     * looping if the hardware is stuck asserting SVRR.
     *
     * On shared IRQ lines (common for ISA), we MUST check SVRR
     * before claiming the interrupt. If no chip has SVRR set,
     * we return FALSE and Windows gives the interrupt to the next
     * driver on the shared IRQ chain. Claiming a shared interrupt
     * that isn't ours would starve the real owner.
     *
     * DEBUG: ISR debug output uses DbgPrint directly (not
     * CyDbgPrint) because CyDbgPrint may not be safe at DIRQL
     * on all platforms. ISR debug is controlled by CY_DEBUG_REGS
     * define and compiled out entirely in release builds.
     * ================================================================ */

/* CY_ISR_DBG moved to top of file for OW forward-reference compat */

    while (iterations < CY_ISR_MAX_ITERATIONS) {

        BOOLEAN anyActivity = FALSE;

        for (chip = 0; chip < fdoExt->NumChips; chip++) {
            PUCHAR chipBase = fdoExt->Chips[chip].Base;

            /* Read the Service Vector Request Register.
             *   Bit 0: Receive service pending
             *   Bit 1: Transmit service pending
             *   Bit 2: Modem signal change pending
             *
             * If SVRR == 0, no service requests on this chip.
             * If SVRR != 0, service the highest-priority bit.
             *
             * We check bits in priority order: RX first (most
             * time-critical — data is in the FIFO and will be
             * lost on overrun), then TX, then modem. */

            svrr = CyReadReg(chipBase, CySVRR);

            if (svrr == 0)
                continue;       /* No service request on this chip  */

            CY_ISR_DBG("chip %lu SVRR=0x%02X (RX=%u TX=%u MDM=%u)\n",
                       chip, svrr,
                       (svrr & CySRReceive) ? 1 : 0,
                       (svrr & CySRTransmit) ? 1 : 0,
                       (svrr & CySRModem) ? 1 : 0);

            /* ---- Receive service (highest priority) ---- */
            if (svrr & CySRReceive) {
                CyIsrServiceReceive(fdoExt, chipBase,
                                    portArray, &rxDpcFlags);
                anyActivity = TRUE;
                serviced = TRUE;
            }

            /* ---- Transmit service ---- */
            if (svrr & CySRTransmit) {
                CyIsrServiceTransmit(fdoExt, chipBase,
                                     portArray, &txDpcFlags);
                anyActivity = TRUE;
                serviced = TRUE;
            }

            /* ---- Modem signal change (lowest priority) ---- */
            if (svrr & CySRModem) {
                CyIsrServiceModem(fdoExt, chipBase,
                                  portArray, &mdmDpcFlags);
                anyActivity = TRUE;
                serviced = TRUE;
            }
        }

        if (!anyActivity)
            break;              /* No more service requests anywhere */

        iterations++;
    }

    /* ================================================================
     * Queue DPCs for ports that received data or events
     * ================================================================
     * We can't complete IRPs at DIRQL — that requires DISPATCH_LEVEL
     * or below. So we queue a DPC for each port that needs attention.
     * The DPC runs shortly after the ISR returns, at DISPATCH_LEVEL,
     * where it can safely call IoCompleteRequest.
     *
     * We only queue DPCs for ports that actually had activity,
     * using the bitmask flags set by the service routines. This
     * avoids waking up DPCs for ports with no new data.
     * ================================================================ */

    for (i = 0; i < fdoExt->NumPDOs; i++) {
        if (!portArray[i])
            continue;

        /* Queue read DPC if this port received data or an error */
        if (rxDpcFlags & (1 << i)) {
            KeInsertQueueDpc(&portArray[i]->ReadDpc, NULL, NULL);
        }

        /* Queue write DPC if this port drained TX data
         * (there may now be space for more pending write data) */
        if (txDpcFlags & (1 << i)) {
            KeInsertQueueDpc(&portArray[i]->WriteDpc, NULL, NULL);
        }

        /* Queue modem DPC if signals changed */
        if (mdmDpcFlags & (1 << i)) {
            KeInsertQueueDpc(&portArray[i]->ModemDpc, NULL, NULL);
        }
    }

    return serviced;
}


/* ====================================================================
 * DPC Routines — ISR Bottom Halves
 * ====================================================================
 * These run at DISPATCH_LEVEL after the ISR queues them via
 * KeInsertQueueDpc. At DISPATCH_LEVEL:
 *   - CAN call IoCompleteRequest to finish pending IRPs
 *   - CAN access non-paged pool memory
 *   - CANNOT access paged pool or paged code (will bugcheck)
 *   - CANNOT block or wait (KeWaitForSingleObject is forbidden)
 *
 * Each DPC checks the ring buffer and pending IRP queue. If there's
 * data in RxBuf and a pending read IRP, it copies data and completes
 * the IRP. If there's space in TxBuf and a pending write IRP, it
 * copies data from the IRP and completes it.
 * ==================================================================== */

/* ====================================================================
 * CyReadDpcRoutine — Complete pending read IRPs with received data
 * ====================================================================
 * Runs at DISPATCH_LEVEL. Checks the RX ring buffer for available
 * data. If data is present and a read IRP is pending in the
 * cancel-safe queue, dequeues the IRP, copies data from the ring
 * buffer to the IRP's system buffer, and completes the IRP.
 * ==================================================================== */

VOID CyReadDpcRoutine(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2)
{
    PCY_PDO_EXT pdoExt = (PCY_PDO_EXT)DeferredContext;
    UNREFERENCED_PARAMETER(pdoExt);

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    /* Delegate to the real implementation in cyread.c.
     * CyReadDpcComplete dequeues pending read IRPs from the
     * cancel-safe queue and fills them from the RX ring buffer. */
    CyReadDpcComplete(pdoExt);
}


/* ====================================================================
 * CyWriteDpcRoutine — Fill TX ring buffer from pending write IRPs
 * ==================================================================== */

VOID CyWriteDpcRoutine(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2)
{
    PCY_PDO_EXT pdoExt = (PCY_PDO_EXT)DeferredContext;
    UNREFERENCED_PARAMETER(pdoExt);

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    /* Delegate to the real implementation in cywrite.c.
     * CyWriteDpcComplete dequeues pending write IRPs and fills
     * TxBuf, then kicks the transmitter if needed. */
    CyWriteDpcComplete(pdoExt);
}


/* ====================================================================
 * CyModemDpcRoutine — Complete pending WaitCommEvent IRPs
 * ==================================================================== */

VOID CyModemDpcRoutine(
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2)
{
    PCY_PDO_EXT pdoExt = (PCY_PDO_EXT)DeferredContext;
    UNREFERENCED_PARAMETER(pdoExt);

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    /* NOTE: WAIT_ON_MASK IOCTL is implemented in cyioctl.c.
     *
     * 1. Acquire EventLock spinlock
     * 2. Check if WaitIrp is pending and EventHistory & WaitMask != 0
     * 3. If so, copy matching events to the IRP's output buffer
     * 4. Clear the matched bits from EventHistory
     * 5. Complete the WaitIrp
     * 6. Release EventLock
     */
}


/* ====================================================================
 * CyConnectInterrupt — Connect ISR to hardware interrupt
 * ====================================================================
 * Called from PDO START_DEVICE. Connects our ISR to the interrupt
 * line assigned by the PCI bus driver.
 *
 * For shared interrupts (common on ISA, always on PCI INTx), we
 * set ShareVector = TRUE. Our ISR returns FALSE when SVRR shows
 * no pending service, allowing other drivers on the same IRQ to
 * process the interrupt.
 *
 * NOTE: All ports on one card share the same interrupt. We connect
 * the ISR once for the first port that starts, and disconnect when
 * the last port stops. The FDO extension holds the interrupt object.
 * ==================================================================== */

NTSTATUS CyConnectInterrupt(PCY_FDO_EXT fdoExt, PCY_PDO_EXT pdoExt)
{

    /* Only connect if not already connected.
     * Multiple ports share one interrupt — first port connects. */
    if (fdoExt->NumPDOs > 0 && pdoExt->InterruptConnected) {
        return STATUS_SUCCESS;  /* Already connected                */
    }

    /* Interrupt resources are parsed in CyFdoPnP START_DEVICE
     * (cyenum.c) and stored in the FDO extension. The connect
     * call uses those values.
     *
     * IoConnectInterrupt parameters:
     *   InterruptObject  — receives the PKINTERRUPT handle
     *   ServiceRoutine   — our ISR function
     *   ServiceContext    — FDO extension (passed as Interrupt context)
     *   SpinLock          — NULL = system provides one
     *   Vector            — interrupt vector from PCI config
     *   Irql              — DIRQL for this interrupt
     *   SynchronizeIrql   — same as Irql (no sync across multiple IRQs)
     *   InterruptMode     — LevelSensitive for PCI, Latched for ISA
     *   ShareVector        — TRUE (PCI INTx is always shared)
     *   ProcessorAffinity — run on any processor
     *   FloatingSave       — FALSE (we don't use FPU in ISR)
     */

    /* NOTE: Interrupt connect uses resources parsed in FDO START_DEVICE */
    pdoExt->InterruptConnected = FALSE;

    return STATUS_SUCCESS;
}


/* ====================================================================
 * CyDisconnectInterrupt — Disconnect ISR from hardware interrupt
 * ====================================================================
 * Called from PDO REMOVE_DEVICE. Disconnects the ISR.
 * After this call, no more interrupts will be delivered to our ISR.
 * ==================================================================== */

VOID CyDisconnectInterrupt(PCY_PDO_EXT pdoExt)
{
    if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
        IoDisconnectInterrupt(pdoExt->Interrupt);
        pdoExt->Interrupt = NULL;
        pdoExt->InterruptConnected = FALSE;
    }
}


/* ====================================================================
 * KeSynchronizeExecution Wrappers
 * ====================================================================
 * These functions allow dispatch-level code to safely access CD1400
 * registers by running inside KeSynchronizeExecution, which:
 *   1. Raises IRQL to DIRQL (same level as the ISR)
 *   2. Acquires the ISR spinlock
 *   3. Runs the callback function
 *   4. Releases the spinlock
 *   5. Lowers IRQL back to the caller's level
 *
 * This guarantees that the callback and the ISR can NEVER run
 * simultaneously. So the callback can safely write CAR and then
 * read/write per-channel registers without the ISR changing CAR
 * between the two operations.
 *
 * This is the CORRECT pattern for CD1400 channel access outside
 * the ISR. The original cyyport.sys imports KeSynchronizeExecution
 * for exactly this purpose (confirmed by import analysis).
 *
 * Usage:
 *   CY_SYNC_CONTEXT ctx;
 *   ctx.Extension = pdoExt;
 *   ctx.Data = &baudRate;
 *   KeSynchronizeExecution(pdoExt->Interrupt,
 *                          CySyncSetBaud, &ctx);
 * ==================================================================== */

/* CY_SYNC_CONTEXT is defined in cycommon.h so that cyioctl.c and
 * other files can also use KeSynchronizeExecution wrappers.
 * (Audit W2 fix — was originally defined only in this .c file) */

/* ====================================================================
 * CySyncReadMSVR — Read modem status inside ISR sync
 * ====================================================================
 * Selects the channel, reads MSVR1, stores in context.
 * Called via KeSynchronizeExecution from GET_MODEMSTATUS IOCTL. */

BOOLEAN NTAPI CySyncReadMSVR(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    UCHAR msvr;

    /* Safe to write CAR here — ISR is blocked */
    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Read BOTH MSVR1 and MSVR2 and OR them together.
     * On some CD1400 revisions, MSVR1 and MSVR2 report different
     * subsets of modem signals. MSVR1 has input signals (CTS/DSR/
     * RI/DCD), MSVR2 may have output signal readback (DTR/RTS).
     * Linux reads both and ORs them — we must match.
     * (Audit B1 platform-difference fix) */
    msvr = CyReadReg(ext->ChipBase, CyMSVR1);
    msvr |= CyReadReg(ext->ChipBase, CyMSVR2);

    *(PUCHAR)ctx->Data = msvr;
    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}

/* ====================================================================
 * CySyncEnableTx — Enable TX interrupt to kick the transmitter
 * ====================================================================
 * Called via KeSynchronizeExecution after dispatch-level code puts
 * data into the TX ring buffer. Sets the TxRdy bit in SRER so the
 * chip will interrupt us when the hardware TX FIFO has space. */

BOOLEAN NTAPI CySyncEnableTx(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    UCHAR srer;
    UCHAR msvr;

    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Check if CTS flow control is active. If CTS is low (remote
     * end says "stop sending"), don't enable TxRdy — the chip's
     * CtsAE (COR2) auto-enable handles this in hardware, but we
     * also check here to avoid queuing a spurious TX interrupt
     * that would fire immediately and find nothing to send.
     *
     * Without this check, the ISR services TX even when CTS is
     * deasserted, wastes CPU cycles, and may confuse flow control
     * state tracking. (sysop/0 audit fix) */
    if (ext->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {
        msvr = CyReadReg(ext->ChipBase, CyMSVR1)
             | CyReadReg(ext->ChipBase, CyMSVR2);
        if (!(msvr & CyCTS)) {
            /* CTS is low — remote says stop. Don't enable TX.
             * When CTS goes high, the modem-change ISR will
             * re-enable TxRdy via the DPC. */
            ctx->Status = STATUS_SUCCESS;
            return TRUE;
        }
    }

    /* Read current SRER, set TxRdy bit, write back.
     * This tells the chip to interrupt us when the TX FIFO
     * has space for more bytes. */
    srer = CyReadReg(ext->ChipBase, CySRER);
    srer |= CyTxRdy;
    CyWriteReg(ext->ChipBase, CySRER, srer);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}

/* ====================================================================
 * CySyncPurgeBuffers — Reset ring buffer counts inside ISR sync
 * ====================================================================
 * Called via KeSynchronizeExecution from PURGE IOCTL.
 * Resets head/tail/count atomically with respect to the ISR.
 * This fixes audit bug #9 (PURGE without spinlock). */

BOOLEAN NTAPI CySyncPurgeBuffers(PVOID Context)
{
    PCY_SYNC_CONTEXT ctx = (PCY_SYNC_CONTEXT)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    ULONG mask = *(PULONG)ctx->Data;

    if (mask & SERIAL_PURGE_RXCLEAR) {
        ext->RxHead = ext->RxTail = ext->RxCount = 0;

        /* Also drain the hardware FIFO — read and discard any
         * bytes sitting in the CD1400's 12-byte RX FIFO */
        CySelectChannel(ext->ChipBase, ext->Channel);
        {
            ULONG maxDrain = CY_ISR_MAX_RX_BYTES;
            while (CyReadReg(ext->ChipBase, CyRDCR) && maxDrain-- > 0)
                (void)CyReadReg(ext->ChipBase, CyRDSR);
        }
    }

    if (mask & SERIAL_PURGE_TXCLEAR) {
        ext->TxHead = ext->TxTail = ext->TxCount = 0;

        /* Flush the hardware TX FIFO via CCR command */
        CySelectChannel(ext->ChipBase, ext->Channel);
        CyWriteReg(ext->ChipBase, CyCCR, CyFlushTransFIFO);
    }

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}
