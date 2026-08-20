/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* qfront.h -- QFront FidoNet Mailer Orchestrator                           */
/*                                                                           */
/* Clean-room implementation from FTS-5005 (BSO), FTS-0001 (.PKT),          */
/* and FTS-1026 (BinkP). No reverse engineering -- all from public           */
/* FidoNet Technical Standards.                                              */
/*                                                                           */
/* Default install path: C:\PCB\QFRONT (PCBoard 15.x)                       */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef QFRONT_H
#define QFRONT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* POSIX signals -- SIGHUP/SIGPIPE may not exist on Win32 or OW */
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


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     FidoNet 5D Address                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Zone:Net/Node.Point@Domain (FTS-0001) */

typedef struct {
    uint16_t zone;                      /* zone number (1-32767)         */
    uint16_t net;                       /* net number                    */
    uint16_t node;                      /* node number                   */
    uint16_t point;                     /* point number (0=boss node)    */
    char     domain[32];                /* e.g. "fidonet"                */
} FTN_ADDR;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                  BSO Flow File Types (FTS-5005)                           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Controls when the mailer should poll the remote system.
 * Listed in priority order (immediate > continuous > ... > hold). */

typedef enum {
    BSO_IMMEDIATE  = 'i',               /* ignore all restrictions       */
    BSO_CONTINUOUS = 'c',               /* ignore external restrictions  */
    BSO_DIRECT     = 'd',               /* respect all restrictions      */
    BSO_NORMAL     = 'f',               /* normal priority, may reroute  */
    BSO_HOLD       = 'h'                /* wait for remote to call us    */
} BsoFlavour;

/* One line from a .?lo file: a path with an optional directive. */
typedef struct {
    char     Path[260];                 /* file path to send             */
    char     Directive;                 /* '#'=truncate, '^'=delete,
                                         * '~'=skip, 0=send only        */
    int      Sent;                      /* 1 if successfully sent        */
} BsoFlowEntry;

/* Represents all pending mail/files for one remote address. */
typedef struct {
    FTN_ADDR     addr;                  /* remote system address         */
    BsoFlavour   flavour;               /* highest priority flavour      */
    int          has_netmail;           /* .?ut packet exists            */
    int          has_filelist;          /* .?lo file list exists         */
    int          has_request;           /* .req file exists              */
    char         basepath[260];         /* e.g. "outbound/00680024"      */
} BsoItem;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Event Schedule                                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    int      day_mask;                  /* bit 0=Sun ... bit 6=Sat       */
    int      start_hour;                /* 0-23                          */
    int      start_min;                 /* 0-59                          */
    int      end_hour;                  /* 0-23                          */
    int      end_min;                   /* 0-59                          */
    int      flags;                     /* EVENT_FORCE_POLL, etc.        */
    char     tag[16];                   /* event tag name                */
} QfEvent;

#define EVENT_FORCE_POLL   0x01         /* force outbound poll           */
#define EVENT_NO_SEND      0x02         /* don't send during this event  */
#define EVENT_NO_RECEIVE   0x04         /* don't receive                 */
#define EVENT_MAIL_ONLY    0x08         /* only process mail, no files   */
#define MAX_EVENTS         32           /* max event definitions         */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     QFront Configuration                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    /* Our addresses (up to 16 AKAs) */
    FTN_ADDR  aka[16];                  /* FidoNet AKA list              */
    int       num_aka;                  /* number of configured AKAs     */

    /* Paths */
    char      outbound[260];            /* BSO outbound root             */
    char      inbound[260];             /* inbound file directory        */
    char      temp_inbound[260];        /* temp/unsecure inbound         */
    char      netmail_dir[260];         /* netmail storage               */
    char      logfile[260];             /* log file path                 */

    /* External programs */
    char      binkd_path[260];          /* path to binkd or pcbbinkp     */
    char      tosser_path[260];         /* path to tosser (pcbtoss etc.) */
    char      tic_proc[260];            /* path to TIC processor         */

    /* Nodelist */
    char      nodelist_dir[260];        /* directory with nodelist       */
    char      nodelist_base[32];        /* base name (e.g. "NODELIST")   */

    /* Session */
    int       com_port;                 /* COM port number (1-4)         */
    int       locked_baud;              /* DTE locked baud rate          */
    int       max_baud;                 /* max baud rate                 */
    int       max_retries;              /* max call attempts before hold */
    int       retry_delay;              /* seconds between retries       */
    int       hold_time;                /* seconds to hold after max     */

    /* Events */
    QfEvent   events[MAX_EVENTS];       /* event schedule                */
    int       num_events;               /* number of configured events   */

    /* Flags */
    int       debug;                    /* debug logging                 */
} QfConfig;


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                            Logging                                        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef enum {
    LOG_DEBUG = 0,                      /* verbose debug output          */
    LOG_INFO  = 1,                      /* normal informational          */
    LOG_WARN  = 2,                      /* warning (non-fatal)           */
    LOG_ERROR = 3,                      /* error (operation failed)      */
    LOG_FATAL = 4                       /* fatal (program exits)         */
} LogLevel;

void qf_log(LogLevel Level, const char *Fmt, ...);
void qf_log_init(const char *LogFile);
void qf_log_close(void);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      FTN Address Helpers                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int  ftn_parse_addr(const char *Str, FTN_ADDR *Addr);
void ftn_format_addr(const FTN_ADDR *Addr, char *Buf, int BufSize);
int  ftn_addr_equal(const FTN_ADDR *A, const FTN_ADDR *B);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     BSO Scanner (FTS-5005)                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int  bso_scan(const QfConfig *Cfg, BsoItem *Items, int MaxItems);
int  bso_create_poll(const QfConfig *Cfg, const FTN_ADDR *Addr,
                     BsoFlavour Flavour);
int  bso_lock(const QfConfig *Cfg, const FTN_ADDR *Addr);
void bso_unlock(const QfConfig *Cfg, const FTN_ADDR *Addr);
int  bso_check_hold(const QfConfig *Cfg, const FTN_ADDR *Addr);
void bso_record_try(const QfConfig *Cfg, const FTN_ADDR *Addr,
                    int Success, const char *Msg);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     Session Dispatch                                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int  qf_call_node(const QfConfig *Cfg, const BsoItem *Item);
int  qf_post_session(const QfConfig *Cfg, int Success);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Event Scheduler                                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int  qf_event_check(const QfConfig *Cfg, const QfEvent **Active);
int  qf_event_should_poll(const QfEvent *Ev);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Configuration                                       */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int  qf_config_load(const char *Path, QfConfig *Cfg);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                  .PKT Header (FTS-0001 Type 2+)                           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#pragma pack(push, 1)
typedef struct {
    uint16_t OrigNode;                  /* originating node number       */
    uint16_t DestNode;                  /* destination node number       */
    uint16_t Year;                      /* year (e.g. 2026)              */
    uint16_t Month;                     /* month (0-11)                  */
    uint16_t Day;                       /* day (1-31)                    */
    uint16_t Hour;                      /* hour (0-23)                   */
    uint16_t Minute;                    /* minute (0-59)                 */
    uint16_t Second;                    /* second (0-59)                 */
    uint16_t Baud;                      /* baud rate (0 for IP)          */
    uint16_t PktVer;                    /* packet version (always 2)     */
    uint16_t OrigNet;                   /* originating net number        */
    uint16_t DestNet;                   /* destination net number        */
    uint8_t  ProductLo;                 /* product code low byte         */
    uint8_t  RevisionMajor;             /* major revision                */
    char     Password[8];               /* session password              */
    uint16_t OrigZone;                  /* originating zone (QMail)      */
    uint16_t DestZone;                  /* destination zone (QMail)      */
    uint16_t AuxNet;                    /* aux net for Type 2+           */
    uint16_t CwValid;                   /* CW validation copy            */
    uint8_t  ProductHi;                 /* product code high byte        */
    uint8_t  RevisionMinor;             /* minor revision                */
    uint16_t Cw;                        /* capability word               */
    uint16_t OrigZone2;                 /* originating zone (Type 2+)    */
    uint16_t DestZone2;                 /* destination zone (Type 2+)    */
    uint16_t OrigPoint;                 /* originating point             */
    uint16_t DestPoint;                 /* destination point             */
    uint32_t ProdData;                  /* product-specific data         */
} PKT_HEADER;                           /* 58 bytes total                */
#pragma pack(pop)

#define PKT_VERSION     2               /* FTS-0001 packet version       */
#define PKT_CW_VALID    0x0100          /* capability word validation    */
#define PKT_CW_TYPE2PLUS 0x0001         /* Type 2+ capable               */


#endif /* QFRONT_H */
