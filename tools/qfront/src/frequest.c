/* ====================================================================
 * frequest.c — File Request Processor
 * ====================================================================
 * Processes incoming .REQ (file request) files from remote systems.
 * Handles magic filenames (QMAGIC.DAT) and request limits (QRLIMIT.DAT).
 *
 * From binary:
 *   "Processing request file"
 *   "Requested file(s) <list>"
 *   "Found magic file <name>"
 *   "File request" / "File update request"
 *   "Maximum bytes/number of requests reached"
 *   "Requests not allowed during this event"
 *   "Connect speed too low for file requests"
 *
 * Clean-room from FTS-0006 Section 6 + QFront binary analysis.
 * ==================================================================== */

#include "qfront.h"

#define MAX_MAGIC      128
#define MAX_REQ_FILES  64

/* ---- Magic Filename Entry ----
 * Maps a short alias to a real file path.
 * From binary: "Found magic file <name>"
 * Stored in QMAGIC.DAT. */

typedef struct {
    char alias[32];               /* Magic name (e.g. "FILES")   */
    char path[260];               /* Real file path               */
    int  security;                /* Required security level      */
} MagicEntry;

/* ---- Request Limits ----
 * From binary: "Maximum bytes/number of requests reached"
 * Stored in QRLIMIT.DAT (or in config). */

typedef struct {
    long max_bytes_session;       /* Max bytes per session        */
    int  max_files_session;       /* Max files per session        */
    long max_bytes_day;           /* Max bytes per day            */
    int  max_files_day;           /* Max files per day            */
    int  min_speed;               /* Min connect speed required   */
    int  allow_unlisted;          /* Allow from unlisted nodes    */
} ReqLimits;

/* ---- Request State ---- */

typedef struct {
    long bytes_sent;              /* Bytes sent this session      */
    int  files_sent;              /* Files sent this session      */
    long bytes_today;             /* Bytes sent today             */
    int  files_today;             /* Files sent today             */
} ReqState;


/* ---- Load Magic Filenames ---- */

static MagicEntry g_magic[MAX_MAGIC];
static int        g_magic_count = 0;

int freq_load_magic(const char *path)
{
    FILE *f;
    char line[512];

    g_magic_count = 0;

    f = fopen(path, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f) && g_magic_count < MAX_MAGIC) {
        char alias[32], fpath[260];
        int sec = 0;

        /* Format: ALIAS PATH [security_level] */
        if (sscanf(line, "%31s %259s %d", alias, fpath, &sec) >= 2) {
            strncpy(g_magic[g_magic_count].alias, alias, 31);
            strncpy(g_magic[g_magic_count].path, fpath, 259);
            g_magic[g_magic_count].security = sec;
            g_magic_count++;
        }
    }

    fclose(f);
    qf_log(LOG_DEBUG, "Loaded %d magic filenames", g_magic_count);
    return 0;
}

/* ---- Lookup Magic Filename ---- */

static const char *freq_find_magic(const char *name)
{
    int i;
    for (i = 0; i < g_magic_count; i++) {
        if (strcasecmp(g_magic[i].alias, name) == 0) {
            qf_log(LOG_DEBUG, "Found magic file \"%s\" -> %s",
                   name, g_magic[i].path);
            return g_magic[i].path;
        }
    }
    return NULL;
}


/* ---- Load Request Limits ---- */

static ReqLimits g_limits = {
    .max_bytes_session = 999999999L,
    .max_files_session = 999999999,
    .max_bytes_day     = 999999999L,
    .max_files_day     = 999999999,
    .min_speed         = 0,
    .allow_unlisted    = 0
};

int freq_load_limits(const char *path)
{
    FILE *f;
    char line[256], key[32], val[32];

    f = fopen(path, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%31[^=]=%31s", key, val) == 2) {
            if (strcmp(key, "MaxBytesSession") == 0)
                g_limits.max_bytes_session = atol(val);
            else if (strcmp(key, "MaxFilesSession") == 0)
                g_limits.max_files_session = atoi(val);
            else if (strcmp(key, "MaxBytesDay") == 0)
                g_limits.max_bytes_day = atol(val);
            else if (strcmp(key, "MaxFilesDay") == 0)
                g_limits.max_files_day = atoi(val);
            else if (strcmp(key, "MinSpeed") == 0)
                g_limits.min_speed = atoi(val);
            else if (strcmp(key, "AllowUnlisted") == 0)
                g_limits.allow_unlisted = atoi(val);
        }
    }

    fclose(f);
    return 0;
}


/* ---- Check Request Limits ---- */

static int freq_check_limits(const ReqState *state, long file_size,
                              int connect_speed, int is_listed)
{
    if (!is_listed && !g_limits.allow_unlisted) {
        qf_log(LOG_WARN, "Requests not allowed from unlisted system");
        return -1;
    }

    if (connect_speed > 0 && connect_speed < g_limits.min_speed) {
        qf_log(LOG_WARN, "Connect speed too low for file requests");
        qf_log(LOG_WARN, "Minimum connect speed required for requests is %d",
               g_limits.min_speed);
        return -1;
    }

    if (state->bytes_sent + file_size > g_limits.max_bytes_session) {
        qf_log(LOG_WARN, "Maximum bytes in requests reached");
        return -1;
    }

    if (state->files_sent >= g_limits.max_files_session) {
        qf_log(LOG_WARN, "Maximum number of requests reached");
        return -1;
    }

    if (state->bytes_today + file_size > g_limits.max_bytes_day) {
        qf_log(LOG_WARN, "Maximum bytes in requests reached (daily limit)");
        return -1;
    }

    return 0;
}


/* ---- Build File List from .REQ ----
 *
 * .REQ file format (FTS-0006 Section 6):
 *   One filename per line. Optional:
 *     filename            Request this file
 *     filename !password  Password-protected request
 *     filename +datetime  Update request (only if newer)
 *
 * Returns number of files queued for sending. */

typedef struct {
    char path[260];               /* Resolved path to send       */
    int  is_update;               /* Update request (only newer) */
} FreqFile;

int freq_process_req(const char *req_path, const char *req_dirs,
                      FreqFile *files, int max_files)
{
    FILE *f;
    char line[512];
    int count = 0;

    f = fopen(req_path, "r");
    if (!f) return 0;

    qf_log(LOG_INFO, "Processing request file: %s", req_path);

    while (fgets(line, sizeof(line), f) && count < max_files) {
        char *p = line;
        char name[260];
        int is_update = 0;
        const char *magic;

        /* Strip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        {
            char *end = p + strlen(p) - 1;
            while (end > p && (*end == '\n' || *end == '\r' || *end == ' '))
                *end-- = '\0';
        }

        if (*p == '\0' || *p == ';') continue;

        /* Extract filename (first token) */
        sscanf(p, "%259s", name);

        /* Check for update flag (+) */
        if (strchr(p, '+'))
            is_update = 1;

        /* Check magic filenames first */
        magic = freq_find_magic(name);
        if (magic) {
            strncpy(files[count].path, magic, 259);
            files[count].is_update = is_update;
            count++;
            qf_log(LOG_INFO, "Requested file(s): %s (magic -> %s)",
                   name, magic);
            continue;
        }

        /* Search request directories for the file */
        {
            char search[520];
            char *dir = NULL;
            char dirs_copy[1024];
            FILE *test;

            strncpy(dirs_copy, req_dirs, sizeof(dirs_copy) - 1);
            dir = strtok(dirs_copy, ";");

            while (dir) {
                snprintf(search, sizeof(search), "%s%c%s",
                         dir, PATH_SEP, name);
                test = fopen(search, "rb");
                if (test) {
                    fclose(test);
                    strncpy(files[count].path, search, 259);
                    files[count].is_update = is_update;
                    count++;
                    qf_log(LOG_INFO, "Requested file(s): %s", search);
                    break;
                }
                dir = strtok(NULL, ";");
            }

            if (!dir)
                qf_log(LOG_WARN, "File not found: %s", name);
        }
    }

    fclose(f);
    return count;
}


/* ---- Build Outgoing .REQ File ----
 * Creates a .REQ file in the BSO outbound for a file request. */

int freq_build_req(const char *outbound, const FTN_ADDR *addr,
                    const char **filenames, int num_files)
{
    char path[260];
    FILE *f;
    int i;

    /* Build BSO path: NNNNNNNN.req */
    snprintf(path, sizeof(path), "%s%c%04x%04x.req",
             outbound, PATH_SEP, addr->net, addr->node);

    f = fopen(path, "w");
    if (!f) {
        qf_log(LOG_ERROR, "Cannot create request file: %s", path);
        return -1;
    }

    for (i = 0; i < num_files; i++)
        fprintf(f, "%s\n", filenames[i]);

    fclose(f);

    {
        char buf[64];
        ftn_format_addr(addr, buf, sizeof(buf));
        qf_log(LOG_INFO, "File request for %s: %d file(s)", buf, num_files);
    }

    return 0;
}
