#ifndef _VSIO_H_
#define _VSIO_H_

/* ====================================================================
 * vsio.h — Shared definitions between SIO.SYS PDD and VSIO.SYS VDD
 * ====================================================================
 */

#ifndef VSIO_H
#define VSIO_H

/* -------------------------------------------------------------------- */
/* PDD-VDD Communication Command Codes                                  */
/* -------------------------------------------------------------------- */

/* VDD → PDD commands (VSIO calling SIO) */
#define SIOCMD_GETPORTINFO  0x10    /* Get port configuration           */
#define SIOCMD_OPENPORT     0x11    /* Open a port for VDM use          */
#define SIOCMD_CLOSEPORT    0x12    /* Close a port                     */
#define SIOCMD_READBYTE     0x13    /* Read byte from RX buffer         */
#define SIOCMD_WRITEBYTE    0x14    /* Write byte to TX buffer          */
#define SIOCMD_GETMSR       0x15    /* Get modem status register        */
#define SIOCMD_GETLSR       0x16    /* Get line status register         */
#define SIOCMD_SETMCR       0x17    /* Set modem control register       */
#define SIOCMD_SETBAUD      0x18    /* Set baud rate (divisor)          */
#define SIOCMD_SETLCR       0x19    /* Set line control register        */
#define SIOCMD_SETFCR       0x1A    /* Set FIFO control register        */
#define SIOCMD_SETIER       0x1B    /* Set interrupt enable register    */
#define SIOCMD_TXREADY      0x1C    /* Query TX ready state             */
#define SIOCMD_RXREADY      0x1D    /* Query RX data available          */

/* PDD → VDD commands (SIO notifying VSIO) */
#define SIOCMD_NOTIFY       0x1E    /* Async notification               */

/* Notification event flags (ul2 for SIOCMD_NOTIFY) */
#define SIOEVT_RXDATA       0x0001  /* Data available in RX buffer      */
#define SIOEVT_TXEMPTY      0x0002  /* TX buffer has space              */
#define SIOEVT_MSR          0x0004  /* Modem status changed             */
#define SIOEVT_LSR          0x0008  /* Line status (error/break)        */
#define SIOEVT_BREAK        0x0010  /* Break detected                   */

/* -------------------------------------------------------------------- */
/* Port Info Structure (returned by SIOCMD_GETPORTINFO)                 */
/* -------------------------------------------------------------------- */

typedef struct _SIOPORT_INFO {
    USHORT  ioBase;         /* Real I/O base address                    */
    USHORT  ioBaseDOS;      /* Virtual I/O port for DOS sessions        */
    BYTE    irq;            /* Real IRQ                                 */
    BYTE    irqDOS;         /* Virtual IRQ for DOS sessions             */
    BYTE    uartType;       /* UART chip type                           */
    BYTE    flags;          /* Port flags                               */
    BYTE    isInternet;     /* Non-zero = VMODEM port                   */
    BYTE    reserved;
} SIOPORT_INFO, *PSIOPORT_INFO;

/* Flags for SIOPORT_INFO.flags */
#define SPIF_EXISTS     0x01
#define SPIF_OPEN       0x02
#define SPIF_FIFO       0x04

#endif /* VSIO_H */


#endif /* _VSIO_H_ */
