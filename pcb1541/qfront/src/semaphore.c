/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* semaphore.c -- Multi-Node Semaphore Management                           */
/*                                                                           */
/* Prevents conflicts between multiple QFront instances running on           */
/* different BBS nodes. Manages state files:                                 */
/*   QPOLLED.DAT   -- recently polled nodes (cooldown)                      */
/*   QUNDIAL.DAT   -- undialable nodes (max retries exceeded)               */
/*   QQUEUE.DAT    -- outbound queue serialization                          */
/*   QFIXUPS.DAT   -- incomplete transfers to retry                         */
/*                                                                           */
/* Also handles semaphore file exit triggers:                                */
/*   "Semaphore file <name> found, exiting with errorlevel <n>"             */
/*                                                                           */
/* Clean-room from QFront binary analysis.                                   */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Polled Node Tracking                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Prevents re-polling a node within a cooldown window.
 * Original QFront stored this in QPOLLED.DAT. */

#define MAX_POLLED 512                  /* max tracked polled nodes      */

typedef struct {
    FTN_ADDR Addr;                      /* node address                  */
    time_t   When;                      /* when we last polled           */
} PolledEntry;

static PolledEntry g_Polled[MAX_POLLED];  /* polled node table           */
static int         g_PolledCount = 0;     /* entries in polled table     */


/*-----------------------------------------------------------------------*/
/* sem_mark_polled() -- Record that we just polled a node                */
/*                                                                       */
/* Stores the current timestamp for this address. sem_was_polled()       */
/* checks this to enforce the cooldown window (retry_delay seconds)      */
/* between successive calls to the same node.                            */
/*                                                                       */
/* Without cooldown, the mailer would redial a busy node on every        */
/* loop iteration (every 1-5 seconds), wasting phone charges and         */
/* annoying the remote sysop.                                            */
/*-----------------------------------------------------------------------*/

void sem_mark_polled(const FTN_ADDR *Addr)
{
    int  i;                             /* search index                  */
    char AddrBuf[64];                   /* formatted address for log     */

    ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
    qf_log(LOG_DEBUG, "sem_mark_polled: %s", AddrBuf);

    /* Check if already in list -- update timestamp */
    for (i = 0; i < g_PolledCount; i++) {
        if (ftn_addr_equal(&g_Polled[i].Addr, Addr)) {
            g_Polled[i].When = time(NULL);
            return;
        }
    }

    /* Add new entry */
    if (g_PolledCount < MAX_POLLED) {
        g_Polled[g_PolledCount].Addr = *Addr;
        g_Polled[g_PolledCount].When = time(NULL);
        g_PolledCount++;
    }
}


/*-----------------------------------------------------------------------*/
/* sem_was_polled() -- Check if we polled a node recently                 */
/*                                                                       */
/* Returns 1 if the node was polled within CooldownSec seconds.          */
/* Returns 0 if cooldown has expired or node was never polled.           */
/*-----------------------------------------------------------------------*/

int sem_was_polled(const FTN_ADDR *Addr, int CooldownSec)
{
    int    i;                           /* search index                  */
    time_t Now = time(NULL);            /* current time                  */

    for (i = 0; i < g_PolledCount; i++) {
        if (ftn_addr_equal(&g_Polled[i].Addr, Addr)) {
            if ((Now - g_Polled[i].When) < CooldownSec)
                return 1;               /* recently polled -- skip       */
            return 0;                   /* cooldown expired              */
        }
    }

    return 0;                           /* never polled                  */
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Undialable Node Tracking                             */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Nodes that have failed too many consecutive sessions.
 * Original QFront: "Mark undialable after 3 days" */

#define MAX_UNDIAL 256                  /* max tracked undialable nodes  */

typedef struct {
    FTN_ADDR Addr;                      /* node address                  */
    int      FailCount;                 /* consecutive failures          */
    time_t   FirstFail;                 /* when failures started         */
    time_t   LastFail;                  /* most recent failure           */
} UndialEntry;

static UndialEntry g_Undial[MAX_UNDIAL];  /* undialable node table       */
static int         g_UndialCount = 0;     /* entries in undial table     */


/*-----------------------------------------------------------------------*/
/* sem_record_failure() -- Record a session failure for a node           */
/*                                                                       */
/* Increments the consecutive failure counter. If the node is already    */
/* in the undial table, updates the counter and timestamp. Otherwise     */
/* adds a new entry.                                                     */
/*-----------------------------------------------------------------------*/

void sem_record_failure(const FTN_ADDR *Addr)
{
    int    i;                           /* search index                  */
    time_t Now = time(NULL);            /* current time                  */
    char   AddrBuf[64];                 /* formatted address for log     */

    /* Find existing entry */
    for (i = 0; i < g_UndialCount; i++) {
        if (ftn_addr_equal(&g_Undial[i].Addr, Addr)) {
            g_Undial[i].FailCount++;
            g_Undial[i].LastFail = Now;
            ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
            qf_log(LOG_DEBUG, "Node %s: %d consecutive failures",
                   AddrBuf, g_Undial[i].FailCount);
            return;
        }
    }

    /* New entry */
    if (g_UndialCount < MAX_UNDIAL) {
        g_Undial[g_UndialCount].Addr      = *Addr;
        g_Undial[g_UndialCount].FailCount  = 1;
        g_Undial[g_UndialCount].FirstFail  = Now;
        g_Undial[g_UndialCount].LastFail   = Now;
        g_UndialCount++;
    }
}


/*-----------------------------------------------------------------------*/
/* sem_record_success() -- Record a session success for a node           */
/*                                                                       */
/* Clears the failure counter by removing the node from the undial       */
/* table. Uses swap-with-last for O(1) removal.                          */
/*-----------------------------------------------------------------------*/

void sem_record_success(const FTN_ADDR *Addr)
{
    int i;                              /* search index                  */

    for (i = 0; i < g_UndialCount; i++) {
        if (ftn_addr_equal(&g_Undial[i].Addr, Addr)) {
            /* Remove by swapping with last entry */
            g_Undial[i] = g_Undial[--g_UndialCount];
            return;
        }
    }
}


/*-----------------------------------------------------------------------*/
/* sem_is_undialable() -- Check if a node is undialable                  */
/*                                                                       */
/* A node is undialable if:                                              */
/*   - It has failed for 3+ consecutive days (from QFCONFIG), OR        */
/*   - It has exceeded MaxRetries consecutive failures                   */
/*                                                                       */
/* Returns 1 if undialable, 0 if clear to call.                          */
/*-----------------------------------------------------------------------*/

int sem_is_undialable(const FTN_ADDR *Addr, int MaxRetries)
{
    int    i;                           /* search index                  */
    time_t Now = time(NULL);            /* current time                  */

    for (i = 0; i < g_UndialCount; i++) {
        if (ftn_addr_equal(&g_Undial[i].Addr, Addr)) {
            /* "Mark undialable after 3 days" -- from QFCONFIG */
            if ((Now - g_Undial[i].FirstFail) > (3 * 24 * 3600)) {
                char AddrBuf[64];       /* formatted address for log     */
                ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
                qf_log(LOG_INFO, "Node %s is undialable (3+ days of failures)",
                       AddrBuf);
                return 1;
            }
            /* Also check max retries per day */
            if (g_Undial[i].FailCount >= MaxRetries)
                return 1;
            return 0;
        }
    }

    return 0;                           /* not in undial table           */
}


/*-----------------------------------------------------------------------*/
/* sem_clear_undialable() -- Clear undialable status (sysop override)    */
/*                                                                       */
/* Removes a node from the undial table. Called when the sysop manually  */
/* clears a node's undialable status via QFUTIL.                         */
/*-----------------------------------------------------------------------*/

void sem_clear_undialable(const FTN_ADDR *Addr)
{
    int i;                              /* search index                  */

    for (i = 0; i < g_UndialCount; i++) {
        if (ftn_addr_equal(&g_Undial[i].Addr, Addr)) {
            g_Undial[i] = g_Undial[--g_UndialCount];
            return;
        }
    }
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                     Semaphore File Exit Triggers                           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* QFront checks for semaphore files periodically.
 * From binary: "Semaphore file <name> found, exiting with errorlevel <n>"
 *
 * Configured in qfront.cfg:
 *   Semaphore=<filename>:<errorlevel>
 */

#define MAX_SEM_FILES 16                /* max semaphore triggers         */

typedef struct {
    char Path[260];                     /* semaphore file path           */
    int  Errorlevel;                    /* exit code when triggered      */
} SemTrigger;

static SemTrigger g_SemTriggers[MAX_SEM_FILES];  /* trigger table        */
static int        g_NumSemTriggers = 0;           /* trigger count       */


/*-----------------------------------------------------------------------*/
/* sem_add_trigger() -- Add a semaphore trigger                          */
/*                                                                       */
/* Called during config parsing for each Semaphore= line.                */
/*-----------------------------------------------------------------------*/

void sem_add_trigger(const char *Path, int Errorlevel)
{
    if (g_NumSemTriggers >= MAX_SEM_FILES) return;
    strncpy(g_SemTriggers[g_NumSemTriggers].Path, Path, 259);
    g_SemTriggers[g_NumSemTriggers].Errorlevel = Errorlevel;
    g_NumSemTriggers++;
}


/*-----------------------------------------------------------------------*/
/* sem_check_triggers() -- Check for semaphore exit trigger files         */
/*                                                                       */
/* Semaphore files are a simple IPC mechanism for multi-node BBS systems. */
/* Another program (PCBoard, a door game, a batch file) creates a file    */
/* with a specific name, and QFront checks for it periodically. When      */
/* found, QFront deletes the file and exits with the configured           */
/* errorlevel.                                                            */
/*                                                                       */
/* This allows external programs to tell QFront to:                      */
/*   - Reload configuration (exit and restart via BOARD.BAT)             */
/*   - Compile nodelist (exit with errorlevel that triggers QNLIST)      */
/*   - Shut down for maintenance                                        */
/*                                                                       */
/* Returns: errorlevel if a trigger file was found, -1 if none.          */
/*-----------------------------------------------------------------------*/

int sem_check_triggers(void)
{
    int   i;                            /* trigger loop index            */
    FILE *f;                            /* test file handle              */

    qf_log(LOG_DEBUG, "sem_check_triggers: checking %d trigger files",
           g_NumSemTriggers);

    for (i = 0; i < g_NumSemTriggers; i++) {
        f = fopen(g_SemTriggers[i].Path, "r");
        if (f) {
            fclose(f);
            /* Remove the semaphore file after detecting it */
            remove(g_SemTriggers[i].Path);

            qf_log(LOG_INFO,
                   "Semaphore file \"%s\" found, exiting with errorlevel %d",
                   g_SemTriggers[i].Path, g_SemTriggers[i].Errorlevel);

            return g_SemTriggers[i].Errorlevel;
        }
    }

    return -1;                          /* no semaphore found            */
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                         Save/Load State                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Persist polled and undialable lists across restarts. */


/*-----------------------------------------------------------------------*/
/* sem_save_state() -- Save polled and undialable state to disk          */
/*                                                                       */
/* Writes QPOLLED.DAT and QUNDIAL.DAT to the specified directory.        */
/* Called on clean shutdown so state survives restart.                    */
/*-----------------------------------------------------------------------*/

void sem_save_state(const char *Dir)
{
    char  Path[260];                    /* file path buffer              */
    FILE *f;                            /* output file handle            */
    int   i;                            /* loop index                    */

    /* Save QPOLLED.DAT */
    snprintf(Path, sizeof(Path), "%s%cQPOLLED.DAT", Dir, PATH_SEP);
    f = fopen(Path, "wb");
    if (f) {
        for (i = 0; i < g_PolledCount; i++) {
            fprintf(f, "%u:%u/%u %ld\n",
                    g_Polled[i].Addr.zone,
                    g_Polled[i].Addr.net,
                    g_Polled[i].Addr.node,
                    (long)g_Polled[i].When);
        }
        fclose(f);
    }

    /* Save QUNDIAL.DAT */
    snprintf(Path, sizeof(Path), "%s%cQUNDIAL.DAT", Dir, PATH_SEP);
    f = fopen(Path, "wb");
    if (f) {
        for (i = 0; i < g_UndialCount; i++) {
            fprintf(f, "%u:%u/%u %d %ld %ld\n",
                    g_Undial[i].Addr.zone,
                    g_Undial[i].Addr.net,
                    g_Undial[i].Addr.node,
                    g_Undial[i].FailCount,
                    (long)g_Undial[i].FirstFail,
                    (long)g_Undial[i].LastFail);
        }
        fclose(f);
    }
}


/*-----------------------------------------------------------------------*/
/* sem_load_state() -- Load polled and undialable state from disk        */
/*                                                                       */
/* Reads QPOLLED.DAT and QUNDIAL.DAT from the specified directory.       */
/* Called on startup to restore state from previous session.              */
/*-----------------------------------------------------------------------*/

void sem_load_state(const char *Dir)
{
    char  Path[260];                    /* file path buffer              */
    char  Line[256];                    /* line read buffer              */
    FILE *f;                            /* input file handle             */

    /* Load QPOLLED.DAT */
    snprintf(Path, sizeof(Path), "%s%cQPOLLED.DAT", Dir, PATH_SEP);
    f = fopen(Path, "r");
    if (f) {
        while (fgets(Line, sizeof(Line), f) && g_PolledCount < MAX_POLLED) {
            char AddrStr[64];           /* address string from file      */
            long When;                  /* timestamp from file           */

            if (sscanf(Line, "%63s %ld", AddrStr, &When) == 2) {
                ftn_parse_addr(AddrStr, &g_Polled[g_PolledCount].Addr);
                g_Polled[g_PolledCount].When = (time_t)When;
                g_PolledCount++;
            }
        }
        fclose(f);
    }

    /* Load QUNDIAL.DAT */
    snprintf(Path, sizeof(Path), "%s%cQUNDIAL.DAT", Dir, PATH_SEP);
    f = fopen(Path, "r");
    if (f) {
        while (fgets(Line, sizeof(Line), f) && g_UndialCount < MAX_UNDIAL) {
            char AddrStr[64];           /* address string from file      */
            int  Fc;                    /* fail count from file          */
            long Ff;                    /* first fail timestamp          */
            long Lf;                    /* last fail timestamp           */

            if (sscanf(Line, "%63s %d %ld %ld", AddrStr, &Fc, &Ff, &Lf) == 4) {
                ftn_parse_addr(AddrStr, &g_Undial[g_UndialCount].Addr);
                g_Undial[g_UndialCount].FailCount  = Fc;
                g_Undial[g_UndialCount].FirstFail  = (time_t)Ff;
                g_Undial[g_UndialCount].LastFail   = (time_t)Lf;
                g_UndialCount++;
            }
        }
        fclose(f);
    }
}
