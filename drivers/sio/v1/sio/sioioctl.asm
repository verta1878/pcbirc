;; ====================================================================
;; sioioctl.asm — Generic IOCtl handler (Category 01h)
;; ====================================================================
;; Implements all 20+ ASYNC IOCtl functions per SIOREF.TXT and the
;; OS/2 bsedev.inc ASYNC_* definitions.
;; ====================================================================

;; ====================================================================
;; CmdGenIOCtl — Main IOCtl dispatcher
;; ====================================================================
;; Request packet for Generic IOCtl (command 16):
;;   0Dh: BYTE  — category (must be 01h for serial)
;;   0Eh: BYTE  — function code
;;   0Fh: DWORD — parameter packet address
;;   13h: DWORD — data packet address
;; ====================================================================

CmdGenIOCtl:
        call    GetPortData
        jc      @@IOCtlErr

        les     bx, DWORD PTR [ReqPktOff]

        ;; Verify category = 01h (IOCTL_ASYNC)
        cmp     BYTE PTR es:[bx+0Dh], IOCTL_ASYNC
        jne     @@IOCtlBadCat

        ;; Get function code
        mov     al, es:[bx+0Eh]

        ;; Load parameter and data packet pointers
        ;; (Need DevHlp_VerifyAccess in production — simplified here)
        push    es
        push    bx
        les     bx, es:[bx+0Fh]        ; ES:BX → param packet
        mov     WORD PTR [IOCtlPrmOff], bx
        mov     WORD PTR [IOCtlPrmSeg], es
        pop     bx
        pop     es

        push    es
        push    bx
        les     bx, es:[bx+13h]        ; ES:BX → data packet
        mov     WORD PTR [IOCtlDatOff], bx
        mov     WORD PTR [IOCtlDatSeg], es
        pop     bx
        pop     es

        ;; Dispatch on function code
        cmp     al, 41h
        je      @@F41
        cmp     al, 42h
        je      @@F42
        cmp     al, 43h
        je      @@F43
        cmp     al, 44h
        je      @@F44
        cmp     al, 45h
        je      @@F45
        cmp     al, 46h
        je      @@F46
        cmp     al, 47h
        je      @@F47
        cmp     al, 48h
        je      @@F48
        cmp     al, 4Bh
        je      @@F4B
        cmp     al, 53h
        je      @@F53
        cmp     al, 54h
        je      @@F54
        cmp     al, 61h
        je      @@F61
        cmp     al, 62h
        je      @@F62
        cmp     al, 63h
        je      @@F63
        cmp     al, 64h
        je      @@F64
        cmp     al, 65h
        je      @@F65
        cmp     al, 66h
        je      @@F66
        cmp     al, 67h
        je      @@F67
        cmp     al, 68h
        je      @@F68
        cmp     al, 69h
        je      @@F69
        cmp     al, 6Dh
        je      @@F6D
        cmp     al, 72h
        je      @@F72
        cmp     al, 73h
        je      @@F73
        cmp     al, 74h
        je      @@F74

        ;; Unknown function
        mov     ax, 8103h
        jmp     StratExit

@@F41:  call    IOCtl_SetBaudRate
        jmp     StratExit
@@F42:  call    IOCtl_SetLineCtrl
        jmp     StratExit
@@F43:  call    IOCtl_ExtSetBaudRate
        jmp     StratExit
@@F44:  call    IOCtl_TransmitImm
        jmp     StratExit
@@F45:  call    IOCtl_SetBreakOff
        jmp     StratExit
@@F46:  call    IOCtl_SetModemCtrl
        jmp     StratExit
@@F47:  call    IOCtl_HoldTransmit
        jmp     StratExit
@@F48:  call    IOCtl_StartTransmit
        jmp     StratExit
@@F4B:  call    IOCtl_SetBreakOn
        jmp     StratExit
@@F53:  call    IOCtl_WriteDCB
        jmp     StratExit
@@F54:  call    IOCtl_WriteEnhanced
        jmp     StratExit
@@F61:  call    IOCtl_QueryBaudRate
        jmp     StratExit
@@F62:  call    IOCtl_QueryLineCtrl
        jmp     StratExit
@@F63:  call    IOCtl_ExtQueryBaudRate
        jmp     StratExit
@@F64:  call    IOCtl_QueryStatus
        jmp     StratExit
@@F65:  call    IOCtl_QueryTxStatus
        jmp     StratExit
@@F66:  call    IOCtl_QueryModemOut
        jmp     StratExit
@@F67:  call    IOCtl_QueryModemIn
        jmp     StratExit
@@F68:  call    IOCtl_QueryRxCount
        jmp     StratExit
@@F69:  call    IOCtl_QueryTxCount
        jmp     StratExit
@@F6D:  call    IOCtl_QueryError
        jmp     StratExit
@@F72:  call    IOCtl_QueryEvent
        jmp     StratExit
@@F73:  call    IOCtl_ReadDCB
        jmp     StratExit
@@F74:  call    IOCtl_ReadEnhanced
        jmp     StratExit

@@IOCtlBadCat:
@@IOCtlErr:
        mov     ax, 8103h
        jmp     StratExit

;; IOCtl packet pointer storage
IOCtlPrmOff     DW      ?
IOCtlPrmSeg     DW      ?
IOCtlDatOff     DW      ?
IOCtlDatSeg     DW      ?


;; ====================================================================
;; 41h — Set Bit Rate (WORD parameter)
;; ====================================================================

IOCtl_SetBaudRate:
        les     bx, DWORD PTR [IOCtlPrmOff]
        movzx   eax, WORD PTR es:[bx]   ; Baud rate (16-bit)
        call    SetBaudRate
        jc      @@Fail
        xor     ax, ax
        ret
@@Fail: mov     ax, 8103h
        ret


;; ====================================================================
;; 42h — Set Line Characteristics
;; ====================================================================

IOCtl_SetLineCtrl:
        les     bx, DWORD PTR [IOCtlPrmOff]
        mov     cl, es:[bx]             ; Data bits
        mov     ch, es:[bx+1]           ; Parity
        mov     dl, es:[bx+2]           ; Stop bits

        ;; Build LCR value
        xor     al, al

        ;; Data bits: 5=00, 6=01, 7=10, 8=11
        cmp     cl, 5
        jb      @@BadLine
        cmp     cl, 8
        ja      @@BadLine
        sub     cl, 5
        or      al, cl

        ;; Parity: 0=none, 1=odd, 2=even, 3=mark, 4=space
        cmp     ch, 0
        je      @@NoPar
        cmp     ch, 4
        ja      @@BadLine
        or      al, LCR_PEN             ; Enable parity
        cmp     ch, 2
        jne     @@NotEven
        or      al, LCR_EPS             ; Even parity
        jmp     @@NoPar
@@NotEven:
        cmp     ch, 3
        jne     @@NotMark
        or      al, LCR_STICK           ; Mark parity
        jmp     @@NoPar
@@NotMark:
        cmp     ch, 4
        jne     @@NoPar
        or      al, LCR_EPS OR LCR_STICK ; Space parity
@@NoPar:

        ;; Stop bits: 0=1, 1=1.5, 2=2
        cmp     dl, 0
        je      @@NoStop
        or      al, LCR_STB
@@NoStop:

        ;; Write to UART
        mov     [si].pdLineCtrl, al
        mov     dx, [si].pdIOBase
        add     dx, UART_LCR
        out     dx, al

        xor     ax, ax
        ret
@@BadLine:
        mov     ax, 8103h
        ret


;; ====================================================================
;; 43h — Extended Set Bit Rate (DWORD + BYTE fraction)
;; ====================================================================

IOCtl_ExtSetBaudRate:
        les     bx, DWORD PTR [IOCtlPrmOff]
        mov     eax, es:[bx]            ; DWORD baud rate
        ;; Fraction byte at es:[bx+4] — ignored per spec
        call    SetBaudRate
        jc      @@Fail
        xor     ax, ax
        ret
@@Fail: mov     ax, 8103h
        ret


;; ====================================================================
;; 44h — Transmit Byte Immediate
;; ====================================================================

IOCtl_TransmitImm:
        les     bx, DWORD PTR [IOCtlPrmOff]
        mov     al, es:[bx]
        mov     [si].pdTxImmChar, al
        mov     [si].pdTxImm, 1
        ;; Kick transmitter
        call    TryTransmit
        xor     ax, ax
        ret


;; ====================================================================
;; 45h — Set Break Off
;; ====================================================================

IOCtl_SetBreakOff:
        and     [si].pdFlags, NOT PF_BREAKON

        ;; Clear break bit in LCR
        mov     dx, [si].pdIOBase
        add     dx, UART_LCR
        in      al, dx
        and     al, NOT LCR_BREAK
        out     dx, al

        ;; Return SIO Error Word in data packet
        call    ReturnErrorWord
        xor     ax, ax
        ret


;; ====================================================================
;; 46h — Set Modem Control Signals
;; ====================================================================

IOCtl_SetModemCtrl:
        les     bx, DWORD PTR [IOCtlPrmOff]
        mov     cl, es:[bx]             ; ON mask
        mov     ch, es:[bx+1]           ; OFF mask

        ;; Apply ON mask (set bits for DTR/RTS)
        ;; Bit 0 = DTR, Bit 1 = RTS
        ;; Only modify if signal is not used for flow control
        mov     al, [si].pdMCR

        ;; Check if DTR is flow controlled
        mov     ah, [si].pdDCB.dcbFlags1
        and     ah, DCB_F1_DTR_MASK
        cmp     ah, DCB_F1_DTR_HS
        je      @@NoDTRChange
        ;; Apply DTR from masks
        test    cl, 01h                 ; ON mask bit 0
        jz      @@DTROff
        or      al, MCR_DTR
        jmp     @@NoDTRChange
@@DTROff:
        test    ch, 01h                 ; OFF mask bit 0 (0 = turn off)
        jnz     @@NoDTRChange
        and     al, NOT MCR_DTR
@@NoDTRChange:

        ;; Check if RTS is flow controlled
        mov     ah, [si].pdDCB.dcbFlags2
        and     ah, DCB_F2_RTS_MASK
        cmp     ah, DCB_F2_RTS_HS
        je      @@NoRTSChange
        test    cl, 02h
        jz      @@RTSOff
        or      al, MCR_RTS
        jmp     @@NoRTSChange
@@RTSOff:
        test    ch, 02h
        jnz     @@NoRTSChange
        and     al, NOT MCR_RTS
@@NoRTSChange:

        mov     [si].pdMCR, al
        call    WriteMCR

        call    ReturnErrorWord
        xor     ax, ax
        ret


;; ====================================================================
;; 47h — Hold Transmit
;; ====================================================================

IOCtl_HoldTransmit:
        mov     [si].pdTxHold, 1
        xor     ax, ax
        ret


;; ====================================================================
;; 48h — Start Transmit
;; ====================================================================
;; Note SIO difference from COM.SYS: SIO will NOT re-enable TX on
;; XON receipt after a Hold. Only this IOCtl re-enables.

IOCtl_StartTransmit:
        mov     [si].pdTxHold, 0
        call    TryTransmit
        xor     ax, ax
        ret


;; ====================================================================
;; 4Bh — Set Break On
;; ====================================================================

IOCtl_SetBreakOn:
        or      [si].pdFlags, PF_BREAKON

        mov     dx, [si].pdIOBase
        add     dx, UART_LCR
        in      al, dx
        or      al, LCR_BREAK
        out     dx, al

        call    ReturnErrorWord
        xor     ax, ax
        ret


;; ====================================================================
;; 53h — Write DCB
;; ====================================================================

IOCtl_WriteDCB:
        les     bx, DWORD PTR [IOCtlPrmOff]

        ;; Copy 11 bytes from parameter packet to port's DCB
        push    di
        lea     di, [si].pdDCB
        mov     cx, SIODCB_SIZE
@@CopyDCB:
        mov     al, es:[bx]
        mov     [di], al
        inc     bx
        inc     di
        loop    @@CopyDCB
        pop     di

        ;; SIO forces certain Flags3 bits:
        ;;   FIFO control = 10 (enable)
        ;;   Trigger level = 10 (8 chars)
        ;;   TX load count = 1 (16 chars)
        and     [si].pdDCB.dcbFlags3, NOT (DCB_F3_FIFO_MASK OR DCB_F3_TRIG_MASK)
        or      [si].pdDCB.dcbFlags3, DCB_F3_FIFO_ENA OR DCB_F3_TRIG_8 OR DCB_F3_TXLOAD

        ;; Apply RTS handshake mode if changed
        mov     al, [si].pdDCB.dcbFlags2
        and     al, DCB_F2_RTS_MASK
        cmp     al, DCB_F2_RTS_HS
        jne     @@NoRTSHS
        ;; If RTS handshake, also force CTS enable
        or      [si].pdDCB.dcbFlags1, DCB_F1_CTS_ENA
@@NoRTSHS:

        xor     ax, ax
        ret


;; ====================================================================
;; 54h — Write Enhanced Mode Parameters (ignored by SIO)
;; ====================================================================

IOCtl_WriteEnhanced:
        xor     ax, ax
        ret


;; ====================================================================
;; 61h — Query Current Bit Rate
;; ====================================================================

IOCtl_QueryBaudRate:
        les     bx, DWORD PTR [IOCtlDatOff]
        mov     ax, WORD PTR [si].pdBaudRate
        mov     es:[bx], ax
        xor     ax, ax
        ret


;; ====================================================================
;; 62h — Query Line Characteristics
;; ====================================================================

IOCtl_QueryLineCtrl:
        les     bx, DWORD PTR [IOCtlDatOff]

        ;; Decode LCR back to data/parity/stop
        mov     al, [si].pdLineCtrl

        ;; Data bits
        mov     cl, al
        and     cl, LCR_WLS_MASK
        add     cl, 5                   ; 00=5, 01=6, 10=7, 11=8
        mov     es:[bx], cl

        ;; Parity
        test    al, LCR_PEN
        jz      @@ParNone
        test    al, LCR_STICK
        jnz     @@ParStick
        test    al, LCR_EPS
        jnz     @@ParEven
        mov     BYTE PTR es:[bx+1], 1   ; Odd
        jmp     @@ParDone
@@ParEven:
        mov     BYTE PTR es:[bx+1], 2   ; Even
        jmp     @@ParDone
@@ParStick:
        test    al, LCR_EPS
        jnz     @@ParSpace
        mov     BYTE PTR es:[bx+1], 3   ; Mark
        jmp     @@ParDone
@@ParSpace:
        mov     BYTE PTR es:[bx+1], 4   ; Space
        jmp     @@ParDone
@@ParNone:
        mov     BYTE PTR es:[bx+1], 0
@@ParDone:

        ;; Stop bits
        test    al, LCR_STB
        jnz     @@Stop2
        mov     BYTE PTR es:[bx+2], 0   ; 1 stop bit
        jmp     @@StopDone
@@Stop2:
        mov     BYTE PTR es:[bx+2], 2   ; 2 stop bits
@@StopDone:

        ;; Transmitting break
        test    [si].pdFlags, PF_BREAKON
        jnz     @@BreakYes
        mov     BYTE PTR es:[bx+3], 0
        jmp     @@BreakDone
@@BreakYes:
        mov     BYTE PTR es:[bx+3], 1
@@BreakDone:

        xor     ax, ax
        ret


;; ====================================================================
;; 63h — Query Extended Bit Rate
;; ====================================================================

IOCtl_ExtQueryBaudRate:
        les     bx, DWORD PTR [IOCtlDatOff]

        ;; Current rate + fraction
        mov     eax, [si].pdBaudRate
        mov     es:[bx], eax            ; DWORD current
        mov     BYTE PTR es:[bx+4], 0   ; Fraction = 0

        ;; Minimum rate
        mov     DWORD PTR es:[bx+5], 50
        mov     BYTE PTR es:[bx+9], 0

        ;; Maximum rate (depends on UART type)
        mov     eax, 115200             ; Default max
        cmp     [si].pdUARTType, UART_16550A_HI
        jb      @@MaxSet
        mov     eax, 460800
@@MaxSet:
        mov     es:[bx+10], eax
        mov     BYTE PTR es:[bx+14], 0

        xor     ax, ax
        ret


;; ====================================================================
;; 64h — Query SIO Status
;; ====================================================================

IOCtl_QueryStatus:
        les     bx, DWORD PTR [IOCtlDatOff]
        xor     al, al

        ;; Bit 0: waiting for CTS
        test    [si].pdDCB.dcbFlags1, DCB_F1_CTS_ENA
        jz      @@NoCTSWait
        test    [si].pdMSR, MSR_CTS
        jnz     @@NoCTSWait
        or      al, 01h
@@NoCTSWait:

        ;; Bit 3: waiting because XOFF received or TX held
        cmp     [si].pdXoffRecvd, 0
        jnz     @@XoffWait
        cmp     [si].pdTxHold, 0
        jz      @@NoXoffWait
@@XoffWait:
        or      al, 08h
@@NoXoffWait:

        ;; Bit 5: break being sent
        test    [si].pdFlags, PF_BREAKON
        jz      @@NoBreak
        or      al, 20h
@@NoBreak:

        ;; Bit 6: transmit immediate pending
        cmp     [si].pdTxImm, 0
        jz      @@NoImm
        or      al, 40h
@@NoImm:
        mov     es:[bx], al
        xor     ax, ax
        ret


;; ====================================================================
;; 65h — Query Transmit Data Status
;; ====================================================================

IOCtl_QueryTxStatus:
        les     bx, DWORD PTR [IOCtlDatOff]
        xor     al, al

        ;; Bit 0: write in progress
        test    [si].pdFlags, PF_WRITING
        jz      @@NoWrite
        or      al, 01h
@@NoWrite:

        ;; Bit 1: data in TX buffer
        push    di
        lea     di, [si].pdTxBuf
        cmp     WORD PTR [di].rbCount, 0
        pop     di
        jz      @@NoTxData
        or      al, 02h
@@NoTxData:

        ;; Bit 2: UART currently transmitting
        push    dx
        mov     dx, [si].pdIOBase
        add     dx, UART_LSR
        push    ax
        in      al, dx
        test    al, LSR_TEMT
        pop     ax
        pop     dx
        jnz     @@NotTxing
        or      al, 04h
@@NotTxing:

        ;; Bit 3: transmit immediate pending
        cmp     [si].pdTxImm, 0
        jz      @@NoTxImm
        or      al, 08h
@@NoTxImm:

        mov     es:[bx], al
        xor     ax, ax
        ret


;; ====================================================================
;; 66h — Query Current Modem Output Control Signals
;; ====================================================================

IOCtl_QueryModemOut:
        les     bx, DWORD PTR [IOCtlDatOff]
        mov     al, [si].pdMCR
        and     al, MCR_DTR OR MCR_RTS  ; Only bits 0-1
        mov     es:[bx], al
        xor     ax, ax
        ret


;; ====================================================================
;; 67h — Query Current Modem Input Signals
;; ====================================================================

IOCtl_QueryModemIn:
        ;; Read fresh MSR
        mov     dx, [si].pdIOBase
        add     dx, UART_MSR
        in      al, dx
        mov     [si].pdMSR, al

        les     bx, DWORD PTR [IOCtlDatOff]
        and     al, MSR_CTS OR MSR_DSR OR MSR_RI OR MSR_DCD  ; Bits 4-7
        mov     es:[bx], al
        xor     ax, ax
        ret


;; ====================================================================
;; 68h — Query Number of Characters in Receive Buffer
;; ====================================================================

IOCtl_QueryRxCount:
        les     bx, DWORD PTR [IOCtlDatOff]
        push    di
        lea     di, [si].pdRxBuf
        mov     ax, [di].rbCount
        mov     es:[bx], ax             ; Count
        mov     ax, [di].rbSize
        mov     es:[bx+2], ax           ; Buffer size
        pop     di
        xor     ax, ax
        ret


;; ====================================================================
;; 69h — Query Number of Characters in Transmit Buffer
;; ====================================================================

IOCtl_QueryTxCount:
        les     bx, DWORD PTR [IOCtlDatOff]
        push    di
        lea     di, [si].pdTxBuf
        mov     ax, [di].rbCount
        mov     es:[bx], ax
        mov     ax, [di].rbSize
        mov     es:[bx+2], ax
        pop     di
        xor     ax, ax
        ret


;; ====================================================================
;; 6Dh — Query SIO Error (resets error word after reading)
;; ====================================================================

IOCtl_QueryError:
        les     bx, DWORD PTR [IOCtlDatOff]
        mov     ax, [si].pdErrorWord
        mov     es:[bx], ax
        mov     [si].pdErrorWord, 0     ; Reset after read
        xor     ax, ax
        ret


;; ====================================================================
;; 72h — Query SIO Event Information (resets after reading)
;; ====================================================================

IOCtl_QueryEvent:
        les     bx, DWORD PTR [IOCtlDatOff]
        mov     ax, [si].pdEventWord
        mov     es:[bx], ax
        mov     [si].pdEventWord, 0     ; Reset after read
        xor     ax, ax
        ret


;; ====================================================================
;; 73h — Read DCB
;; ====================================================================

IOCtl_ReadDCB:
        les     bx, DWORD PTR [IOCtlDatOff]
        push    di
        lea     di, [si].pdDCB
        mov     cx, SIODCB_SIZE
@@ReadDCBLoop:
        mov     al, [di]
        mov     es:[bx], al
        inc     di
        inc     bx
        loop    @@ReadDCBLoop
        pop     di
        xor     ax, ax
        ret


;; ====================================================================
;; 74h — Read Enhanced Mode Parameters
;; ====================================================================

IOCtl_ReadEnhanced:
        les     bx, DWORD PTR [IOCtlDatOff]
        mov     BYTE PTR es:[bx], 0     ; No enhanced mode support
        mov     DWORD PTR es:[bx+1], 0  ; Reserved
        xor     ax, ax
        ret


;; ====================================================================
;; Helper: ReturnErrorWord — Write error word to data packet
;; ====================================================================

ReturnErrorWord:
        push    es
        push    bx
        les     bx, DWORD PTR [IOCtlDatOff]
        test    bx, bx                  ; NULL data packet?
        jz      @@NoDataPkt
        mov     ax, [si].pdErrorWord
        mov     es:[bx], ax
@@NoDataPkt:
        pop     bx
        pop     es
        ret

;; End-of-data marker for INIT discard
EndOfData       LABEL   BYTE
