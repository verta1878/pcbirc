;; ====================================================================
;; siodata.inc — SIO internal data structures
;; ====================================================================
;; Per-port state, ring buffers, DCB, and global driver state.
;; ====================================================================

;; ---------- Device Control Block (matches OS/2 ASYNC spec) ----------

SIODCB  STRUC
dcbWriteTimeout DW      ?       ; Write timeout (1/100 sec, 0-based)
dcbReadTimeout  DW      ?       ; Read timeout (1/100 sec, 0-based)
dcbFlags1       DB      ?       ; DTR mode, CTS/DSR/DCD enables
dcbFlags2       DB      ?       ; XON/XOFF, RTS mode
dcbFlags3       DB      ?       ; Timeout modes, FIFO, trigger, TX load
dcbErrorRepl    DB      ?       ; Error replacement byte (unused)
dcbBreakRepl    DB      ?       ; Break replacement byte (unused)
dcbXonChar      DB      ?       ; XON character (default 11h)
dcbXoffChar     DB      ?       ; XOFF character (default 13h)
SIODCB  ENDS

SIODCB_SIZE     EQU     SIZE SIODCB     ; 11 bytes

;; DCB Flags1 bit definitions
DCB_F1_DTR_MASK EQU     03h     ; DTR Control Mode [1:0]
DCB_F1_DTR_DIS  EQU     00h     ;   00 = disable DTR changes at open/close
DCB_F1_DTR_ENA  EQU     01h     ;   01 = enable DTR changes at open/close
DCB_F1_DTR_HS   EQU     02h     ;   10 = DTR input handshake (not supported)
DCB_F1_CTS_ENA  EQU     08h     ; Bit 3: CTS required to transmit
DCB_F1_DSR_HS   EQU     10h     ; Bit 4: DSR handshake (not supported)
DCB_F1_DCD_TX   EQU     20h     ; Bit 5: DCD required to transmit (not supported)
DCB_F1_DSR_RX   EQU     40h     ; Bit 6: DSR required to receive

;; DCB Flags2 bit definitions
DCB_F2_XON_RX   EQU     01h     ; Bit 0: honor received XON/XOFF
DCB_F2_XON_TX   EQU     02h     ; Bit 1: may send XON/XOFF
DCB_F2_RTS_MASK EQU     0C0h    ; RTS Control Mode [7:6]
DCB_F2_RTS_DIS  EQU     000h    ;   00 = disable RTS changes
DCB_F2_RTS_ENA  EQU     040h    ;   01 = enable RTS changes at open/close
DCB_F2_RTS_HS   EQU     080h    ;   10 = RTS input handshake

;; DCB Flags3 bit definitions
DCB_F3_WTOINF   EQU     01h     ; Bit 0: infinite write timeout
DCB_F3_RDTO_MASK EQU    06h     ; Read timeout mode [2:1]
DCB_F3_RDTO_NRM EQU     02h     ;   01 = normal read timeout
DCB_F3_RDTO_WAIT EQU    04h     ;   10 = wait for something
DCB_F3_RDTO_NOW EQU     06h     ;   11 = no wait
DCB_F3_FIFO_MASK EQU    18h     ; FIFO control [4:3]
DCB_F3_FIFO_NC  EQU     00h     ;   00 = no change
DCB_F3_FIFO_DIS EQU     08h     ;   01 = disable FIFO
DCB_F3_FIFO_ENA EQU     10h     ;   10 = enable FIFO  (SIO forces this)
DCB_F3_FIFO_DYN EQU     18h     ;   11 = dynamic
DCB_F3_TRIG_MASK EQU    60h     ; Trigger level [6:5]
DCB_F3_TRIG_1   EQU     00h     ;   00 = 1 char
DCB_F3_TRIG_4   EQU     20h     ;   01 = 4 chars
DCB_F3_TRIG_8   EQU     40h     ;   10 = 8 chars  (SIO forces this)
DCB_F3_TRIG_14  EQU     60h     ;   11 = 14 chars
DCB_F3_TXLOAD   EQU     80h     ; Bit 7: TX load 16 chars (SIO forces on)

;; ---------- Ring Buffer ----------

RINGBUF STRUC
rbHead          DW      ?       ; Read position (consumer)
rbTail          DW      ?       ; Write position (producer)
rbCount         DW      ?       ; Current byte count
rbSize          DW      ?       ; Buffer capacity
rbBase          DD      ?       ; Far pointer to buffer memory
RINGBUF ENDS

;; ---------- Per-Port Data Structure ----------

PORTDATA STRUC
;; -- Hardware --
pdIOBase        DW      ?       ; UART base I/O port address
pdIOBaseDOS     DW      ?       ; Virtual I/O port for DOS sessions
pdIRQ           DB      ?       ; Hardware IRQ number (0FFh = NONE)
pdIRQDOS        DB      ?       ; Virtual IRQ for DOS sessions
pdUARTType      DB      ?       ; UART chip type (UART_xxx)
pdFIFOSize      DB      ?       ; FIFO depth (0 if no FIFO)

;; -- State --
pdFlags         DW      ?       ; Port state flags (see PF_xxx below)
pdOpenCount     DW      ?       ; Number of active opens
pdBaudRate      DD      ?       ; Current baud rate
pdBaudDiv       DW      ?       ; Current divisor value
pdLineCtrl      DB      ?       ; Current LCR value (data/parity/stop)
pdMCR           DB      ?       ; Current MCR shadow
pdMSR           DB      ?       ; Last MSR reading
pdLSR           DB      ?       ; Accumulated LSR errors

;; -- DCB --
pdDCB           DB      11 DUP (?)              ; SIODCB (11 bytes)

;; -- Error/Event tracking --
pdErrorWord     DW      ?       ; SIO Error Word (IOCtl 6Dh)
pdEventWord     DW      ?       ; SIO Event Word (IOCtl 72h)

;; -- Ring Buffers --
pdRxBuf         DB      12 DUP (?)              ; RINGBUF (12 bytes)
pdTxBuf         DB      12 DUP (?)              ; RINGBUF (12 bytes)

;; -- Flow Control State --
pdTxHold        DB      ?       ; Transmitter held (IOCtl 47h)
pdXoffRecvd     DB      ?       ; Remote sent XOFF
pdXoffSent      DB      ?       ; We sent XOFF
pdTxImm         DB      ?       ; Transmit immediate byte pending
pdTxImmChar     DB      ?       ; The immediate byte to send

;; -- Synchronization --
pdReadWait      DD      ?       ; Block ID for read waiters
pdWriteWait     DD      ?       ; Block ID for write waiters

;; -- Internet port (VMODEM) --
pdIsInternet    DB      ?       ; Non-zero if this is a VMODEM port

;; -- Protection --
pdProtMode      DB      ?       ; 0=normal, '-'=no prot, '+'=inhibit DOS DTR off

;; -- Locked baud --
pdBaudLocked    DB      ?       ; Non-zero if baud rate is locked
pdComNum        DB      ?       ; 1-based COMn number parsed from CONFIG.SYS
                                ; (was "pdPad" alignment filler — unused
                                ;  until now; ParseOnePort parsed this
                                ;  value via ParseDecimal but never
                                ;  stored it anywhere, so PrintPortDetect
                                ;  and the port-overlap check had no way
                                ;  to name a port. See BUGFIXES.md.)

PORTDATA ENDS

;; Port state flags (pdFlags)
PF_EXISTS       EQU     0001h   ; Port hardware detected
PF_OPEN         EQU     0002h   ; Port is open (at least one opener)
PF_IRQCLAIMED   EQU     0004h   ; IRQ is claimed from OS
PF_FIFOENABLED  EQU     0008h   ; FIFO is enabled
PF_READING      EQU     0010h   ; Read operation in progress
PF_WRITING      EQU     0020h   ; Write operation in progress
PF_BREAKON      EQU     0040h   ; Break is being transmitted
PF_INTERNET     EQU     0080h   ; This is a VMODEM port
PF_ESP          EQU     0100h   ; Hayes ESP card
PF_TPORT        EQU     0200h   ; Telcor Tport card
PF_HIGHBAUD     EQU     0400h   ; 16550A supports >115K baud (green)

;; ---------- Global Driver Data ----------

DRIVERDATA STRUC
ddNumPorts      DW      ?       ; Number of configured ports
ddDevHlp        DD      ?       ; DevHlp entry point (from OS/2 kernel)
ddPorts         DB      16384 DUP (?)           ; MAX_PORTS * PORTDATA
DRIVERDATA ENDS
