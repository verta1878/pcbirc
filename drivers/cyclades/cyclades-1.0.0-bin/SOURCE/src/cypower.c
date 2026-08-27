/* ====================================================================
 * cypower.c — Power Management (Sleep / Resume / Hibernate)
 * ====================================================================
 * Handles IRP_MJ_POWER for both FDO and PDO devices.
 *
 * When the system enters sleep (S1-S4) or hibernate (S4), we must:
 *   1. Save the CD1400 channel state (baud, line, modem, flow)
 *   2. Disable the channel (stop TX/RX, drop signals)
 *   3. Allow the PCI bus to power down the card
 *
 * When the system resumes (back to S0/D0), we must:
 *   1. Wait for the PCI bus to restore BAR mappings
 *   2. Re-initialize the CD1400 chip (it lost all state on power-off)
 *   3. Restore the saved channel state
 *   4. Re-enable TX/RX and raise signals
 *
 * The CD1400 has NO power management of its own — it's a dumb UART.
 * When the PCI card loses power, ALL register contents are lost.
 * We must save everything in software and restore on resume.
 *
 * Key rules for WDM power handling:
 *   - FDO: forward power IRPs to lower driver via PoCallDriver
 *     (NOT IoCallDriver — power IRPs have special routing)
 *   - PDO: complete power IRPs directly (bottom of stack)
 *   - MUST call PoStartNextPowerIrp on EVERY power IRP (Win2K
 *     requirement — without it, the power manager deadlocks
 *     waiting for the current power IRP to complete)
 *   - Device power transitions: D0 (full on) ↔ D3 (off)
 *   - System power transitions: S0 (working) → S1-S4 (sleep/hibernate)
 *
 * Sources:
 *   - Original cyyport.sys imports: PoCallDriver, PoRequestPowerIrp,
 *     PoSetPowerState, PoStartNextPowerIrp (all confirmed)
 *   - MSDN "Handling Power IRPs" documentation
 *   - WDK toaster sample power management
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cypower.tmh"
#endif


/* ====================================================================
 * Saved Channel State
 * ====================================================================
 * The CY_SAVED_STATE structure lives in CY_PDO_EXT.SavedPowerState
 * (defined in cycommon.h) so it persists between the D0→D3 power
 * IRP (where we save) and the D3→D0 power IRP (where we restore).
 * These are separate dispatch calls with separate stack frames —
 * a local variable would be lost between them.
 *
 * We DON'T save ring buffer contents — any data in transit is lost
 * during sleep. Applications should flush before sleep. This is
 * standard behavior for serial ports.
 *
 * The typedef below is a convenience alias for the anonymous struct
 * in CY_PDO_EXT. The save/restore functions use it via pointer.
 * ==================================================================== */

/* CY_SAVED_STATE is the type of CY_PDO_EXT.SavedPowerState.
 * We can't typedef an anonymous struct member directly, so we
 * define a matching struct here. The layouts MUST match — any
 * change to one must be mirrored in the other. */
typedef struct _CY_SAVED_STATE {
    BOOLEAN     Valid;
    UCHAR       COR1;
    UCHAR       COR2;
    UCHAR       COR3;
    UCHAR       TCOR;
    UCHAR       TBPR;
    UCHAR       RCOR;
    UCHAR       RBPR;
    UCHAR       SRER;
    UCHAR       MSVR;
    UCHAR       SCHR1;
    UCHAR       SCHR2;
} CY_SAVED_STATE;


/* ====================================================================
 * CySaveChannelState — Save CD1400 registers before power-down
 * ====================================================================
 * Runs via KeSynchronizeExecution to safely read per-channel
 * registers while the ISR is blocked.
 *
 * We save everything needed to fully restore the channel on resume:
 * baud rate (TCOR/TBPR/RCOR/RBPR), line control (COR1), flow
 * control (COR2), FIFO threshold (COR3), interrupt enables (SRER),
 * modem outputs (MSVR), and special characters (SCHR1/2).
 * ==================================================================== */

typedef struct _CY_POWER_CONTEXT {
    PCY_PDO_EXT     Extension;
    CY_SAVED_STATE  *SavedState;    /* Where to save/restore         */
    NTSTATUS        Status;
} CY_POWER_CONTEXT;

static BOOLEAN NTAPI CySaveChannelStateSync(PVOID Context)
{
    CY_POWER_CONTEXT *ctx = (CY_POWER_CONTEXT *)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    CY_SAVED_STATE *state = ctx->SavedState;

    /* Select our channel — safe at DIRQL */
    CySelectChannel(ext->ChipBase, ext->Channel);

    /* Read all per-channel registers that have configuration state.
     * We don't need to save data registers (RDSR/TDR) — those are
     * transient. We don't save status registers (MSVR1 read-only
     * inputs like CTS/DSR) — those come from the remote end and
     * will be re-read on resume. We only save OUTPUT state (DTR/RTS)
     * and CONFIGURATION state (baud, line, flow). */

    state->COR1 = CyReadReg(ext->ChipBase, CyCOR1);
    state->COR2 = CyReadReg(ext->ChipBase, CyCOR2);
    state->COR3 = CyReadReg(ext->ChipBase, CyCOR3);
    state->TCOR = CyReadReg(ext->ChipBase, CyTCOR);
    state->TBPR = CyReadReg(ext->ChipBase, CyTBPR);
    state->RCOR = CyReadReg(ext->ChipBase, CyRCOR);
    state->RBPR = CyReadReg(ext->ChipBase, CyRBPR);
    state->SRER = CyReadReg(ext->ChipBase, CySRER);
    state->SCHR1 = CyReadReg(ext->ChipBase, CySCHR1);
    state->SCHR2 = CyReadReg(ext->ChipBase, CySCHR2);

    /* Read MSVR1 and MSVR2 for our OUTPUT signals (DTR/RTS).
     * OR them together — some revisions split signals across registers.
     * We only care about bits 0 (RTS) and 1 (DTR) which we control.
     * (Audit B1 platform-difference fix) */
    state->MSVR = (CyReadReg(ext->ChipBase, CyMSVR1)
                 | CyReadReg(ext->ChipBase, CyMSVR2)) & 0x03;

    state->Valid = TRUE;

    CyInfo("POWER: port %lu state saved (COR1=0x%02X TCOR=0x%02X "
           "TBPR=0x%02X MSVR=0x%02X)\n",
           ext->PortIndex, state->COR1, state->TCOR,
           state->TBPR, state->MSVR);

    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * CyRestoreChannelState — Restore CD1400 registers after power-up
 * ====================================================================
 * Runs via KeSynchronizeExecution. Fully re-initializes the channel
 * from the saved state. The CD1400 has lost ALL register contents
 * during power-off, so we must:
 *   1. Reset the channel (CCR CHAN_RESET)
 *   2. Write all COR registers
 *   3. Issue COR_CHANGE command
 *   4. Write baud rate registers
 *   5. Enable transmitter and receiver
 *   6. Restore modem output signals (DTR/RTS)
 *   7. Re-enable interrupts (SRER)
 *   8. Clear ring buffers (data in transit is lost)
 * ==================================================================== */

static BOOLEAN NTAPI CyRestoreChannelStateSync(PVOID Context)
{
    CY_POWER_CONTEXT *ctx = (CY_POWER_CONTEXT *)Context;
    PCY_PDO_EXT ext = ctx->Extension;
    CY_SAVED_STATE *state = ctx->SavedState;

    if (!state->Valid) {
        CyWarn("POWER: port %lu — no saved state to restore\n",
               ext->PortIndex);
        ctx->Status = STATUS_UNSUCCESSFUL;
        return TRUE;
    }

    CySelectChannel(ext->ChipBase, ext->Channel);

    /* ---- Step 1: Reset the channel ----
     * The chip may be in an unknown state after power-up.
     * A channel reset clears everything and returns to defaults. */
    CyWriteReg(ext->ChipBase, CyCCR, CyCHAN_RESET);
    KeStallExecutionProcessor(100);     /* Wait for reset to complete */

    /* ---- Step 2: Restore COR registers ----
     * These control line format (data/stop/parity), flow control,
     * and RX FIFO threshold. */
    CyWriteReg(ext->ChipBase, CyCOR1, state->COR1);
    CyWriteReg(ext->ChipBase, CyCOR2, state->COR2);
    CyWriteReg(ext->ChipBase, CyCOR3, state->COR3);

    /* ---- Step 3: Tell chip to apply COR changes ----
     * The CD1400 doesn't apply COR writes until a COR_CHANGE
     * command is issued via CCR. */
    CyWriteReg(ext->ChipBase, CyCCR,
               CyCOR_CHANGE | CyCOR1ch | CyCOR2ch | CyCOR3ch);
    KeStallExecutionProcessor(10);

    /* ---- Step 4: Restore baud rate ----
     * Both TX and RX clock option + baud period registers. */
    CyWriteReg(ext->ChipBase, CyTCOR, state->TCOR);
    CyWriteReg(ext->ChipBase, CyTBPR, state->TBPR);
    CyWriteReg(ext->ChipBase, CyRCOR, state->RCOR);
    CyWriteReg(ext->ChipBase, CyRBPR, state->RBPR);

    /* ---- Step 5: Restore special characters ----
     * XON (SCHR1) and XOFF (SCHR2) for software flow control. */
    CyWriteReg(ext->ChipBase, CySCHR1, state->SCHR1);
    CyWriteReg(ext->ChipBase, CySCHR2, state->SCHR2);

    /* ---- Step 6: Enable transmitter and receiver ---- */
    CyWriteReg(ext->ChipBase, CyCCR, CyENB_RCVR | CyENB_XMTR);
    KeStallExecutionProcessor(10);

    /* ---- Step 7: Restore modem output signals (DTR/RTS) ----
     * Only restore if the port was open before sleep. If it was
     * closed, leave signals deasserted. */
    if (ext->IsOpen) {
        CyWriteReg(ext->ChipBase, CyMSVR1, state->MSVR);
        CyWriteReg(ext->ChipBase, CyMSVR2, state->MSVR);
    }

    /* ---- Step 8: Re-enable interrupts ----
     * Restore the SRER value that was active before sleep.
     * This re-enables RX data, TX ready, and modem change
     * interrupts as appropriate. */
    if (ext->IsOpen) {
        CyWriteReg(ext->ChipBase, CySRER, state->SRER);
    }

    /* ---- Step 9: Clear ring buffers ----
     * Any data in transit is lost during sleep. Applications
     * should have flushed before sleep. Starting with empty
     * buffers avoids delivering stale data after resume. */
    ext->RxHead = ext->RxTail = ext->RxCount = 0;
    ext->TxHead = ext->TxTail = ext->TxCount = 0;

    /* ---- Step 10: Re-read modem status ----
     * Read both MSVRs and OR together.
     * (Audit B1 platform-difference fix) */
    ext->ShadowMSVR = CyReadReg(ext->ChipBase, CyMSVR1)
                    | CyReadReg(ext->ChipBase, CyMSVR2);

    CyInfo("POWER: port %lu state restored (COR1=0x%02X TCOR=0x%02X "
           "TBPR=0x%02X MSVR=0x%02X open=%u)\n",
           ext->PortIndex, state->COR1, state->TCOR,
           state->TBPR, state->MSVR, ext->IsOpen ? 1 : 0);

    state->Valid = FALSE;       /* Consumed — don't restore twice    */
    ctx->Status = STATUS_SUCCESS;
    return TRUE;
}


/* ====================================================================
 * CyPdoSetPower — Handle SET_POWER for child PDO (serial port)
 * ====================================================================
 * Called when the power manager transitions our device between
 * D0 (full power) and D3 (off).
 *
 * D0 → D3: Save channel state, disable channel.
 * D3 → D0: Restore channel state, re-enable channel.
 *
 * PDOs complete power IRPs directly — they don't forward.
 * ==================================================================== */

static NTSTATUS CyPdoSetPower(
    PCY_PDO_EXT     pdoExt,
    PIRP            Irp,
    PIO_STACK_LOCATION IrpSp)
{
    POWER_STATE_TYPE type;
    POWER_STATE      powerState;
    POWER_STATE      reportState;   /* For PoSetPowerState            */

    UNREFERENCED_PARAMETER(Irp);    /* (Audit B2 fix)                 */

    type       = IrpSp->Parameters.Power.Type;
    powerState = IrpSp->Parameters.Power.State;

    if (type == DevicePowerState) {
        /* ---- Device power state change ---- */

        if (powerState.DeviceState == PowerDeviceD0 &&
            pdoExt->Common.DevicePowerState != PowerDeviceD0) {

            /* ---- Powering UP (D3 → D0) ----
             * The PCI bus has restored our BAR mappings. The CD1400
             * has been reset by the power cycle — ALL register
             * contents are lost. Restore from saved state. */

            TraceEvents(CYPORT_LEVEL_INFO, CYPORT_POWER,
                   "Port %lu D3 → D0 resuming", pdoExt->PortIndex);
            CyInfo("POWER: port %lu D3 → D0 (resuming)\n",
                   pdoExt->PortIndex);

            if (pdoExt->IsOpen && pdoExt->SavedPowerState.Valid) {
                /* Restore the CD1400 channel from saved state.
                 * This re-initializes baud, line control, flow,
                 * modem signals, and interrupts. Ring buffers are
                 * cleared (data in transit is lost during sleep).
                 *
                 * Uses KeSynchronizeExecution because we're writing
                 * per-channel registers that require CAR to be set,
                 * and the ISR may be reconnected by now.
                 * (Audit B1 fix — was a TODO stub) */
                CY_POWER_CONTEXT ctx;
                ctx.Extension = pdoExt;
                ctx.SavedState = (CY_SAVED_STATE *)&pdoExt->SavedPowerState;

                if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                    KeSynchronizeExecution(pdoExt->Interrupt,
                                           CyRestoreChannelStateSync, &ctx);
                } else {
                    CyRestoreChannelStateSync(&ctx);
                }
            }

            pdoExt->Common.DevicePowerState = PowerDeviceD0;

            /* Notify the power manager of our new device state.
             * The original cyyport.sys imports PoSetPowerState
             * for exactly this purpose. (Audit B2 fix) */
            reportState.DeviceState = PowerDeviceD0;
            PoSetPowerState(pdoExt->Common.Self,
                            DevicePowerState, reportState);

        } else if (powerState.DeviceState == PowerDeviceD3 &&
                   pdoExt->Common.DevicePowerState == PowerDeviceD0) {

            /* ---- Powering DOWN (D0 → D3) ----
             * Save the complete CD1400 channel configuration before
             * the card loses power. Without this, the port is dead
             * after resume — the CD1400 resets to defaults on power
             * cycle and we'd have no record of the baud rate, line
             * control, or modem signal state. */

            TraceEvents(CYPORT_LEVEL_INFO, CYPORT_POWER,
                   "Port %lu D0 → D3 suspending", pdoExt->PortIndex);
            CyInfo("POWER: port %lu D0 → D3 (suspending)\n",
                   pdoExt->PortIndex);

            if (pdoExt->IsOpen) {
                /* Save ALL configuration registers from the CD1400.
                 * (Audit B1 fix — was a TODO stub) */
                CY_POWER_CONTEXT ctx;
                ctx.Extension = pdoExt;
                ctx.SavedState = (CY_SAVED_STATE *)&pdoExt->SavedPowerState;

                if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
                    KeSynchronizeExecution(pdoExt->Interrupt,
                                           CySaveChannelStateSync, &ctx);
                } else {
                    CySaveChannelStateSync(&ctx);
                }
            }

            pdoExt->Common.DevicePowerState = PowerDeviceD3;

            reportState.DeviceState = PowerDeviceD3;
            PoSetPowerState(pdoExt->Common.Self,
                            DevicePowerState, reportState);
        }

    } else if (type == SystemPowerState) {
        /* ---- System power state change ----
         * We track the system state but don't act on it directly.
         * The power manager will send us a device power IRP
         * (D0→D3 or D3→D0) based on our device capabilities
         * that we reported in QUERY_CAPABILITIES.
         *
         * The S-state arithmetic below converts the enum to a
         * human-readable number: PowerSystemWorking(1) → "S0",
         * PowerSystemSleeping1(2) → "S1", etc. This relies on
         * the SYSTEM_POWER_STATE enum starting at 1, which has
         * been true since NT 4.0. (Audit W1 note) */

        CyInfo("POWER: port %lu system S%d → S%d\n",
               pdoExt->PortIndex,
               pdoExt->Common.SystemPowerState - 1,
               powerState.SystemState - 1);

        pdoExt->Common.SystemPowerState = powerState.SystemState;
    }

    return STATUS_SUCCESS;
}


/* ====================================================================
 * CyDispatchPower — Power IRP Dispatch (replaces stub in cyenum.c)
 * ====================================================================
 * Routes power IRPs to FDO (forward to lower) or PDO (handle directly).
 *
 * CRITICAL: PoStartNextPowerIrp MUST be called on EVERY power IRP,
 * even ones we don't care about. On Win2K, omitting this call causes
 * the power manager to deadlock — it waits for "next power IRP ready"
 * signal that never comes. The system hangs on sleep or shutdown.
 *
 * On XP+ this call is harmless (the power manager ignores it for
 * newer drivers), but we call it unconditionally for Win2K compat.
 * ==================================================================== */

NTSTATUS CyDispatchPower(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp)
{
    PCY_COMMON_EXT      common = CyGetCommon(DeviceObject);
    PIO_STACK_LOCATION  irpSp  = IoGetCurrentIrpStackLocation(Irp);
    PCY_FDO_EXT         fdoExt;     /* Used in FDO path only         */
    PCY_PDO_EXT         pdoExt;     /* Used in PDO path only         */
    NTSTATUS            status;

    /* PoStartNextPowerIrp — REQUIRED on Win2K.
     * Must be called BEFORE completing or forwarding the IRP.
     * On Win2K, the power manager serializes power IRPs per device.
     * This call tells it "I'm done with this one, send the next."
     * Without it, the next power IRP is never dispatched and the
     * system hangs during sleep, hibernate, or shutdown. */
    PoStartNextPowerIrp(Irp);

    if (common->IsFDO) {
        /* ---- FDO: Forward to lower driver ----
         * The PCI bus driver handles the actual power transitions
         * (saving/restoring PCI config space, controlling power pins).
         * We just pass the IRP through.
         *
         * MUST use PoCallDriver, NOT IoCallDriver. Power IRPs have
         * special routing that PoCallDriver handles. Using IoCallDriver
         * for a power IRP can cause deadlocks on Win2K.
         *
         * PoStartNextPowerIrp was called above for BOTH paths. On
         * Win2K this is correct — we call it once for our stack
         * location. The lower driver calls it for its own stack
         * location. IoSkipCurrentIrpStackLocation reuses ours, so
         * the lower driver sees a fresh stack entry. (Audit B3 note) */
        fdoExt = CyGetFdo(DeviceObject);

        IoSkipCurrentIrpStackLocation(Irp);
        return PoCallDriver(fdoExt->LowerDevice, Irp);

    } else {
        /* ---- PDO: Handle directly ----
         * PDOs are the bottom of the stack. We process the power
         * state change (save/restore CD1400 state) and complete
         * the IRP. No forwarding — there's no lower device. */
        pdoExt = CyGetPdo(DeviceObject);

        if (irpSp->MinorFunction == IRP_MN_SET_POWER) {
            status = CyPdoSetPower(pdoExt, Irp, irpSp);
        } else if (irpSp->MinorFunction == IRP_MN_QUERY_POWER) {
            /* We always say YES to power queries — we can handle
             * any power state. If we couldn't (e.g., mid-transfer),
             * we'd return STATUS_DEVICE_BUSY to veto the transition. */
            CyTrace("POWER: port %lu QUERY_POWER — accepting\n",
                     pdoExt->PortIndex);
            status = STATUS_SUCCESS;
        } else {
            /* Unknown power minor — succeed silently */
            status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }
}
