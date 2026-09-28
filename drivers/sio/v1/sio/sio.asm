;; ====================================================================
;; SIO.SYS — Serial I/O Device Driver for OS/2
;; ====================================================================
;; Clean-room reimplementation of Ray Gwinn's SIO.
;; Built from SIOREF.TXT, OS/2 Toolkit 4.5 DDK, and UART datasheets.
;; No binary analysis or reverse engineering was performed.
;; ====================================================================

        .386p
        TITLE   SIO — Serial I/O Character Device Driver for OS/2

        INCLUDE os2ddk.inc
        INCLUDE uart.inc
        INCLUDE siodata.inc

;; ====================================================================
;; DATA SEGMENT — Device Header + Driver State
;; ====================================================================

_DATA   SEGMENT WORD PUBLIC USE16 'DATA'

;; ---------- OS/2 Device Header (must be at offset 0) ----------
;; This is the structure the OS/2 kernel reads to identify the driver.
;; Character device, supports IOCtl, Generic IOCtl, Open/Close.

        PUBLIC  SIODevHdr
SIODevHdr  LABEL  BYTE
        DD      -1                      ; SDevNext  — filled by OS/2
        DW      0D840h                  ; SDevAtt: CHAR|IOCTL|GIOCTL|SHARE|30
        DW      OFFSET Strategy         ; SDevStrat — strategy entry point
        DW      0                       ; SDevInt   — IDC entry (unused)
        DB      'SIO$    '              ; SDevName  — 8-char device name
                                        ;   (apps open "SIO$" or "COMn")
        DW      0                       ; SDevProtCS
        DW      0                       ; SDevProtDS
        DW      0                       ; SDevRealCS
        DW      0                       ; SDevRealDS
        DD      0011h                   ; SDevCaps: DEV_INITCOMPLETE | DEV_IOCTL2

;; ---------- Additional device headers for COM1..COM8 ----------
;; Each COMn port gets its own header so apps can DosOpen("COM1") etc.
;; These are chained via SDevNext during INIT.

NUM_COM_HEADERS EQU     8

COMHeaders      LABEL   BYTE
        REPT    NUM_COM_HEADERS
        DD      -1                      ; SDevNext
        DW      0D840h
        DW      OFFSET Strategy
        DW      0
        DB      'COM     '              ; Patched to "COM1" etc during INIT
        DW      0, 0, 0, 0
        DD      0011h
        ENDM

;; ---------- Global Driver State ----------

        PUBLIC  DriverData
DriverData      DB      SIZE DRIVERDATA DUP (0)

;; ---------- Strategy routine request packet pointer ----------

ReqPktOff       DW      ?
ReqPktSeg       DW      ?

;; ---------- Init message ----------

InitMsg         DB      13, 10
                DB      'SIO v1.00 — Serial I/O Driver for OS/2', 13, 10
                DB      'Clean-room reimplementation', 13, 10
                ;; The original shareware SIO.SYS showed a
                ;; "Registered to <name>" banner line (or a nag if
                ;; unregistered). This clean-room build has no
                ;; registration system at all (see BUGFIXES.md M07 /
                ;; STATUS.md's "Registration Key Check" section) — no
                ;; key, no serial number, nothing to register. This
                ;; line takes that banner line's place with the
                ;; actual license status instead of a name/nag.
                DB      'Registered: is free software released under', 13, 10
                DB      'the GNU General Public License v3.0 (GPLv3)', 13, 10, 0

PortDetectMsg   DB      '  COM'
PortDetectNum   DB      '?'
                DB      ' at '
PortDetectAddr  DB      '????h '
PortDetectType  DB      '                ', 13, 10, 0

;; ---------- Detection message pieces (used by PrintPortDetect) ----------
;; The fixed-width buffer above (PortDetectMsg..PortDetectType) can't
;; safely hold every case: PortDetectNum is 1 byte (COM10-16 need two
;; digits) and PortDetectType is 16 bytes (the longest UART name,
;; "Detected 16550A (high-speed)", is 30). PrintPortDetect below
;; prints each piece with its own PrintMsg call instead of patching
;; into this buffer, so nothing is silently truncated.
PortDetectPfx   DB      '  COM', 0
PortDetectAtStr DB      ' at ', 0
PortDetectHSfx  DB      'h  ', 0
PortMsgCRLF     DB      13, 10, 0

;; Overlap / shared-IRQ warnings (see ValidatePortConfig)
OverlapWarnMsg  DB      13, 10
                DB      'Warning, overlapping port addresses specified', 13, 10, 0
MouseWarnMsg    DB      13, 10
                DB      'Warning, the mouse driver must be loaded before SIO.SYS.', 13, 10, 0

;; Decimal port-number scratch buffer for PrintPortDetect (2 digits + NUL)
PortNumScratch  DB      '  ', 0

;; Hex address scratch buffer for PrintPortDetect (4 digits + NUL)
PortAddrScratch DB      '    ', 0

;; Loop indices for ValidatePortConfig's overlap check. Kept in memory
;; (not registers) so `mul` — needed for PORTDATA pointer arithmetic
;; inside the loop — can never clobber the loop state the way it did
;; in an earlier draft of this routine (see BUGFIXES.md).
VPCOuterIdx     DW      ?
VPCInnerIdx     DW      ?

UARTNames       DW      OFFSET szNone
                DW      OFFSET sz8250
                DW      OFFSET sz8250A
                DW      OFFSET sz16450
                DW      OFFSET sz16550
                DW      OFFSET sz16550A
                DW      OFFSET sz16550AHi
                DW      OFFSET sz82510
                DW      OFFSET sz16650
                DW      OFFSET sz16750
                DW      OFFSET szComBic

szNone          DB      'Not detected', 0
sz8250          DB      'Detected 8250', 0
sz8250A         DB      'Detected 8250A', 0
sz16450         DB      'Detected 16450', 0
sz16550         DB      'Detected 16550 (FIFO disabled)', 0
sz16550A        DB      'Detected 16550A', 0
sz16550AHi      DB      'Detected 16550A (high-speed)', 0
sz82510         DB      'Detected 82510', 0
sz16650         DB      'Detected 16650', 0
sz16750         DB      'Detected 16750', 0
szComBic        DB      'Detected ComBic (ESP)', 0

;; ---------- Receive and Transmit buffer pools ----------
;; Allocated during INIT via DevHlp_AllocPhys, then mapped.
;; Each port gets RX_BUF_SIZE + TX_BUF_SIZE bytes.

RxBufPool       DD      MAX_PORTS DUP (0)       ; Physical addresses
TxBufPool       DD      MAX_PORTS DUP (0)

_DATA   ENDS


;; ====================================================================
;; CODE SEGMENT
;; ====================================================================

_TEXT   SEGMENT WORD PUBLIC USE16 'CODE'
        ASSUME  CS:_TEXT, DS:_DATA, ES:NOTHING

;; ====================================================================
;; Strategy Entry Point
;; ====================================================================
;; Called by OS/2 kernel with ES:BX → request packet.
;; We save the pointer and dispatch based on the command code.

        PUBLIC  Strategy
Strategy PROC FAR
        push    es
        push    bx
        push    ds
        push    ax
        push    cx
        push    dx
        push    si
        push    di

        ;; Save request packet address
        mov     WORD PTR [ReqPktOff], bx
        mov     WORD PTR [ReqPktSeg], es

        ;; Get command code from request packet
        mov     al, es:[bx+2]           ; ReqPkt.Command
        xor     ah, ah

        ;; Bounds check
        cmp     al, CMDInitComplete
        ja      @@BadCmd

        ;; Dispatch through command table
        shl     ax, 1                   ; * 2 for word table
        mov     si, ax
        jmp     WORD PTR CS:[CmdTable + si]

@@BadCmd:
        mov     ax, 8103h               ; STATUS_ERR + ERROR_I24_INVALID_COMMAND
        jmp     StratExit

;; ---------- Command Dispatch Table ----------

CmdTable LABEL WORD
        DW      OFFSET CmdInit          ;  0 — INIT
        DW      OFFSET CmdNotSupported  ;  1 — Media Check (block only)
        DW      OFFSET CmdNotSupported  ;  2 — Build BPB (block only)
        DW      OFFSET CmdNotSupported  ;  3 — IOCtl Read (old style)
        DW      OFFSET CmdRead          ;  4 — READ
        DW      OFFSET CmdNDRead        ;  5 — Non-Destructive Read
        DW      OFFSET CmdInputStatus   ;  6 — Input Status
        DW      OFFSET CmdInputFlush    ;  7 — Input Flush
        DW      OFFSET CmdWrite         ;  8 — WRITE
        DW      OFFSET CmdWriteVerify   ;  9 — Write with Verify
        DW      OFFSET CmdOutputStatus  ; 10 — Output Status
        DW      OFFSET CmdOutputFlush   ; 11 — Output Flush
        DW      OFFSET CmdNotSupported  ; 12 — IOCtl Write (old style)
        DW      OFFSET CmdOpen          ; 13 — OPEN
        DW      OFFSET CmdClose         ; 14 — CLOSE
        DW      OFFSET CmdNotSupported  ; 15 — Removable Media
        DW      OFFSET CmdGenIOCtl      ; 16 — Generic IOCtl
        DW      OFFSET CmdNotSupported  ; 17 — Reset Media
        DW      OFFSET CmdNotSupported  ; 18 — Get Logical Map
        DW      OFFSET CmdNotSupported  ; 19 — Set Logical Map
        DW      OFFSET CmdDeInstall     ; 20 — DeInstall
        DW      OFFSET CmdNotSupported  ; 21
        DW      OFFSET CmdNotSupported  ; 22
        DW      OFFSET CmdNotSupported  ; 23
        DW      OFFSET CmdNotSupported  ; 24
        DW      OFFSET CmdNotSupported  ; 25
        DW      OFFSET CmdNotSupported  ; 26
        DW      OFFSET CmdNotSupported  ; 27 — InitBase
        DW      OFFSET CmdShutdown      ; 28 — Shutdown
        DW      OFFSET CmdNotSupported  ; 29
        DW      OFFSET CmdNotSupported  ; 30
        DW      OFFSET CmdInitComplete  ; 31 — InitComplete

;; ====================================================================
;; Strategy Exit — store status in request packet and return
;; ====================================================================
;; AX = status word on entry

        PUBLIC  StratExit
StratExit:
        les     bx, DWORD PTR [ReqPktOff]
        or      ax, 0100h               ; Set DONE bit
        mov     es:[bx+3], ax           ; ReqPkt.Status

        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     ax
        pop     ds
        pop     bx
        pop     es
        ret
Strategy ENDP


;; ====================================================================
;; Command: Not Supported
;; ====================================================================

CmdNotSupported:
        mov     ax, 8103h               ; ERROR_I24_INVALID_COMMAND
        jmp     StratExit



;; ====================================================================
;; Command: Write with Verify (same as Write for char devices)
;; ====================================================================

CmdWriteVerify:
        jmp     CmdWrite



;; ====================================================================
;; Command: Shutdown
;; ====================================================================

CmdShutdown:
        ;; Drop DTR on all open ports, disable interrupts
        xor     ax, ax                  ; STATUS_OK
        jmp     StratExit



;; ====================================================================
;; Command: DeInstall
;; ====================================================================

CmdDeInstall:
        xor     ax, ax
        jmp     StratExit



;; ====================================================================
;; Command: InitComplete
;; ====================================================================

CmdInitComplete:
        xor     ax, ax
        jmp     StratExit



;; ====================================================================
;; GetPortData — get PORTDATA pointer for the device being accessed
;; ====================================================================
;; In:  ES:BX → request packet (from Strategy)
;; Out: DS:SI → PORTDATA for this port
;;      CF set if invalid port
;; Trashes: AX, CX
;; ====================================================================

GetPortData:
        ;; The device unit number is derived from which device header
        ;; received the request. We compare the header pointer in the
        ;; request packet against our COMn headers.
        ;; For now, use the SFN or unit field — this will be refined
        ;; during integration testing.

        ;; Placeholder: return port 0
        lea     si, [DriverData.ddPorts]
        clc
        ret



;; ====================================================================
;; INCLUDE the implementation files
;; ====================================================================

        INCLUDE sioinit.asm             ; CmdInit — driver initialization
        INCLUDE sioopen.asm             ; CmdOpen, CmdClose
        INCLUDE sioio.asm               ; CmdRead, CmdWrite, CmdNDRead
        INCLUDE siostatus.asm           ; Input/Output Status/Flush
        INCLUDE sioioctl.asm            ; CmdGenIOCtl — all 20+ IOCtl functions
        INCLUDE sioisr.asm              ; Interrupt service routine
        INCLUDE siouart.asm             ; UART detection and hardware abstraction
        INCLUDE siobuf.asm              ; Ring buffer operations

_TEXT   ENDS

        END
