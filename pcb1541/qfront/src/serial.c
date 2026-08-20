/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* serial.c -- Serial Port Abstraction Layer                                */
/*                                                                           */
/* Portable serial I/O across:                                               */
/*   - Win32 (CreateFile COM port)                                          */
/*   - Linux/BSD (termios /dev/ttyS*)                                       */
/*   - DOS FOSSIL (INT 14h via CYFOSSIL)                                    */
/*   - DOS UART (direct 16550 register access)                              */
/*                                                                           */
/* From binary: "Using UART for communications"                              */
/*              "Using fossil for communications"                            */
/*              "Using DigiBoard for communications"                         */
/*              "Enabling 16550 UART buffer"                                 */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"
#include <string.h>

#ifdef _WIN32
/* Win32 serial via CreateFile */
#elif defined(__MSDOS__) || defined(__DOS__)
#include <dos.h>
#include <conio.h>
#define QF_DOS 1
#else
/* POSIX serial via termios */
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#endif


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Serial Port Handle                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef enum {
    SER_NONE = 0,                       /* not initialized               */
    SER_UART,                           /* direct 16550 UART (DOS)       */
    SER_FOSSIL,                         /* FOSSIL driver INT 14h (DOS)   */
    SER_WIN32,                          /* Win32 COM port                */
    SER_POSIX                           /* Linux/BSD /dev/ttyS*          */
} SerType;

typedef struct {
    SerType  Type;                      /* port type                     */
    int      PortNum;                   /* COM port number (1-based)     */
    uint32_t Baud;                      /* current baud rate             */
    int      FlowRtsCts;                /* RTS/CTS hardware flow         */
    int      FlowXonXoff;               /* XON/XOFF software flow        */

#ifdef _WIN32
    HANDLE   hCom;                      /* Win32 file handle             */
#elif defined(QF_DOS)
    uint16_t BaseAddr;                  /* UART base I/O address         */
    int      HasFifo;                   /* 16550 FIFO available          */
    int      FossilActive;              /* FOSSIL driver present         */
#else
    int      Fd;                        /* POSIX file descriptor         */
    struct termios OrigTio;             /* original terminal settings    */
#endif
} SerPort;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    DOS UART Register Definitions                          */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifdef QF_DOS
static const uint16_t UartBases[] = {
    0x3F8,                              /* COM1                          */
    0x2F8,                              /* COM2                          */
    0x3E8,                              /* COM3                          */
    0x2E8                               /* COM4                          */
};

/* 16550 UART registers (offsets from base address) */
#define UART_RBR    0                   /* receive buffer (read)         */
#define UART_THR    0                   /* transmit holding (write)      */
#define UART_IER    1                   /* interrupt enable               */
#define UART_IIR    2                   /* interrupt identification (rd)  */
#define UART_FCR    2                   /* FIFO control (write)          */
#define UART_LCR    3                   /* line control                  */
#define UART_MCR    4                   /* modem control                 */
#define UART_LSR    5                   /* line status                   */
#define UART_MSR    6                   /* modem status                  */
#define UART_DLL    0                   /* divisor latch low (DLAB=1)    */
#define UART_DLM    1                   /* divisor latch high (DLAB=1)   */

/* Line Status Register bits */
#define LSR_DR      0x01                /* data ready                    */
#define LSR_THRE    0x20                /* TX holding register empty     */

/* Modem Status Register bits */
#define MSR_CTS     0x10                /* clear to send                 */
#define MSR_DSR     0x20                /* data set ready                */
#define MSR_DCD     0x80                /* data carrier detect           */

/* Modem Control Register bits */
#define MCR_DTR     0x01                /* data terminal ready           */
#define MCR_RTS     0x02                /* request to send               */
#endif /* QF_DOS */


/*-----------------------------------------------------------------------*/
/* ser_open() -- Open a serial port                                     */
/*                                                                       */
/* Initializes the serial port for the specified platform:               */
/*   Win32:  CreateFile on \\.\COMn, 9600 8N1, DTR+RTS raised           */
/*   FOSSIL: INT 14h AH=04 init, check for 0x1954 signature             */
/*   UART:   Direct register access, detect 16550 FIFO                  */
/*   POSIX:  open /dev/ttyS(n-1) or /dev/ttyUSB(n-1), raw mode          */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int ser_open(SerPort *Sp, int PortNum, int UseFossil)
{
    memset(Sp, 0, sizeof(*Sp));
    Sp->PortNum = PortNum;
    Sp->Baud    = 9600;

#ifdef _WIN32
    {
        char         Name[16];          /* COM port device name          */
        DCB          Dcb;               /* device control block          */
        COMMTIMEOUTS Timeouts;          /* read/write timeouts           */

        snprintf(Name, sizeof(Name), "\\\\.\\COM%d", PortNum);
        Sp->hCom = CreateFileA(Name, GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_EXISTING, 0, NULL);
        if (Sp->hCom == INVALID_HANDLE_VALUE) {
            qf_log(LOG_ERROR, "Cannot open COM%d", PortNum);
            return -1;
        }

        /* Configure COM port -- match original QFront behavior.
         * Raises DTR+RTS on open, disables flow control so writes
         * don't block waiting for CTS from modem. */
        memset(&Dcb, 0, sizeof(Dcb));
        Dcb.DCBlength = sizeof(Dcb);
        GetCommState(Sp->hCom, &Dcb);
        Dcb.BaudRate     = 9600;
        Dcb.ByteSize     = 8;
        Dcb.Parity       = NOPARITY;
        Dcb.StopBits     = ONESTOPBIT;
        Dcb.fBinary      = TRUE;

        /* Flow control -- disabled by default */
        Dcb.fOutxCtsFlow = FALSE;
        Dcb.fOutxDsrFlow = FALSE;
        Dcb.fOutX        = FALSE;
        Dcb.fInX         = FALSE;

        /* DTR+RTS -- raise both on open */
        Dcb.fDtrControl  = DTR_CONTROL_ENABLE;
        Dcb.fRtsControl  = RTS_CONTROL_ENABLE;

        /* Modem signal sensitivity */
        Dcb.fDsrSensitivity = FALSE;
        Dcb.fAbortOnError   = FALSE;
        Dcb.fNull           = FALSE;

        SetCommState(Sp->hCom, &Dcb);
        SetupComm(Sp->hCom, 4096, 4096);

        /* Set timeouts: 100ms read timeout */
        Timeouts.ReadIntervalTimeout        = 100;
        Timeouts.ReadTotalTimeoutMultiplier  = 0;
        Timeouts.ReadTotalTimeoutConstant    = 100;
        Timeouts.WriteTotalTimeoutMultiplier = 0;
        Timeouts.WriteTotalTimeoutConstant   = 1000;
        SetCommTimeouts(Sp->hCom, &Timeouts);

        Sp->Type = SER_WIN32;
        qf_log(LOG_INFO, "Opening port COM%d (Win32)", PortNum);
    }

#elif defined(QF_DOS)
    if (UseFossil) {
        /* Check for FOSSIL driver via INT 14h/AH=04 (init) */
        union REGS Regs;                /* DOS interrupt registers       */

        Regs.h.ah = 0x04;              /* FOSSIL init                   */
        Regs.w.dx = PortNum - 1;
        int386(0x14, &Regs, &Regs);

        if (Regs.w.ax == 0x1954) {
            Sp->Type = SER_FOSSIL;
            Sp->FossilActive = 1;
            qf_log(LOG_INFO, "Using fossil for communications (COM%d)",
                   PortNum);
            return 0;
        }
        qf_log(LOG_WARN, "FOSSIL not found, falling back to UART");
    }

    /* Direct UART access */
    if (PortNum < 1 || PortNum > 4) {
        qf_log(LOG_ERROR, "Invalid port number: %d", PortNum);
        return -1;
    }

    Sp->BaseAddr = UartBases[PortNum - 1];

    /* Detect 16550 FIFO */
    outp(Sp->BaseAddr + UART_FCR, 0xC1);
    if (inp(Sp->BaseAddr + UART_IIR) & 0xC0) {
        Sp->HasFifo = 1;
        qf_log(LOG_INFO, "Enabling 16550 UART buffer");
        outp(Sp->BaseAddr + UART_FCR, 0xC7);
    } else {
        Sp->HasFifo = 0;
        outp(Sp->BaseAddr + UART_FCR, 0x00);
        qf_log(LOG_INFO, "16550 UART not found");
        qf_log(LOG_INFO, "Using UART for communications");
    }

    Sp->Type = SER_UART;
    qf_log(LOG_INFO, "Base address = %04Xh", Sp->BaseAddr);

#else
    /* POSIX */
    {
        char        DevName[32];        /* device path                   */
        struct termios Tio;             /* terminal settings             */

        (void)UseFossil;                /* not used on POSIX             */

        snprintf(DevName, sizeof(DevName), "/dev/ttyS%d", PortNum - 1);
        Sp->Fd = open(DevName, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (Sp->Fd < 0) {
            /* Try USB serial */
            snprintf(DevName, sizeof(DevName), "/dev/ttyUSB%d", PortNum - 1);
            Sp->Fd = open(DevName, O_RDWR | O_NOCTTY | O_NONBLOCK);
        }
        if (Sp->Fd < 0) {
            qf_log(LOG_ERROR, "Cannot open %s: %s",
                   DevName, strerror(errno));
            return -1;
        }

        /* Save original settings */
        tcgetattr(Sp->Fd, &Sp->OrigTio);

        /* Raw mode: 9600 8N1 */
        memset(&Tio, 0, sizeof(Tio));
        Tio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        Tio.c_iflag = 0;
        Tio.c_oflag = 0;
        Tio.c_lflag = 0;
        Tio.c_cc[VMIN]  = 0;
        Tio.c_cc[VTIME] = 1;           /* 100ms timeout                 */
        tcsetattr(Sp->Fd, TCSANOW, &Tio);

        /* Clear O_NONBLOCK after setup */
        fcntl(Sp->Fd, F_SETFL, 0);

        Sp->Type = SER_POSIX;
        qf_log(LOG_INFO, "Opening port %s", DevName);
        qf_log(LOG_DEBUG, "  fd=%d, raw mode 9600 8N1, VMIN=0 VTIME=1",
               Sp->Fd);
    }
#endif

    qf_log(LOG_DEBUG, "ser_open: port_num=%d type=%d success",
           Sp->PortNum, (int)Sp->Type);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* ser_close() -- Close the serial port and restore original settings    */
/*                                                                       */
/* On POSIX: restores the original termios settings saved in ser_open()  */
/* so we don't leave the port in raw mode for other programs.            */
/*                                                                       */
/* On DOS FOSSIL: sends the FOSSIL deinit call (INT 14h AH=05). This    */
/* releases the FOSSIL driver's buffers and interrupt hooks. Failing to  */
/* call this leaks interrupt vectors and can crash the system.           */
/*                                                                       */
/* On DOS UART: disables the 16550 FIFO to leave the hardware clean.    */
/*                                                                       */
/* IMPORTANT: For inbound human callers, we close the port handle but    */
/* do NOT drop DTR -- the modem connection must stay up so PCBoard can   */
/* take over the caller. DTR drop is handled separately by mdm_hangup().*/
/*-----------------------------------------------------------------------*/

void ser_close(SerPort *Sp)
{
    if (!Sp) return;
    qf_log(LOG_DEBUG, "ser_close: closing COM%d (type=%d)",
           Sp->PortNum, (int)Sp->Type);

#ifdef _WIN32
    if (Sp->Type == SER_WIN32 && Sp->hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(Sp->hCom);
        Sp->hCom = INVALID_HANDLE_VALUE;
    }
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        union REGS Regs;                /* DOS interrupt registers       */
        Regs.h.ah = 0x05;              /* FOSSIL deinit                 */
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
    }
    if (Sp->HasFifo)
        outp(Sp->BaseAddr + UART_FCR, 0x00);
#else
    if (Sp->Type == SER_POSIX && Sp->Fd >= 0) {
        tcsetattr(Sp->Fd, TCSANOW, &Sp->OrigTio);
        close(Sp->Fd);
        Sp->Fd = -1;
    }
#endif

    qf_log(LOG_DEBUG, "Closing port COM%d", Sp->PortNum);
    Sp->Type = SER_NONE;
}


/*-----------------------------------------------------------------------*/
/* ser_read_byte() -- Read a single byte with timeout                   */
/*                                                                       */
/* This is the fundamental receive primitive. All protocol modules       */
/* (EMSI, YooHoo, Zmodem, Xmodem) call this to read from the port.     */
/*                                                                       */
/* Returns the byte value (0-255) on success, or -1 on timeout.         */
/*                                                                       */
/* Implementation per platform:                                          */
/*   Win32:  ReadFile with ReadTotalTimeoutConstant                      */
/*   FOSSIL: INT 14h AH=02 (read with wait)                             */
/*   UART:   Poll LSR Data Ready bit until timeout                      */
/*   POSIX:  select() with timeout, then read()                          */
/*-----------------------------------------------------------------------*/

int ser_read_byte(SerPort *Sp, int TimeoutMs)
{
#ifdef _WIN32
    {
        BYTE         b;                 /* received byte                 */
        DWORD        Got;               /* bytes actually read           */
        COMMTIMEOUTS Ct;                /* timeout settings              */

        GetCommTimeouts(Sp->hCom, &Ct);
        Ct.ReadTotalTimeoutConstant = TimeoutMs;
        SetCommTimeouts(Sp->hCom, &Ct);
        if (ReadFile(Sp->hCom, &b, 1, &Got, NULL) && Got == 1)
            return (int)b;
        return -1;
    }
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        union REGS Regs;                /* DOS interrupt registers       */
        Regs.h.ah = 0x02;              /* FOSSIL read with wait         */
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
        if (Regs.h.ah == 0)
            return (int)Regs.h.al;
        return -1;
    } else {
        /* Direct UART polling */
        long Deadline;                  /* timeout deadline              */
        Deadline = clock() + (long)TimeoutMs * (CLOCKS_PER_SEC / 1000);
        while (clock() < Deadline) {
            if (inp(Sp->BaseAddr + UART_LSR) & LSR_DR)
                return (int)inp(Sp->BaseAddr + UART_RBR);
        }
        return -1;
    }
#else
    {
        unsigned char  b;              /* received byte                 */
        fd_set         Fds;            /* select file descriptor set    */
        struct timeval Tv;             /* select timeout                */

        FD_ZERO(&Fds);
        FD_SET(Sp->Fd, &Fds);
        Tv.tv_sec  = TimeoutMs / 1000;
        Tv.tv_usec = (TimeoutMs % 1000) * 1000;
        if (select(Sp->Fd + 1, &Fds, NULL, NULL, &Tv) > 0) {
            if (read(Sp->Fd, &b, 1) == 1)
                return (int)b;
        }
        return -1;
    }
#endif
}


/*-----------------------------------------------------------------------*/
/* ser_write() -- Write bytes to serial port                            */
/*                                                                       */
/* Sends Len bytes from Buf. On FOSSIL and UART, sends one byte at a    */
/* time. On Win32 and POSIX, writes the full buffer in one call.        */
/*                                                                       */
/* Returns number of bytes written.                                      */
/*-----------------------------------------------------------------------*/

int ser_write(SerPort *Sp, const void *Buf, int Len)
{
#ifdef _WIN32
    {
        DWORD Written;                  /* bytes actually written        */
        WriteFile(Sp->hCom, Buf, Len, &Written, NULL);
        return (int)Written;
    }
#elif defined(QF_DOS)
    {
        const unsigned char *p = (const unsigned char *)Buf;
        int i;                          /* byte loop index               */

        if (Sp->Type == SER_FOSSIL) {
            for (i = 0; i < Len; i++) {
                union REGS Regs;        /* DOS interrupt registers       */
                Regs.h.ah = 0x01;       /* FOSSIL write                  */
                Regs.h.al = p[i];
                Regs.w.dx = Sp->PortNum - 1;
                int386(0x14, &Regs, &Regs);
            }
        } else {
            for (i = 0; i < Len; i++) {
                while (!(inp(Sp->BaseAddr + UART_LSR) & LSR_THRE))
                    ;                   /* wait for TX ready             */
                outp(Sp->BaseAddr + UART_THR, p[i]);
            }
        }
        return Len;
    }
#else
    return (int)write(Sp->Fd, Buf, Len);
#endif
}


/*-----------------------------------------------------------------------*/
/* ser_write_str() -- Write a null-terminated string to serial port     */
/*-----------------------------------------------------------------------*/

int ser_write_str(SerPort *Sp, const char *Str)
{
    return ser_write(Sp, Str, (int)strlen(Str));
}


/*-----------------------------------------------------------------------*/
/* ser_set_baud() -- Set the serial port baud rate                      */
/*                                                                       */
/* Updates the baud rate on all platforms. On DOS FOSSIL, maps to the    */
/* FOSSIL baud rate codes. On DOS UART, programs the divisor latch      */
/* directly (115200 / baud = divisor).                                   */
/*                                                                       */
/* Returns 0 on success.                                                 */
/*-----------------------------------------------------------------------*/

int ser_set_baud(SerPort *Sp, uint32_t BaudRate)
{
    Sp->Baud = BaudRate;

#ifdef _WIN32
    {
        DCB Dcb;                        /* device control block          */
        GetCommState(Sp->hCom, &Dcb);
        Dcb.BaudRate = BaudRate;
        SetCommState(Sp->hCom, &Dcb);
        qf_log(LOG_INFO, "Baud rate set to %lu", (unsigned long)BaudRate);
    }
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        /* FOSSIL Fn00: set baud via INT 14h */
        union REGS    Regs;             /* DOS interrupt registers       */
        unsigned char Code;             /* FOSSIL baud rate code         */

        if      (BaudRate <= 300)    Code = 0x43;
        else if (BaudRate <= 1200)   Code = 0x83;
        else if (BaudRate <= 2400)   Code = 0xA3;
        else if (BaudRate <= 9600)   Code = 0xE3;
        else if (BaudRate <= 19200)  Code = 0x03;
        else if (BaudRate <= 38400)  Code = 0x23;
        else                         Code = 0xE3; /* default 9600        */

        Regs.h.ah = 0x00;
        Regs.h.al = Code;
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
    } else {
        /* Direct UART: set divisor latch */
        uint16_t      Divisor;          /* baud rate divisor             */
        unsigned char Lcr;              /* saved line control register   */

        Divisor = (uint16_t)(115200UL / BaudRate);
        Lcr = inp(Sp->BaseAddr + UART_LCR);
        outp(Sp->BaseAddr + UART_LCR, Lcr | 0x80);  /* DLAB=1           */
        outp(Sp->BaseAddr + UART_DLL, Divisor & 0xFF);
        outp(Sp->BaseAddr + UART_DLM, Divisor >> 8);
        outp(Sp->BaseAddr + UART_LCR, Lcr);          /* DLAB=0           */
    }
#else
    {
        struct termios Tio;             /* terminal settings             */
        speed_t        Spd;             /* POSIX speed constant          */

        switch (BaudRate) {
        case 300:    Spd = B300;    break;
        case 1200:   Spd = B1200;   break;
        case 2400:   Spd = B2400;   break;
        case 4800:   Spd = B4800;   break;
        case 9600:   Spd = B9600;   break;
        case 19200:  Spd = B19200;  break;
        case 38400:  Spd = B38400;  break;
        case 57600:  Spd = B57600;  break;
        case 115200: Spd = B115200; break;
        case 230400: Spd = B230400; break;
        default:     Spd = B9600;   break;
        }

        tcgetattr(Sp->Fd, &Tio);
        cfsetispeed(&Tio, Spd);
        cfsetospeed(&Tio, Spd);
        tcsetattr(Sp->Fd, TCSANOW, &Tio);
    }
#endif

    qf_log(LOG_DEBUG, "Baud rate set to %lu", (unsigned long)BaudRate);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* ser_get_dcd() -- Read Data Carrier Detect signal from modem          */
/*                                                                       */
/* DCD (pin 8 on DB-25, pin 1 on DB-9) tells us whether the modem       */
/* has an active carrier -- i.e., whether someone is on the line.        */
/*                                                                       */
/* Checked frequently during file transfers (Zmodem, etc.) to detect    */
/* if the caller hung up.                                                */
/*                                                                       */
/* Returns: 1 = carrier present (connected), 0 = no carrier.            */
/*-----------------------------------------------------------------------*/

int ser_get_dcd(SerPort *Sp)
{
#ifdef _WIN32
    {
        DWORD Status;                   /* modem status flags            */
        GetCommModemStatus(Sp->hCom, &Status);
        return (Status & MS_RLSD_ON) ? 1 : 0;
    }
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        union REGS Regs;                /* DOS interrupt registers       */
        Regs.h.ah = 0x03;              /* FOSSIL status                 */
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
        return (Regs.h.al & 0x80) ? 1 : 0;
    }
    return (inp(Sp->BaseAddr + UART_MSR) & MSR_DCD) ? 1 : 0;
#else
    {
        int Status;                     /* modem signal bits             */
        ioctl(Sp->Fd, TIOCMGET, &Status);
        return (Status & TIOCM_CD) ? 1 : 0;
    }
#endif
}


/*-----------------------------------------------------------------------*/
/* ser_set_dtr() -- Control the Data Terminal Ready signal               */
/*                                                                       */
/* DTR (pin 20 on DB-25, pin 4 on DB-9) tells the modem that the DTE    */
/* (our computer) is ready. Most modems are configured (AT&D2) to:       */
/*   - Hang up the call when DTR drops                                   */
/*   - Ignore AT commands when DTR is low                                */
/*   - Start auto-answer when DTR is raised (if S0>0)                    */
/*                                                                       */
/* Usage in QFront:                                                      */
/*   ser_set_dtr(Sp, 1)  -- raise DTR after opening port (ready)        */
/*   ser_set_dtr(Sp, 0)  -- drop DTR to hang up the call                */
/*                                                                       */
/* CRITICAL: For human callers (errorlevel 1 path), we must NOT drop    */
/* DTR before closing the port, or the caller gets disconnected before   */
/* PCBoard can take over.                                                */
/*-----------------------------------------------------------------------*/

void ser_set_dtr(SerPort *Sp, int On)
{
    qf_log(LOG_DEBUG, "ser_set_dtr: COM%d DTR=%s",
           Sp->PortNum, On ? "ON" : "OFF");

#ifdef _WIN32
    EscapeCommFunction(Sp->hCom, On ? SETDTR : CLRDTR);
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        union REGS Regs;                /* DOS interrupt registers       */
        Regs.h.ah = 0x06;              /* FOSSIL DTR control            */
        Regs.h.al = On ? 1 : 0;
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
    } else {
        unsigned char Mcr;              /* modem control register        */
        Mcr = inp(Sp->BaseAddr + UART_MCR);
        if (On) Mcr |= MCR_DTR; else Mcr &= ~MCR_DTR;
        outp(Sp->BaseAddr + UART_MCR, Mcr);
    }
#else
    {
        int Status;                     /* modem signal bits             */
        ioctl(Sp->Fd, TIOCMGET, &Status);
        if (On) Status |= TIOCM_DTR; else Status &= ~TIOCM_DTR;
        ioctl(Sp->Fd, TIOCMSET, &Status);
    }
#endif
}


/*-----------------------------------------------------------------------*/
/* ser_flush() -- Discard all data in serial port buffers                */
/*                                                                       */
/* Clears both the receive and transmit buffers. Called before sending   */
/* AT commands to ensure we read the response to our command, not        */
/* stale data from a previous exchange.                                  */
/*-----------------------------------------------------------------------*/

void ser_flush(SerPort *Sp)
{
    qf_log(LOG_DEBUG, "ser_flush: clearing buffers COM%d", Sp->PortNum);

#ifdef _WIN32
    FlushFileBuffers(Sp->hCom);
    PurgeComm(Sp->hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        union REGS Regs;                /* DOS interrupt registers       */
        Regs.h.ah = 0x08;              /* FOSSIL flush                  */
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
    }
    if (Sp->Type == SER_UART) {
        /* UART: drain by reading */
        while (inp(Sp->BaseAddr + UART_LSR) & LSR_DR)
            inp(Sp->BaseAddr + UART_RBR);
    }
#else
    tcflush(Sp->Fd, TCIOFLUSH);
#endif
}


/*-----------------------------------------------------------------------*/
/* ser_data_ready() -- Check if data is available to read               */
/*                                                                       */
/* Non-blocking check. Returns 1 if at least one byte is waiting in     */
/* the receive buffer, 0 if empty.                                       */
/*-----------------------------------------------------------------------*/

int ser_data_ready(SerPort *Sp)
{
#ifdef _WIN32
    {
        COMSTAT Cs;                     /* COM port status               */
        DWORD   Errors;                 /* error flags                   */
        ClearCommError(Sp->hCom, &Errors, &Cs);
        return (Cs.cbInQue > 0) ? 1 : 0;
    }
#elif defined(QF_DOS)
    if (Sp->Type == SER_FOSSIL) {
        union REGS Regs;                /* DOS interrupt registers       */
        Regs.h.ah = 0x03;
        Regs.w.dx = Sp->PortNum - 1;
        int386(0x14, &Regs, &Regs);
        return (Regs.h.ah & 0x01) ? 1 : 0;
    }
    return (inp(Sp->BaseAddr + UART_LSR) & LSR_DR) ? 1 : 0;
#else
    {
        int Bytes;                      /* bytes waiting in buffer       */
        ioctl(Sp->Fd, FIONREAD, &Bytes);
        return (Bytes > 0) ? 1 : 0;
    }
#endif
}
