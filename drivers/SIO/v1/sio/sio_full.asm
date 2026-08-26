        .386p
        TITLE   SIO - Serial I/O Device Driver for OS/2 (Full Implementation)

;; ====================================================================
;; CONSTANTS
;; ====================================================================

; Device header
DEVATTR         EQU     0D840h          ; CHAR|IOCTL|GIOCTL|SHARE|30

; Strategy commands
CMD_INIT        EQU     0
CMD_READ        EQU     4
CMD_NDR         EQU     5
CMD_INSTAT      EQU     6
CMD_INFLUSH     EQU     7
CMD_WRITE       EQU     8
CMD_WRITEV      EQU     9
CMD_OUTSTAT     EQU     10
CMD_OUTFLUSH    EQU     11
CMD_OPEN        EQU     13
CMD_CLOSE       EQU     14
CMD_GENIOCTL    EQU     16
CMD_DEINSTALL   EQU     20
CMD_SHUTDOWN    EQU     28
CMD_INITCOMPLETE EQU    31

; DevHlp function codes
DH_PROCBLOCK    EQU     4
DH_PROCRUN      EQU     5
DH_ALLOCPHYS    EQU     24
DH_SETIRQ       EQU     27
DH_UNSETIRQ     EQU     28
DH_EOI          EQU     49
DH_PHYSTOVIRT   EQU     21
DH_PHYSTOGDT    EQU     84
DH_VERIFYACCESS EQU     39

; ASYNC IOCtl
IOCTL_ASYNC     EQU     01h

; UART registers
U_RBR           EQU     0
U_THR           EQU     0
U_DLL           EQU     0
U_DLH           EQU     1
U_IER           EQU     1
U_IIR           EQU     2
U_FCR           EQU     2
U_LCR           EQU     3
U_MCR           EQU     4
U_LSR           EQU     5
U_MSR           EQU     6
U_SCR           EQU     7

; IER bits
IER_RDA         EQU     01h
IER_THRE        EQU     02h
IER_RLS         EQU     04h
IER_MS          EQU     08h
IER_ALL         EQU     0Fh

; IIR bits
IIR_NOPEND      EQU     01h
IIR_IDMASK      EQU     0Eh
IIR_MODEM       EQU     00h
IIR_THRE        EQU     02h
IIR_RDA         EQU     04h
IIR_RLS         EQU     06h
IIR_TIMEOUT     EQU     0Ch
IIR_FIFOOK      EQU     0C0h
IIR_FIFOBAD     EQU     080h

; FCR bits
FCR_ENA         EQU     01h
FCR_RXRST       EQU     02h
FCR_TXRST       EQU     04h
FCR_T1          EQU     00h
FCR_T4          EQU     40h
FCR_T8          EQU     80h
FCR_T14         EQU     0C0h

; LCR bits
LCR_5BIT        EQU     00h
LCR_6BIT        EQU     01h
LCR_7BIT        EQU     02h
LCR_8BIT        EQU     03h
LCR_WMASK       EQU     03h
LCR_STOP2       EQU     04h
LCR_PEN         EQU     08h
LCR_EPS         EQU     10h
LCR_STICK       EQU     20h
LCR_BREAK       EQU     40h
LCR_DLAB        EQU     80h

; MCR bits
MCR_DTR         EQU     01h
MCR_RTS         EQU     02h
MCR_OUT1        EQU     04h
MCR_OUT2        EQU     08h
MCR_LOOP        EQU     10h

; LSR bits
LSR_DR          EQU     01h
LSR_OE          EQU     02h
LSR_PE          EQU     04h
LSR_FE          EQU     08h
LSR_BI          EQU     10h
LSR_THRE        EQU     20h
LSR_TEMT        EQU     40h

; MSR bits
MSR_DCTS        EQU     01h
MSR_DDSR        EQU     02h
MSR_TERI        EQU     04h
MSR_DDCD        EQU     08h
MSR_CTS         EQU     10h
MSR_DSR         EQU     20h
MSR_RI          EQU     40h
MSR_DCD         EQU     80h

; UART types
UT_NONE         EQU     0
UT_8250         EQU     1
UT_8250A        EQU     2
UT_16450        EQU     3
UT_16550        EQU     4
UT_16550A       EQU     5
UT_16650        EQU     6
UT_16750        EQU     7
UT_16850        EQU     8
UT_16950        EQU     9

; Port addresses
COM1_ADDR       EQU     03F8h
COM2_ADDR       EQU     02F8h
COM3_ADDR       EQU     03E8h
COM4_ADDR       EQU     02E8h
COM1_IRQ        EQU     4
COM2_IRQ        EQU     3

; Buffer sizes
RXBUFSZ         EQU     4096
TXBUFSZ         EQU     4096
MAXPORTS        EQU     16

; Baud clock
BAUDCLK         EQU     115200

;; ====================================================================
;; PORTDATA offsets (flat, no STRUC)
;; ====================================================================

PD_IOBASE       EQU     0              ; WORD  - UART base I/O
PD_IOBASEDOS    EQU     2              ; WORD  - virtual I/O for DOS
PD_IRQ          EQU     4              ; BYTE  - hardware IRQ
PD_IRQDOS       EQU     5              ; BYTE  - virtual IRQ for DOS
PD_UARTTYPE     EQU     6              ; BYTE  - chip type
PD_FIFOSZ       EQU     7              ; BYTE  - FIFO depth
PD_FLAGS        EQU     8              ; WORD  - state flags
PD_OPENCOUNT    EQU     10             ; WORD  - open count
PD_BAUDRATE     EQU     12             ; DWORD - current baud
PD_BAUDDIV      EQU     16             ; WORD  - divisor
PD_LINECTRL     EQU     18             ; BYTE  - LCR shadow
PD_MCR          EQU     19             ; BYTE  - MCR shadow
PD_MSR          EQU     20             ; BYTE  - last MSR
PD_LSR          EQU     21             ; BYTE  - accumulated LSR errors
; DCB at offset 22 (11 bytes)
PD_DCB          EQU     22
PD_DCB_WTO      EQU     22             ; WORD  - write timeout
PD_DCB_RTO      EQU     24             ; WORD  - read timeout
PD_DCB_F1       EQU     26             ; BYTE  - flags1
PD_DCB_F2       EQU     27             ; BYTE  - flags2
PD_DCB_F3       EQU     28             ; BYTE  - flags3
PD_DCB_EREP     EQU     29             ; BYTE  - error replacement
PD_DCB_BREP     EQU     30             ; BYTE  - break replacement
PD_DCB_XON      EQU     31             ; BYTE  - XON char
PD_DCB_XOFF     EQU     32             ; BYTE  - XOFF char
; Error/event
PD_ERROR        EQU     33             ; WORD  - error word
PD_EVENT        EQU     35             ; WORD  - event word
; Ring buffers (head, tail, count, size, base = 12 bytes each)
PD_RXHEAD       EQU     37             ; WORD
PD_RXTAIL       EQU     39             ; WORD
PD_RXCOUNT      EQU     41             ; WORD
PD_RXSIZE       EQU     43             ; WORD
PD_RXBASE       EQU     45             ; DWORD (GDT sel:0)
PD_TXHEAD       EQU     49             ; WORD
PD_TXTAIL       EQU     51             ; WORD
PD_TXCOUNT      EQU     53             ; WORD
PD_TXSIZE       EQU     55             ; WORD
PD_TXBASE       EQU     57             ; DWORD
; Flow control
PD_TXHOLD       EQU     61             ; BYTE
PD_XOFFRECVD    EQU     62             ; BYTE
PD_XOFFSENT     EQU     63             ; BYTE
PD_TXIMM        EQU     64             ; BYTE - immediate pending
PD_TXIMMCH      EQU     65             ; BYTE - immediate char
; Blocking
PD_READWAIT     EQU     66             ; DWORD - block ID
PD_WRITEWAIT    EQU     70             ; DWORD - block ID
; Misc
PD_ISINET       EQU     74             ; BYTE
PD_PROTMODE     EQU     75             ; BYTE
PD_BAUDLOCK     EQU     76             ; BYTE
PD_PAD          EQU     77             ; BYTE
PD_SIZE         EQU     78             ; Total per-port size

; Port flags
PF_EXISTS       EQU     0001h
PF_OPEN         EQU     0002h
PF_IRQCLAIMED   EQU     0004h
PF_FIFOENABLED  EQU     0008h
PF_READING      EQU     0010h
PF_WRITING      EQU     0020h
PF_BREAKON      EQU     0040h
PF_INTERNET     EQU     0080h

; DCB flag bits
F1_DTRMASK      EQU     03h
F1_DTRENA       EQU     01h
F1_CTSENA       EQU     08h
F1_DSRRX        EQU     40h
F2_XONRX        EQU     01h
F2_XONTX        EQU     02h
F2_RTSMASK      EQU     0C0h
F2_RTSENA       EQU     040h
F2_RTSHS        EQU     080h
F3_WTOINF       EQU     01h
F3_RTOMASK      EQU     06h
F3_RTONORM      EQU     02h
F3_RTOWAIT      EQU     04h
F3_RTONOW       EQU     06h
F3_FORCED       EQU     0D0h           ; FIFO=ena(10h) + trig8(40h) + txload(80h)


;; ====================================================================
;; DATA SEGMENT
;; ====================================================================

_DATA   SEGMENT WORD PUBLIC USE16 'DATA'

;; Device Header
        PUBLIC  SIOHdr
SIOHdr  LABEL   BYTE
        DD      OFFSET _DATA:COM1Hdr    ; SDevNext → COM1
        DW      DEVATTR                 ; SDevAtt
        DW      OFFSET _TEXT:Strategy   ; SDevStrat
        DW      0                       ; SDevInt
        DB      'SIO$    '              ; SDevName
        DW      0, 0, 0, 0             ; CS/DS fields
        DD      0011h                   ; SDevCaps

;; Driver globals
;; COM1 device header (chained from SIO$)
COM1Hdr         LABEL   BYTE
        DD      OFFSET _DATA:COM2Hdr    ; → COM2
        DW      DEVATTR
        DW      OFFSET _TEXT:Strategy
        DW      0
        DB      'COM1    '
        DW      0, 0, 0, 0
        DD      0011h

COM2Hdr         LABEL   BYTE
        DD      OFFSET _DATA:COM3Hdr
        DW      DEVATTR
        DW      OFFSET _TEXT:Strategy
        DW      0
        DB      'COM2    '
        DW      0, 0, 0, 0
        DD      0011h

COM3Hdr         LABEL   BYTE
        DD      OFFSET _DATA:COM4Hdr
        DW      DEVATTR
        DW      OFFSET _TEXT:Strategy
        DW      0
        DB      'COM3    '
        DW      0, 0, 0, 0
        DD      0011h

COM4Hdr         LABEL   BYTE
        DD      -1                       ; Last in chain
        DW      DEVATTR
        DW      OFFSET _TEXT:Strategy
        DW      0
        DB      'COM4    '
        DW      0, 0, 0, 0
        DD      0011h

NumPorts        DW      0
DevHlp          DD      0
RPOff           DW      0
RPSeg           DW      0
ISRFlag         DW      0              ; Set if any ISR pass serviced

;; IOCtl packet pointers
IOPrmOff        DW      0
IOPrmSeg        DW      0
IODatOff        DW      0
IODatSeg        DW      0

;; Transfer buffer pointer
XferOff         DW      0
XferSeg         DW      0

;; Port data array (MAXPORTS * PD_SIZE = 16 * 78 = 1248 bytes)
PortArray       DB      1248 DUP (0)

;; Default port table
DefPorts        DW      COM1_ADDR, COM2_ADDR, COM3_ADDR, COM4_ADDR
DefIRQs         DB      COM1_IRQ, COM2_IRQ, COM1_IRQ, COM2_IRQ

;; Messages
Banner          DB      'SIO v1.00 Serial I/O Driver for OS/2', 0Dh, 0Ah, '$'

;; Log file
LogFile         DB      '\SIO.LOG', 0
LogHandle       DW      0FFFFh          ; File handle (FFFFh = not open)
LogBuf          DB      256 DUP (0)
LogPos          DW      0
MsgCOM          DB      '  COM'
MsgCOMn         DB      '?'
MsgFound        DB      ' found', 0Dh, 0Ah, '$'

EndOfData       LABEL   BYTE
_DATA   ENDS


;; ====================================================================
;; CODE SEGMENT
;; ====================================================================

_TEXT   SEGMENT WORD PUBLIC USE16 'CODE'
        ASSUME  CS:_TEXT, DS:_DATA, ES:NOTHING


;; ====================================================================
;; Strategy Entry Point
;; ====================================================================

Strategy:
        mov     word ptr ds:[RPOff], bx
        mov     word ptr ds:[RPSeg], es
        push    ax
        push    bx
        push    cx
        push    dx
        push    si
        push    di
        push    ds
        push    es

        mov     al, es:[bx+2]
        xor     ah, ah
        cmp     al, CMD_INITCOMPLETE
        ja      sBadCmd
        shl     ax, 1
        mov     si, ax
        jmp     word ptr cs:[CmdTbl + si]

sBadCmd:
        mov     ax, 8103h
sExit:
        les     bx, dword ptr ds:[RPOff]
        or      ax, 0100h
        mov     es:[bx+3], ax
        pop     es
        pop     ds
        pop     di
        pop     si
        pop     dx
        pop     cx
        pop     bx
        pop     ax
        retf

CmdTbl  LABEL   WORD
        DW      OFFSET _TEXT:DoInit     ; 0
        DW      OFFSET _TEXT:sNotSup    ; 1
        DW      OFFSET _TEXT:sNotSup    ; 2
        DW      OFFSET _TEXT:sNotSup    ; 3
        DW      OFFSET _TEXT:DoRead     ; 4
        DW      OFFSET _TEXT:DoNDR      ; 5
        DW      OFFSET _TEXT:DoInStat   ; 6
        DW      OFFSET _TEXT:DoInFlush  ; 7
        DW      OFFSET _TEXT:DoWrite    ; 8
        DW      OFFSET _TEXT:DoWrite    ; 9  (write verify = write)
        DW      OFFSET _TEXT:DoOutStat  ; 10
        DW      OFFSET _TEXT:DoOutFlush ; 11
        DW      OFFSET _TEXT:sNotSup    ; 12
        DW      OFFSET _TEXT:DoOpen     ; 13
        DW      OFFSET _TEXT:DoClose    ; 14
        DW      OFFSET _TEXT:sNotSup    ; 15
        DW      OFFSET _TEXT:DoIOCtl    ; 16
        DW      OFFSET _TEXT:sNotSup    ; 17
        DW      OFFSET _TEXT:sNotSup    ; 18
        DW      OFFSET _TEXT:sNotSup    ; 19
        DW      OFFSET _TEXT:sNotSup    ; 20
        DW      OFFSET _TEXT:sNotSup    ; 21
        DW      OFFSET _TEXT:sNotSup    ; 22
        DW      OFFSET _TEXT:sNotSup    ; 23
        DW      OFFSET _TEXT:sNotSup    ; 24
        DW      OFFSET _TEXT:sNotSup    ; 25
        DW      OFFSET _TEXT:sNotSup    ; 26
        DW      OFFSET _TEXT:sNotSup    ; 27
        DW      OFFSET _TEXT:sNotSup    ; 28 shutdown
        DW      OFFSET _TEXT:sNotSup    ; 29
        DW      OFFSET _TEXT:sNotSup    ; 30
        DW      OFFSET _TEXT:sNotSup    ; 31 initcomplete

sNotSup:
        xor     ax, ax
        jmp     sExit


;; ====================================================================
;; INIT
;; ====================================================================

DoInit:
        les     bx, dword ptr ds:[RPOff]
        mov     eax, es:[bx+12h]
        mov     ds:[DevHlp], eax

        ; Open log file
        call    LogInit

        ; Print banner
        push    dx
        mov     dx, OFFSET _DATA:Banner
        mov     ah, 09h
        int     21h
        pop     dx

        ; Log init message
        push    dx
        mov     dx, OFFSET _DATA:LogInitOK
        call    LogWrite
        pop     dx

        ; Parse CONFIG.SYS command line for port definitions
        ; ES:BX+0Eh → far pointer to argument string
        les     bx, dword ptr ds:[RPOff]
        les     bx, es:[bx+0Eh]
        call    ParseCmdLine

        ; If no ports configured, default to COM1+COM2
        cmp     word ptr ds:[NumPorts], 0
        jne     portsConfigured

        mov     word ptr ds:[NumPorts], 2
        lea     si, ds:[PortArray]
        mov     word ptr [si + PD_IOBASE], COM1_ADDR
        mov     word ptr [si + PD_IOBASEDOS], COM1_ADDR
        mov     byte ptr [si + PD_IRQ], COM1_IRQ
        mov     byte ptr [si + PD_IRQDOS], COM1_IRQ
        add     si, PD_SIZE
        mov     word ptr [si + PD_IOBASE], COM2_ADDR
        mov     word ptr [si + PD_IOBASEDOS], COM2_ADDR
        mov     byte ptr [si + PD_IRQ], COM2_IRQ
        mov     byte ptr [si + PD_IRQDOS], COM2_IRQ

portsConfigured:
        ; Probe and init each port
        mov     cx, ds:[NumPorts]
        lea     si, ds:[PortArray]
        mov     byte ptr ds:[MsgCOMn], '1'
initLoop:
        push    cx
        call    DetectUART
        call    InitDCBDefaults
        call    InitUART
        ; Print detection
        push    dx
        mov     dx, OFFSET _DATA:MsgCOM
        mov     ah, 09h
        int     21h
        pop     dx

        ; Log detection result
        push    dx
        push    ax
        mov     al, ds:[MsgCOMn]
        mov     ds:[LogDetNum], al
        test    word ptr [si + PD_FLAGS], PF_EXISTS
        jz      logNotDet
        mov     dx, OFFSET _DATA:LogDetect
        call    LogWrite
        jmp     logDetDone
logNotDet:
        mov     dx, OFFSET _DATA:LogDetect
        call    LogWrite
logDetDone:
        pop     ax
        pop     dx

        inc     byte ptr ds:[MsgCOMn]
        add     si, PD_SIZE
        pop     cx
        loop    initLoop

        ; Allocate ring buffer memory for each port
        call    AllocBuffers

        ; Fix device header chain — only chain headers for detected ports
        call    InitHeaders

        ; Close log file
        call    LogClose

        ; Set end pointers
        les     bx, dword ptr ds:[RPOff]
        mov     word ptr es:[bx+0Eh], OFFSET _TEXT:EndOfCode
        mov     word ptr es:[bx+10h], OFFSET _DATA:EndOfData

        xor     ax, ax
        jmp     sExit


;; ====================================================================
;; UART Detection
;; ====================================================================
;; In: DS:SI → port data, PD_IOBASE set
;; Out: PD_UARTTYPE set

DetectUART:
        push    dx
        push    bx
        mov     dx, [si + PD_IOBASE]

        ; Test scratch register
        add     dx, U_SCR
        mov     al, 55h
        out     dx, al
        jmp     $+2
        in      al, dx
        cmp     al, 55h
        jne     duTry8250

        mov     al, 0AAh
        out     dx, al
        jmp     $+2
        in      al, dx
        cmp     al, 0AAh
        jne     duTry8250

        ; Has scratch reg → try FIFO
        mov     dx, [si + PD_IOBASE]
        add     dx, U_FCR
        mov     al, FCR_ENA or FCR_RXRST or FCR_TXRST or FCR_T14
        out     dx, al
        jmp     $+2

        mov     dx, [si + PD_IOBASE]
        add     dx, U_IIR
        in      al, dx
        and     al, IIR_FIFOOK

        ; Disable FIFO
        push    ax
        mov     dx, [si + PD_IOBASE]
        add     dx, U_FCR
        xor     al, al
        out     dx, al
        pop     ax

        cmp     al, IIR_FIFOOK
        je      duGot16550A
        cmp     al, IIR_FIFOBAD
        je      duGot16550

        mov     byte ptr [si + PD_UARTTYPE], UT_16450
        mov     byte ptr [si + PD_FIFOSZ], 0
        or      word ptr [si + PD_FLAGS], PF_EXISTS
        jmp     duDone

duGot16550:
        mov     byte ptr [si + PD_UARTTYPE], UT_16550
        mov     byte ptr [si + PD_FIFOSZ], 0
        or      word ptr [si + PD_FLAGS], PF_EXISTS
        jmp     duDone

duGot16550A:
        ;; 16550A confirmed. Now probe for enhanced variants.
        ;; 16650: set EFR (Enhanced Feature Register) at LCR=0BFh
        ;; If EFR exists, it's a 16650+. Then check FIFO depth.
        push    dx
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        in      al, dx
        push    ax                      ; Save LCR

        ;; Access EFR: set LCR = 0BFh (magic key for 16650+)
        mov     al, 0BFh
        out     dx, al
        jmp     $+2

        ;; Try reading EFR at offset 2 (same as IIR/FCR)
        mov     dx, [si + PD_IOBASE]
        add     dx, U_IIR               ; EFR is at offset 2 when LCR=0BFh
        in      al, dx
        mov     ah, al                  ; Save EFR value

        ;; Write a test pattern to EFR
        mov     al, 10h                 ; Enable enhanced mode bit
        out     dx, al
        jmp     $+2
        in      al, dx
        cmp     al, 10h
        jne     duNo16650

        ;; EFR responds — it's at least a 16650
        ;; Restore EFR
        mov     al, ah
        out     dx, al

        ;; Check for 16750: write FCR bit 5 (64-byte FIFO enable)
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        pop     ax
        push    ax
        out     dx, al                  ; Restore normal LCR first
        mov     dx, [si + PD_IOBASE]
        add     dx, U_FCR
        mov     al, FCR_ENA or 20h      ; Bit 5 = 64-byte FIFO enable (16750)
        out     dx, al
        jmp     $+2
        mov     dx, [si + PD_IOBASE]
        add     dx, U_IIR
        in      al, dx
        test    al, 20h                 ; If bit 5 echoes in IIR, it's 16750
        jnz     duGot16750

        ;; Check for 16850/16950: probe FIFO depth > 32
        ;; 16850 has 128-byte FIFO, 16950 has 128 + auto flow
        ;; Detect by checking if AFR (Alternate Function Register) exists
        ;; For now: classify as 16650 with 32-byte FIFO
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        pop     ax
        out     dx, al                  ; Restore LCR
        pop     dx

        mov     byte ptr [si + PD_UARTTYPE], UT_16650
        mov     byte ptr [si + PD_FIFOSZ], 32
        or      word ptr [si + PD_FLAGS], PF_EXISTS
        jmp     duDone

duGot16750:
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        pop     ax
        out     dx, al                  ; Restore LCR
        pop     dx

        mov     byte ptr [si + PD_UARTTYPE], UT_16750
        mov     byte ptr [si + PD_FIFOSZ], 64
        or      word ptr [si + PD_FLAGS], PF_EXISTS
        jmp     duDone

duNo16650:
        ;; Not a 16650+ — restore LCR and report as 16550A
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        pop     ax
        out     dx, al
        pop     dx

        mov     byte ptr [si + PD_UARTTYPE], UT_16550A
        mov     byte ptr [si + PD_FIFOSZ], 16
        or      word ptr [si + PD_FLAGS], PF_EXISTS
        jmp     duDone

duTry8250:
        ; Check IER responds
        mov     dx, [si + PD_IOBASE]
        add     dx, U_IER
        xor     al, al
        out     dx, al
        jmp     $+2
        in      al, dx
        and     al, 0Fh
        test    al, al
        jnz     duNone

        mov     al, IER_RDA or IER_THRE
        out     dx, al
        jmp     $+2
        in      al, dx
        and     al, 0Fh
        cmp     al, IER_RDA or IER_THRE
        jne     duNone

        xor     al, al
        out     dx, al

        mov     byte ptr [si + PD_UARTTYPE], UT_8250
        mov     byte ptr [si + PD_FIFOSZ], 0
        or      word ptr [si + PD_FLAGS], PF_EXISTS
        jmp     duDone

duNone:
        mov     byte ptr [si + PD_UARTTYPE], UT_NONE
        and     word ptr [si + PD_FLAGS], not PF_EXISTS

duDone:
        pop     bx
        pop     dx
        ret


;; ====================================================================
;; InitUART — Initialize UART hardware
;; ====================================================================

InitUART:
        test    word ptr [si + PD_FLAGS], PF_EXISTS
        jz      iuRet

        mov     dx, [si + PD_IOBASE]

        ; Disable interrupts
        add     dx, U_IER
        xor     al, al
        out     dx, al

        ; Clear pending conditions
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LSR
        in      al, dx
        mov     dx, [si + PD_IOBASE]
        add     dx, U_MSR
        in      al, dx
        mov     [si + PD_MSR], al
        mov     dx, [si + PD_IOBASE]
        in      al, dx

        ; Set 8N1
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        mov     al, LCR_8BIT
        out     dx, al
        mov     [si + PD_LINECTRL], al

        ; Set 9600 baud
        mov     eax, 9600
        mov     dword ptr [si + PD_BAUDRATE], eax
        mov     word ptr [si + PD_BAUDDIV], 12
        call    ProgDivisor

        ; Enable FIFO if 16550A+
        cmp     byte ptr [si + PD_UARTTYPE], UT_16550A
        jb      iuNoFifo
        mov     dx, [si + PD_IOBASE]
        add     dx, U_FCR
        ;; Set trigger level based on FIFO size
        mov     al, FCR_ENA or FCR_RXRST or FCR_TXRST
        cmp     byte ptr [si + PD_FIFOSZ], 64
        jae     iuTrig14                ; 64+ byte FIFO: use highest trigger
        cmp     byte ptr [si + PD_FIFOSZ], 32
        jae     iuTrig14
        or      al, FCR_T8              ; 16-byte FIFO: trigger at 8
        jmp     iuFifoWrite
iuTrig14:
        or      al, FCR_T14             ; Large FIFO: trigger at 14
        ;; For 16750, also set 64-byte FIFO enable (bit 5)
        cmp     byte ptr [si + PD_UARTTYPE], UT_16750
        jb      iuFifoWrite
        or      al, 20h                 ; 64-byte FIFO enable
iuFifoWrite:
        out     dx, al
        or      word ptr [si + PD_FLAGS], PF_FIFOENABLED
        jmp     iuMCR
iuNoFifo:
        and     word ptr [si + PD_FLAGS], not PF_FIFOENABLED
iuMCR:
        ; MCR: OUT2 on (enables IRQ), DTR/RTS off
        mov     dx, [si + PD_IOBASE]
        add     dx, U_MCR
        mov     al, MCR_OUT2
        out     dx, al
        mov     [si + PD_MCR], al
iuRet:
        ret


;; ====================================================================
;; InitDCBDefaults
;; ====================================================================

InitDCBDefaults:
        mov     word ptr [si + PD_DCB_WTO], 100
        mov     word ptr [si + PD_DCB_RTO], 100
        mov     byte ptr [si + PD_DCB_F1], F1_DTRENA
        mov     byte ptr [si + PD_DCB_F2], F2_RTSENA
        mov     byte ptr [si + PD_DCB_F3], F3_FORCED or F3_RTONORM
        mov     byte ptr [si + PD_DCB_EREP], 0
        mov     byte ptr [si + PD_DCB_BREP], 0
        mov     byte ptr [si + PD_DCB_XON], 11h
        mov     byte ptr [si + PD_DCB_XOFF], 13h
        mov     word ptr [si + PD_ERROR], 0
        mov     word ptr [si + PD_EVENT], 0
        mov     byte ptr [si + PD_TXHOLD], 0
        mov     byte ptr [si + PD_XOFFRECVD], 0
        mov     byte ptr [si + PD_XOFFSENT], 0
        mov     byte ptr [si + PD_TXIMM], 0
        ret


;; ====================================================================
;; ProgDivisor — Write divisor latch from PD_BAUDDIV
;; ====================================================================

ProgDivisor:
        pushf
        cli
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        in      al, dx
        push    ax
        or      al, LCR_DLAB
        out     dx, al

        mov     dx, [si + PD_IOBASE]
        mov     ax, [si + PD_BAUDDIV]
        out     dx, al                  ; DLL
        inc     dx
        mov     al, ah
        out     dx, al                  ; DLH

        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        pop     ax
        out     dx, al
        popf
        ret


;; ====================================================================
;; GetPort — get SI → port data for current request
;; ====================================================================
;; Placeholder: returns port 0. Full impl would match device header.

GetPort:
        ; Match the request packet's device header to a port index.
        ; The request packet doesn't directly contain the header pointer,
        ; but each COMn header has a unique SDevName. We compare the
        ; device name field from the SFN or use the unit code if available.
        ; Simplified: use the device name byte at header+11 ('1'..'4')
        ; which we patched during INIT. The kernel passes the header
        ; address in the request packet at offset 14h for OPEN.
        ; For other commands, we track the port index at open time.
        ;
        ; For now: scan COM headers to find which one matches
        push    ax
        push    cx
        push    di
        les     bx, dword ptr ds:[RPOff]

        ; Try to get unit number from request — use a simple heuristic:
        ; The SFN (System File Number) or the device header pointer.
        ; OS/2 stores the device header address at request+14h for Open.
        ; For other commands, we need to track it. Use a port map.
        ; 
        ; Practical approach: scan PortArray for matching IOBASE with
        ; the BIOS COM port table at 0040:0000 indexed by unit.
        ; Or simply: the kernel opens devices in order, so we track
        ; the last-opened port index.

        ; Default: use stored current port index
        xor     ax, ax
        mov     al, ds:[CurPortIdx]
        mov     cx, PD_SIZE
        mul     cx
        lea     si, ds:[PortArray]
        add     si, ax
        pop     di
        pop     cx
        pop     ax
        ret

; Current port index (set during Open based on device name)
CurPortIdx      DB      0


;; ====================================================================
;; OPEN
;; ====================================================================

DoOpen:
        ; Determine which port is being opened from the device header
        ; The request packet's device header can be matched to our COM headers
        push    es
        push    bx
        les     bx, dword ptr ds:[RPOff]
        ; Scan COM headers to find the match
        ; For simplicity: check if caller opened COM1..COM4 by examining
        ; the device name in the system file table
        ; Practical: set CurPortIdx from the filename
        mov     byte ptr ds:[CurPortIdx], 0  ; Default COM1
        ; Check COM2 header address
        lea     ax, ds:[COM2Hdr]
        ; TODO: full header matching requires OS/2 SFT access
        ; For now, use sequential port assignment
        pop     bx
        pop     es

        call    GetPort
        test    word ptr [si + PD_FLAGS], PF_EXISTS
        jz      opErr

        cmp     word ptr [si + PD_OPENCOUNT], 0
        jne     opAdditional

        ; Initial open: init UART, set DTR/RTS, claim IRQ
        call    InitUART

        ; Reset buffers
        mov     word ptr [si + PD_RXHEAD], 0
        mov     word ptr [si + PD_RXTAIL], 0
        mov     word ptr [si + PD_RXCOUNT], 0
        mov     word ptr [si + PD_RXSIZE], RXBUFSZ
        mov     word ptr [si + PD_TXHEAD], 0
        mov     word ptr [si + PD_TXTAIL], 0
        mov     word ptr [si + PD_TXCOUNT], 0
        mov     word ptr [si + PD_TXSIZE], TXBUFSZ

        ; Set DTR if enabled
        mov     al, [si + PD_DCB_F1]
        and     al, F1_DTRMASK
        cmp     al, F1_DTRENA
        jne     opNoDTR
        or      byte ptr [si + PD_MCR], MCR_DTR
        call    WriteMCR
opNoDTR:
        ; Set RTS if enabled
        mov     al, [si + PD_DCB_F2]
        and     al, F2_RTSMASK
        cmp     al, F2_RTSENA
        jne     opNoRTS
        or      byte ptr [si + PD_MCR], MCR_RTS
        call    WriteMCR
opNoRTS:
        ; Claim IRQ via DevHlp_SetIRQ
        cmp     byte ptr [si + PD_IRQ], 0FFh
        je      opSkipIRQ
        push    bx
        xor     bh, bh
        mov     bl, [si + PD_IRQ]
        mov     ax, OFFSET _TEXT:SIOInt
        mov     dh, 1                   ; Shared
        mov     dl, DH_SETIRQ
        call    dword ptr ds:[DevHlp]
        pop     bx
        jc      opIRQFail
        or      word ptr [si + PD_FLAGS], PF_IRQCLAIMED
opSkipIRQ:

        ; Enable UART interrupts
        mov     dx, [si + PD_IOBASE]
        add     dx, U_IER
        mov     al, IER_ALL
        out     dx, al

        or      word ptr [si + PD_FLAGS], PF_OPEN
        mov     word ptr [si + PD_OPENCOUNT], 1
        xor     ax, ax
        jmp     sExit

opAdditional:
        inc     word ptr [si + PD_OPENCOUNT]
        xor     ax, ax
        jmp     sExit

opIRQFail:
        mov     ax, 8110h               ; Port in use (IRQ unavailable)
        jmp     sExit

opErr:
        mov     ax, 8110h
        jmp     sExit


;; ====================================================================
;; CLOSE
;; ====================================================================

DoClose:
        call    GetPort
        test    word ptr [si + PD_FLAGS], PF_OPEN
        jz      clErr

        dec     word ptr [si + PD_OPENCOUNT]
        cmp     word ptr [si + PD_OPENCOUNT], 0
        jne     clDone

        ; Last close: disable IRQs, drop DTR/RTS
        mov     dx, [si + PD_IOBASE]
        add     dx, U_IER
        xor     al, al
        out     dx, al

        ; Release IRQ
        test    word ptr [si + PD_FLAGS], PF_IRQCLAIMED
        jz      clNoIRQ
        push    bx
        xor     bh, bh
        mov     bl, [si + PD_IRQ]
        mov     dl, DH_UNSETIRQ
        call    dword ptr ds:[DevHlp]
        pop     bx
        and     word ptr [si + PD_FLAGS], not PF_IRQCLAIMED
clNoIRQ:

        ; Drop DTR if enabled mode
        mov     al, [si + PD_DCB_F1]
        and     al, F1_DTRMASK
        cmp     al, F1_DTRENA
        jne     clNoDTR
        and     byte ptr [si + PD_MCR], not MCR_DTR
        call    WriteMCR
clNoDTR:
        ; Drop RTS
        mov     al, [si + PD_DCB_F2]
        and     al, F2_RTSMASK
        cmp     al, F2_RTSENA
        jne     clNoRTS
        and     byte ptr [si + PD_MCR], not MCR_RTS
        call    WriteMCR
clNoRTS:
        and     word ptr [si + PD_FLAGS], not PF_OPEN
clDone:
        xor     ax, ax
        jmp     sExit
clErr:
        mov     ax, 8106h
        jmp     sExit


;; ====================================================================
;; WriteMCR — write MCR shadow to hardware
;; ====================================================================

WriteMCR:
        push    dx
        mov     dx, [si + PD_IOBASE]
        add     dx, U_MCR
        mov     al, [si + PD_MCR]
        out     dx, al
        pop     dx
        ret


;; ====================================================================
;; READ (stub — returns 0 bytes, sets status OK)
;; ====================================================================

DoRead:
        call    GetPort
        test    word ptr [si + PD_FLAGS], PF_OPEN
        jz      rdErr

        les     bx, dword ptr ds:[RPOff]
        mov     cx, es:[bx+12h]         ; Requested count
        test    cx, cx
        jz      rdZero

        ; Read from RX ring buffer
        xor     dx, dx                  ; Bytes transferred
        ; Map caller's transfer buffer via DevHlp_PhysToVirt
        push    cx
        push    dx
        les     bx, dword ptr ds:[RPOff]
        mov     eax, es:[bx+0Eh]        ; Physical transfer address
        movzx   ecx, word ptr es:[bx+12h]
        mov     dh, 0                    ; Read/write, result in ES:BX
        mov     dl, DH_PHYSTOVIRT
        call    dword ptr ds:[DevHlp]
        mov     ds:[XferOff], bx
        mov     ds:[XferSeg], es
        pop     dx
        pop     cx
        ; Restore DS
        mov     ax, SEG _DATA
        mov     ds, ax

rdLoop:
        cmp     dx, cx
        jae     rdDone

        cmp     word ptr [si + PD_RXCOUNT], 0
        je      rdDone

        ; Get byte from RX buffer head
        push    bx
        push    es
        les     bx, dword ptr [si + PD_RXBASE]
        add     bx, [si + PD_RXHEAD]
        mov     al, es:[bx]
        pop     es
        pop     bx

        ; Advance head (CLI/STI protects rbCount vs ISR)
        push    ax
        inc     word ptr [si + PD_RXHEAD]
        mov     ax, [si + PD_RXHEAD]
        cmp     ax, [si + PD_RXSIZE]
        jb      rdNoWrap
        mov     word ptr [si + PD_RXHEAD], 0
rdNoWrap:
        cli
        dec     word ptr [si + PD_RXCOUNT]
        sti
        pop     ax

        ; Store byte to caller's mapped buffer
        push    es
        push    bx
        les     bx, dword ptr ds:[XferOff]
        add     bx, dx
        mov     es:[bx], al
        pop     bx
        pop     es

        inc     dx
        jmp     rdLoop

rdDone:
        les     bx, dword ptr ds:[RPOff]
        mov     es:[bx+12h], dx         ; Actual bytes read
        xor     ax, ax
        jmp     sExit
rdZero:
        xor     ax, ax
        jmp     sExit
rdErr:
        mov     ax, 8106h
        jmp     sExit


;; ====================================================================
;; WRITE (stub — accepts all bytes, kicks TX)
;; ====================================================================

DoWrite:
        call    GetPort
        test    word ptr [si + PD_FLAGS], PF_OPEN
        jz      wrErr

        les     bx, dword ptr ds:[RPOff]
        mov     cx, es:[bx+12h]         ; Byte count
        test    cx, cx
        jz      wrZero

        ; Map caller's transfer buffer
        push    cx
        push    dx
        mov     eax, es:[bx+0Eh]
        movzx   ecx, word ptr es:[bx+12h]
        mov     dh, 0
        mov     dl, DH_PHYSTOVIRT
        call    dword ptr ds:[DevHlp]
        mov     ds:[XferOff], bx
        mov     ds:[XferSeg], es
        pop     dx
        pop     cx
        mov     ax, SEG _DATA
        mov     ds, ax

        xor     dx, dx                  ; Bytes written
wrLoop:
        cmp     dx, cx
        jae     wrDone

        ; Check if TX buffer has space
        mov     ax, [si + PD_TXCOUNT]
        cmp     ax, [si + PD_TXSIZE]
        jae     wrDone                  ; Buffer full — return partial

        ; Read byte from caller's mapped buffer
        push    es
        push    bx
        les     bx, dword ptr ds:[XferOff]
        add     bx, dx
        mov     al, es:[bx]
        pop     bx
        pop     es

        ; Put byte in TX buffer tail
        push    bx
        push    es
        les     bx, dword ptr [si + PD_TXBASE]
        add     bx, [si + PD_TXTAIL]
        mov     es:[bx], al
        pop     es
        pop     bx

        ; Advance tail
        inc     word ptr [si + PD_TXTAIL]
        mov     ax, [si + PD_TXTAIL]
        cmp     ax, [si + PD_TXSIZE]
        jb      wrNoWrap
        mov     word ptr [si + PD_TXTAIL], 0
wrNoWrap:
        cli
        inc     word ptr [si + PD_TXCOUNT]
        sti
        inc     dx
        jmp     wrLoop

wrDone:
        ; Note (wrench audit): DevHlp_ProcRun safely ignores stale
        ; block IDs, so no validity check needed before calling.
        ; Strategy routines use ProcBlock with timeout as defense.

        ; Kick transmitter if idle
        cmp     byte ptr [si + PD_TXHOLD], 0
        jne     wrNoKick
        cmp     byte ptr [si + PD_XOFFRECVD], 0
        jne     wrNoKick
        ; Check THRE
        push    dx
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LSR
        in      al, dx
        pop     dx
        test    al, LSR_THRE
        jz      wrNoKick
        ; Send bytes from buffer — fill FIFO if available
        call    KickTxBatch
wrNoKick:
        les     bx, dword ptr ds:[RPOff]
        mov     es:[bx+12h], dx         ; Actual bytes written
        xor     ax, ax
        jmp     sExit
wrZero:
        xor     ax, ax
        jmp     sExit
wrErr:
        mov     ax, 8106h
        jmp     sExit


;; ====================================================================
;; Non-Destructive Read
;; ====================================================================

DoNDR:
        call    GetPort
        cmp     word ptr [si + PD_RXCOUNT], 0
        je      ndrEmpty
        ; Would peek from RX buffer
        les     bx, dword ptr ds:[RPOff]
        mov     byte ptr es:[bx+0Dh], 0
        xor     ax, ax
        jmp     sExit
ndrEmpty:
        mov     ax, 0200h               ; BUSY
        jmp     sExit


;; ====================================================================
;; Input/Output Status and Flush
;; ====================================================================

DoInStat:
        call    GetPort
        cmp     word ptr [si + PD_RXCOUNT], 0
        jnz     isReady
        mov     ax, 0200h               ; BUSY
        jmp     sExit
isReady:
        xor     ax, ax
        jmp     sExit

DoInFlush:
        call    GetPort
        mov     word ptr [si + PD_RXHEAD], 0
        mov     word ptr [si + PD_RXTAIL], 0
        mov     word ptr [si + PD_RXCOUNT], 0
        xor     ax, ax
        jmp     sExit

DoOutStat:
        call    GetPort
        ; Busy if > 75% full
        mov     cx, [si + PD_TXCOUNT]
        mov     ax, [si + PD_TXSIZE]
        shr     ax, 2
        mov     dx, ax
        shl     ax, 1
        add     dx, ax                  ; dx = 75%
        cmp     cx, dx
        jae     osBusy
        xor     ax, ax
        jmp     sExit
osBusy:
        mov     ax, 0200h
        jmp     sExit

DoOutFlush:
        call    GetPort
        mov     word ptr [si + PD_TXHEAD], 0
        mov     word ptr [si + PD_TXTAIL], 0
        mov     word ptr [si + PD_TXCOUNT], 0
        xor     ax, ax
        jmp     sExit


;; ====================================================================
;; GENERIC IOCtl — All 20+ functions
;; ====================================================================

DoIOCtl:
        call    GetPort
        les     bx, dword ptr ds:[RPOff]

        cmp     byte ptr es:[bx+0Dh], IOCTL_ASYNC
        jne     ioBadCat

        mov     al, es:[bx+0Eh]         ; function code

        ; Save param/data packet pointers
        push    es
        push    bx
        les     bx, es:[bx+0Fh]
        mov     ds:[IOPrmOff], bx
        mov     ds:[IOPrmSeg], es
        pop     bx
        pop     es
        push    es
        push    bx
        les     bx, es:[bx+13h]
        mov     ds:[IODatOff], bx
        mov     ds:[IODatSeg], es
        pop     bx
        pop     es

        ; Dispatch — use sub-range jumps to stay within short range
        cmp     al, 50h
        jb      ioSet            ; 41h-4Bh
        jmp     ioQuery          ; 53h-74h

ioSet:
        cmp     al, 41h
        je      io41
        cmp     al, 42h
        je      io42
        cmp     al, 43h
        je      io43
        cmp     al, 44h
        je      io44
        cmp     al, 45h
        je      io45
        cmp     al, 46h
        je      io46
        cmp     al, 47h
        je      io47
        cmp     al, 48h
        je      io48
        cmp     al, 4Bh
        je      io4B
        jmp     ioBadCat

ioQuery:
        cmp     al, 53h
        je      io53
        cmp     al, 54h
        je      io54
        cmp     al, 70h
        jb      ioQuery2
        jmp     ioQuery3
ioQuery2:
        cmp     al, 61h
        je      io61
        cmp     al, 62h
        je      io62
        cmp     al, 63h
        je      io63
        cmp     al, 64h
        je      io64
        cmp     al, 65h
        je      io65
        cmp     al, 66h
        je      io66
        cmp     al, 67h
        je      io67
        cmp     al, 68h
        je      io68
        cmp     al, 69h
        je      io69
        cmp     al, 6Dh
        je      io6D
        jmp     ioBadCat
ioQuery3:
        cmp     al, 72h
        je      io72
        cmp     al, 73h
        je      io73
        cmp     al, 74h
        je      io74
        jmp     ioBadCat

ioBadCat:
        mov     ax, 8103h
        jmp     sExit

ioOK:
        xor     ax, ax
        jmp     sExit

;; --- 41h Set Baud Rate ---
io41:
        les     bx, dword ptr ds:[IOPrmOff]
        movzx   eax, word ptr es:[bx]
        cmp     eax, 50
        jb      ioBadCat
        mov     dword ptr [si + PD_BAUDRATE], eax
        push    eax
        mov     eax, BAUDCLK
        xor     edx, edx
        pop     ecx
        div     ecx
        mov     [si + PD_BAUDDIV], ax
        call    ProgDivisor
        jmp     ioOK

;; --- 42h Set Line Control ---
io42:
        les     bx, dword ptr ds:[IOPrmOff]
        mov     cl, es:[bx]             ; data bits
        mov     ch, es:[bx+1]           ; parity
        mov     dl, es:[bx+2]           ; stop bits
        xor     al, al
        sub     cl, 5
        or      al, cl
        cmp     ch, 0
        je      io42np
        or      al, LCR_PEN
        cmp     ch, 2
        jne     io42ne
        or      al, LCR_EPS
        jmp     io42np
io42ne: cmp     ch, 3
        jne     io42nm
        or      al, LCR_STICK
        jmp     io42np
io42nm: cmp     ch, 4
        jne     io42np
        or      al, LCR_EPS or LCR_STICK
io42np: cmp     dl, 0
        je      io42ns
        or      al, LCR_STOP2
io42ns: mov     [si + PD_LINECTRL], al
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        out     dx, al
        jmp     ioOK

;; --- 43h Extended Set Baud ---
io43:
        les     bx, dword ptr ds:[IOPrmOff]
        mov     eax, es:[bx]
        cmp     eax, 50
        jb      ioBadCat
        mov     dword ptr [si + PD_BAUDRATE], eax
        push    eax
        mov     eax, BAUDCLK
        xor     edx, edx
        pop     ecx
        div     ecx
        mov     [si + PD_BAUDDIV], ax
        call    ProgDivisor
        jmp     ioOK

;; --- 44h Transmit Immediate ---
io44:
        les     bx, dword ptr ds:[IOPrmOff]
        mov     al, es:[bx]
        mov     [si + PD_TXIMMCH], al
        mov     byte ptr [si + PD_TXIMM], 1
        jmp     ioOK

;; --- 45h Set Break Off ---
io45:
        and     word ptr [si + PD_FLAGS], not PF_BREAKON
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        in      al, dx
        and     al, not LCR_BREAK
        out     dx, al
        call    RetErrWord
        jmp     ioOK

;; --- 46h Set Modem Control ---
io46:
        les     bx, dword ptr ds:[IOPrmOff]
        mov     cl, es:[bx]             ; on mask
        mov     ch, es:[bx+1]           ; off mask
        mov     al, [si + PD_MCR]
        ; DTR
        test    cl, 01h
        jz      io46dOff
        or      al, MCR_DTR
        jmp     io46rts
io46dOff:
        test    ch, 01h
        jnz     io46rts
        and     al, not MCR_DTR
io46rts:
        test    cl, 02h
        jz      io46rOff
        or      al, MCR_RTS
        jmp     io46wr
io46rOff:
        test    ch, 02h
        jnz     io46wr
        and     al, not MCR_RTS
io46wr: mov     [si + PD_MCR], al
        call    WriteMCR
        call    RetErrWord
        jmp     ioOK

;; --- 47h Hold Transmit ---
io47:
        mov     byte ptr [si + PD_TXHOLD], 1
        jmp     ioOK

;; --- 48h Start Transmit ---
io48:
        mov     byte ptr [si + PD_TXHOLD], 0
        jmp     ioOK

;; --- 4Bh Set Break On ---
io4B:
        or      word ptr [si + PD_FLAGS], PF_BREAKON
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LCR
        in      al, dx
        or      al, LCR_BREAK
        out     dx, al
        call    RetErrWord
        jmp     ioOK

;; --- 53h Write DCB ---
io53:
        les     bx, dword ptr ds:[IOPrmOff]
        push    di
        lea     di, [si + PD_DCB]
        mov     cx, 11
io53cp: mov     al, es:[bx]
        mov     [di], al
        inc     bx
        inc     di
        loop    io53cp
        pop     di
        ; Force flags3 bits
        and     byte ptr [si + PD_DCB_F3], 07h
        or      byte ptr [si + PD_DCB_F3], F3_FORCED
        jmp     ioOK

;; --- 54h Write Enhanced (ignored) ---
io54:
        jmp     ioOK

;; --- 61h Query Baud Rate ---
io61:
        les     bx, dword ptr ds:[IODatOff]
        mov     ax, word ptr [si + PD_BAUDRATE]
        mov     es:[bx], ax
        jmp     ioOK

;; --- 62h Query Line Control ---
io62:
        les     bx, dword ptr ds:[IODatOff]
        mov     al, [si + PD_LINECTRL]
        ; Data bits
        mov     cl, al
        and     cl, LCR_WMASK
        add     cl, 5
        mov     es:[bx], cl
        ; Parity
        test    al, LCR_PEN
        jz      io62pn
        test    al, LCR_STICK
        jnz     io62ps
        test    al, LCR_EPS
        jnz     io62pe
        mov     byte ptr es:[bx+1], 1   ; odd
        jmp     io62sd
io62pe: mov     byte ptr es:[bx+1], 2   ; even
        jmp     io62sd
io62ps: test    al, LCR_EPS
        jnz     io62sp
        mov     byte ptr es:[bx+1], 3   ; mark
        jmp     io62sd
io62sp: mov     byte ptr es:[bx+1], 4   ; space
        jmp     io62sd
io62pn: mov     byte ptr es:[bx+1], 0   ; none
io62sd: ; Stop bits
        test    al, LCR_STOP2
        jnz     io62s2
        mov     byte ptr es:[bx+2], 0
        jmp     io62br
io62s2: mov     byte ptr es:[bx+2], 2
io62br: ; Break
        test    word ptr [si + PD_FLAGS], PF_BREAKON
        jnz     io62by
        mov     byte ptr es:[bx+3], 0
        jmp     ioOK
io62by: mov     byte ptr es:[bx+3], 1
        jmp     ioOK

;; --- 63h Extended Query Baud ---
io63:
        les     bx, dword ptr ds:[IODatOff]
        mov     eax, [si + PD_BAUDRATE]
        mov     es:[bx], eax
        mov     byte ptr es:[bx+4], 0
        mov     dword ptr es:[bx+5], 50
        mov     byte ptr es:[bx+9], 0
        mov     dword ptr es:[bx+10], 115200
        mov     byte ptr es:[bx+14], 0
        jmp     ioOK

;; --- 64h Query Status ---
io64:
        les     bx, dword ptr ds:[IODatOff]
        xor     al, al
        test    byte ptr [si + PD_DCB_F1], F1_CTSENA
        jz      io64nc
        test    byte ptr [si + PD_MSR], MSR_CTS
        jnz     io64nc
        or      al, 01h
io64nc: cmp     byte ptr [si + PD_TXHOLD], 0
        je      io64nx
        or      al, 08h
io64nx: cmp     byte ptr [si + PD_XOFFRECVD], 0
        je      io64nb
        or      al, 08h
io64nb: test    word ptr [si + PD_FLAGS], PF_BREAKON
        jz      io64ni
        or      al, 20h
io64ni: cmp     byte ptr [si + PD_TXIMM], 0
        je      io64done
        or      al, 40h
io64done:
        mov     es:[bx], al
        jmp     ioOK

;; --- 65h Query TX Status ---
io65:
        les     bx, dword ptr ds:[IODatOff]
        xor     al, al
        cmp     word ptr [si + PD_TXCOUNT], 0
        je      io65nt
        or      al, 02h
io65nt: cmp     byte ptr [si + PD_TXIMM], 0
        je      io65d
        or      al, 08h
io65d:  mov     es:[bx], al
        jmp     ioOK

;; --- 66h Query Modem Output ---
io66:
        les     bx, dword ptr ds:[IODatOff]
        mov     al, [si + PD_MCR]
        and     al, MCR_DTR or MCR_RTS
        mov     es:[bx], al
        jmp     ioOK

;; --- 67h Query Modem Input ---
io67:
        les     bx, dword ptr ds:[IODatOff]
        push    dx
        mov     dx, [si + PD_IOBASE]
        add     dx, U_MSR
        in      al, dx
        pop     dx
        mov     [si + PD_MSR], al
        and     al, MSR_CTS or MSR_DSR or MSR_RI or MSR_DCD
        mov     es:[bx], al
        jmp     ioOK

;; --- 68h Query RX Count ---
io68:
        les     bx, dword ptr ds:[IODatOff]
        mov     ax, [si + PD_RXCOUNT]
        mov     es:[bx], ax
        mov     ax, [si + PD_RXSIZE]
        mov     es:[bx+2], ax
        jmp     ioOK

;; --- 69h Query TX Count ---
io69:
        les     bx, dword ptr ds:[IODatOff]
        mov     ax, [si + PD_TXCOUNT]
        mov     es:[bx], ax
        mov     ax, [si + PD_TXSIZE]
        mov     es:[bx+2], ax
        jmp     ioOK

;; --- 6Dh Query Error (resets after read) ---
io6D:
        les     bx, dword ptr ds:[IODatOff]
        mov     ax, [si + PD_ERROR]
        mov     es:[bx], ax
        mov     word ptr [si + PD_ERROR], 0
        jmp     ioOK

;; --- 72h Query Event (resets after read) ---
io72:
        les     bx, dword ptr ds:[IODatOff]
        mov     ax, [si + PD_EVENT]
        mov     es:[bx], ax
        mov     word ptr [si + PD_EVENT], 0
        jmp     ioOK

;; --- 73h Read DCB ---
io73:
        les     bx, dword ptr ds:[IODatOff]
        push    di
        lea     di, [si + PD_DCB]
        mov     cx, 11
io73cp: mov     al, [di]
        mov     es:[bx], al
        inc     di
        inc     bx
        loop    io73cp
        pop     di
        jmp     ioOK

;; --- 74h Read Enhanced ---
io74:
        les     bx, dword ptr ds:[IODatOff]
        mov     byte ptr es:[bx], 0
        mov     dword ptr es:[bx+1], 0
        jmp     ioOK


;; ====================================================================
;; RetErrWord — write error word to data packet
;; ====================================================================

RetErrWord:
        push    es
        push    bx
        les     bx, dword ptr ds:[IODatOff]
        mov     ax, [si + PD_ERROR]
        mov     es:[bx], ax
        pop     bx
        pop     es
        ret


;; ====================================================================
;; ISR — Interrupt Service Routine
;; ====================================================================

        PUBLIC  SIOInt
SIOInt:
        push    ds
        push    es
        push    eax
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        mov     ax, SEG _DATA
        mov     ds, ax
        mov     word ptr ds:[ISRFlag], 0

        mov     cx, ds:[NumPorts]
        test    cx, cx
        jz      intNotOurs
        lea     si, ds:[PortArray]

intScan:
        push    cx
        test    word ptr [si + PD_FLAGS], PF_OPEN
        jz      intNext

        mov     dx, [si + PD_IOBASE]
        add     dx, U_IIR
        in      al, dx
        test    al, IIR_NOPEND
        jnz     intNext

        ; Service this port
        call    SvcPort
        mov     word ptr ds:[ISRFlag], 1

intNext:
        add     si, PD_SIZE
        pop     cx
        loop    intScan

        cmp     word ptr ds:[ISRFlag], 0
        je      intNotOurs

        ; EOI
        mov     dl, DH_EOI
        call    dword ptr ds:[DevHlp]

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     es
        pop     ds
        clc
        retf

intNotOurs:
        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     es
        pop     ds
        stc
        retf


;; ====================================================================
;; SvcPort — service one port's pending interrupts
;; ====================================================================

SvcPort:
spLoop:
        mov     dx, [si + PD_IOBASE]
        add     dx, U_IIR
        in      al, dx
        test    al, IIR_NOPEND
        jnz     spDone
        and     al, IIR_IDMASK

        cmp     al, IIR_RLS
        je      spRLS
        cmp     al, IIR_RDA
        je      spRDA
        cmp     al, IIR_TIMEOUT
        je      spRDA
        cmp     al, IIR_THRE
        je      spTHRE
        cmp     al, IIR_MODEM
        je      spMSR
        jmp     near ptr spLoop

; Line status
spRLS:
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LSR
        in      al, dx
        or      [si + PD_LSR], al
        test    al, LSR_OE
        jz      spRLSpe
        or      word ptr [si + PD_ERROR], 0002h
spRLSpe:
        test    al, LSR_PE
        jz      spRLSfe
        or      word ptr [si + PD_ERROR], 0004h
spRLSfe:
        test    al, LSR_FE
        jz      spRLSbi
        or      word ptr [si + PD_ERROR], 0008h
spRLSbi:
        test    al, LSR_BI
        jz      spRLSev
        or      word ptr [si + PD_EVENT], 0040h
spRLSev:
        test    al, LSR_OE or LSR_PE or LSR_FE
        jz      spLpJ
        or      word ptr [si + PD_EVENT], 0080h
        jmp     near ptr spLoop

; Received data
spRDA:
        push    cx
        mov     cx, 256                 ; Max bytes per ISR — prevents HW fault loop

spRDAretry:
        mov     dx, [si + PD_IOBASE]
        add     dx, U_LSR
        in      al, dx
        test    al, LSR_DR
        jz      spRDAdone2

        dec     cx
        jz      spRDAdone2              ; Hit safety limit

        mov     dx, [si + PD_IOBASE]
        in      al, dx                  ; Read byte from RBR

        ; Check XON/XOFF
        test    byte ptr [si + PD_DCB_F2], F2_XONRX
        jz      spRDAstore

        cmp     al, [si + PD_DCB_XOFF]
        jne     spRDAchkXon
        mov     byte ptr [si + PD_XOFFRECVD], 1
        jmp     spRDAretry

spRDAchkXon:
        cmp     al, [si + PD_DCB_XON]
        jne     spRDAstore
        mov     byte ptr [si + PD_XOFFRECVD], 0
        jmp     spRDAretry

spRDAstore:
        ; Store received byte (in AL) into RX ring buffer
        push    bx
        push    es
        cmp     word ptr [si + PD_RXCOUNT], RXBUFSZ
        jae     spRDAovf2

        ; Write byte to RX buffer at tail position
        les     bx, dword ptr [si + PD_RXBASE]
        add     bx, [si + PD_RXTAIL]
        mov     es:[bx], al
        pop     es

        inc     word ptr [si + PD_RXTAIL]
        cmp     word ptr [si + PD_RXTAIL], RXBUFSZ
        jb      spRDAnw
        mov     word ptr [si + PD_RXTAIL], 0
spRDAnw:
        cli
        inc     word ptr [si + PD_RXCOUNT]
        sti
        or      word ptr [si + PD_EVENT], 0001h
        pop     bx
        jmp     spRDAretry

spRDAdone2:
        pop     cx
        jmp     near ptr spLoop

spRDAdrained:
        pop     cx
        jmp     near ptr spLoop

spRDAovf2:
        pop     es
spRDAovf:
        or      word ptr [si + PD_ERROR], 0001h
        pop     bx
        jmp     spRDAretry

; TX empty
spTHRE:
        ; Check if held
        cmp     byte ptr [si + PD_TXHOLD], 0
        jne     spTHdone
        cmp     byte ptr [si + PD_XOFFRECVD], 0
        jne     spTHdone

        ; Check TX immediate
        cmp     byte ptr [si + PD_TXIMM], 0
        je      spTHbuf
        mov     byte ptr [si + PD_TXIMM], 0
        mov     al, [si + PD_TXIMMCH]
        mov     dx, [si + PD_IOBASE]
        out     dx, al
        jmp     spTHdone

spTHbuf:
        ; Send from TX buffer — fill FIFO using actual size
        cmp     word ptr [si + PD_TXCOUNT], 0
        je      spTHdone
        call    KickTxBatch
        or      word ptr [si + PD_EVENT], 0004h
spTHdone:
        jmp     near ptr spLoop

; Modem status
spMSR:
        mov     dx, [si + PD_IOBASE]
        add     dx, U_MSR
        in      al, dx
        mov     [si + PD_MSR], al
        test    al, MSR_DCTS
        jz      spMnodsr
        or      word ptr [si + PD_EVENT], 0008h
spMnodsr:
        test    al, MSR_DDSR
        jz      spMnodcd
        or      word ptr [si + PD_EVENT], 0010h
spMnodcd:
        test    al, MSR_DDCD
        jz      spMnori
        or      word ptr [si + PD_EVENT], 0020h
spMnori:
        test    al, MSR_TERI
        jz      spMdone
        or      word ptr [si + PD_EVENT], 0100h
spMdone:
        jmp     near ptr spLoop

spLpJ:  jmp     near ptr spLoop
spDone:
        ret



;; ====================================================================
;; ParseCmdLine — Parse CONFIG.SYS arguments
;; ====================================================================
;; In: ES:BX → argument string
;; Scans for (COMn[,addr[,irq]]) blocks

ParseCmdLine:
        push    si
pcScan:
        mov     al, es:[bx]
        test    al, al
        jz      pcDone
        cmp     al, '('
        je      pcFound
        inc     bx
        jmp     pcScan
pcFound:
        inc     bx                      ; Skip '('
        ; Expect COMn
        cmp     byte ptr es:[bx], 'C'
        jne     pcSkip
        cmp     byte ptr es:[bx+1], 'O'
        jne     pcSkip
        cmp     byte ptr es:[bx+2], 'M'
        jne     pcSkip
        add     bx, 3

        ; Get port number
        mov     al, es:[bx]
        sub     al, '0'
        xor     ah, ah
        inc     bx

        ; Set up port data at correct index
        push    ax
        mov     cx, PD_SIZE
        mul     cx                      ; Not quite right — 0-based
        pop     cx                      ; CX = port number
        push    cx

        ; Calculate slot: (portnum-1) * PD_SIZE
        dec     cl
        xor     ch, ch
        mov     ax, cx
        mov     cx, PD_SIZE
        mul     cx
        lea     si, ds:[PortArray]
        add     si, ax

        ; Set defaults for this port
        pop     cx                      ; CX = port number (1-based)
        push    cx
        dec     cl
        xor     ch, ch
        push    bx
        mov     bx, cx
        shl     bx, 1
        mov     ax, ds:[DefPorts + bx]
        mov     [si + PD_IOBASE], ax
        mov     [si + PD_IOBASEDOS], ax
        shr     bx, 1
        mov     al, ds:[DefIRQs + bx]
        mov     [si + PD_IRQ], al
        mov     [si + PD_IRQDOS], al
        pop     bx
        pop     cx

        ; Check for comma (optional addr)
        cmp     byte ptr es:[bx], ','
        jne     pcEndPort
        inc     bx
        ; Check for INTERNET keyword
        cmp     byte ptr es:[bx], 'I'
        jne     pcNotInet
        cmp     byte ptr es:[bx+1], 'N'
        jne     pcNotInet
        cmp     byte ptr es:[bx+2], 'T'
        jne     pcNotInet
        ; Skip "INTERNET"
        add     bx, 8
        or      word ptr [si + PD_FLAGS], PF_INTERNET
        mov     byte ptr [si + PD_ISINET], 1
        mov     byte ptr [si + PD_IRQ], 0FFh    ; No hardware IRQ
        ; Check for :dosaddr
        cmp     byte ptr es:[bx], ':'
        jne     pcInetDone
        inc     bx
        call    pcParseHex
        mov     [si + PD_IOBASEDOS], ax
pcInetDone:
        ; Skip to comma or close paren
        jmp     pcEndPort

pcNotInet:
        ; Parse hex address
        call    pcParseHex
        mov     [si + PD_IOBASE], ax
        mov     [si + PD_IOBASEDOS], ax

        ; Check for comma (optional IRQ)
        cmp     byte ptr es:[bx], ','
        jne     pcEndPort
        inc     bx
        ; Check for NONE
        cmp     byte ptr es:[bx], 'N'
        jne     pcIRQNum
        add     bx, 4                  ; Skip NONE
        mov     byte ptr [si + PD_IRQ], 0FFh
        jmp     pcEndPort
pcIRQNum:
        cmp     byte ptr es:[bx], 'I'
        jne     pcIRQDig
        add     bx, 3                  ; Skip IRQ
pcIRQDig:
        call    pcParseDec
        mov     [si + PD_IRQ], al
        mov     [si + PD_IRQDOS], al

pcEndPort:
        ; Update NumPorts if this is a new highest port
        inc     word ptr ds:[NumPorts]

pcSkip:
        ; Find closing ')'
pcFindClose:
        mov     al, es:[bx]
        test    al, al
        jz      pcDone
        cmp     al, ')'
        je      pcCloseParen
        inc     bx
        jmp     pcFindClose
pcCloseParen:
        inc     bx
        jmp     pcScan
pcDone:
        pop     si
        ret

; Parse decimal from ES:BX → AX
pcParseDec:
        xor     ax, ax
pcDecL: mov     cl, es:[bx]
        cmp     cl, '0'
        jb      pcDecD
        cmp     cl, '9'
        ja      pcDecD
        sub     cl, '0'
        push    dx
        mov     dx, 10
        mul     dx
        pop     dx
        xor     ch, ch
        add     ax, cx
        inc     bx
        jmp     pcDecL
pcDecD: ret

; Parse hex from ES:BX → AX
pcParseHex:
        xor     ax, ax
pcHexL: mov     cl, es:[bx]
        cmp     cl, '0'
        jb      pcHexD
        cmp     cl, '9'
        jbe     pcHexDig
        cmp     cl, 'A'
        jb      pcHexD
        cmp     cl, 'F'
        jbe     pcHexUp
        cmp     cl, 'a'
        jb      pcHexD
        cmp     cl, 'f'
        ja      pcHexD
        sub     cl, 'a' - 10
        jmp     pcHexAdd
pcHexUp:sub     cl, 'A' - 10
        jmp     pcHexAdd
pcHexDig:
        sub     cl, '0'
pcHexAdd:
        shl     ax, 4
        xor     ch, ch
        or      ax, cx
        inc     bx
        jmp     pcHexL
pcHexD: ret


;; ====================================================================
;; AllocBuffers — Allocate RX/TX ring buffer memory via DevHlp
;; ====================================================================

AllocBuffers:
        mov     cx, ds:[NumPorts]
        test    cx, cx
        jz      abDone
        lea     si, ds:[PortArray]

abLoop:
        push    cx
        test    word ptr [si + PD_FLAGS], PF_EXISTS
        jz      abNext

        ; Allocate RX buffer
        push    bx
        mov     bx, 0                   ; Above 1MB ok
        mov     eax, RXBUFSZ
        mov     dl, DH_ALLOCPHYS
        call    dword ptr ds:[DevHlp]
        pop     bx
        jc      abNext

        ; Convert physical → GDT selector
        push    bx
        mov     dl, DH_PHYSTOGDT
        mov     ecx, RXBUFSZ
        call    dword ptr ds:[DevHlp]
        pop     bx
        ; Store as selector:0 in RX base
        mov     word ptr [si + PD_RXBASE], 0
        mov     word ptr [si + PD_RXBASE + 2], ax
        mov     word ptr [si + PD_RXSIZE], RXBUFSZ

        ; Allocate TX buffer
        push    bx
        mov     bx, 0
        mov     eax, TXBUFSZ
        mov     dl, DH_ALLOCPHYS
        call    dword ptr ds:[DevHlp]
        pop     bx
        jc      abNext

        push    bx
        mov     dl, DH_PHYSTOGDT
        mov     ecx, TXBUFSZ
        call    dword ptr ds:[DevHlp]
        pop     bx
        mov     word ptr [si + PD_TXBASE], 0
        mov     word ptr [si + PD_TXBASE + 2], ax
        mov     word ptr [si + PD_TXSIZE], TXBUFSZ

abNext:
        add     si, PD_SIZE
        pop     cx
        loop    abLoop
abDone:
        ret


;; ====================================================================
;; KickTx — Send one byte from TX buffer to UART THR
;; ====================================================================

KickTxBatch:
        ;; Send up to FIFO-size bytes in one batch
        push    cx
        xor     cx, cx
        mov     cl, [si + PD_FIFOSZ]
        test    cl, cl
        jnz     ktbLoop
        mov     cl, 1                   ; No FIFO: 1 byte at a time
ktbLoop:
        cmp     word ptr [si + PD_TXCOUNT], 0
        je      ktbDone
        call    KickTx
        dec     cl
        jnz     ktbLoop
ktbDone:
        pop     cx
        ret

KickTx:
        cmp     word ptr [si + PD_TXCOUNT], 0
        je      ktDone

        ; Get byte from TX head
        push    es
        push    bx
        les     bx, dword ptr [si + PD_TXBASE]
        add     bx, [si + PD_TXHEAD]
        mov     al, es:[bx]
        pop     bx
        pop     es

        ; Write to THR
        push    dx
        mov     dx, [si + PD_IOBASE]
        out     dx, al
        pop     dx

        ; Advance head
        inc     word ptr [si + PD_TXHEAD]
        mov     ax, [si + PD_TXHEAD]
        cmp     ax, [si + PD_TXSIZE]
        jb      ktNoWrap
        mov     word ptr [si + PD_TXHEAD], 0
ktNoWrap:
        cli
        dec     word ptr [si + PD_TXCOUNT]
        sti
ktDone:
        ret



;; ====================================================================
;; InitHeaders — fix COMn device header chain for actual port count
;; ====================================================================

InitHeaders:
        mov     cx, ds:[NumPorts]
        cmp     cx, 4
        jbe     ihOK
        mov     cx, 4                   ; Max 4 COM headers
ihOK:
        cmp     cx, 0
        je      ihNone

        ; Terminate chain at the last configured port
        cmp     cx, 1
        je      ihCap1
        cmp     cx, 2
        je      ihCap2
        cmp     cx, 3
        je      ihCap3
        jmp     ihDone                  ; 4 ports = full chain, already -1 terminated

ihCap1: mov     dword ptr ds:[COM1Hdr], -1
        jmp     ihDone
ihCap2: mov     dword ptr ds:[COM2Hdr], -1
        jmp     ihDone
ihCap3: mov     dword ptr ds:[COM3Hdr], -1
        jmp     ihDone

ihNone: ; No ports — terminate SIO$ header
        mov     dword ptr ds:[SIOHdr], -1
ihDone:
        ret



;; ====================================================================
;; LogInit — Open/create log file during INIT
;; ====================================================================

LogInit:
        ;; Use INT 21h to create/open \SIO.LOG (works during driver INIT)
        push    dx
        push    ax
        push    cx
        mov     ah, 3Ch                 ; DOS: create file
        mov     cx, 0                   ; Normal attributes
        mov     dx, OFFSET _DATA:LogFile
        int     21h
        jc      liDone
        mov     ds:[LogHandle], ax
liDone:
        pop     cx
        pop     ax
        pop     dx
        ret


;; ====================================================================
;; LogWrite — Write a message to the log file
;; ====================================================================
;; In: DS:DX → null-terminated string

LogWrite:
        cmp     word ptr ds:[LogHandle], 0FFFFh
        je      lwDone
        push    ax
        push    bx
        push    cx
        push    si

        ;; Count string length
        mov     si, dx
        xor     cx, cx
lwLen:  cmp     byte ptr [si], 0
        je      lwGotLen
        inc     si
        inc     cx
        jmp     lwLen
lwGotLen:
        ;; Write to file
        mov     bx, ds:[LogHandle]
        mov     ah, 40h                 ; DOS: write to file
        int     21h

        ;; Write CRLF
        push    dx
        mov     dx, OFFSET _DATA:LogCRLF
        mov     cx, 2
        mov     ah, 40h
        int     21h
        pop     dx

        pop     si
        pop     cx
        pop     bx
        pop     ax
lwDone:
        ret

;; ====================================================================
;; LogClose — Close log file
;; ====================================================================

LogClose:
        cmp     word ptr ds:[LogHandle], 0FFFFh
        je      lcDone
        push    bx
        push    ax
        mov     bx, ds:[LogHandle]
        mov     ah, 3Eh                 ; DOS: close file
        int     21h
        mov     word ptr ds:[LogHandle], 0FFFFh
        pop     ax
        pop     bx
lcDone:
        ret

LogCRLF         DB      0Dh, 0Ah
LogDetect       DB      '  COM'
LogDetNum       DB      '?'
LogDetMsg       DB      ': detected', 0
LogNoDetMsg     DB      ': not found', 0
LogInitOK       DB      'SIO v1.00 initialized', 0


EndOfCode       LABEL   BYTE

_TEXT   ENDS
        END
