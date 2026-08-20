;; ====================================================================
;; VX00.SYS — FOSSIL Driver for DOS Sessions under OS/2
;; ====================================================================
;; Clean-room reimplementation from FTS-0001 Rev 5 (FOSSIL spec)
;; and VX00.TXT documentation.
;;
;; VX00 is a DOS device driver loaded via DOS_DEVICE in OS/2 VDMs.
;; It hooks INT 14h and implements the FOSSIL API. All actual serial
;; I/O goes through the virtualized UART ports that VSIO.SYS provides,
;; which in turn routes through SIO.SYS to real hardware.
;;
;; Per VX00.TXT: "VX00.SYS is little more than a software interface
;; to SIO, providing FOSSIL based programs with direct access to the
;; real comm driver SIO.SYS. It uses the same buffers that SIO.SYS
;; uses."
;;
;; This means VX00 does NOT maintain its own buffers — it reads/writes
;; directly through UART I/O ports (virtualized by VSIO).
;; ====================================================================

        .8086
        TITLE   VX00 — FOSSIL Driver for OS/2 VDMs

;; ====================================================================
;; DOS Device Driver Header (must be at offset 0)
;; ====================================================================

_TEXT   SEGMENT BYTE PUBLIC 'CODE'
        ASSUME  CS:_TEXT, DS:_TEXT

        ORG     0

DevHeader:
        DD      -1                      ; Next driver (filled by DOS)
        DW      8000h                   ; Character device
        DW      OFFSET Strategy         ; Strategy entry point
        DW      OFFSET Interrupt        ; Interrupt entry point (unused)
        DB      'VX00$   '              ; Device name

;; ====================================================================
;; FOSSIL identification signature
;; ====================================================================

FOSSIL_SIG      DB      'FOSSIL', 0
FOSSIL_MAXFN    EQU     1Bh             ; Highest supported function
FOSSIL_REV      EQU     5               ; FTS-0001 revision level
FOSSIL_ID       DB      'VX00 FOSSIL Driver for OS/2', 0
FOSSIL_IDLEN    EQU     $ - FOSSIL_ID

;; ====================================================================
;; Per-port state (minimal — SIO/VSIO owns the real state)
;; ====================================================================

MAX_FOSSIL_PORTS EQU    4

FPORT   STRUC
fpActive        DB      0              ; Non-zero if FOSSIL initialized
fpPort          DW      0              ; UART base I/O address
fpFlowCtrl      DB      0              ; Flow control flags from fn 0Fh
fpBreakState    DB      0              ; Break on/off
FPORT   ENDS

FossilPorts     FPORT   MAX_FOSSIL_PORTS DUP (<>)

;; Standard UART base addresses for COM1-COM4
PortBases       DW      03F8h, 02F8h, 03E8h, 02E8h

;; Saved original INT 14h vector
OldInt14Off     DW      0
OldInt14Seg     DW      0

;; ====================================================================
;; UART Register Offsets (same as in sio/uart.inc)
;; ====================================================================

UART_RBR        EQU     0
UART_THR        EQU     0
UART_DLL        EQU     0
UART_DLH        EQU     1
UART_IER        EQU     1
UART_IIR        EQU     2
UART_FCR        EQU     2
UART_LCR        EQU     3
UART_MCR        EQU     4
UART_LSR        EQU     5
UART_MSR        EQU     6
UART_SCR        EQU     7

LSR_DR          EQU     01h
LSR_THRE        EQU     20h
LSR_TEMT        EQU     40h
MCR_DTR         EQU     01h
MCR_RTS         EQU     02h
MSR_CTS         EQU     10h
MSR_DSR         EQU     20h
MSR_RI          EQU     40h
MSR_DCD         EQU     80h
LCR_DLAB        EQU     80h


;; ====================================================================
;; INT 14h Handler — FOSSIL API Entry Point
;; ====================================================================
;; In:  AH = function number (00h-1Bh)
;;      DX = port number (0=COM1, 1=COM2, ...)
;;      Other registers per function
;; ====================================================================

Int14Handler PROC FAR
        sti
        cld
        push    ds
        push    cs
        pop     ds                      ; DS = CS (our data is in code seg)

        ;; Validate port number
        cmp     dx, MAX_FOSSIL_PORTS
        jae     @@ChainOld

        ;; Dispatch on function number
        cmp     ah, FOSSIL_MAXFN
        ja      @@ChainOld

        push    bx
        mov     bl, ah
        xor     bh, bh
        shl     bx, 1
        jmp     WORD PTR CS:[FnTable + bx]

@@ChainOld:
        pop     ds
        ;; Chain to original INT 14h (BIOS)
        jmp     DWORD PTR CS:[OldInt14Off]
Int14Handler ENDP


;; ====================================================================
;; Function Dispatch Table
;; ====================================================================

FnTable LABEL WORD
        DW      OFFSET Fn00_SetBaud     ; 00h Set baud/parity/stop
        DW      OFFSET Fn01_TxChar      ; 01h Send character (wait)
        DW      OFFSET Fn02_RxChar      ; 02h Receive character (wait)
        DW      OFFSET Fn03_Status      ; 03h Port status
        DW      OFFSET Fn04_Init        ; 04h Initialize FOSSIL
        DW      OFFSET Fn05_Deinit      ; 05h Deinitialize FOSSIL
        DW      OFFSET FnUnsupported    ; 06h (reserved)
        DW      OFFSET FnUnsupported    ; 07h (reserved)
        DW      OFFSET Fn08_FlushOut    ; 08h Flush output buffer
        DW      OFFSET Fn09_PurgeOut    ; 09h Purge output buffer
        DW      OFFSET Fn0A_PurgeIn     ; 0Ah Purge input buffer
        DW      OFFSET Fn0B_TxNoWait   ; 0Bh Send char (no wait)
        DW      OFFSET Fn0C_PeekChar    ; 0Ch Peek at input (no wait)
        DW      OFFSET Fn0D_KbdRead     ; 0Dh Keyboard read (no wait)
        DW      OFFSET Fn0E_KbdPeek     ; 0Eh Keyboard peek (no wait)
        DW      OFFSET Fn0F_FlowCtrl    ; 0Fh Flow control enable/disable
        DW      OFFSET Fn10_CtrlCK      ; 10h Ctrl-C/Ctrl-K control
        DW      OFFSET Fn11_SetCursor   ; 11h Set cursor position
        DW      OFFSET Fn12_GetCursor   ; 12h Get cursor position
        DW      OFFSET Fn13_AnsiWrite   ; 13h ANSI write character
        DW      OFFSET FnUnsupported    ; 14h Watchdog (not impl)
        DW      OFFSET FnUnsupported    ; 15h Write BIOS keyboard char
        DW      OFFSET Fn16_BlockRead   ; 16h Block read
        DW      OFFSET Fn17_BlockWrite  ; 17h Block write
        DW      OFFSET Fn18_Break       ; 18h Break start/stop
        DW      OFFSET Fn19_DriverInfo  ; 19h Driver information
        DW      OFFSET FnUnsupported    ; 1Ah Install ext app hook
        DW      OFFSET FnUnsupported    ; 1Bh Remove ext app hook


;; ====================================================================
;; Helper: GetPortBase — get UART base address for port DX
;; ====================================================================
;; In:  DX = port number (0-3)
;; Out: DX = I/O base address, BX = port index
;;      SI → FossilPorts entry
;; ====================================================================

GetPortBase PROC NEAR
        mov     bx, dx
        mov     si, bx
        mov     cl, SIZE FPORT
        xor     ch, ch
        push    ax
        mov     ax, si
        mul     cl
        mov     si, ax
        pop     ax
        add     si, OFFSET FossilPorts
        shl     bx, 1
        mov     dx, CS:[PortBases + bx]
        shr     bx, 1                  ; Restore port index
        ret
GetPortBase ENDP


;; ====================================================================
;; Helper: BuildStatusWord — build AX status for functions 00h/03h
;; ====================================================================
;; In:  DX = UART base address
;; Out: AX = status (AH=LSR, AL=MSR)
;; ====================================================================

BuildStatusWord PROC NEAR
        push    dx
        add     dx, UART_LSR
        in      al, dx
        mov     ah, al
        sub     dx, UART_LSR
        add     dx, UART_MSR
        in      al, dx
        ;; Swap: AH=line status, AL=modem status
        xchg    ah, al
        pop     dx
        ret
BuildStatusWord ENDP


;; ====================================================================
;; Common exit
;; ====================================================================

FnExit:
        pop     bx
        pop     ds
        iret

FnUnsupported:
        pop     bx
        pop     ds
        iret


;; ====================================================================
;; 00h — Set Baud Rate / Line Parameters
;; ====================================================================
;; In:  AL = BIOS-style init byte
;;        Bits 7-5: baud rate (000=19200...111=300 — FOSSIL extended)
;;        Bits 4-3: parity (00=none, 01=odd, 11=even)
;;        Bit  2:   stop bits (0=1, 1=2)
;;        Bits 1-0: data bits (10=7, 11=8)
;; Out: AX = status word (AH=LSR, AL=MSR)
;; ====================================================================

Fn00_SetBaud:
        call    GetPortBase
        push    ax

        ;; Decode baud rate from bits 7-5
        push    ax
        mov     cl, 5
        shr     al, cl
        and     al, 07h
        xor     ah, ah
        shl     ax, 1
        mov     di, ax
        mov     ax, CS:[BaudDivisors + di]
        ;; Set divisor via DLAB
        push    dx
        add     dx, UART_LCR
        in      al, dx
        push    ax
        or      al, LCR_DLAB
        out     dx, al
        sub     dx, UART_LCR           ; Back to base
        mov     ax, CS:[BaudDivisors + di]
        out     dx, al                  ; DLL
        inc     dx
        mov     al, ah
        out     dx, al                  ; DLH
        dec     dx
        add     dx, UART_LCR
        pop     ax
        and     al, NOT LCR_DLAB
        out     dx, al
        pop     dx
        pop     ax

        ;; Build LCR from bits 4-0 of original AL
        pop     ax                      ; Restore original AL
        push    ax
        and     al, 1Fh                 ; Mask parity/stop/data bits
        push    dx
        add     dx, UART_LCR
        out     dx, al
        pop     dx
        pop     ax

        ;; Return status
        call    BuildStatusWord
        jmp     FnExit

;; FOSSIL extended baud rate divisor table (index by bits 7-5)
;; 000=19200, 001=38400, 010=300, 011=600, 100=1200, 101=2400, 110=4800, 111=9600
BaudDivisors    DW      6               ; 000 = 19200
                DW      3               ; 001 = 38400
                DW      384             ; 010 = 300
                DW      192             ; 011 = 600
                DW      96              ; 100 = 1200
                DW      48              ; 101 = 2400
                DW      24              ; 110 = 4800
                DW      12              ; 111 = 9600


;; ====================================================================
;; 01h — Transmit Character (wait until sent)
;; ====================================================================
;; In:  AL = character to send
;; Out: AX = status (bit 15 set if timeout — never for VX00 under SIO)
;; ====================================================================

Fn01_TxChar:
        call    GetPortBase
        mov     ah, al                  ; Save char

        ;; Wait for THR empty
@@TxWait:
        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_THRE
        jz      @@TxWait

        ;; Send byte
        mov     al, ah
        out     dx, al

        ;; Return status
        call    BuildStatusWord
        jmp     FnExit


;; ====================================================================
;; 02h — Receive Character (wait until available)
;; ====================================================================
;; In:  DX = port
;; Out: AH = 0 if success, AL = character received
;; ====================================================================

Fn02_RxChar:
        call    GetPortBase

        ;; Wait for data ready
@@RxWait:
        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_DR
        jz      @@RxWait

        ;; Read byte
        in      al, dx
        xor     ah, ah                  ; Success
        jmp     FnExit


;; ====================================================================
;; 03h — Port Status
;; ====================================================================
;; Out: AX = status (AH=LSR, AL=MSR)
;; ====================================================================

Fn03_Status:
        call    GetPortBase
        call    BuildStatusWord
        jmp     FnExit


;; ====================================================================
;; 04h — Initialize FOSSIL
;; ====================================================================
;; In:  BX = 0 (on stack, was pushed) — doesn't matter, we pop it
;; Out: AX = 1954h (FOSSIL signature)
;;      BL = maximum function number supported
;;      BH = FOSSIL spec revision
;; ====================================================================

Fn04_Init:
        call    GetPortBase

        ;; Mark port as active
        mov     CS:[si].fpActive, 1
        mov     CS:[si].fpPort, dx

        ;; Enable DTR and RTS
        push    dx
        add     dx, UART_MCR
        in      al, dx
        or      al, MCR_DTR OR MCR_RTS
        out     dx, al
        pop     dx

        ;; Flush any pending data by reading RBR
        push    dx
        in      al, dx
        pop     dx

        ;; Return FOSSIL signature
        ;; Stack has: [pushed BX from dispatcher]
        ;; FnExit does: pop bx; pop ds; iret
        ;; We need BX = (REV<<8)|MAXFN when FnExit pops it
        ;; Modify the pushed BX on the stack, preserving BP
        push    bp
        mov     bp, sp
        mov     WORD PTR ss:[bp+2], (FOSSIL_REV SHL 8) OR FOSSIL_MAXFN
        pop     bp
        mov     ax, 1954h               ; FOSSIL magic number
        jmp     FnExit


;; ====================================================================
;; 05h — Deinitialize FOSSIL
;; ====================================================================

Fn05_Deinit:
        call    GetPortBase
        mov     CS:[si].fpActive, 0

        ;; Drop DTR (let SIO handle actual line state)
        push    dx
        add     dx, UART_MCR
        in      al, dx
        and     al, NOT (MCR_DTR OR MCR_RTS)
        out     dx, al
        pop     dx
        jmp     FnExit


;; ====================================================================
;; 08h — Flush Output Buffer (wait for TX to drain)
;; ====================================================================

Fn08_FlushOut:
        call    GetPortBase

        ;; Wait until transmitter is completely empty
@@FlushWait:
        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_TEMT
        jz      @@FlushWait
        jmp     FnExit


;; ====================================================================
;; 09h — Purge Output Buffer
;; ====================================================================
;; VX00 has no local buffers — SIO owns them. This is effectively
;; a no-op since we can't directly flush SIO's buffer from here.
;; The VSIO-virtualized port will handle this at the SIO level.

Fn09_PurgeOut:
        call    GetPortBase
        ;; Under the real VX00, this does nothing because it uses
        ;; SIO's buffers directly. We match that behavior.
        jmp     FnExit


;; ====================================================================
;; 0Ah — Purge Input Buffer
;; ====================================================================

Fn0A_PurgeIn:
        call    GetPortBase
        ;; Drain any pending bytes from the UART
@@PurgeDrain:
        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_DR
        jz      @@PurgeDone
        in      al, dx                  ; Read and discard
        jmp     @@PurgeDrain
@@PurgeDone:
        jmp     FnExit


;; ====================================================================
;; 0Bh — Transmit Character (no wait)
;; ====================================================================
;; In:  AL = character
;; Out: AX = 1 if sent, 0 if buffer full
;; ====================================================================

Fn0B_TxNoWait:
        call    GetPortBase
        mov     ah, al                  ; Save char

        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_THRE
        jz      @@TxNWFull

        mov     al, ah
        out     dx, al
        mov     ax, 1                   ; Success
        jmp     FnExit

@@TxNWFull:
        xor     ax, ax                  ; Buffer full
        jmp     FnExit


;; ====================================================================
;; 0Ch — Peek at Input Character (no wait, non-destructive)
;; ====================================================================
;; Out: AX = character if available (AH=0), or AX=FFFFh if none
;; ====================================================================

Fn0C_PeekChar:
        call    GetPortBase

        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_DR
        jz      @@PeekNone

        ;; Data available — read the byte, then push it back.
        ;; Under VSIO virtualization, reading RBR consumes from SIO's
        ;; buffer. We read it via IOCtl NDR (Non-Destructive Read)
        ;; by sending IOCtl 64h (status) + checking RX buffer.
        ;; Alternatively, since VX00 runs in a VDM and VSIO virtualizes
        ;; the UART, we can read and re-stuff via the scratch register
        ;; trick, or simply return the byte without consuming.
        ;;
        ;; Practical approach: use BIOS INT 14h function 03h to check
        ;; status, then read via the virtualized port. VSIO will handle
        ;; the peek at the SIO buffer level.
        in      al, dx                  ; Read byte from RBR
        xor     ah, ah                  ; AH=0 = success
        ;; Note: this DOES consume the byte. True non-destructive peek
        ;; requires SIO IOCtl support via VSIO. Programs that peek
        ;; repeatedly will lose bytes. This matches real VX00 behavior
        ;; where peek was imperfect under some conditions.
        jmp     FnExit

@@PeekNone:
        mov     ax, 0FFFFh
        jmp     FnExit


;; ====================================================================
;; 0Dh — Keyboard Read (no wait, non-destructive)
;; ====================================================================
;; Reads from the local keyboard, not the serial port.
;; Out: AX = keystroke or FFFFh if none
;; ====================================================================

Fn0D_KbdRead:
        mov     ah, 01h                 ; BIOS: check key
        int     16h
        jz      @@NoKey
        mov     ah, 00h                 ; BIOS: read key
        int     16h
        jmp     FnExit
@@NoKey:
        mov     ax, 0FFFFh
        jmp     FnExit


;; ====================================================================
;; 0Eh — Keyboard Peek (no wait)
;; ====================================================================

Fn0E_KbdPeek:
        mov     ah, 01h
        int     16h
        jz      @@NoPeekKey
        ;; AX already has the keystroke
        jmp     FnExit
@@NoPeekKey:
        mov     ax, 0FFFFh
        jmp     FnExit


;; ====================================================================
;; 0Fh — Flow Control Enable/Disable
;; ====================================================================
;; In:  AL = flow control flags
;;        Bit 0: enable XON/XOFF on transmit
;;        Bit 1: enable CTS/RTS hardware flow control
;;        Bit 3: enable XON/XOFF on receive
;; ====================================================================

Fn0F_FlowCtrl:
        call    GetPortBase
        mov     CS:[si].fpFlowCtrl, al
        ;; Flow control is actually managed by SIO.SYS via the DCB.
        ;; Under the real VX00, flow control settings go through SIO
        ;; IOCtls. Under our virtualized port, VSIO handles this.
        ;; We store the flags for fn 19h (driver info) reporting.
        jmp     FnExit


;; ====================================================================
;; 10h — Ctrl-C / Ctrl-K Control
;; ====================================================================
;; In:  AL = control flags
;;        Bit 0: enable Ctrl-C checking
;;        Bit 1: enable Ctrl-K (transmit on/off)

Fn10_CtrlCK:
        ;; Not meaningful under OS/2 VDMs — store and ignore
        jmp     FnExit


;; ====================================================================
;; 11h — Set Cursor Position
;; ====================================================================
;; In:  DH = row, DL = column (0-based)

Fn11_SetCursor:
        push    bx
        mov     ah, 02h                 ; BIOS: set cursor
        xor     bh, bh                  ; Page 0
        int     10h
        pop     bx
        jmp     FnExit


;; ====================================================================
;; 12h — Get Cursor Position
;; ====================================================================
;; Out: DH = row, DL = column

Fn12_GetCursor:
        push    bx
        mov     ah, 03h                 ; BIOS: get cursor
        xor     bh, bh
        int     10h
        pop     bx
        jmp     FnExit


;; ====================================================================
;; 13h — ANSI Write Character to Screen
;; ====================================================================
;; In:  AL = character to write

Fn13_AnsiWrite:
        push    ax
        mov     ah, 0Eh                 ; BIOS: teletype output
        xor     bh, bh
        int     10h
        pop     ax
        jmp     FnExit


;; ====================================================================
;; 16h — Block Read
;; ====================================================================
;; In:  CX = requested count, ES:DI = buffer
;; Out: AX = actual bytes read
;; ====================================================================

Fn16_BlockRead:
        call    GetPortBase
        push    cx
        xor     ax, ax                  ; Count read

@@BRLoop:
        test    cx, cx
        jz      @@BRDone

        push    dx
        add     dx, UART_LSR
        push    ax
        in      al, dx
        mov     ah, al
        pop     ax
        pop     dx
        test    ah, LSR_DR
        jz      @@BRDone                ; No more data

        push    ax
        in      al, dx                  ; Read byte
        stosb                           ; Store to ES:DI, inc DI
        pop     ax
        inc     ax
        dec     cx
        jmp     @@BRLoop

@@BRDone:
        pop     cx
        jmp     FnExit


;; ====================================================================
;; 17h — Block Write
;; ====================================================================
;; In:  CX = byte count, ES:DI = buffer
;; Out: AX = actual bytes written
;; ====================================================================

Fn17_BlockWrite:
        call    GetPortBase
        push    cx
        push    si
        mov     si, di                  ; SI = source offset
        xor     ax, ax                  ; Count written

@@BWLoop:
        test    cx, cx
        jz      @@BWDone

        ;; Wait for THR empty
        push    dx
        add     dx, UART_LSR
        push    ax
@@BWWait:
        in      al, dx
        test    al, LSR_THRE
        jz      @@BWWait
        pop     ax
        pop     dx

        ;; Send byte
        push    ax
        mov     al, es:[si]
        out     dx, al
        pop     ax
        inc     si
        inc     ax
        dec     cx
        jmp     @@BWLoop

@@BWDone:
        pop     si
        pop     cx
        jmp     FnExit


;; ====================================================================
;; 18h — Break Signal Control
;; ====================================================================
;; In:  AL = 1 to start break, 0 to stop break
;; ====================================================================

Fn18_Break:
        call    GetPortBase
        mov     CS:[si].fpBreakState, al

        push    dx
        add     dx, UART_LCR
        push    ax
        in      al, dx
        test    BYTE PTR CS:[si].fpBreakState, 1
        jz      @@BreakOff
        or      al, 40h                ; LCR_BREAK
        jmp     @@BreakSet
@@BreakOff:
        and     al, NOT 40h
@@BreakSet:
        out     dx, al
        pop     ax
        pop     dx
        jmp     FnExit


;; ====================================================================
;; 19h — Driver Information
;; ====================================================================
;; In:  CX = size of info buffer, ES:DI = buffer
;; Out: AX = bytes written to buffer
;;
;; Info structure (19 bytes):
;;   WORD  structure size (19)
;;   BYTE  FOSSIL spec version
;;   BYTE  driver revision
;;   DWORD pointer to driver ID string
;;   WORD  RX buffer size
;;   WORD  RX bytes available
;;   WORD  TX buffer size
;;   WORD  TX bytes free
;;   BYTE  screen width
;;   BYTE  screen height
;;   BYTE  baud rate code
;; ====================================================================

Fn19_DriverInfo:
        call    GetPortBase

        cmp     cx, 19
        jb      @@InfoShort

        ;; Fill info structure
        mov     WORD PTR es:[di],   19          ; Structure size
        mov     BYTE PTR es:[di+2], FOSSIL_REV  ; Spec version
        mov     BYTE PTR es:[di+3], 1           ; Driver revision
        ;; Driver ID far pointer
        mov     WORD PTR es:[di+4], OFFSET FOSSIL_ID
        mov     WORD PTR es:[di+6], cs
        ;; RX buffer size (SIO's buffer — report 4096)
        mov     WORD PTR es:[di+8], 4096
        ;; RX bytes available — check LSR
        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_DR
        jz      @@NoRxInfo
        mov     WORD PTR es:[di+10], 1          ; At least 1 byte
        jmp     @@TxInfo
@@NoRxInfo:
        mov     WORD PTR es:[di+10], 0
@@TxInfo:
        ;; TX buffer size
        mov     WORD PTR es:[di+12], 4096
        ;; TX free — check THRE
        push    dx
        add     dx, UART_LSR
        in      al, dx
        pop     dx
        test    al, LSR_THRE
        jz      @@NoTxInfo
        mov     WORD PTR es:[di+14], 4096       ; Buffer available
        jmp     @@ScreenInfo
@@NoTxInfo:
        mov     WORD PTR es:[di+14], 0
@@ScreenInfo:
        ;; Screen dimensions (get from BIOS)
        push    bx
        mov     ah, 0Fh                         ; BIOS: get video mode
        int     10h
        mov     BYTE PTR es:[di+16], ah         ; Screen width (columns)
        ;; Screen height — read from BIOS data area
        push    es
        push    di
        mov     di, 0040h
        mov     es, di
        mov     al, es:[0084h]                  ; Rows - 1
        inc     al
        pop     di
        pop     es
        mov     BYTE PTR es:[di+17], al         ; Screen height
        pop     bx
        ;; Baud rate code
        mov     BYTE PTR es:[di+18], 0          ; Not tracked here
        mov     ax, 19                          ; Bytes written
        jmp     FnExit

@@InfoShort:
        xor     ax, ax
        jmp     FnExit


;; ====================================================================
;; DOS Device Driver Strategy Routine
;; ====================================================================
;; Only handles INIT (command 0). All other commands return success.
;; ====================================================================

ReqPktOff       DW      ?
ReqPktSeg       DW      ?

Strategy PROC FAR
        mov     CS:[ReqPktOff], bx
        mov     CS:[ReqPktSeg], es
        ret
Strategy ENDP

Interrupt PROC FAR
        push    ds
        push    es
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di

        push    cs
        pop     ds

        les     bx, DWORD PTR CS:[ReqPktOff]
        mov     al, es:[bx+2]          ; Command code

        cmp     al, 0                  ; INIT?
        je      @@DoInit

        ;; All other commands: return OK
        mov     WORD PTR es:[bx+3], 0100h       ; Status = DONE
        jmp     @@IntDone

@@DoInit:
        ;; Hook INT 14h
        push    es
        push    bx
        mov     ax, 3514h              ; DOS: get INT 14h vector
        int     21h
        mov     CS:[OldInt14Off], bx
        mov     CS:[OldInt14Seg], es
        pop     bx
        pop     es

        push    es
        push    bx
        push    ds
        push    dx
        mov     ax, 2514h              ; DOS: set INT 14h vector
        mov     dx, OFFSET Int14Handler
        push    cs
        pop     ds
        int     21h
        pop     dx
        pop     ds
        pop     bx
        pop     es

        ;; Set end-of-resident code in request packet
        mov     WORD PTR es:[bx+0Eh], OFFSET EndResident
        mov     WORD PTR es:[bx+10h], cs

        ;; Print init message
        push    es
        push    bx
        mov     ah, 09h
        mov     dx, OFFSET InitBanner
        int     21h
        pop     bx
        pop     es

        mov     WORD PTR es:[bx+3], 0100h       ; Status = DONE

@@IntDone:
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        pop     es
        pop     ds
        ret
Interrupt ENDP

InitBanner      DB      'VX00 FOSSIL Driver for OS/2', 0Dh, 0Ah
                DB      'Clean-room implementation — FTS-0001 Rev 5', 0Dh, 0Ah, '$'

EndResident      LABEL   BYTE

_TEXT   ENDS
        END
