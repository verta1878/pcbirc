/* ====================================================================
 * wf_core.c — WinFOSSIL Core Engine Implementation
 * ====================================================================
 * Shared by all 3 platform builds. No platform-specific code here.
 * All platform calls go through wfp_* callbacks in wf_core.h.
 *
 * Adapted from netmodem2irc GPLv3 FOSSIL engine.
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#include "wf_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ================================================================
 * RING BUFFER
 * ================================================================ */

int wf_buf_count(const WfRingBuf *b)
{
    int n = b->head - b->tail;
    if (n < 0) n += WF_BUF_SIZE;
    return n;
}

int wf_buf_free(const WfRingBuf *b)
{
    return WF_BUF_SIZE - 1 - wf_buf_count(b);
}

void wf_buf_put(WfRingBuf *b, uint8_t ch)
{
    int next = (b->head + 1) % WF_BUF_SIZE;
    if (next == b->tail) return;    /* Full — drop byte              */
    b->data[b->head] = ch;
    b->head = next;
}

int wf_buf_get(WfRingBuf *b)
{
    if (b->tail == b->head) return -1;
    uint8_t ch = b->data[b->tail];
    b->tail = (b->tail + 1) % WF_BUF_SIZE;
    return ch;
}

int wf_buf_peek(const WfRingBuf *b)
{
    if (b->tail == b->head) return -1;
    return b->data[b->tail];
}

void wf_buf_clear(WfRingBuf *b)
{
    b->head = b->tail = 0;
}


/* ================================================================
 * BAUD RATE ENCODE / DECODE
 * ================================================================ */

uint32_t wf_decode_baud(uint8_t code)
{
    switch (code & 0xE0) {
    case 0x00: return 19200;
    case 0x20: return 38400;
    case 0x40: return 300;
    case 0x60: return 600;
    case 0x80: return 1200;
    case 0xA0: return 2400;
    case 0xC0: return 4800;
    case 0xE0: return 9600;
    }
    return 9600;
}

uint8_t wf_encode_baud(uint32_t baud)
{
    if (baud <= 300)   return 0x43;
    if (baud <= 600)   return 0x63;
    if (baud <= 1200)  return 0x83;
    if (baud <= 2400)  return 0xA3;
    if (baud <= 4800)  return 0xC3;
    if (baud <= 9600)  return 0xE3;
    if (baud <= 19200) return 0x03;
    return 0x23; /* 38400+ */
}


/* ================================================================
 * PORT LIFECYCLE
 * ================================================================ */

int wf_init(WfPort *p, int index)
{
    memset(p, 0, sizeof(WfPort));
    p->index = index;
    p->mode = WF_MODE_NONE;
    p->active = 1;
    p->dtr_on = 1;
    p->rts_on = 1;
    p->cfg.baud = 9600;
    p->cfg.rx_buf_size = WF_BUF_SIZE;
    p->cfg.tx_buf_size = WF_BUF_SIZE;
    p->cfg.auto_open = 1;
    p->cfg.timeslice = 1;
    p->vm_state = WF_VM_COMMAND;
    p->vm_dial_port = 23;

    /* Create synchronization primitives */
    p->cs = wfp_cs_create();
    p->hReadEvent = wfp_event_create();
    p->hWriteEvent = wfp_event_create();

    wf_buf_clear(&p->rxbuf);
    wf_buf_clear(&p->txbuf);

    /* Load config from registry */
    wfp_reg_read_port(index, &p->cfg);

    return WF_SIGNATURE;
}

void wf_deinit(WfPort *p)
{
    /* Signal threads to stop FIRST — they check p->active in their loops.
     * BUG FIX: original destroyed handles before signaling, causing hangs. */
    p->active = 0;

    /* Wake threads so they see active=0 and exit */
    if (p->hReadEvent)  wfp_event_set(p->hReadEvent);
    if (p->hWriteEvent) wfp_event_set(p->hWriteEvent);

    /* Wait for threads to finish, then destroy handles */
    if (p->hReadThread)  wfp_thread_destroy(p->hReadThread);
    if (p->hWriteThread) wfp_thread_destroy(p->hWriteThread);
    p->hReadThread = p->hWriteThread = NULL;

    /* Close transport */
    if (p->mode == WF_MODE_COM)
        wfp_com_close(p);
    else if (p->mode == WF_MODE_TCP)
        wfp_tcp_close(p);

    /* Destroy sync */
    if (p->hReadEvent)  wfp_event_destroy(p->hReadEvent);
    if (p->hWriteEvent) wfp_event_destroy(p->hWriteEvent);
    if (p->cs)          wfp_cs_destroy(p->cs);

    p->active = 0;
    p->mode = WF_MODE_NONE;
}


/* ================================================================
 * BACKGROUND THREADS
 * ================================================================
 * Read thread: pulls bytes from COM/TCP into rxbuf.
 * Write thread: pushes bytes from txbuf to COM/TCP.
 * Original WinFOSSIL used these (commReadThread/commWriteThread).
 * ================================================================ */

static void wf_read_thread(void *arg)
{
    WfPort *p = (WfPort *)arg;
    uint8_t buf[256];
    int n, i;

    while (p->active) {
        n = 0;
        if (p->mode == WF_MODE_COM)
            n = wfp_com_read(p, buf, sizeof(buf));
        else if (p->mode == WF_MODE_TCP)
            n = wfp_tcp_read(p, buf, sizeof(buf));

        if (n > 0) {
            /* VMODEM telnet IAC filtering */
            if (p->mode == WF_MODE_TCP && p->vm_online)
                wf_vm_filter_telnet(p, buf, &n);

            wfp_cs_enter(p->cs);
            for (i = 0; i < n; i++) {
                wf_buf_put(&p->rxbuf, buf[i]);
                p->perf_rx_bytes++;
            }
            wfp_cs_leave(p->cs);
            wfp_event_set(p->hReadEvent);
        } else if (n < 0) {
            /* Connection lost — log and exit thread cleanly */
            wfp_log("Port %d: read thread got error (n=%d), stopping", p->index, n);
            if (p->mode == WF_MODE_TCP) {
                p->vm_online = 0;
                wf_vm_set_state(p, WF_VM_HANGUP);
            }
            break;
        } else {
            /* No data — yield */
            if (p->cfg.timeslice)
                wfp_sleep_ms(1);
            else
                wfp_sleep_ms(0);
        }
    }
}

static void wf_write_thread(void *arg)
{
    WfPort *p = (WfPort *)arg;
    uint8_t buf[256];
    int n;

    while (p->active) {
        n = 0;
        wfp_cs_enter(p->cs);
        while (n < (int)sizeof(buf) && wf_buf_count(&p->txbuf) > 0) {
            int ch = wf_buf_get(&p->txbuf);
            if (ch >= 0) buf[n++] = (uint8_t)ch;
        }
        wfp_cs_leave(p->cs);

        if (n > 0) {
            int written = 0;
            if (p->mode == WF_MODE_COM)
                written = wfp_com_write(p, buf, n);
            else if (p->mode == WF_MODE_TCP)
                written = wfp_tcp_write(p, buf, n);

            if (written > 0)
                p->perf_tx_bytes += written;
            if (written < 0 && p->mode == WF_MODE_TCP) {
                p->vm_online = 0;
                break;
            }
        } else {
            /* Nothing to write — wait for signal or timeout */
            wfp_event_wait(p->hWriteEvent, 100);
        }
    }
}


/* ================================================================
 * FOSSIL API FUNCTIONS (Fn 00h - 1Bh)
 * ================================================================ */

/* Fn 00h: Set baud/parity/data/stop from FOSSIL parameter byte */
void wf_set_params(WfPort *p, uint8_t params)
{
    uint32_t baud;
    uint8_t parity, databits, stopbits;

    if (p->cfg.locked) return;      /* Baud locked — ignore          */

    baud = wf_decode_baud(params);
    parity   = (params >> 3) & 0x03;
    databits = (params & 0x03) + 5; /* 00=5, 01=6, 10=7, 11=8       */
    stopbits = (params & 0x04) ? 2 : 1;

    p->cfg.baud = baud;

    if (p->mode == WF_MODE_COM)
        wfp_com_set_baud(p, baud);

    wfp_log("Port %d: baud=%lu parity=%d data=%d stop=%d",
            p->index, (unsigned long)baud, parity, databits, stopbits);
}

/* Fn 00h extended: Set explicit baud/parity/data/stop */
void wf_set_params_ex(WfPort *p, uint32_t baud, uint8_t parity,
                       uint8_t databits, uint8_t stopbits)
{
    (void)parity; (void)databits; (void)stopbits;
    if (p->cfg.locked) return;
    p->cfg.baud = baud;
    if (p->mode == WF_MODE_COM)
        wfp_com_set_baud(p, baud);
}

/* Fn 01h: Send character, wait until sent */
int wf_send_wait(WfPort *p, uint8_t ch)
{
    wfp_cs_enter(p->cs);
    wf_buf_put(&p->txbuf, ch);
    wfp_cs_leave(p->cs);
    wfp_event_set(p->hWriteEvent);  /* Wake write thread             */
    return 1;
}

/* Fn 0Bh: Send character, no wait (returns 0 if buffer full) */
int wf_send_nowait(WfPort *p, uint8_t ch)
{
    int ok;
    wfp_cs_enter(p->cs);
    ok = (wf_buf_free(&p->txbuf) > 0);
    if (ok) wf_buf_put(&p->txbuf, ch);
    wfp_cs_leave(p->cs);
    if (ok) wfp_event_set(p->hWriteEvent);
    return ok;
}

/* Fn 02h: Receive character, wait until available */
int wf_recv_wait(WfPort *p)
{
    int ch;
    int retries = 0;

    while (retries < 1000) {        /* ~1 second timeout             */
        wfp_cs_enter(p->cs);
        ch = wf_buf_get(&p->rxbuf);
        wfp_cs_leave(p->cs);
        if (ch >= 0) return ch;
        wfp_event_wait(p->hReadEvent, 1);
        retries++;
    }

    p->perf_rx_timeouts++;
    return -1;
}

/* Fn 0Ch: Peek at next byte without consuming */
int wf_peek(WfPort *p)
{
    int ch;
    wfp_cs_enter(p->cs);
    ch = wf_buf_peek(&p->rxbuf);
    wfp_cs_leave(p->cs);
    return ch;
}

/* Fn 03h: Status request */
uint16_t wf_status(WfPort *p)
{
    uint16_t st = 0;
    int com_st;

    /* TX always ready (buffered) */
    st |= WF_ST_THRE | WF_ST_TSRE;

    /* RX data available */
    wfp_cs_enter(p->cs);
    if (wf_buf_count(&p->rxbuf) > 0)
        st |= WF_ST_RDA;
    wfp_cs_leave(p->cs);

    /* Modem status from hardware or TCP */
    if (p->mode == WF_MODE_COM) {
        com_st = wfp_com_status(p);
        st |= (com_st & 0x00FF);   /* Low byte = modem status       */
    } else if (p->mode == WF_MODE_TCP) {
        if (p->vm_online)
            st |= WF_ST_DCD | WF_ST_CTS | WF_ST_DSR;
    }

    return st;
}

/* Fn 06h: DTR control (0=drop, 1=raise) */
void wf_set_dtr(WfPort *p, int on)
{
    p->dtr_on = on;
    if (p->mode == WF_MODE_COM)
        wfp_com_set_dtr(p, on);
    else if (p->mode == WF_MODE_TCP && !on)
        wf_vm_hangup(p);            /* DTR drop = disconnect         */
}

/* Fn 0Fh: Flow control */
void wf_set_flow(WfPort *p, int flags)
{
    p->flow_xon = (flags & WF_FLOW_XON) ? 1 : 0;
    p->flow_cts = (flags & WF_FLOW_CTS) ? 1 : 0;
    if (p->mode == WF_MODE_COM)
        wfp_com_set_flow(p, p->flow_xon, p->flow_cts);
}

/* Fn 1Ah: Break signal */
void wf_set_break(WfPort *p, int on)
{
    p->break_on = on;
    if (p->mode == WF_MODE_COM)
        wfp_com_set_break(p, on);
}

/* Fn 08h: Flush output buffer */
void wf_flush(WfPort *p)
{
    /* Wait for txbuf to drain, then flush hardware */
    int retries = 0;
    while (wf_buf_count(&p->txbuf) > 0 && retries < 500) {
        wfp_sleep_ms(10);
        retries++;
    }
    if (p->mode == WF_MODE_COM)
        wfp_com_flush(p);
}

/* Fn 09h: Purge output buffer */
void wf_purge_tx(WfPort *p)
{
    wfp_cs_enter(p->cs);
    wf_buf_clear(&p->txbuf);
    wfp_cs_leave(p->cs);
    if (p->mode == WF_MODE_COM)
        wfp_com_purge_tx(p);
}

/* Fn 0Ah: Purge input buffer */
void wf_purge_rx(WfPort *p)
{
    wfp_cs_enter(p->cs);
    wf_buf_clear(&p->rxbuf);
    wfp_cs_leave(p->cs);
    if (p->mode == WF_MODE_COM)
        wfp_com_purge_rx(p);
}

/* Fn 10h: ETX handler (Ctrl-C/K interception) */
void wf_etx_handler(WfPort *p, int flags)
{
    p->etx_enabled = flags;
}

/* Fn 18h: Read block */
int wf_read_block(WfPort *p, void *buf, int len)
{
    uint8_t *dst = (uint8_t *)buf;
    int count = 0;

    wfp_cs_enter(p->cs);
    while (count < len && wf_buf_count(&p->rxbuf) > 0) {
        int ch = wf_buf_get(&p->rxbuf);
        if (ch >= 0) dst[count++] = (uint8_t)ch;
    }
    wfp_cs_leave(p->cs);

    return count;
}

/* Fn 19h: Write block */
int wf_write_block(WfPort *p, const void *buf, int len)
{
    const uint8_t *src = (const uint8_t *)buf;
    int count = 0;

    wfp_cs_enter(p->cs);
    while (count < len && wf_buf_free(&p->txbuf) > 0) {
        wf_buf_put(&p->txbuf, src[count++]);
    }
    wfp_cs_leave(p->cs);

    if (count > 0)
        wfp_event_set(p->hWriteEvent);

    return count;
}

/* Fn 1Bh: Get FOSSIL driver info */
void wf_get_info(WfPort *p, WfFossilInfo *info)
{
    memset(info, 0, sizeof(WfFossilInfo));
    info->size       = sizeof(WfFossilInfo);
    info->spec_rev   = WF_SPEC_REV;
    info->driver_rev = (WF_VERSION_MAJ << 4) | WF_VERSION_MIN;
    info->id_string  = 0;          /* Platform sets this             */
    info->rx_buf_size = WF_BUF_SIZE;
    info->tx_buf_size = WF_BUF_SIZE;

    wfp_cs_enter(p->cs);
    info->rx_buf_free = (uint16_t)wf_buf_free(&p->rxbuf);
    info->tx_buf_free = (uint16_t)wf_buf_free(&p->txbuf);
    wfp_cs_leave(p->cs);

    info->screen_w = 80;
    info->screen_h = 25;
    info->baud_mask = 0xFF;
}


/* ================================================================
 * VMODEM AT COMMAND ENGINE
 * ================================================================
 * Full AT command parser state machine.
 * Matches original WinFOSSIL behavior.
 *
 * Commands:
 *   ATZ           Reset (close, clear buffers)
 *   ATD<addr>     Dial (TCP connect to host:port)
 *   ATDT<addr>    Same (tone dial = TCP)
 *   ATDP<addr>    Same (pulse dial = TCP)
 *   ATH           Hangup (close TCP)
 *   ATH0          Same
 *   ATE0          Echo off
 *   ATE1          Echo on
 *   ATS0=0        Auto-answer off
 *   ATS0=1        Auto-answer on (listen port 23)
 *   ATS0=N        Auto-answer on port N
 *   AT&D0         Ignore DTR drop
 *   AT&D2         Hangup on DTR drop
 *   ATA           Manual answer (accept connection)
 *   ATI           Identification
 *   ATM0/ATM1     Speaker (no-op)
 *   ATL0/ATL1     Volume (no-op)
 *   +++           Escape to command mode (1s guard)
 *
 * Result codes:
 *   OK            Command accepted
 *   CONNECT baud  TCP connected
 *   RING          Incoming TCP
 *   NO CARRIER    Connection lost / failed
 *   ERROR         Bad command
 *   NO DIALTONE   DNS failure
 *   BUSY          Port in use
 * ================================================================ */

static const char *wf_result_strings[] = {
    "OK\r\n",           /* 0 */
    "CONNECT",          /* 1 — baud appended */
    "RING\r\n",         /* 2 */
    "NO CARRIER\r\n",   /* 3 */
    "ERROR\r\n",        /* 4 */
    "",                 /* 5 — unused */
    "NO DIALTONE\r\n",  /* 6 */
    "BUSY\r\n",         /* 7 */
};

/* Numeric result codes (ATV0). Standard Hayes numbering. */
static const char *wf_result_numeric[] = {
    "0\r",              /* 0 = OK */
    "1\r",              /* 1 = CONNECT (bare, or use 5/10/etc for speed) */
    "2\r",              /* 2 = RING */
    "3\r",              /* 3 = NO CARRIER */
    "4\r",              /* 4 = ERROR */
    "",                 /* 5 — unused */
    "6\r",              /* 6 = NO DIALTONE */
    "7\r",              /* 7 = BUSY */
};

void wf_vm_init(WfPort *p)
{
    p->vm_state = WF_VM_COMMAND;
    p->vm_online = 0;
    p->vm_echo = 1;
    p->vm_verbose = 1;              /* ATV1 — verbose text by default */
    p->vm_quiet = 0;               /* ATQ0 — result codes enabled    */
    p->vm_auto_answer = 0;
    p->vm_cmd_len = 0;
    p->vm_escape_count = 0;
    p->vm_dial_port = 23;
}

void wf_vm_set_state(WfPort *p, int state)
{
    p->vm_state = state;
    wfp_log("Port %d: VMODEM state → %d", p->index, state);
}

void wf_vm_send_result(WfPort *p, int code)
{
    char buf[64];

    /* ATQ1 — suppress all result codes */
    if (p->vm_quiet)
        return;

    if (code == WF_RC_CONNECT) {
        if (p->vm_verbose) {
            snprintf(buf, sizeof(buf), "CONNECT %lu\r\n",
                     (unsigned long)p->cfg.baud);
        } else {
            /* ATV0 numeric: CONNECT speed codes. 1=CONNECT (300),
             * higher codes map to speeds, but bare "1" is universally
             * accepted. Use "1" for compatibility. */
            snprintf(buf, sizeof(buf), "1\r");
        }
        wf_vm_stuff_rx(p, buf, (int)strlen(buf));
    } else if (code >= 0 && code <= WF_RC_BUSY) {
        const char *s = p->vm_verbose ?
            wf_result_strings[code] : wf_result_numeric[code];
        wf_vm_stuff_rx(p, s, (int)strlen(s));
    }
}

void wf_vm_echo(WfPort *p, uint8_t ch)
{
    if (p->vm_echo)
        wf_vm_stuff_rx(p, &ch, 1);
}

void wf_vm_stuff_rx(WfPort *p, const void *buf, int len)
{
    const uint8_t *src = (const uint8_t *)buf;
    int i;
    wfp_cs_enter(p->cs);
    for (i = 0; i < len; i++)
        wf_buf_put(&p->rxbuf, src[i]);
    wfp_cs_leave(p->cs);
}

void wf_vm_connect_msg(WfPort *p)
{
    wf_vm_send_result(p, WF_RC_CONNECT);
    wf_vm_set_state(p, WF_VM_ONLINE);
    p->vm_online = 1;
    p->perf_connect_tick = wfp_tick_ms();
}

void wf_vm_hangup(WfPort *p)
{
    if (p->mode == WF_MODE_TCP)
        wfp_tcp_close(p);
    p->vm_online = 0;
    wf_vm_set_state(p, WF_VM_COMMAND);
    wf_vm_send_result(p, WF_RC_NOCARRIER);
    p->perf_connect_secs += (wfp_tick_ms() - p->perf_connect_tick) / 1000;

    /* Clear telnet state for next connection */
    p->tn_in_subneg = 0;
    p->tn_pending_iac = 0;
    p->tn_pending_cmd = 0;
    p->tn_subneg_iac = 0;
    memset(p->tn_accepted, 0, sizeof(p->tn_accepted));
}

int wf_vm_dial(WfPort *p, const char *address)
{
    char host[WF_DIAL_ADDR_LEN];
    int port = 23;
    char *colon;

    strncpy(host, address, WF_DIAL_ADDR_LEN - 1);
    host[WF_DIAL_ADDR_LEN - 1] = '\0';

    /* Strip leading spaces */
    char *h = host;
    while (*h == ' ') h++;

    colon = strrchr(h, ':');
    if (colon) {
        *colon = '\0';
        port = atoi(colon + 1);
        if (port <= 0 || port > 65535) port = 23;
    }

    strncpy(p->vm_dial_addr, h, WF_DIAL_ADDR_LEN - 1);
    p->vm_dial_addr[WF_DIAL_ADDR_LEN - 1] = '\0';
    p->vm_dial_port = port;

    wf_vm_set_state(p, WF_VM_DIALING);
    wfp_log("Port %d: VMODEM dialing %s:%d", p->index, h, port);

    if (wfp_tcp_connect(p, h, port) == 0) {
        p->mode = WF_MODE_TCP;

        /* Start background threads for TCP */
        if (!p->hReadThread)
            p->hReadThread = wfp_thread_create(wf_read_thread, p);
        if (!p->hWriteThread)
            p->hWriteThread = wfp_thread_create(wf_write_thread, p);

        wf_vm_connect_msg(p);
        return 0;
    }

    /* DNS or connect failure */
    wf_vm_set_state(p, WF_VM_COMMAND);
    wf_vm_send_result(p, WF_RC_NODIALTONE);
    return -1;
}

/* Parse AT command string accumulated in vm_cmd_buf */
int wf_vm_parse_cmd(WfPort *p)
{
    char *cmd = p->vm_cmd_buf;
    int len = p->vm_cmd_len;
    int pos = 0;

    /* Null-terminate */
    if (len >= WF_CMD_BUF_LEN) len = WF_CMD_BUF_LEN - 1;
    cmd[len] = '\0';

    /* A/ = repeat last command (Hayes standard, MF-3) */
    static char last_cmd[WF_CMD_BUF_LEN];
    static int last_len = 0;

    if (len >= 2 && toupper(cmd[0]) == 'A' && cmd[1] == '/') {
        if (last_len > 0) {
            memcpy(p->vm_cmd_buf, last_cmd, last_len);
            p->vm_cmd_len = last_len;
            cmd = p->vm_cmd_buf;
            len = p->vm_cmd_len;
        } else {
            wf_vm_send_result(p, WF_RC_OK);
            return 0;
        }
    }

    /* Must start with AT */
    if (len < 2) return -1;
    if (toupper(cmd[0]) != 'A' || toupper(cmd[1]) != 'T')
        return -1;

    /* Save for A/ repeat */
    memcpy(last_cmd, cmd, len < WF_CMD_BUF_LEN ? len : WF_CMD_BUF_LEN - 1);
    last_len = len;

    pos = 2;

    /* Empty AT = just OK */
    if (pos >= len) {
        wf_vm_send_result(p, WF_RC_OK);
        return 0;
    }

    /* Parse commands (can be chained: ATE1V1H0) */
    while (pos < len) {
        char c = toupper(cmd[pos++]);

        switch (c) {
        case 'Z':                   /* ATZ — Reset                   */
            wf_vm_hangup(p);
            wf_buf_clear(&p->rxbuf);
            wf_buf_clear(&p->txbuf);
            wf_vm_send_result(p, WF_RC_OK);
            return 0;

        case 'D':                   /* ATD — Dial                    */
        {
            /* Skip T/P prefix (tone/pulse — both map to TCP) */
            if (pos < len && (toupper(cmd[pos]) == 'T' ||
                              toupper(cmd[pos]) == 'P'))
                pos++;
            /* Rest of string is the address */
            return wf_vm_dial(p, cmd + pos);
        }

        case 'H':                   /* ATH — Hangup                  */
            if (pos < len && cmd[pos] == '0') pos++;
            if (p->vm_online)
                wf_vm_hangup(p);
            else
                wf_vm_send_result(p, WF_RC_OK);
            return 0;

        case 'A':                   /* ATA — Answer                  */
            if (p->vm_state == WF_VM_RINGING) {
                if (wfp_tcp_accept(p) == 0) {
                    if (!p->hReadThread)
                        p->hReadThread = wfp_thread_create(wf_read_thread, p);
                    if (!p->hWriteThread)
                        p->hWriteThread = wfp_thread_create(wf_write_thread, p);
                    wf_vm_connect_msg(p);
                }
            }
            return 0;

        case 'E':                   /* ATE — Echo control            */
            if (pos < len) {
                p->vm_echo = (cmd[pos] != '0');
                pos++;
            }
            break;

        case 'S':                   /* ATS — Register access         */
        {
            int reg = 0;
            while (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9')
                reg = reg * 10 + (cmd[pos++] - '0');
            if (pos < len && cmd[pos] == '=') {
                pos++;
                int val = 0;
                while (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9')
                    val = val * 10 + (cmd[pos++] - '0');
                if (reg == 0) {
                    p->vm_auto_answer = val;
                    if (val > 0) {
                        /* Start listening */
                        int lport = (val > 1) ? val : 23;
                        wfp_tcp_listen(p, lport);
                        p->mode = WF_MODE_TCP;
                    }
                }
            }
            break;
        }

        case '&':                   /* AT& — Extended commands       */
            if (pos < len) {
                char ext = toupper(cmd[pos++]);
                if (ext == 'D') {
                    /* AT&D0 = ignore DTR, AT&D2 = hangup on DTR */
                    if (pos < len) pos++;
                } else if (ext == 'F') {
                    /* AT&F — Factory defaults (MF-4) */
                    p->vm_echo = 1;
                    p->vm_auto_answer = 0;
                    p->cfg.baud = 9600;
                    p->flow_xon = 0;
                    p->flow_cts = 0;
                }
            }
            break;

        case 'I':                   /* ATI — Identification          */
        {
            char id[64];
            snprintf(id, sizeof(id), "WinFOSSIL v%s\r\n", WF_VERSION_STR);
            wf_vm_stuff_rx(p, id, (int)strlen(id));
            break;
        }

        case 'M': case 'L':        /* ATM/ATL — Speaker (no-op)     */
            if (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9') pos++;
            break;

        case 'V':                   /* ATV — Verbose/numeric results */
            if (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9') {
                p->vm_verbose = (cmd[pos] != '0');
                pos++;
            } else {
                p->vm_verbose = 1;  /* ATV alone = ATV1 */
            }
            break;

        case 'Q':                   /* ATQ — Quiet (suppress results)*/
            if (pos < len && cmd[pos] >= '0' && cmd[pos] <= '9') {
                p->vm_quiet = (cmd[pos] != '0');
                pos++;
            } else {
                p->vm_quiet = 0;    /* ATQ alone = ATQ0 */
            }
            break;

        case ' ':                   /* Skip spaces                   */
            break;

        default:
            wf_vm_send_result(p, WF_RC_ERROR);
            return -1;
        }
    }

    wf_vm_send_result(p, WF_RC_OK);
    return 0;
}

/* Main VMODEM engine — called when data is written to port in command mode */
void wf_vm_engine(WfPort *p)
{
    int ch;

    if (p->vm_state == WF_VM_ONLINE) {
        /* +++ escape detection: 3 consecutive '+' with 1-second guard time.
         * Guard: 1s silence before +++, 1s silence after.
         * Original WinFOSSIL and Hayes standard. */
        uint32_t now = wfp_tick_ms();

        /* Drain txbuf into a local send buffer while holding CS,
         * then send OUTSIDE the lock to avoid blocking (WF-9 fix). */
        {
            uint8_t sendbuf[512];
            int sendlen = 0;
            int escaped = 0;

            wfp_cs_enter(p->cs);
            while (wf_buf_count(&p->txbuf) > 0 && sendlen < (int)sizeof(sendbuf) - 4) {
                int ch = wf_buf_get(&p->txbuf);
                if (ch < 0) break;

                if (ch == '+') {
                    if (p->vm_escape_count == 0)
                        p->vm_escape_tick = now;
                    p->vm_escape_count++;
                    if (p->vm_escape_count >= 3 &&
                        (now - p->vm_escape_tick) >= 1000) {
                        escaped = 1;
                        break;
                    }
                } else {
                    /* Flush accumulated +'s then this byte */
                    while (p->vm_escape_count > 0) {
                        sendbuf[sendlen++] = '+';
                        p->vm_escape_count--;
                    }
                    sendbuf[sendlen++] = (uint8_t)ch;
                }
            }
            wfp_cs_leave(p->cs);

            /* Send outside CS — safe to block here (WF-9) */
            if (sendlen > 0 && p->mode == WF_MODE_TCP) {
                wfp_tcp_write(p, sendbuf, sendlen);
                p->perf_tx_bytes += sendlen;
            }

            if (escaped) {
                wf_vm_set_state(p, WF_VM_COMMAND);
                wf_vm_send_result(p, WF_RC_OK);
                wfp_log("Port %d: +++ escape to command mode", p->index);
                return;
            }

            /* WF-10 fix: if we have 1-2 pending + chars and >1s since last +,
             * they're not part of an escape — flush them to TCP. */
            if (p->vm_escape_count > 0 && p->vm_escape_count < 3 &&
                (now - p->vm_escape_tick) >= 1000) {
                uint8_t plusbuf[3];
                int j;
                for (j = 0; j < p->vm_escape_count; j++)
                    plusbuf[j] = '+';
                if (p->mode == WF_MODE_TCP) {
                    wfp_tcp_write(p, plusbuf, p->vm_escape_count);
                    p->perf_tx_bytes += p->vm_escape_count;
                }
                p->vm_escape_count = 0;
            }
        }
        return;
    }

    if (p->vm_state == WF_VM_RINGING && p->vm_auto_answer > 0) {
        /* Auto-answer */
        if (wfp_tcp_accept(p) == 0) {
            if (!p->hReadThread)
                p->hReadThread = wfp_thread_create(wf_read_thread, p);
            if (!p->hWriteThread)
                p->hWriteThread = wfp_thread_create(wf_write_thread, p);
            wf_vm_connect_msg(p);
        }
        return;
    }

    /* Command mode: accumulate characters until CR */
    wfp_cs_enter(p->cs);
    while (wf_buf_count(&p->txbuf) > 0) {
        ch = wf_buf_get(&p->txbuf);
        if (ch < 0) break;

        wf_vm_echo(p, (uint8_t)ch);

        if (ch == '\r' || ch == '\n') {
            wfp_cs_leave(p->cs);
            wf_vm_parse_cmd(p);
            p->vm_cmd_len = 0;
            return;
        }
        if (ch == '\b' || ch == 127) {
            if (p->vm_cmd_len > 0) p->vm_cmd_len--;
        } else if (p->vm_cmd_len < WF_CMD_BUF_LEN - 1) {
            p->vm_cmd_buf[p->vm_cmd_len++] = (char)ch;
        }
    }
    wfp_cs_leave(p->cs);
}

/* Filter telnet IAC sequences from data stream */
void wf_vm_filter_telnet(WfPort *p, uint8_t *buf, int *len)
{
    int src, dst;

    /* Telnet IAC (0xFF) filter with full persistent state (WF-15).
     * All state lives in WfPort->tn_* fields — no more etx_enabled/
     * break_on hacks. Handles:
     *   0xFF 0xFF          → single 0xFF (escaped)
     *   0xFF FB/FC/FD/FE   → WILL/WONT/DO/DONT + 1 option byte
     *   0xFF FA ... FF F0  → Subnegotiation (variable length)
     *   0xFF xx            → 2-byte command (skip both)
     * Accepts SGA (3) and BINARY (0), refuses everything else (MF-5).
     * Tracks accepted options to prevent echo-loops (WF-14).
     * Handles IAC SE split across buffers inside subneg (WF-13). */

    dst = 0;
    for (src = 0; src < *len; src++) {

        /* ---- Inside subnegotiation: skip until IAC SE ---- */
        if (p->tn_in_subneg) {
            if (p->tn_subneg_iac) {
                /* Previous byte was 0xFF inside subneg (WF-13).
                 * This byte determines what it was. */
                p->tn_subneg_iac = 0;
                if (buf[src] == 0xF0) {
                    /* IAC SE — subneg complete */
                    p->tn_in_subneg = 0;
                } else if (buf[src] == 0xFF) {
                    /* Escaped 0xFF inside subneg — skip, stay in subneg */
                }
                /* Any other byte after IAC inside subneg: protocol error,
                 * but stay in subneg per RFC 854 robustness */
                continue;
            }
            if (buf[src] == 0xFF) {
                /* Could be start of IAC SE or escaped 0xFF.
                 * If more data follows, check next byte now.
                 * If this is the last byte, save state (WF-13). */
                if (src + 1 < *len) {
                    if (buf[src + 1] == 0xF0) {
                        src++;  /* Skip SE byte */
                        p->tn_in_subneg = 0;
                    } else if (buf[src + 1] == 0xFF) {
                        src++;  /* Escaped 0xFF inside subneg — skip both */
                    }
                    /* Else: protocol error, skip the IAC, stay in subneg */
                } else {
                    /* Buffer ends with 0xFF inside subneg — save for next read */
                    p->tn_subneg_iac = 1;
                }
            }
            continue;
        }

        /* ---- Continuation of partial IAC from previous buffer (WF-8) ---- */
        if (p->tn_pending_iac) {
            if (p->tn_pending_cmd == 0) {
                /* Had lone 0xFF at end of last buffer — this is the command */
                uint8_t cmd = buf[src];
                if (cmd == 0xFF) {
                    buf[dst++] = 0xFF;  /* Escaped 0xFF */
                    p->tn_pending_iac = 0;
                    continue;
                }
                if (cmd >= 0xFB && cmd <= 0xFE) {
                    /* Need one more byte (option) */
                    p->tn_pending_cmd = cmd;
                    continue;
                }
                if (cmd == 0xFA) {
                    p->tn_in_subneg = 1;
                    p->tn_pending_iac = 0;
                    continue;
                }
                /* Other 2-byte command — skip */
                p->tn_pending_iac = 0;
                continue;
            } else {
                /* Had 0xFF + cmd at end of last buffer — this is the option.
                 * tn_pending_cmd holds the cmd byte, buf[src] is the option. */
                goto handle_option;
            handle_option_done:
                p->tn_pending_iac = 0;
                p->tn_pending_cmd = 0;
                continue;
            }
        }

        /* ---- Normal byte processing ---- */
        if (buf[src] == 0xFF) {
            if (src + 1 >= *len) {
                /* Lone 0xFF at buffer end — save for next read (WF-8) */
                p->tn_pending_iac = 1;
                p->tn_pending_cmd = 0;
                continue;
            }

            {
                uint8_t cmd = buf[src + 1];

                if (cmd == 0xFF) {
                    /* Escaped 0xFF → pass one through */
                    buf[dst++] = 0xFF;
                    src++;
                    continue;
                }

                if (cmd >= 0xFB && cmd <= 0xFE) {
                    if (src + 2 >= *len) {
                        /* Command byte present but option byte missing —
                         * save partial state for next buffer (WF-8) */
                        p->tn_pending_iac = 1;
                        p->tn_pending_cmd = cmd;
                        src++;
                        continue;
                    }
                    {
                        p->tn_pending_cmd = cmd;  /* For shared handler */
                        src += 2;
                        goto handle_option;
                    }
                }

                if (cmd == 0xFA) {
                    /* Subnegotiation: skip until IAC SE (0xFF 0xF0).
                     * If SE not in this buffer, tn_in_subneg persists (WF-3). */
                    src += 2;
                    p->tn_in_subneg = 1;
                    while (src < *len) {
                        if (buf[src] == 0xFF) {
                            if (src + 1 < *len) {
                                if (buf[src + 1] == 0xF0) {
                                    src++;  /* Skip SE */
                                    p->tn_in_subneg = 0;
                                    break;
                                } else if (buf[src + 1] == 0xFF) {
                                    src++;  /* Escaped 0xFF in subneg */
                                }
                            } else {
                                /* Buffer ends with 0xFF inside subneg (WF-13) */
                                p->tn_subneg_iac = 1;
                            }
                        }
                        src++;
                    }
                    continue;
                }

                /* Other 2-byte commands: skip both */
                src++;
                continue;
            }
        }

        buf[dst++] = buf[src];
        continue;

    handle_option:
        /* Shared handler for WILL/WONT/DO/DONT + option byte.
         * Precondition: tn_pending_cmd holds the command byte (FB-FE),
         *               buf[src] holds the option byte.
         * WF-14: track accepted options to prevent echo-loops. */
        {
            uint8_t opt = buf[src];
            uint8_t cmd_byte = p->tn_pending_cmd;
            uint8_t resp[3] = { 0xFF, 0, opt };
            int already_accepted = (opt < 32) ?
                (p->tn_accepted[opt] != 0) : 0;

            if (cmd_byte == 0xFB) {
                /* Remote says WILL */
                if (opt == 3 || opt == 0) {
                    if (!already_accepted) {
                        resp[1] = 0xFD;  /* DO (accept) */
                        if (opt < 32) p->tn_accepted[opt] = 1;
                    }
                    /* WF-14: already accepted — don't re-send DO */
                } else {
                    resp[1] = 0xFE;  /* DONT (refuse) */
                }
            } else if (cmd_byte == 0xFC) {
                /* Remote says WONT — acknowledge, clear accepted */
                if (opt < 32) p->tn_accepted[opt] = 0;
                /* No response needed for WONT */
            } else if (cmd_byte == 0xFD) {
                /* Remote says DO */
                if (opt == 3 || opt == 0) {
                    if (!already_accepted) {
                        resp[1] = 0xFB;  /* WILL (accept) */
                        if (opt < 32) p->tn_accepted[opt] = 1;
                    }
                    /* WF-14: already accepted — don't re-send WILL */
                } else {
                    resp[1] = 0xFC;  /* WONT (refuse) */
                }
            } else if (cmd_byte == 0xFE) {
                /* Remote says DONT — acknowledge, clear accepted */
                if (opt < 32) p->tn_accepted[opt] = 0;
                /* No response needed for DONT */
            }

            if (resp[1] && p->mode == WF_MODE_TCP)
                wfp_tcp_write(p, resp, 3);

            p->tn_pending_iac = 0;
            p->tn_pending_cmd = 0;
            goto handle_option_done;
        }
    }
    *len = dst;
}

int wf_vm_wait_state(WfPort *p, int state, int timeout_ms)
{
    uint32_t start = wfp_tick_ms();
    while (p->vm_state != state) {
        if ((int)(wfp_tick_ms() - start) >= timeout_ms) return -1;
        wfp_sleep_ms(10);
    }
    return 0;
}


/* ================================================================
 * PERFORMANCE MONITORING
 * ================================================================ */

void wf_perf_update(WfPort *p, uint32_t now_tick)
{
    uint32_t elapsed;

    if (!p->cfg.perf_stats) return;

    /* Guard against GetTickCount wrap-around (~49 days).
     * If elapsed is negative (wrapped), reset the baseline. (WF-7 fix) */
    elapsed = now_tick - p->perf_last_tick;
    if (elapsed > 0x80000000U) {
        /* Tick wrapped — reset baseline, skip this interval */
        p->perf_last_tick = now_tick;
        p->perf_last_rx = p->perf_rx_bytes;
        p->perf_last_tx = p->perf_tx_bytes;
        return;
    }
    if (elapsed < 1000) return;

    p->perf_cps_rx = ((p->perf_rx_bytes - p->perf_last_rx) * 1000) / elapsed;
    p->perf_cps_tx = ((p->perf_tx_bytes - p->perf_last_tx) * 1000) / elapsed;

    if (p->perf_cps_rx > p->perf_peak_rx) p->perf_peak_rx = p->perf_cps_rx;
    if (p->perf_cps_tx > p->perf_peak_tx) p->perf_peak_tx = p->perf_cps_tx;

    if (p->vm_online)
        p->perf_connect_secs = (now_tick - p->perf_connect_tick) / 1000;

    p->perf_last_rx = p->perf_rx_bytes;
    p->perf_last_tx = p->perf_tx_bytes;
    p->perf_last_tick = now_tick;
}

void wf_perf_reset(WfPort *p, uint32_t now_tick)
{
    p->perf_rx_bytes = p->perf_tx_bytes = 0;
    p->perf_rx_timeouts = p->perf_tx_timeouts = 0;
    p->perf_vm_wakeups = 0;
    p->perf_cps_rx = p->perf_cps_tx = 0;
    p->perf_peak_rx = p->perf_peak_tx = 0;
    p->perf_last_rx = p->perf_last_tx = 0;
    p->perf_last_tick = now_tick;
    p->perf_connect_tick = now_tick;
    p->perf_connect_secs = 0;
}


/* ================================================================
 * PORT OPEN HELPERS
 * ================================================================ */

int wf_open_com(WfPort *p)
{
    char name[WF_PORT_NAME_LEN];

    /* Don't open twice — close first if already open */
    if (p->mode != WF_MODE_NONE) {
        wfp_log("Port %d: already open (mode %d), closing first", p->index, p->mode);
        if (p->mode == WF_MODE_COM) wfp_com_close(p);
        else if (p->mode == WF_MODE_TCP) wfp_tcp_close(p);
        p->mode = WF_MODE_NONE;
    }

    if (!p->cfg.name[0]) {
        snprintf(name, sizeof(name), "COM%d", p->index + 1);
    } else {
        strncpy(name, p->cfg.name, WF_PORT_NAME_LEN - 1);
        name[WF_PORT_NAME_LEN - 1] = '\0';
    }

    if (wfp_com_open(p, name, p->cfg.baud) != 0)
        return -1;

    p->mode = WF_MODE_COM;
    wfp_com_setup_buffers(p, p->cfg.rx_buf_size, p->cfg.tx_buf_size);
    wfp_com_set_dtr(p, 1);
    wfp_com_set_rts(p, 1);

    /* Start background I/O threads (WF-11: check if already running) */
    if (!p->hReadThread)
        p->hReadThread  = wfp_thread_create(wf_read_thread, p);
    if (!p->hWriteThread)
        p->hWriteThread = wfp_thread_create(wf_write_thread, p);

    wf_perf_reset(p, wfp_tick_ms());
    wfp_log("Port %d: COM opened (%s @ %lu baud)",
            p->index, name, (unsigned long)p->cfg.baud);
    return 0;
}

int wf_open_vmodem(WfPort *p)
{
    p->mode = WF_MODE_TCP;
    wf_vm_init(p);
    wf_perf_reset(p, wfp_tick_ms());
    wfp_log("Port %d: VMODEM initialized", p->index);
    return 0;
}


/* ================================================================
 * ACCESS SECURITY
 * ================================================================
 * New in v2.0. Original WinFOSSIL had no access control.
 *
 * Features:
 *   RequireAdmin    — Only admins can change port config
 *   LogAccess       — Log open/close/dial to Windows Event Log
 *   MaxConnections  — Limit concurrent TCP connections per port
 *   TCPWhitelist    — Only accept TCP from listed IP ranges
 *   TCPBlacklist    — Reject TCP from listed IP ranges
 *
 * Registry: HKLM\SOFTWARE\WinFOSSIL\Security
 * ================================================================ */

int wf_sec_check_ip(WfPort *p, const char *ip)
{
    int i;

    if (!ip || !ip[0]) return 1;    /* No IP = local, always allow   */

    /* Blacklist takes priority.
     * Match on octet boundary: prefix "10" must not match "100.x".
     * A prefix matches if the IP starts with it AND the next char
     * is '.' or end-of-string. (WF-5 fix) */
    for (i = 0; i < p->security.blacklist_count; i++) {
        int plen = (int)strlen(p->security.tcp_blacklist[i]);
        if (strncmp(ip, p->security.tcp_blacklist[i], plen) == 0 &&
            (ip[plen] == '.' || ip[plen] == '\0')) {
            wfp_log("Port %d: BLOCKED connection from %s (blacklisted)",
                    p->index, ip);
            return 0;
        }
    }

    /* If whitelist is empty, allow all */
    if (p->security.whitelist_count == 0) return 1;

    /* Check whitelist — octet-boundary match (WF-5 fix) */
    for (i = 0; i < p->security.whitelist_count; i++) {
        int plen = (int)strlen(p->security.tcp_whitelist[i]);
        if (strncmp(ip, p->security.tcp_whitelist[i], plen) == 0 &&
            (ip[plen] == '.' || ip[plen] == '\0')) {
            return 1;
        }
    }

    wfp_log("Port %d: BLOCKED connection from %s (not in whitelist)",
            p->index, ip);
    return 0;
}

void wf_sec_log(WfPort *p, const char *action, const char *detail)
{
    if (!p->security.log_access) return;
    wfp_log("SECURITY Port %d: %s — %s", p->index, action, detail);
    /* Platform wrapper can write to Windows Event Log via wfp_log */
}

int wf_sec_load(WfPort *p)
{
    uint32_t val;

    p->security.require_admin = 0;
    p->security.log_access = 0;
    p->security.max_connections = 4;
    p->security.whitelist_count = 0;
    p->security.blacklist_count = 0;

    if (wfp_reg_read_global(WF_RV_REQUIRE_ADMIN, &val) == 0)
        p->security.require_admin = val;
    if (wfp_reg_read_global(WF_RV_LOG_ACCESS, &val) == 0)
        p->security.log_access = val;
    if (wfp_reg_read_global(WF_RV_MAX_CONN, &val) == 0)
        p->security.max_connections = val;

    return 0;
}

int wf_sec_save(WfPort *p)
{
    wfp_reg_write_global(WF_RV_REQUIRE_ADMIN,
                         (uint32_t)p->security.require_admin);
    wfp_reg_write_global(WF_RV_LOG_ACCESS,
                         (uint32_t)p->security.log_access);
    wfp_reg_write_global(WF_RV_MAX_CONN,
                         (uint32_t)p->security.max_connections);
    return 0;
}


/* ================================================================
 * REGISTRY PATH HELPER
 * ================================================================ */

const char *wf_reg_path(void)
{
    return wfp_reg_base_key();
}
