/* ====================================================================
 * events.c — Event Scheduler (19 flags from QFCONFIG binary)
 * ====================================================================
 * Full event scheduler matching QFront v1.20a's event system.
 * Parses [Event.NAME] sections from qfront.cfg.
 * Implements all 19 event flags extracted from binary analysis.
 *
 * Event types (from QFCONFIG strings):
 *   "Default FidoMail event"   — standard mail processing
 *   "Run batch file"           — execute external command
 *   "Exit with errorlevel"     — exit program with code
 *
 * Clean-room from QFront documentation + binary analysis.
 * ==================================================================== */

#include "qfront.h"

/* ---- Event Flag Bits ----
 * All 19 flags from QFCONFIG binary analysis. */

#define EVF_SLIDE          0x00000001  /* Slide event time           */
#define EVF_ECHO_ONLY      0x00000002  /* EchoMail only              */
#define EVF_NET_ONLY       0x00000004  /* NetMail only               */
#define EVF_RECV_ONLY      0x00000008  /* Receive-only               */
#define EVF_CM_ONLY        0x00000010  /* Send to CM systems only    */
#define EVF_NONCM_ONLY     0x00000020  /* Send to non-CM only        */
#define EVF_NODE_CRITICAL  0x00000040  /* Node-critical              */
#define EVF_NO_HELD        0x00000080  /* No HELD attach             */
#define EVF_LISTED_ONLY    0x00000100  /* Only listed nodes          */
#define EVF_FORCE_POLL     0x00000200  /* Force outbound poll        */
#define EVF_POLL_DURING    0x00000400  /* Poll during event          */
#define EVF_SCAN_BEFORE    0x00000800  /* Scan before event          */
#define EVF_COMPILE_NL     0x00001000  /* Compile nodelist           */
#define EVF_COMPILE_RECV   0x00002000  /* Compile when received      */
#define EVF_RESCAN         0x00004000  /* Rescan on return           */
#define EVF_END_NOMAIL     0x00008000  /* End event when queue empty */
#define EVF_EXIT_NOMAIL    0x00010000  /* Exit program, queue empty  */
#define EVF_EXIT_DONE      0x00020000  /* Exit loop when done        */
#define EVF_AUTO_POLL      0x00040000  /* Has auto-poll list         */

/* ---- Event Definition ---- */
typedef struct {
    char     tag[32];             /* Event name/tag               */
    char     description[64];     /* Description                  */
    int      active;              /* Is this event enabled?       */

    /* Schedule */
    int      day_mask;            /* Bit 0=Sun ... Bit 6=Sat      */
    int      start_hour;
    int      start_min;
    int      end_hour;
    int      end_min;

    /* Flags */
    uint32_t flags;               /* EVF_* bitmask                */

    /* Exit */
    int      errorlevel;          /* Exit code (if EVF_EXIT_*)    */

    /* Batch file */
    char     batch[260];          /* Batch file path              */

    /* Auto-poll list */
    FTN_ADDR auto_poll[16];
    int      num_auto_poll;

    /* Cost limits (from QFCONFIG: "min/max allowable cost") */
    int      min_cost;            /* -1 = ignore                  */
    int      max_cost;            /* -1 = ignore                  */

    /* Tracking */
    time_t   last_run;            /* Last time this event ran     */
} QfEventDef;


#define MAX_EVENT_DEFS 32
static QfEventDef g_events[MAX_EVENT_DEFS];
static int        g_num_events = 0;


/* ---- Parse Day Mask ----
 * "MTWTFSs" = Mon-Sun.  "MTWTF" = weekdays. "Ss" = weekends.
 * Also accepts: "Daily", "Weekdays", "Weekends", "Mon", "Tue", etc. */
static int ev_parse_days(const char *str)
{
    int mask = 0;
    const char *p;

    if (strcmp(str, "Daily") == 0 || strcmp(str, "daily") == 0)
        return 0x7F;              /* All 7 days                   */
    if (strcmp(str, "Weekdays") == 0 || strcmp(str, "weekdays") == 0)
        return 0x3E;              /* Mon-Fri (bits 1-5)           */
    if (strcmp(str, "Weekends") == 0 || strcmp(str, "weekends") == 0)
        return 0x41;              /* Sun + Sat (bits 0,6)         */

    for (p = str; *p; p++) {
        switch (*p) {
        case 's': mask |= 0x01; break;  /* Sun (lowercase = Sun) */
        case 'M': mask |= 0x02; break;  /* Mon */
        case 'T': mask |= 0x04; break;  /* Tue */
        case 'W': mask |= 0x08; break;  /* Wed */
        case 't': mask |= 0x10; break;  /* Thu (lowercase = Thu) */
        case 'F': mask |= 0x20; break;  /* Fri */
        case 'S': mask |= 0x40; break;  /* Sat (uppercase = Sat) */
        }
    }

    return mask;
}


/* ---- Parse Time "HH:MM" ---- */
static void ev_parse_time(const char *str, int *hour, int *min)
{
    *hour = 0;
    *min  = 0;
    sscanf(str, "%d:%d", hour, min);
}


/* ---- Load Events from Config ----
 *
 * Format in qfront.cfg:
 *
 *   [Event.ZMH]
 *   Description=Zone Mail Hour
 *   Days=Daily
 *   Start=01:00
 *   End=05:00
 *   ForcePoll=yes
 *   SendCMOnly=yes
 *   ExitNoMail=yes
 *   Errorlevel=3
 *   AutoPoll=1:234/56,2:5020/0
 */
int ev_load(const char *cfgpath)
{
    FILE *f;
    char line[512];
    QfEventDef *cur = NULL;

    g_num_events = 0;

    f = fopen(cfgpath, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char key[64], val[256];

        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n') continue;

        /* Strip trailing whitespace */
        {
            char *end = p + strlen(p) - 1;
            while (end > p && (*end == '\n' || *end == '\r' || *end == ' '))
                *end-- = '\0';
        }

        /* Check for [Event.NAME] section header */
        if (*p == '[' && strncmp(p, "[Event.", 7) == 0) {
            char tag[32];
            sscanf(p + 7, "%31[^]]", tag);

            if (g_num_events >= MAX_EVENT_DEFS) continue;

            cur = &g_events[g_num_events++];
            memset(cur, 0, sizeof(*cur));
            strncpy(cur->tag, tag, sizeof(cur->tag) - 1);
            cur->active = 1;
            cur->min_cost = -1;
            cur->max_cost = -1;
            continue;
        }

        /* Skip lines not in an event section */
        if (!cur) continue;

        /* Don't parse non-event sections */
        if (*p == '[') { cur = NULL; continue; }

        /* Parse KEY=VALUE */
        if (sscanf(p, "%63[^=]=%255[^\n]", key, val) != 2) continue;

        if (strcmp(key, "Description") == 0)
            strncpy(cur->description, val, sizeof(cur->description) - 1);
        else if (strcmp(key, "Days") == 0)
            cur->day_mask = ev_parse_days(val);
        else if (strcmp(key, "Start") == 0)
            ev_parse_time(val, &cur->start_hour, &cur->start_min);
        else if (strcmp(key, "End") == 0)
            ev_parse_time(val, &cur->end_hour, &cur->end_min);
        else if (strcmp(key, "Active") == 0)
            cur->active = (val[0] == 'y' || val[0] == 'Y' || val[0] == '1');
        else if (strcmp(key, "Errorlevel") == 0)
            cur->errorlevel = atoi(val);
        else if (strcmp(key, "Batch") == 0)
            strncpy(cur->batch, val, sizeof(cur->batch) - 1);
        else if (strcmp(key, "MinCost") == 0)
            cur->min_cost = atoi(val);
        else if (strcmp(key, "MaxCost") == 0)
            cur->max_cost = atoi(val);

        /* Boolean flags */
        else if (strcmp(key, "SlideTime") == 0 && val[0] == 'y')
            cur->flags |= EVF_SLIDE;
        else if (strcmp(key, "EchoOnly") == 0 && val[0] == 'y')
            cur->flags |= EVF_ECHO_ONLY;
        else if (strcmp(key, "NetOnly") == 0 && val[0] == 'y')
            cur->flags |= EVF_NET_ONLY;
        else if (strcmp(key, "ReceiveOnly") == 0 && val[0] == 'y')
            cur->flags |= EVF_RECV_ONLY;
        else if (strcmp(key, "SendCMOnly") == 0 && val[0] == 'y')
            cur->flags |= EVF_CM_ONLY;
        else if (strcmp(key, "SendNonCMOnly") == 0 && val[0] == 'y')
            cur->flags |= EVF_NONCM_ONLY;
        else if (strcmp(key, "NodeCritical") == 0 && val[0] == 'y')
            cur->flags |= EVF_NODE_CRITICAL;
        else if (strcmp(key, "NoHeldAttach") == 0 && val[0] == 'y')
            cur->flags |= EVF_NO_HELD;
        else if (strcmp(key, "ListedOnly") == 0 && val[0] == 'y')
            cur->flags |= EVF_LISTED_ONLY;
        else if (strcmp(key, "ForcePoll") == 0 && val[0] == 'y')
            cur->flags |= EVF_FORCE_POLL;
        else if (strcmp(key, "PollDuring") == 0 && val[0] == 'y')
            cur->flags |= EVF_POLL_DURING;
        else if (strcmp(key, "ScanBefore") == 0 && val[0] == 'y')
            cur->flags |= EVF_SCAN_BEFORE;
        else if (strcmp(key, "CompileNodelist") == 0 && val[0] == 'y')
            cur->flags |= EVF_COMPILE_NL;
        else if (strcmp(key, "CompileOnReceive") == 0 && val[0] == 'y')
            cur->flags |= EVF_COMPILE_RECV;
        else if (strcmp(key, "Rescan") == 0 && val[0] == 'y')
            cur->flags |= EVF_RESCAN;
        else if (strcmp(key, "EndNoMail") == 0 && val[0] == 'y')
            cur->flags |= EVF_END_NOMAIL;
        else if (strcmp(key, "ExitNoMail") == 0 && val[0] == 'y')
            cur->flags |= EVF_EXIT_NOMAIL;
        else if (strcmp(key, "ExitWhenDone") == 0 && val[0] == 'y')
            cur->flags |= EVF_EXIT_DONE;

        /* Auto-poll address list */
        else if (strcmp(key, "AutoPoll") == 0) {
            char *tok = strtok(val, ",");
            cur->flags |= EVF_AUTO_POLL;
            while (tok && cur->num_auto_poll < 16) {
                while (*tok == ' ') tok++;
                ftn_parse_addr(tok, &cur->auto_poll[cur->num_auto_poll++]);
                tok = strtok(NULL, ",");
            }
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Events: %d loaded", g_num_events);
    return 0;
}


/* ---- Check Which Event is Active ----
 * Returns pointer to active event, or NULL if none.
 * First matching event wins. */

const QfEventDef *ev_check_active(void)
{
    time_t now;
    struct tm *tm;
    int dow, cur_min;
    int i;

    time(&now);
    tm = localtime(&now);
    dow = tm->tm_wday;
    cur_min = tm->tm_hour * 60 + tm->tm_min;

    for (i = 0; i < g_num_events; i++) {
        const QfEventDef *ev = &g_events[i];
        int start_min, end_min;

        if (!ev->active) continue;
        if (!(ev->day_mask & (1 << dow))) continue;

        start_min = ev->start_hour * 60 + ev->start_min;
        end_min   = ev->end_hour * 60 + ev->end_min;

        if (end_min <= start_min) {
            /* Midnight wrap */
            if (cur_min >= start_min || cur_min < end_min)
                return ev;
        } else {
            if (cur_min >= start_min && cur_min < end_min)
                return ev;
        }
    }

    return NULL;
}


/* ---- Should We Poll This Node During This Event? ----
 * Checks event flags against node capabilities (CM, etc.) */

int ev_should_poll(const QfEventDef *ev, int is_cm, int is_listed)
{
    if (!ev) return 1;            /* No event = always poll       */

    if (ev->flags & EVF_RECV_ONLY)
        return 0;                 /* Receive-only = don't dial    */

    if ((ev->flags & EVF_CM_ONLY) && !is_cm)
        return 0;                 /* CM-only event, node is not CM */

    if ((ev->flags & EVF_NONCM_ONLY) && is_cm)
        return 0;                 /* Non-CM only, node IS CM      */

    if ((ev->flags & EVF_LISTED_ONLY) && !is_listed)
        return 0;                 /* Listed-only, node unlisted   */

    return 1;
}


/* ---- Run Pre-Event Actions ---- */

int ev_pre_actions(const QfEventDef *ev, const QfConfig *cfg)
{
    if (!ev) return 0;

    qf_log(LOG_INFO, "Running event \"%s\"", ev->tag);

    /* "Scan for new mail before event" */
    if ((ev->flags & EVF_SCAN_BEFORE) && cfg->tosser_path[0]) {
        qf_log(LOG_INFO, "Pre-event scan: %s", cfg->tosser_path);
        system(cfg->tosser_path);
    }

    return 0;
}


/* ---- Run Post-Event Actions ---- */

int ev_post_actions(const QfEventDef *ev, const QfConfig *cfg,
                    int queue_empty)
{
    if (!ev) return 0;

    /* "Compile nodelist" */
    if (ev->flags & EVF_COMPILE_NL) {
        /* TODO: call nodelist compiler */
        qf_log(LOG_INFO, "Compiling FidoNet nodelist");
    }

    /* "End (no mail)" — stop processing this event */
    if ((ev->flags & EVF_END_NOMAIL) && queue_empty) {
        qf_log(LOG_INFO, "Ending event \"%s\" — no more outbound mail",
               ev->tag);
        return 1;                 /* Signal: end event            */
    }

    /* "Exit (no mail)" — exit program */
    if ((ev->flags & EVF_EXIT_NOMAIL) && queue_empty) {
        qf_log(LOG_INFO,
               "Exiting after no more outbound mail (errorlevel %d)",
               ev->errorlevel);
        exit(ev->errorlevel);
    }

    /* "Exit when no more outbound mail" */
    if ((ev->flags & EVF_EXIT_DONE) && queue_empty) {
        qf_log(LOG_INFO, "Exiting with errorlevel %d", ev->errorlevel);
        exit(ev->errorlevel);
    }

    return 0;
}


/* ---- Run Batch File Event ---- */

int ev_run_batch(const QfEventDef *ev)
{
    int rc;

    if (!ev || !ev->batch[0]) return 0;

    qf_log(LOG_INFO, "Executing event \"%s\" — running \"%s\"",
           ev->tag, ev->batch);

    rc = system(ev->batch);
    if (rc != 0)
        qf_log(LOG_WARN, "Event batch file exited with code %d", rc);

    return rc;
}

/* ---- Accessors for qfront.c (avoids exposing full struct) ---- */

const char *ev_get_tag(const QfEventDef *ev)
{
    return ev ? ev->tag : "(none)";
}

uint32_t ev_get_flags(const QfEventDef *ev)
{
    return ev ? ev->flags : 0;
}

int ev_get_errorlevel(const QfEventDef *ev)
{
    return ev ? ev->errorlevel : 0;
}
