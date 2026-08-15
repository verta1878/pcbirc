/* ====================================================================
 * serial.c — Serial Port Abstraction Layer
 * ====================================================================
 * Portable serial I/O across:
 *   - Win32 (CreateFile COM port)
 *   - Linux/BSD (termios /dev/ttyS*)
 *   - DOS FOSSIL (INT 14h via CYFOSSIL)
 *   - DOS UART (direct 16550 register access)
 *
 * From binary: "Using UART for communications"
 *              "Using fossil for communications"
 *              "Using DigiBoard for communications"
 *              "Enabling 16550 UART buffer"
 * ==================================================================== */

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

/* ---- Serial Port Handle ---- */

typedef enum {
    SER_NONE = 0,
    SER_UART,                     /* Direct 16550 UART (DOS)      */
    SER_FOSSIL,                   /* FOSSIL driver INT 14h (DOS)  */
    SER_WIN32,                    /* Win32 COM port               */
    SER_POSIX                     /* Linux/BSD /dev/ttyS*         */
} SerType;

typedef struct {
    SerType  type;
    int      port_num;            /* COM port number (1-based)    */
    uint32_t baud;                /* Current baud rate            */
    int      flow_rtscts;         /* RTS/CTS hardware flow        */
    int      flow_xonxoff;        /* XON/XOFF software flow       */

#ifdef _WIN32
    HANDLE   hCom;                /* Win32 file handle            */
#elif defined(QF_DOS)
    uint16_t base_addr;           /* UART base I/O address        */
    int      has_fifo;            /* 16550 FIFO available         */
    int      fossil_active;       /* FOSSIL driver present        */
#else
    int      fd;                  /* POSIX file descriptor        */
    struct termios orig_tio;      /* Original terminal settings   */
#endif
} SerPort;


/* ---- Standard UART base addresses ---- */
#ifdef QF_DOS
static const uint16_t uart_bases[] = {
    0x3F8,  /* COM1 */
    0x2F8,  /* COM2 */
    0x3E8,  /* COM3 */
    0x2E8   /* COM4 */
};

/* 16550 UART registers */
#define UART_RBR    0   /* Receive Buffer (read)               */
#define UART_THR    0   /* Transmit Holding (write)            */
#define UART_IER    1   /* Interrupt Enable                    */
#define UART_IIR    2   /* Interrupt Identification (read)     */
#define UART_FCR    2   /* FIFO Control (write)                */
#define UART_LCR    3   /* Line Control                        */
#define UART_MCR    4   /* Modem Control                       */
#define UART_LSR    5   /* Line Status                         */
#define UART_MSR    6   /* Modem Status                        */
#define UART_DLL    0   /* Divisor Latch Low (DLAB=1)          */
#define UART_DLM    1   /* Divisor Latch High (DLAB=1)         */

#define LSR_DR      0x01  /* Data Ready                        */
#define LSR_THRE    0x20  /* TX Holding Register Empty          */
#define MSR_CTS     0x10  /* Clear To Send                     */
#define MSR_DSR     0x20  /* Data Set Ready                    */
#define MSR_DCD     0x80  /* Data Carrier Detect               */
#define MCR_DTR     0x01  /* Data Terminal Ready               */
#define MCR_RTS     0x02  /* Request To Send                   */
#endif /* QF_DOS */


/* ---- Open Serial Port ---- */

int ser_open(SerPort *sp, int port_num, int use_fossil)
{
    memset(sp, 0, sizeof(*sp));
    sp->port_num = port_num;
    sp->baud = 9600;

#ifdef _WIN32
    {
        char name[16];
        DCB dcb;
        COMMTIMEOUTS timeouts;

        snprintf(name, sizeof(name), "\\\\.\\COM%d", port_num);
        sp->hCom = CreateFileA(name, GENERIC_READ | GENERIC_WRITE,
                               0, NULL, OPEN_EXISTING, 0, NULL);
        if (sp->hCom == INVALID_HANDLE_VALUE) {
            qf_log(LOG_ERROR, "Cannot open COM%d", port_num);
            return -1;
        }

        /* Configure COM port — match original QFront behavior.
         * Original raises DTR+RTS on open, disables flow control
         * so writes don't block waiting for CTS from modem. */
        memset(&dcb, 0, sizeof(dcb));
        dcb.DCBlength = sizeof(dcb);
        GetCommState(sp->hCom, &dcb);
        dcb.BaudRate     = 9600;
        dcb.ByteSize     = 8;
        dcb.Parity       = NOPARITY;
        dcb.StopBits     = ONESTOPBIT;
        dcb.fBinary      = TRUE;

        /* Flow control — disabled by default (original behavior).
         * CTS/DSR flow blocks writes if modem doesn't assert CTS.
         * XON/XOFF can interfere with Zmodem binary transfers. */
        dcb.fOutxCtsFlow = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutX        = FALSE;
        dcb.fInX         = FALSE;

        /* DTR+RTS control — raise both on open.
         * DTR tells modem "DTE is ready" — modem won't respond
         * to AT commands without DTR raised.
         * RTS tells modem "ready to receive" — needed for hardware
         * handshaking even when we disable CTS flow control. */
        dcb.fDtrControl  = DTR_CONTROL_ENABLE;
        dcb.fRtsControl  = RTS_CONTROL_ENABLE;

        /* Modem signal sensitivity */
        dcb.fDsrSensitivity = FALSE;  /* Don't ignore data if DSR off */
        dcb.fAbortOnError   = FALSE;  /* Don't abort on parity/frame err */
        dcb.fNull           = FALSE;  /* Don't discard null bytes */

        SetCommState(sp->hCom, &dcb);

        /* Set buffer sizes — 4K each like original FOSSIL buffers */
        SetupComm(sp->hCom, 4096, 4096);

        /* Set timeouts: 100ms read timeout */
        timeouts.ReadIntervalTimeout = 100;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 100;
        timeouts.WriteTotalTimeoutMultiplier = 0;
        timeouts.WriteTotalTimeoutConstant = 1000;
        SetCommTimeouts(sp->hCom, &timeouts);

        sp->type = SER_WIN32;
        qf_log(LOG_INFO, "Opening port COM%d (Win32)", port_num);
    }

#elif defined(QF_DOS)
    if (use_fossil) {
        /* Check for FOSSIL driver via INT 14h/AH=04 (init) */
        union REGS regs;
        regs.h.ah = 0x04;        /* FOSSIL init                 */
        regs.x.dx = port_num - 1;
        int86(0x14, &regs, &regs);

        if (regs.x.ax == 0x1954) {
            sp->type = SER_FOSSIL;
            sp->fossil_active = 1;
            qf_log(LOG_INFO, "Using fossil for communications (COM%d)",
                   port_num);
            return 0;
        }
        qf_log(LOG_WARN, "FOSSIL not found, falling back to UART");
    }

    /* Direct UART access */
    if (port_num < 1 || port_num > 4) {
        qf_log(LOG_ERROR, "Invalid port number: %d", port_num);
        return -1;
    }

    sp->base_addr = uart_bases[port_num - 1];

    /* Detect 16550 FIFO */
    outp(sp->base_addr + UART_FCR, 0xC1);  /* Enable FIFO */
    if (inp(sp->base_addr + UART_IIR) & 0xC0) {
        sp->has_fifo = 1;
        qf_log(LOG_INFO, "Enabling 16550 UART buffer");
        outp(sp->base_addr + UART_FCR, 0xC7);  /* Enable + 14-byte trigger */
    } else {
        sp->has_fifo = 0;
        outp(sp->base_addr + UART_FCR, 0x00);  /* Disable FIFO */
        qf_log(LOG_INFO, "16550 UART not found");
        qf_log(LOG_INFO, "Using UART for communications");
    }

    sp->type = SER_UART;
    qf_log(LOG_INFO, "Base address = %04Xh", sp->base_addr);

#else
    /* POSIX */
    {
        char devname[32];
        struct termios tio;

        snprintf(devname, sizeof(devname), "/dev/ttyS%d", port_num - 1);
        sp->fd = open(devname, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (sp->fd < 0) {
            /* Try USB serial */
            snprintf(devname, sizeof(devname), "/dev/ttyUSB%d", port_num - 1);
            sp->fd = open(devname, O_RDWR | O_NOCTTY | O_NONBLOCK);
        }
        if (sp->fd < 0) {
            qf_log(LOG_ERROR, "Cannot open %s: %s", devname, strerror(errno));
            return -1;
        }

        /* Save original settings */
        tcgetattr(sp->fd, &sp->orig_tio);

        /* Raw mode: 9600 8N1 */
        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
        tio.c_iflag = 0;
        tio.c_oflag = 0;
        tio.c_lflag = 0;
        tio.c_cc[VMIN]  = 0;
        tio.c_cc[VTIME] = 1;     /* 100ms timeout               */
        tcsetattr(sp->fd, TCSANOW, &tio);

        /* Clear O_NONBLOCK after setup */
        fcntl(sp->fd, F_SETFL, 0);

        sp->type = SER_POSIX;
        qf_log(LOG_INFO, "Opening port %s", devname);
    }
#endif

    return 0;
}


/* ---- Close Port ---- */

void ser_close(SerPort *sp)
{
    if (!sp) return;

#ifdef _WIN32
    if (sp->type == SER_WIN32 && sp->hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(sp->hCom);
        sp->hCom = INVALID_HANDLE_VALUE;
    }
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        union REGS regs;
        regs.h.ah = 0x05;        /* FOSSIL deinit               */
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
    }
    if (sp->has_fifo)
        outp(sp->base_addr + UART_FCR, 0x00);
#else
    if (sp->type == SER_POSIX && sp->fd >= 0) {
        tcsetattr(sp->fd, TCSANOW, &sp->orig_tio);
        close(sp->fd);
        sp->fd = -1;
    }
#endif

    qf_log(LOG_DEBUG, "Closing port COM%d", sp->port_num);
    sp->type = SER_NONE;
}


/* ---- Read Byte (with timeout in ms, -1 = no data) ---- */

int ser_read_byte(SerPort *sp, int timeout_ms)
{
#ifdef _WIN32
    {
        BYTE b;
        DWORD got;
        COMMTIMEOUTS ct;
        GetCommTimeouts(sp->hCom, &ct);
        ct.ReadTotalTimeoutConstant = timeout_ms;
        SetCommTimeouts(sp->hCom, &ct);
        if (ReadFile(sp->hCom, &b, 1, &got, NULL) && got == 1)
            return (int)b;
        return -1;
    }
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        union REGS regs;
        regs.h.ah = 0x02;        /* FOSSIL read with wait       */
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
        if (regs.h.ah == 0)
            return (int)regs.h.al;
        return -1;
    } else {
        /* Direct UART polling */
        long deadline = clock() + (long)timeout_ms * (CLOCKS_PER_SEC / 1000);
        while (clock() < deadline) {
            if (inp(sp->base_addr + UART_LSR) & LSR_DR)
                return (int)inp(sp->base_addr + UART_RBR);
        }
        return -1;
    }
#else
    {
        unsigned char b;
        fd_set fds;
        struct timeval tv;
        FD_ZERO(&fds);
        FD_SET(sp->fd, &fds);
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        if (select(sp->fd + 1, &fds, NULL, NULL, &tv) > 0) {
            if (read(sp->fd, &b, 1) == 1)
                return (int)b;
        }
        return -1;
    }
#endif
}


/* ---- Write Bytes ---- */

int ser_write(SerPort *sp, const void *buf, int len)
{
#ifdef _WIN32
    {
        DWORD written;
        WriteFile(sp->hCom, buf, len, &written, NULL);
        return (int)written;
    }
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        const unsigned char *p = (const unsigned char *)buf;
        int i;
        for (i = 0; i < len; i++) {
            union REGS regs;
            regs.h.ah = 0x01;    /* FOSSIL write                */
            regs.h.al = p[i];
            regs.x.dx = sp->port_num - 1;
            int86(0x14, &regs, &regs);
        }
        return len;
    } else {
        const unsigned char *p = (const unsigned char *)buf;
        int i;
        for (i = 0; i < len; i++) {
            while (!(inp(sp->base_addr + UART_LSR) & LSR_THRE))
                ;                 /* Wait for TX ready           */
            outp(sp->base_addr + UART_THR, p[i]);
        }
        return len;
    }
#else
    return (int)write(sp->fd, buf, len);
#endif
}


/* ---- Write String ---- */

int ser_write_str(SerPort *sp, const char *str)
{
    return ser_write(sp, str, (int)strlen(str));
}


/* ---- Set Baud Rate ---- */

int ser_set_baud(SerPort *sp, uint32_t baud)
{
    sp->baud = baud;

#ifdef _WIN32
    {
        DCB dcb;
        GetCommState(sp->hCom, &dcb);
        dcb.BaudRate = baud;
        /* Preserve flow control settings from ser_open */
        SetCommState(sp->hCom, &dcb);
        qf_log(LOG_INFO, "Baud rate set to %lu", (unsigned long)baud);
    }
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        /* FOSSIL Fn00: set baud via INT 14h */
        union REGS regs;
        unsigned char code;
        /* Map baud to FOSSIL code */
        if      (baud <= 300)    code = 0x43;
        else if (baud <= 1200)   code = 0x83;
        else if (baud <= 2400)   code = 0xA3;
        else if (baud <= 9600)   code = 0xE3;
        else if (baud <= 19200)  code = 0x03;
        else if (baud <= 38400)  code = 0x23;
        else                     code = 0xE3; /* Default 9600 */
        regs.h.ah = 0x00;
        regs.h.al = code;
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
    } else {
        /* Direct UART: set divisor latch */
        uint16_t divisor = (uint16_t)(115200UL / baud);
        unsigned char lcr = inp(sp->base_addr + UART_LCR);
        outp(sp->base_addr + UART_LCR, lcr | 0x80);  /* DLAB=1 */
        outp(sp->base_addr + UART_DLL, divisor & 0xFF);
        outp(sp->base_addr + UART_DLM, divisor >> 8);
        outp(sp->base_addr + UART_LCR, lcr);          /* DLAB=0 */
    }
#else
    {
        struct termios tio;
        speed_t spd;
        switch (baud) {
        case 300:    spd = B300;    break;
        case 1200:   spd = B1200;   break;
        case 2400:   spd = B2400;   break;
        case 4800:   spd = B4800;   break;
        case 9600:   spd = B9600;   break;
        case 19200:  spd = B19200;  break;
        case 38400:  spd = B38400;  break;
        case 57600:  spd = B57600;  break;
        case 115200: spd = B115200; break;
        case 230400: spd = B230400; break;
        default:     spd = B9600;   break;
        }
        tcgetattr(sp->fd, &tio);
        cfsetispeed(&tio, spd);
        cfsetospeed(&tio, spd);
        tcsetattr(sp->fd, TCSANOW, &tio);
    }
#endif

    qf_log(LOG_DEBUG, "Baud rate set to %lu", (unsigned long)baud);
    return 0;
}


/* ---- Get DCD (Carrier Detect) ---- */

int ser_get_dcd(SerPort *sp)
{
#ifdef _WIN32
    {
        DWORD status;
        GetCommModemStatus(sp->hCom, &status);
        return (status & MS_RLSD_ON) ? 1 : 0;
    }
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        union REGS regs;
        regs.h.ah = 0x03;        /* FOSSIL status               */
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
        return (regs.h.al & 0x80) ? 1 : 0;
    }
    return (inp(sp->base_addr + UART_MSR) & MSR_DCD) ? 1 : 0;
#else
    {
        int status;
        ioctl(sp->fd, TIOCMGET, &status);
        return (status & TIOCM_CD) ? 1 : 0;
    }
#endif
}


/* ---- Set DTR ---- */

void ser_set_dtr(SerPort *sp, int on)
{
#ifdef _WIN32
    EscapeCommFunction(sp->hCom, on ? SETDTR : CLRDTR);
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        union REGS regs;
        regs.h.ah = 0x06;        /* FOSSIL DTR control          */
        regs.h.al = on ? 1 : 0;
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
    } else {
        unsigned char mcr = inp(sp->base_addr + UART_MCR);
        if (on) mcr |= MCR_DTR; else mcr &= ~MCR_DTR;
        outp(sp->base_addr + UART_MCR, mcr);
    }
#else
    {
        int status;
        ioctl(sp->fd, TIOCMGET, &status);
        if (on) status |= TIOCM_DTR; else status &= ~TIOCM_DTR;
        ioctl(sp->fd, TIOCMSET, &status);
    }
#endif
}


/* ---- Flush Buffers ---- */

void ser_flush(SerPort *sp)
{
#ifdef _WIN32
    FlushFileBuffers(sp->hCom);
    PurgeComm(sp->hCom, PURGE_RXCLEAR | PURGE_TXCLEAR);
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        union REGS regs;
        regs.h.ah = 0x08;        /* FOSSIL flush                */
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
    }
    /* UART: drain by reading */
    if (sp->type == SER_UART) {
        while (inp(sp->base_addr + UART_LSR) & LSR_DR)
            inp(sp->base_addr + UART_RBR);
    }
#else
    tcflush(sp->fd, TCIOFLUSH);
#endif
}


/* ---- Check if Data Available ---- */

int ser_data_ready(SerPort *sp)
{
#ifdef _WIN32
    {
        COMSTAT cs;
        DWORD errors;
        ClearCommError(sp->hCom, &errors, &cs);
        return (cs.cbInQue > 0) ? 1 : 0;
    }
#elif defined(QF_DOS)
    if (sp->type == SER_FOSSIL) {
        union REGS regs;
        regs.h.ah = 0x03;
        regs.x.dx = sp->port_num - 1;
        int86(0x14, &regs, &regs);
        return (regs.h.ah & 0x01) ? 1 : 0;
    }
    return (inp(sp->base_addr + UART_LSR) & LSR_DR) ? 1 : 0;
#else
    {
        int bytes;
        ioctl(sp->fd, FIONREAD, &bytes);
        return (bytes > 0) ? 1 : 0;
    }
#endif
}
