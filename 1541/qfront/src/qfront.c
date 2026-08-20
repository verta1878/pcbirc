/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* qfront.c -- QFront Main Orchestrator                                     */
/*                                                                           */
/* The mailer's main loop -- fully wired with all modules:                   */
/*   1. Load config, nodelist, routing rules, events, semaphore state        */
/*   2. Check event schedule -- run pre-event actions                        */
/*   3. Scan BSO outbound for pending nodes                                  */
/*   4. For each node: check routing, nodelist, semaphores, events           */
/*   5. Lock, dispatch session via binkd, unlock, record attempt             */
/*   6. Post-session: run tosser, TIC processor, check fixups                */
/*   7. Check semaphore exit triggers                                        */
/*   8. Status display, sleep, repeat                                        */
/*                                                                           */
/* Clean-room from FTS-5005 (BSO), FTS-0001 (.PKT), FTS-5001 (nodelist).    */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"
#include <stdarg.h>

#define QFRONT_VERSION "1.0.0"


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                  External Module Declarations                             */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* These are defined in their respective .c files.
 * Forward-declared here to avoid a massive header. */

/* nodelist.c */
typedef struct NlDatabase NlDatabase;
typedef struct NlEntry NlEntry;
extern NlDatabase  *nl_open(const char *Path);
extern const NlEntry *nl_lookup(const NlDatabase *Db, const FTN_ADDR *Addr);
extern const NlEntry *nl_find_host(const NlDatabase *Db, const FTN_ADDR *Addr);
extern void           nl_close(NlDatabase *Db);
extern int            nl_entry_is_cm(const NlEntry *E);
extern int            nl_entry_is_down(const NlEntry *E);
extern int            nl_entry_is_hold(const NlEntry *E);
extern const char    *nl_entry_ibn_host(const NlEntry *E);

/* route.c */
extern int rt_load(const char *CfgPath);
extern int rt_resolve(const FTN_ADDR *Dest, FTN_ADDR *ViaAddr);

/* events.c */
typedef struct QfEventDef QfEventDef;
extern int                ev_load(const char *CfgPath);
extern const QfEventDef  *ev_check_active(void);
extern int                ev_should_poll(const QfEventDef *Ev, int IsCM, int IsListed);
extern int                ev_pre_actions(const QfEventDef *Ev, const QfConfig *Cfg);
extern int                ev_post_actions(const QfEventDef *Ev, const QfConfig *Cfg, int QueueEmpty);
extern int                ev_run_batch(const QfEventDef *Ev);
extern const char        *ev_get_tag(const QfEventDef *Ev);
extern uint32_t           ev_get_flags(const QfEventDef *Ev);
extern int                ev_get_errorlevel(const QfEventDef *Ev);

/* semaphore.c */
extern void sem_mark_polled(const FTN_ADDR *Addr);
extern int  sem_was_polled(const FTN_ADDR *Addr, int CooldownSec);
extern void sem_record_failure(const FTN_ADDR *Addr);
extern void sem_record_success(const FTN_ADDR *Addr);
extern int  sem_is_undialable(const FTN_ADDR *Addr, int MaxRetries);
extern void sem_clear_undialable(const FTN_ADDR *Addr);
extern void sem_add_trigger(const char *Path, int Errorlevel);
extern int  sem_check_triggers(void);
extern void sem_save_state(const char *Dir);
extern void sem_load_state(const char *Dir);

/* tic.c */
extern int tic_process(const QfConfig *Cfg);

/* modem.c */
extern int mdm_wait_ring(void *Sp, int TimeoutMs);
extern int mdm_answer_call(void *Sp, void *Mcfg, int *ConnectSpeed);

/* session.c */
extern int qf_answer_session(void *Sp, const QfConfig *Cfg,
                              int *FilesIn, int *FilesOut);


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                           Logging                                         */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static FILE *g_LogFp  = NULL;           /* log file handle               */
static int   g_Debug  = 0;              /* debug mode flag               */


/*-----------------------------------------------------------------------*/
/* log_level_str() -- Convert log level to 3-char display string        */
/*-----------------------------------------------------------------------*/

static const char *log_level_str(LogLevel Level)
{
    switch (Level) {
    case LOG_DEBUG: return "DBG";
    case LOG_INFO:  return "   ";
    case LOG_WARN:  return "WRN";
    case LOG_ERROR: return "ERR";
    case LOG_FATAL: return "FTL";
    default:        return "???";
    }
}


/*-----------------------------------------------------------------------*/
/* qf_log_init() -- Open the log file                                   */
/*-----------------------------------------------------------------------*/

void qf_log_init(const char *LogFile)
{
    if (LogFile && LogFile[0])
        g_LogFp = fopen(LogFile, "a");
}


/*-----------------------------------------------------------------------*/
/* qf_log_close() -- Close the log file                                 */
/*-----------------------------------------------------------------------*/

void qf_log_close(void)
{
    if (g_LogFp) { fclose(g_LogFp); g_LogFp = NULL; }
}


/*-----------------------------------------------------------------------*/
/* qf_log() -- Write a timestamped log message                          */
/*                                                                       */
/* Writes to the log file and (for INFO and above) to stderr.            */
/* Debug messages are suppressed unless g_Debug is set.                  */
/*-----------------------------------------------------------------------*/

void qf_log(LogLevel Level, const char *Fmt, ...)
{
    va_list    Ap;                      /* variable argument list         */
    time_t     Now;                     /* current time                  */
    struct tm *Tm;                      /* broken-down time              */
    char       Ts[32];                  /* timestamp string              */
    FILE      *Out;                     /* output file handle            */

    if (Level == LOG_DEBUG && !g_Debug) return;

    time(&Now);
    Tm = localtime(&Now);
    strftime(Ts, sizeof(Ts), "%Y-%m-%d %H:%M:%S", Tm);

    Out = g_LogFp ? g_LogFp : stderr;
    fprintf(Out, "%s [%s] ", Ts, log_level_str(Level));
    va_start(Ap, Fmt); vfprintf(Out, Fmt, Ap); va_end(Ap);
    fprintf(Out, "\n");
    fflush(Out);

    if (g_LogFp && Level >= LOG_INFO) {
        fprintf(stderr, "%s [%s] ", Ts, log_level_str(Level));
        va_start(Ap, Fmt); vfprintf(stderr, Fmt, Ap); va_end(Ap);
        fprintf(stderr, "\n");
    }
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     Display and Status                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* qf_display_file() -- Display a text file (DOS .TXT files)            */
/*                                                                       */
/* Shows NORMAL.TXT, CRITICAL.TXT, FAILED.TXT, LOWBAUD.TXT,             */
/* EXPWARN.TXT, NOCALLER.TXT to callers via serial port.                 */
/*-----------------------------------------------------------------------*/

static void qf_display_file(const char *FileName)
{
    char  Path[260];                    /* file path                     */
    FILE *f;                            /* text file handle              */
    char  Line[256];                    /* line read buffer              */

    snprintf(Path, sizeof(Path), "%s", FileName);
    f = fopen(Path, "r");
    if (!f) return;

    while (fgets(Line, sizeof(Line), f))
        qf_log(LOG_DEBUG, "DISPLAY: %s", Line);

    fclose(f);
}


/*-----------------------------------------------------------------------*/
/* qf_status() -- Update the status line display                        */
/*-----------------------------------------------------------------------*/

static void qf_status(const char *State, const QfConfig *Cfg,
                       int QueueCount, const QfEventDef *Ev)
{
    time_t     Now;                     /* current time                  */
    struct tm *Tm;                      /* broken-down time              */
    char       Ts[20];                  /* time string                   */
    char       Aka[64];                 /* formatted primary AKA         */

    time(&Now);
    Tm = localtime(&Now);
    strftime(Ts, sizeof(Ts), "%H:%M:%S", Tm);

    ftn_format_addr(&Cfg->aka[0], Aka, sizeof(Aka));

    fprintf(stderr, "\r[%s] %s | Queue: %d | Event: %s  ",
            Ts, State, QueueCount,
            Ev ? ev_get_tag(Ev) : "(none)");
    fflush(stderr);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Signal Handling                                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* BUG-1 fix: Ctrl-C or kill without this leaves serial ports open,
 * .bsy locks orphaned, and state files unwritten. */

static volatile int g_Shutdown = 0;     /* shutdown requested flag       */

static void qf_signal_handler(int Sig)
{
    (void)Sig;
    g_Shutdown = 1;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    Session History (Today's Activity)                      */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

static int  g_SessionsOk   = 0;        /* successful sessions today     */
static int  g_SessionsFail = 0;        /* failed sessions today         */
static long g_BytesSent    = 0;        /* bytes sent today              */
static long g_BytesRecv    = 0;        /* bytes received today          */


/*-----------------------------------------------------------------------*/
/* qf_print_activity() -- Display today's session summary               */
/*-----------------------------------------------------------------------*/

static void qf_print_activity(void)
{
    qf_log(LOG_INFO,
           "Today's activity: %d OK, %d failed, %ld sent, %ld recv",
           g_SessionsOk, g_SessionsFail, g_BytesSent, g_BytesRecv);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Session Dispatch                                     */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* qf_call_node_full() -- Call a node with all checks                   */
/*                                                                       */
/* The central dispatch function. For each pending node:                  */
/*   1. Check semaphores (recently polled? undialable?)                   */
/*   2. Resolve routing (direct, via host, via hub, hold)                */
/*   3. Nodelist lookup (CM status, DOWN/HOLD, IBN host)                 */
/*   4. Event filtering (CM-only, non-CM only, listed-only)              */
/*   5. BSO lock (.bsy file)                                             */
/*   6. Build command line and execute external mailer                    */
/*   7. Record result (success/failure, polled, .try file)               */
/*   8. Unlock (.bsy file)                                               */
/*                                                                       */
/* Returns 0 on success, -1 on skip/failure.                             */
/*-----------------------------------------------------------------------*/

static int qf_call_node_full(const QfConfig *Cfg, const BsoItem *Item,
                              const NlDatabase *Nl, const QfEventDef *Ev)
{
    char           AddrStr[64];         /* formatted address for display */
    char           CmdLine[512];        /* external mailer command       */
    FTN_ADDR       ViaAddr;             /* resolved routing address      */
    const FTN_ADDR *DialTarget;         /* actual address to call        */
    const NlEntry  *NlEnt;             /* nodelist entry                */
    int            RouteRc;             /* routing result                */
    int            Rc;                  /* session result                */
    int            IsCM = 0;            /* node is Continuous Mail       */
    int            IsListed = 0;        /* node is in nodelist           */

    ftn_format_addr(&Item->addr, AddrStr, sizeof(AddrStr));

    /* ---- Semaphore checks ---- */

    if (sem_was_polled(&Item->addr, Cfg->retry_delay)) {
        qf_log(LOG_DEBUG, "Skipping %s -- recently polled", AddrStr);
        return -1;
    }

    if (sem_is_undialable(&Item->addr, Cfg->max_retries)) {
        qf_log(LOG_DEBUG, "Skipping %s -- undialable", AddrStr);
        return -1;
    }

    /* ---- Routing ---- */

    RouteRc = rt_resolve(&Item->addr, &ViaAddr);
    if (RouteRc < 0) {
        qf_log(LOG_DEBUG, "Skipping %s -- route says hold/nopoll",
               AddrStr);
        return -1;
    }

    DialTarget = (RouteRc == 1) ? &ViaAddr : &Item->addr;

    /* ---- Nodelist lookup ---- */

    NlEnt = Nl ? nl_lookup(Nl, DialTarget) : NULL;
    if (NlEnt) {
        IsCM     = nl_entry_is_cm(NlEnt);
        IsListed = 1;

        if (nl_entry_is_down(NlEnt) || nl_entry_is_hold(NlEnt)) {
            qf_log(LOG_DEBUG, "Skipping %s -- nodelist: DOWN/HOLD",
                   AddrStr);
            return -1;
        }
    } else {
        qf_log(LOG_DEBUG, "Address not found in nodelist: %s", AddrStr);
    }

    /* ---- Event filtering ---- */

    if (!ev_should_poll(Ev, IsCM, IsListed)) {
        qf_log(LOG_DEBUG, "Skipping %s -- event filter", AddrStr);
        return -1;
    }

    /* ---- BSO lock ---- */

    if (bso_check_hold(Cfg, &Item->addr)) {
        qf_log(LOG_DEBUG, "Skipping %s -- on hold", AddrStr);
        return -1;
    }

    if (bso_lock(Cfg, &Item->addr) != 0) {
        qf_log(LOG_WARN, "Skipping %s -- busy", AddrStr);
        return -1;
    }

    /* ---- Dial ---- */

    {
        char DialStr[64];               /* formatted dial target         */

        ftn_format_addr(DialTarget, DialStr, sizeof(DialStr));
        qf_log(LOG_INFO,
               "Dialing %s%s%s (flavour=%c mail=%d files=%d)",
               AddrStr,
               RouteRc == 1 ? " via " : "",
               RouteRc == 1 ? DialStr : "",
               (char)Item->flavour,
               Item->has_netmail, Item->has_filelist);
    }

    /* Build command line for external mailer */
    {
        char DialStr[64];               /* formatted dial target         */

        ftn_format_addr(DialTarget, DialStr, sizeof(DialStr));

        if (NlEnt && nl_entry_ibn_host(NlEnt)[0]) {
            snprintf(CmdLine, sizeof(CmdLine), "%s -p -P %s -h %s",
                     Cfg->binkd_path, DialStr, nl_entry_ibn_host(NlEnt));
        } else {
            snprintf(CmdLine, sizeof(CmdLine), "%s -p -P %s",
                     Cfg->binkd_path, DialStr);
        }
    }

    /* Execute */
#ifdef _WIN32
    Rc = system(CmdLine);
#else
    Rc = system(CmdLine);
    if (WIFEXITED(Rc))
        Rc = WEXITSTATUS(Rc);
#endif

    /* ---- Record result ---- */

    if (Rc == 0) {
        qf_log(LOG_INFO,
               "Successfully sent packet(s)/file(s) to %s", AddrStr);
        g_SessionsOk++;
        bso_record_try(Cfg, &Item->addr, 1, "OK");
        sem_record_success(&Item->addr);
    } else {
        qf_log(LOG_WARN,
               "Attempt to send packet(s)/file(s) to %s was unsuccessful",
               AddrStr);
        g_SessionsFail++;
        bso_record_try(Cfg, &Item->addr, 0, "Session failed");
        sem_record_failure(&Item->addr);
    }

    sem_mark_polled(&Item->addr);
    bso_unlock(Cfg, &Item->addr);

    return Rc;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    Post-Session Processing                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* qf_post_session_full() -- Run tosser and TIC processor               */
/*                                                                       */
/* Called after any successful session. Tosses received .PKT files       */
/* into the message base and processes TIC file echoes.                  */
/*-----------------------------------------------------------------------*/

static int qf_post_session_full(const QfConfig *Cfg, int AnySuccess)
{
    int Rc = 0;                         /* tosser exit code              */

    if (!AnySuccess) return 0;

    /* Run tosser */
    if (Cfg->tosser_path[0]) {
        qf_log(LOG_INFO, "Scanning/tossing FidoMail");
        Rc = system(Cfg->tosser_path);
        if (Rc != 0)
            qf_log(LOG_WARN, "Tosser exited with code %d", Rc);
    }

    /* Process TIC files */
    tic_process(Cfg);

    /* Check inbound for new netmail notifications */
#ifndef _WIN32
    {
        char           InbPath[520];    /* inbound directory path        */
        int            NewMail = 0;     /* new .PKT count               */
        int            NewFiles = 0;    /* new .TIC count               */
        DIR           *d;              /* directory handle               */
        struct dirent *Ent;            /* directory entry                */

        snprintf(InbPath, sizeof(InbPath), "%s", Cfg->inbound);
        d = opendir(InbPath);
        if (d) {
            while ((Ent = readdir(d)) != NULL) {
                int Len = (int)strlen(Ent->d_name);
                if (Len > 4 &&
                    strcasecmp(Ent->d_name + Len - 4, ".pkt") == 0)
                    NewMail++;
                else if (Len > 4 &&
                         strcasecmp(Ent->d_name + Len - 4, ".tic") == 0)
                    NewFiles++;
            }
            closedir(d);
        }
        if (NewMail > 0)
            qf_log(LOG_INFO,
                   "New NetMail: %d packet(s) in inbound", NewMail);
        if (NewFiles > 0)
            qf_log(LOG_INFO,
                   "New file: %d TIC(s) in inbound", NewFiles);
    }
#endif

    return Rc;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                QFIXUPS: Retry Incomplete Transfers                        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#define MAX_FIXUPS 128                  /* max incomplete transfers       */

typedef struct {
    FTN_ADDR Addr;                      /* node address                  */
    char     FileName[260];             /* incomplete filename            */
    time_t   When;                      /* when transfer failed          */
} FixupEntry;

static FixupEntry g_Fixups[MAX_FIXUPS]; /* fixup queue                   */
static int        g_FixupCount = 0;     /* entries in fixup queue        */


/*-----------------------------------------------------------------------*/
/* fixup_add() -- Add an incomplete transfer to the retry queue         */
/*-----------------------------------------------------------------------*/

static void fixup_add(const FTN_ADDR *Addr, const char *FileName)
{
    if (g_FixupCount >= MAX_FIXUPS) return;
    g_Fixups[g_FixupCount].Addr = *Addr;
    strncpy(g_Fixups[g_FixupCount].FileName, FileName, 259);
    g_Fixups[g_FixupCount].When = time(NULL);
    g_FixupCount++;
}


/*-----------------------------------------------------------------------*/
/* fixup_save() -- Save fixup queue to QFIXUPS.DAT                     */
/*-----------------------------------------------------------------------*/

static void fixup_save(const char *Dir)
{
    char  Path[260];                    /* file path                     */
    FILE *f;                            /* output file handle            */
    int   i;                            /* loop index                    */

    snprintf(Path, sizeof(Path), "%s%cQFIXUPS.DAT", Dir, PATH_SEP);
    f = fopen(Path, "w");
    if (!f) return;
    for (i = 0; i < g_FixupCount; i++) {
        fprintf(f, "%u:%u/%u %s %ld\n",
                g_Fixups[i].Addr.zone, g_Fixups[i].Addr.net,
                g_Fixups[i].Addr.node, g_Fixups[i].FileName,
                (long)g_Fixups[i].When);
    }
    fclose(f);
}


/*-----------------------------------------------------------------------*/
/* fixup_load() -- Load fixup queue from QFIXUPS.DAT                   */
/*-----------------------------------------------------------------------*/

static void fixup_load(const char *Dir)
{
    char  Path[260];                    /* file path                     */
    char  Line[512];                    /* line read buffer              */
    FILE *f;                            /* input file handle             */

    snprintf(Path, sizeof(Path), "%s%cQFIXUPS.DAT", Dir, PATH_SEP);
    f = fopen(Path, "r");
    if (!f) return;
    while (fgets(Line, sizeof(Line), f) && g_FixupCount < MAX_FIXUPS) {
        char AddrStr[64];               /* address string from file      */
        char Fname[260];                /* filename from file            */
        long When;                      /* timestamp from file           */

        if (sscanf(Line, "%63s %259s %ld", AddrStr, Fname, &When) == 3) {
            ftn_parse_addr(AddrStr, &g_Fixups[g_FixupCount].Addr);
            strncpy(g_Fixups[g_FixupCount].FileName, Fname, 259);
            g_Fixups[g_FixupCount].When = (time_t)When;
            g_FixupCount++;
        }
    }
    fclose(f);
    if (g_FixupCount > 0)
        qf_log(LOG_INFO, "Loaded %d fixup entries", g_FixupCount);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Configuration Loader                                 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* qf_config_load() -- Load qfront.cfg                                  */
/*                                                                       */
/* Parses key=value pairs from the config file. Sets defaults for        */
/* unspecified values. Semaphore= lines are forwarded to sem_add_trigger. */
/* Event sections ([Event.NAME]) are skipped here -- ev_load() handles   */
/* those separately.                                                      */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int qf_config_load(const char *Path, QfConfig *Cfg)
{
    FILE *f;                            /* config file handle            */
    char  Line[512];                    /* line read buffer              */
    char  Key[64];                      /* parsed key name               */
    char  Val[448];                     /* parsed value string           */

    memset(Cfg, 0, sizeof(*Cfg));
    Cfg->com_port     = 1;
    Cfg->locked_baud  = 115200;
    Cfg->max_retries  = 5;
    Cfg->retry_delay  = 300;
    Cfg->hold_time    = 3600;

    f = fopen(Path, "r");
    if (!f) {
        qf_log(LOG_FATAL, "Cannot open config: %s", Path);
        return -1;
    }

    while (fgets(Line, sizeof(Line), f)) {
        char *p = Line;                 /* line scan pointer             */

        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n' ||
            *p == '\r' || *p == '\0')
            continue;
        if (*p == '[') continue;        /* skip section headers (events) */

        /* Strip trailing whitespace */
        {
            char *End = p + strlen(p) - 1;
            while (End > p && (*End == '\n' || *End == '\r' || *End == ' '))
                *End-- = '\0';
        }

        if (sscanf(p, "%63[^=]=%447[^\n]", Key, Val) != 2) continue;

        if (strcmp(Key, "Address") == 0 && Cfg->num_aka < 16)
            ftn_parse_addr(Val, &Cfg->aka[Cfg->num_aka++]);
        else if (strcmp(Key, "Outbound") == 0)
            strncpy(Cfg->outbound, Val, 259);
        else if (strcmp(Key, "Inbound") == 0)
            strncpy(Cfg->inbound, Val, 259);
        else if (strcmp(Key, "TempInbound") == 0)
            strncpy(Cfg->temp_inbound, Val, 259);
        else if (strcmp(Key, "NetmailDir") == 0)
            strncpy(Cfg->netmail_dir, Val, 259);
        else if (strcmp(Key, "LogFile") == 0)
            strncpy(Cfg->logfile, Val, 259);
        else if (strcmp(Key, "Mailer") == 0)
            strncpy(Cfg->binkd_path, Val, 259);
        else if (strcmp(Key, "Tosser") == 0)
            strncpy(Cfg->tosser_path, Val, 259);
        else if (strcmp(Key, "TicProc") == 0)
            strncpy(Cfg->tic_proc, Val, 259);
        else if (strcmp(Key, "NodelistDir") == 0)
            strncpy(Cfg->nodelist_dir, Val, 259);
        else if (strcmp(Key, "NodelistBase") == 0)
            strncpy(Cfg->nodelist_base, Val, 31);
        else if (strcmp(Key, "MaxRetries") == 0)
            Cfg->max_retries = atoi(Val);
        else if (strcmp(Key, "RetryDelay") == 0)
            Cfg->retry_delay = atoi(Val);
        else if (strcmp(Key, "HoldTime") == 0)
            Cfg->hold_time = atoi(Val);
        else if (strcmp(Key, "ComPort") == 0)
            Cfg->com_port = atoi(Val);
        else if (strcmp(Key, "BaudRate") == 0 ||
                 strcmp(Key, "LockedBaud") == 0)
            Cfg->locked_baud = atoi(Val);
        else if (strcmp(Key, "Debug") == 0)
            Cfg->debug = atoi(Val);
        else if (strcmp(Key, "Semaphore") == 0) {
            /* Semaphore=<path>:<errorlevel> */
            char *Colon = strrchr(Val, ':');
            if (Colon) {
                *Colon = '\0';
                sem_add_trigger(Val, atoi(Colon + 1));
            }
        }
    }

    fclose(f);

    if (Cfg->num_aka == 0) {
        qf_log(LOG_FATAL, "No Address defined in config");
        return -1;
    }
    if (!Cfg->outbound[0]) {
        qf_log(LOG_FATAL, "No Outbound directory defined");
        return -1;
    }

    g_Debug = Cfg->debug;

    qf_log(LOG_INFO,
           "Config loaded: %d AKAs, outbound=%s, mailer=%s",
           Cfg->num_aka, Cfg->outbound,
           Cfg->binkd_path[0] ? Cfg->binkd_path : "(none)");

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                             Main Entry                                    */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* print_usage() -- Display command-line help                           */
/*-----------------------------------------------------------------------*/

static void print_usage(void)
{
    fprintf(stderr,
        "QFront v" QFRONT_VERSION " -- FidoNet Mailer Orchestrator\n"
        "Clean-room implementation from FTS-5005/FTS-0001/FTS-5001\n\n"
        "Usage: qfront [options]\n"
        "  -c <config>   Config file (default: qfront.cfg)\n"
        "  -p <addr>     Poll a specific node and exit\n"
        "  -s            Single pass (scan once, no loop)\n"
        "  -d            Debug mode (verbose logging)\n"
        "  -h            This help\n"
        "\n"
        "DOS compatibility options:\n"
        "  /DEBUG         Create QFRONT.DBG debug log\n"
        "  /LOCALONLY     Local-only mode (no modem)\n"
        "  /NOANSWER      Don't auto-answer phone\n"
        "  /NOCLEARWC     Don't clear NODEINFO.DAT on startup\n"
        "  /NO16550       Disable 16550 UART FIFO\n"
        "  /NOMOUSE       Disable mouse support\n"
        "  /COLOR         Force color display\n"
        "  /MONO          Force monochrome display\n"
        "  /C<config>     Specify config file path\n");
}


#ifndef QFRONT_LIB
int main(int Argc, char *Argv[])
{
    QfConfig        Cfg;                /* loaded configuration          */
    NlDatabase     *Nodelist = NULL;    /* parsed nodelist database      */
    const char     *CfgFile  = "qfront.cfg";  /* config file path       */
    const char     *PollAddr = NULL;    /* manual poll address           */
    int             SinglePass = 0;     /* single scan mode              */
    int             i;                  /* argument loop index           */

    /* Ignore SIGPIPE -- broken pipe on socket/serial write */
#if !defined(_WIN32) && defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif

    /* Install shutdown handlers (BUG-1 fix) */
    signal(SIGINT,  qf_signal_handler);
    signal(SIGTERM, qf_signal_handler);
#ifdef SIGHUP
    signal(SIGHUP,  qf_signal_handler);
#endif

    /* Parse command line -- supports both Unix and DOS styles */
    for (i = 1; i < Argc; i++) {
        if (strcmp(Argv[i], "-c") == 0 && i + 1 < Argc)
            CfgFile = Argv[++i];
        else if (strncasecmp(Argv[i], "/C", 2) == 0 && Argv[i][2])
            CfgFile = Argv[i] + 2;
        else if (strcasecmp(Argv[i], "/DEBUG") == 0)
            g_Debug = 1;
        else if (strcasecmp(Argv[i], "/LOCALONLY") == 0)
            qf_log(LOG_INFO, "Local-only mode (modem disabled)");
        else if (strcasecmp(Argv[i], "/NOANSWER") == 0)
            qf_log(LOG_INFO, "Auto-answer disabled");
        else if (strcasecmp(Argv[i], "/NO16550") == 0)
            qf_log(LOG_INFO, "16550 UART FIFO disabled");
        else if (strcasecmp(Argv[i], "/COLOR") == 0)
            ;                           /* color mode (default)          */
        else if (strcasecmp(Argv[i], "/MONO") == 0)
            ;                           /* monochrome mode               */
        else if (strcmp(Argv[i], "-p") == 0 && i + 1 < Argc)
            PollAddr = Argv[++i];
        else if (strcmp(Argv[i], "-s") == 0)
            SinglePass = 1;
        else if (strcmp(Argv[i], "-d") == 0)
            g_Debug = 1;
        else if (strcmp(Argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    /* ---- Phase 1: Load everything ---- */

    if (qf_config_load(CfgFile, &Cfg) != 0)
        return 1;

    qf_log_init(Cfg.logfile);
    qf_log(LOG_INFO,
           "QFront v" QFRONT_VERSION " starting -- %d AKA(s)",
           Cfg.num_aka);

    {
        char Buf[64];                   /* formatted AKA for display     */
        for (i = 0; i < Cfg.num_aka; i++) {
            ftn_format_addr(&Cfg.aka[i], Buf, sizeof(Buf));
            qf_log(LOG_INFO, "  AKA %d: %s", i, Buf);
        }
    }

    rt_load(CfgFile);                   /* load routing rules            */
    ev_load(CfgFile);                   /* load event schedule           */

    /* Load nodelist */
    if (Cfg.nodelist_dir[0] && Cfg.nodelist_base[0]) {
        char NlPath[520];              /* nodelist file path            */

        snprintf(NlPath, sizeof(NlPath), "%s%c%s",
                 Cfg.nodelist_dir, PATH_SEP, Cfg.nodelist_base);
        Nodelist = nl_open(NlPath);
    }

    sem_load_state(Cfg.outbound);       /* restore semaphore state       */
    fixup_load(Cfg.outbound);           /* restore fixup queue           */

    /* ---- Manual poll mode ---- */

    if (PollAddr) {
        FTN_ADDR Addr;                  /* parsed poll address           */
        BsoItem  Item;                  /* BSO item for poll             */

        if (ftn_parse_addr(PollAddr, &Addr) != 0) {
            qf_log(LOG_FATAL, "Invalid address: %s", PollAddr);
            return 1;
        }

        memset(&Item, 0, sizeof(Item));
        Item.addr    = Addr;
        Item.flavour = BSO_IMMEDIATE;
        Item.has_netmail = 1;

        bso_create_poll(&Cfg, &Addr, BSO_IMMEDIATE);

        i = qf_call_node_full(&Cfg, &Item, Nodelist, NULL);
        qf_post_session_full(&Cfg, i == 0);

        sem_save_state(Cfg.outbound);
        nl_close(Nodelist);
        qf_log_close();
        return (i == 0) ? 0 : 1;
    }

    /* ---- Main orchestrator loop ---- */

    qf_log(LOG_INFO, "Waiting for a call");

    do {
        BsoItem            Items[256];  /* pending nodes from BSO scan   */
        int                Count;       /* nodes in queue                */
        int                j;           /* node loop index               */
        int                AnySuccess = 0;  /* session success flag      */
        const QfEventDef  *Ev = NULL;   /* active event                  */
        int                SemExit;     /* semaphore exit code           */
        int                AnswerRc;    /* inbound answer result         */
        int                FilesIn = 0; /* files received this session   */
        int                FilesOut = 0;/* files sent this session       */

        /* ---- Check semaphore exit triggers ---- */
        SemExit = sem_check_triggers();
        if (SemExit >= 0) {
            qf_log(LOG_INFO, "Exiting with errorlevel %d", SemExit);
            sem_save_state(Cfg.outbound);
            fixup_save(Cfg.outbound);
            nl_close(Nodelist);
            qf_log_close();
            return SemExit;
        }

        /* ---- Check event schedule ---- */
        Ev = ev_check_active();
        if (Ev) {
            qf_log(LOG_DEBUG, "Running event \"%s\"", ev_get_tag(Ev));
            ev_pre_actions(Ev, &Cfg);
        }

        /* ---- Scan BSO outbound ---- */
        qf_log(LOG_DEBUG, "Building queue");
        Count = bso_scan(&Cfg, Items, 256);

        if (Count == 0) {
            qf_status("Waiting for a call", &Cfg, 0, Ev);
        } else {
            qf_log(LOG_INFO,
                   "Scanning for mail -- %d nodes pending", Count);
        }

        /* ---- Process each node ---- */
        for (j = 0; j < Count; j++) {
            char AddrStr[64];           /* formatted address for status  */

            ftn_format_addr(&Items[j].addr, AddrStr, sizeof(AddrStr));

            /* Skip hold flavour unless event forces */
            if (Items[j].flavour == BSO_HOLD && Ev &&
                !(ev_get_flags(Ev) & 0x200 /*EVF_FORCE_POLL*/)) {
                qf_log(LOG_DEBUG,
                       "Skipping %s (hold flavour)", AddrStr);
                continue;
            }

            qf_status(AddrStr, &Cfg, Count - j, Ev);

            if (qf_call_node_full(&Cfg, &Items[j], Nodelist, Ev) == 0)
                AnySuccess = 1;
        }

        /* ---- Post-session processing ---- */
        if (AnySuccess)
            qf_post_session_full(&Cfg, 1);

        /* ---- Event post-actions ---- */
        if (Ev) {
            int EndEvent;               /* event end flag                */

            EndEvent = ev_post_actions(Ev, &Cfg, Count == 0);
            if (EndEvent)
                qf_log(LOG_INFO,
                       "Ending event \"%s\"", ev_get_tag(Ev));
        }

        /* ---- Save state ---- */
        sem_save_state(Cfg.outbound);
        fixup_save(Cfg.outbound);

        /* ---- Check for incoming calls ----
         *
         * Between outbound poll cycles, check the modem for incoming
         * calls. qf_answer_session() handles the full inbound flow:
         *   - Wait for RING (times out after retry_delay seconds)
         *   - Answer with ATA
         *   - Detect FidoNet handshake vs human caller
         *   - Run FidoNet session or exit for PCBoard
         *
         * Return codes from qf_answer_session():
         *   -1 = no call (timeout) -- normal, loop back to outbound
         *    0 = FidoNet session completed -- toss mail, loop
         *    1 = human caller -- EXIT QFront, return errorlevel 1
         *    5 = FAX call -- EXIT QFront, return errorlevel 5
         *
         * Errorlevels 1 and 5 cause QFront to terminate so BOARD.BAT
         * can load the appropriate program (PCBoard or FAX receiver).
         */
        if (!SinglePass && !g_Shutdown) {
            qf_log(LOG_DEBUG,
                   "Checking for incoming calls on COM%d", Cfg.com_port);

            AnswerRc = qf_answer_session(NULL, &Cfg, &FilesIn, &FilesOut);

            qf_log(LOG_DEBUG,
                   "qf_answer_session returned %d "
                   "(files_in=%d, files_out=%d)",
                   AnswerRc, FilesIn, FilesOut);

            if (AnswerRc == 1) {
                /* Human caller -- exit for PCBoard */
                qf_log(LOG_INFO,
                       "Human caller -- exiting with errorlevel 1");
                qf_log(LOG_INFO,
                       "BOARD.BAT should load PCBOARD.EXE now");
                sem_save_state(Cfg.outbound);
                fixup_save(Cfg.outbound);
                nl_close(Nodelist);
                qf_log_close();
                return 1;
            } else if (AnswerRc == 5) {
                /* FAX call -- exit for FAX software */
                qf_log(LOG_INFO,
                       "FAX call -- exiting with errorlevel 5");
                sem_save_state(Cfg.outbound);
                fixup_save(Cfg.outbound);
                nl_close(Nodelist);
                qf_log_close();
                return 5;
            } else if (AnswerRc == 0) {
                /* FidoNet session completed */
                if (FilesIn > 0 || FilesOut > 0) {
                    qf_log(LOG_INFO,
                           "Inbound session complete: "
                           "%d file(s) received, %d file(s) sent",
                           FilesIn, FilesOut);
                    AnySuccess = 1;
                }
            }
            /* AnswerRc == -1: no call, normal timeout */
        }

        /* ---- Sleep ---- */
        if (!SinglePass && !g_Shutdown) {
            qf_status("Waiting for a call", &Cfg, 0, Ev);
#ifdef _WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
        }

        if (g_Shutdown) {
            qf_log(LOG_INFO, "Caught signal -- clean shutdown");
            break;
        }

    } while (!SinglePass);

    /* ---- Shutdown ---- */

    qf_print_activity();
    qf_log(LOG_INFO, "Normal exit");
    fprintf(stderr, "\n");

    sem_save_state(Cfg.outbound);
    fixup_save(Cfg.outbound);
    nl_close(Nodelist);
    qf_log_close();

    return 0;
}
#endif /* QFRONT_LIB */
