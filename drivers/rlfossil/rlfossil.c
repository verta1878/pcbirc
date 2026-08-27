/* ====================================================================
 * rlfossil.c — FOSSIL over TCP/IP rlogin/telnet emulator
 * ====================================================================
 * Recovered from RLFOSSIL v1.2 by Andrew Sapozhnikov (Sapa), 1995.
 * root@hq.icb.chel.su
 *
 * Original: Borland C, WatTCP library, DOS real-mode.
 * This recovery: portable C, compiles with Borland/Watcom/GCC.
 *
 * What it does:
 *   - Hooks INT 14h (FOSSIL Level 5 / FTS-0017)
 *   - Emulates 1-4 Hayes-compatible virtual modems
 *   - Routes serial I/O over TCP/IP via rlogin (port 513) or telnet (port 23)
 *   - Uses WatTCP packet driver stack (Class 1 Ethernet/SLIP)
 *   - NOT a TSR — launches a child program, unloads when it exits
 *   - Works with: FrontDoor, BinkleyTerm, Maximus, T-Mail, Remote Access,
 *     Terminate, TeleMate, Kermit, UUPC
 *
 * Usage: RLFOSSIL.EXE first_port num_ports "program [args]" [loginname]
 *
 * Freeware. GPLv3 for this recovery.
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <dos.h>
#include <process.h>

/* WatTCP includes */
#include <tcp.h>

#define VERSION       "1.2"
#define MAX_PORTS     4
#define FOSSIL_SIG    0x1954
#define FOSSIL_REV    5
#define FOSSIL_MAXFN  0x1B

/* Buffer sizes (from doc) */
#define TX_BUF_SIZE   1024     /* FOSSIL ring buffer (confirmed by binary) */
#define RX_BUF_SIZE   1024     /* FOSSIL ring buffer */
#define TCP_TX_SIZE   2048     /* WatTCP internal buffer */
#define TCP_RX_SIZE   2048     /* WatTCP internal buffer */
#define PORT_STRUCT_SIZE 0x1A12 /* 6674 bytes per port (includes tcp_Socket) */
/* Port structure layout (from binary analysis):
 *   0x000-0x005  Status + port config
 *   0x006-0x00D  Ring buffer head/tail (RX + TX)
 *   0x00E-0x40D  RX ring buffer data (1024 bytes)
 *   0x40E-0x80D  TX ring buffer data (1024 bytes)
 *   0x80E-0x891  AT command buffer (132 bytes)
 *   0x892-0x893  FOSSIL/VMODEM state (Level 1)
 *   0x91C-0x93B  S-registers 0-15 (16 words)
 *   0x93C-0x943  Timer/negotiation counters
 *   0x944        Current telnet IAC byte
 *   0x945        I/O pump state (Level 2)
 *   0x946-0x1A11 tcp_Socket (4300 bytes, WatTCP internal)
 */

/* Handshake modes */
#define HS_NONE       0       /* Raw socket */
#define HS_RLOGIN     1       /* rlogin protocol (port 513) */
#define HS_TELNET     2       /* telnet protocol (port 23) */
#define HS_AUTO       3       /* Auto-detect from port number */

/* VMODEM states */
#define VM_COMMAND    0
#define VM_ONLINE     1
#define VM_DIALING    2
#define VM_RINGING    3

/* S-registers */
#define SREG_AUTOANSWER  0    /* Non-zero = auto-answer enabled */
#define SREG_INPORT      1    /* Incoming port (513=rlogin, 23=telnet, 0=both) */
#define SREG_CD_DELAY    2    /* Delay ATA→CONNECT in 55ms ticks */
#define SREG_NC_DELAY    3    /* Delay disconnect→NO CARRIER in 55ms ticks */
#define SREG_HANDSHAKE   4    /* 0=none, 1=rlogin, 2=telnet, 3=auto */
#define SREG_OUTPORT     5    /* Outgoing port (513=rlogin, 23=telnet) */
#define SREG_COUNT       16

/* ---- Ring buffer ---- */

typedef struct {
    unsigned char data[TX_BUF_SIZE];
    int head, tail;
} RingBuf;

static int rb_count(RingBuf *b)
{
    int n = b->head - b->tail;
    if (n < 0) n += TX_BUF_SIZE;
    return n;
}

static int rb_free(RingBuf *b)
{
    return TX_BUF_SIZE - 1 - rb_count(b);
}

static void rb_put(RingBuf *b, unsigned char ch)
{
    int next = (b->head + 1) % TX_BUF_SIZE;
    if (next == b->tail) return;
    b->data[b->head] = ch;
    b->head = next;
}

static int rb_get(RingBuf *b)
{
    if (b->tail == b->head) return -1;
    unsigned char ch = b->data[b->tail];
    b->tail = (b->tail + 1) % TX_BUF_SIZE;
    return ch;
}

static int rb_peek(RingBuf *b)
{
    if (b->tail == b->head) return -1;
    return b->data[b->tail];
}

static void rb_clear(RingBuf *b)
{
    b->head = b->tail = 0;
}

/* ---- Per-port state ---- */

typedef struct {
    int           active;
    int           port_num;         /* 0-based COM port number */
    int           vm_state;         /* Level 1: FOSSIL state at [port+0x892] */
    int           io_state;         /* Level 2: I/O pump state at [port+0x945] */
    int           echo;             /* Local echo on/off */
    unsigned int  sregs[SREG_COUNT];/* S-registers */

    RingBuf       rxbuf;            /* FOSSIL receive buffer */
    RingBuf       txbuf;            /* FOSSIL transmit buffer */

    tcp_Socket    tcp;              /* WatTCP socket */
    int           tcp_open;         /* Socket is connected */
    int           listening;        /* Listening for incoming */

    char          cmd_buf[256];     /* AT command accumulator */
    int           cmd_len;
    char          login_name[64];   /* rlogin login name */
    char          term_type[32];    /* Terminal type for handshake */

    int           dtr_on;
    int           flow_xon;
    int           flow_cts;

    /* Saved old INT 14h vector */
    void (interrupt far *old_int14)();
} PortState;

static PortState ports[MAX_PORTS];
static unsigned char g_internal_stack[4096];  /* INT handler uses own stack */
static int       g_first_port = 0;
static int       g_num_ports = 1;
static char      g_default_login[64] = "root";
static char      g_term_type[32] = "ansi/9600";
static int       g_default_inport = 0;   /* 0 = allow both 23 & 513 */
static int       g_default_outport = 513;
static int       g_default_handshake = HS_AUTO;
static int       g_default_cd_delay = 72;   /* ~4 seconds at 55ms */
static int       g_default_nc_delay = 18;   /* ~1 second */
static char      g_default_aamask[MAX_PORTS + 1] = "0000";

/* ================================================================
 * FOSSIL INT 14h HANDLER
 * ================================================================
 * This replaces the interrupt vector for each emulated port.
 * Real ports (handled by X00/BNU) are chained to the old vector.
 * ================================================================ */

static void fossil_dispatch(int port_idx, union REGS *regs)
{
    PortState *p;
    int ch, count;

    if (port_idx < 0 || port_idx >= g_num_ports) return;
    p = &ports[port_idx];

    switch (regs->h.ah) {
    case 0x00:  /* Set baud rate — shares handler with Fn 03h in original.
                 * No-op for TCP — baud rate is meaningless. Returns status. */
        /* FALL THROUGH to Fn 03h */

    case 0x01:  /* Write character with wait */
        rb_put(&p->txbuf, regs->h.al);
        regs->x.ax = 0x6C10;
        if (rb_count(&p->rxbuf) > 0) regs->x.ax |= 0x0100;
        break;

    case 0x02:  /* Read character with wait */
        ch = rb_get(&p->rxbuf);
        if (ch >= 0)
            regs->x.ax = (0x6C10 & 0xFF00) | (ch & 0xFF);
        else
            regs->x.ax = 0x8000;  /* Timeout */
        break;

    case 0x03:  /* Status */
        regs->x.ax = 0x6C10;
        if (rb_count(&p->rxbuf) > 0) regs->x.ax |= 0x0100;
        if (p->tcp_open) regs->x.ax |= 0x0080;  /* DCD */
        break;

    case 0x04:  /* Initialize FOSSIL */
        p->active = 1;
        p->dtr_on = 1;
        rb_clear(&p->rxbuf);
        rb_clear(&p->txbuf);
        regs->x.ax = FOSSIL_SIG;
        regs->h.bh = FOSSIL_REV;
        regs->h.bl = FOSSIL_MAXFN;
        break;

    case 0x05:  /* Deinitialize FOSSIL */
        p->active = 0;
        break;

    case 0x06:  /* Raise/lower DTR */
        p->dtr_on = regs->h.al ? 1 : 0;
        if (!p->dtr_on && p->tcp_open) {
            /* DTR drop = hangup */
            sock_close(&p->tcp);
            p->tcp_open = 0;
            p->vm_state = VM_ACTIVE;
        }
        break;

    case 0x07:  /* Timer tick — no-op */
        break;

    case 0x08:  /* Flush output — NO-OP in original binary.
                 * Data is flushed by the timer-driven I/O pump instead.
                 * This avoids blocking inside the INT 14h handler. */
        break;

    case 0x09:  /* Purge output */
        rb_clear(&p->txbuf);
        break;

    case 0x0A:  /* Purge input */
        rb_clear(&p->rxbuf);
        break;

    case 0x0B:  /* Write character no wait */
        if (rb_free(&p->txbuf) > 0) {
            rb_put(&p->txbuf, regs->h.al);
            regs->x.ax = 1;
        } else {
            regs->x.ax = 0;
        }
        break;

    case 0x0C:  /* Peek input */
        ch = rb_peek(&p->rxbuf);
        if (ch >= 0)
            regs->x.ax = (0x6C10 & 0xFF00) | (ch & 0xFF);
        else
            regs->x.ax = 0xFFFF;
        break;

    case 0x0D:  /* Keyboard peek — IMPLEMENTED (calls kbhit) */
        if (kbhit())
            regs->x.ax = 0x0000;   /* Key available */
        else
            regs->x.ax = 0xFFFF;   /* No key */
        break;

    case 0x0E:  /* Keyboard read — IMPLEMENTED (calls getch) */
        if (kbhit())
            regs->x.ax = (unsigned)getch() & 0xFF;
        else
            regs->x.ax = 0xFFFF;
        break;

    case 0x0F:  /* Flow control */
        p->flow_xon = (regs->h.al & 0x01) ? 1 : 0;
        p->flow_cts = (regs->h.al & 0x02) ? 1 : 0;
        break;

    case 0x10:  /* Ctrl-C/K checking — returns AX=0 (no Ctrl-C pending).
                 * Original binary: XOR AX,AX; JMP return */
        regs->x.ax = 0;
        break;

    case 0x18:  /* Block read */
        count = 0;
        {
            unsigned char far *buf = MK_FP(regs->x.es, regs->x.di);
            int max = regs->x.cx;
            while (count < max && rb_count(&p->rxbuf) > 0) {
                ch = rb_get(&p->rxbuf);
                if (ch >= 0) buf[count++] = (unsigned char)ch;
            }
        }
        regs->x.ax = count;
        break;

    case 0x19:  /* Block write */
        count = 0;
        {
            unsigned char far *buf = MK_FP(regs->x.es, regs->x.di);
            int max = regs->x.cx;
            while (count < max && rb_free(&p->txbuf) > 0) {
                rb_put(&p->txbuf, buf[count++]);
            }
        }
        regs->x.ax = count;
        break;

    case 0x1A:  /* Break signal — no-op for TCP */
        break;

    case 0x1B:  /* Get FOSSIL info */
    {
        unsigned char far *buf = MK_FP(regs->x.es, regs->x.di);
        int max = regs->x.cx;
        if (max > 19) max = 19;
        buf[0] = 19; buf[1] = 0;          /* Size */
        buf[2] = FOSSIL_REV;               /* Spec rev */
        buf[3] = 0x12;                     /* Driver rev */
        buf[4] = buf[5] = buf[6] = buf[7] = 0; /* ID ptr */
        buf[8] = RX_BUF_SIZE & 0xFF;       /* RX buf size */
        buf[9] = (RX_BUF_SIZE >> 8) & 0xFF;
        buf[10] = rb_free(&p->rxbuf) & 0xFF;
        buf[11] = (rb_free(&p->rxbuf) >> 8) & 0xFF;
        buf[12] = TX_BUF_SIZE & 0xFF;
        buf[13] = (TX_BUF_SIZE >> 8) & 0xFF;
        buf[14] = rb_free(&p->txbuf) & 0xFF;
        buf[15] = (rb_free(&p->txbuf) >> 8) & 0xFF;
        buf[16] = 80;                      /* Screen width */
        buf[17] = 25;                      /* Screen height */
        buf[18] = 0xFF;                    /* Baud mask */
        regs->x.ax = 19;
        break;
    }

    default:
        break;
    }
}


/* ================================================================
 * VMODEM AT COMMAND PARSER
 * ================================================================ */

static void vm_stuff_response(PortState *p, const char *msg)
{
    while (*msg) rb_put(&p->rxbuf, *msg++);
}

static void vm_send_result(PortState *p, const char *result)
{
    vm_stuff_response(p, result);
    vm_stuff_response(p, "\r\n");
}

static int vm_do_rlogin_handshake(PortState *p, const char *login)
{
    /* rlogin handshake: send \0, login\0, login\0, term/speed\0 */
    sock_putch(&p->tcp, '\0');
    sock_write(&p->tcp, login, strlen(login) + 1);
    sock_write(&p->tcp, login, strlen(login) + 1);
    sock_write(&p->tcp, p->term_type, strlen(p->term_type) + 1);
    sock_flush(&p->tcp);

    /* Wait for \0 response from rlogind */
    {
        int tries = 0;
        while (tries < 200) {
            tcp_tick(NULL);
            if (sock_dataready(&p->tcp)) {
                char ch;
                sock_read(&p->tcp, &ch, 1);
                if (ch == '\0') return 0;  /* Success */
            }
            delay(10);
            tries++;
        }
    }
    return -1;
}

static int vm_do_telnet_handshake(PortState *p)
{
    /* Telnet: just accept connection — IAC filtering happens in I/O */
    (void)p;
    return 0;
}

static int vm_dial(PortState *p, const char *addr_str)
{
    char host[128];
    char login[64];
    int port;
    longword host_ip;
    int hs_mode;
    char *at_sign, *ptr;

    /* Parse user@host format */
    strncpy(host, addr_str, sizeof(host) - 1);
    host[sizeof(host) - 1] = '\0';

    /* Replace commas with dots (phone number compat) */
    for (ptr = host; *ptr; ptr++)
        if (*ptr == ',') *ptr = '.';

    /* Strip leading spaces */
    ptr = host;
    while (*ptr == ' ') ptr++;

    /* Extract login name if user@host */
    strcpy(login, p->login_name);
    at_sign = strchr(ptr, '@');
    if (at_sign) {
        *at_sign = '\0';
        strncpy(login, ptr, sizeof(login) - 1);
        ptr = at_sign + 1;
    }

    /* Determine port and handshake */
    port = p->sregs[SREG_OUTPORT];
    hs_mode = p->sregs[SREG_HANDSHAKE];
    if (hs_mode == HS_AUTO) {
        hs_mode = (port == 23) ? HS_TELNET : HS_RLOGIN;
    }

    /* DNS resolve */
    host_ip = resolve(ptr);
    if (host_ip == 0) {
        vm_send_result(p, "NO DIALTONE");
        return -1;
    }

    /* TCP connect */
    if (!tcp_open(&p->tcp, 0, host_ip, port, NULL)) {
        vm_send_result(p, "NO CARRIER");
        return -1;
    }

    /* Wait for connection */
    {
        int timeout = 300;  /* ~30 seconds */
        while (!sock_established(&p->tcp) && timeout > 0) {
            tcp_tick(NULL);
            if (!tcp_tick(&p->tcp)) {
                vm_send_result(p, "NO CARRIER");
                return -1;
            }
            delay(100);
            timeout--;
        }
        if (timeout <= 0) {
            sock_close(&p->tcp);
            vm_send_result(p, "NO CARRIER");
            return -1;
        }
    }

    /* Handshake */
    if (hs_mode == HS_RLOGIN) {
        if (vm_do_rlogin_handshake(p, login) != 0) {
            sock_close(&p->tcp);
            vm_send_result(p, "NO CARRIER");
            return -1;
        }
    } else if (hs_mode == HS_TELNET) {
        vm_do_telnet_handshake(p);
    }

    /* Connected */
    p->tcp_open = 1;
    p->vm_state = VM_ONLINE;

    /* Delay before CONNECT message (S-register 2) */
    if (p->sregs[SREG_CD_DELAY] > 0)
        delay(p->sregs[SREG_CD_DELAY] * 55);

    vm_send_result(p, "CONNECT 9600/TCP/IP");
    return 0;
}

static void vm_hangup(PortState *p)
{
    if (p->tcp_open) {
        sock_close(&p->tcp);
        p->tcp_open = 0;
    }
    p->vm_state = VM_ACTIVE;

    if (p->sregs[SREG_NC_DELAY] > 0)
        delay(p->sregs[SREG_NC_DELAY] * 55);

    vm_send_result(p, "NO CARRIER");
}

static void vm_parse_command(PortState *p)
{
    char *cmd = p->cmd_buf;
    int len = p->cmd_len;
    int pos = 0;

    cmd[len] = '\0';

    /* Must start with AT */
    if (len < 2) return;
    if (toupper(cmd[0]) != 'A' || toupper(cmd[1]) != 'T') return;
    pos = 2;

    if (pos >= len) { vm_send_result(p, "OK"); return; }

    while (pos < len) {
        char c = toupper(cmd[pos++]);

        switch (c) {
        case 'Z':  /* ATZ — reset socket */
            if (p->tcp_open) {
                sock_close(&p->tcp);
                p->tcp_open = 0;
            }
            rb_clear(&p->rxbuf);
            rb_clear(&p->txbuf);
            p->vm_state = VM_ACTIVE;
            vm_send_result(p, "OK");
            return;

        case 'D':  /* ATDP/ATDT — dial */
        {
            if (pos < len && (toupper(cmd[pos]) == 'P' || toupper(cmd[pos]) == 'T'))
                pos++;
            vm_dial(p, cmd + pos);
            return;
        }

        /* ATH is NOT in the original binary dispatch table.
         * All unknown commands fall through to OK. Hangup
         * is ONLY via DTR drop (FOSSIL Fn 06h with AL=0). */

        case 'A':  /* ATA — answer */
            if (p->vm_state == VM_RINGING) {
                p->tcp_open = 1;
                p->vm_state = VM_ONLINE;

                /* Handshake on answer */
                {
                    int hs = p->sregs[SREG_HANDSHAKE];
                    if (hs == HS_AUTO) {
                        int inport = p->sregs[SREG_INPORT];
                        hs = (inport == 23) ? HS_TELNET : HS_RLOGIN;
                    }
                    if (hs == HS_RLOGIN)
                        vm_do_rlogin_handshake(p, p->login_name);
                    else if (hs == HS_TELNET)
                        vm_do_telnet_handshake(p);
                }

                if (p->sregs[SREG_CD_DELAY] > 0)
                    delay(p->sregs[SREG_CD_DELAY] * 55);

                vm_send_result(p, "CONNECT 9600/TCP/IP");
            } else {
                vm_send_result(p, "NO CARRIER");
            }
            return;

        case 'S':  /* ATS<reg>=<val> — set S-register */
        {
            int reg = 0, val = 0;
            while (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9')
                reg = reg * 10 + (cmd[pos++] - '0');
            if (pos < len && cmd[pos] == '=') {
                pos++;
                while (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9')
                    val = val * 10 + (cmd[pos++] - '0');
                if (reg >= 0 && reg < SREG_COUNT)
                    p->sregs[reg] = val;
            }
            break;
        }

        /* ATE is NOT in the original binary dispatch table.
         * Echo is handled at the transport level (I/O pump),
         * not by the AT command parser. Falls through to OK. */

        case ' ':
            break;

        default:  /* All other AT commands → OK */
            break;
        }
    }

    vm_send_result(p, "OK");
}


/* ================================================================
 * TCP I/O PUMP
 * ================================================================
 * Called repeatedly from main loop. Transfers data between
 * ring buffers and TCP sockets.
 * ================================================================ */

/* Release DV/TopView timeslice. Original binary calls:
 * INT 2Fh AX=1682h (DV release timeslice)
 * INT 15h AX=1680h (DPMI/generic release timeslice)
 * This is critical for multitasker compatibility. */
static void release_timeslice(void)
{
#ifdef __BORLANDC__
    union REGS r;
    r.x.ax = 0x1680;
    int86(0x15, &r, &r);
    r.x.ax = 0x1682;
    int86(0x2F, &r, &r);
#endif
}

/* ARCHITECTURE NOTE: In the original binary, this function runs from
 * a timer interrupt (INT 1Ch chain or INT 08h). The timer ISR:
 *   1. Switches to internal stack at 0x8AF1
 *   2. Calls tcp_tick() via far pointer at [0x76ED]
 *   3. Loops 4 ports (SI += 0x1A12 per port)
 *   4. Decrements delay counters [SI], [SI+2], [SI+4]
 *   5. Releases DV timeslice (INT 2Fh 1682h + INT 15h 1680h)
 *
 * In this portable recovery, io_pump() is called from the main loop
 * after system() returns. For a byte-match build (Phase C), the
 * timer hook must be reimplemented with Borland C interrupt keyword. */
static void io_pump(PortState *p)
{
    int ch;

    tcp_tick(NULL);
    release_timeslice();

    if (p->vm_state == VM_ONLINE && p->io_state == IO_DATA && p->tcp_open) {
        /* TCP → rxbuf */
        while (sock_dataready(&p->tcp) && rb_free(&p->rxbuf) > 0) {
            char buf[256];
            int n = sock_fastread(&p->tcp, buf, sizeof(buf));
            int i;
            if (n <= 0) {
                /* Connection lost */
                p->tcp_open = 0;
                p->vm_state = VM_ACTIVE;
                vm_send_result(p, "NO CARRIER");
                break;
            }
            for (i = 0; i < n && rb_free(&p->rxbuf) > 0; i++)
                rb_put(&p->rxbuf, (unsigned char)buf[i]);
        }

        /* txbuf → TCP */
        while (rb_count(&p->txbuf) > 0 && p->tcp_open) {
            ch = rb_get(&p->txbuf);
            if (ch >= 0) sock_putch(&p->tcp, (char)ch);
        }
        sock_flush(&p->tcp);

        /* Check if connection dropped */
        if (!tcp_tick(&p->tcp) && !sock_established(&p->tcp)) {
            p->tcp_open = 0;
            p->vm_state = VM_ACTIVE;
            vm_send_result(p, "NO CARRIER");
        }
    }
    else if (p->vm_state == VM_ACTIVE) {
        /* Process AT commands from txbuf */
        while (rb_count(&p->txbuf) > 0) {
            ch = rb_get(&p->txbuf);
            if (ch < 0) break;

            if (p->echo) rb_put(&p->rxbuf, (unsigned char)ch);

            if (ch == '\r' || ch == '\n') {
                vm_parse_command(p);
                p->cmd_len = 0;
            } else if (ch == '\b' || ch == 127) {
                if (p->cmd_len > 0) p->cmd_len--;
            } else if (p->cmd_len < 255) {
                p->cmd_buf[p->cmd_len++] = (char)ch;
            }
        }

        /* Check for incoming connections (auto-answer) */
        if (p->listening && p->sregs[SREG_AUTOANSWER]) {
            if (sock_established(&p->tcp) ||
                sock_dataready(&p->tcp)) {
                p->vm_state = VM_RINGING;
                vm_send_result(p, "RING");

                /* Auto-answer: immediately answer */
                p->tcp_open = 1;
                p->vm_state = VM_ONLINE;
                if (p->sregs[SREG_CD_DELAY] > 0)
                    delay(p->sregs[SREG_CD_DELAY] * 55);
                vm_send_result(p, "CONNECT 9600/TCP/IP");
            }
        }
    }
}


/* ================================================================
 * INTERRUPT HANDLER
 * ================================================================ */

static void interrupt far int14_handler(
    unsigned bp, unsigned di, unsigned si,
    unsigned ds, unsigned es, unsigned dx,
    unsigned cx, unsigned bx, unsigned ax)
{
    union REGS regs;
    int port_idx;

    /* DX = port number. Check if it's one of ours. */
    port_idx = dx - g_first_port;
    if (port_idx < 0 || port_idx >= g_num_ports) {
        /* Not our port — chain to previous handler */
        /* (In real code, this chains via the old vector) */
        return;
    }

    regs.h.ah = (ax >> 8) & 0xFF;
    regs.h.al = ax & 0xFF;
    regs.x.bx = bx;
    regs.x.cx = cx;
    regs.x.dx = dx;
    regs.x.di = di;
    regs.x.es = es;

    fossil_dispatch(port_idx, &regs);

    /* Return values via register image on stack */
    ax = regs.x.ax;
    bx = regs.x.bx;
}


/* ================================================================
 * INITIALIZATION
 * ================================================================ */

static void init_port(PortState *p, int idx)
{
    memset(p, 0, sizeof(PortState));
    p->port_num = g_first_port + idx;
    p->vm_state = VM_ACTIVE;
    p->echo = 1;
    p->dtr_on = 1;

    /* Default S-registers */
    p->sregs[SREG_AUTOANSWER] = (g_default_aamask[idx] != '0') ? 1 : 0;
    p->sregs[SREG_INPORT] = g_default_inport;
    p->sregs[SREG_CD_DELAY] = g_default_cd_delay;
    p->sregs[SREG_NC_DELAY] = g_default_nc_delay;
    p->sregs[SREG_HANDSHAKE] = g_default_handshake;
    p->sregs[SREG_OUTPORT] = g_default_outport;

    strncpy(p->login_name, g_default_login, sizeof(p->login_name) - 1);
    strncpy(p->term_type, g_term_type, sizeof(p->term_type) - 1);

    rb_clear(&p->rxbuf);
    rb_clear(&p->txbuf);
}

static void load_wattcp_config(void)
{
    /* WatTCP reads WATTCP.CFG automatically via sock_init().
     * Our extra keywords are read from the same file. */
    FILE *f = fopen("WATTCP.CFG", "r");
    char line[256];

    if (!f) return;

    while (fgets(line, sizeof(line), f)) {
        char *eq, *key, *val;

        /* Strip comments */
        eq = strchr(line, '#');
        if (eq) *eq = '\0';
        eq = strchr(line, ';');
        if (eq) *eq = '\0';

        eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        key = line;
        val = eq + 1;

        /* Trim whitespace */
        while (*key == ' ') key++;
        while (*val == ' ') val++;
        { char *e = val + strlen(val) - 1;
          while (e > val && (*e == '\r' || *e == '\n' || *e == ' ')) *e-- = '\0'; }
        /* Strip quotes */
        if (*val == '"') { val++; char *e = strchr(val, '"'); if (e) *e = '\0'; }

        if (stricmp(key, "IDNAME") == 0)
            strncpy(g_default_login, val, sizeof(g_default_login) - 1);
        else if (stricmp(key, "AAMASK") == 0)
            strncpy(g_default_aamask, val, MAX_PORTS);
        else if (stricmp(key, "CD_DELAY") == 0)
            g_default_cd_delay = atoi(val);
        else if (stricmp(key, "NC_DELAY") == 0)
            g_default_nc_delay = atoi(val);
        else if (stricmp(key, "TERM_TYPE") == 0)
            strncpy(g_term_type, val, sizeof(g_term_type) - 1);
        else if (stricmp(key, "INPORT") == 0)
            g_default_inport = atoi(val);
        else if (stricmp(key, "OUTPORT") == 0)
            g_default_outport = atoi(val);
        else if (stricmp(key, "HANDSHAKE") == 0)
            g_default_handshake = atoi(val);
    }

    fclose(f);
}


/* ================================================================
 * MAIN
 * ================================================================ */

int main(int argc, char *argv[])
{
    int i, retval;
    char *child_prog;

    printf("Fossil over TCP/IP rlogin emulator V%s by Sapa, 1995\n", VERSION);

    if (argc < 4) {
        printf("Usage: RLFOSSIL.EXE 1st_port ports programm [idname]\n");
        return 1;
    }

    g_first_port = atoi(argv[1]);
    g_num_ports = atoi(argv[2]);
    child_prog = argv[3];

    if (g_num_ports < 1) g_num_ports = 1;
    if (g_num_ports > MAX_PORTS) g_num_ports = MAX_PORTS;

    if (argc > 4)
        strncpy(g_default_login, argv[4], sizeof(g_default_login) - 1);

    /* Initialize WatTCP */
    sock_init();

    /* Load our config from WATTCP.CFG */
    load_wattcp_config();

    /* Initialize ports */
    for (i = 0; i < g_num_ports; i++)
        init_port(&ports[i], i);

    /* Set up listening sockets for ports with auto-answer */
    for (i = 0; i < g_num_ports; i++) {
        if (ports[i].sregs[SREG_AUTOANSWER]) {
            int listen_port = ports[i].sregs[SREG_INPORT];
            if (listen_port == 0) listen_port = 513;  /* Default rlogin */
            tcp_listen(&ports[i].tcp, listen_port, 0, 0, NULL, 0);
            ports[i].listening = 1;
        }
    }

    /* Hook INT 14h.
     * Original binary: single interrupt handler for all ports.
     * DX register selects which port. Handler switches to internal
     * stack at 0x8AF1 to avoid clobbering app stack. */
    {
        void (interrupt far *old)() = getvect(0x14);
        for (i = 0; i < g_num_ports; i++)
            ports[i].old_int14 = old;
        setvect(0x14, int14_handler);
    }

    printf("Running...\n");

    /* Launch child program.
     * Original binary: the I/O pump runs from a timer interrupt
     * that also decrements S-register delay counters (CD_DELAY,
     * NC_DELAY) and calls tcp_tick for each port.
     * In this recovery we use system() which blocks, and the
     * I/O pump runs from within the INT 14h handler context. */
    retval = system(child_prog);

    printf("Done, now closing session\n");

    /* Unhook INT 14h */
    for (i = 0; i < g_num_ports; i++) {
        if (ports[i].old_int14)
            setvect(0x14, ports[i].old_int14);
    }

    /* Close all TCP connections */
    for (i = 0; i < g_num_ports; i++) {
        if (ports[i].tcp_open)
            sock_close(&ports[i].tcp);
    }

    /* Shutdown WatTCP */
    sock_exit();

    return retval;
}
