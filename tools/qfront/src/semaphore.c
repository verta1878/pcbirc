/* ====================================================================
 * semaphore.c — Multi-Node Semaphore Management
 * ====================================================================
 * Prevents conflicts between multiple QFront instances running on
 * different BBS nodes. Manages state files identified from binary:
 *
 *   QQUEUE.DAT    — outbound queue serialization
 *   QPOLLED.DAT   — recently polled nodes (cooldown)
 *   QUNDIAL.DAT   — undialable nodes (max retries exceeded)
 *   QFIXUPS.DAT   — incomplete transfers to retry
 *
 * Also handles semaphore file exit triggers:
 *   "Semaphore file <name> found, exiting with errorlevel <n>"
 *
 * Clean-room from QFront binary analysis.
 * ==================================================================== */

#include "qfront.h"

/* ---- Polled Node Tracking ----
 * Prevents re-polling a node within a cooldown window.
 * Original QFront stored this in QPOLLED.DAT. */

#define MAX_POLLED 512

typedef struct {
    FTN_ADDR addr;
    time_t   when;                /* When we last polled          */
} PolledEntry;

static PolledEntry g_polled[MAX_POLLED];
static int         g_polled_count = 0;


/* Record that we polled a node. */
void sem_mark_polled(const FTN_ADDR *addr)
{
    int i;

    /* Check if already in list — update timestamp */
    for (i = 0; i < g_polled_count; i++) {
        if (ftn_addr_equal(&g_polled[i].addr, addr)) {
            g_polled[i].when = time(NULL);
            return;
        }
    }

    /* Add new entry */
    if (g_polled_count < MAX_POLLED) {
        g_polled[g_polled_count].addr = *addr;
        g_polled[g_polled_count].when = time(NULL);
        g_polled_count++;
    }
}

/* Check if we polled this node recently (within cooldown_sec). */
int sem_was_polled(const FTN_ADDR *addr, int cooldown_sec)
{
    int i;
    time_t now = time(NULL);

    for (i = 0; i < g_polled_count; i++) {
        if (ftn_addr_equal(&g_polled[i].addr, addr)) {
            if ((now - g_polled[i].when) < cooldown_sec)
                return 1;         /* Recently polled — skip       */
            return 0;             /* Cooldown expired              */
        }
    }

    return 0;                     /* Never polled                  */
}


/* ---- Undialable Node Tracking ----
 * Nodes that have failed too many consecutive sessions.
 * Original QFront: "Mark undialable after 3 days" */

#define MAX_UNDIAL 256

typedef struct {
    FTN_ADDR addr;
    int      fail_count;          /* Consecutive failures         */
    time_t   first_fail;          /* When failures started        */
    time_t   last_fail;           /* Most recent failure          */
} UndialEntry;

static UndialEntry g_undial[MAX_UNDIAL];
static int         g_undial_count = 0;


/* Record a session failure for a node. */
void sem_record_failure(const FTN_ADDR *addr)
{
    int i;
    time_t now = time(NULL);
    char buf[64];

    /* Find existing entry */
    for (i = 0; i < g_undial_count; i++) {
        if (ftn_addr_equal(&g_undial[i].addr, addr)) {
            g_undial[i].fail_count++;
            g_undial[i].last_fail = now;
            ftn_format_addr(addr, buf, sizeof(buf));
            qf_log(LOG_DEBUG, "Node %s: %d consecutive failures",
                   buf, g_undial[i].fail_count);
            return;
        }
    }

    /* New entry */
    if (g_undial_count < MAX_UNDIAL) {
        g_undial[g_undial_count].addr = *addr;
        g_undial[g_undial_count].fail_count = 1;
        g_undial[g_undial_count].first_fail = now;
        g_undial[g_undial_count].last_fail = now;
        g_undial_count++;
    }
}

/* Record a session success — clears failure counter. */
void sem_record_success(const FTN_ADDR *addr)
{
    int i;

    for (i = 0; i < g_undial_count; i++) {
        if (ftn_addr_equal(&g_undial[i].addr, addr)) {
            /* Remove from undial list by swapping with last */
            g_undial[i] = g_undial[--g_undial_count];
            return;
        }
    }
}

/* Check if a node is undialable (exceeded max retries or 3 days). */
int sem_is_undialable(const FTN_ADDR *addr, int max_retries)
{
    int i;
    time_t now = time(NULL);

    for (i = 0; i < g_undial_count; i++) {
        if (ftn_addr_equal(&g_undial[i].addr, addr)) {
            /* "Mark undialable after 3 days" — from QFCONFIG */
            if ((now - g_undial[i].first_fail) > (3 * 24 * 3600)) {
                char buf[64];
                ftn_format_addr(addr, buf, sizeof(buf));
                qf_log(LOG_INFO, "Node %s is undialable (3+ days of failures)",
                       buf);
                return 1;
            }
            /* Also check max retries per day */
            if (g_undial[i].fail_count >= max_retries)
                return 1;
            return 0;
        }
    }

    return 0;
}

/* Clear undialable status for a node (sysop override). */
void sem_clear_undialable(const FTN_ADDR *addr)
{
    int i;
    for (i = 0; i < g_undial_count; i++) {
        if (ftn_addr_equal(&g_undial[i].addr, addr)) {
            g_undial[i] = g_undial[--g_undial_count];
            return;
        }
    }
}


/* ---- Semaphore File Exit Triggers ----
 * QFront checks for semaphore files periodically.
 * From binary: "Semaphore file <name> found, exiting with errorlevel <n>"
 *
 * Configured in qfront.cfg:
 *   Semaphore=<filename>:<errorlevel>
 */

#define MAX_SEM_FILES 16

typedef struct {
    char path[260];
    int  errorlevel;
} SemTrigger;

static SemTrigger g_sem_triggers[MAX_SEM_FILES];
static int        g_num_sem_triggers = 0;

/* Add a semaphore trigger. */
void sem_add_trigger(const char *path, int errorlevel)
{
    if (g_num_sem_triggers >= MAX_SEM_FILES) return;
    strncpy(g_sem_triggers[g_num_sem_triggers].path, path, 259);
    g_sem_triggers[g_num_sem_triggers].errorlevel = errorlevel;
    g_num_sem_triggers++;
}

/* Check for semaphore files. Returns errorlevel if found, -1 if none. */
int sem_check_triggers(void)
{
    int i;
    FILE *f;

    for (i = 0; i < g_num_sem_triggers; i++) {
        f = fopen(g_sem_triggers[i].path, "r");
        if (f) {
            fclose(f);
            /* Remove the semaphore file after detecting it */
            remove(g_sem_triggers[i].path);

            qf_log(LOG_INFO,
                   "Semaphore file \"%s\" found, exiting with errorlevel %d",
                   g_sem_triggers[i].path, g_sem_triggers[i].errorlevel);

            return g_sem_triggers[i].errorlevel;
        }
    }

    return -1;                    /* No semaphore found            */
}


/* ---- Save/Load State ----
 * Persist polled and undialable lists across restarts. */

void sem_save_state(const char *dir)
{
    char path[260];
    FILE *f;
    int i;

    /* Save QPOLLED.DAT */
    snprintf(path, sizeof(path), "%s%cQPOLLED.DAT", dir, PATH_SEP);
    f = fopen(path, "wb");
    if (f) {
        for (i = 0; i < g_polled_count; i++) {
            fprintf(f, "%u:%u/%u %ld\n",
                    g_polled[i].addr.zone,
                    g_polled[i].addr.net,
                    g_polled[i].addr.node,
                    (long)g_polled[i].when);
        }
        fclose(f);
    }

    /* Save QUNDIAL.DAT */
    snprintf(path, sizeof(path), "%s%cQUNDIAL.DAT", dir, PATH_SEP);
    f = fopen(path, "wb");
    if (f) {
        for (i = 0; i < g_undial_count; i++) {
            fprintf(f, "%u:%u/%u %d %ld %ld\n",
                    g_undial[i].addr.zone,
                    g_undial[i].addr.net,
                    g_undial[i].addr.node,
                    g_undial[i].fail_count,
                    (long)g_undial[i].first_fail,
                    (long)g_undial[i].last_fail);
        }
        fclose(f);
    }
}

void sem_load_state(const char *dir)
{
    char path[260], line[256];
    FILE *f;

    /* Load QPOLLED.DAT */
    snprintf(path, sizeof(path), "%s%cQPOLLED.DAT", dir, PATH_SEP);
    f = fopen(path, "r");
    if (f) {
        while (fgets(line, sizeof(line), f) && g_polled_count < MAX_POLLED) {
            char addr_str[64];
            long when;
            if (sscanf(line, "%63s %ld", addr_str, &when) == 2) {
                ftn_parse_addr(addr_str, &g_polled[g_polled_count].addr);
                g_polled[g_polled_count].when = (time_t)when;
                g_polled_count++;
            }
        }
        fclose(f);
    }

    /* Load QUNDIAL.DAT */
    snprintf(path, sizeof(path), "%s%cQUNDIAL.DAT", dir, PATH_SEP);
    f = fopen(path, "r");
    if (f) {
        while (fgets(line, sizeof(line), f) && g_undial_count < MAX_UNDIAL) {
            char addr_str[64];
            int fc;
            long ff, lf;
            if (sscanf(line, "%63s %d %ld %ld", addr_str, &fc, &ff, &lf) == 4) {
                ftn_parse_addr(addr_str, &g_undial[g_undial_count].addr);
                g_undial[g_undial_count].fail_count = fc;
                g_undial[g_undial_count].first_fail = (time_t)ff;
                g_undial[g_undial_count].last_fail = (time_t)lf;
                g_undial_count++;
            }
        }
        fclose(f);
    }
}
