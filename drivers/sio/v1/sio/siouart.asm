;; ====================================================================
;; siouart.asm — UART detection and hardware abstraction
;; ====================================================================

;; ====================================================================
;; DetectUART — Identify the UART chip at a given I/O port
;; ====================================================================
;; In:  DX = base I/O port address
;; Out: AL = UART type (UART_xxx constant)
;;      CF set if no UART found
;; Trashes: AH, CX
;; ====================================================================

DetectUART:
        push    dx
        push    bx

        ;; Step 1: Check if anything is there at all.
        ;; Write a known value to the scratch register and read it back.
        ;; If nothing responds, I/O reads return FFh.
        mov     bx, dx                  ; Save base port
        add     dx, UART_SCR            ; Point to scratch register
        mov     al, 055h
        out     dx, al
        jmp     $+2                     ; I/O delay
        in      al, dx
        cmp     al, 055h
        jne     @@Try8250               ; No scratch reg → might be 8250

        ;; Write another pattern to confirm
        mov     al, 0AAh
        out     dx, al
        jmp     $+2
        in      al, dx
        cmp     al, 0AAh
        jne     @@Try8250

        ;; Scratch register works → at least 8250A or better.
        ;; Step 2: Try enabling FIFOs to distinguish 16450 from 16550x
        mov     dx, bx
        add     dx, UART_FCR            ; FIFO Control Register
        mov     al, FCR_ENABLE OR FCR_RXRESET OR FCR_TXRESET OR FCR_TRIG_14
        out     dx, al
        jmp     $+2

        ;; Read IIR to check FIFO status bits
        mov     dx, bx
        add     dx, UART_IIR
        in      al, dx
        mov     ah, al                  ; Save IIR
        and     al, IIR_FIFO_MASK

        ;; Disable FIFOs again for clean detection
        mov     dx, bx
        add     dx, UART_FCR
        push    ax
        xor     al, al
        out     dx, al
        pop     ax

        cmp     al, IIR_FIFO_OK         ; Both FIFO bits set?
        je      @@Got16550A

        cmp     al, IIR_FIFO_BAD         ; Only bit 7 set?
        je      @@Got16550NonA

        ;; No FIFO capability → 16450 or 8250A
        ;; Distinguish by speed (we can't easily in software)
        ;; Convention: if scratch register works, call it 16450
        mov     al, UART_16450
        jmp     @@Done

@@Got16550NonA:
        ;; 16550 with broken FIFO — treat as 16450
        mov     al, UART_16550
        jmp     @@Done

@@Got16550A:
        ;; Working FIFO. Check if it supports high baud rates.
        ;; Newer 16550As can do 230400/460800.
        ;; Test: set divisor to 1 (115200), then try divisor for 230400
        ;; by checking if the UART accepts a non-standard divisor latch.
        ;; For now, report standard 16550A; high-speed detection
        ;; will be added when we have real hardware to test.
        mov     al, UART_16550A
        jmp     @@Done

@@Try8250:
        ;; No scratch register. Could be original 8250 or nothing.
        ;; Try writing to IER and reading back to confirm UART exists.
        mov     dx, bx
        add     dx, UART_IER
        xor     al, al                  ; Disable all interrupts
        out     dx, al
        jmp     $+2
        in      al, dx
        and     al, 0Fh                 ; Mask to valid IER bits
        test    al, al
        jnz     @@NoUART                ; Should be 0 after writing 0

        ;; Write a pattern
        mov     al, IER_RDA OR IER_THRE
        out     dx, al
        jmp     $+2
        in      al, dx
        and     al, 0Fh
        cmp     al, IER_RDA OR IER_THRE
        jne     @@NoUART

        ;; Restore IER to 0
        xor     al, al
        out     dx, al

        ;; IER responds → it's an 8250 (no scratch register)
        mov     al, UART_8250
        jmp     @@Done

@@NoUART:
        mov     al, UART_NONE
        stc
        jmp     @@Exit

@@Done:
        clc

@@Exit:
        pop     bx
        pop     dx
        ret


;; ====================================================================
;; InitUART — Initialize UART hardware for a port
;; ====================================================================
;; In:  DS:SI → PORTDATA for this port
;; Out: None
;; Trashes: AX, DX
;; ====================================================================

InitUART:
        mov     dx, [si].pdIOBase

        ;; Disable all interrupts
        add     dx, UART_IER
        xor     al, al
        out     dx, al

        ;; Read LSR and MSR to clear any pending conditions
        mov     dx, [si].pdIOBase
        add     dx, UART_LSR
        in      al, dx

        mov     dx, [si].pdIOBase
        add     dx, UART_MSR
        in      al, dx
        mov     [si].pdMSR, al

        ;; Read RBR to clear any pending data
        mov     dx, [si].pdIOBase
        in      al, dx

        ;; Set default line control: 8N1
        mov     dx, [si].pdIOBase
        add     dx, UART_LCR
        mov     al, LCR_WLS_8           ; 8 data bits, no parity, 1 stop
        out     dx, al
        mov     [si].pdLineCtrl, al

        ;; Set default baud rate: 9600
        call    SetBaudRate9600

        ;; Enable FIFOs if 16550A or better
        cmp     [si].pdUARTType, UART_16550A
        jb      @@NoFIFO

        mov     dx, [si].pdIOBase
        add     dx, UART_FCR
        mov     al, FCR_ENABLE OR FCR_RXRESET OR FCR_TXRESET OR FCR_TRIG_8
        out     dx, al
        or      [si].pdFlags, PF_FIFOENABLED
        jmp     @@MCR

@@NoFIFO:
        and     [si].pdFlags, NOT PF_FIFOENABLED

@@MCR:
        ;; Set MCR: DTR off, RTS off, OUT2 on (enables IRQ on PC)
        mov     dx, [si].pdIOBase
        add     dx, UART_MCR
        mov     al, MCR_OUT2
        out     dx, al
        mov     [si].pdMCR, al

        ret


;; ====================================================================
;; SetBaudRate — Set UART divisor from a baud rate value
;; ====================================================================
;; In:  DS:SI → PORTDATA
;;      EAX = desired baud rate
;; Out: CF set if invalid (baud < 50)
;; Trashes: EAX, ECX, EDX
;; ====================================================================

SetBaudRate:
        ;; Check minimum
        cmp     eax, 50
        jb      @@Invalid

        ;; Check for locked baud
        cmp     [si].pdBaudLocked, 0
        jne     @@Locked

        ;; Calculate divisor: 115200 / baud
        mov     [si].pdBaudRate, eax
        push    eax
        mov     eax, BAUD_CLOCK
        xor     edx, edx
        pop     ecx
        div     ecx                     ; EAX = divisor
        test    eax, eax
        jz      @@Invalid               ; Divisor can't be 0
        mov     [si].pdBaudDiv, ax

        ;; Program the UART divisor latch
        call    ProgramDivisor
        clc
        ret

@@Locked:
        clc                             ; Silently ignore when locked
        ret

@@Invalid:
        stc
        ret


;; ====================================================================
;; SetBaudRate9600 — Convenience: set 9600 baud
;; ====================================================================

SetBaudRate9600:
        mov     eax, 9600
        call    SetBaudRate
        ret


;; ====================================================================
;; ProgramDivisor — Write divisor to UART divisor latch
;; ====================================================================
;; In:  DS:SI → PORTDATA (pdIOBase, pdBaudDiv set)
;; Trashes: AX, DX
;; ====================================================================

ProgramDivisor:
        pushf
        cli                             ; Protect LCR DLAB manipulation

        ;; Set DLAB to access divisor latch
        mov     dx, [si].pdIOBase
        add     dx, UART_LCR
        in      al, dx
        push    ax                      ; Save current LCR
        or      al, LCR_DLAB
        out     dx, al

        ;; Write divisor low byte
        mov     dx, [si].pdIOBase
        add     dx, UART_DLL
        mov     ax, [si].pdBaudDiv
        out     dx, al

        ;; Write divisor high byte
        inc     dx                      ; UART_DLH
        mov     al, ah
        out     dx, al

        ;; Restore LCR (clears DLAB)
        mov     dx, [si].pdIOBase
        add     dx, UART_LCR
        pop     ax
        out     dx, al

        popf
        ret


;; ====================================================================
;; EnableUARTInterrupts — Turn on UART IRQ sources
;; ====================================================================
;; In:  DS:SI → PORTDATA
;; Trashes: AX, DX
;; ====================================================================

EnableUARTInterrupts:
        mov     dx, [si].pdIOBase
        add     dx, UART_IER
        mov     al, IER_RDA OR IER_THRE OR IER_RLS OR IER_MS
        out     dx, al
        ret


;; ====================================================================
;; DisableUARTInterrupts — Turn off all UART IRQ sources
;; ====================================================================

DisableUARTInterrupts:
        mov     dx, [si].pdIOBase
        add     dx, UART_IER
        xor     al, al
        out     dx, al
        ret


;; ====================================================================
;; SetDTR / ClearDTR / SetRTS / ClearRTS — Modem signal control
;; ====================================================================
;; In:  DS:SI → PORTDATA
;; ====================================================================

SetDTR :
        or      [si].pdMCR, MCR_DTR
        jmp     WriteMCR

ClearDTR:
        and     [si].pdMCR, NOT MCR_DTR
        jmp     WriteMCR

SetRTS :
        or      [si].pdMCR, MCR_RTS
        jmp     WriteMCR

ClearRTS:
        and     [si].pdMCR, NOT MCR_RTS
        jmp     WriteMCR

WriteMCR:
        mov     dx, [si].pdIOBase
        add     dx, UART_MCR
        mov     al, [si].pdMCR
        out     dx, al
        ret
