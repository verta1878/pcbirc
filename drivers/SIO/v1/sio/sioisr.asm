;; ====================================================================
;; sioisr.asm — Interrupt Service Routine
;; ====================================================================

;; DCB field offsets within PORTDATA.pdDCB (for ISR access)
;; These avoid complex OFFSET arithmetic that may not assemble.
DCB_OFS_FLAGS1  EQU     4
DCB_OFS_FLAGS2  EQU     5
DCB_OFS_FLAGS3  EQU     6
DCB_OFS_XONCH   EQU     9
DCB_OFS_XOFFCH  EQU     10

;; Handles all UART interrupts: RX data, TX empty, line status,
;; modem status. Supports shared IRQs (loops through all ports on
;; the same IRQ until no more interrupts are pending).
;; ====================================================================

;; ====================================================================
;; SIOInterrupt — Main ISR entry point
;; ====================================================================
;; OS/2 calls this via DevHlp_SetIRQ.
;; We must:
;;   1. Check all ports sharing this IRQ
;;   2. Service any pending UART interrupts
;;   3. Call DevHlp_EOI when done
;;   4. Return with CF clear if we handled the interrupt
;; ====================================================================

SIOInterrupt:
        push    ds
        push    es
        push    eax
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        ;; Load our data segment
        mov     ax, SEG _DATA
        mov     ds, ax

        ;; Loop through all configured ports, servicing any with
        ;; pending interrupts. Keep looping until a full pass finds
        ;; nothing — this handles shared IRQs correctly.

        xor     ebx, ebx               ; Port index
        xor     esi, esi                ; Total serviced across all passes
        mov     cx, [DriverData.ddNumPorts]
        test    cx, cx
        jz      @@NotOurs

@@ScanPorts:
        xor     ebx, ebx
        mov     cx, [DriverData.ddNumPorts]
        xor     edi, edi                ; Count of ports serviced this pass

@@NextPort:
        ;; Calculate PORTDATA pointer
        push    cx
        mov     eax, ebx
        mov     cx, SIZE PORTDATA
        imul    cx
        lea     si, [DriverData.ddPorts]
        add     si, ax

        ;; Skip if port not open or not real hardware
        test    [si].pdFlags, PF_OPEN
        jz      @@SkipPort
        test    [si].pdFlags, PF_INTERNET
        jnz     @@SkipPort

        ;; Read IIR to check for pending interrupt
        mov     dx, [si].pdIOBase
        add     dx, UART_IIR
        in      al, dx
        test    al, IIR_PENDING         ; Bit 0 = 0 means interrupt pending
        jnz     @@SkipPort

        ;; Service this port's interrupt(s)
        call    ServicePort
        inc     edi                     ; We handled something

@@SkipPort:
        pop     cx
        inc     ebx
        loop    @@NextPort

        ;; If we serviced any port this pass, track it and rescan
        test    edi, edi
        jz      @@CheckTotal
        add     esi, edi                ; Accumulate total serviced
        jmp     @@ScanPorts

@@CheckTotal:
        ;; If we never serviced anything across ALL passes, not our IRQ
        test    esi, esi
        jz      @@NotOurs

@@Handled:
        ;; Send EOI
        mov     dl, DevHlp_EOI
        mov     ax, SEG _DATA
        mov     ds, ax
        call    [DriverData.ddDevHlp]

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     es
        pop     ds
        clc                             ; We handled it
        ret

@@NotOurs:
        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     es
        pop     ds
        stc                             ; Not our interrupt
        ret


;; ====================================================================
;; ServicePort — Handle all pending interrupts for one port
;; ====================================================================
;; In:  DS:SI → PORTDATA
;; Trashes: AX, DX
;; ====================================================================

ServicePort:

@@Loop:
        ;; Read IIR
        mov     dx, [si].pdIOBase
        add     dx, UART_IIR
        in      al, dx
        test    al, IIR_PENDING
        jnz     @@Done                  ; No more interrupts on this port

        and     al, IIR_ID_MASK         ; Isolate interrupt ID

        cmp     al, IIR_RLS
        je      @@LineStatus

        cmp     al, IIR_RDA
        je      @@ReceiveData

        cmp     al, IIR_TIMEOUT
        je      @@ReceiveData           ; Character timeout — drain FIFO

        cmp     al, IIR_THRE
        je      @@TransmitEmpty

        cmp     al, IIR_MS
        je      @@ModemStatus

        ;; Unknown interrupt ID — read all status regs to clear
        jmp     @@ClearAll

;; ---------- Receiver Line Status ----------
@@LineStatus:
        mov     dx, [si].pdIOBase
        add     dx, UART_LSR
        in      al, dx
        or      [si].pdLSR, al          ; Accumulate errors

        ;; Update error word for IOCtl 6Dh
        test    al, LSR_OE
        jz      @@NoOE
        or      [si].pdErrorWord, 0002h ; UART overrun
@@NoOE:
        test    al, LSR_PE
        jz      @@NoPE
        or      [si].pdErrorWord, 0004h ; Parity error
@@NoPE:
        test    al, LSR_FE
        jz      @@NoFE
        or      [si].pdErrorWord, 0008h ; Framing error
@@NoFE:
        test    al, LSR_BI
        jz      @@NoBI
        or      [si].pdEventWord, 0040h ; Break detected
@@NoBI:
        ;; Set event bit 7: error occurred
        test    al, LSR_OE OR LSR_PE OR LSR_FE
        jz      @@Loop
        or      [si].pdEventWord, 0080h
        jmp     @@Loop

;; ---------- Received Data Available ----------
@@ReceiveData:
        mov     dx, [si].pdIOBase
        add     dx, UART_LSR
        in      al, dx
        test    al, LSR_DR              ; Data ready?
        jz      @@Loop                  ; Spurious — back to IIR check

        ;; Read the byte
        mov     dx, [si].pdIOBase
        in      al, dx

        ;; Check for XON/XOFF if flow control enabled
        push    ax
        mov     ah, [si].pdDCB + DCB_OFS_FLAGS2
        test    ah, DCB_F2_XON_RX       ; Honor received XON/XOFF?
        jz      @@NotFlowChar

        ;; Check if received byte is XOFF
        cmp     al, [si].pdDCB + DCB_OFS_XOFFCH
        jne     @@CheckXon
        mov     [si].pdXoffRecvd, 1      ; Stop transmitting
        or      [si].pdEventWord, 0008h  ; CTS/flow change
        pop     ax
        jmp     @@ReceiveData            ; Check for more data

@@CheckXon:
        cmp     al, [si].pdDCB + DCB_OFS_XONCH
        jne     @@NotFlowChar
        mov     [si].pdXoffRecvd, 0      ; Resume transmitting
        ;; Kick the transmitter
        call    TryTransmit
        pop     ax
        jmp     @@ReceiveData

@@NotFlowChar:
        pop     ax

        ;; Put byte in receive ring buffer
        push    di
        lea     di, [si].pdRxBuf
        call    RingBufPut
        pop     di
        jc      @@RxOverflow

        ;; Set event: received data
        or      [si].pdEventWord, 0001h

        ;; Unblock any waiting reader
        ;; (DevHlp_ProcRun on pdReadWait)
        push    eax
        mov     eax, [si].pdReadWait
        test    eax, eax
        jz      @@NoWakeReader
        mov     dl, DevHlp_ProcRun
        call    [DriverData.ddDevHlp]
@@NoWakeReader:
        pop     eax

        ;; Check for more data in FIFO
        jmp     @@ReceiveData

@@RxOverflow:
        or      [si].pdErrorWord, 0001h ; Buffer overrun
        jmp     @@ReceiveData

;; ---------- Transmitter Holding Register Empty ----------
@@TransmitEmpty:
        call    TryTransmit
        or      [si].pdEventWord, 0004h ; TX buffer space available
        jmp     @@Loop

;; ---------- Modem Status Change ----------
@@ModemStatus:
        mov     dx, [si].pdIOBase
        add     dx, UART_MSR
        in      al, dx
        mov     [si].pdMSR, al

        ;; Set event bits for signal changes
        test    al, MSR_DCTS
        jz      @@NoCTS
        or      [si].pdEventWord, 0008h ; CTS changed
@@NoCTS:
        test    al, MSR_DDSR
        jz      @@NoDSR
        or      [si].pdEventWord, 0010h ; DSR changed
@@NoDSR:
        test    al, MSR_DDCD
        jz      @@NoDCD
        or      [si].pdEventWord, 0020h ; DCD changed
@@NoDCD:
        test    al, MSR_TERI
        jz      @@NoRI
        or      [si].pdEventWord, 0100h ; RI trailing edge
@@NoRI:
        ;; If CTS came on and we were waiting, try to transmit
        test    al, MSR_CTS
        jz      @@Loop
        call    TryTransmit
        jmp     @@Loop

;; ---------- Clear all status to dismiss unknown interrupt ----------
@@ClearAll:
        mov     dx, [si].pdIOBase
        add     dx, UART_LSR
        in      al, dx
        mov     dx, [si].pdIOBase
        add     dx, UART_MSR
        in      al, dx
        mov     dx, [si].pdIOBase
        in      al, dx
        jmp     @@Loop

@@Done:
        ret


;; ====================================================================
;; TryTransmit — Load bytes from TX buffer into UART
;; ====================================================================
;; In:  DS:SI → PORTDATA
;; Trashes: AX, DX
;; ====================================================================

TryTransmit:
        ;; Check if transmitter is held
        cmp     [si].pdTxHold, 0
        jne     @@Blocked
        cmp     [si].pdXoffRecvd, 0
        jne     @@Blocked

        ;; Check if break is active
        test    [si].pdFlags, PF_BREAKON
        jnz     @@Blocked

        ;; Check CTS if CTS flow control enabled
        mov     al, [si].pdDCB + DCB_OFS_FLAGS1
        test    al, DCB_F1_CTS_ENA
        jz      @@CTSOk
        test    [si].pdMSR, MSR_CTS
        jz      @@Blocked               ; CTS is off, can't send
@@CTSOk:

        ;; Check for transmit immediate byte first
        cmp     [si].pdTxImm, 0
        je      @@NoImmediate
        mov     [si].pdTxImm, 0
        mov     al, [si].pdTxImmChar
        mov     dx, [si].pdIOBase
        out     dx, al
        ret

@@NoImmediate:
        ;; Check if XON needs to be sent
        cmp     [si].pdXoffSent, 2      ; 2 = need to send XON
        jne     @@NoXonPending
        mov     al, [si].pdDCB + DCB_OFS_XONCH
        mov     dx, [si].pdIOBase
        out     dx, al
        mov     [si].pdXoffSent, 0
        ret

@@NoXonPending:
        ;; Load bytes from TX ring buffer
        ;; On 16550A with FIFO, we can load up to 16 bytes at once
        mov     cx, 1                   ; Default: 1 byte
        test    [si].pdFlags, PF_FIFOENABLED
        jz      @@LoadLoop
        mov     cx, 16                  ; FIFO: load up to 16

@@LoadLoop:
        push    di
        lea     di, [si].pdTxBuf
        call    RingBufGet
        pop     di
        jc      @@TxEmpty               ; Buffer empty

        ;; Write byte to THR
        mov     dx, [si].pdIOBase
        out     dx, al

        dec     cx
        jnz     @@LoadLoop

        ;; Unblock any writer waiting for buffer space
        push    eax
        mov     eax, [si].pdWriteWait
        test    eax, eax
        jz      @@NoWakeWriter
        mov     dl, DevHlp_ProcRun
        call    [DriverData.ddDevHlp]
@@NoWakeWriter:
        pop     eax

@@Blocked:
@@TxEmpty:
        ret
