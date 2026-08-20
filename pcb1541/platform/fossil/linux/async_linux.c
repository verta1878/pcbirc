/* ===========================================================================
   netfosll — Linux FOSSIL / ASYNC Layer
   GPLv3 — Copyright (C) 2026 wrench (netmodem2irc)

   Linux implementation of the 23 ASYNC functions PCBoard uses for
   serial I/O. Replaces FOSSIL INT 14h with socket/pty operations.

   Design: hexadecimal (LINUX_SERIAL_PLAN.md)
   Socket backend: sysop/0 (m_fossil_socket.pas)
   C implementation: wrench

   Build:
     wcc386 async_linux.c -bt=linux -mf -5 -ox
   =========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

/* MSG_DONTWAIT may not exist on all platforms */
#ifndef MSG_DONTWAIT
#define MSG_DONTWAIT 0x40
#endif

/* --- Globals (PCBoard expects these) --- */

volatile char CDokay    = 0;   /* 1 = connected, 0 = disconnected */
volatile char _CTSokay  = 1;   /* always 1 on sockets */
volatile char _RingDetect = 0; /* always 0 */
char CDup = 0;                  /* 1 = connected */

/* UART chip flags — all 0, no hardware */
char B8250 = 0, B16550 = 0, B16550A = 0, B16650 = 0;

/* Byte counters */
volatile int _InBytes  = 0;
volatile int _OutBytes = 0;

/* Error counters — always 0 on sockets */
volatile int _OverrunErrors = 0;
volatile int _ParityErrors  = 0;
volatile int _FramingErrors = 0;

/* --- Internal state --- */

static int comm_fd = -1;       /* socket file descriptor */
static int listen_fd = -1;     /* listening socket (accept mode) */

/* --- ASYNC Functions (23 total) --- */

/* 1. ASYNC_INIT: initialize the communication channel */
int ASYNC_INIT(int irq, int addr, char *inbuf, char *outbuf,
               int insize, int outsize, int cts, int share)
{
    const char *fd_env;
    int port;

    (void)irq; (void)addr; (void)inbuf; (void)outbuf;
    (void)insize; (void)outsize; (void)cts; (void)share;

    /* Ignore SIGPIPE — broken pipe should return error, not kill us */
    signal(SIGPIPE, SIG_IGN);

    /* Option A: fd passed via environment variable */
    fd_env = getenv("PCBFD");
    if (fd_env != NULL) {
        comm_fd = atoi(fd_env);
        if (comm_fd >= 0) {
            CDokay = 1;
            CDup = 1;
            return 0;
        }
    }

    /* Option B: accept on localhost port (from PCBPORT env) */
    fd_env = getenv("PCBPORT");
    if (fd_env != NULL) {
        struct sockaddr_in sa;
        port = atoi(fd_env);

        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd < 0) return -1;

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sa.sin_port = htons(port);

        if (bind(listen_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            close(listen_fd);
            listen_fd = -1;
            return -1;
        }

        listen(listen_fd, 1);
        comm_fd = accept(listen_fd, NULL, NULL);
        close(listen_fd);
        listen_fd = -1;

        if (comm_fd < 0) return -1;

        CDokay = 1;
        CDup = 1;
        return 0;
    }

    /* Option C: connect to localhost port (from NMPORT env) */
    fd_env = getenv("NMPORT");
    if (fd_env != NULL) {
        struct sockaddr_in sa;
        port = atoi(fd_env);

        comm_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (comm_fd < 0) return -1;

        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        sa.sin_port = htons(port);

        if (connect(comm_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            close(comm_fd);
            comm_fd = -1;
            return -1;
        }

        CDokay = 1;
        CDup = 1;
        return 0;
    }

    return -1;  /* no configuration found */
}

/* 2. ASYNC_OPENCOM: set baud rate — no-op for sockets */
void ASYNC_OPENCOM(long baud, int databits)
{
    (void)baud; (void)databits;
}

/* 3. ASYNC_CLOSECOM: close the connection */
void ASYNC_CLOSECOM(void)
{
    if (comm_fd >= 0) {
        close(comm_fd);
        comm_fd = -1;
    }
    CDokay = 0;
    CDup = 0;
}

/* 4. ASYNC_COMMINKEY: non-blocking read one character */
int ASYNC_COMMINKEY(void)
{
    fd_set fds;
    struct timeval tv;
    unsigned char ch;

    if (comm_fd < 0) return -1;

    FD_ZERO(&fds);
    FD_SET(comm_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 10000;  /* 10ms timeout — yield CPU */

    if (select(comm_fd + 1, &fds, NULL, NULL, &tv) > 0) {
        if (read(comm_fd, &ch, 1) == 1) {
            _InBytes++;
            return (int)ch;
        }
        /* read failed = disconnected */
        CDokay = 0;
        CDup = 0;
        return -1;
    }
    return -1;  /* no data */
}

/* 5. ASYNC_CSENDBYTE: send one character */
void ASYNC_CSENDBYTE(unsigned char ch)
{
    if (comm_fd < 0) return;
    if (write(comm_fd, &ch, 1) == 1)
        _OutBytes++;
    else {
        CDokay = 0;
        CDup = 0;
    }
}

/* 6. ASYNC_CSENDSTR: send a string */
void ASYNC_CSENDSTR(const char *str, int len)
{
    int sent;
    if (comm_fd < 0) return;
    sent = write(comm_fd, str, len);
    if (sent > 0)
        _OutBytes += sent;
    else {
        CDokay = 0;
        CDup = 0;
    }
}

/* 7. ASYNC_CGETBUF: read up to maxlen bytes */
int ASYNC_CGETBUF(char *buf, int maxlen)
{
    int n;
    if (comm_fd < 0) return 0;
    n = read(comm_fd, buf, maxlen);
    if (n > 0) {
        _InBytes += n;
        return n;
    }
    if (n == 0) {
        CDokay = 0;
        CDup = 0;
    }
    return 0;
}

/* 8. ASYNC_CGETSTR: read until CR/LF or maxlen */
int ASYNC_CGETSTR(char *buf, int maxlen)
{
    int i = 0;
    int ch;
    while (i < maxlen - 1) {
        ch = ASYNC_COMMINKEY();
        if (ch < 0) break;
        buf[i++] = (char)ch;
        if (ch == '\r' || ch == '\n') break;
    }
    buf[i] = '\0';
    return i;
}

/* 9. ASYNC_CHECKCOMM: check if data is waiting */
int ASYNC_CHECKCOMM(void)
{
    fd_set fds;
    struct timeval tv;

    if (comm_fd < 0) return 0;

    FD_ZERO(&fds);
    FD_SET(comm_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return (select(comm_fd + 1, &fds, NULL, NULL, &tv) > 0) ? 1 : 0;
}

/* 10. ASYNC_ONLINE: check if connection is still alive */
int ASYNC_ONLINE(void)
{
    char probe;
    if (comm_fd < 0) return 0;
    /* MSG_PEEK: check without consuming data */
    if (recv(comm_fd, &probe, 1, MSG_PEEK | MSG_DONTWAIT) == 0) {
        CDokay = 0;
        CDup = 0;
        return 0;
    }
    return CDokay;
}

/* 11. ASYNC_CDSTILLUP: same as ONLINE */
int ASYNC_CDSTILLUP(void)
{
    return ASYNC_ONLINE();
}

/* 12. ASYNC_CLEARINBUF: flush input buffer */
void ASYNC_CLEARINBUF(void)
{
    char discard[256];
    if (comm_fd < 0) return;
    /* Read and discard until empty (max 64 iterations) */
    int iter = 0;
    while (ASYNC_CHECKCOMM() && iter++ < 64)
        { int r = (int)read(comm_fd, discard, sizeof(discard)); (void)r; }
}

/* 13. ASYNC_CLEAROUTBUF: flush output buffer — no-op for sockets */
void ASYNC_CLEAROUTBUF(void)
{
    /* TCP handles buffering; nothing to flush */
}

/* 14-16. No-ops */
void ASYNC_COMMGO(void) { }
void ASYNC_COMMSTOP(void) { }
void ASYNC_COMMPAUSE(void) { usleep(10000); }  /* 10ms yield */

/* 17. ASYNC_TURNONDTR: no-op (no hardware lines) */
void ASYNC_TURNONDTR(void) { }

/* 18. ASYNC_TURNOFFDTR: simulate hangup */
void ASYNC_TURNOFFDTR(void)
{
    if (comm_fd >= 0)
        shutdown(comm_fd, SHUT_WR);
    CDokay = 0;
    CDup = 0;
}

/* 19-22. No-ops (no hardware) */
void ASYNC_TURNONRTS(void) { }
void ASYNC_TURNOFFRTS(void) { }
void ASYNC_TURNONFIFO(void) { }
void ASYNC_TURNONXMIT(void) { }

/* 23. ASYNC_SETPORT: no-op */
void ASYNC_SETPORT(long baud, int databits) { (void)baud; (void)databits; }
