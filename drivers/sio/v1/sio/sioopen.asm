;; ====================================================================
;; sioopen.asm — Open and Close command handlers
;; ====================================================================

;; ====================================================================
;; CmdOpen — Handle device Open request
;; ====================================================================
;; Initial Open:
;;   - Claim IRQ via DevHlp_SetIRQ
;;   - Set DTR/RTS per DCB Flags1/Flags2
;;   - Set XON/XOFF defaults
;;   - Clear and reinitialize buffers
;;   - Reset status bits
;;   - Set read timeout to "Normal" for OS/2 processes
;; Additional Open:
;;   - Increment open count, no changes to port settings
;; ====================================================================

CmdOpen:
        call    GetPortData
        jc      @@OpenErr

        ;; Check if port hardware exists
        test    [si].pdFlags, PF_EXISTS OR PF_INTERNET
        jz      @@OpenErr

        ;; Is this an Initial Open or Additional Open?
        cmp     [si].pdOpenCount, 0
        jne     @@AdditionalOpen

;; ---------- Initial Open ----------

        ;; Initialize UART hardware
        test    [si].pdFlags, PF_INTERNET
        jnz     @@SkipHWInit
        call    InitUART
@@SkipHWInit:

        ;; Reset DCB to defaults
        mov     [si].pdDCB.dcbXonChar, 11h
        mov     [si].pdDCB.dcbXoffChar, 13h

        ;; Flush both ring buffers
        push    di
        lea     di, [si].pdRxBuf
        call    RingBufFlush
        lea     di, [si].pdTxBuf
        call    RingBufFlush
        pop     di

        ;; Clear flow control state
        mov     [si].pdTxHold, 0
        mov     [si].pdXoffRecvd, 0
        mov     [si].pdXoffSent, 0
        mov     [si].pdTxImm, 0

        ;; Clear error and event words
        mov     [si].pdErrorWord, 0
        mov     [si].pdEventWord, 0

        ;; Set DTR per DCB Flags1
        mov     al, [si].pdDCB.dcbFlags1
        and     al, DCB_F1_DTR_MASK
        cmp     al, DCB_F1_DTR_ENA
        jne     @@NoDTR
        call    SetDTR
@@NoDTR:

        ;; Set RTS per DCB Flags2
        mov     al, [si].pdDCB.dcbFlags2
        and     al, DCB_F2_RTS_MASK
        cmp     al, DCB_F2_RTS_ENA
        jne     @@NoRTS
        call    SetRTS
@@NoRTS:

        ;; Set read timeout to Normal for OS/2 processes
        ;; (The request packet tells us if caller is DOS or OS/2)
        and     [si].pdDCB.dcbFlags3, NOT DCB_F3_RDTO_MASK
        or      [si].pdDCB.dcbFlags3, DCB_F3_RDTO_NRM

        ;; Claim IRQ (skip for INTERNET ports)
        test    [si].pdFlags, PF_INTERNET
        jnz     @@SkipIRQ

        cmp     [si].pdIRQ, 0FFh         ; NONE?
        je      @@SkipIRQ

        ;; DevHlp_SetIRQ: BX=IRQ, AX=offset of ISR, DL=function
        push    bx
        xor     bh, bh
        mov     bl, [si].pdIRQ
        mov     ax, OFFSET SIOInterrupt
        mov     dh, 1                    ; Shared IRQ
        mov     dl, DevHlp_SetIRQ
        call    [DriverData.ddDevHlp]
        pop     bx
        jc      @@IRQFail

        or      [si].pdFlags, PF_IRQCLAIMED
@@SkipIRQ:

        ;; Enable UART interrupts
        test    [si].pdFlags, PF_INTERNET
        jnz     @@SkipEnableIRQ
        call    EnableUARTInterrupts
@@SkipEnableIRQ:

        ;; Mark port as open
        or      [si].pdFlags, PF_OPEN
        mov     [si].pdOpenCount, 1

        xor     ax, ax
        jmp     StratExit

;; ---------- Additional (Shared) Open ----------
@@AdditionalOpen:
        inc     [si].pdOpenCount
        xor     ax, ax
        jmp     StratExit

@@OpenErr:
        mov     ax, 8110h               ; ERROR_I24_DEVICE_IN_USE / not found
        jmp     StratExit

@@IRQFail:
        mov     ax, 8110h               ; Port in use (IRQ not available)
        jmp     StratExit


;; ====================================================================
;; CmdClose — Handle device Close request
;; ====================================================================
;; Intermediate Close:
;;   - Decrement open count, no action at port
;; Last Close:
;;   - Wait for pending writes to complete (up to 60 sec timeout)
;;   - Drop DTR/RTS if DCB says "Enabled"
;;   - Release IRQ
;;   - Unblock any waiting readers
;; ====================================================================

CmdClose:
        call    GetPortData
        jc      @@CloseErr

        ;; Must be open
        test    [si].pdFlags, PF_OPEN
        jz      @@CloseErr

        ;; Decrement open count
        dec     [si].pdOpenCount
        cmp     [si].pdOpenCount, 0
        jne     @@IntermediateClose

;; ---------- Last Close ----------

        ;; Wait for TX buffer to drain (with 60-second watchdog)
        test    [si].pdFlags, PF_INTERNET
        jnz     @@SkipDrain

        push    di
        lea     di, [si].pdTxBuf
        call    RingBufCount
        pop     di
        test    cx, cx
        jz      @@TxDrained

        ;; Block waiting for TX to empty
        ;; DevHlp_ProcBlock with 60-second timeout
        push    eax
        mov     eax, [si].pdWriteWait
        mov     ecx, 60000              ; 60 seconds in milliseconds
        mov     dh, 1                   ; Interruptible
        mov     dl, DevHlp_ProcBlock
        call    [DriverData.ddDevHlp]
        pop     eax
        ;; If timeout or interrupted, force close anyway

@@TxDrained:
@@SkipDrain:

        ;; Disable UART interrupts
        test    [si].pdFlags, PF_INTERNET
        jnz     @@SkipDisableIRQ
        call    DisableUARTInterrupts
@@SkipDisableIRQ:

        ;; Drop DTR if DTR mode is "Enabled"
        mov     al, [si].pdDCB.dcbFlags1
        and     al, DCB_F1_DTR_MASK
        cmp     al, DCB_F1_DTR_ENA
        jne     @@NoDTRDrop
        test    [si].pdFlags, PF_INTERNET
        jnz     @@NoDTRDrop
        call    ClearDTR
@@NoDTRDrop:

        ;; Drop RTS if RTS mode is "Enabled"
        mov     al, [si].pdDCB.dcbFlags2
        and     al, DCB_F2_RTS_MASK
        cmp     al, DCB_F2_RTS_ENA
        jne     @@NoRTSDrop
        test    [si].pdFlags, PF_INTERNET
        jnz     @@NoRTSDrop
        call    ClearRTS
@@NoRTSDrop:

        ;; Release IRQ
        test    [si].pdFlags, PF_IRQCLAIMED
        jz      @@NoIRQRelease
        push    bx
        xor     bh, bh
        mov     bl, [si].pdIRQ
        mov     dl, DevHlp_UnSetIRQ
        call    [DriverData.ddDevHlp]
        pop     bx
        and     [si].pdFlags, NOT PF_IRQCLAIMED
@@NoIRQRelease:

        ;; Unblock any waiting readers
        push    eax
        mov     eax, [si].pdReadWait
        test    eax, eax
        jz      @@NoWakeRead
        mov     dl, DevHlp_ProcRun
        call    [DriverData.ddDevHlp]
@@NoWakeRead:
        pop     eax

        ;; Mark port as closed
        and     [si].pdFlags, NOT PF_OPEN
        mov     [si].pdOpenCount, 0

@@IntermediateClose:
        xor     ax, ax
        jmp     StratExit

@@CloseErr:
        mov     ax, 8106h               ; ERROR_I24_INVALID_HANDLE
        jmp     StratExit
