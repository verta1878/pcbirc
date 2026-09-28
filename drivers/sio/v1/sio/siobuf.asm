;; ====================================================================
;; siobuf.asm — Ring buffer operations
;; ====================================================================
;; Lock-free single-producer single-consumer ring buffer.
;; IMPORTANT: rbBase must contain a GDT selector:offset (virtual address),
;; NOT a physical address. During INIT, AllocPhys returns a physical addr
;; which must be converted via DevHlp_PhysToGDTSelector before storing
;; in rbBase. See AllocateBuffers in sioinit.asm.
;; ISR is the producer for RX, consumer for TX.
;; Strategy routines are the consumer for RX, producer for TX.
;; CLI/STI used at boundaries for safety on shared-IRQ systems.
;; ====================================================================

;; ====================================================================
;; RingBufInit — Initialize a ring buffer
;; ====================================================================
;; In:  DS:DI → RINGBUF structure
;;      CX = buffer size
;;      EAX = physical address of buffer memory
;; Out: None
;; ====================================================================

RingBufInit:
        mov     [di].rbHead, 0
        mov     [di].rbTail, 0
        mov     [di].rbCount, 0
        mov     [di].rbSize, cx
        mov     [di].rbBase, eax
        ret


;; ====================================================================
;; RingBufPut — Add a byte to the ring buffer
;; ====================================================================
;; In:  DS:DI → RINGBUF structure
;;      AL = byte to add
;; Out: CF set if buffer full (byte NOT added)
;; Trashes: BX
;; ====================================================================

RingBufPut:
        push    es
        push    cx

        ;; Check if full
        mov     cx, [di].rbCount
        cmp     cx, [di].rbSize
        jae     @@Full

        ;; Get virtual pointer to buffer (set up during INIT via PhysToVirt)
        les     bx, [di].rbBase
        add     bx, [di].rbTail
        mov     es:[bx], al

        ;; Advance tail
        inc     WORD PTR [di].rbTail
        mov     cx, [di].rbTail
        cmp     cx, [di].rbSize
        jb      @@NoWrap
        mov     WORD PTR [di].rbTail, 0
@@NoWrap:
        inc     WORD PTR [di].rbCount
        pop     cx
        pop     es
        clc
        ret

@@Full:
        pop     cx
        pop     es
        stc
        ret


;; ====================================================================
;; RingBufGet — Remove a byte from the ring buffer
;; ====================================================================
;; In:  DS:DI → RINGBUF structure
;; Out: AL = byte read
;;      CF set if buffer empty
;; Trashes: BX
;; ====================================================================

RingBufGet:
        push    es
        push    cx

        ;; Check if empty
        mov     cx, [di].rbCount
        test    cx, cx
        jz      @@Empty

        ;; Get byte from head
        les     bx, [di].rbBase
        add     bx, [di].rbHead
        mov     al, es:[bx]

        ;; Advance head
        inc     WORD PTR [di].rbHead
        mov     cx, [di].rbHead
        cmp     cx, [di].rbSize
        jb      @@NoWrap
        mov     WORD PTR [di].rbHead, 0
@@NoWrap:
        dec     WORD PTR [di].rbCount
        pop     cx
        pop     es
        clc
        ret

@@Empty:
        pop     cx
        pop     es
        stc
        ret


;; ====================================================================
;; RingBufPeek — Read byte without removing
;; ====================================================================
;; In:  DS:DI → RINGBUF
;; Out: AL = byte at head, CF set if empty
;; ====================================================================

RingBufPeek:
        push    es
        push    bx

        cmp     WORD PTR [di].rbCount, 0
        je      @@Empty

        les     bx, [di].rbBase
        add     bx, [di].rbHead
        mov     al, es:[bx]
        pop     bx
        pop     es
        clc
        ret

@@Empty:
        pop     bx
        pop     es
        stc
        ret


;; ====================================================================
;; RingBufFlush — Empty the buffer
;; ====================================================================
;; In:  DS:DI → RINGBUF
;; ====================================================================

RingBufFlush:
        pushf
        cli
        mov     WORD PTR [di].rbHead, 0
        mov     WORD PTR [di].rbTail, 0
        mov     WORD PTR [di].rbCount, 0
        popf
        ret


;; ====================================================================
;; RingBufCount — Return number of bytes in buffer
;; ====================================================================
;; In:  DS:DI → RINGBUF
;; Out: CX = byte count
;; ====================================================================

RingBufCount:
        mov     cx, [di].rbCount
        ret


;; ====================================================================
;; RingBufFree — Return free space in buffer
;; ====================================================================
;; In:  DS:DI → RINGBUF
;; Out: CX = free bytes
;; ====================================================================

RingBufFree:
        mov     cx, [di].rbSize
        sub     cx, [di].rbCount
        ret
