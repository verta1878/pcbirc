;; ====================================================================
;; sioio.asm — Read, Write, and Non-Destructive Read
;; ====================================================================
;; Key SIO behavior difference from COM.SYS:
;;   Write returns immediately if all data fits in the TX buffer.
;;   COM.SYS blocks until the write completes at the hardware.
;; ====================================================================

;; ====================================================================
;; CmdRead — Read data from receive buffer
;; ====================================================================
;; Request packet fields:
;;   0Eh: DWORD — transfer address (caller's buffer)
;;   12h: WORD  — requested byte count
;;   12h: WORD  — actual bytes transferred (output)
;; ====================================================================

CmdRead:
        call    GetPortData
        jc      @@ReadErr

        test    [si].pdFlags, PF_OPEN
        jz      @@ReadErr

        les     bx, DWORD PTR [ReqPktOff]

        ;; Get transfer address (physical) and count
        ;; OS/2 PDD request packets store physical addresses for buffers.
        ;; Use DevHlp_PhysToVirt to get a usable pointer.
        mov     eax, es:[bx+0Eh]        ; EAX = physical transfer address
        mov     cx, es:[bx+12h]         ; CX = requested count
        test    cx, cx
        jz      @@ReadZero

        ;; Map physical address to virtual for our access
        ;; DevHlp_PhysToVirt: EAX=phys, ECX=length, DH=direction(0=result in DS:SI)
        push    cx
        movzx   ecx, cx
        mov     dh, 0                   ; Result in DS:SI
        mov     dl, DevHlp_PhysToVirt
        call    [DriverData.ddDevHlp]
        pop     cx
        jc      @@ReadErr               ; Mapping failed

        ;; DS:SI now points to caller's buffer
        ;; Save mapped pointer
        mov     WORD PTR [XferBufOff], si
        mov     WORD PTR [XferBufSeg], ds

        ;; Restore our DS
        push    cs
        pop     ds                      ; Restore DS (may have been changed)
        ;; Actually for PDD, DS is already our data seg. Reload it.
        mov     ax, SEG _DATA
        mov     ds, ax

        ;; Determine read behavior from DCB Flags3
        mov     al, [si].pdDCB.dcbFlags3
        and     al, DCB_F3_RDTO_MASK

        cmp     al, DCB_F3_RDTO_NOW
        je      @@ReadNoWait

        cmp     al, DCB_F3_RDTO_WAIT
        je      @@ReadWaitSome

        ;; Default: Normal read timeout
        jmp     @@ReadNormal

;; ---------- No Wait Mode (11) ----------
;; Return immediately with whatever is available

@@ReadNoWait:
        xor     dx, dx                  ; Bytes transferred count
        push    di
        push    es

@@NWLoop:
        test    cx, cx
        jz      @@NWDone
        push    di
        push    cx
        lea     di, [si].pdRxBuf
        call    RingBufGet
        pop     cx
        pop     di
        jc      @@NWDone                ; Buffer empty

        ;; Store byte in caller's mapped buffer
        push    es
        push    di
        les     di, DWORD PTR [XferBufOff]
        add     di, dx                  ; Offset by bytes already read
        mov     es:[di], al
        pop     di
        pop     es
        inc     dx
        dec     cx
        jmp     @@NWLoop

@@NWDone:
        pop     es
        pop     di
        ;; Store actual count
        les     bx, DWORD PTR [ReqPktOff]
        mov     es:[bx+12h], dx
        xor     ax, ax
        jmp     StratExit

;; ---------- Wait For Something Mode (10) ----------
;; Wait until at least one byte is available, then return what's there

@@ReadWaitSome:
        push    di
        push    cx
        lea     di, [si].pdRxBuf
        call    RingBufCount
        pop     cx
        pop     di
        test    cx, cx
        jnz     @@ReadNoWait            ; Something available, grab it

        ;; Block until data arrives or timeout
        call    BlockForRead
        jc      @@ReadTimeout
        jmp     @@ReadWaitSome

;; ---------- Normal Read Timeout Mode (01) ----------
;; Wait for full count or timeout

@@ReadNormal:
        xor     dx, dx                  ; Bytes transferred

@@NormLoop:
        test    cx, cx
        jz      @@NormDone

        ;; Try to get a byte
        push    di
        push    cx
        lea     di, [si].pdRxBuf
        call    RingBufGet
        pop     cx
        pop     di
        jnc     @@NormGotByte

        ;; Buffer empty — block and wait
        push    cx
        push    dx
        call    BlockForRead
        pop     dx
        pop     cx
        jc      @@NormTimeout           ; Timeout expired
        jmp     @@NormLoop

@@NormGotByte:
        ;; Store byte in caller's mapped buffer
        push    es
        push    di
        les     di, DWORD PTR [XferBufOff]
        add     di, dx                  ; Offset by bytes transferred so far
        mov     es:[di], al
        pop     di
        pop     es
        inc     dx
        dec     cx
        jmp     @@NormLoop

@@NormDone:
@@NormTimeout:
        les     bx, DWORD PTR [ReqPktOff]
        mov     es:[bx+12h], dx
        xor     ax, ax
        ;; Set event bit 1 if timeout occurred
        jmp     StratExit

@@ReadTimeout:
        or      [si].pdEventWord, 0002h  ; Read timeout event
        les     bx, DWORD PTR [ReqPktOff]
        mov     WORD PTR es:[bx+12h], 0
        xor     ax, ax
        jmp     StratExit

@@ReadZero:
        xor     ax, ax
        jmp     StratExit

@@ReadErr:
        mov     ax, 8106h
        jmp     StratExit


;; ====================================================================
;; CmdWrite — Write data to transmit buffer
;; ====================================================================
;; SIO behavior: returns immediately if all data fits in TX buffer.
;; Only blocks if the buffer is full.
;; ====================================================================

CmdWrite:
        call    GetPortData
        jc      @@WriteErr

        test    [si].pdFlags, PF_OPEN
        jz      @@WriteErr

        les     bx, DWORD PTR [ReqPktOff]
        mov     cx, es:[bx+12h]         ; CX = byte count
        test    cx, cx
        jz      @@WriteZero

        ;; Map physical transfer address to virtual
        mov     eax, es:[bx+0Eh]        ; Physical address
        push    cx
        movzx   ecx, cx
        mov     dh, 0                   ; DS:SI result
        mov     dl, DevHlp_PhysToVirt
        call    [DriverData.ddDevHlp]
        pop     cx
        jc      @@WriteErr
        mov     WORD PTR [XferBufOff], si
        mov     WORD PTR [XferBufSeg], ds
        mov     ax, SEG _DATA
        mov     ds, ax

        xor     dx, dx                  ; Bytes transferred

@@WriteLoop:
        test    cx, cx
        jz      @@WriteDone

        ;; Map caller's write buffer (first time through)
        ;; The physical address mapping was done at CmdWrite entry
        ;; Read byte from caller's buffer at current offset
        push    es
        push    di
        les     di, DWORD PTR [XferBufOff]
        add     di, dx                  ; Offset by bytes written so far
        mov     al, es:[di]
        pop     di
        pop     es

        push    di
        lea     di, [si].pdTxBuf
        call    RingBufPut
        pop     di
        jnc     @@WroteByte

        ;; Buffer full — kick transmitter and block
        call    TryTransmit

        ;; Check for infinite write timeout
        test    [si].pdDCB.dcbFlags3, DCB_F3_WTOINF
        jnz     @@WriteBlockInf

        ;; Block with timeout
        push    cx
        push    dx
        call    BlockForWrite
        pop     dx
        pop     cx
        jc      @@WriteTimeout
        jmp     @@WriteLoop

@@WriteBlockInf:
        push    cx
        push    dx
        call    BlockForWriteInfinite
        pop     dx
        pop     cx
        jmp     @@WriteLoop

@@WroteByte:
        inc     dx
        dec     cx
        jmp     @@WriteLoop

@@WriteDone:
        ;; Kick transmitter to start sending
        call    TryTransmit

        les     bx, DWORD PTR [ReqPktOff]
        mov     es:[bx+12h], dx
        xor     ax, ax
        jmp     StratExit

@@WriteTimeout:
        les     bx, DWORD PTR [ReqPktOff]
        mov     es:[bx+12h], dx
        xor     ax, ax
        jmp     StratExit

@@WriteZero:
        xor     ax, ax
        jmp     StratExit

@@WriteErr:
        mov     ax, 8106h
        jmp     StratExit


;; ====================================================================
;; CmdNDRead — Non-Destructive Read (peek)
;; ====================================================================
;; Returns one byte without removing it from the buffer.
;; Sets BUSY if no data available.
;; ====================================================================

CmdNDRead:
        call    GetPortData
        jc      @@NDErr

        push    di
        lea     di, [si].pdRxBuf
        call    RingBufPeek
        pop     di
        jc      @@NDEmpty

        ;; Store byte in request packet byte field
        les     bx, DWORD PTR [ReqPktOff]
        mov     es:[bx+0Dh], al         ; NDR byte field
        xor     ax, ax
        jmp     StratExit

@@NDEmpty:
        mov     ax, 0200h               ; STATUS_BUSY (no data)
        jmp     StratExit

@@NDErr:
        mov     ax, 8106h
        jmp     StratExit


;; ====================================================================
;; BlockForRead — Block current thread until RX data or timeout
;; ====================================================================
;; In:  DS:SI → PORTDATA
;; Out: CF set if timeout
;; ====================================================================

BlockForRead:
        ;; Calculate timeout in milliseconds from DCB
        ;; DCB read timeout is in 1/100 sec, 0-based (0 = 10ms)
        movzx   eax, [si].pdDCB.dcbReadTimeout
        inc     eax                     ; 0-based → 1-based
        imul    eax, 10                 ; Convert to milliseconds
        mov     ecx, eax

        ;; Set block ID
        lea     eax, [si].pdReadWait
        mov     [si].pdReadWait, eax

        ;; DevHlp_ProcBlock
        mov     dh, 1                   ; Interruptible
        mov     dl, DevHlp_ProcBlock
        call    [DriverData.ddDevHlp]
        ;; Returns: CF set if timeout, AX = wake reason
        ret


;; ====================================================================
;; BlockForWrite — Block until TX buffer space or timeout
;; ====================================================================

BlockForWrite:
        movzx   eax, [si].pdDCB.dcbWriteTimeout
        inc     eax
        imul    eax, 10
        mov     ecx, eax

        lea     eax, [si].pdWriteWait
        mov     [si].pdWriteWait, eax

        mov     dh, 1
        mov     dl, DevHlp_ProcBlock
        call    [DriverData.ddDevHlp]
        ret


;; ====================================================================
;; BlockForWriteInfinite — Block until TX buffer space, no timeout
;; ====================================================================

BlockForWriteInfinite:
        lea     eax, [si].pdWriteWait
        mov     [si].pdWriteWait, eax

        mov     ecx, -1                 ; Infinite timeout
        mov     dh, 1
        mov     dl, DevHlp_ProcBlock
        call    [DriverData.ddDevHlp]
        ret

;; Transfer buffer mapped pointer (set by CmdRead/CmdWrite via PhysToVirt)
XferBufOff      DW      0
XferBufSeg      DW      0
