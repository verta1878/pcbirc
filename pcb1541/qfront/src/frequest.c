/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* frequest.c -- File Request Processor                                     */
/*                                                                           */
/* Processes incoming .REQ (file request) files from remote systems.         */
/* Handles magic filenames (QMAGIC.DAT) and request limits (QRLIMIT.DAT).   */
/*                                                                           */
/* From binary:                                                              */
/*   "Processing request file"                                              */
/*   "Requested file(s) <list>"                                             */
/*   "Found magic file <name>"                                              */
/*   "Maximum bytes/number of requests reached"                             */
/*   "Requests not allowed during this event"                               */
/*   "Connect speed too low for file requests"                              */
/*                                                                           */
/* Clean-room from FTS-0006 Section 6 + QFront binary analysis.              */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

#define MAX_MAGIC      128              /* max magic filename entries     */
#define MAX_REQ_FILES  64               /* max files per request         */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                        Magic Filename Table                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Maps a short alias to a real file path.
 * From binary: "Found magic file <name>"
 * Stored in QMAGIC.DAT. */

typedef struct {
    char Alias[32];                     /* magic name (e.g. "FILES")     */
    char Path[260];                     /* real file path                */
    int  Security;                      /* required security level       */
} MagicEntry;

static MagicEntry g_Magic[MAX_MAGIC];   /* magic filename table          */
static int        g_MagicCount = 0;     /* entries in magic table        */


/*-----------------------------------------------------------------------*/
/* freq_load_magic() -- Load magic filenames from QMAGIC.DAT            */
/*                                                                       */
/* Format: ALIAS PATH [security_level]                                   */
/* Example: FILES C:\PCB\GEN\DLPATH.LST 10                               */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int freq_load_magic(const char *Path)
{
    FILE *f;                            /* magic file handle             */
    char  Line[512];                    /* line read buffer              */

    g_MagicCount = 0;

    f = fopen(Path, "r");
    if (!f) return -1;

    while (fgets(Line, sizeof(Line), f) && g_MagicCount < MAX_MAGIC) {
        char Alias[32];                 /* parsed alias name             */
        char FPath[260];                /* parsed file path              */
        int  Sec = 0;                   /* parsed security level         */

        /* Format: ALIAS PATH [security_level] */
        if (sscanf(Line, "%31s %259s %d", Alias, FPath, &Sec) >= 2) {
            strncpy(g_Magic[g_MagicCount].Alias, Alias, 31);
            strncpy(g_Magic[g_MagicCount].Path, FPath, 259);
            g_Magic[g_MagicCount].Security = Sec;
            g_MagicCount++;
        }
    }

    fclose(f);
    qf_log(LOG_DEBUG, "Loaded %d magic filenames", g_MagicCount);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* freq_find_magic() -- Lookup a magic filename alias                    */
/*                                                                       */
/* Case-insensitive search. Returns the real path, or NULL if not found. */
/*-----------------------------------------------------------------------*/

static const char *freq_find_magic(const char *Name)
{
    int i;                              /* search index                  */

    for (i = 0; i < g_MagicCount; i++) {
        if (strcasecmp(g_Magic[i].Alias, Name) == 0) {
            qf_log(LOG_DEBUG, "Found magic file \"%s\" -> %s",
                   Name, g_Magic[i].Path);
            return g_Magic[i].Path;
        }
    }
    return NULL;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                          Request Limits                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* From binary: "Maximum bytes/number of requests reached"
 * Stored in QRLIMIT.DAT (or in config). */

typedef struct {
    long MaxBytesSession;               /* max bytes per session         */
    int  MaxFilesSession;               /* max files per session         */
    long MaxBytesDay;                   /* max bytes per day             */
    int  MaxFilesDay;                   /* max files per day             */
    int  MinSpeed;                      /* min connect speed required    */
    int  AllowUnlisted;                 /* allow from unlisted nodes     */
} ReqLimits;

typedef struct {
    long BytesSent;                     /* bytes sent this session       */
    int  FilesSent;                     /* files sent this session       */
    long BytesToday;                    /* bytes sent today              */
    int  FilesToday;                    /* files sent today              */
} ReqState;

static ReqLimits g_Limits = {
    999999999L,                         /* MaxBytesSession               */
    999999999,                          /* MaxFilesSession               */
    999999999L,                         /* MaxBytesDay                   */
    999999999,                          /* MaxFilesDay                   */
    0,                                  /* MinSpeed                      */
    0                                   /* AllowUnlisted                 */
};


/*-----------------------------------------------------------------------*/
/* freq_load_limits() -- Load request limits from QRLIMIT.DAT           */
/*                                                                       */
/* Format: Key=Value, one per line.                                      */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int freq_load_limits(const char *Path)
{
    FILE *f;                            /* limits file handle            */
    char  Line[256];                    /* line read buffer              */
    char  Key[32];                      /* parsed key                    */
    char  Val[32];                      /* parsed value                  */

    f = fopen(Path, "r");
    if (!f) return -1;

    while (fgets(Line, sizeof(Line), f)) {
        if (sscanf(Line, "%31[^=]=%31s", Key, Val) == 2) {
            if (strcmp(Key, "MaxBytesSession") == 0)
                g_Limits.MaxBytesSession = atol(Val);
            else if (strcmp(Key, "MaxFilesSession") == 0)
                g_Limits.MaxFilesSession = atoi(Val);
            else if (strcmp(Key, "MaxBytesDay") == 0)
                g_Limits.MaxBytesDay = atol(Val);
            else if (strcmp(Key, "MaxFilesDay") == 0)
                g_Limits.MaxFilesDay = atoi(Val);
            else if (strcmp(Key, "MinSpeed") == 0)
                g_Limits.MinSpeed = atoi(Val);
            else if (strcmp(Key, "AllowUnlisted") == 0)
                g_Limits.AllowUnlisted = atoi(Val);
        }
    }

    fclose(f);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* freq_check_limits() -- Check if a file request is within limits       */
/*                                                                       */
/* Returns 0 if allowed, -1 if limit exceeded.                           */
/*-----------------------------------------------------------------------*/

static int freq_check_limits(const ReqState *State, long FileSize,
                              int ConnectSpeed, int IsListed)
{
    if (!IsListed && !g_Limits.AllowUnlisted) {
        qf_log(LOG_WARN, "Requests not allowed from unlisted system");
        return -1;
    }

    if (ConnectSpeed > 0 && ConnectSpeed < g_Limits.MinSpeed) {
        qf_log(LOG_WARN, "Connect speed too low for file requests");
        qf_log(LOG_WARN, "Minimum connect speed required for requests is %d",
               g_Limits.MinSpeed);
        return -1;
    }

    if (State->BytesSent + FileSize > g_Limits.MaxBytesSession) {
        qf_log(LOG_WARN, "Maximum bytes in requests reached");
        return -1;
    }

    if (State->FilesSent >= g_Limits.MaxFilesSession) {
        qf_log(LOG_WARN, "Maximum number of requests reached");
        return -1;
    }

    if (State->BytesToday + FileSize > g_Limits.MaxBytesDay) {
        qf_log(LOG_WARN, "Maximum bytes in requests reached (daily limit)");
        return -1;
    }

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                       Request File Processing                             */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    char Path[260];                     /* resolved path to send         */
    int  IsUpdate;                      /* update request (only newer)   */
} FreqFile;


/*-----------------------------------------------------------------------*/
/* freq_process_req() -- Parse a .REQ file and resolve filenames         */
/*                                                                       */
/* .REQ file format (FTS-0006 Section 6):                                */
/*   One filename per line, with optional modifiers:                     */
/*     FILENAME            -- request this file                          */
/*     FILENAME !password  -- password-protected request                 */
/*     FILENAME +datetime  -- update request (only send if newer)        */
/*                                                                       */
/* Resolution order:                                                     */
/*   1. Check magic filenames (QMAGIC.DAT aliases like "FILES")          */
/*   2. Search request directories (semicolon-separated in config)       */
/*   3. Log "File not found" if neither matches                          */
/*                                                                       */
/* Returns number of files resolved for sending.                         */
/*-----------------------------------------------------------------------*/

int freq_process_req(const char *ReqPath, const char *ReqDirs,
                      FreqFile *Files, int MaxFiles)
{
    FILE *f;                            /* .REQ file handle              */
    char  Line[512];                    /* line read buffer              */
    int   Count = 0;                    /* files resolved                */

    f = fopen(ReqPath, "r");
    if (!f) return 0;

    qf_log(LOG_INFO, "Processing request file: %s", ReqPath);
    qf_log(LOG_DEBUG, "  search dirs: %s", ReqDirs);

    while (fgets(Line, sizeof(Line), f) && Count < MaxFiles) {
        char       *p = Line;           /* line scan pointer             */
        char        Name[260];          /* requested filename            */
        int         IsUpdate = 0;       /* update request flag           */
        const char *MagicPath;          /* resolved magic path           */

        /* Strip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        {
            char *End = p + strlen(p) - 1;
            while (End > p && (*End == '\n' || *End == '\r' || *End == ' '))
                *End-- = '\0';
        }

        if (*p == '\0' || *p == ';') continue;

        /* Extract filename (first token) */
        sscanf(p, "%259s", Name);

        /* Check for update flag (+) */
        if (strchr(p, '+'))
            IsUpdate = 1;

        /* Check magic filenames first */
        MagicPath = freq_find_magic(Name);
        if (MagicPath) {
            strncpy(Files[Count].Path, MagicPath, 259);
            Files[Count].IsUpdate = IsUpdate;
            Count++;
            qf_log(LOG_INFO, "Requested file(s): %s (magic -> %s)",
                   Name, MagicPath);
            continue;
        }

        /* Search request directories for the file */
        {
            char  SearchPath[520];      /* candidate file path           */
            char *Dir = NULL;           /* current search directory      */
            char  DirsCopy[1024];       /* mutable copy of search dirs   */
            FILE *Test;                 /* test open handle              */

            strncpy(DirsCopy, ReqDirs, sizeof(DirsCopy) - 1);
            Dir = strtok(DirsCopy, ";");

            while (Dir) {
                snprintf(SearchPath, sizeof(SearchPath), "%s%c%s",
                         Dir, PATH_SEP, Name);
                Test = fopen(SearchPath, "rb");
                if (Test) {
                    fclose(Test);
                    strncpy(Files[Count].Path, SearchPath, 259);
                    Files[Count].IsUpdate = IsUpdate;
                    Count++;
                    qf_log(LOG_INFO, "Requested file(s): %s", SearchPath);
                    break;
                }
                Dir = strtok(NULL, ";");
            }

            if (!Dir)
                qf_log(LOG_WARN, "File not found: %s", Name);
        }
    }

    fclose(f);
    return Count;
}


/*-----------------------------------------------------------------------*/
/* freq_build_req() -- Build outgoing .REQ file in BSO outbound         */
/*                                                                       */
/* Creates a .REQ file for a file request to a remote system.            */
/* FTS-5005: NNNNNNNN.req in the outbound directory.                     */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int freq_build_req(const char *Outbound, const FTN_ADDR *Addr,
                    const char **Filenames, int NumFiles)
{
    char  Path[260];                    /* .REQ file path                */
    FILE *f;                            /* output file handle            */
    int   i;                            /* file loop index               */

    /* Build BSO path: NNNNNNNN.req */
    snprintf(Path, sizeof(Path), "%s%c%04x%04x.req",
             Outbound, PATH_SEP, Addr->net, Addr->node);

    f = fopen(Path, "w");
    if (!f) {
        qf_log(LOG_ERROR, "Cannot create request file: %s", Path);
        return -1;
    }

    for (i = 0; i < NumFiles; i++)
        fprintf(f, "%s\n", Filenames[i]);

    fclose(f);

    {
        char AddrBuf[64];               /* formatted address for log     */
        ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
        qf_log(LOG_INFO, "File request for %s: %d file(s)", AddrBuf, NumFiles);
    }

    return 0;
}
