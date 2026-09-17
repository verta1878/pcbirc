/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* pcbic.c -- PCBoard Internet Component (Phase 27 Reproduction)            */
/*                                                                           */
/* Exact reproduction of Clark's PCBIC v1.2 (April 30, 1997).                */
/*   Pcbic.exe     313K   Main IC program                                   */
/*   Pcbic2.exe    217K   IC v2                                             */
/*   PCBICCFG.EXE  185K   IC configurator                                  */
/*   PCBICEVT.EXE   90K   IC event scheduler                               */
/*   TESTIC.EXE     40K   IC test                                           */
/*   TESTIC2.EXE    47K   IC test 2                                         */
/*   RUNINET.PPE     2K   PPE launcher (source: RUNINET.PPS)                */
/*   PCBIC.DOC    112K   Documentation (text)                               */
/*   PCBIC.PDF    339K   Documentation (PDF)                                */
/*                                                                           */
/* Services: FTP, Gopher, Finger, Ping, Telnet, RLOGIN, PPP/SLIP, WHO       */
/*                                                                           */
/* This is the ancestor of our pcbis (Phase 6). Behavior must match          */
/* exactly before we extend it.                                              */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef __WATCOMC__
#include <conio.h>
#define strcasecmp stricmp
#endif


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       PCBIC Architecture                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* PCBIC provides internet services for PCBoard callers:
 *
 * FTP CLIENT:
 *   - Connect to FTP servers from within PCBoard
 *   - Upload/download files
 *   - Directory listing
 *   - MGET (wildcard gets, added in 15.4)
 *   - Resume support
 *
 * GOPHER CLIENT:
 *   - Browse Gopher servers
 *   - Navigate menu hierarchy
 *   - Download files via Gopher
 *
 * FINGER CLIENT:
 *   - Query user info from remote systems
 *
 * PING:
 *   - ICMP echo to test connectivity
 *
 * TELNET:
 *   - Outbound telnet connections
 *   - Terminal emulation (ANSI/VT100)
 *   - Pass-through to PCBoard caller
 *
 * RLOGIN:
 *   - Remote login to Unix systems
 *
 * PPP/SLIP:
 *   - Serial line IP for dial-up internet
 *
 * WHO:
 *   - Display online users
 *
 * PCBIC uses PCBIC.CFG for configuration:
 *   - DNS server addresses
 *   - Default FTP/Gopher/Telnet servers
 *   - Timeout values
 *   - Logging options
 *   - Security levels per service
 *
 * RUNINET.PPE is the PCBoard PPE that launches PCBIC services.
 * We have RUNINET.PPS (source) — this shows the interface between
 * PCBoard's PPL and PCBIC's services.
 */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      PCBIC Configuration                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    char     DnsServer[64];             /* primary DNS server            */
    char     DnsServer2[64];            /* secondary DNS server          */
    char     DefaultFtp[128];           /* default FTP server            */
    char     DefaultGopher[128];        /* default Gopher server         */
    char     DefaultTelnet[128];        /* default Telnet server         */
    int      FtpTimeout;                /* FTP timeout (seconds)         */
    int      TelnetTimeout;             /* Telnet timeout (seconds)      */
    int      MinSecurity;               /* minimum security level        */
    int      LogLevel;                  /* logging verbosity             */
    char     LogFile[260];              /* log file path                 */
} PcbicConfig;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                        FTP Client Stub                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static int ftp_connect(const char *Host, int Port)
{
    printf("Connecting to %s:%d...\n", Host, Port);
    /* TODO: Exact reproduction from disassembly */
    return 0;
}

static int ftp_login(const char *User, const char *Pass)
{
    printf("Login: %s\n", User);
    (void)Pass;
    /* TODO: Exact reproduction from disassembly */
    return 0;
}

static int ftp_list(const char *Path)
{
    printf("LIST %s\n", Path ? Path : ".");
    /* TODO: Exact reproduction from disassembly */
    return 0;
}

static int ftp_get(const char *Remote, const char *Local)
{
    printf("GET %s -> %s\n", Remote, Local);
    /* TODO: Exact reproduction from disassembly */
    return 0;
}

static int ftp_mget(const char *Pattern)
{
    printf("MGET %s\n", Pattern);
    /* TODO: Exact reproduction from disassembly (15.4 addition) */
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Telnet Client Stub                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static int telnet_connect(const char *Host, int Port)
{
    printf("Telnet to %s:%d...\n", Host, Port);
    /* TODO: Exact reproduction from disassembly */
    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Gopher Client Stub                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static int gopher_connect(const char *Host, int Port)
{
    printf("Gopher to %s:%d...\n", Host, Port);
    /* TODO: Exact reproduction from disassembly */
    return 0;
}


int main(int Argc, char *Argv[])
{
    (void)Argc; (void)Argv;
    (void)ftp_connect; (void)ftp_login; (void)ftp_list;
    (void)ftp_get; (void)ftp_mget;
    (void)telnet_connect; (void)gopher_connect;

    printf("PCBoard Internet Component v1.2\n");
    printf("TODO: Exact reproduction from disassembly\n");
    printf("\nServices:\n");
    printf("  FTP      — File Transfer Protocol client\n");
    printf("  Gopher   — Gopher menu browser\n");
    printf("  Finger   — User info query\n");
    printf("  Ping     — ICMP echo test\n");
    printf("  Telnet   — Terminal session\n");
    printf("  RLOGIN   — Remote login\n");
    printf("  PPP/SLIP — Serial line IP\n");
    printf("  WHO      — Online users\n");
    return 0;
}
