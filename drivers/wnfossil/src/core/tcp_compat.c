/* ====================================================================
 * tcp_compat.c — TCP/Winsock Backend (VMODEM transport)
 * ====================================================================
 * Implements wfp_tcp_* callbacks. Winsock 1.1 on Win98, 2.2 on NT+.
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "wf_core.h"

#pragma comment(lib, "ws2_32.lib")

static int g_wsa_init = 0;

static void wsa_ensure(void)
{
    if (!g_wsa_init) {
        WSADATA wsa;
        /* Winsock 1.1 on Win98, 2.2 on NT+. Request 2.2, fall back. */
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            WSAStartup(MAKEWORD(1, 1), &wsa);
        g_wsa_init = 1;
    }
}

int wfp_tcp_connect(WfPort *p, const char *host, int port)
{
    SOCKET s;
    struct sockaddr_in addr;
    struct hostent *he;
    u_long nonblock = 1;

    wsa_ensure();

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        wfp_log("Port %d: socket() failed (%d)", p->index, WSAGetLastError());
        return -1;
    }

    he = gethostbyname(host);
    if (!he) {
        wfp_log("Port %d: DNS failed for %s", p->index, host);
        closesocket(s);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((u_short)port);
    memcpy(&addr.sin_addr, he->h_addr_list[0], (size_t)he->h_length);

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR) {
        wfp_log("Port %d: connect to %s:%d failed (%d)",
                p->index, host, port, WSAGetLastError());
        closesocket(s);
        return -1;
    }

    ioctlsocket(s, FIONBIO, &nonblock);

    /* Disable Nagle for low latency (critical for BBS) */
    {
        int nodelay = 1;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY,
                   (const char *)&nodelay, sizeof(nodelay));
    }

    /* TCP keepalive — detect dead connections. Without this,
     * a dropped telnet connection hangs forever. */
    {
        BOOL keepalive = TRUE;
        setsockopt(s, SOL_SOCKET, SO_KEEPALIVE,
                   (const char *)&keepalive, sizeof(keepalive));
    }

    p->hSock = (void *)(uintptr_t)s;
    p->vm_online = 1;
    wfp_log("Port %d: TCP connected to %s:%d", p->index, host, port);
    return 0;
}

int wfp_tcp_listen(WfPort *p, int port)
{
    SOCKET s;
    struct sockaddr_in addr;
    int reuse = 1;

    wsa_ensure();

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return -1;

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((u_short)port);

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR ||
        listen(s, SOMAXCONN) == SOCKET_ERROR) {
        wfp_log("Port %d: listen on %d failed (%d)", p->index, port, WSAGetLastError());
        closesocket(s);
        return -1;
    }

    p->hListen = (void *)(uintptr_t)s;
    wfp_log("Port %d: listening on TCP port %d", p->index, port);
    return 0;
}

int wfp_tcp_accept(WfPort *p)
{
    SOCKET ls = (SOCKET)(uintptr_t)p->hListen;
    SOCKET cs;
    struct sockaddr_in client;
    int clen = sizeof(client);
    u_long nonblock = 1;
    fd_set readfds;
    struct timeval tv;

    if (ls == INVALID_SOCKET || ls == 0) return -1;

    /* Non-blocking check for incoming connection */
    FD_ZERO(&readfds);
    FD_SET(ls, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (select(0, &readfds, NULL, NULL, &tv) <= 0)
        return -1;

    cs = accept(ls, (struct sockaddr *)&client, &clen);
    if (cs == INVALID_SOCKET) return -1;

    /* Security check */
    {
        char ip[20];
        snprintf(ip, sizeof(ip), "%s", inet_ntoa(client.sin_addr));
        if (!wf_sec_check_ip(p, ip)) {
            closesocket(cs);
            return -1;
        }
        wfp_log("Port %d: accepted from %s", p->index, ip);
    }

    ioctlsocket(cs, FIONBIO, &nonblock);

    {
        int nodelay = 1;
        setsockopt(cs, IPPROTO_TCP, TCP_NODELAY,
                   (const char *)&nodelay, sizeof(nodelay));
    }

    p->hSock = (void *)(uintptr_t)cs;
    p->vm_online = 1;
    return 0;
}

void wfp_tcp_close(WfPort *p)
{
    SOCKET s = (SOCKET)(uintptr_t)p->hSock;
    SOCKET ls = (SOCKET)(uintptr_t)p->hListen;

    if (s && s != INVALID_SOCKET) {
        shutdown(s, SD_BOTH);
        closesocket(s);
        p->hSock = NULL;
    }
    if (ls && ls != INVALID_SOCKET) {
        closesocket(ls);
        p->hListen = NULL;
    }
    p->vm_online = 0;
}

int wfp_tcp_read(WfPort *p, void *buf, int len)
{
    SOCKET s = (SOCKET)(uintptr_t)p->hSock;
    int n;

    if (!s || s == INVALID_SOCKET) return -1;

    n = recv(s, (char *)buf, len, 0);
    if (n == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK) return 0;
        p->vm_online = 0;
        return -1;
    }
    if (n == 0) { p->vm_online = 0; return -1; } /* Graceful close */
    return n;
}

int wfp_tcp_write(WfPort *p, const void *buf, int len)
{
    SOCKET s = (SOCKET)(uintptr_t)p->hSock;
    int n;

    if (!s || s == INVALID_SOCKET) return -1;

    n = send(s, (const char *)buf, len, 0);
    if (n == SOCKET_ERROR) {
        p->vm_online = 0;
        return -1;
    }
    return n;
}

int wfp_tcp_data_ready(WfPort *p)
{
    SOCKET s = (SOCKET)(uintptr_t)p->hSock;
    u_long avail = 0;
    if (!s || s == INVALID_SOCKET) return 0;
    ioctlsocket(s, FIONREAD, &avail);
    return (avail > 0) ? 1 : 0;
}

/* WSACleanup — call on DLL_PROCESS_DETACH (MF-1 fix) */
void wfp_tcp_cleanup(void)
{
    if (g_wsa_init) {
        WSACleanup();
        g_wsa_init = 0;
    }
}

#endif /* _WIN32 */
