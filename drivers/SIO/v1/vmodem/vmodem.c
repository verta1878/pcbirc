/* ====================================================================
 * VMODEM.EXE — Virtual Modem for OS/2
 * ====================================================================
 * Clean-room reimplementation from VMODEM.TXT (Aug 1996).
 *
 * Turns any SIO COM port configured as "INTERNET" into a virtual modem.
 * Supports both outbound (client) and inbound (server) connections.
 *
 * Protocols:
 *   - Telnet (RFC 854) on port 23 (configurable via SERVICES "VMOTelnet")
 *   - VMP (Virtual Modem Protocol) on port 3141 (IANA assigned)
 *
 * Security:
 *   - MD5 Shared Secret authentication (RFC 1321)
 * ====================================================================
 */

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSSEMAPHORES
#include <os2.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdarg.h>

/* OS/2 TCP/IP socket API */
#include <types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <nerrno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <utils.h>

#include "vmodem.h"
#include "md5.h"


/* -------------------------------------------------------------------- */
/* Constants                                                            */
/* -------------------------------------------------------------------- */

#define VMODEM_VERSION      "1.00"
#define VMP_PORT_DEFAULT    3141        /* IANA assigned for VMP       */
#define TELNET_PORT_DEFAULT 23          /* Standard Telnet port        */
#define MAX_VMODEM_PORTS    16
#define AT_BUF_SIZE         256
#define DATA_BUF_SIZE       4096
#define CONNECT_TIMEOUT_DEF 60          /* S7 register default (sec)   */
#define INACTIVITY_DEF      0           /* S19 register default (min)  */


/* -------------------------------------------------------------------- */
/* Telnet Protocol Constants (RFC 854)                                  */
/* -------------------------------------------------------------------- */

#define TEL_IAC     255     /* Interpret As Command                     */
#define TEL_DONT    254
#define TEL_DO      253
#define TEL_WONT    252
#define TEL_WILL    251
#define TEL_SB      250     /* Subnegotiation Begin                    */
#define TEL_SE      240     /* Subnegotiation End                      */
#define TEL_ECHO    1       /* Echo option                             */
#define TEL_SGA     3       /* Suppress Go Ahead                       */
#define TEL_TTYPE   24      /* Terminal Type option                    */
#define TEL_NAWS    31      /* Negotiate About Window Size             */
#define TEL_BIN     0       /* Binary Transmission                     */


/* -------------------------------------------------------------------- */
/* Per-Port Virtual Modem State                                         */
/* -------------------------------------------------------------------- */

typedef enum {
    VM_COMMAND,             /* In AT command mode                       */
    VM_DATA,                /* Connected, passing data                  */
    VM_DIALING              /* Dial in progress                        */
} VMSTATE;

typedef struct _VMPORT {
    int         portNum;        /* COM port number (1-based)           */
    BOOL        active;         /* Port is configured as INTERNET      */
    VMSTATE     state;          /* Current state                       */
    int         sock;           /* Connected socket (-1 if none)       */
    BOOL        isVMP;          /* TRUE if VMP, FALSE if Telnet        */
    HFILE       hCom;           /* SIO COM port handle                 */

    /* AT command buffer */
    char        atBuf[AT_BUF_SIZE];
    int         atPos;

    /* ATDL: redial. Original SIO's VMODEM.EXE supports "ATDL —
     * Repeat the Last connect attempt", which had no equivalent
     * here — CmdDial never recorded what it last dialed. */
    char        lastDialAddr[256];
    BOOL        lastDialIsVMP;
    BOOL        haveLastDial;

    /* Modem settings */
    BOOL        echo;           /* E0/E1: command echo                 */
    BOOL        localEcho;      /* F0/F1: post-connect echo            */
    int         dcdMode;        /* &C0=always on, &C1=normal           */
    int         dsrMode;        /* &S0=always on, &S1=normal           */
    BOOL        autoAnswer;     /* S0: auto-answer enable              */
    BOOL        csUploadFix;    /* C0/C1: CompuServe kludge            */
    char        termType[40];   /* &T: terminal type string            */

    /* S registers */
    int         sRegs[128];

    /* Semaphores */
    HEV         hSem16;         /* \SEM\VMODEMn                        */
    HEV         hSem32;         /* \SEM32\VMODEMn                      */

    /* Telnet state machine */
    int         telState;       /* 0=normal, 1=got IAC, 2=got cmd      */
    BYTE        telCmd;         /* Pending telnet command               */
} VMPORT, *PVMPORT;


/* -------------------------------------------------------------------- */
/* Globals                                                              */
/* -------------------------------------------------------------------- */

static VMPORT   g_ports[MAX_VMODEM_PORTS];
static int      g_numPorts = 0;
static int      g_vmpPort  = VMP_PORT_DEFAULT;
static int      g_telPort  = TELNET_PORT_DEFAULT;
static BOOL     g_running  = TRUE;
static FILE    *g_logFile  = NULL;


/* -------------------------------------------------------------------- */
/* Forward Declarations                                                 */
/* -------------------------------------------------------------------- */

static void     InitPort(PVMPORT pv, int portNum);
static void     LogMsg(const char *fmt, ...);
static void     SendResult(PVMPORT pv, const char *result);
static void     SendToCom(PVMPORT pv, const char *str);
static int      ReadFromCom(PVMPORT pv, char *buf, int maxlen);
static void     ProcessATCommand(PVMPORT pv, const char *cmd);
static void     CmdDial(PVMPORT pv, const char *addr, BOOL isVMP);
static void     CmdDialVMP(PVMPORT pv, const char *args);
static void     CmdHangup(PVMPORT pv);
static void     CmdReset(PVMPORT pv);
static void     CmdSetSReg(PVMPORT pv, const char *args);
static void     CmdQuerySReg(PVMPORT pv, const char *args);
static void     CmdViewSettings(PVMPORT pv);
static void     CmdHelp(PVMPORT pv);
static void     CmdDialExamples(PVMPORT pv);
static void     HandleDataMode(PVMPORT pv);
static void     HandleCommandMode(PVMPORT pv);
static void     AcceptIncoming(int listenSock, BOOL isVMP);
static BYTE     TelnetFilter(PVMPORT pv, BYTE b);
static void     TelnetNegotiate(PVMPORT pv);
static void     SetDCD(PVMPORT pv, BOOL on);
static void     SetDSR(PVMPORT pv, BOOL on);
static void     SetRI(PVMPORT pv, BOOL on);
static void     CreateSemaphores(PVMPORT pv);
static void     SetSemaphores(PVMPORT pv);
static void     ClearSemaphores(PVMPORT pv);
static int      ResolveAddress(const char *addr, struct sockaddr_in *sa, int port);
static void     MD5SharedSecret(PVMPORT pv, const char *challenge, const char *secret, char *response);


/* ====================================================================
 * Main Entry Point
 * ==================================================================== */

int main(int argc, char *argv[])
{
    int         vmpSock = -1;
    int         telSock = -1;
    int         i;

    printf("VMODEM v%s — Virtual Modem for OS/2\n", VMODEM_VERSION);
    printf("Clean-room reimplementation\n\n");

    /* Optional log file */
    if (argc > 1) {
        g_logFile = fopen(argv[1], "w");
        if (g_logFile)
            LogMsg("Log file opened: %s", argv[1]);
    }

    /* Read VMP and Telnet port numbers from SERVICES file */
    {
        struct servent *se;
        se = getservbyname("vmodem", "tcp");
        if (se) g_vmpPort = ntohs(se->s_port);

        se = getservbyname("VMOTelnet", "tcp");
        if (se) g_telPort = ntohs(se->s_port);
    }

    /* Scan COM ports — try to open each one and check if it's available.
     * INTERNET (VMODEM) ports are configured in SIO's CONFIG.SYS line.
     * We try COM1-COM8 and activate any that open successfully. */
    g_numPorts = 0;
    for (i = 0; i < 8 && g_numPorts < MAX_VMODEM_PORTS; i++) {
        HFILE   hf;
        ULONG   action;
        APIRET  rc;
        char    comName[16];

        InitPort(&g_ports[g_numPorts], i + 1);

        sprintf(comName, "COM%d", i + 1);
        rc = DosOpen(comName, &hf, &action, 0, 0,
                     OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE, NULL);
        if (rc == 0) {
            /* Port opened successfully — activate it for VMODEM */
            g_ports[g_numPorts].hCom   = hf;
            g_ports[g_numPorts].active = TRUE;

            /* Set non-blocking read mode via DCB */
            {
                BYTE dcb[11];
                ULONG pl = 0, dl = 11;
                if (DosDevIOCtl(hf, 0x01, 0x73, NULL, 0, &pl,
                                dcb, 11, &dl) == 0) {
                    dcb[6] = (dcb[6] & 0x01) | 0x06 | 0xD0;
                    /* Flags3: no-wait read + forced SIO bits */
                    pl = 11; dl = 0;
                    DosDevIOCtl(hf, 0x01, 0x53, dcb, 11, &pl,
                                NULL, 0, &dl);
                }
            }

            LogMsg("Activated COM%d for VMODEM", i + 1);
            g_numPorts++;
        }
        /* If DosOpen fails, port doesn't exist or is in use — skip */
    }

    if (g_numPorts == 0) {
        printf("Warning: No COM ports available for VMODEM.\n");
        printf("Configure INTERNET ports in SIO CONFIG.SYS.\n");
    }

    /* Create VMP listener socket */
    vmpSock = socket(AF_INET, SOCK_STREAM, 0);
    if (vmpSock >= 0) {
        struct sockaddr_in sa;
        int optval = 1;

        setsockopt(vmpSock, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&optval, sizeof(optval));

        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port        = htons((unsigned short)g_vmpPort);

        if (bind(vmpSock, (struct sockaddr *)&sa, sizeof(sa)) == 0) {
            listen(vmpSock, 5);
            LogMsg("VMP listener on port %d", g_vmpPort);
        } else {
            LogMsg("ERROR: Cannot bind VMP port %d", g_vmpPort);
            soclose(vmpSock);
            vmpSock = -1;
        }
    }

    /* Create Telnet listener socket */
    telSock = socket(AF_INET, SOCK_STREAM, 0);
    if (telSock >= 0) {
        struct sockaddr_in sa;
        int optval = 1;

        setsockopt(telSock, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&optval, sizeof(optval));

        memset(&sa, 0, sizeof(sa));
        sa.sin_family      = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port        = htons((unsigned short)g_telPort);

        if (bind(telSock, (struct sockaddr *)&sa, sizeof(sa)) == 0) {
            listen(telSock, 5);
            LogMsg("Telnet listener on port %d", g_telPort);
        } else {
            LogMsg("ERROR: Cannot bind Telnet port %d", g_telPort);
            soclose(telSock);
            telSock = -1;
        }
    }

    /* ================================================================
     * Main Loop
     * ================================================================
     * Poll COM ports for AT commands (command mode) or relay data
     * (data mode). Accept incoming connections on VMP/Telnet ports.
     * ================================================================ */

    LogMsg("VMODEM running. Press Ctrl-C to terminate.");

    while (g_running) {
        fd_set  readfds;
        struct  timeval tv;
        int     maxfd = 0;

        FD_ZERO(&readfds);

        /* Add listener sockets */
        if (vmpSock >= 0) {
            FD_SET(vmpSock, &readfds);
            if (vmpSock > maxfd) maxfd = vmpSock;
        }
        if (telSock >= 0) {
            FD_SET(telSock, &readfds);
            if (telSock > maxfd) maxfd = telSock;
        }

        /* Add connected data sockets */
        for (i = 0; i < g_numPorts; i++) {
            if (g_ports[i].active && g_ports[i].state == VM_DATA &&
                g_ports[i].sock >= 0) {
                FD_SET(g_ports[i].sock, &readfds);
                if (g_ports[i].sock > maxfd)
                    maxfd = g_ports[i].sock;
            }
        }

        /* Short timeout so we can poll COM ports */
        tv.tv_sec  = 0;
        tv.tv_usec = 10000;    /* 10ms */

        select(maxfd + 1, &readfds, NULL, NULL, &tv);

        /* Check for incoming connections */
        if (vmpSock >= 0 && FD_ISSET(vmpSock, &readfds))
            AcceptIncoming(vmpSock, TRUE);
        if (telSock >= 0 && FD_ISSET(telSock, &readfds))
            AcceptIncoming(telSock, FALSE);

        /* Process each active port */
        for (i = 0; i < g_numPorts; i++) {
            PVMPORT pv = &g_ports[i];
            if (!pv->active) continue;

            switch (pv->state) {
            case VM_COMMAND:
                HandleCommandMode(pv);
                break;

            case VM_DATA:
                /* Check for network data → COM */
                if (pv->sock >= 0 && FD_ISSET(pv->sock, &readfds)) {
                    HandleDataMode(pv);
                }
                /* Check for COM data → network */
                {
                    char buf[DATA_BUF_SIZE];
                    int n = ReadFromCom(pv, buf, sizeof(buf));
                    if (n > 0 && pv->sock >= 0) {
                        send(pv->sock, buf, n, 0);
                    }
                }
                break;

            case VM_DIALING:
                /* Handled synchronously in CmdDial */
                break;
            }
        }
    }

    /* Cleanup */
    if (vmpSock >= 0) soclose(vmpSock);
    if (telSock >= 0) soclose(telSock);
    for (i = 0; i < g_numPorts; i++) {
        if (g_ports[i].sock >= 0) soclose(g_ports[i].sock);
    }
    if (g_logFile) fclose(g_logFile);

    return 0;
}


/* ====================================================================
 * Port Initialization
 * ==================================================================== */

static void InitPort(PVMPORT pv, int portNum)
{
    memset(pv, 0, sizeof(VMPORT));
    pv->portNum    = portNum;
    pv->active     = FALSE;
    pv->state      = VM_COMMAND;
    pv->sock       = -1;
    pv->hCom       = (HFILE)-1;
    pv->echo       = TRUE;         /* E1 default */
    pv->localEcho  = FALSE;        /* F1 default (disabled) */
    pv->dcdMode    = 1;            /* &C1 default (normal) */
    pv->dsrMode    = 0;            /* &S0 default (always on) */
    pv->autoAnswer = FALSE;        /* S0=0 */
    pv->csUploadFix = FALSE;       /* C0 */
    strcpy(pv->termType, "ANSI");

    /* S register defaults */
    pv->sRegs[0]  = 0;             /* Auto-answer rings (0=disabled) */
    pv->sRegs[7]  = CONNECT_TIMEOUT_DEF;  /* Carrier wait (seconds) */
    pv->sRegs[19] = INACTIVITY_DEF;       /* Inactivity timeout (min) */
    pv->sRegs[38] = 0;             /* DTR-to-disconnect delay */

    CreateSemaphores(pv);
    SetSemaphores(pv);              /* Set = disconnected */
}


/* ====================================================================
 * AT Command Mode Processing
 * ==================================================================== */

static void HandleCommandMode(PVMPORT pv)
{
    char buf[64];
    int  n, i;

    n = ReadFromCom(pv, buf, sizeof(buf));
    if (n <= 0) return;

    for (i = 0; i < n; i++) {
        char c = buf[i];

        /* Echo if enabled */
        if (pv->echo) {
            char ec[2];
            ec[0] = c; ec[1] = 0;
            SendToCom(pv, ec);
        }

        /* CR = execute command */
        if (c == '\r') {
            pv->atBuf[pv->atPos] = '\0';
            if (pv->atPos > 0) {
                ProcessATCommand(pv, pv->atBuf);
            }
            pv->atPos = 0;
            continue;
        }

        /* Backspace */
        if (c == '\b' || c == 127) {
            if (pv->atPos > 0) pv->atPos--;
            continue;
        }

        /* Ignore LF */
        if (c == '\n') continue;

        /* Buffer the character */
        if (pv->atPos < AT_BUF_SIZE - 1) {
            pv->atBuf[pv->atPos++] = c;
        }
    }
}


/* ====================================================================
 * AT Command Parser
 * ==================================================================== */

static void ProcessATCommand(PVMPORT pv, const char *cmd)
{
    /* Skip leading whitespace */
    while (*cmd == ' ') cmd++;

    /* Must start with "AT" (case insensitive) */
    if (toupper(cmd[0]) != 'A' || toupper(cmd[1]) != 'T') {
        SendResult(pv, "ERROR");
        return;
    }
    cmd += 2;

    /* Empty "AT" = OK */
    if (*cmd == '\0') {
        SendResult(pv, "OK");
        return;
    }

    /* Parse commands */
    while (*cmd) {
        char c = toupper(*cmd++);

        switch (c) {
        case ' ':
            continue;

        case '?':
            CmdHelp(pv);
            return;

        case 'C':
            /* Cn — CompuServe upload kludge */
            pv->csUploadFix = (*cmd == '1');
            if (*cmd >= '0' && *cmd <= '1') cmd++;
            break;

        case 'D':
            /* Dial commands */
            {
                char dc = toupper(*cmd);
                if (dc == '?' || dc == '$') {
                    /* ATD$ is a plain alias for ATD? in the original
                     * SIO VMODEM.EXE (identical "Show this display"
                     * description for both) — there was no handling
                     * for '$' here at all before. */
                    CmdDialExamples(pv);
                    return;
                }
                if (dc == 'L') {
                    /* ATDL — redial whatever CmdDial last recorded,
                     * matching the original's "Repeat the Last
                     * connect attempt". Previously unimplemented:
                     * nothing tracked a "last dial" at all. */
                    if (!pv->haveLastDial) {
                        SendResult(pv, "ERROR");
                        return;
                    }
                    CmdDial(pv, pv->lastDialAddr, pv->lastDialIsVMP);
                    return;
                }
                if (dc == 'T' || dc == 'P') {
                    cmd++;
                    while (*cmd == ' ') cmd++;
                    /* Check for # prefix = VMP */
                    if (*cmd == '#') {
                        CmdDial(pv, cmd + 1, TRUE);
                    } else {
                        CmdDial(pv, cmd, FALSE);
                    }
                    return;
                }
                if (dc == 'V') {
                    cmd++;
                    CmdDialVMP(pv, cmd);
                    return;
                }
            }
            SendResult(pv, "ERROR");
            return;

        case 'E':
            /* En — Echo */
            pv->echo = (*cmd != '0');
            if (*cmd >= '0' && *cmd <= '1') cmd++;
            break;

        case 'F':
            /* Fn — Local echo after connect (currently disabled per spec) */
            if (*cmd >= '0' && *cmd <= '1') cmd++;
            break;

        case 'H':
            /* Hn — Hook control */
            if (*cmd == '1') {
                cmd++;
                /* Off hook — no action for virtual modem */
            } else {
                if (*cmd == '0') cmd++;
                CmdHangup(pv);
            }
            break;

        case 'M':
            /* Mn — Incoming connect notify */
            if (*cmd >= '0' && *cmd <= '1') cmd++;
            /* Not meaningful for virtual modem */
            break;

        case 'S':
            /* S register operations */
            if (*cmd == '?') {
                cmd++;
                /* List S register info */
                SendToCom(pv, "S0  Auto-answer (0=disabled)\r\n");
                SendToCom(pv, "S7  Carrier wait (seconds)\r\n");
                SendToCom(pv, "S19 Inactivity timeout (minutes)\r\n");
                SendToCom(pv, "S38 DTR disconnect delay\r\n");
            } else if (isdigit(*cmd)) {
                /* Sr=n or Sr=? */
                int reg = atoi(cmd);
                while (isdigit(*cmd)) cmd++;
                if (*cmd == '=') {
                    cmd++;
                    if (*cmd == '?') {
                        char regStr2[8];
                        sprintf(regStr2, "%d", reg);
                        CmdQuerySReg(pv, regStr2);
                        cmd++;
                    } else {
                        /* Set register */
                        int val = atoi(cmd);
                        while (isdigit(*cmd)) cmd++;
                        if (reg >= 0 && reg < 128)
                            pv->sRegs[reg] = val;
                        if (reg == 0)
                            pv->autoAnswer = (val != 0);
                    }
                }
            }
            break;

        case 'Z':
            CmdReset(pv);
            return;

        case '&':
            /* Extended commands */
            c = toupper(*cmd++);
            switch (c) {
            case 'C':
                pv->dcdMode = (*cmd == '0') ? 0 : 1;
                if (*cmd >= '0' && *cmd <= '1') cmd++;
                break;
            case 'S':
                pv->dsrMode = (*cmd == '1') ? 1 : 0;
                if (*cmd >= '0' && *cmd <= '1') cmd++;
                break;
            case 'T':
                /* &T"termtype" */
                if (*cmd == '"') {
                    cmd++;
                    {
                        int j = 0;
                        while (*cmd && *cmd != '"' && j < 39)
                            pv->termType[j++] = *cmd++;
                        pv->termType[j] = '\0';
                        if (*cmd == '"') cmd++;
                    }
                }
                break;
            case 'V':
                CmdViewSettings(pv);
                return;
            default:
                break;
            }
            break;

        default:
            break;
        }
    }

    SendResult(pv, "OK");
}


/* ====================================================================
 * Dial Command
 * ==================================================================== */

static void CmdDial(PVMPORT pv, const char *addr, BOOL isVMP)
{
    struct sockaddr_in sa;
    char   addrBuf[256];
    char   secret[128];
    char  *p;
    int    port;
    int    timeout;
    int    sock;

    /* Record for ATDL (redial), unless this call *is* the redial —
     * re-recording pv->lastDialAddr from itself is harmless (it's
     * the same buffer) but pointless; skipped for clarity. */
    if (addr != pv->lastDialAddr) {
        strncpy(pv->lastDialAddr, addr, sizeof(pv->lastDialAddr) - 1);
        pv->lastDialAddr[sizeof(pv->lastDialAddr) - 1] = '\0';
        pv->lastDialIsVMP = isVMP;
        pv->haveLastDial = TRUE;
    }

    /* Copy address, translate * → . (per VMODEM.TXT) */
    strncpy(addrBuf, addr, sizeof(addrBuf) - 1);
    addrBuf[sizeof(addrBuf) - 1] = '\0';
    for (p = addrBuf; *p; p++) {
        if (*p == '*') *p = '.';
    }

    /* Strip quotes if present */
    if (addrBuf[0] == '"') {
        memmove(addrBuf, addrBuf + 1, strlen(addrBuf));
        p = strchr(addrBuf, '"');
        if (p) *p = '\0';
    }

    /* Extract shared secret if present — look for "secret" after address
     * in the ORIGINAL addr string, not the modified addrBuf */
    secret[0] = '\0';
    {
        const char *sq;
        const char *eq;
        sq = strchr(addr, '"');
        if (sq) {
            sq++;  /* Skip opening quote */
            eq = strchr(sq, '"');
            if (eq) {
                int slen = (int)(eq - sq);
                if (slen > (int)sizeof(secret) - 1) slen = sizeof(secret) - 1;
                strncpy(secret, sq, slen);
                secret[slen] = '\0';
            }
        }
    }

    /* Extract optional port override */
    port = isVMP ? g_vmpPort : g_telPort;
    p = strrchr(addrBuf, ' ');
    if (p) {
        *p++ = '\0';
        while (*p == ' ') p++;
        if (isdigit(*p))
            port = atoi(p);
    }

    /* Trim trailing whitespace from address */
    p = addrBuf + strlen(addrBuf) - 1;
    while (p >= addrBuf && (*p == ' ' || *p == '\r' || *p == '\n'))
        *p-- = '\0';

    LogMsg("Dialing %s:%d (%s)", addrBuf, port, isVMP ? "VMP" : "Telnet");

    /* Resolve address */
    if (ResolveAddress(addrBuf, &sa, port) != 0) {
        SendResult(pv, "NO DIALTONE");
        return;
    }

    /* Create socket and connect */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        SendResult(pv, "NO DIALTONE");
        return;
    }

    pv->state = VM_DIALING;
    timeout = pv->sRegs[7];

    /* Set connect timeout via SO_SNDTIMEO */
    {
        struct timeval tv;
        tv.tv_sec  = timeout;
        tv.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv));
    }

    if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0) {
        soclose(sock);
        SendResult(pv, "NO CARRIER");
        pv->state = VM_COMMAND;
        return;
    }

    /* Connected */
    pv->sock  = sock;
    pv->isVMP = isVMP;
    pv->state = VM_DATA;

    /* Set DCD on */
    if (pv->dcdMode == 1)
        SetDCD(pv, TRUE);

    /* Clear semaphores (= connected) */
    ClearSemaphores(pv);

    /* Negotiate telnet options */
    if (!isVMP) {
        TelnetNegotiate(pv);
    }

    /* Send connect result */
    if (isVMP) {
        SendResult(pv, "CONNECT 57600/ARQ/VMP");
    } else {
        SendResult(pv, "CONNECT 57600/ARQ/TEL");
    }

    LogMsg("Connected to %s:%d", addrBuf, port);
}


/* ====================================================================
 * Dial Specific VMP Port
 * ==================================================================== */

static void CmdDialVMP(PVMPORT pv, const char *args)
{
    /* ATDV n address — dial VMP port n at address */
    while (*args == ' ') args++;
    /* Parse port number, then address */
    CmdDial(pv, args, TRUE);
}


/* ====================================================================
 * Hangup
 * ==================================================================== */

static void CmdHangup(PVMPORT pv)
{
    if (pv->sock >= 0) {
        soclose(pv->sock);
        pv->sock = -1;
    }
    pv->state = VM_COMMAND;

    if (pv->dcdMode == 1)
        SetDCD(pv, FALSE);

    SetSemaphores(pv);              /* Set = disconnected */

    SendResult(pv, "NO CARRIER");
    LogMsg("Port %d disconnected", pv->portNum);
}


/* ====================================================================
 * Reset (ATZ)
 * ==================================================================== */

static void CmdReset(PVMPORT pv)
{
    if (pv->sock >= 0) {
        soclose(pv->sock);
        pv->sock = -1;
    }
    pv->state       = VM_COMMAND;
    pv->echo        = TRUE;
    pv->dcdMode     = 1;
    pv->dsrMode     = 0;
    pv->csUploadFix = FALSE;
    strcpy(pv->termType, "ANSI");
    pv->sRegs[0]    = 0;
    pv->sRegs[7]    = CONNECT_TIMEOUT_DEF;
    pv->autoAnswer  = FALSE;

    SetSemaphores(pv);
    SendResult(pv, "OK");
}


/* ====================================================================
 * Data Mode — Relay data between network and COM port
 * ==================================================================== */

static void HandleDataMode(PVMPORT pv)
{
    char buf[DATA_BUF_SIZE];
    int  n;

    /* Network → COM */
    n = recv(pv->sock, buf, sizeof(buf), 0);
    if (n <= 0) {
        /* Connection closed */
        CmdHangup(pv);
        return;
    }

    if (pv->isVMP) {
        /* VMP: binary passthrough */
        /* TODO: Implement VMP framing */
        SendToCom(pv, "");
        /* Write raw bytes to COM */
        DosWrite(pv->hCom, buf, n, (PULONG)&n);
    } else {
        /* Telnet: filter IAC sequences */
        int i;
        for (i = 0; i < n; i++) {
            BYTE b = TelnetFilter(pv, (BYTE)buf[i]);
            if (b != 0) {
                char c = (char)b;
                DosWrite(pv->hCom, &c, 1, (PULONG)&n);
            }
        }
    }
}


/* ====================================================================
 * Incoming Connection Handler
 * ==================================================================== */

static void AcceptIncoming(int listenSock, BOOL isVMP)
{
    struct sockaddr_in sa;
    int     saLen = sizeof(sa);
    int     newSock;
    int     i;
    PVMPORT pv = NULL;

    newSock = accept(listenSock, (struct sockaddr *)&sa, &saLen);
    if (newSock < 0) return;

    /* Find a free INTERNET port */
    for (i = 0; i < g_numPorts; i++) {
        if (g_ports[i].active && g_ports[i].state == VM_COMMAND &&
            g_ports[i].sock < 0) {
            pv = &g_ports[i];
            break;
        }
    }

    if (!pv) {
        /* No available port — send BUSY and close */
        const char *busy = "BUSY\r\n";
        send(newSock, busy, strlen(busy), 0);
        soclose(newSock);
        LogMsg("Incoming connection rejected (BUSY)");
        return;
    }

    LogMsg("Incoming %s connection on port %d from %s",
           isVMP ? "VMP" : "Telnet", pv->portNum,
           inet_ntoa(sa.sin_addr));

    pv->sock  = newSock;
    pv->isVMP = isVMP;

    /* Send RING to COM port */
    SendResult(pv, "RING");
    ClearSemaphores(pv);

    /* If auto-answer enabled, connect immediately */
    if (pv->autoAnswer) {
        pv->state = VM_DATA;
        if (pv->dcdMode == 1) SetDCD(pv, TRUE);
        if (!isVMP) TelnetNegotiate(pv);
        SendResult(pv, isVMP ? "CONNECT 57600/ARQ/VMP" : "CONNECT 57600/ARQ/TEL");
    }
}


/* ====================================================================
 * Telnet IAC Filter
 * ==================================================================== */

static BYTE TelnetFilter(PVMPORT pv, BYTE b)
{
    switch (pv->telState) {
    case 0: /* Normal */
        if (b == TEL_IAC) {
            pv->telState = 1;
            return 0;
        }
        return b;

    case 1: /* Got IAC */
        if (b == TEL_IAC) {
            pv->telState = 0;
            return TEL_IAC;         /* Escaped IAC = literal 255 */
        }
        if (b == TEL_WILL || b == TEL_WONT || b == TEL_DO || b == TEL_DONT) {
            pv->telCmd   = b;
            pv->telState = 2;
            return 0;
        }
        if (b == TEL_SB) {
            pv->telState = 3;       /* Subnegotiation — eat until SE */
            return 0;
        }
        pv->telState = 0;
        return 0;

    case 2: /* Got WILL/WONT/DO/DONT, next byte is option */
        {
            BYTE resp[3];
            resp[0] = TEL_IAC;

            /* Refuse everything except ECHO, SGA, BIN, TTYPE */
            if (b == TEL_ECHO || b == TEL_SGA || b == TEL_BIN) {
                resp[1] = (pv->telCmd == TEL_DO) ? TEL_WILL : TEL_DO;
            } else if (b == TEL_TTYPE) {
                resp[1] = (pv->telCmd == TEL_DO) ? TEL_WILL : TEL_DO;
            } else {
                resp[1] = (pv->telCmd == TEL_DO) ? TEL_WONT : TEL_DONT;
            }
            resp[2] = b;
            send(pv->sock, (char *)resp, 3, 0);
        }
        pv->telState = 0;
        return 0;

    case 3: /* In subnegotiation — collect until IAC SE */
        if (b == TEL_IAC) pv->telState = 4;
        /* Track subneg option for TTYPE response */
        return 0;

    case 4: /* Got IAC inside subnegotiation */
        if (b == TEL_SE) {
            pv->telState = 0;
        } else {
            pv->telState = 3;
        }
        return 0;
    }

    pv->telState = 0;
    return 0;
}


/* ====================================================================
 * Telnet Option Negotiation (initial)
 * ==================================================================== */

static void TelnetNegotiate(PVMPORT pv)
{
    BYTE neg[12];
    int  pos = 0;

    /* WILL ECHO */
    neg[pos++] = TEL_IAC; neg[pos++] = TEL_WILL; neg[pos++] = TEL_ECHO;
    /* WILL SGA */
    neg[pos++] = TEL_IAC; neg[pos++] = TEL_WILL; neg[pos++] = TEL_SGA;
    /* DO SGA */
    neg[pos++] = TEL_IAC; neg[pos++] = TEL_DO;   neg[pos++] = TEL_SGA;
    /* WILL BIN */
    neg[pos++] = TEL_IAC; neg[pos++] = TEL_WILL; neg[pos++] = TEL_BIN;

    send(pv->sock, (char *)neg, pos, 0);

    /* Prepare to respond to SB TTYPE SEND with our terminal type.
     * We send: IAC SB TTYPE IS <termtype> IAC SE */
    {
        BYTE ttype[50];
        int  tpos = 0;
        int  j;
        ttype[tpos++] = TEL_IAC;
        ttype[tpos++] = TEL_SB;
        ttype[tpos++] = TEL_TTYPE;
        ttype[tpos++] = 0;     /* IS */
        for (j = 0; pv->termType[j] && tpos < 45; j++)
            ttype[tpos++] = (BYTE)pv->termType[j];
        ttype[tpos++] = TEL_IAC;
        ttype[tpos++] = TEL_SE;
        /* Store for when server asks — send proactively */
        send(pv->sock, (char *)ttype, tpos, 0);
    }
}


/* ====================================================================
 * Helper Functions
 * ==================================================================== */

static void SendResult(PVMPORT pv, const char *result)
{
    char buf[80];
    sprintf(buf, "\r\n%s\r\n", result);
    SendToCom(pv, buf);
    LogMsg("Port %d: %s", pv->portNum, result);
}

static void SendToCom(PVMPORT pv, const char *str)
{
    ULONG written;
    if (pv->hCom != (HFILE)-1) {
        DosWrite(pv->hCom, (PVOID)str, strlen(str), &written);
    }
}

static int ReadFromCom(PVMPORT pv, char *buf, int maxlen)
{
    ULONG actual = 0;
    if (pv->hCom != (HFILE)-1) {
        DosRead(pv->hCom, buf, maxlen, &actual);
    }
    return (int)actual;
}

static void LogMsg(const char *fmt, ...)
{
    va_list ap;
    char    buf[256];
    time_t  t = time(NULL);
    struct  tm *tm = localtime(&t);

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    printf("[%02d:%02d:%02d] %s\n", tm->tm_hour, tm->tm_min, tm->tm_sec, buf);

    if (g_logFile) {
        fprintf(g_logFile, "[%02d:%02d:%02d] %s\n",
                tm->tm_hour, tm->tm_min, tm->tm_sec, buf);
        fflush(g_logFile);
    }
}

static int ResolveAddress(const char *addr, struct sockaddr_in *sa, int port)
{
    struct hostent *he;

    memset(sa, 0, sizeof(*sa));
    sa->sin_family = AF_INET;
    sa->sin_port   = htons((unsigned short)port);

    /* Try numeric first */
    sa->sin_addr.s_addr = inet_addr(addr);
    if (sa->sin_addr.s_addr != INADDR_NONE)
        return 0;

    /* DNS lookup */
    he = gethostbyname(addr);
    if (!he) return -1;

    memcpy(&sa->sin_addr, he->h_addr, he->h_length);
    return 0;
}

static void CmdHelp(PVMPORT pv)
{
    SendToCom(pv, "\r\nVMODEM AT Commands:\r\n");
    SendToCom(pv, "  AT?        This help\r\n");
    SendToCom(pv, "  ATCn       CompuServe upload fix\r\n");
    SendToCom(pv, "  ATD?       Dialing examples\r\n");
    SendToCom(pv, "  ATDL       Repeat the last connect attempt\r\n");
    SendToCom(pv, "  ATDT addr  Dial (Telnet)\r\n");
    SendToCom(pv, "  ATDT #addr Dial (VMP)\r\n");
    SendToCom(pv, "  ATEn       Echo (E0=off E1=on)\r\n");
    SendToCom(pv, "  ATH        Hang up\r\n");
    SendToCom(pv, "  ATSr=n     Set S register\r\n");
    SendToCom(pv, "  ATZ        Reset\r\n");
    SendToCom(pv, "  AT&Cn      DCD mode (&C0=on &C1=normal)\r\n");
    SendToCom(pv, "  AT&Sn      DSR mode (&S0=on &S1=normal)\r\n");
    SendToCom(pv, "  AT&T\"type\" Terminal type\r\n");
    SendToCom(pv, "  AT&V       View settings\r\n");
}

static void CmdDialExamples(PVMPORT pv)
{
    SendToCom(pv, "\r\nDialing Examples:\r\n");
    SendToCom(pv, "  ATDT bbs.example.com       Telnet\r\n");
    SendToCom(pv, "  ATDT #bbs.example.com      VMP\r\n");
    SendToCom(pv, "  ATDT 192.168.1.1           Numeric\r\n");
    SendToCom(pv, "  ATDT bbs.example.com 3141  Custom port\r\n");
    SendToCom(pv, "  ATDL                       Repeat the last connect attempt\r\n");
}

static void CmdQuerySReg(PVMPORT pv, const char *args)
{
    char buf[32];
    int  reg = atoi(args);
    if (reg >= 0 && reg < 128)
        sprintf(buf, "%d", pv->sRegs[reg]);
    else
        strcpy(buf, "0");
    SendResult(pv, buf);
}

static void CmdViewSettings(PVMPORT pv)
{
    char buf[80];
    SendToCom(pv, "\r\nVMODEM Settings:\r\n");
    sprintf(buf, "  Echo: %s\r\n", pv->echo ? "ON" : "OFF");
    SendToCom(pv, buf);
    sprintf(buf, "  DCD: %s\r\n", pv->dcdMode ? "Normal" : "Always ON");
    SendToCom(pv, buf);
    sprintf(buf, "  DSR: %s\r\n", pv->dsrMode ? "Normal" : "Always ON");
    SendToCom(pv, buf);
    sprintf(buf, "  Terminal: %s\r\n", pv->termType);
    SendToCom(pv, buf);
    sprintf(buf, "  Auto-answer: %s\r\n", pv->autoAnswer ? "ON" : "OFF");
    SendToCom(pv, buf);
    sprintf(buf, "  S7 (timeout): %d sec\r\n", pv->sRegs[7]);
    SendToCom(pv, buf);
}

/* Modem signal control via SIO IOCtl */
static void SetDCD(PVMPORT pv, BOOL on)  { (void)pv; (void)on; /* TODO: IOCtl 46h */ }
static void SetDSR(PVMPORT pv, BOOL on)  { (void)pv; (void)on; }
static void SetRI(PVMPORT pv, BOOL on)   { (void)pv; (void)on; }

/* Semaphore management */
static void CreateSemaphores(PVMPORT pv)
{
    char name[40];
    sprintf(name, "\\SEM32\\VMODEM%d", pv->portNum);
    DosCreateEventSem(name, &pv->hSem32, DC_SEM_SHARED, FALSE);
}

static void SetSemaphores(PVMPORT pv)
{
    if (pv->hSem32) DosPostEventSem(pv->hSem32);
}

static void ClearSemaphores(PVMPORT pv)
{
    ULONG count;
    if (pv->hSem32) DosResetEventSem(pv->hSem32, &count);
}
