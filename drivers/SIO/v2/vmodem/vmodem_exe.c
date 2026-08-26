/* ====================================================================
 * vmodem_exe.c — VMODEM.EXE Telnet/Rlogin Client for OS/2
 * ====================================================================
 * OS/2 console application. Opens shared memory from VMODEM.SYS,
 * reads AT commands from the app-side ring buffer, manages TCP/IP
 * connections, and writes received data back.
 *
 * Hayes AT commands: ATZ, ATD, ATDT, ATDP, ATH, ATA, ATE, ATS, ATI,
 *                    AT&D, AT&F, A/ (repeat), +++ (escape)
 * Protocols: rlogin (port 513), telnet (port 23), auto-detect
 * Telnet: IAC filtering, SGA+BINARY accepted
 *
 * Adapted from WinFOSSIL wf_core.c AT parser (GPLv3).
 * Clean-room reimplementation. GPLv3.
 * ==================================================================== */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define VERSION         "2.0.0"
#define VM_SHM_MAGIC    0x564D4F44
#define VM_BUF_SIZE     4096
#define CMD_BUF_LEN     256
#define DIAL_ADDR_LEN   256
#define VMP_PORT_DEFAULT 3141  /* IANA-assigned, matches v1's vmodem.c */

/* Ring buffer — must match VMODEM.SYS layout exactly */
typedef struct {
    volatile unsigned short head;
    volatile unsigned short tail;
    unsigned char           data[VM_BUF_SIZE];
} VM_RINGBUF;

typedef struct {
    unsigned long   magic;
    unsigned short  version;
    unsigned short  portIndex;
    unsigned char   dtr, rts, breakOn, flowMode;
    unsigned long   baudRate;
    unsigned char   lcr;
    unsigned char   msr, lsr, connected;
    volatile unsigned short events;
    VM_RINGBUF      txbuf;
    VM_RINGBUF      rxbuf;
} VM_SHARED, *PVM_SHARED;

/* VMODEM states */
#define VM_COMMAND  0
#define VM_ONLINE   1
#define VM_DIALING  2
#define VM_RINGING  3

/* Per-port VMODEM state */
typedef struct {
    PVM_SHARED  pShared;
    int         state;
    int         echo;
    int         autoAnswer;
    int         sock;               /* TCP socket (-1 = closed) */
    int         listenSock;         /* Listen socket (-1 = closed) */
    int         outPort;            /* Default outgoing port */
    int         handshake;          /* 0=none, 1=rlogin, 2=telnet, 3=auto */
    int         isVMP;              /* Virtual Modem Protocol: skip telnet
                                      * IAC filtering, default port 3141
                                      * (IANA-assigned), "CONNECT .../VMP"
                                      * result. Ported from v1's vmodem.c —
                                      * previously undone here because v2's
                                      * STATUS.md called VMP "proprietary,
                                      * skip," but v1's clean-room
                                      * implementation shows VMP has no
                                      * actual proprietary framing: it's
                                      * plain TCP with telnet negotiation
                                      * skipped, nothing more. See
                                      * STATUS.md's corrected B-5 entry. */
    int         dcdMode;            /* &C: 0=always on, 1=normal (from DCD) */
    int         dsrMode;            /* &S: 0=always on, 1=normal */
    int         postEcho;           /* F: 0=half duplex (echo), 1=full (no echo) */
    int         dialTimeout;        /* S7: seconds to wait for carrier (default 30) */
    int         inactivityTimer;    /* S19: minutes before auto-disconnect (0=off) */
    int         dtrDelay;           /* S38: seconds to wait after DTR drop (default 0) */
    unsigned long inactivityTick;   /* Last data activity timestamp */
    char        cmdBuf[CMD_BUF_LEN];
    int         cmdLen;
    char        lastCmd[CMD_BUF_LEN];
    int         lastLen;
    char        loginName[64];
    char        termType[32];
} VM_STATE;

/* Ring buffer helpers */
static int rb_count(VM_RINGBUF *b) {
    int n = b->head - b->tail; return (n < 0) ? n + VM_BUF_SIZE : n;
}
static int rb_free(VM_RINGBUF *b) { return VM_BUF_SIZE - 1 - rb_count(b); }
static void rb_put(VM_RINGBUF *b, unsigned char ch) {
    unsigned short next = (b->head + 1) % VM_BUF_SIZE;
    if (next != b->tail) { b->data[b->head] = ch; b->head = next; }
}
static int rb_get(VM_RINGBUF *b) {
    if (b->tail == b->head) return -1;
    unsigned char ch = b->data[b->tail];
    b->tail = (b->tail + 1) % VM_BUF_SIZE;
    return ch;
}

/* Stuff a string into the RX buffer (app reads this) */
static void stuff_rx(PVM_SHARED ps, const char *msg)
{
    while (*msg) rb_put(&ps->rxbuf, (unsigned char)*msg++);
}

static void send_result(PVM_SHARED ps, const char *result)
{
    stuff_rx(ps, result);
    stuff_rx(ps, "\r\n");
}

/* ================================================================
 * RLOGIN HANDSHAKE
 * ================================================================ */

static int do_rlogin(VM_STATE *v)
{
    char zero = '\0';

    send(v->sock, &zero, 1, 0);
    send(v->sock, v->loginName, strlen(v->loginName) + 1, 0);
    send(v->sock, v->loginName, strlen(v->loginName) + 1, 0);
    send(v->sock, v->termType, strlen(v->termType) + 1, 0);

    /* Wait for \0 response */
    {
        char ch; int tries = 0;
        while (tries < 200) {
            if (recv(v->sock, &ch, 1, 0) == 1 && ch == '\0')
                return 0;
            DosSleep(10);
            tries++;
        }
    }
    return -1;
}

/* ================================================================
 * TELNET IAC FILTER
 * ================================================================ */

/* Telnet option codes */
#define TELOPT_BINARY    0   /* Binary Transmission */
#define TELOPT_ECHO      1   /* Echo */
#define TELOPT_SGA       3   /* Suppress Go Ahead */
#define TELOPT_STATUS    5   /* Status */
#define TELOPT_TTYPE    24   /* Terminal Type */
#define TELOPT_NAWS     31   /* Negotiate About Window Size */
#define TELOPT_TSPEED   32   /* Terminal Speed */
#define TELOPT_XDISPLOC 35   /* X Display Location */
#define TELOPT_NEWENV   39   /* New Environment Option */

/* Should we accept this telnet option? */
static int telopt_accept(unsigned char opt)
{
    switch (opt) {
    case TELOPT_BINARY:  return 1;
    case TELOPT_ECHO:    return 1;
    case TELOPT_SGA:     return 1;
    case TELOPT_TTYPE:   return 1;
    case TELOPT_NAWS:    return 1;
    case TELOPT_TSPEED:  return 1;
    default:             return 0;
    }
}

/* Handle telnet subnegotiation (IAC SB ... IAC SE) */
static void handle_subneg(VM_STATE *v, unsigned char *sub, int sublen)
{
    if (sublen < 1) return;

    switch (sub[0]) {
    case TELOPT_TTYPE:
        /* Terminal Type: SB 24 1 IAC SE = "send your terminal type"
         * We respond: SB 24 0 <type> IAC SE */
        if (sublen >= 2 && sub[1] == 1) {
            unsigned char resp[64];
            int rlen = 0, tlen = (int)strlen(v->termType);
            resp[rlen++] = 0xFF;    /* IAC */
            resp[rlen++] = 0xFA;    /* SB */
            resp[rlen++] = TELOPT_TTYPE;
            resp[rlen++] = 0;       /* IS */
            memcpy(resp + rlen, v->termType, tlen); rlen += tlen;
            resp[rlen++] = 0xFF;    /* IAC */
            resp[rlen++] = 0xF0;    /* SE */
            send(v->sock, (char *)resp, rlen, 0);
        }
        break;

    case TELOPT_TSPEED:
        /* Terminal Speed: respond with baud rate */
        if (sublen >= 2 && sub[1] == 1) {
            unsigned char resp[32];
            int rlen = 0;
            char speed[16];
            snprintf(speed, sizeof(speed), "%lu,%lu", v->pShared->baudRate, v->pShared->baudRate);
            resp[rlen++] = 0xFF; resp[rlen++] = 0xFA;
            resp[rlen++] = TELOPT_TSPEED; resp[rlen++] = 0;
            memcpy(resp + rlen, speed, strlen(speed)); rlen += (int)strlen(speed);
            resp[rlen++] = 0xFF; resp[rlen++] = 0xF0;
            send(v->sock, (char *)resp, rlen, 0);
        }
        break;

    case TELOPT_NAWS:
        /* NAWS is sent by us, not requested — ignore subneg from server */
        break;
    }
}

/* Send NAWS (window size) notification */
static void send_naws(VM_STATE *v)
{
    unsigned char naws[9] = {
        0xFF, 0xFA, TELOPT_NAWS,
        0, 80,      /* Width: 80 columns */
        0, 25,      /* Height: 25 rows */
        0xFF, 0xF0  /* IAC SE */
    };
    send(v->sock, (char *)naws, 9, 0);
}

static int filter_telnet(VM_STATE *v, unsigned char *buf, int len)
{
    int src, dst = 0;

    for (src = 0; src < len; src++) {
        if (buf[src] == 0xFF && src + 1 < len) {
            unsigned char cmd = buf[src + 1];
            if (cmd == 0xFF) { buf[dst++] = 0xFF; src++; continue; }

            if (cmd >= 0xFB && cmd <= 0xFE && src + 2 < len) {
                unsigned char opt = buf[src + 2];
                unsigned char resp[3] = { 0xFF, 0, opt };

                if (cmd == 0xFB) {
                    /* WILL — server offers option */
                    resp[1] = telopt_accept(opt) ? 0xFD : 0xFE;
                } else if (cmd == 0xFC) {
                    /* WONT — server refuses, acknowledge */
                    resp[1] = 0xFE;  /* DONT */
                } else if (cmd == 0xFD) {
                    /* DO — server requests we enable option */
                    resp[1] = telopt_accept(opt) ? 0xFB : 0xFC;
                    /* Send NAWS data after accepting NAWS */
                    if (opt == TELOPT_NAWS && telopt_accept(opt))
                        send_naws(v);
                } else if (cmd == 0xFE) {
                    /* DONT — server demands we disable, acknowledge */
                    resp[1] = 0xFC;  /* WONT */
                }

                if (resp[1]) send(v->sock, (char *)resp, 3, 0);
                src += 2; continue;
            }

            if (cmd == 0xFA) {
                /* Subnegotiation: collect until IAC SE */
                unsigned char sub[128];
                int sublen = 0;
                src += 2;
                while (src < len - 1 && sublen < (int)sizeof(sub)) {
                    if (buf[src] == 0xFF && buf[src+1] == 0xF0) {
                        src++; break;
                    }
                    sub[sublen++] = buf[src++];
                }
                handle_subneg(v, sub, sublen);
                continue;
            }

            src++; continue;
        }
        buf[dst++] = buf[src];
    }
    return dst;
}

/* ================================================================
 * TCP CONNECT (ATD)
 * ================================================================ */

static int vm_dial(VM_STATE *v, const char *addr, int isVMP)
{
    char host[DIAL_ADDR_LEN];
    char *ptr, *at, *colon;
    int port;
    struct hostent *he;
    struct sockaddr_in sa;

    v->isVMP = isVMP;

    strncpy(host, addr, DIAL_ADDR_LEN - 1);
    host[DIAL_ADDR_LEN - 1] = '\0';

    /* Replace commas with dots (phone number compat) */
    for (ptr = host; *ptr; ptr++) if (*ptr == ',') *ptr = '.';

    /* Strip leading spaces */
    ptr = host;
    while (*ptr == ' ') ptr++;

    /* user@host extraction */
    at = strchr(ptr, '@');
    if (at) {
        *at = '\0';
        strncpy(v->loginName, ptr, 63);
        ptr = at + 1;
    }

    /* host:port — VMP defaults to 3141 (IANA-assigned) instead of 23 */
    port = isVMP ? VMP_PORT_DEFAULT : v->outPort;
    colon = strrchr(ptr, ':');
    if (colon) { *colon = '\0'; port = atoi(colon + 1); if (port <= 0) port = isVMP ? VMP_PORT_DEFAULT : 23; }

    /* DNS resolve */
    he = gethostbyname(ptr);
    if (!he) { send_result(v->pShared, "NO DIALTONE"); return -1; }

    /* TCP connect */
    v->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (v->sock < 0) { send_result(v->pShared, "NO CARRIER"); return -1; }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((unsigned short)port);
    memcpy(&sa.sin_addr, he->h_addr, he->h_length);

    if (connect(v->sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        close(v->sock); v->sock = -1;
        send_result(v->pShared, "NO CARRIER");
        return -1;
    }

    /* Handshake — VMP skips rlogin/telnet negotiation entirely; it's
     * plain TCP with nothing layered on top. */
    if (!isVMP) {
        int hs = v->handshake;
        if (hs == 3) hs = (port == 23) ? 2 : 1; /* auto */
        if (hs == 1 && do_rlogin(v) != 0) {
            close(v->sock); v->sock = -1;
            send_result(v->pShared, "NO CARRIER"); return -1; }
    }

    v->state = VM_ONLINE;
    v->pShared->connected = 1;
    /* Set modem status based on &C and &S modes */
    v->pShared->msr = 0x10;    /* CTS always on */
    if (v->dcdMode == 0) v->pShared->msr |= 0x80;  /* &C0: DCD always on */
    else v->pShared->msr |= 0x80;                   /* &C1: DCD on when connected */
    if (v->dsrMode == 0) v->pShared->msr |= 0x20;  /* &S0: DSR always on */
    else v->pShared->msr |= 0x20;                   /* &S1: DSR on when connected */

    { char msg[64];
      if (isVMP)
          snprintf(msg, sizeof(msg), "CONNECT %lu/ARQ/VMP", v->pShared->baudRate);
      else
          snprintf(msg, sizeof(msg), "CONNECT %lu/TCP/IP", v->pShared->baudRate);
      send_result(v->pShared, msg); }
    return 0;
}

static void vm_hangup(VM_STATE *v)
{
    if (v->sock >= 0) { close(v->sock); v->sock = -1; }
    v->state = VM_COMMAND;
    v->pShared->connected = 0;
    /* &C0 keeps DCD on even after disconnect, &C1 drops it */
    v->pShared->msr = 0x10;    /* CTS stays on */
    if (v->dcdMode == 0) v->pShared->msr |= 0x80;  /* &C0: DCD stays */
    if (v->dsrMode == 0) v->pShared->msr |= 0x20;  /* &S0: DSR stays */
    send_result(v->pShared, "NO CARRIER");
}

/* ================================================================
 * AT COMMAND PARSER
 * ================================================================ */

static void vm_parse(VM_STATE *v)
{
    char *cmd = v->cmdBuf;
    int len = v->cmdLen, pos = 0;
    cmd[len] = '\0';

    /* A/ = repeat last command */
    if (len >= 2 && toupper(cmd[0]) == 'A' && cmd[1] == '/') {
        if (v->lastLen > 0) {
            memcpy(v->cmdBuf, v->lastCmd, v->lastLen);
            v->cmdLen = len = v->lastLen; cmd = v->cmdBuf; cmd[len] = '\0';
        } else { send_result(v->pShared, "OK"); return; }
    }

    if (len < 2 || toupper(cmd[0]) != 'A' || toupper(cmd[1]) != 'T') return;
    memcpy(v->lastCmd, cmd, len); v->lastLen = len;
    pos = 2;

    if (pos >= len) { send_result(v->pShared, "OK"); return; }

    while (pos < len) {
        char c = toupper(cmd[pos++]);
        switch (c) {
        case 'Z':
            vm_hangup(v);
            v->pShared->txbuf.head = v->pShared->txbuf.tail = 0;
            v->pShared->rxbuf.head = v->pShared->rxbuf.tail = 0;
            send_result(v->pShared, "OK"); return;
        case 'D':
            if (pos < len && (toupper(cmd[pos]) == 'T' || toupper(cmd[pos]) == 'P')) pos++;
            if (pos < len && toupper(cmd[pos]) == 'V') {
                /* ATDV — dial VMP explicitly (matches v1's vmodem.c) */
                pos++;
                while (pos < len && cmd[pos] == ' ') pos++;
                vm_dial(v, cmd + pos, TRUE);
                return;
            }
            /* '#' prefix also means VMP, same convention as v1 */
            if (pos < len && cmd[pos] == '#') {
                vm_dial(v, cmd + pos + 1, TRUE);
                return;
            }
            vm_dial(v, cmd + pos, FALSE); return;
        case 'H':
        {
            int hval = 0;
            if (pos < len && isdigit(cmd[pos])) hval = cmd[pos++] - '0';
            if (hval == 0) {
                /* ATH0 — Hang up */
                if (v->pShared->connected) vm_hangup(v);
                else send_result(v->pShared, "OK");
            } else {
                /* ATH1 — Go off-hook (prepare modem, no-op for TCP) */
                send_result(v->pShared, "OK");
            }
            return;
        }
        case 'A':
            if (v->state == VM_RINGING) {
                v->state = VM_ONLINE; v->pShared->connected = 1;
                v->pShared->msr = 0xB0;
                send_result(v->pShared, "CONNECT 9600/TCP/IP");
            } return;
        case 'E':
            if (pos < len) { v->echo = (cmd[pos] != '0'); pos++; } break;
        case 'S':
        {
            int reg = 0, val = 0;

            /* S? with no register number = show all S-register help */
            if (pos < len && cmd[pos] == '?') {
                pos++;
                stuff_rx(v->pShared,
                    "\r\n  S0   Auto-answer (0=off, non-zero=on)\r\n"
                    "  S7   Dial timeout in seconds (default 30)\r\n"
                    "  S19  Inactivity timer in minutes (0=off)\r\n"
                    "  S38  DTR drop delay in seconds\r\n"
                    "  Sr=n Set register r to n\r\n"
                    "  Sr=? Display register r value\r\n");
                break;
            }

            while (pos < len && isdigit(cmd[pos]))
                reg = reg * 10 + (cmd[pos++] - '0');

            if (pos < len && cmd[pos] == '=') {
                pos++;
                /* Sr=? — display register value */
                if (pos < len && cmd[pos] == '?') {
                    pos++;
                    { char rbuf[32]; int rval = 0;
                      switch (reg) {
                      case 0:  rval = v->autoAnswer; break;
                      case 7:  rval = v->dialTimeout; break;
                      case 19: rval = v->inactivityTimer; break;
                      case 38: rval = v->dtrDelay; break;
                      }
                      snprintf(rbuf, sizeof(rbuf), "\r\nS%d=%d\r\n", reg, rval);
                      stuff_rx(v->pShared, rbuf);
                    }
                    break;
                }
                /* Sr=n — set register value */
                while (pos < len && isdigit(cmd[pos]))
                    val = val * 10 + (cmd[pos++] - '0');
                switch (reg) {
                case 0:
                    v->autoAnswer = val;
                    if (val > 0) {
                        v->listenSock = socket(AF_INET, SOCK_STREAM, 0);
                        if (v->listenSock >= 0) {
                            struct sockaddr_in la; int reuse = 1;
                            setsockopt(v->listenSock, SOL_SOCKET, SO_REUSEADDR,
                                       (char *)&reuse, sizeof(reuse));
                            memset(&la, 0, sizeof(la));
                            la.sin_family = AF_INET;
                            la.sin_addr.s_addr = INADDR_ANY;
                            la.sin_port = htons(val > 1 ? (unsigned short)val : 23);
                            bind(v->listenSock, (struct sockaddr *)&la, sizeof(la));
                            listen(v->listenSock, 1);
                        }
                    }
                    break;
                case 7:  v->dialTimeout = val; break;
                case 19: v->inactivityTimer = val; break;
                case 38: v->dtrDelay = val; break;
                }
            }
        } break;
        case 'I':
            { char id[64]; snprintf(id, sizeof(id), "VMODEM/SIO2K v%s", VERSION);
              send_result(v->pShared, id); } break;
        case '&':
            if (pos < len) {
                char ext = toupper(cmd[pos++]);
                if (ext == 'F') {
                    /* AT&F — Factory defaults */
                    v->echo = 1; v->autoAnswer = 0; v->outPort = 23;
                    v->handshake = 3; v->dcdMode = 1; v->dsrMode = 1;
                    v->postEcho = 1; v->dialTimeout = 30;
                    v->inactivityTimer = 0; v->dtrDelay = 0;
                    strncpy(v->termType, "ansi", 31);
                }
                else if (ext == 'D' && pos < len) pos++; /* AT&D0/&D2 */
                else if (ext == 'C') {
                    /* AT&C0 = DCD always on, AT&C1 = DCD follows carrier */
                    if (pos < len) { v->dcdMode = (cmd[pos] == '0') ? 0 : 1; pos++; }
                }
                else if (ext == 'S') {
                    /* AT&S0 = DSR always on, AT&S1 = DSR normal */
                    if (pos < len) { v->dsrMode = (cmd[pos] == '0') ? 0 : 1; pos++; }
                }
                else if (ext == 'T') {
                    /* AT&T"terminal_type" — set telnet terminal type */
                    if (pos < len && cmd[pos] == '"') {
                        pos++;
                        { int tlen = 0;
                          while (pos < len && cmd[pos] != '"' && tlen < 31)
                              v->termType[tlen++] = cmd[pos++];
                          v->termType[tlen] = '\0';
                          if (pos < len && cmd[pos] == '"') pos++;
                        }
                    }
                }
                else if (ext == 'V') {
                    /* AT&V — View current settings */
                    char vbuf[512]; int vlen = 0;
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "\r\n--- Virtual Modem Settings ---\r\n");
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Echo:      %s\r\n", v->echo ? "ON" : "OFF");
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  DCD mode:  &C%d (%s)\r\n", v->dcdMode,
                                   v->dcdMode ? "follows carrier" : "always on");
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  DSR mode:  &S%d (%s)\r\n", v->dsrMode,
                                   v->dsrMode ? "normal" : "always on");
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Duplex:    F%d (%s)\r\n", v->postEcho,
                                   v->postEcho ? "full" : "half");
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Term type: %s\r\n", v->termType);
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Handshake: %d (%s)\r\n", v->handshake,
                                   v->handshake == 1 ? "rlogin" :
                                   v->handshake == 2 ? "telnet" : "auto");
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Out port:  %d\r\n", v->outPort);
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Auto-ans:  S0=%d\r\n", v->autoAnswer);
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Dial tmout:S7=%d sec\r\n", v->dialTimeout);
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Inactivity:S19=%d min\r\n", v->inactivityTimer);
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  DTR delay: S38=%d sec\r\n", v->dtrDelay);
                    vlen += snprintf(vbuf + vlen, sizeof(vbuf) - vlen, "  Connected: %s\r\n",
                                   v->pShared->connected ? "YES" : "NO");
                    stuff_rx(v->pShared, vbuf);
                }
            } break;
        case 'C':
            /* ATC0 = enable CompuServe upload, ATC1 = disable (no-op compat) */
            if (pos < len && isdigit(cmd[pos])) pos++;
            break;
        case 'F':
            /* ATF0 = half duplex (echo after connect), ATF1 = full duplex */
            if (pos < len) { v->postEcho = (cmd[pos] == '0') ? 0 : 1; pos++; }
            break;
        case ' ': break;
        default: break;
        }
    }
    send_result(v->pShared, "OK");
}

/* ================================================================
 * MAIN I/O LOOP
 * ================================================================ */

static void io_loop(VM_STATE *v)
{
    PVM_SHARED ps = v->pShared;
    int ch;

    /* Inactivity timer (S19): disconnect after N minutes of no data */
    if (v->state == VM_ONLINE && v->inactivityTimer > 0) {
        unsigned long now;
        DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &now, sizeof(now));
        if (v->inactivityTick == 0) v->inactivityTick = now;
        if ((now - v->inactivityTick) > (unsigned long)v->inactivityTimer * 60000UL) {
            send_result(ps, "Connection timed out - no activity");
            vm_hangup(v);
            return;
        }
    }

    /* Check DTR drop event */
    if (ps->events & 0x0100) {
        ps->events &= ~0x0100;
        if (v->pShared->connected) vm_hangup(v);
    }

    if (v->state == VM_ONLINE && v->sock >= 0) {
        /* TCP → rxbuf */
        { unsigned char buf[512]; int n;
          fd_set fds; struct timeval tv = {0, 0};
          FD_ZERO(&fds); FD_SET(v->sock, &fds);
          if (select(v->sock + 1, &fds, NULL, NULL, &tv) > 0) {
              n = recv(v->sock, (char *)buf, sizeof(buf), 0);
              if (n > 0) {
                  /* VMP is raw TCP passthrough — no telnet IAC layer
                   * to filter, unlike a telnet/rlogin connection. */
                  if (!v->isVMP) n = filter_telnet(v, buf, n);
                  { int i; for (i = 0; i < n && rb_free(&ps->rxbuf) > 0; i++)
                      rb_put(&ps->rxbuf, buf[i]); }
                  /* Reset inactivity timer on data */
                  DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT,
                                  &v->inactivityTick, sizeof(v->inactivityTick));
              } else { vm_hangup(v); }
          }
        }
        /* txbuf → TCP */
        while (rb_count(&ps->txbuf) > 0 && v->sock >= 0) {
            ch = rb_get(&ps->txbuf);
            if (ch >= 0) { char c = (char)ch; send(v->sock, &c, 1, 0); }
        }
    }
    else if (v->state == VM_COMMAND) {
        /* Process AT commands from txbuf */
        while (rb_count(&ps->txbuf) > 0) {
            ch = rb_get(&ps->txbuf);
            if (ch < 0) break;
            if (v->echo) rb_put(&ps->rxbuf, (unsigned char)ch);
            if (ch == '\r' || ch == '\n') { vm_parse(v); v->cmdLen = 0; }
            else if (ch == '\b' || ch == 127) { if (v->cmdLen > 0) v->cmdLen--; }
            else if (v->cmdLen < CMD_BUF_LEN - 1) v->cmdBuf[v->cmdLen++] = (char)ch;
        }
        /* Check for incoming connections */
        if (v->listenSock >= 0 && v->autoAnswer) {
            fd_set fds; struct timeval tv = {0, 0};
            FD_ZERO(&fds); FD_SET(v->listenSock, &fds);
            if (select(v->listenSock + 1, &fds, NULL, NULL, &tv) > 0) {
                struct sockaddr_in ca; int clen = sizeof(ca);
                v->sock = accept(v->listenSock, (struct sockaddr *)&ca, &clen);
                if (v->sock >= 0) {
                    send_result(ps, "RING");
                    v->state = VM_ONLINE; ps->connected = 1; ps->msr = 0xB0;
                    send_result(ps, "CONNECT 9600/TCP/IP");
                }
            }
        }
    }
}

/* ================================================================
 * MAIN
 * ================================================================ */

int main(int argc, char *argv[])
{
    VM_STATE v;
    PVM_SHARED pShared;
    PVOID pvBase = NULL;
    APIRET rc;
    int portIndex = 0;
    char shmName[64];

    printf("VMODEM/SIO2K v%s — Virtual Modem over TCP/IP\n", VERSION);
    printf("GPLv3 — FPC264IRC Contributors, 2026\n\n");

    if (argc > 1) portIndex = atoi(argv[1]);

    /* Open shared memory created by VMODEM.SYS */
    snprintf(shmName, sizeof(shmName), "\\SHAREMEM\\SIO2K\\VMODEM%d", portIndex);
    rc = DosGetNamedSharedMem(&pvBase, shmName, PAG_READ | PAG_WRITE);
    if (rc != 0) {
        /* VMODEM.SYS not loaded — create shared mem ourselves for standalone mode */
        rc = DosAllocSharedMem(&pvBase, shmName, sizeof(VM_SHARED),
                               PAG_COMMIT | PAG_READ | PAG_WRITE);
        if (rc != 0) {
            fprintf(stderr, "ERROR: Cannot allocate shared memory (%lu)\n", rc);
            return 1;
        }
        printf("Running in standalone mode (no VMODEM.SYS)\n");
    }

    pShared = (PVM_SHARED)pvBase;

    /* Initialize state */
    memset(&v, 0, sizeof(v));
    v.pShared = pShared;
    v.state = VM_COMMAND;
    v.echo = 1;
    v.sock = -1;
    v.listenSock = -1;
    v.outPort = 23;
    v.handshake = 3;    /* Auto-detect */
    v.dcdMode = 1;      /* &C1: DCD follows carrier */
    v.dsrMode = 1;      /* &S1: DSR normal */
    v.postEcho = 1;     /* F1: full duplex (no post-connect echo) */
    v.dialTimeout = 30; /* S7: 30 second dial timeout */
    v.inactivityTimer = 0;  /* S19: disabled */
    v.dtrDelay = 0;     /* S38: no delay */
    v.inactivityTick = 0;
    strncpy(v.loginName, "guest", 63);
    strncpy(v.termType, "ansi", 31);

    if (argc > 2) strncpy(v.loginName, argv[2], 63);

    /* Initialize shared memory if we created it */
    if (pShared->magic != VM_SHM_MAGIC) {
        pShared->magic = VM_SHM_MAGIC;
        pShared->version = 1;
        pShared->portIndex = (unsigned short)portIndex;
        pShared->msr = 0; pShared->lsr = 0x60;
        pShared->connected = 0; pShared->events = 0;
        pShared->txbuf.head = pShared->txbuf.tail = 0;
        pShared->rxbuf.head = pShared->rxbuf.tail = 0;
    }

    printf("Port %d active. Waiting for AT commands...\n", portIndex);

    /* Main loop */
    while (1) {
        io_loop(&v);
        DosSleep(1);    /* Yield — 1ms minimum granularity */
    }

    /* Cleanup */
    if (v.sock >= 0) close(v.sock);
    if (v.listenSock >= 0) close(v.listenSock);
    DosFreeMem(pvBase);

    return 0;
}
