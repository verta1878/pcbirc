/* ====================================================================
 * qfront.h — QFront FidoNet Mailer Orchestrator
 * ====================================================================
 * Clean-room implementation from FTS-5005 (BSO), FTS-0001 (.PKT),
 * and FTS-1026 (BinkP). No reverse engineering — all from public
 * FidoNet Technical Standards.
 *
 * License: GPLv3
 * ==================================================================== */

#ifndef QFRONT_H
#define QFRONT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* POSIX signals — SIGHUP/SIGPIPE may not exist on Win32 or OW */
#ifndef SIGHUP
#define SIGHUP  1
#endif
#ifndef SIGPIPE
#define SIGPIPE 13
#endif
#include <time.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <io.h>
#define PATH_SEP '\\'
#define mkdir(p,m) _mkdir(p)
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#define PATH_SEP '/'
#endif

/* ---- FidoNet 5D Address (Zone:Net/Node.Point@Domain) ---- */
typedef struct {
    uint16_t zone;
    uint16_t net;
    uint16_t node;
    uint16_t point;
    char     domain[32];          /* e.g. "fidonet"              */
} FTN_ADDR;

/* ---- BSO Flow File Flavour (FTS-5005 Section 3.2) ----
 * Controls when the mailer should poll the remote system.
 * Listed in priority order (immediate > continuous > ... > hold). */
typedef enum {
    BSO_IMMEDIATE  = 'i',         /* Ignore all restrictions     */
    BSO_CONTINUOUS = 'c',         /* Ignore external restrictions */
    BSO_DIRECT     = 'd',         /* Respect all restrictions    */
    BSO_NORMAL     = 'f',         /* Normal priority, may reroute */
    BSO_HOLD       = 'h'          /* Wait for remote to call us  */
} BsoFlavour;

/* ---- BSO Flow File Entry ----
 * One line from a .?lo file: a path with an optional directive. */
typedef struct {
    char     path[260];           /* File path to send           */
    char     directive;           /* '#'=truncate, '^'=delete,
                                   * '~'=skip, 0=send only       */
    int      sent;                /* 1 if successfully sent       */
} BsoFlowEntry;

/* ---- BSO Outbound Item ----
 * Represents all pending mail/files for one remote address. */
typedef struct {
    FTN_ADDR     addr;            /* Remote system address        */
    BsoFlavour   flavour;         /* Highest priority flavour     */
    int          has_netmail;     /* .?ut packet exists           */
    int          has_filelist;    /* .?lo file list exists        */
    int          has_request;     /* .req file exists             */
    char         basepath[260];   /* e.g. "outbound/00680024"    */
} BsoItem;

/* ---- Event Schedule Entry ---- */
typedef struct {
    int      day_mask;            /* Bit 0=Sun ... Bit 6=Sat     */
    int      start_hour;          /* 0-23                        */
    int      start_min;           /* 0-59                        */
    int      end_hour;
    int      end_min;
    int      flags;               /* EVENT_FORCE_POLL, etc.      */
    char     tag[16];             /* Event tag name               */
} QfEvent;

#define EVENT_FORCE_POLL   0x01   /* Force outbound poll          */
#define EVENT_NO_SEND      0x02   /* Don't send during this event */
#define EVENT_NO_RECEIVE   0x04   /* Don't receive                */
#define EVENT_MAIL_ONLY    0x08   /* Only process mail, no files  */
#define MAX_EVENTS         32

/* ---- QFront Configuration ---- */
typedef struct {
    /* Our addresses (up to 16 AKAs) */
    FTN_ADDR  aka[16];
    int       num_aka;

    /* Paths */
    char      outbound[260];      /* BSO outbound root           */
    char      inbound[260];       /* Inbound file directory      */
    char      temp_inbound[260];  /* Temp/unsecure inbound       */
    char      netmail_dir[260];   /* Netmail storage              */
    char      logfile[260];       /* Log file path                */

    /* External programs */
    char      binkd_path[260];    /* Path to binkd or pcbbinkp   */
    char      tosser_path[260];   /* Path to tosser (pcbtoss etc) */
    char      tic_proc[260];      /* Path to TIC processor        */

    /* Nodelist */
    char      nodelist_dir[260];  /* Directory with nodelist      */
    char      nodelist_base[32];  /* Base name (e.g. "NODELIST")  */

    /* Session */
    int       com_port;           /* COM port number (1-4)        */
    int       locked_baud;         /* DTE locked baud rate         */
    int       max_baud;           /* Max baud rate                */
    int       max_retries;        /* Max call attempts before hold */
    int       retry_delay;        /* Seconds between retries      */
    int       hold_time;          /* Seconds to hold after max retries */

    /* Events */
    QfEvent   events[MAX_EVENTS];
    int       num_events;

    /* Flags */
    int       debug;              /* Debug logging                */
} QfConfig;

/* ---- Logging ---- */
typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO  = 1,
    LOG_WARN  = 2,
    LOG_ERROR = 3,
    LOG_FATAL = 4
} LogLevel;

void qf_log(LogLevel level, const char *fmt, ...);
void qf_log_init(const char *logfile);
void qf_log_close(void);

/* ---- FTN Address Helpers ---- */
int  ftn_parse_addr(const char *str, FTN_ADDR *addr);
void ftn_format_addr(const FTN_ADDR *addr, char *buf, int bufsize);
int  ftn_addr_equal(const FTN_ADDR *a, const FTN_ADDR *b);

/* ---- BSO Scanner (FTS-5005) ---- */
int  bso_scan(const QfConfig *cfg, BsoItem *items, int max_items);
int  bso_create_poll(const QfConfig *cfg, const FTN_ADDR *addr,
                     BsoFlavour flavour);
int  bso_lock(const QfConfig *cfg, const FTN_ADDR *addr);
void bso_unlock(const QfConfig *cfg, const FTN_ADDR *addr);
int  bso_check_hold(const QfConfig *cfg, const FTN_ADDR *addr);
void bso_record_try(const QfConfig *cfg, const FTN_ADDR *addr,
                    int success, const char *msg);

/* ---- Session Dispatch ---- */
int  qf_call_node(const QfConfig *cfg, const BsoItem *item);
int  qf_post_session(const QfConfig *cfg, int success);

/* ---- Event Scheduler ---- */
int  qf_event_check(const QfConfig *cfg, const QfEvent **active);
int  qf_event_should_poll(const QfEvent *ev);

/* ---- Config ---- */
int  qf_config_load(const char *path, QfConfig *cfg);

/* ---- .PKT Header (FTS-0001) ---- */
#pragma pack(push, 1)
typedef struct {
    uint16_t orig_node;           /* Originating node number      */
    uint16_t dest_node;           /* Destination node number      */
    uint16_t year;                /* Year (e.g., 2026)            */
    uint16_t month;               /* Month (0-11)                 */
    uint16_t day;                 /* Day (1-31)                   */
    uint16_t hour;                /* Hour (0-23)                  */
    uint16_t minute;              /* Minute (0-59)                */
    uint16_t second;              /* Second (0-59)                */
    uint16_t baud;                /* Baud rate (0 for IP)         */
    uint16_t pkt_ver;             /* Packet version (always 2)    */
    uint16_t orig_net;            /* Originating net number       */
    uint16_t dest_net;            /* Destination net number       */
    uint8_t  product_lo;          /* Product code low byte        */
    uint8_t  revision_major;      /* Major revision               */
    char     password[8];         /* Session password              */
    uint16_t orig_zone;           /* Originating zone (QMail)     */
    uint16_t dest_zone;           /* Destination zone (QMail)     */
    uint16_t aux_net;             /* Aux net for Type 2+          */
    uint16_t cw_valid;            /* CW validation copy           */
    uint8_t  product_hi;          /* Product code high byte       */
    uint8_t  revision_minor;      /* Minor revision               */
    uint16_t cw;                  /* Capability word              */
    uint16_t orig_zone2;          /* Originating zone (Type 2+)   */
    uint16_t dest_zone2;          /* Destination zone (Type 2+)   */
    uint16_t orig_point;          /* Originating point            */
    uint16_t dest_point;          /* Destination point            */
    uint32_t prod_data;           /* Product-specific data        */
} PKT_HEADER;                     /* 58 bytes total               */
#pragma pack(pop)

#define PKT_VERSION 2
#define PKT_CW_VALID 0x0100       /* Capability word validation   */
#define PKT_CW_TYPE2PLUS 0x0001   /* Type 2+ capable              */

/* Wildcat! BBS compatibility files (read if present):
 *   MAKEWILD.DAT  — Wildcat! system configuration
 *   NODEINFO.DAT  — Node status information
 *   CONFDESC.DAT  — Conference descriptions
 *   ALLUSERS.DAT  — User database
 *   USERNET.XXX   — Inter-node messages
 *   BBSBATCH.BAT  — BBS loader batch file
 * These are Wildcat!-specific and not required for
 * standalone FidoNet operation. */

#endif /* QFRONT_H */
