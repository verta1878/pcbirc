/* ====================================================================
 * pdnet.c — PabloDraw Network Protocol (Client/Server)
 * ====================================================================
 * C port of sysop/0's pdnet.pas. TCP teleconference for
 * collaborative ANSI art editing.
 *
 * Wire protocol: [LEN:4 LE][CMD:1][DATA:LEN-1]
 *
 * Copyright (C) 2026 pcbrevival contributors (GPLv3)
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __OS2__
 #define INCL_DOS
 #include <os2.h>
 #include <types.h>
 #include <sys/socket.h>
 #include <sys/time.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <tcpustd.h>
 typedef int socklen_t;
 #define closesocket soclose
#elif defined(__NT__) || defined(__WINDOWS__)
 #include <winsock2.h>
 typedef int socklen_t;
#else
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/select.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <unistd.h>
 #define closesocket close
#endif

#include "pcbdraw.h"

/* ---- Message pack/unpack helpers ---- */

void msg_pack_word(unsigned char *d, int *p, unsigned short w)
{
    d[*p] = (unsigned char)(w & 0xFF);
    d[*p + 1] = (unsigned char)((w >> 8) & 0xFF);
    *p += 2;
}

unsigned short msg_unpack_word(const unsigned char *d, int *p)
{
    unsigned short w = d[*p] | ((unsigned short)d[*p + 1] << 8);
    *p += 2;
    return w;
}

void msg_pack_str(unsigned char *d, int *p, const char *s)
{
    int len = (int)strlen(s);
    if (len > 255) len = 255;
    d[*p] = (unsigned char)len;
    (*p)++;
    memcpy(d + *p, s, len);
    *p += len;
}

int msg_unpack_str(const unsigned char *d, int *p, char *out, int maxlen)
{
    int len = d[*p];
    (*p)++;
    if (len > maxlen - 1) len = maxlen - 1;
    memcpy(out, d + *p, len);
    out[len] = '\0';
    *p += d[(*p) - 1]; /* advance by original length, not truncated */
    /* Fix: advance by actual wire length */
    *p = *p - d[(*p) - len - 1] + d[*p - len - 1];
    return len;
}

/* Simpler unpack that just works */
static int unpack_str(const unsigned char *d, int *p, char *out, int maxlen)
{
    int len = d[*p];
    int copy = len;
    (*p)++;
    if (copy > maxlen - 1) copy = maxlen - 1;
    memcpy(out, d + *p, copy);
    out[copy] = '\0';
    *p += len;
    return copy;
}

/* ---- Send raw frame ---- */

static int send_raw(int sock, const PDNetMsg *msg)
{
    unsigned char hdr[5];

    hdr[0] = (unsigned char)(msg->len & 0xFF);
    hdr[1] = (unsigned char)((msg->len >> 8) & 0xFF);
    hdr[2] = (unsigned char)((msg->len >> 16) & 0xFF);
    hdr[3] = (unsigned char)((msg->len >> 24) & 0xFF);
    hdr[4] = msg->cmd;

    if (send(sock, (char *)hdr, 5, 0) != 5)
        return -1;
    if (msg->len > 1) {
        if (send(sock, (char *)msg->data, msg->len - 1, 0) != (int)(msg->len - 1))
            return -1;
    }
    return 0;
}

/* ---- Build chat message ---- */

static void build_chat_msg(PDNetMsg *msg, const char *from, const char *text)
{
    int p = 0;
    msg->cmd = CMD_CHAT;
    msg_pack_str(msg->data, &p, from);
    msg_pack_str(msg->data, &p, text);
    msg->len = p + 1;
}

/* ==================================================================
 * SERVER
 * ================================================================== */

static int find_free_slot(PDServer *srv)
{
    int i;
    for (i = 0; i < PD_MAX_USERS; i++)
        if (!srv->users[i].active) return i;
    return -1;
}

static int user_count(PDServer *srv)
{
    int i, c = 0;
    for (i = 0; i < PD_MAX_USERS; i++)
        if (srv->users[i].active) c++;
    return c;
}

static void send_to_all(PDServer *srv, const PDNetMsg *msg, int except)
{
    int i;
    for (i = 0; i < PD_MAX_USERS; i++)
        if (srv->users[i].active && i != except)
            send_raw(srv->users[i].socket, msg);
}

static void send_welcome(PDServer *srv, int idx)
{
    PDNetMsg msg;
    int p = 0;

    msg.cmd = CMD_WELCOME;
    msg.data[p++] = (unsigned char)idx;
    msg.data[p++] = (unsigned char)srv->users[idx].level;
    msg_pack_word(msg.data, &p, (unsigned short)srv->canvas->width);
    msg_pack_word(msg.data, &p, (unsigned short)srv->canvas->height);
    msg.len = p + 1;
    send_raw(srv->users[idx].socket, &msg);
}

static void send_canvas(PDServer *srv, int idx)
{
    PDNetMsg msg;
    int p = 0, x, y;
    PDCanvasElement e;

    msg.cmd = CMD_LOADDOC;
    msg_pack_word(msg.data, &p, (unsigned short)srv->canvas->width);
    msg_pack_word(msg.data, &p, (unsigned short)srv->canvas->height);

    for (y = 0; y < srv->canvas->height; y++)
        for (x = 0; x < srv->canvas->width; x++) {
            if (p + 3 >= PD_MAX_MSG) break;
            e = canvas_get(srv->canvas, x, y);
            msg.data[p++] = (unsigned char)(e.ch.ch & 0xFF);
            msg.data[p++] = (unsigned char)((e.ch.ch >> 8) & 0xFF);
            msg.data[p++] = ATTR_BYTE(e.attr);
        }

    msg.len = p + 1;
    send_raw(srv->users[idx].socket, &msg);
}

static void send_user_list(PDServer *srv)
{
    PDNetMsg msg;
    int p = 0, i, count;

    msg.cmd = CMD_USERLIST;
    count = user_count(srv);
    msg.data[p++] = (unsigned char)count;

    for (i = 0; i < PD_MAX_USERS; i++)
        if (srv->users[i].active) {
            msg.data[p++] = (unsigned char)i;
            msg_pack_str(msg.data, &p, srv->users[i].alias);
            msg.data[p++] = (unsigned char)srv->users[i].level;
        }

    msg.len = p + 1;
    send_to_all(srv, &msg, -1);
}

static void disconnect_user(PDServer *srv, int idx, const char *reason)
{
    PDNetMsg msg;
    int p = 0;

    if (!srv->users[idx].active) return;

    msg.cmd = CMD_BYE;
    msg_pack_str(msg.data, &p, reason);
    msg.len = p + 1;
    send_raw(srv->users[idx].socket, &msg);
    closesocket(srv->users[idx].socket);

    printf("[PCBDRAW] %s left: %s\n", srv->users[idx].alias, reason);

    srv->users[idx].active = 0;
    srv->users[idx].recv_len = 0;

    pd_server_broadcast_chat(srv, "Server",
        srv->users[idx].alias);  /* simplified */
    send_user_list(srv);
}

static void server_process_msg(PDServer *srv, int idx, const PDNetMsg *msg)
{
    int p = 0;
    char alias[32], pass[64], text[256];
    PDNetMsg chat_msg;
    int x1, y1, x2, y2, x, y;
    PDCanvasElement e;

    switch (msg->cmd) {
    case CMD_AUTH:
        p = 1; /* skip version byte */
        unpack_str(msg->data, &p, alias, sizeof(alias));
        unpack_str(msg->data, &p, pass, sizeof(pass));

        if (srv->password[0] && strcmp(pass, srv->password) != 0) {
            disconnect_user(srv, idx, "Wrong password");
            return;
        }

        strncpy(srv->users[idx].alias, alias, 31);
        if (srv->password[0] && strcmp(pass, srv->password) == 0)
            srv->users[idx].level = LEVEL_OPERATOR;

        send_welcome(srv, idx);
        send_canvas(srv, idx);
        send_user_list(srv);

        printf("[PCBDRAW] %s joined (slot %d)\n", alias, idx);
        pd_server_broadcast_chat(srv, "Server", alias);
        break;

    case CMD_CHAT:
        p = 0;
        unpack_str(msg->data, &p, text, sizeof(text));
        build_chat_msg(&chat_msg, srv->users[idx].alias, text);
        send_to_all(srv, &chat_msg, -1);
        printf("[PCBDRAW] <%s> %s\n", srv->users[idx].alias, text);
        break;

    case CMD_UPDATE:
        if (srv->users[idx].level < LEVEL_EDITOR) break;
        p = 0;
        x1 = msg_unpack_word(msg->data, &p);
        y1 = msg_unpack_word(msg->data, &p);
        x2 = msg_unpack_word(msg->data, &p);
        y2 = msg_unpack_word(msg->data, &p);
        for (y = y1; y <= y2; y++)
            for (x = x1; x <= x2; x++) {
                if (p + 2 < (int)msg->len) {
                    e.ch.ch = msg->data[p] | ((short)msg->data[p+1] << 8);
                    p += 2;
                    ATTR_INIT(e.attr, msg->data[p]);
                    p++;
                    canvas_set(srv->canvas, x, y, e);
                }
            }
        send_to_all(srv, msg, idx);
        break;

    case CMD_CURSOR:
        p = 0;
        srv->users[idx].cursor_x = msg_unpack_word(msg->data, &p);
        srv->users[idx].cursor_y = msg_unpack_word(msg->data, &p);
        send_to_all(srv, msg, idx);
        break;
    }
}

static void handle_user_data(PDServer *srv, int idx)
{
    int n;
    unsigned long frame_len;
    PDNetMsg msg;

    n = recv(srv->users[idx].socket,
             (char *)(srv->users[idx].recv_buf + srv->users[idx].recv_len),
             PD_RECV_BUF - srv->users[idx].recv_len, 0);

    if (n <= 0) {
        disconnect_user(srv, idx, "Connection lost");
        return;
    }

    srv->users[idx].recv_len += n;

    /* Process complete frames */
    while (srv->users[idx].recv_len >= 5) {
        frame_len = srv->users[idx].recv_buf[0] |
            ((unsigned long)srv->users[idx].recv_buf[1] << 8) |
            ((unsigned long)srv->users[idx].recv_buf[2] << 16) |
            ((unsigned long)srv->users[idx].recv_buf[3] << 24);

        if (frame_len > PD_MAX_MSG) {
            disconnect_user(srv, idx, "Message too large");
            return;
        }

        if ((unsigned long)srv->users[idx].recv_len < frame_len + 4)
            break;

        msg.len = frame_len;
        msg.cmd = srv->users[idx].recv_buf[4];
        if (frame_len > 1)
            memcpy(msg.data, srv->users[idx].recv_buf + 5, frame_len - 1);

        n = (int)(frame_len + 4);
        srv->users[idx].recv_len -= n;
        if (srv->users[idx].recv_len > 0)
            memmove(srv->users[idx].recv_buf,
                    srv->users[idx].recv_buf + n,
                    srv->users[idx].recv_len);

        server_process_msg(srv, idx, &msg);
    }
}

/* ---- Public Server API ---- */

int pd_server_start(PDServer *srv, PDCanvas *canvas, unsigned short port)
{
    struct sockaddr_in addr;
    int opt = 1;
    int i;

    memset(srv, 0, sizeof(PDServer));
    srv->canvas = canvas;
    srv->port = port;
    srv->default_level = LEVEL_EDITOR;
    srv->listen_sock = -1;

    for (i = 0; i < PD_MAX_USERS; i++) {
        srv->users[i].active = 0;
        srv->users[i].recv_len = 0;
    }

    srv->listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->listen_sock < 0) return -1;

    setsockopt(srv->listen_sock, SOL_SOCKET, SO_REUSEADDR,
               (char *)&opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(srv->listen_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        closesocket(srv->listen_sock);
        srv->listen_sock = -1;
        return -1;
    }

    if (listen(srv->listen_sock, 5) < 0) {
        closesocket(srv->listen_sock);
        srv->listen_sock = -1;
        return -1;
    }

    srv->running = 1;
    printf("[PCBDRAW] Server started on port %u\n", port);
    return 0;
}

void pd_server_stop(PDServer *srv)
{
    int i;
    if (!srv->running) return;

    for (i = 0; i < PD_MAX_USERS; i++)
        if (srv->users[i].active)
            disconnect_user(srv, i, "Server shutting down");

    closesocket(srv->listen_sock);
    srv->listen_sock = -1;
    srv->running = 0;
    printf("[PCBDRAW] Server stopped\n");
}

void pd_server_poll(PDServer *srv)
{
    fd_set rset;
    struct timeval tv;
    int i, maxfd;
    struct sockaddr_in client_addr;
    socklen_t addr_len;
    int client_sock, idx;

    if (!srv->running) return;

    FD_ZERO(&rset);
    FD_SET(srv->listen_sock, &rset);
    maxfd = srv->listen_sock;

    for (i = 0; i < PD_MAX_USERS; i++)
        if (srv->users[i].active) {
            FD_SET(srv->users[i].socket, &rset);
            if (srv->users[i].socket > maxfd)
                maxfd = srv->users[i].socket;
        }

    tv.tv_sec = 0;
    tv.tv_usec = 10000; /* 10ms */

    if (select(maxfd + 1, &rset, NULL, NULL, &tv) <= 0)
        return;

    /* New connection? */
    if (FD_ISSET(srv->listen_sock, &rset)) {
        addr_len = sizeof(client_addr);
        client_sock = accept(srv->listen_sock,
                             (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock >= 0) {
            idx = find_free_slot(srv);
            if (idx >= 0) {
                srv->users[idx].active = 1;
                srv->users[idx].socket = client_sock;
                sprintf(srv->users[idx].alias, "User%d", idx);
                srv->users[idx].level = srv->default_level;
                srv->users[idx].cursor_x = 0;
                srv->users[idx].cursor_y = 0;
                srv->users[idx].recv_len = 0;
            } else {
                closesocket(client_sock);
            }
        }
    }

    /* Data from users? */
    for (i = 0; i < PD_MAX_USERS; i++)
        if (srv->users[i].active && FD_ISSET(srv->users[i].socket, &rset))
            handle_user_data(srv, i);
}

void pd_server_kick(PDServer *srv, int idx, const char *reason)
{
    if (idx >= 0 && idx < PD_MAX_USERS && srv->users[idx].active)
        disconnect_user(srv, idx, reason);
}

void pd_server_broadcast_chat(PDServer *srv, const char *from, const char *text)
{
    PDNetMsg msg;
    build_chat_msg(&msg, from, text);
    send_to_all(srv, &msg, -1);
}

/* ==================================================================
 * CLIENT
 * ================================================================== */

int pd_client_connect(PDClient *cli, PDCanvas *canvas,
                      const char *host, unsigned short port,
                      const char *alias, const char *pass)
{
    struct hostent *he;
    struct sockaddr_in addr;
    PDNetMsg msg;
    int p;

    memset(cli, 0, sizeof(PDClient));
    cli->canvas = canvas;
    cli->socket = -1;
    cli->my_index = -1;
    cli->level = LEVEL_VIEWER;
    strncpy(cli->alias, alias, 31);

    he = gethostbyname(host);
    if (!he) return -1;

    cli->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli->socket < 0) return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(cli->socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        closesocket(cli->socket);
        cli->socket = -1;
        return -1;
    }

    cli->connected = 1;

    /* Send AUTH */
    msg.cmd = CMD_AUTH;
    p = 0;
    msg.data[p++] = PD_NET_VERSION;
    msg_pack_str(msg.data, &p, alias);
    msg_pack_str(msg.data, &p, pass);
    msg.len = p + 1;
    send_raw(cli->socket, &msg);

    printf("[PCBDRAW] Connected to %s:%u as %s\n", host, port, alias);
    return 0;
}

void pd_client_disconnect(PDClient *cli)
{
    PDNetMsg msg;
    int p = 0;

    if (!cli->connected) return;

    msg.cmd = CMD_BYE;
    msg_pack_str(msg.data, &p, "Quit");
    msg.len = p + 1;
    send_raw(cli->socket, &msg);

    closesocket(cli->socket);
    cli->socket = -1;
    cli->connected = 0;
    printf("[PCBDRAW] Disconnected\n");
}

static void client_process_msg(PDClient *cli, const PDNetMsg *msg)
{
    int p = 0, i, count, x, y, idx;
    char from[32], text[256];
    unsigned short w, h, x1, y1, x2, y2;
    PDCanvasElement e;

    switch (msg->cmd) {
    case CMD_WELCOME:
        cli->my_index = msg->data[p++];
        cli->level = (UserLevel)msg->data[p++];
        w = msg_unpack_word(msg->data, &p);
        h = msg_unpack_word(msg->data, &p);
        canvas_resize(cli->canvas, w, h);
        printf("[PCBDRAW] Welcome: slot %d, level %d, canvas %dx%d\n",
               cli->my_index, cli->level, w, h);
        break;

    case CMD_CHAT:
        unpack_str(msg->data, &p, from, sizeof(from));
        unpack_str(msg->data, &p, text, sizeof(text));
        printf("[PCBDRAW] <%s> %s\n", from, text);
        break;

    case CMD_USERLIST:
        count = msg->data[p++];
        cli->user_count = count;
        for (i = 0; i < PD_MAX_USERS; i++) cli->users[i].active = 0;
        for (i = 0; i < count; i++) {
            idx = msg->data[p++];
            unpack_str(msg->data, &p, cli->users[idx].alias, 32);
            cli->users[idx].level = (UserLevel)msg->data[p++];
            cli->users[idx].active = 1;
        }
        break;

    case CMD_LOADDOC:
        w = msg_unpack_word(msg->data, &p);
        h = msg_unpack_word(msg->data, &p);
        canvas_resize(cli->canvas, w, h);
        for (y = 0; y < h; y++)
            for (x = 0; x < w; x++) {
                if (p + 2 < (int)msg->len) {
                    e.ch.ch = msg->data[p] | ((short)msg->data[p+1] << 8);
                    p += 2;
                    ATTR_INIT(e.attr, msg->data[p]);
                    p++;
                    canvas_set(cli->canvas, x, y, e);
                }
            }
        break;

    case CMD_UPDATE:
        x1 = msg_unpack_word(msg->data, &p);
        y1 = msg_unpack_word(msg->data, &p);
        x2 = msg_unpack_word(msg->data, &p);
        y2 = msg_unpack_word(msg->data, &p);
        for (y = y1; y <= y2; y++)
            for (x = x1; x <= x2; x++) {
                if (p + 2 < (int)msg->len) {
                    e.ch.ch = msg->data[p] | ((short)msg->data[p+1] << 8);
                    p += 2;
                    ATTR_INIT(e.attr, msg->data[p]);
                    p++;
                    canvas_set(cli->canvas, x, y, e);
                }
            }
        break;

    case CMD_USERSTATUS:
        idx = msg->data[p++];
        cli->users[idx].level = (UserLevel)msg->data[p++];
        if (idx == cli->my_index) cli->level = cli->users[idx].level;
        break;

    case CMD_BYE:
        pd_client_disconnect(cli);
        break;
    }
}

void pd_client_poll(PDClient *cli)
{
    fd_set rset;
    struct timeval tv;
    int n;
    PDNetMsg msg;
    unsigned long frame_len;

    if (!cli->connected) return;

    FD_ZERO(&rset);
    FD_SET(cli->socket, &rset);
    tv.tv_sec = 0;
    tv.tv_usec = 10000;

    if (select(cli->socket + 1, &rset, NULL, NULL, &tv) > 0) {
        n = recv(cli->socket,
                 (char *)(cli->recv_buf + cli->recv_len),
                 PD_RECV_BUF - cli->recv_len, 0);
        if (n <= 0) { pd_client_disconnect(cli); return; }
        cli->recv_len += n;
    }

    /* Process complete frames */
    while (cli->recv_len >= 5) {
        frame_len = cli->recv_buf[0] |
            ((unsigned long)cli->recv_buf[1] << 8) |
            ((unsigned long)cli->recv_buf[2] << 16) |
            ((unsigned long)cli->recv_buf[3] << 24);

        if (frame_len > PD_MAX_MSG) { pd_client_disconnect(cli); return; }
        if ((unsigned long)cli->recv_len < frame_len + 4) break;

        msg.len = frame_len;
        msg.cmd = cli->recv_buf[4];
        if (frame_len > 1)
            memcpy(msg.data, cli->recv_buf + 5, frame_len - 1);

        n = (int)(frame_len + 4);
        cli->recv_len -= n;
        if (cli->recv_len > 0)
            memmove(cli->recv_buf, cli->recv_buf + n, cli->recv_len);

        client_process_msg(cli, &msg);
    }
}

void pd_client_send_chat(PDClient *cli, const char *text)
{
    PDNetMsg msg;
    int p = 0;
    msg.cmd = CMD_CHAT;
    msg_pack_str(msg.data, &p, text);
    msg.len = p + 1;
    send_raw(cli->socket, &msg);
}

void pd_client_send_update(PDClient *cli, int x1, int y1, int x2, int y2)
{
    PDNetMsg msg;
    int p = 0, x, y;
    PDCanvasElement e;

    msg.cmd = CMD_UPDATE;
    msg_pack_word(msg.data, &p, (unsigned short)x1);
    msg_pack_word(msg.data, &p, (unsigned short)y1);
    msg_pack_word(msg.data, &p, (unsigned short)x2);
    msg_pack_word(msg.data, &p, (unsigned short)y2);

    for (y = y1; y <= y2; y++)
        for (x = x1; x <= x2; x++) {
            if (p + 3 >= PD_MAX_MSG) break;
            e = canvas_get(cli->canvas, x, y);
            msg.data[p++] = (unsigned char)(e.ch.ch & 0xFF);
            msg.data[p++] = (unsigned char)((e.ch.ch >> 8) & 0xFF);
            msg.data[p++] = ATTR_BYTE(e.attr);
        }

    msg.len = p + 1;
    send_raw(cli->socket, &msg);
}

void pd_client_send_cursor(PDClient *cli, int x, int y)
{
    PDNetMsg msg;
    int p = 0;
    msg.cmd = CMD_CURSOR;
    msg_pack_word(msg.data, &p, (unsigned short)x);
    msg_pack_word(msg.data, &p, (unsigned short)y);
    msg.len = p + 1;
    send_raw(cli->socket, &msg);
}
