;; ====================================================================
;; uart.inc — 8250/16450/16550A UART register and constant definitions
;; ====================================================================
;; Clean-room implementation from published UART datasheets.
;; National Semiconductor PC16550D, Intel 8250A/16450 documentation.
;; ====================================================================

;; ---------- UART Register Offsets (from base I/O port) ----------

UART_RBR        EQU     0       ; Receive Buffer Register (read, DLAB=0)
UART_THR        EQU     0       ; Transmit Holding Register (write, DLAB=0)
UART_DLL        EQU     0       ; Divisor Latch Low (DLAB=1)
UART_DLH        EQU     1       ; Divisor Latch High (DLAB=1)
UART_IER        EQU     1       ; Interrupt Enable Register (DLAB=0)
UART_IIR        EQU     2       ; Interrupt Identification Reg (read)
UART_FCR        EQU     2       ; FIFO Control Register (write, 16550+)
UART_LCR        EQU     3       ; Line Control Register
UART_MCR        EQU     4       ; Modem Control Register
UART_LSR        EQU     5       ; Line Status Register
UART_MSR        EQU     6       ; Modem Status Register
UART_SCR        EQU     7       ; Scratch Register (16450+)

;; ---------- IER bits ----------

IER_RDA         EQU     01h     ; Received Data Available
IER_THRE        EQU     02h     ; Transmitter Holding Register Empty
IER_RLS         EQU     04h     ; Receiver Line Status
IER_MS          EQU     08h     ; Modem Status

;; ---------- IIR bits ----------

IIR_PENDING     EQU     01h     ; 0 = interrupt pending
IIR_ID_MASK     EQU     0Eh     ; Interrupt ID bits [3:1]
IIR_MS          EQU     00h     ; Modem Status change
IIR_THRE        EQU     02h     ; THR Empty
IIR_RDA         EQU     04h     ; Received Data Available
IIR_RLS         EQU     06h     ; Receiver Line Status
IIR_TIMEOUT     EQU     0Ch     ; Character Timeout (16550+)
IIR_FIFO_MASK   EQU     0C0h    ; FIFO enabled bits
IIR_FIFO_NONE   EQU     000h    ; No FIFO (8250/16450)
IIR_FIFO_BAD    EQU     080h    ; FIFO enabled but unusable (16550 non-A)
IIR_FIFO_OK     EQU     0C0h    ; FIFO enabled and usable (16550A+)

;; ---------- FCR bits (write-only on 16550+) ----------

FCR_ENABLE      EQU     01h     ; Enable FIFOs
FCR_RXRESET     EQU     02h     ; Reset receive FIFO
FCR_TXRESET     EQU     04h     ; Reset transmit FIFO
FCR_DMA         EQU     08h     ; DMA mode select
FCR_TRIG_1      EQU     00h     ; RX trigger level: 1 byte
FCR_TRIG_4      EQU     40h     ; RX trigger level: 4 bytes
FCR_TRIG_8      EQU     80h     ; RX trigger level: 8 bytes
FCR_TRIG_14     EQU     0C0h    ; RX trigger level: 14 bytes

;; ---------- LCR bits ----------

LCR_WLS_MASK    EQU     03h     ; Word Length Select bits [1:0]
LCR_WLS_5       EQU     00h     ; 5 data bits
LCR_WLS_6       EQU     01h     ; 6 data bits
LCR_WLS_7       EQU     02h     ; 7 data bits
LCR_WLS_8       EQU     03h     ; 8 data bits
LCR_STB         EQU     04h     ; Stop Bits (0=1, 1=1.5/2)
LCR_PEN         EQU     08h     ; Parity Enable
LCR_EPS         EQU     10h     ; Even Parity Select
LCR_STICK       EQU     20h     ; Stick Parity
LCR_BREAK       EQU     40h     ; Set Break
LCR_DLAB        EQU     80h     ; Divisor Latch Access Bit

;; ---------- MCR bits ----------

MCR_DTR         EQU     01h     ; Data Terminal Ready
MCR_RTS         EQU     02h     ; Request To Send
MCR_OUT1        EQU     04h     ; Output 1 (general purpose)
MCR_OUT2        EQU     08h     ; Output 2 (enables IRQ on PC)
MCR_LOOP        EQU     10h     ; Loopback mode

;; ---------- LSR bits ----------

LSR_DR          EQU     01h     ; Data Ready
LSR_OE          EQU     02h     ; Overrun Error
LSR_PE          EQU     04h     ; Parity Error
LSR_FE          EQU     08h     ; Framing Error
LSR_BI          EQU     10h     ; Break Interrupt
LSR_THRE        EQU     20h     ; Transmitter Holding Reg Empty
LSR_TEMT        EQU     40h     ; Transmitter Empty
LSR_FIFOERR     EQU     80h     ; Error in FIFO (16550+)

;; ---------- MSR bits ----------

MSR_DCTS        EQU     01h     ; Delta CTS
MSR_DDSR        EQU     02h     ; Delta DSR
MSR_TERI        EQU     04h     ; Trailing Edge Ring Indicator
MSR_DDCD        EQU     08h     ; Delta DCD
MSR_CTS         EQU     10h     ; Clear To Send
MSR_DSR         EQU     20h     ; Data Set Ready
MSR_RI          EQU     40h     ; Ring Indicator
MSR_DCD         EQU     80h     ; Data Carrier Detect

;; ---------- UART Chip Type IDs ----------

UART_NONE       EQU     0       ; No UART detected
UART_8250       EQU     1       ; 8250 (original, slow)
UART_8250A      EQU     2       ; 8250A (scratch register)
UART_16450      EQU     3       ; 16450 (fast 8250A)
UART_16550      EQU     4       ; 16550 (broken FIFO — treat as 16450)
UART_16550A     EQU     5       ; 16550A (working 16-byte FIFO)
UART_16550A_HI  EQU     6       ; 16550A with high-speed baud (460K)
UART_82510      EQU     7       ; Intel 82510 (4-byte FIFO)
UART_16650      EQU     8       ; Startech 16650 (32-byte FIFO)
UART_16750      EQU     9       ; TI 16750 (64-byte FIFO)
UART_COMBIC     EQU     10      ; Hayes ComBic (1K FIFO)

;; ---------- Standard COM Port Addresses ----------

COM1_BASE       EQU     03F8h
COM2_BASE       EQU     02F8h
COM3_BASE       EQU     03E8h
COM4_BASE       EQU     02E8h

;; PS/2 COM port addresses
COM3_PS2        EQU     3220h
COM4_PS2        EQU     3228h
COM5_PS2        EQU     4220h
COM6_PS2        EQU     4228h
COM7_PS2        EQU     5220h
COM8_PS2        EQU     5228h

;; ---------- Standard COM Port IRQs ----------

COM1_IRQ        EQU     4
COM2_IRQ        EQU     3
COM3_IRQ        EQU     4       ; Defacto standard (shared with COM1)
COM4_IRQ        EQU     3       ; Defacto standard (shared with COM2)

;; ---------- Baud Rate Divisors (1.8432 MHz crystal) ----------
;; Divisor = 115200 / desired_baud

BAUD_CLOCK      EQU     115200  ; Standard oscillator / 16

DIV_300         EQU     384
DIV_1200        EQU     96
DIV_2400        EQU     48
DIV_4800        EQU     24
DIV_9600        EQU     12
DIV_19200       EQU     6
DIV_38400       EQU     3
DIV_57600       EQU     2
DIV_115200      EQU     1

;; ---------- Buffer Sizes ----------

RX_BUF_SIZE     EQU     4096    ; Receive ring buffer
TX_BUF_SIZE     EQU     4096    ; Transmit ring buffer
MAX_PORTS       EQU     16      ; Maximum supported COM ports
