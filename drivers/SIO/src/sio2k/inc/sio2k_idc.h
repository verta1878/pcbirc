/* ====================================================================
 * sio2k_idc.h — Inter-Device Communication interface
 * ====================================================================
 * Defines the contract between SIO2K.SYS (logical layer) and physical
 * layer drivers (UART.SYS, ESP.SYS, VMODEM.SYS).
 *
 * Physical drivers register with SIO2K via IDC name (e.g., "UART$").
 * SIO2K calls physical drivers for hardware operations.
 * Physical drivers call SIO2K to deliver received data and events.
 * ====================================================================
 */

#ifndef SIO2K_IDC_H
#define SIO2K_IDC_H

/* -------------------------------------------------------------------- */
/* IDC Device Names                                                     */
/* -------------------------------------------------------------------- */

#define IDC_NAME_UART       "UART$"
#define IDC_NAME_ESP        "ESP$"
#define IDC_NAME_VMODEM     "VMOD$"

/* -------------------------------------------------------------------- */
/* SIO2K → Physical Driver Commands                                     */
/* (SIO2K calls these on the physical driver)                           */
/* -------------------------------------------------------------------- */

#define PHYS_CMD_INIT       0x01    /* Initialize hardware              */
#define PHYS_CMD_OPEN       0x02    /* Open port — probe & enable       */
#define PHYS_CMD_CLOSE      0x03    /* Close port — disable hardware    */
#define PHYS_CMD_SETBAUD    0x04    /* Set baud rate (divisor)          */
#define PHYS_CMD_SETLINE    0x05    /* Set line control (LCR)           */
#define PHYS_CMD_SETMCR     0x06    /* Set modem control (DTR/RTS)      */
#define PHYS_CMD_GETMSR     0x07    /* Get modem status                 */
#define PHYS_CMD_GETLSR     0x08    /* Get line status                  */
#define PHYS_CMD_TXSTART    0x09    /* Start transmitter                */
#define PHYS_CMD_TXSTOP     0x0A    /* Stop transmitter (hold)          */
#define PHYS_CMD_TXFLUSH    0x0B    /* Flush TX buffer                  */
#define PHYS_CMD_RXFLUSH    0x0C    /* Flush RX buffer                  */
#define PHYS_CMD_BREAKON    0x0D    /* Start break signal               */
#define PHYS_CMD_BREAKOFF   0x0E    /* Stop break signal                */
#define PHYS_CMD_SETFIFO    0x0F    /* Set FIFO control                 */
#define PHYS_CMD_GETINFO    0x10    /* Get port info (UART type, etc.)  */
#define PHYS_CMD_TXBYTE     0x11    /* Transmit immediate byte          */
#define PHYS_CMD_TXBLOCK    0x12    /* Transmit block of bytes          */
#define PHYS_CMD_RXBLOCK    0x13    /* Read block of bytes from RX buf  */
#define PHYS_CMD_RXCOUNT    0x14    /* Query RX buffer byte count       */
#define PHYS_CMD_TXCOUNT    0x15    /* Query TX buffer byte count       */
#define PHYS_CMD_SETFLOW    0x16    /* Set flow control mode            */
#define PHYS_CMD_AUTOIRQ    0x17    /* Detect IRQ (V2 auto-detect)      */
#define PHYS_CMD_AUTOFIFO   0x18    /* Probe FIFO size                  */
#define PHYS_CMD_AUTOCRYSTAL 0x19   /* Detect crystal frequency         */

/* -------------------------------------------------------------------- */
/* Physical Driver → SIO2K Callbacks                                    */
/* (Physical driver calls these to deliver data/events to SIO2K)        */
/* -------------------------------------------------------------------- */

#define SIO2K_CB_RXDATA     0x01    /* Received data available          */
#define SIO2K_CB_TXEMPTY    0x02    /* TX buffer space available         */
#define SIO2K_CB_MSRCHANGE  0x03    /* Modem status changed             */
#define SIO2K_CB_LSREVENT   0x04    /* Line status event (error/break)  */
#define SIO2K_CB_BREAK      0x05    /* Break detected                   */

/* -------------------------------------------------------------------- */
/* Port Info Structure (returned by PHYS_CMD_GETINFO)                   */
/* -------------------------------------------------------------------- */

typedef struct _PHYS_PORT_INFO {
    unsigned short  ioBase;         /* I/O port base address            */
    unsigned char   irq;            /* IRQ number (auto-detected)       */
    unsigned char   uartType;       /* UART chip type                   */
    unsigned short  fifoSize;       /* Actual FIFO depth (probed)       */
    unsigned long   crystalFreq;    /* Crystal frequency in Hz          */
    unsigned long   maxBaud;        /* Maximum baud rate                */
    unsigned char   flags;          /* Port flags                       */
    unsigned char   reserved;
} PHYS_PORT_INFO;

/* uartType values */
#define PHYS_UART_NONE      0
#define PHYS_UART_8250      1
#define PHYS_UART_16450     2
#define PHYS_UART_16550     3       /* Broken FIFO — treated as 16450   */
#define PHYS_UART_16550A    4
#define PHYS_UART_16650     5
#define PHYS_UART_16750     6
#define PHYS_UART_16850     7
#define PHYS_UART_16950     8
#define PHYS_UART_ESP       20      /* Hayes ESP card                   */
#define PHYS_UART_VMODEM    30      /* Virtual modem (VMODEM.SYS)       */

/* flags */
#define PHYS_FLAG_EXISTS    0x01
#define PHYS_FLAG_FIFO      0x02
#define PHYS_FLAG_PCI       0x04    /* PCI-based UART                   */
#define PHYS_FLAG_SUPERIO   0x08    /* SuperIO enhanced UART            */
#define PHYS_FLAG_NONSTANDARD_CRYSTAL 0x10

/* -------------------------------------------------------------------- */
/* Flow Control Modes (for PHYS_CMD_SETFLOW)                            */
/* -------------------------------------------------------------------- */

#define FLOW_NONE           0x00
#define FLOW_XONXOFF_TX     0x01    /* Honor received XON/XOFF          */
#define FLOW_XONXOFF_RX     0x02    /* Send XON/XOFF to remote          */
#define FLOW_CTS            0x04    /* CTS required to transmit         */
#define FLOW_RTS            0x08    /* RTS input handshake              */
#define FLOW_DSR            0x10    /* DSR required to receive          */

/* -------------------------------------------------------------------- */
/* IDC Entry Point Prototype                                            */
/* -------------------------------------------------------------------- */

/* Physical driver entry: cmd, port_index, data_ptr, data_len */
typedef unsigned short (*PFN_PHYS_IDC)(
    unsigned short  cmd,
    unsigned short  portIndex,      /* 1-based port within this driver  */
    void           *pData,
    unsigned short  cbData
);

/* SIO2K callback entry: event, port_handle, data_ptr, data_len */
typedef unsigned short (*PFN_SIO2K_CB)(
    unsigned short  event,
    unsigned short  portHandle,     /* SIO2K's internal port handle     */
    void           *pData,
    unsigned short  cbData
);

/* -------------------------------------------------------------------- */
/* IDC Registration Structure                                           */
/* -------------------------------------------------------------------- */

typedef struct _IDC_REGISTER {
    char            idcName[8];     /* Driver IDC name (e.g., "UART$")  */
    unsigned short  numPorts;       /* Number of ports this driver has  */
    PFN_PHYS_IDC    pfnEntry;      /* Physical driver entry point      */
    PFN_SIO2K_CB    pfnCallback;   /* SIO2K callback (filled by SIO2K) */
} IDC_REGISTER;

#endif /* SIO2K_IDC_H */
