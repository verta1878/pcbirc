/* ====================================================================
 * binkp.c — BinkP/1.1 Protocol Core for PCBoard
 * ====================================================================
 * Clean-room implementation based on FTS-1026 and FSP-1024 specs.
 * NOT derived from binkd source — protocol spec is public.
 *
 * Builds with OpenWatcom: wcc386 -bt=os2 -5r -oxs binkp.c
 * Tagged output: all printf goes through binkp_log() for pcbis_ui
 *
 * Copyright (C) 2026 pcbrevival contributors
 * License: GPLv3
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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
 #include <tcpustd.h>
 #define closesocket soclose
 #define SOCKET      int
 #define INVALID_SOCKET (-1)
#elif defined(__NT__) || defined(__WINDOWS__)
 #include <winsock2.h>
 #define closesocket closesocket
 typedef int socklen_t;
#else
 /* DOS — sockets via FOSSIL/VMODEM tunnel, or Waterloo TCP */
 #include <sys/types.h>
 #define SOCKET      int
 #define INVALID_SOCKET (-1)
 #define closesocket close
#endif

#include "binkp.h"

/* ====================================================================
 * Tagged logging — all output goes through here for pcbis_ui
 * ==================================================================== */

int g_verbose = 1;      /* 0=quiet, 1=normal, 2=debug, 3=trace */

void binkp_log(int level, const char *fmt, ...)
{
    va_list ap;

    if (level > g_verbose)
        return;

    printf("[BINKP] ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
    fflush(stdout);
}

/* ====================================================================
 * FTN Address Helpers
 * ==================================================================== */

/* Parse "zone:net/node.point" or "zone:net/node" */
int ftn_parse(const char *str, FtnAddr *addr)
{
    unsigned int z, n, nd, p;
    int rc;

    memset(addr, 0, sizeof(FtnAddr));
    p = 0;
    rc = sscanf(str, "%u:%u/%u.%u", &z, &n, &nd, &p);
    if (rc < 3)
        return -1;

    addr->zone  = (unsigned short)z;
    addr->net   = (unsigned short)n;
    addr->node  = (unsigned short)nd;
    addr->point = (unsigned short)p;
    return 0;
}

int ftn_format(const FtnAddr *addr, char *buf, int bufsize)
{
    if (addr->point)
        return snprintf(buf, bufsize, "%u:%u/%u.%u",
                        addr->zone, addr->net, addr->node, addr->point);
    else
        return snprintf(buf, bufsize, "%u:%u/%u",
                        addr->zone, addr->net, addr->node);
}

int ftn_match(const FtnAddr *a, const FtnAddr *b)
{
    return (a->zone == b->zone && a->net == b->net &&
            a->node == b->node && a->point == b->point);
}

/* ====================================================================
 * Low-Level Frame I/O
 * ==================================================================== */

/* Read exactly n bytes from socket. Returns 0 on success, -1 on error. */
static int sock_readn(int sock, unsigned char *buf, int n)
{
    int total = 0, r;

    while (total < n) {
        r = recv(sock, (char *)(buf + total), n - total, 0);
        if (r <= 0)
            return -1;
        total += r;
    }
    return 0;
}

/* Write exactly n bytes to socket. Returns 0 on success, -1 on error. */
static int sock_writen(int sock, const unsigned char *buf, int n)
{
    int total = 0, w;

    while (total < n) {
        w = send(sock, (const char *)(buf + total), n - total, 0);
        if (w <= 0)
            return -1;
        total += w;
    }
    return 0;
}

/* Send a command frame: [hdr_hi | hdr_lo] [cmd_id] [data...] */
int binkp_send_cmd(BinkpSession *s, int cmd, const char *data)
{
    unsigned char hdr[2];
    int datalen;

    datalen = data ? (int)strlen(data) : 0;
    if (datalen + 1 > BINKP_MAX_BLOCK) {
        binkp_log(0, "ERROR: command too long (%d bytes)", datalen);
        return -1;
    }

    /* Header: bit 15 set = command, bits 14..0 = payload length */
    hdr[0] = (unsigned char)(((datalen + 1) >> 8) | 0x80);
    hdr[1] = (unsigned char)((datalen + 1) & 0xFF);

    if (sock_writen(s->sock, hdr, 2) < 0)
        return -1;

    /* Payload: command byte + data */
    s->obuf[0] = (unsigned char)cmd;
    if (datalen > 0)
        memcpy(s->obuf + 1, data, datalen);

    if (sock_writen(s->sock, s->obuf, datalen + 1) < 0)
        return -1;

    if (cmd < M_COUNT) {
        static const char *cmd_names[] = {
            "NUL","ADR","PWD","FILE","OK","EOB","GOT","ERR","BSY","GET","SKIP"
        };
        binkp_log(2, ">> %s %s", cmd_names[cmd], data ? data : "");
    }

    return 0;
}

/* Send a data frame: [hdr_hi | hdr_lo] [data...] */
int binkp_send_data(BinkpSession *s, const unsigned char *data, int len)
{
    unsigned char hdr[2];

    if (len > BINKP_MAX_BLOCK || len < 0)
        return -1;

    /* Header: bit 15 clear = data, bits 14..0 = payload length */
    hdr[0] = (unsigned char)((len >> 8) & 0x7F);
    hdr[1] = (unsigned char)(len & 0xFF);

    if (sock_writen(s->sock, hdr, 2) < 0)
        return -1;
    if (len > 0 && sock_writen(s->sock, data, len) < 0)
        return -1;

    return 0;
}

/* Receive one frame. Sets *is_cmd, *cmd (if command), fills data, sets *len.
 * Returns 0 on success, -1 on error/disconnect. */
int binkp_recv_frame(BinkpSession *s, int *is_cmd, int *cmd,
                     unsigned char *data, int *len)
{
    unsigned char hdr[2];
    int payload_len;

    if (sock_readn(s->sock, hdr, 2) < 0)
        return -1;

    *is_cmd = (hdr[0] & 0x80) ? 1 : 0;
    payload_len = ((hdr[0] & 0x7F) << 8) | hdr[1];

    if (payload_len > BINKP_MAX_BLOCK) {
        binkp_log(0, "ERROR: frame too large (%d bytes)", payload_len);
        return -1;
    }

    if (payload_len > 0) {
        if (sock_readn(s->sock, data, payload_len) < 0)
            return -1;
    }

    if (*is_cmd && payload_len > 0) {
        *cmd = data[0];
        /* Shift data left past command byte */
        memmove(data, data + 1, payload_len - 1);
        data[payload_len - 1] = '\0';
        *len = payload_len - 1;

        if (*cmd < M_COUNT) {
            static const char *cmd_names[] = {
                "NUL","ADR","PWD","FILE","OK","EOB","GOT","ERR","BSY","GET","SKIP"
            };
            binkp_log(2, "<< %s %s", cmd_names[*cmd], (char *)data);
        }
    } else {
        *cmd = -1;
        *len = payload_len;
    }

    return 0;
}

/* ====================================================================
 * Command Handlers
 * ==================================================================== */

/* M_NUL — informational: SYS, ZYZ, LOC, NDL, TIME, VER, OPT, TRF */
int binkp_handle_nul(BinkpSession *s, const char *data)
{
    binkp_log(1, "Remote info: %s", data);

    /* Check for OPT CRAM-MD5-xxxxxxxx challenge */
    if (strncmp(data, "OPT ", 4) == 0) {
        const char *p = strstr(data, "CRAM-MD5-");
        if (p) {
            strncpy(s->challenge, p + 9, sizeof(s->challenge) - 1);
            s->challenge[sizeof(s->challenge) - 1] = '\0';
            s->options |= OPT_CRAM;
            binkp_log(1, "CRAM-MD5 challenge received");
        }
        if (strstr(data, "NR"))
            s->options |= OPT_NR;
        if (strstr(data, "ND"))
            s->options |= OPT_ND;
        if (strstr(data, "CRYPT"))
            s->options |= OPT_CRYPT;
    }
    return 0;
}

/* M_ADR — remote's FTN address list (space-separated) */
int binkp_handle_adr(BinkpSession *s, const char *data)
{
    char tmp[512];
    char *tok;
    char buf[40];

    strncpy(tmp, data, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    /* Take first address as primary */
    tok = strtok(tmp, " ");
    if (!tok || ftn_parse(tok, &s->remote_addr) < 0) {
        binkp_log(0, "ERROR: cannot parse remote address: %s", data);
        binkp_send_cmd(s, M_ERR, "Bad address");
        s->state = STATE_ERROR;
        return -1;
    }

    ftn_format(&s->remote_addr, buf, sizeof(buf));
    binkp_log(1, "Remote address: %s", buf);

    if (s->direction == BINKP_ANSWER)
        s->state = STATE_WAIT_PWD;
    else
        s->state = STATE_WAIT_OK;

    return 0;
}

/* M_PWD — password from caller (answering side handles this) */
int binkp_handle_pwd(BinkpSession *s, const char *data)
{
    int auth_ok = 0;

    if (s->direction != BINKP_ANSWER) {
        binkp_log(0, "ERROR: got M_PWD but we are originating");
        return -1;
    }

    /* Check for CRAM-MD5 digest */
    if (strncmp(data, "CRAM-MD5-", 9) == 0) {
        if (binkp_verify_digest(s->password, s->challenge, data + 9))
            auth_ok = 1;
        else
            binkp_log(1, "CRAM-MD5 authentication failed");
    } else {
        /* Plain text password */
        if (strcmp(data, s->password) == 0 || strcmp(data, "-") == 0)
            auth_ok = 1;
    }

    if (auth_ok || strlen(s->password) == 0 || strcmp(s->password, "-") == 0) {
        binkp_send_cmd(s, M_OK, auth_ok ? "secure" : "non-secure");
        s->state = STATE_AUTH_OK;
        binkp_log(1, "Session authenticated (%s)",
                  auth_ok ? "secure" : "non-secure");
    } else {
        binkp_send_cmd(s, M_ERR, "Bad password");
        s->state = STATE_ERROR;
        binkp_log(0, "Authentication FAILED");
        return -1;
    }

    return 0;
}

/* M_FILE — incoming file header: "name size unixtime offset" */
int binkp_handle_file(BinkpSession *s, const char *data)
{
    char name[256];
    long size, ftime, offset;
    char path[520];
    int rc;

    rc = sscanf(data, "%255s %ld %ld %ld", name, &size, &ftime, &offset);
    if (rc < 3) {
        binkp_log(0, "ERROR: bad M_FILE args: %s", data);
        binkp_send_cmd(s, M_SKIP, data);
        return 0;
    }
    if (rc < 4)
        offset = 0;

    /* Sanitize filename — no path separators allowed */
    {
        char *p;
        for (p = name; *p; p++) {
            if (*p == '/' || *p == '\\' || *p == ':')
                *p = '_';
        }
    }

    binkp_log(1, "Receiving: %s (%ld bytes, offset %ld)", name, size, offset);

    /* Open file in temp inbound */
    snprintf(path, sizeof(path), "%s%s", s->temp_inbound, name);

    if (offset > 0)
        s->in_file.fp = fopen(path, "r+b");
    else
        s->in_file.fp = fopen(path, "wb");

    if (!s->in_file.fp) {
        binkp_log(0, "ERROR: cannot open %s for writing", path);
        binkp_send_cmd(s, M_SKIP, data);
        return 0;
    }

    if (offset > 0)
        fseek(s->in_file.fp, offset, SEEK_SET);

    strncpy(s->in_file.name, name, sizeof(s->in_file.name) - 1);
    s->in_file.size   = size;
    s->in_file.time   = ftime;
    s->in_file.offset = offset;
    s->state = STATE_TRANSFER;

    return 0;
}

/* M_OK — password accepted (calling side gets this) */
int binkp_handle_ok(BinkpSession *s, const char *data)
{
    binkp_log(1, "Session OK: %s", data ? data : "");
    s->state = STATE_AUTH_OK;
    s->md5_ok = (data && strstr(data, "secure")) ? 1 : 0;
    return 0;
}

/* M_EOB — remote has no more files */
int binkp_handle_eob(BinkpSession *s)
{
    binkp_log(1, "Remote sent EOB");
    s->rx_eob = 1;

    if (s->tx_eob) {
        s->state = STATE_DONE;
        binkp_log(1, "Both sides EOB — session complete");
    }
    return 0;
}

/* M_GOT — remote got our file OK */
int binkp_handle_got(BinkpSession *s, const char *data)
{
    binkp_log(1, "Remote GOT: %s", data);
    s->files_sent++;

    /* Close outbound file if open */
    if (s->out_file.fp) {
        fclose(s->out_file.fp);
        s->out_file.fp = NULL;
        bso_mark_sent(s, s->out_file.name);
    }

    return 0;
}

/* M_ERR — fatal error from remote */
int binkp_handle_err(BinkpSession *s, const char *data)
{
    binkp_log(0, "REMOTE ERROR: %s", data);
    s->state = STATE_ERROR;
    return -1;
}

/* ====================================================================
 * Receive file data (data frames during STATE_TRANSFER)
 * ==================================================================== */

static int handle_data_frame(BinkpSession *s, const unsigned char *data, int len)
{
    long written;

    if (!s->in_file.fp) {
        /* Data frame but no file open — protocol error */
        binkp_log(0, "WARNING: data frame with no file open, ignoring");
        return 0;
    }

    written = (long)fwrite(data, 1, len, s->in_file.fp);
    if (written != len) {
        binkp_log(0, "ERROR: write failed for %s", s->in_file.name);
        return -1;
    }

    s->bytes_rcvd += len;
    s->in_file.offset += len;

    /* Check if file is complete */
    if (s->in_file.offset >= s->in_file.size) {
        char got_str[512];
        char src_path[520], dst_path[520];

        fclose(s->in_file.fp);
        s->in_file.fp = NULL;
        s->files_rcvd++;

        binkp_log(1, "Received: %s (%ld bytes)", s->in_file.name, s->in_file.size);

        /* Send M_GOT */
        snprintf(got_str, sizeof(got_str), "%s %ld %ld",
                 s->in_file.name, s->in_file.size, s->in_file.time);
        binkp_send_cmd(s, M_GOT, got_str);

        /* Move from temp to final inbound */
        snprintf(src_path, sizeof(src_path), "%s%s",
                 s->temp_inbound, s->in_file.name);
        snprintf(dst_path, sizeof(dst_path), "%s%s",
                 s->inbound, s->in_file.name);
        rename(src_path, dst_path);
    }

    return 0;
}

/* ====================================================================
 * Send next outbound file
 * ==================================================================== */

static int send_next_file(BinkpSession *s)
{
    unsigned char buf[4096];
    int n;
    char file_hdr[512];

    /* If we already sent EOB, nothing to do */
    if (s->tx_eob)
        return 0;

    /* If no file open, try to get next one from BSO */
    if (!s->out_file.fp) {
        if (bso_next_file(s) < 0) {
            /* No more files — send EOB */
            binkp_send_cmd(s, M_EOB, "");
            s->tx_eob = 1;
            binkp_log(1, "Sent EOB — no more files");
            if (s->rx_eob) {
                s->state = STATE_DONE;
                binkp_log(1, "Both sides EOB — session complete");
            }
            return 0;
        }

        /* Send M_FILE header */
        snprintf(file_hdr, sizeof(file_hdr), "%s %ld %ld %ld",
                 s->out_file.name, s->out_file.size,
                 s->out_file.time, s->out_file.offset);
        binkp_send_cmd(s, M_FILE, file_hdr);
        binkp_log(1, "Sending: %s (%ld bytes)", s->out_file.name, s->out_file.size);
    }

    /* Send data frames */
    while (s->out_file.fp) {
        n = (int)fread(buf, 1, sizeof(buf), s->out_file.fp);
        if (n <= 0) {
            /* EOF — file sent, wait for M_GOT */
            fclose(s->out_file.fp);
            s->out_file.fp = NULL;
            binkp_log(2, "File data sent, waiting for M_GOT");
            break;
        }

        if (binkp_send_data(s, buf, n) < 0)
            return -1;

        s->bytes_sent += n;
        s->out_file.offset += n;
    }

    return 0;
}

/* ====================================================================
 * Session Init
 * ==================================================================== */

int binkp_init(BinkpSession *s, int sock, int direction)
{
    memset(s, 0, sizeof(BinkpSession));
    s->sock      = sock;
    s->direction = direction;
    s->state     = STATE_INIT;
    s->isize     = -1;
    s->md5_ok    = 0;
    return 0;
}

/* ====================================================================
 * Main Session Loop
 * ==================================================================== */

/* Send our M_NUL info lines and M_ADR */
static int send_init(BinkpSession *s)
{
    char buf[512];
    char addr_str[40];
    char challenge_nul[128];

    /* System info */
    binkp_send_cmd(s, M_NUL, "SYS PCBoard 15.4 BBS");
    binkp_send_cmd(s, M_NUL, "ZYZ Sysop");
    binkp_send_cmd(s, M_NUL, "VER pcbbinkp/1.0 binkp/1.1");
    binkp_send_cmd(s, M_NUL, "TIME ");  /* TODO: fill in time */

    snprintf(buf, sizeof(buf), "LOC Unknown");
    binkp_send_cmd(s, M_NUL, buf);

    /* If answering, generate CRAM-MD5 challenge */
    if (s->direction == BINKP_ANSWER && strlen(s->password) > 0) {
        binkp_make_challenge(s->challenge, sizeof(s->challenge));
        snprintf(challenge_nul, sizeof(challenge_nul),
                 "OPT CRAM-MD5-%s", s->challenge);
        binkp_send_cmd(s, M_NUL, challenge_nul);
    }

    /* Send our address */
    ftn_format(&s->local_addr, addr_str, sizeof(addr_str));
    binkp_send_cmd(s, M_ADR, addr_str);

    s->state = STATE_WAIT_ADR;
    return 0;
}

int binkp_run(BinkpSession *s)
{
    unsigned char frame_data[BINKP_MAX_BLOCK + 1];
    int is_cmd, cmd, len;
    int rc;
    fd_set rset;
    struct timeval tv;
    time_t session_start;
    int timeout_secs = 300;     /* 5 minute session timeout */

    binkp_log(1, "Session started (%s)",
              s->direction == BINKP_ORIGINATE ? "originate" : "answer");

    /* Send init */
    if (send_init(s) < 0)
        return -1;

    session_start = time(NULL);

    /* Main loop */
    while (s->state != STATE_DONE && s->state != STATE_ERROR) {

        /* Check session timeout */
        if (time(NULL) - session_start > timeout_secs) {
            binkp_log(0, "Session timeout (%d seconds)", timeout_secs);
            binkp_send_cmd(s, M_ERR, "Timeout");
            s->state = STATE_ERROR;
            break;
        }

        /* If authenticated and have files to send, send them */
        if (s->state == STATE_AUTH_OK || s->state == STATE_TRANSFER) {
            s->state = STATE_TRANSFER;
            send_next_file(s);
        }

        /* Wait for incoming data with 30s select timeout */
        FD_ZERO(&rset);
        FD_SET(s->sock, &rset);
        tv.tv_sec = 30;
        tv.tv_usec = 0;

        rc = select(s->sock + 1, &rset, NULL, NULL, &tv);
        if (rc < 0) {
            binkp_log(0, "select() error");
            s->state = STATE_ERROR;
            break;
        }
        if (rc == 0)
            continue;   /* select timeout, loop back to check session timeout */

        /* Read a frame */
        rc = binkp_recv_frame(s, &is_cmd, &cmd, frame_data, &len);
        if (rc < 0) {
            binkp_log(0, "Connection lost");
            s->state = STATE_ERROR;
            break;
        }

        if (is_cmd) {
            /* Dispatch command */
            switch (cmd) {
            case M_NUL:
                binkp_handle_nul(s, (char *)frame_data);
                break;
            case M_ADR:
                binkp_handle_adr(s, (char *)frame_data);
                break;
            case M_PWD:
                binkp_handle_pwd(s, (char *)frame_data);
                break;
            case M_FILE:
                binkp_handle_file(s, (char *)frame_data);
                break;
            case M_OK:
                binkp_handle_ok(s, (char *)frame_data);
                break;
            case M_EOB:
                binkp_handle_eob(s);
                break;
            case M_GOT:
                binkp_handle_got(s, (char *)frame_data);
                break;
            case M_ERR:
                binkp_handle_err(s, (char *)frame_data);
                break;
            case M_BSY:
                binkp_log(1, "Remote BUSY: %s", (char *)frame_data);
                s->state = STATE_ERROR;
                break;
            case M_GET:
                binkp_log(2, "M_GET: %s (not implemented)", (char *)frame_data);
                break;
            case M_SKIP:
                binkp_log(1, "Remote SKIP: %s", (char *)frame_data);
                if (s->out_file.fp) {
                    fclose(s->out_file.fp);
                    s->out_file.fp = NULL;
                }
                break;
            default:
                binkp_log(1, "Unknown command %d", cmd);
                break;
            }
        } else {
            /* Data frame — file content */
            handle_data_frame(s, frame_data, len);
        }

        /* After AUTH_OK and PWD sent (originate), send password */
        if (s->state == STATE_WAIT_OK && s->direction == BINKP_ORIGINATE &&
            s->remote_addr.zone != 0) {
            /* We got their ADR, now send password */
            if (s->options & OPT_CRAM) {
                /* Build CRAM-MD5 response */
                char digest[128];
                char pwd_str[140];
                if (binkp_build_digest(s->password, s->challenge,
                                       digest, sizeof(digest)) == 0) {
                    snprintf(pwd_str, sizeof(pwd_str), "CRAM-MD5-%s", digest);
                    binkp_send_cmd(s, M_PWD, pwd_str);
                    binkp_log(1, "Sent CRAM-MD5 password");
                } else {
                    binkp_send_cmd(s, M_PWD,
                                   strlen(s->password) ? s->password : "-");
                    binkp_log(1, "Sent plain password (MD5 failed)");
                }
            } else {
                binkp_send_cmd(s, M_PWD,
                               strlen(s->password) ? s->password : "-");
                binkp_log(1, "Sent plain password");
            }
        }
    }

    /* Session summary */
    binkp_log(1, "Session %s: sent %d file(s) / %ld bytes, "
              "rcvd %d file(s) / %ld bytes",
              s->state == STATE_DONE ? "complete" : "FAILED",
              s->files_sent, s->bytes_sent,
              s->files_rcvd, s->bytes_rcvd);

    return (s->state == STATE_DONE) ? 0 : -1;
}

void binkp_cleanup(BinkpSession *s)
{
    if (s->in_file.fp)  { fclose(s->in_file.fp);  s->in_file.fp = NULL; }
    if (s->out_file.fp) { fclose(s->out_file.fp); s->out_file.fp = NULL; }
}
