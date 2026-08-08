;; ====================================================================
;; siostatus.asm — Input/Output Status and Flush
;; ====================================================================

;; ====================================================================
;; CmdInputStatus — Return busy if no data in RX buffer
;; ====================================================================

CmdInputStatus:
        call    GetPortData
        jc      @@ISErr

        push    di
        lea     di, [si].pdRxBuf
        call    RingBufCount            ; CX = bytes available
        pop     di

        test    cx, cx
        jnz     @@ISReady

        ;; No data — return BUSY (process may block on read)
        mov     ax, 0200h               ; STATUS_BUSY
        jmp     StratExit

@@ISReady:
        xor     ax, ax                  ; STATUS_OK (data available)
        jmp     StratExit

@@ISErr:
        mov     ax, 8106h
        jmp     StratExit


;; ====================================================================
;; CmdInputFlush — Clear receive buffer, unblock waiting readers
;; ====================================================================

CmdInputFlush:
        call    GetPortData
        jc      @@IFErr

        ;; Flush RX ring buffer
        push    di
        lea     di, [si].pdRxBuf
        call    RingBufFlush
        pop     di

        ;; Unblock any threads blocked on read
        push    eax
        mov     eax, [si].pdReadWait
        test    eax, eax
        jz      @@IFNoWake
        mov     dl, DevHlp_ProcRun
        call    [DriverData.ddDevHlp]
@@IFNoWake:
        pop     eax

        xor     ax, ax
        jmp     StratExit

@@IFErr:
        mov     ax, 8106h
        jmp     StratExit


;; ====================================================================
;; CmdOutputStatus — Return busy if TX buffer > 75% full
;; ====================================================================

CmdOutputStatus:
        call    GetPortData
        jc      @@OSErr

        push    di
        lea     di, [si].pdTxBuf

        ;; Get count and size
        mov     cx, [di].rbCount
        mov     dx, [di].rbSize
        pop     di

        ;; Calculate 75% threshold: size * 3 / 4
        push    ax
        mov     ax, dx
        shr     ax, 2                   ; size / 4
        mov     dx, ax
        shl     ax, 1                   ; size / 4 * 2 = size / 2
        add     dx, ax                  ; size / 4 * 3 = 75%
        pop     ax

        cmp     cx, dx
        jae     @@OSBusy

        xor     ax, ax                  ; Buffer has space
        jmp     StratExit

@@OSBusy:
        mov     ax, 0200h               ; STATUS_BUSY
        jmp     StratExit

@@OSErr:
        mov     ax, 8106h
        jmp     StratExit


;; ====================================================================
;; CmdOutputFlush — Clear transmit buffer, unblock waiting writers
;; ====================================================================

CmdOutputFlush:
        call    GetPortData
        jc      @@OFErr

        ;; Flush TX ring buffer
        push    di
        lea     di, [si].pdTxBuf
        call    RingBufFlush
        pop     di

        ;; Unblock any threads blocked on write
        push    eax
        mov     eax, [si].pdWriteWait
        test    eax, eax
        jz      @@OFNoWake
        mov     dl, DevHlp_ProcRun
        call    [DriverData.ddDevHlp]
@@OFNoWake:
        pop     eax

        xor     ax, ax
        jmp     StratExit

@@OFErr:
        mov     ax, 8106h
        jmp     StratExit
