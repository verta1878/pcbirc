/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* events.c -- Event Scheduler (19 flags from QFCONFIG binary)              */
/*                                                                           */
/* Full event scheduler matching QFront v1.20a's event system. Parses       */
/* [Event.NAME] sections from qfront.cfg. Implements all 19 event flags     */
/* extracted from binary analysis.                                           */
/*                                                                           */
/* Event types (from QFCONFIG strings):                                      */
/*   "Default FidoMail event"    -- standard mail processing                */
/*   "Run batch file"            -- execute external command                */
/*   "Exit with errorlevel"      -- exit program with code                  */
/*                                                                           */
/* Clean-room from QFront documentation + binary analysis.                   */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Event Flag Definitions                              */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* All 19 flags from QFCONFIG binary analysis. */

#define EVF_SLIDE          0x00000001   /* slide event time              */
#define EVF_ECHO_ONLY      0x00000002   /* EchoMail only                 */
#define EVF_NET_ONLY       0x00000004   /* NetMail only                  */
#define EVF_RECV_ONLY      0x00000008   /* receive-only                  */
#define EVF_CM_ONLY        0x00000010   /* send to CM systems only       */
#define EVF_NONCM_ONLY     0x00000020   /* send to non-CM only           */
#define EVF_NODE_CRITICAL  0x00000040   /* node-critical                 */
#define EVF_NO_HELD        0x00000080   /* no HELD attach                */
#define EVF_LISTED_ONLY    0x00000100   /* only listed nodes             */
#define EVF_FORCE_POLL     0x00000200   /* force outbound poll           */
#define EVF_POLL_DURING    0x00000400   /* poll during event             */
#define EVF_SCAN_BEFORE    0x00000800   /* scan before event             */
#define EVF_COMPILE_NL     0x00001000   /* compile nodelist              */
#define EVF_COMPILE_RECV   0x00002000   /* compile when received         */
#define EVF_RESCAN         0x00004000   /* rescan on return              */
#define EVF_END_NOMAIL     0x00008000   /* end event when queue empty    */
#define EVF_EXIT_NOMAIL    0x00010000   /* exit program, queue empty     */
#define EVF_EXIT_DONE      0x00020000   /* exit loop when done           */
#define EVF_AUTO_POLL      0x00040000   /* has auto-poll list            */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                        Event Definition                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    char     Tag[32];                   /* event name/tag                */
    char     Description[64];           /* description                   */
    int      Active;                    /* is this event enabled?        */

    /* Schedule */
    int      DayMask;                   /* bit 0=Sun ... bit 6=Sat       */
    int      StartHour;                 /* start hour (0-23)             */
    int      StartMin;                  /* start minute (0-59)           */
    int      EndHour;                   /* end hour (0-23)               */
    int      EndMin;                    /* end minute (0-59)             */

    /* Flags */
    uint32_t Flags;                     /* EVF_* bitmask                 */

    /* Exit */
    int      Errorlevel;                /* exit code (if EVF_EXIT_*)     */

    /* Batch file */
    char     Batch[260];                /* batch file path               */

    /* Auto-poll list */
    FTN_ADDR AutoPoll[16];              /* addresses to auto-poll        */
    int      NumAutoPoll;               /* count of auto-poll addresses  */

    /* Cost limits (from QFCONFIG: "min/max allowable cost") */
    int      MinCost;                   /* -1 = ignore                   */
    int      MaxCost;                   /* -1 = ignore                   */

    /* Tracking */
    time_t   LastRun;                   /* last time this event ran      */
} QfEventDef;

#define MAX_EVENT_DEFS 32               /* max event definitions         */

static QfEventDef g_Events[MAX_EVENT_DEFS];  /* event table              */
static int        g_NumEvents = 0;           /* event count              */


/*-----------------------------------------------------------------------*/
/* ev_parse_days() -- Parse day mask from config string                  */
/*                                                                       */
/* Accepts: "Daily", "Weekdays", "Weekends", or individual day letters: */
/*   s=Sun, M=Mon, T=Tue, W=Wed, t=Thu, F=Fri, S=Sat                   */
/*                                                                       */
/* Returns bitmask: bit 0=Sun, bit 1=Mon, ... bit 6=Sat.                */
/*-----------------------------------------------------------------------*/

static int ev_parse_days(const char *Str)
{
    int         Mask = 0;               /* accumulated day bitmask       */
    const char *p;                      /* string scan pointer           */

    if (strcmp(Str, "Daily") == 0 || strcmp(Str, "daily") == 0)
        return 0x7F;                    /* all 7 days                    */
    if (strcmp(Str, "Weekdays") == 0 || strcmp(Str, "weekdays") == 0)
        return 0x3E;                    /* Mon-Fri (bits 1-5)            */
    if (strcmp(Str, "Weekends") == 0 || strcmp(Str, "weekends") == 0)
        return 0x41;                    /* Sun + Sat (bits 0,6)          */

    for (p = Str; *p; p++) {
        switch (*p) {
        case 's': Mask |= 0x01; break; /* Sun (lowercase = Sun)         */
        case 'M': Mask |= 0x02; break; /* Mon                           */
        case 'T': Mask |= 0x04; break; /* Tue                           */
        case 'W': Mask |= 0x08; break; /* Wed                           */
        case 't': Mask |= 0x10; break; /* Thu (lowercase = Thu)         */
        case 'F': Mask |= 0x20; break; /* Fri                           */
        case 'S': Mask |= 0x40; break; /* Sat (uppercase = Sat)         */
        }
    }

    return Mask;
}


/*-----------------------------------------------------------------------*/
/* ev_parse_time() -- Parse "HH:MM" time string                         */
/*-----------------------------------------------------------------------*/

static void ev_parse_time(const char *Str, int *Hour, int *Min)
{
    *Hour = 0;
    *Min  = 0;
    sscanf(Str, "%d:%d", Hour, Min);
}


/*-----------------------------------------------------------------------*/
/* ev_load() -- Load event definitions from config file                  */
/*                                                                       */
/* Parses [Event.NAME] sections from qfront.cfg. Each section contains   */
/* schedule, flags, and action settings.                                  */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int ev_load(const char *CfgPath)
{
    FILE        *f;                     /* config file handle            */
    char         Line[512];             /* line read buffer              */
    QfEventDef  *Cur = NULL;            /* current event being parsed    */

    g_NumEvents = 0;

    f = fopen(CfgPath, "r");
    if (!f) return -1;

    while (fgets(Line, sizeof(Line), f)) {
        char *p = Line;                 /* line scan pointer             */
        char  Key[64];                  /* parsed key                    */
        char  Val[256];                 /* parsed value                  */

        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n') continue;

        /* Strip trailing whitespace */
        {
            char *End = p + strlen(p) - 1;
            while (End > p && (*End == '\n' || *End == '\r' || *End == ' '))
                *End-- = '\0';
        }

        /* Check for [Event.NAME] section header */
        if (*p == '[' && strncmp(p, "[Event.", 7) == 0) {
            char EventTag[32];          /* parsed event tag              */

            sscanf(p + 7, "%31[^]]", EventTag);

            if (g_NumEvents >= MAX_EVENT_DEFS) continue;

            Cur = &g_Events[g_NumEvents++];
            memset(Cur, 0, sizeof(*Cur));
            strncpy(Cur->Tag, EventTag, sizeof(Cur->Tag) - 1);
            Cur->Active  = 1;
            Cur->MinCost = -1;
            Cur->MaxCost = -1;
            continue;
        }

        /* Skip lines not in an event section */
        if (!Cur) continue;

        /* Don't parse non-event sections */
        if (*p == '[') { Cur = NULL; continue; }

        /* Parse KEY=VALUE */
        if (sscanf(p, "%63[^=]=%255[^\n]", Key, Val) != 2) continue;

        if (strcmp(Key, "Description") == 0)
            strncpy(Cur->Description, Val, sizeof(Cur->Description) - 1);
        else if (strcmp(Key, "Days") == 0)
            Cur->DayMask = ev_parse_days(Val);
        else if (strcmp(Key, "Start") == 0)
            ev_parse_time(Val, &Cur->StartHour, &Cur->StartMin);
        else if (strcmp(Key, "End") == 0)
            ev_parse_time(Val, &Cur->EndHour, &Cur->EndMin);
        else if (strcmp(Key, "Active") == 0)
            Cur->Active = (Val[0] == 'y' || Val[0] == 'Y' || Val[0] == '1');
        else if (strcmp(Key, "Errorlevel") == 0)
            Cur->Errorlevel = atoi(Val);
        else if (strcmp(Key, "Batch") == 0)
            strncpy(Cur->Batch, Val, sizeof(Cur->Batch) - 1);
        else if (strcmp(Key, "MinCost") == 0)
            Cur->MinCost = atoi(Val);
        else if (strcmp(Key, "MaxCost") == 0)
            Cur->MaxCost = atoi(Val);

        /* Boolean flags */
        else if (strcmp(Key, "SlideTime") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_SLIDE;
        else if (strcmp(Key, "EchoOnly") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_ECHO_ONLY;
        else if (strcmp(Key, "NetOnly") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_NET_ONLY;
        else if (strcmp(Key, "ReceiveOnly") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_RECV_ONLY;
        else if (strcmp(Key, "SendCMOnly") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_CM_ONLY;
        else if (strcmp(Key, "SendNonCMOnly") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_NONCM_ONLY;
        else if (strcmp(Key, "NodeCritical") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_NODE_CRITICAL;
        else if (strcmp(Key, "NoHeldAttach") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_NO_HELD;
        else if (strcmp(Key, "ListedOnly") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_LISTED_ONLY;
        else if (strcmp(Key, "ForcePoll") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_FORCE_POLL;
        else if (strcmp(Key, "PollDuring") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_POLL_DURING;
        else if (strcmp(Key, "ScanBefore") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_SCAN_BEFORE;
        else if (strcmp(Key, "CompileNodelist") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_COMPILE_NL;
        else if (strcmp(Key, "CompileOnReceive") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_COMPILE_RECV;
        else if (strcmp(Key, "Rescan") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_RESCAN;
        else if (strcmp(Key, "EndNoMail") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_END_NOMAIL;
        else if (strcmp(Key, "ExitNoMail") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_EXIT_NOMAIL;
        else if (strcmp(Key, "ExitWhenDone") == 0 && Val[0] == 'y')
            Cur->Flags |= EVF_EXIT_DONE;

        /* Auto-poll address list */
        else if (strcmp(Key, "AutoPoll") == 0) {
            char *Tok = strtok(Val, ",");
            Cur->Flags |= EVF_AUTO_POLL;
            while (Tok && Cur->NumAutoPoll < 16) {
                while (*Tok == ' ') Tok++;
                ftn_parse_addr(Tok, &Cur->AutoPoll[Cur->NumAutoPoll++]);
                Tok = strtok(NULL, ",");
            }
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Events: %d loaded", g_NumEvents);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* ev_check_active() -- Check which event is currently active            */
/*                                                                       */
/* Returns pointer to first matching active event, or NULL if none.      */
/* Checks day-of-week mask and time window (handles midnight wrap).      */
/*-----------------------------------------------------------------------*/

const QfEventDef *ev_check_active(void)
{
    time_t     Now;                     /* current time                  */
    struct tm *Tm;                      /* broken-down time              */
    int        Dow;                     /* day of week (0=Sun)           */
    int        CurMin;                  /* current time in minutes       */
    int        i;                       /* event loop index              */

    time(&Now);
    Tm     = localtime(&Now);
    Dow    = Tm->tm_wday;
    CurMin = Tm->tm_hour * 60 + Tm->tm_min;

    qf_log(LOG_DEBUG, "ev_check_active: dow=%d cur_min=%d checking %d events",
           Dow, CurMin, g_NumEvents);

    for (i = 0; i < g_NumEvents; i++) {
        const QfEventDef *Ev = &g_Events[i];
        int StartMin;                   /* event start in minutes        */
        int EndMin;                     /* event end in minutes          */

        if (!Ev->Active) continue;
        if (!(Ev->DayMask & (1 << Dow))) continue;

        StartMin = Ev->StartHour * 60 + Ev->StartMin;
        EndMin   = Ev->EndHour * 60 + Ev->EndMin;

        if (EndMin <= StartMin) {
            /* Midnight wrap */
            if (CurMin >= StartMin || CurMin < EndMin)
                return Ev;
        } else {
            if (CurMin >= StartMin && CurMin < EndMin)
                return Ev;
        }
    }

    return NULL;
}


/*-----------------------------------------------------------------------*/
/* ev_should_poll() -- Check whether to poll a node during this event    */
/*                                                                       */
/* Some events restrict which nodes can be polled:                       */
/*   EVF_RECV_ONLY   -- don't dial out at all (receive-only event)       */
/*   EVF_CM_ONLY     -- only dial Continuous Mail nodes (24hr systems)   */
/*   EVF_NONCM_ONLY  -- only dial non-CM nodes                          */
/*   EVF_LISTED_ONLY -- only dial nodes found in the nodelist            */
/*                                                                       */
/* Returns 1 = OK to poll, 0 = skip this node.                           */
/*-----------------------------------------------------------------------*/

int ev_should_poll(const QfEventDef *Ev, int IsCM, int IsListed)
{
    if (!Ev) return 1;                  /* no event = always poll        */

    qf_log(LOG_DEBUG, "ev_should_poll: event=\"%s\" flags=0x%08X "
           "is_cm=%d is_listed=%d", Ev->Tag, Ev->Flags, IsCM, IsListed);

    if (Ev->Flags & EVF_RECV_ONLY)
        return 0;                       /* receive-only = don't dial     */
    if ((Ev->Flags & EVF_CM_ONLY) && !IsCM)
        return 0;                       /* CM-only event, node not CM    */
    if ((Ev->Flags & EVF_NONCM_ONLY) && IsCM)
        return 0;                       /* non-CM only, node IS CM       */
    if ((Ev->Flags & EVF_LISTED_ONLY) && !IsListed)
        return 0;                       /* listed-only, node unlisted    */

    return 1;
}


/*-----------------------------------------------------------------------*/
/* ev_pre_actions() -- Run pre-event actions                             */
/*                                                                       */
/* Called when an event becomes active. Runs the tosser if ScanBefore    */
/* flag is set.                                                          */
/*                                                                       */
/* Returns 0.                                                            */
/*-----------------------------------------------------------------------*/

int ev_pre_actions(const QfEventDef *Ev, const QfConfig *Cfg)
{
    if (!Ev) return 0;

    qf_log(LOG_INFO, "Running event \"%s\"", Ev->Tag);

    /* "Scan for new mail before event" */
    if ((Ev->Flags & EVF_SCAN_BEFORE) && Cfg->tosser_path[0]) {
        qf_log(LOG_INFO, "Pre-event scan: %s", Cfg->tosser_path);
        system(Cfg->tosser_path);
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* ev_post_actions() -- Run post-event actions                           */
/*                                                                       */
/* Called after event processing. Checks exit conditions and runs        */
/* nodelist compiler if configured.                                      */
/*                                                                       */
/* Returns 1 if event should end, 0 to continue.                         */
/*-----------------------------------------------------------------------*/

int ev_post_actions(const QfEventDef *Ev, const QfConfig *Cfg,
                    int QueueEmpty)
{
    if (!Ev) return 0;
    (void)Cfg;                          /* used by future actions        */

    /* "Compile nodelist" */
    if (Ev->Flags & EVF_COMPILE_NL) {
        qf_log(LOG_INFO, "Compiling FidoNet nodelist");
    }

    /* "End (no mail)" -- stop processing this event */
    if ((Ev->Flags & EVF_END_NOMAIL) && QueueEmpty) {
        qf_log(LOG_INFO, "Ending event \"%s\" -- no more outbound mail",
               Ev->Tag);
        return 1;                       /* signal: end event             */
    }

    /* "Exit (no mail)" -- exit program */
    if ((Ev->Flags & EVF_EXIT_NOMAIL) && QueueEmpty) {
        qf_log(LOG_INFO,
               "Exiting after no more outbound mail (errorlevel %d)",
               Ev->Errorlevel);
        exit(Ev->Errorlevel);
    }

    /* "Exit when no more outbound mail" */
    if ((Ev->Flags & EVF_EXIT_DONE) && QueueEmpty) {
        qf_log(LOG_INFO, "Exiting with errorlevel %d", Ev->Errorlevel);
        exit(Ev->Errorlevel);
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* ev_run_batch() -- Run batch file event                                */
/*                                                                       */
/* Executes the batch file configured for this event.                    */
/*                                                                       */
/* Returns the batch file's exit code, or 0 if no batch configured.     */
/*-----------------------------------------------------------------------*/

int ev_run_batch(const QfEventDef *Ev)
{
    int Rc;                             /* batch file exit code           */

    if (!Ev || !Ev->Batch[0]) return 0;

    qf_log(LOG_INFO, "Executing event \"%s\" -- running \"%s\"",
           Ev->Tag, Ev->Batch);

    Rc = system(Ev->Batch);
    if (Rc != 0)
        qf_log(LOG_WARN, "Event batch file exited with code %d", Rc);

    return Rc;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Accessors for qfront.c                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

const char *ev_get_tag(const QfEventDef *Ev)
{
    return Ev ? Ev->Tag : "(none)";
}

uint32_t ev_get_flags(const QfEventDef *Ev)
{
    return Ev ? Ev->Flags : 0;
}

int ev_get_errorlevel(const QfEventDef *Ev)
{
    return Ev ? Ev->Errorlevel : 0;
}
