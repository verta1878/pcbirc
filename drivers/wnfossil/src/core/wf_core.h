/* ====================================================================
 * wf_core.h — WinFOSSIL Core Engine (shared by all platforms)
 * ====================================================================
 * Single source for Win95/98 VxD, NT4/2000 VDD, Win7-Win11 native.
 * Adapted from netmodem2irc GPLv3 FOSSIL engine.
 *
 * Platform wrappers #include this and provide:
 *   wfp_com_open/close/read/write/status  (COM port backend)
 *   wfp_tcp_connect/accept/read/write     (TCP backend)
 *   wfp_reg_read/write                    (Registry backend)
 *   wfp_thread_create/destroy             (Thread backend)
 *   wfp_cs_init/enter/leave               (CriticalSection)
 *   wfp_sleep_ms                          (Timer)
 *   wfp_log                               (Debug output)
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifndef WF_CORE_H
#define WF_CORE_H

#include <stdint.h>
#include <string.h>

/* ---- Constants ---- */

#define WF_VERSION_MAJ    2
#define WF_VERSION_MIN    0
#define WF_VERSION_STR    "2.0.0"
#define WF_SIGNATURE      0x1954    /* FOSSIL init return value      */
#define WF_MAX_PORTS      4
#define WF_MAX_FN         0x1B
#define WF_SPEC_REV       5         /* FTS-0017 revision 5           */
#define WF_BUF_SIZE       4096
#define WF_PORT_NAME_LEN  16
#define WF_DIAL_ADDR_LEN  256
#define WF_CMD_BUF_LEN    256
#define WF_ID_STRING      "WinFOSSIL v2.0.0 (GPLv3)"

/* ---- Registry paths per platform ---- */

/* Win95/98 (v1.12) — VxD services key */
#define WF_REG_95     "System\\CurrentControlSet\\Services\\VxD\\FOSSIL"

/* NT4/2000 (v1.0) — Woodruff vendor key */
#define WF_REG_NT     "Software\\Woodruff\\WinFOSSIL"

/* Win7-Win11 (v2.0) — Modern unified key */
#define WF_REG_MODERN "SOFTWARE\\WinFOSSIL"

/* Uninstall keys */
#define WF_UNREG_95   "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Fossil"
#define WF_UNREG_NT   "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WinFOSSIL"
#define WF_UNREG_MOD  "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\WinFOSSIL"

/* Registry value names (same across all versions) */
#define WF_RV_ENABLED       "enabled"
#define WF_RV_PORT_NAME     "port name"
#define WF_RV_LOCKED_BAUD   "locked baud"
#define WF_RV_RXBUF_SIZE    "receive buffer size"
#define WF_RV_TXBUF_SIZE    "transmit buffer size"
#define WF_RV_LAST_PORT     "last port selected"

/* v1.12 VxD-specific values */
#define WF_RV_STATIC_VXD    "StaticVxD"
#define WF_RV_START          "Start"

/* v1.0 NT-specific values */
#define WF_RV_INSTALLED     "Installed"

/* v2.0 Modern-specific values */
#define WF_RV_AUTO_OPEN     "AutoOpen"
#define WF_RV_KEEP_OPEN     "KeepOpen"
#define WF_RV_TIMESLICE     "TimeSlice"
#define WF_RV_PERF_STATS    "PerfStats"

/* ---- Access Security (v2.0 new feature) ---- */

/* Access control — who can open/configure FOSSIL ports.
 * Original WinFOSSIL had no access control. v2.0 adds:
 *   - Per-port access lists (user/group SIDs)
 *   - Admin-only configuration changes
 *   - Logging of port open/close events
 *   - TCP listen whitelist (IP ranges)
 *   - Max connections per port
 * Registry: HKLM\SOFTWARE\WinFOSSIL\Security */

#define WF_REG_SECURITY     "SOFTWARE\\WinFOSSIL\\Security"
#define WF_RV_REQUIRE_ADMIN "RequireAdmin"
#define WF_RV_LOG_ACCESS    "LogAccess"
#define WF_RV_MAX_CONN      "MaxConnections"
#define WF_RV_TCP_WHITELIST "TCPWhitelist"
#define WF_RV_TCP_BLACKLIST "TCPBlacklist"
#define WF_SEC_MAX_ACL      32
#define WF_SEC_MAX_IPRANGE  16

/* ---- Port mode ---- */

#define WF_MODE_NONE      0         /* Port not open                 */
#define WF_MODE_COM       1         /* Real COM hardware             */
#define WF_MODE_TCP       2         /* TCP/Telnet (VMODEM)           */
#define WF_MODE_PIPE      3         /* Named pipe                    */

/* ---- FOSSIL status bits (returned by Fn 03h in AX) ---- */

#define WF_ST_RDA         0x0100    /* Receive data available        */
#define WF_ST_OVRN        0x0200    /* Overrun error                 */
#define WF_ST_PRTY        0x0400    /* Parity error                  */
#define WF_ST_FRME        0x0800    /* Framing error                 */
#define WF_ST_BREAK       0x1000    /* Break detected                */
#define WF_ST_THRE        0x2000    /* TX holding register empty     */
#define WF_ST_TSRE        0x4000    /* TX shift register empty       */
#define WF_ST_TMOUT       0x8000    /* Timeout                       */
#define WF_ST_DCTS        0x0001    /* Delta CTS                     */
#define WF_ST_DDSR        0x0002    /* Delta DSR                     */
#define WF_ST_TERI        0x0004    /* Trailing edge ring            */
#define WF_ST_DDCD        0x0008    /* Delta DCD                     */
#define WF_ST_CTS         0x0010    /* CTS asserted                  */
#define WF_ST_DSR         0x0020    /* DSR asserted                  */
#define WF_ST_RI          0x0040    /* Ring indicator                */
#define WF_ST_DCD         0x0080    /* Data carrier detect           */

/* ---- Baud rate codes (Fn 00h, bits 7-5 of AL) ---- */

#define WF_BAUD_300       0x40
#define WF_BAUD_600       0x60
#define WF_BAUD_1200      0x80
#define WF_BAUD_2400      0xA0
#define WF_BAUD_4800      0xC0
#define WF_BAUD_9600      0xE0
#define WF_BAUD_19200     0x00
#define WF_BAUD_38400     0x20

/* ---- Flow control (Fn 0Fh) ---- */

#define WF_FLOW_XON       0x01      /* XON/XOFF software             */
#define WF_FLOW_CTS       0x02      /* RTS/CTS hardware              */

/* ---- VMODEM states ---- */

#define WF_VM_COMMAND     0         /* Waiting for AT commands       */
#define WF_VM_ONLINE      1         /* Data mode (connected)         */
#define WF_VM_DIALING     2         /* Resolving / connecting        */
#define WF_VM_RINGING     3         /* Incoming connection           */
#define WF_VM_HANGUP      4         /* Disconnecting                 */
#define WF_VM_ESCAPE      5         /* +++ guard time                */

/* ---- VMODEM result codes ---- */

#define WF_RC_OK          0
#define WF_RC_CONNECT     1
#define WF_RC_RING        2
#define WF_RC_NOCARRIER   3
#define WF_RC_ERROR       4
#define WF_RC_NODIALTONE  6
#define WF_RC_BUSY        7

/* ---- Circular buffer ---- */

typedef struct {
    uint8_t  data[WF_BUF_SIZE];
    volatile int head;
    volatile int tail;
} WfRingBuf;

/* ---- Port configuration (persisted to registry) ---- */

typedef struct {
    int      enabled;               /* Port active                   */
    char     name[WF_PORT_NAME_LEN];/* "COM1", "COM2", etc.          */
    uint32_t baud;                  /* Baud rate                     */
    int      locked;                /* Baud locked (no auto change)  */
    int      rx_buf_size;           /* Receive buffer size           */
    int      tx_buf_size;           /* Transmit buffer size          */
    int      auto_open;             /* Open on first access          */
    int      keep_open;             /* Keep open between sessions    */
    int      timeslice;             /* Yield CPU when idle           */
    int      perf_stats;            /* Performance monitoring        */
} WfPortConfig;

/* ---- Access security config ---- */

typedef struct {
    int      require_admin;         /* Config changes need admin     */
    int      log_access;            /* Log open/close to event log   */
    int      max_connections;       /* Max TCP connections per port   */
    char     tcp_whitelist[WF_SEC_MAX_IPRANGE][20]; /* Allowed IPs   */
    int      whitelist_count;
    char     tcp_blacklist[WF_SEC_MAX_IPRANGE][20]; /* Blocked IPs   */
    int      blacklist_count;
} WfSecurityConfig;

/* ---- Port state (runtime) ---- */

typedef struct WfPort {
    /* Identity */
    int           index;            /* 0-based port number           */
    int           mode;             /* WF_MODE_*                     */
    int           active;           /* FOSSIL initialized            */
    WfPortConfig  cfg;              /* Persisted config              */

    /* Platform handle (opaque — set by wfp_* functions) */
    void         *hCom;             /* COM port handle               */
    void         *hSock;            /* TCP socket handle             */
    void         *hListen;          /* Listen socket handle          */
    void         *hReadThread;      /* Background read thread        */
    void         *hWriteThread;     /* Background write thread       */
    void         *cs;               /* Critical section              */
    void         *hReadEvent;       /* Read completion event         */
    void         *hWriteEvent;      /* Write completion event        */

    /* Ring buffers */
    WfRingBuf     rxbuf;            /* Receive buffer                */
    WfRingBuf     txbuf;            /* Transmit buffer               */

    /* DTR / flow / break state */
    int           dtr_on;
    int           rts_on;
    int           flow_xon;
    int           flow_cts;
    int           break_on;
    int           etx_enabled;      /* Ctrl-C/K interception         */
    int           watchdog;         /* Fn 14h: carrier-loss watchdog */

    /* Telnet IAC state — persistent across buffer boundaries.
     * WF-15: dedicated fields replace etx_enabled/break_on hacks. */
    int           tn_in_subneg;     /* Inside IAC SB ... IAC SE      */
    int           tn_pending_iac;   /* Buffer ended with lone 0xFF   */
    uint8_t       tn_pending_cmd;   /* Pending WILL/WONT/DO/DONT cmd */
    int           tn_subneg_iac;    /* 0xFF seen inside subneg (WF-13) */
    uint8_t       tn_accepted[32];  /* Bitmask: options we've accepted (WF-14) */

    /* VMODEM state machine */
    int           vm_state;         /* WF_VM_*                       */
    int           vm_online;        /* Connected flag                */
    int           vm_echo;          /* Local echo on/off             */
    int           vm_verbose;       /* ATV: 1=verbose text, 0=numeric*/
    int           vm_quiet;         /* ATQ: 1=suppress result codes  */
    int           vm_auto_answer;   /* ATS0 register                 */
    char          vm_dial_addr[WF_DIAL_ADDR_LEN];
    int           vm_dial_port;     /* TCP port                      */
    char          vm_cmd_buf[WF_CMD_BUF_LEN];
    int           vm_cmd_len;       /* Command accumulator position  */
    uint32_t      vm_escape_tick;   /* +++ guard timer               */
    int           vm_escape_count;  /* Consecutive + count           */

    /* Performance counters */
    uint32_t      perf_rx_bytes;
    uint32_t      perf_tx_bytes;
    uint32_t      perf_rx_timeouts;
    uint32_t      perf_tx_timeouts;
    uint32_t      perf_vm_wakeups;
    uint32_t      perf_cps_rx;
    uint32_t      perf_cps_tx;
    uint32_t      perf_peak_rx;
    uint32_t      perf_peak_tx;
    uint32_t      perf_last_rx;
    uint32_t      perf_last_tx;
    uint32_t      perf_last_tick;
    uint32_t      perf_connect_tick;
    uint32_t      perf_connect_secs;

    /* Security */
    WfSecurityConfig security;

    /* Link (for linked list — VxD uses this) */
    struct WfPort *next;
} WfPort;

/* ---- FOSSIL info block (returned by Fn 1Bh) ---- */

#pragma pack(push, 1)
typedef struct {
    uint16_t size;                  /* Structure size (19 bytes)     */
    uint8_t  spec_rev;              /* FOSSIL spec version (5)       */
    uint8_t  driver_rev;            /* Driver revision               */
    uint32_t id_string;             /* Far/flat ptr to ID string     */
    uint16_t rx_buf_size;           /* Receive buffer size           */
    uint16_t rx_buf_free;           /* Receive bytes free            */
    uint16_t tx_buf_size;           /* Transmit buffer size          */
    uint16_t tx_buf_free;           /* Transmit bytes free           */
    uint8_t  screen_w;              /* Screen width                  */
    uint8_t  screen_h;              /* Screen height                 */
    uint8_t  baud_mask;             /* Supported baud rates          */
} WfFossilInfo;
#pragma pack(pop)

/* ---- Extended baud rate table (original _gadwBaudRateEx) ---- */

static const uint32_t wf_baud_table[] = {
    300, 600, 1200, 2400, 4800, 9600, 19200, 38400,
    57600, 115200, 230400, 460800, 0
};

/* ---- Parity table (original _abParity) ---- */

static const uint8_t wf_parity_table[] = {
    0, /* None */  1, /* Odd */  2, /* Even */  3, /* Mark */  4  /* Space */
};

/* ================================================================
 * CORE API — called by platform wrappers
 * ================================================================ */

/* Ring buffer operations */
int   wf_buf_count(const WfRingBuf *b);
int   wf_buf_free(const WfRingBuf *b);
void  wf_buf_put(WfRingBuf *b, uint8_t ch);
int   wf_buf_get(WfRingBuf *b);
int   wf_buf_peek(const WfRingBuf *b);
void  wf_buf_clear(WfRingBuf *b);

/* Port lifecycle */
int   wf_init(WfPort *p, int index);
void  wf_deinit(WfPort *p);

/* FOSSIL API (Fn 00h-1Bh) */
void  wf_set_params(WfPort *p, uint8_t params);
void  wf_set_params_ex(WfPort *p, uint32_t baud, uint8_t parity,
                        uint8_t databits, uint8_t stopbits);
int   wf_send_wait(WfPort *p, uint8_t ch);
int   wf_send_nowait(WfPort *p, uint8_t ch);
int   wf_recv_wait(WfPort *p);
int   wf_peek(WfPort *p);
uint16_t wf_status(WfPort *p);
void  wf_set_dtr(WfPort *p, int on);
void  wf_set_flow(WfPort *p, int flags);
void  wf_set_break(WfPort *p, int on);
void  wf_flush(WfPort *p);
void  wf_purge_rx(WfPort *p);
void  wf_purge_tx(WfPort *p);
int   wf_read_block(WfPort *p, void *buf, int len);
int   wf_write_block(WfPort *p, const void *buf, int len);
void  wf_etx_handler(WfPort *p, int flags);
void  wf_get_info(WfPort *p, WfFossilInfo *info);

/* VMODEM engine */
void  wf_vm_init(WfPort *p);
void  wf_vm_engine(WfPort *p);
int   wf_vm_parse_cmd(WfPort *p);
int   wf_vm_dial(WfPort *p, const char *address);
void  wf_vm_hangup(WfPort *p);
void  wf_vm_connect_msg(WfPort *p);
void  wf_vm_send_result(WfPort *p, int code);
void  wf_vm_echo(WfPort *p, uint8_t ch);
void  wf_vm_stuff_rx(WfPort *p, const void *buf, int len);
void  wf_vm_filter_telnet(WfPort *p, uint8_t *buf, int *len);
void  wf_vm_set_state(WfPort *p, int state);
int   wf_vm_wait_state(WfPort *p, int state, int timeout_ms);

/* Performance monitoring */
void  wf_perf_update(WfPort *p, uint32_t now_tick);
void  wf_perf_reset(WfPort *p, uint32_t now_tick);

/* Access security */
int   wf_sec_check_ip(WfPort *p, const char *ip);
void  wf_sec_log(WfPort *p, const char *action, const char *detail);
int   wf_sec_load(WfPort *p);
int   wf_sec_save(WfPort *p);

/* Registry path helper — returns correct path for current platform */
const char *wf_reg_path(void);

/* Port open helpers */
int   wf_open_com(WfPort *p);
int   wf_open_vmodem(WfPort *p);

/* Port enumeration (implemented in comport_compat.c) */
int   wf_enum_ports(char ports[][WF_PORT_NAME_LEN], int max_ports);

/* Baud rate decode */
uint32_t wf_decode_baud(uint8_t code);
uint8_t  wf_encode_baud(uint32_t baud);

/* ================================================================
 * PLATFORM CALLBACKS — implemented by each wrapper
 * ================================================================
 * These are the ONLY functions that differ per platform.
 * Everything above is shared code.
 * ================================================================ */

/* COM port backend */
int   wfp_com_open(WfPort *p, const char *name, uint32_t baud);
void  wfp_com_close(WfPort *p);
int   wfp_com_read(WfPort *p, void *buf, int len);
int   wfp_com_write(WfPort *p, const void *buf, int len);
int   wfp_com_status(WfPort *p);
void  wfp_com_set_baud(WfPort *p, uint32_t baud);
void  wfp_com_set_dtr(WfPort *p, int on);
void  wfp_com_set_rts(WfPort *p, int on);
void  wfp_com_set_break(WfPort *p, int on);
void  wfp_com_set_flow(WfPort *p, int xon, int cts);
void  wfp_com_flush(WfPort *p);
void  wfp_com_purge_rx(WfPort *p);
void  wfp_com_purge_tx(WfPort *p);
void  wfp_com_setup_buffers(WfPort *p, int rx_size, int tx_size);

/* TCP backend */
int   wfp_tcp_connect(WfPort *p, const char *host, int port);
int   wfp_tcp_listen(WfPort *p, int port);
int   wfp_tcp_accept(WfPort *p);
void  wfp_tcp_close(WfPort *p);
int   wfp_tcp_read(WfPort *p, void *buf, int len);
int   wfp_tcp_write(WfPort *p, const void *buf, int len);
int   wfp_tcp_data_ready(WfPort *p);

/* Registry backend */
int   wfp_reg_read_port(int port_index, WfPortConfig *cfg);
int   wfp_reg_write_port(int port_index, const WfPortConfig *cfg);
int   wfp_reg_read_global(const char *name, uint32_t *val);
int   wfp_reg_write_global(const char *name, uint32_t val);

/* Thread backend */
void *wfp_thread_create(void (*func)(void *), void *arg);
void  wfp_thread_destroy(void *handle);
void *wfp_cs_create(void);
void  wfp_cs_destroy(void *cs);
void  wfp_cs_enter(void *cs);
void  wfp_cs_leave(void *cs);
void *wfp_event_create(void);
void  wfp_event_set(void *event);
void  wfp_event_wait(void *event, int timeout_ms);
void  wfp_event_destroy(void *event);

/* Timer */
uint32_t wfp_tick_ms(void);        /* Millisecond counter           */
void  wfp_sleep_ms(int ms);

/* Registry — platform-specific path */
const char *wfp_reg_base_key(void);

/* Debug */
void  wfp_log(const char *fmt, ...);

#endif /* WF_CORE_H */
