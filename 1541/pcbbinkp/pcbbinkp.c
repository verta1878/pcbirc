/* ====================================================================
 * pcbbinkp.c — PCBoard Standalone BinkP Mailer
 * ====================================================================
 * Standalone EXE called by PCBoard (FIDOPOLL/ALT-F) or pcbis_ui.
 * Reads config from pcbis.cfg, connects to remote FTN nodes via
 * BinkP/1.1 (FTS-1026) with CRAM-MD5 auth (FSP-1024).
 *
 * Usage:
 *   pcbbinkp poll <addr>           Poll a specific node
 *   pcbbinkp answer <port>         Listen for incoming sessions
 *   pcbbinkp status                Show queue status
 *
 * All output is tagged [BINKP] for pcbis_ui scrollback filtering.
 *
 * Build: wcc386 -bt=os2 -5r -oxs pcbbinkp.c binkp.c binkpauth.c bso.c md5.c
 *        wlink @pcbbinkp.lnk
 *
 * Copyright (C) 2026 pcbrevival contributors
 * License: GPLv3
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __OS2__
 #define INCL_DOS
 #define INCL_DOSERRORS
 #include <os2.h>
 #include <types.h>
 #include <sys/socket.h>
 #include <sys/time.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <tcpustd.h>
 typedef int socklen_t;
 #define closesocket soclose
 #define SOCKET      int
 #define INVALID_SOCKET (-1)
#elif defined(__NT__) || defined(__WINDOWS__)
 #include <winsock2.h>
 #include <ws2tcpip.h>
 typedef int socklen_t;
#else
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #define SOCKET      int
 #define INVALID_SOCKET (-1)
 #define closesocket close
#endif

#include "binkp.h"

/* ====================================================================
 * Configuration
 * ==================================================================== */

#define MAX_NODES  64
#define CFG_FILE   "pcbis.cfg"

typedef struct {
    FtnAddr      addr;          /* remote FTN address                  */
    char         host[128];     /* hostname or IP                      */
    unsigned short port;        /* TCP port (default 24554)            */
    char         password[64];  /* session password                    */
} NodeConfig;

typedef struct {
    FtnAddr      local_addr;    /* our FTN address                     */
    char         outbound[260]; /* BSO outbound base                   */
    char         inbound[260];  /* secure inbound                      */
    char         temp_inbound[260]; /* temp inbound                    */
    unsigned short listen_port; /* listen port (default 24554)         */
    NodeConfig   nodes[MAX_NODES];
    int          node_count;
    int          verbose;       /* log verbosity                       */
} Config;

static Config g_cfg;

/* ====================================================================
 * Config file parser
 * ==================================================================== */

static void cfg_defaults(Config *c)
{
    memset(c, 0, sizeof(Config));
    c->listen_port = BINKP_DEFAULT_PORT;
    c->verbose = 1;
    strcpy(c->outbound, "outbound");
    strcpy(c->inbound, "inbound");
    strcpy(c->temp_inbound, "temp_in");
}

/* Parse pcbis.cfg:
 *
 * [binkp]
 * address 1:2320/105
 * outbound C:\PCB\OUTBOUND
 * inbound C:\PCB\INBOUND
 * temp_inbound C:\PCB\TEMP_IN
 * listen 24554
 * verbose 2
 *
 * [node]
 * address 1:2320/100
 * host bbs.example.com
 * port 24554
 * password SECRET
 */
static int cfg_load(const char *filename, Config *c)
{
    FILE *fp;
    char line[512];
    int in_binkp = 0, in_node = 0;
    NodeConfig *cur_node = NULL;

    cfg_defaults(c);

    fp = fopen(filename, "r");
    if (!fp) {
        binkp_log(0, "Cannot open config: %s", filename);
        return -1;
    }

    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        char *end;

        /* Trim */
        while (*p == ' ' || *p == '\t') p++;
        end = p + strlen(p) - 1;
        while (end >= p && (*end == '\r' || *end == '\n' || *end == ' '))
            *end-- = '\0';

        if (*p == '\0' || *p == '#' || *p == ';')
            continue;

        /* Section headers */
        if (stricmp(p, "[binkp]") == 0) {
            in_binkp = 1; in_node = 0;
            continue;
        }
        if (stricmp(p, "[node]") == 0) {
            in_binkp = 0; in_node = 1;
            if (c->node_count < MAX_NODES) {
                cur_node = &c->nodes[c->node_count];
                memset(cur_node, 0, sizeof(NodeConfig));
                cur_node->port = BINKP_DEFAULT_PORT;
                c->node_count++;
            } else {
                cur_node = NULL;
            }
            continue;
        }

        /* Key-value pairs */
        if (in_binkp) {
            if (strnicmp(p, "address ", 8) == 0)
                ftn_parse(p + 8, &c->local_addr);
            else if (strnicmp(p, "outbound ", 9) == 0)
                strncpy(c->outbound, p + 9, sizeof(c->outbound) - 1);
            else if (strnicmp(p, "inbound ", 8) == 0)
                strncpy(c->inbound, p + 8, sizeof(c->inbound) - 1);
            else if (strnicmp(p, "temp_inbound ", 13) == 0)
                strncpy(c->temp_inbound, p + 13, sizeof(c->temp_inbound) - 1);
            else if (strnicmp(p, "listen ", 7) == 0)
                c->listen_port = (unsigned short)atoi(p + 7);
            else if (strnicmp(p, "verbose ", 8) == 0)
                c->verbose = atoi(p + 8);
        }

        if (in_node && cur_node) {
            if (strnicmp(p, "address ", 8) == 0)
                ftn_parse(p + 8, &cur_node->addr);
            else if (strnicmp(p, "host ", 5) == 0)
                strncpy(cur_node->host, p + 5, sizeof(cur_node->host) - 1);
            else if (strnicmp(p, "port ", 5) == 0)
                cur_node->port = (unsigned short)atoi(p + 5);
            else if (strnicmp(p, "password ", 9) == 0)
                strncpy(cur_node->password, p + 9,
                        sizeof(cur_node->password) - 1);
        }
    }

    fclose(fp);

    {
        char buf[40];
        ftn_format(&c->local_addr, buf, sizeof(buf));
        binkp_log(1, "Config loaded: %s, %d node(s), verbose=%d",
                  buf, c->node_count, c->verbose);
    }

    return 0;
}

/* Find node config by FTN address */
static NodeConfig *cfg_find_node(Config *c, const FtnAddr *addr)
{
    int i;
    for (i = 0; i < c->node_count; i++) {
        if (ftn_match(&c->nodes[i].addr, addr))
            return &c->nodes[i];
    }
    return NULL;
}

/* ====================================================================
 * TCP Connect (outbound poll)
 * ==================================================================== */

static int tcp_connect(const char *host, unsigned short port)
{
    struct hostent *he;
    struct sockaddr_in sa;
    int sock;

    binkp_log(1, "Connecting to %s:%u ...", host, port);

    he = gethostbyname(host);
    if (!he) {
        binkp_log(0, "DNS lookup failed for %s", host);
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    memcpy(&sa.sin_addr, he->h_addr, he->h_length);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        binkp_log(0, "socket() failed");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        binkp_log(0, "connect() failed to %s:%u", host, port);
        closesocket(sock);
        return -1;
    }

    binkp_log(1, "Connected to %s:%u", host, port);
    return sock;
}

/* ====================================================================
 * TCP Listen (inbound answer)
 * ==================================================================== */

static int tcp_listen(unsigned short port)
{
    struct sockaddr_in sa;
    int lsock, csock;
    socklen_t salen;
    int opt = 1;

    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0) {
        binkp_log(0, "socket() failed");
        return -1;
    }

    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    memset(&sa, 0, sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_port        = htons(port);

    if (bind(lsock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        binkp_log(0, "bind() failed on port %u", port);
        closesocket(lsock);
        return -1;
    }

    if (listen(lsock, 1) < 0) {
        binkp_log(0, "listen() failed");
        closesocket(lsock);
        return -1;
    }

    binkp_log(1, "Listening on port %u for incoming BinkP ...", port);

    salen = sizeof(sa);
    csock = accept(lsock, (struct sockaddr *)&sa, &salen);
    closesocket(lsock);

    if (csock < 0) {
        binkp_log(0, "accept() failed");
        return -1;
    }

    binkp_log(1, "Incoming connection from %s",
              inet_ntoa(sa.sin_addr));

    return csock;
}

/* ====================================================================
 * Poll a node
 * ==================================================================== */

static int do_poll(Config *c, const char *addr_str)
{
    FtnAddr remote;
    NodeConfig *nc;
    BinkpSession sess;
    int sock, rc;
    char buf[40];

    if (ftn_parse(addr_str, &remote) < 0) {
        binkp_log(0, "Invalid address: %s", addr_str);
        return 1;
    }

    ftn_format(&remote, buf, sizeof(buf));
    nc = cfg_find_node(c, &remote);
    if (!nc) {
        binkp_log(0, "Node %s not found in config", buf);
        return 1;
    }

    if (strlen(nc->host) == 0) {
        binkp_log(0, "No host configured for %s", buf);
        return 1;
    }

    sock = tcp_connect(nc->host, nc->port);
    if (sock < 0)
        return 1;

    binkp_init(&sess, sock, BINKP_ORIGINATE);
    sess.local_addr  = c->local_addr;
    sess.remote_addr = remote;
    strncpy(sess.password, nc->password, sizeof(sess.password) - 1);
    strncpy(sess.outbound, c->outbound, sizeof(sess.outbound) - 1);
    strncpy(sess.inbound, c->inbound, sizeof(sess.inbound) - 1);
    strncpy(sess.temp_inbound, c->temp_inbound, sizeof(sess.temp_inbound) - 1);

    /* Scan outbound for files */
    bso_scan_outbound(&sess, &remote);

    /* Run session */
    rc = binkp_run(&sess);

    binkp_cleanup(&sess);
    closesocket(sock);

    return rc < 0 ? 1 : 0;
}

/* ====================================================================
 * Answer incoming session
 * ==================================================================== */

static int do_answer(Config *c, unsigned short port)
{
    BinkpSession sess;
    int sock, rc;

    sock = tcp_listen(port);
    if (sock < 0)
        return 1;

    binkp_init(&sess, sock, BINKP_ANSWER);
    sess.local_addr = c->local_addr;
    strncpy(sess.outbound, c->outbound, sizeof(sess.outbound) - 1);
    strncpy(sess.inbound, c->inbound, sizeof(sess.inbound) - 1);
    strncpy(sess.temp_inbound, c->temp_inbound, sizeof(sess.temp_inbound) - 1);

    /* Password will be looked up after we get M_ADR from remote */
    /* For now use empty — handle_adr should look up config */

    rc = binkp_run(&sess);

    binkp_cleanup(&sess);
    closesocket(sock);

    return rc < 0 ? 1 : 0;
}

/* ====================================================================
 * Show outbound queue status
 * ==================================================================== */

static int do_status(Config *c)
{
    int i;

    binkp_log(1, "=== Outbound Queue Status ===");

    for (i = 0; i < c->node_count; i++) {
        BinkpSession tmp;
        char buf[40];
        int count;

        memset(&tmp, 0, sizeof(tmp));
        strncpy(tmp.outbound, c->outbound, sizeof(tmp.outbound) - 1);

        ftn_format(&c->nodes[i].addr, buf, sizeof(buf));
        count = bso_scan_outbound(&tmp, &c->nodes[i].addr);

        printf("[BINKP] %-20s  %s:%u  %d file(s)  pw:%s\n",
               buf, c->nodes[i].host, c->nodes[i].port,
               count,
               strlen(c->nodes[i].password) ? "yes" : "no");
    }

    return 0;
}

/* ====================================================================
 * Main
 * ==================================================================== */

static void usage(void)
{
    printf("[BINKP] PCBoard BinkP Mailer v1.0\n");
    printf("[BINKP] Copyright (C) 2026 pcbrevival contributors (GPLv3)\n");
    printf("[BINKP]\n");
    printf("[BINKP] Usage:\n");
    printf("[BINKP]   pcbbinkp poll <zone:net/node[.point]>\n");
    printf("[BINKP]   pcbbinkp answer [port]\n");
    printf("[BINKP]   pcbbinkp status\n");
    printf("[BINKP]\n");
    printf("[BINKP] Config: %s\n", CFG_FILE);
}

int main(int argc, char *argv[])
{
    int rc = 0;

#if defined(__NT__) || defined(__WINDOWS__)
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    printf("[BINKP] pcbbinkp v1.0 starting\n");

    if (cfg_load(CFG_FILE, &g_cfg) < 0) {
        binkp_log(0, "Using defaults — no %s found", CFG_FILE);
        cfg_defaults(&g_cfg);
    }

    /* Set global verbose from config (g_verbose defined in binkp.c) */
    {
        extern int g_verbose;
        g_verbose = g_cfg.verbose;
    }

    if (argc < 2) {
        usage();
        rc = 1;
    } else if (stricmp(argv[1], "poll") == 0) {
        if (argc < 3) {
            binkp_log(0, "poll requires an address");
            rc = 1;
        } else {
            rc = do_poll(&g_cfg, argv[2]);
        }
    } else if (stricmp(argv[1], "answer") == 0) {
        unsigned short port = g_cfg.listen_port;
        if (argc >= 3)
            port = (unsigned short)atoi(argv[2]);
        rc = do_answer(&g_cfg, port);
    } else if (stricmp(argv[1], "status") == 0) {
        rc = do_status(&g_cfg);
    } else {
        binkp_log(0, "Unknown command: %s", argv[1]);
        usage();
        rc = 1;
    }

#if defined(__NT__) || defined(__WINDOWS__)
    WSACleanup();
#endif

    printf("[BINKP] pcbbinkp exiting (rc=%d)\n", rc);
    return rc;
}
