/* ====================================================================
 * qfront.c — QFront Main Orchestrator
 * ====================================================================
 * The mailer's main loop — fully wired with all modules:
 *   1. Load config, nodelist, routing rules, events, semaphore state
 *   2. Check event schedule — run pre-event actions
 *   3. Scan BSO outbound for pending nodes
 *   4. For each node: check routing, nodelist, semaphores, events
 *   5. Lock, dispatch session via binkd, unlock, record attempt
 *   6. Post-session: run tosser, TIC processor, check fixups
 *   7. Check semaphore exit triggers
 *   8. Status display, sleep, repeat
 *
 * Clean-room from FTS-5005 (BSO), FTS-0001 (.PKT), FTS-5001 (nodelist).
 * ==================================================================== */

#include "qfront.h"
#include <stdarg.h>

#define QFRONT_VERSION "1.0.0"

/* ---- External module declarations ----
 * These are defined in their respective .c files.
 * We forward-declare here to avoid a massive header. */

/* nodelist.c */
typedef struct NlDatabase NlDatabase;
typedef struct NlEntry NlEntry;
extern NlDatabase *nl_open(const char *path);
extern const NlEntry *nl_lookup(const NlDatabase *db, const FTN_ADDR *addr);
extern const NlEntry *nl_find_host(const NlDatabase *db, const FTN_ADDR *addr);
extern void nl_close(NlDatabase *db);

/* route.c */
extern int rt_load(const char *cfgpath);
extern int rt_resolve(const FTN_ADDR *dest, FTN_ADDR *via_addr);

/* events.c */
typedef struct QfEventDef QfEventDef;
extern int ev_load(const char *cfgpath);
extern const QfEventDef *ev_check_active(void);
extern int ev_should_poll(const QfEventDef *ev, int is_cm, int is_listed);
extern int ev_pre_actions(const QfEventDef *ev, const QfConfig *cfg);
extern int ev_post_actions(const QfEventDef *ev, const QfConfig *cfg, int queue_empty);
extern int ev_run_batch(const QfEventDef *ev);

/* Access event fields without full struct def */
extern const char *ev_get_tag(const QfEventDef *ev);
extern uint32_t ev_get_flags(const QfEventDef *ev);
extern int ev_get_errorlevel(const QfEventDef *ev);

/* semaphore.c */
extern void sem_mark_polled(const FTN_ADDR *addr);
extern int sem_was_polled(const FTN_ADDR *addr, int cooldown_sec);
extern void sem_record_failure(const FTN_ADDR *addr);
extern void sem_record_success(const FTN_ADDR *addr);
extern int sem_is_undialable(const FTN_ADDR *addr, int max_retries);
extern void sem_clear_undialable(const FTN_ADDR *addr);
extern void sem_add_trigger(const char *path, int errorlevel);
extern int sem_check_triggers(void);
extern void sem_save_state(const char *dir);
extern void sem_load_state(const char *dir);

/* tic.c */
extern int tic_process(const QfConfig *cfg);

/* NlEntry field access (avoid exposing full struct) */
extern int nl_entry_is_cm(const NlEntry *e);
extern int nl_entry_is_down(const NlEntry *e);
extern int nl_entry_is_hold(const NlEntry *e);
extern const char *nl_entry_ibn_host(const NlEntry *e);


/* ---- Logging ---- */

static FILE *g_logfp = NULL;
static int   g_debug = 0;

static const char *log_level_str(LogLevel level)
{
    switch (level) {
    case LOG_DEBUG: return "DBG";
    case LOG_INFO:  return "   ";
    case LOG_WARN:  return "WRN";
    case LOG_ERROR: return "ERR";
    case LOG_FATAL: return "FTL";
    default:        return "???";
    }
}

void qf_log_init(const char *logfile)
{
    if (logfile && logfile[0])
        g_logfp = fopen(logfile, "a");
}

void qf_log_close(void)
{
    if (g_logfp) { fclose(g_logfp); g_logfp = NULL; }
}

void qf_log(LogLevel level, const char *fmt, ...)
{
    va_list ap;
    time_t now;
    struct tm *tm;
    char ts[32];
    FILE *out;

    if (level == LOG_DEBUG && !g_debug) return;

    time(&now);
    tm = localtime(&now);
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm);

    out = g_logfp ? g_logfp : stderr;
    fprintf(out, "%s [%s] ", ts, log_level_str(level));
    va_start(ap, fmt); vfprintf(out, fmt, ap); va_end(ap);
    fprintf(out, "\n");
    fflush(out);

    if (g_logfp && level >= LOG_INFO) {
        fprintf(stderr, "%s [%s] ", ts, log_level_str(level));
        va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
        fprintf(stderr, "\n");
    }
}


/* ---- Status Display ---- */

static void qf_status(const char *state, const QfConfig *cfg,
                       int queue_count, const QfEventDef *ev)
{
    time_t now;
    struct tm *tm;
    char ts[20];
    char aka[64];

    time(&now);
    tm = localtime(&now);
    strftime(ts, sizeof(ts), "%H:%M:%S", tm);

    ftn_format_addr(&cfg->aka[0], aka, sizeof(aka));

    fprintf(stderr, "\r[%s] %s | Queue: %d | Event: %s  ",
            ts, state, queue_count,
            ev ? ev_get_tag(ev) : "(none)");
    fflush(stderr);
}


/* ---- Signal Handler (BUG-1 fix) ----
 * Ctrl-C or kill without this leaves serial ports open, .bsy locks
 * orphaned, and state files unwritten. Clean shutdown on signals.
 * Win32 doesn't have SIGHUP. SIGPIPE may not exist on all platforms. */


static volatile int g_shutdown = 0;

static void qf_signal_handler(int sig)
{
    (void)sig;
    g_shutdown = 1;
}

/* ---- Session History (Today's Activity) ---- */

static int g_sessions_ok = 0;
static int g_sessions_fail = 0;
static long g_bytes_sent = 0;
static long g_bytes_recv = 0;

static void qf_print_activity(void)
{
    qf_log(LOG_INFO, "Today's activity: %d OK, %d failed, %ld sent, %ld recv",
           g_sessions_ok, g_sessions_fail, g_bytes_sent, g_bytes_recv);
}


/* ---- Session Dispatch (fully wired) ---- */

static int qf_call_node_full(const QfConfig *cfg, const BsoItem *item,
                              const NlDatabase *nl, const QfEventDef *ev)
{
    char addrstr[64];
    char cmdline[512];
    FTN_ADDR via_addr;
    const FTN_ADDR *dial_target;
    const NlEntry *nlent;
    int route_rc, rc;
    int is_cm = 0, is_listed = 0;

    ftn_format_addr(&item->addr, addrstr, sizeof(addrstr));

    /* ---- Semaphore checks ---- */

    /* Recently polled? (cooldown) */
    if (sem_was_polled(&item->addr, cfg->retry_delay)) {
        qf_log(LOG_DEBUG, "Skipping %s — recently polled", addrstr);
        return -1;
    }

    /* Undialable? (exceeded max retries / 3 days) */
    if (sem_is_undialable(&item->addr, cfg->max_retries)) {
        qf_log(LOG_DEBUG, "Skipping %s — undialable", addrstr);
        return -1;
    }

    /* ---- Routing ---- */

    route_rc = rt_resolve(&item->addr, &via_addr);
    if (route_rc < 0) {
        qf_log(LOG_DEBUG, "Skipping %s — route says hold/nopoll", addrstr);
        return -1;
    }

    dial_target = (route_rc == 1) ? &via_addr : &item->addr;

    /* ---- Nodelist lookup ---- */

    nlent = nl ? nl_lookup(nl, dial_target) : NULL;
    if (nlent) {
        is_cm = nl_entry_is_cm(nlent);
        is_listed = 1;

        if (nl_entry_is_down(nlent) || nl_entry_is_hold(nlent)) {
            qf_log(LOG_DEBUG, "Skipping %s — nodelist: DOWN/HOLD", addrstr);
            return -1;
        }
    } else {
        qf_log(LOG_DEBUG, "Address not found in nodelist: %s", addrstr);
    }

    /* ---- Event filtering ---- */

    if (!ev_should_poll(ev, is_cm, is_listed)) {
        qf_log(LOG_DEBUG, "Skipping %s — event filter", addrstr);
        return -1;
    }

    /* ---- BSO lock ---- */

    if (bso_check_hold(cfg, &item->addr)) {
        qf_log(LOG_DEBUG, "Skipping %s — on hold", addrstr);
        return -1;
    }

    if (bso_lock(cfg, &item->addr) != 0) {
        qf_log(LOG_WARN, "Skipping %s — busy", addrstr);
        return -1;
    }

    /* ---- Dial ---- */

    {
        char dialstr[64];
        ftn_format_addr(dial_target, dialstr, sizeof(dialstr));
        qf_log(LOG_INFO, "Dialing %s%s%s (flavour=%c mail=%d files=%d)",
               addrstr,
               route_rc == 1 ? " via " : "",
               route_rc == 1 ? dialstr : "",
               (char)item->flavour,
               item->has_netmail, item->has_filelist);
    }

    /* Build command line for external mailer */
    {
        char dialstr[64];
        ftn_format_addr(dial_target, dialstr, sizeof(dialstr));

        /* If nodelist has IBN host, pass it to binkd */
        if (nlent && nl_entry_ibn_host(nlent)[0]) {
            snprintf(cmdline, sizeof(cmdline), "%s -p -P %s -h %s",
                     cfg->binkd_path, dialstr, nl_entry_ibn_host(nlent));
        } else {
            snprintf(cmdline, sizeof(cmdline), "%s -p -P %s",
                     cfg->binkd_path, dialstr);
        }
    }

    /* Execute */
#ifdef _WIN32
    rc = system(cmdline);
#else
    rc = system(cmdline);
    if (WIFEXITED(rc))
        rc = WEXITSTATUS(rc);
#endif

    /* ---- Record result ---- */

    if (rc == 0) {
        qf_log(LOG_INFO, "Successfully sent packet(s)/file(s) to %s", addrstr);
        g_sessions_ok++;
        bso_record_try(cfg, &item->addr, 1, "OK");
        sem_record_success(&item->addr);
    } else {
        qf_log(LOG_WARN, "Attempt to send packet(s)/file(s) to %s was unsuccessful", addrstr);
        g_sessions_fail++;
        bso_record_try(cfg, &item->addr, 0, "Session failed");
        sem_record_failure(&item->addr);
    }

    sem_mark_polled(&item->addr);
    bso_unlock(cfg, &item->addr);

    return rc;
}


/* ---- Post-Session Processing ---- */

static int qf_post_session_full(const QfConfig *cfg, int any_success)
{
    int rc = 0;

    if (!any_success) return 0;

    /* Run tosser */
    if (cfg->tosser_path[0]) {
        qf_log(LOG_INFO, "Scanning/tossing FidoMail");
        rc = system(cfg->tosser_path);
        if (rc != 0)
            qf_log(LOG_WARN, "Tosser exited with code %d", rc);
    }

    /* Process TIC files */
    tic_process(cfg);

    /* Check inbound for new netmail notifications */
    {
        char inb_path[520];
        int new_mail = 0, new_files = 0;
#ifndef _WIN32
        DIR *d;
        struct dirent *ent;
        snprintf(inb_path, sizeof(inb_path), "%s", cfg->inbound);
        d = opendir(inb_path);
        if (d) {
            while ((ent = readdir(d)) != NULL) {
                int len = (int)strlen(ent->d_name);
                if (len > 4 && strcasecmp(ent->d_name + len - 4, ".pkt") == 0)
                    new_mail++;
                else if (len > 4 && strcasecmp(ent->d_name + len - 4, ".tic") == 0)
                    new_files++;
            }
            closedir(d);
        }
        if (new_mail > 0)
            qf_log(LOG_INFO, "New NetMail: %d packet(s) in inbound", new_mail);
        if (new_files > 0)
            qf_log(LOG_INFO, "New file: %d TIC(s) in inbound", new_files);
#endif
    }

    return rc;
}


/* ---- QFIXUPS: Retry Incomplete Transfers ---- */

#define MAX_FIXUPS 128

typedef struct {
    FTN_ADDR addr;
    char     filename[260];
    time_t   when;
} FixupEntry;

static FixupEntry g_fixups[MAX_FIXUPS];
static int        g_fixup_count = 0;

static void fixup_add(const FTN_ADDR *addr, const char *filename)
{
    if (g_fixup_count >= MAX_FIXUPS) return;
    g_fixups[g_fixup_count].addr = *addr;
    strncpy(g_fixups[g_fixup_count].filename, filename, 259);
    g_fixups[g_fixup_count].when = time(NULL);
    g_fixup_count++;
}

static void fixup_save(const char *dir)
{
    char path[260];
    FILE *f;
    int i;

    snprintf(path, sizeof(path), "%s%cQFIXUPS.DAT", dir, PATH_SEP);
    f = fopen(path, "w");
    if (!f) return;
    for (i = 0; i < g_fixup_count; i++) {
        fprintf(f, "%u:%u/%u %s %ld\n",
                g_fixups[i].addr.zone, g_fixups[i].addr.net,
                g_fixups[i].addr.node, g_fixups[i].filename,
                (long)g_fixups[i].when);
    }
    fclose(f);
}

static void fixup_load(const char *dir)
{
    char path[260], line[512], addr_str[64], fname[260];
    long when;
    FILE *f;

    snprintf(path, sizeof(path), "%s%cQFIXUPS.DAT", dir, PATH_SEP);
    f = fopen(path, "r");
    if (!f) return;
    while (fgets(line, sizeof(line), f) && g_fixup_count < MAX_FIXUPS) {
        if (sscanf(line, "%63s %259s %ld", addr_str, fname, &when) == 3) {
            ftn_parse_addr(addr_str, &g_fixups[g_fixup_count].addr);
            strncpy(g_fixups[g_fixup_count].filename, fname, 259);
            g_fixups[g_fixup_count].when = (time_t)when;
            g_fixup_count++;
        }
    }
    fclose(f);
    if (g_fixup_count > 0)
        qf_log(LOG_INFO, "Loaded %d fixup entries", g_fixup_count);
}


/* ---- Configuration Loader ---- */

int qf_config_load(const char *path, QfConfig *cfg)
{
    FILE *f;
    char line[512], key[64], val[448];

    memset(cfg, 0, sizeof(*cfg));
    cfg->max_retries = 5;
    cfg->retry_delay = 300;
    cfg->hold_time   = 3600;

    f = fopen(path, "r");
    if (!f) {
        qf_log(LOG_FATAL, "Cannot open config: %s", path);
        return -1;
    }

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n' || *p == '\r' || *p == '\0')
            continue;
        if (*p == '[') continue;  /* Skip section headers (events) */

        { char *end = p + strlen(p) - 1;
          while (end > p && (*end == '\n' || *end == '\r' || *end == ' '))
              *end-- = '\0'; }

        if (sscanf(p, "%63[^=]=%447[^\n]", key, val) != 2) continue;

        if (strcmp(key, "Address") == 0 && cfg->num_aka < 16)
            ftn_parse_addr(val, &cfg->aka[cfg->num_aka++]);
        else if (strcmp(key, "Outbound") == 0)
            strncpy(cfg->outbound, val, 259);
        else if (strcmp(key, "Inbound") == 0)
            strncpy(cfg->inbound, val, 259);
        else if (strcmp(key, "TempInbound") == 0)
            strncpy(cfg->temp_inbound, val, 259);
        else if (strcmp(key, "NetmailDir") == 0)
            strncpy(cfg->netmail_dir, val, 259);
        else if (strcmp(key, "LogFile") == 0)
            strncpy(cfg->logfile, val, 259);
        else if (strcmp(key, "Mailer") == 0)
            strncpy(cfg->binkd_path, val, 259);
        else if (strcmp(key, "Tosser") == 0)
            strncpy(cfg->tosser_path, val, 259);
        else if (strcmp(key, "TicProc") == 0)
            strncpy(cfg->tic_proc, val, 259);
        else if (strcmp(key, "NodelistDir") == 0)
            strncpy(cfg->nodelist_dir, val, 259);
        else if (strcmp(key, "NodelistBase") == 0)
            strncpy(cfg->nodelist_base, val, 31);
        else if (strcmp(key, "MaxRetries") == 0)
            cfg->max_retries = atoi(val);
        else if (strcmp(key, "RetryDelay") == 0)
            cfg->retry_delay = atoi(val);
        else if (strcmp(key, "HoldTime") == 0)
            cfg->hold_time = atoi(val);
        else if (strcmp(key, "Debug") == 0)
            cfg->debug = atoi(val);
        else if (strcmp(key, "Semaphore") == 0) {
            /* Semaphore=<path>:<errorlevel> */
            char *colon = strrchr(val, ':');
            if (colon) {
                *colon = '\0';
                sem_add_trigger(val, atoi(colon + 1));
            }
        }
    }

    fclose(f);

    if (cfg->num_aka == 0) {
        qf_log(LOG_FATAL, "No Address defined in config");
        return -1;
    }
    if (!cfg->outbound[0]) {
        qf_log(LOG_FATAL, "No Outbound directory defined");
        return -1;
    }

    g_debug = cfg->debug;

    qf_log(LOG_INFO, "Config loaded: %d AKAs, outbound=%s, mailer=%s",
           cfg->num_aka, cfg->outbound,
           cfg->binkd_path[0] ? cfg->binkd_path : "(none)");

    return 0;
}


/* ---- Main ---- */

static void print_usage(void)
{
    fprintf(stderr,
        "QFront v" QFRONT_VERSION " — FidoNet Mailer Orchestrator\n"
        "Clean-room implementation from FTS-5005/FTS-0001/FTS-5001\n\n"
        "Usage: qfront [options]\n"
        "  -c <config>   Config file (default: qfront.cfg)\n"
        "  -p <addr>     Poll a specific node and exit\n"
        "  -s            Single pass (scan once, no loop)\n"
        "  -d            Debug mode (verbose logging)\n"
        "  -h            This help\n");
}

int main(int argc, char *argv[])
{
    QfConfig cfg;
    NlDatabase *nodelist = NULL;
    const char *cfgfile = "qfront.cfg";
    const char *poll_addr = NULL;
    int single_pass = 0;
    int i;

    /* Ignore SIGPIPE — broken pipe on socket/serial write.
     * Win32 doesn't have SIGPIPE. Some POSIX platforms define
     * it only if certain headers are included. Guard both. */
#if !defined(_WIN32) && defined(SIGPIPE)
    signal(SIGPIPE, SIG_IGN);
#endif

    /* Install shutdown handlers (BUG-1 fix) */
    signal(SIGINT,  qf_signal_handler);
    signal(SIGTERM, qf_signal_handler);
#ifdef SIGHUP
    signal(SIGHUP,  qf_signal_handler);
#endif

    /* Parse command line */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
            cfgfile = argv[++i];
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            poll_addr = argv[++i];
        else if (strcmp(argv[i], "-s") == 0)
            single_pass = 1;
        else if (strcmp(argv[i], "-d") == 0)
            g_debug = 1;
        else if (strcmp(argv[i], "-h") == 0) {
            print_usage();
            return 0;
        }
    }

    /* ---- Phase 1: Load everything ---- */

    if (qf_config_load(cfgfile, &cfg) != 0)
        return 1;

    qf_log_init(cfg.logfile);
    qf_log(LOG_INFO, "QFront v" QFRONT_VERSION " starting — %d AKA(s)",
           cfg.num_aka);

    {
        char buf[64];
        for (i = 0; i < cfg.num_aka; i++) {
            ftn_format_addr(&cfg.aka[i], buf, sizeof(buf));
            qf_log(LOG_INFO, "  AKA %d: %s", i, buf);
        }
    }

    /* Load routing rules from config */
    rt_load(cfgfile);

    /* Load event schedule from config */
    ev_load(cfgfile);

    /* Load nodelist */
    if (cfg.nodelist_dir[0] && cfg.nodelist_base[0]) {
        char nlpath[520];
        snprintf(nlpath, sizeof(nlpath), "%s%c%s.*",
                 cfg.nodelist_dir, PATH_SEP, cfg.nodelist_base);

        /* Try common extensions: .999 (day number), no extension */
        snprintf(nlpath, sizeof(nlpath), "%s%c%s",
                 cfg.nodelist_dir, PATH_SEP, cfg.nodelist_base);
        nodelist = nl_open(nlpath);
        /* If that fails, it logs a warning — non-fatal */
    }

    /* Load semaphore state from previous run */
    sem_load_state(cfg.outbound);

    /* Load fixup queue */
    fixup_load(cfg.outbound);

    /* ---- Manual poll mode ---- */

    if (poll_addr) {
        FTN_ADDR addr;
        BsoItem item;

        if (ftn_parse_addr(poll_addr, &addr) != 0) {
            qf_log(LOG_FATAL, "Invalid address: %s", poll_addr);
            return 1;
        }

        memset(&item, 0, sizeof(item));
        item.addr = addr;
        item.flavour = BSO_IMMEDIATE;
        item.has_netmail = 1;

        bso_create_poll(&cfg, &addr, BSO_IMMEDIATE);

        i = qf_call_node_full(&cfg, &item, nodelist, NULL);
        qf_post_session_full(&cfg, i == 0);

        sem_save_state(cfg.outbound);
        nl_close(nodelist);
        qf_log_close();
        return (i == 0) ? 0 : 1;
    }

    /* ---- Main orchestrator loop ---- */

    qf_log(LOG_INFO, "Waiting for a call");

    do {
        BsoItem items[256];
        int count, j;
        int any_success = 0;
        const QfEventDef *ev = NULL;
        int sem_exit;

        /* ---- Check semaphore exit triggers ---- */
        sem_exit = sem_check_triggers();
        if (sem_exit >= 0) {
            qf_log(LOG_INFO, "Exiting with errorlevel %d", sem_exit);
            sem_save_state(cfg.outbound);
            fixup_save(cfg.outbound);
            nl_close(nodelist);
            qf_log_close();
            return sem_exit;
        }

        /* ---- Check event schedule ---- */
        ev = ev_check_active();
        if (ev) {
            qf_log(LOG_DEBUG, "Running event \"%s\"", ev_get_tag(ev));
            ev_pre_actions(ev, &cfg);
        }

        /* ---- Scan BSO outbound ---- */
        qf_log(LOG_DEBUG, "Building queue");
        count = bso_scan(&cfg, items, 256);

        if (count == 0) {
            qf_status("Waiting for a call", &cfg, 0, ev);
        } else {
            qf_log(LOG_INFO, "Scanning for mail — %d nodes pending", count);
        }

        /* ---- Process each node ---- */
        for (j = 0; j < count; j++) {
            char addrstr[64];
            ftn_format_addr(&items[j].addr, addrstr, sizeof(addrstr));

            /* Skip hold flavour unless event forces */
            if (items[j].flavour == BSO_HOLD && ev &&
                !(ev_get_flags(ev) & 0x200 /*EVF_FORCE_POLL*/)) {
                qf_log(LOG_DEBUG, "Skipping %s (hold flavour)", addrstr);
                continue;
            }

            qf_status(addrstr, &cfg, count - j, ev);

            /* Call the node (routing + nodelist + event checks inside) */
            if (qf_call_node_full(&cfg, &items[j], nodelist, ev) == 0) {
                any_success = 1;
            }
        }

        /* ---- Post-session processing ---- */
        if (any_success)
            qf_post_session_full(&cfg, 1);

        /* ---- Event post-actions ---- */
        if (ev) {
            int end_event = ev_post_actions(ev, &cfg, count == 0);
            if (end_event) {
                qf_log(LOG_INFO, "Ending event \"%s\"", ev_get_tag(ev));
            }
        }

        /* ---- Save state ---- */
        sem_save_state(cfg.outbound);
        fixup_save(cfg.outbound);

        /* ---- Sleep ---- */
        if (!single_pass) {
            qf_status("Waiting for a call", &cfg, 0, ev);
#ifdef _WIN32
            Sleep(cfg.retry_delay * 1000);
#else
            sleep(cfg.retry_delay);
#endif
        }

        if (g_shutdown) {
            qf_log(LOG_INFO, "Caught signal — clean shutdown");
            break;
        }

    } while (!single_pass);

    qf_print_activity();
    qf_log(LOG_INFO, "Normal exit");
    fprintf(stderr, "\n");

    sem_save_state(cfg.outbound);
    fixup_save(cfg.outbound);
    nl_close(nodelist);
    qf_log_close();

    return 0;
}
