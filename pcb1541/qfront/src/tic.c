/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* tic.c -- TIC File Processor                                              */
/*                                                                           */
/* Scans the inbound directory for .TIC files, parses them, and either      */
/* processes them directly or calls an external TIC processor (htick,        */
/* pcbtic, etc.).                                                            */
/*                                                                           */
/* TIC format is a simple keyword-value text file:                           */
/*   Area <areaname>        File area name                                   */
/*   Origin <zone:net/node> Original sender                                  */
/*   From <zone:net/node>   Immediate sender                                */
/*   To <zone:net/node>     Destination                                      */
/*   File <filename>        Attached filename                                */
/*   Desc <description>     File description                                 */
/*   CRC <hex_crc32>        CRC-32 of attached file                          */
/*   Path <addr> <time>     Routing path entry                               */
/*   Seenby <addr>          Seen-by list entry                               */
/*   Pw <password>          Area password                                    */
/*                                                                           */
/* Clean-room from published TIC specification.                              */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

typedef struct {
    char     Area[64];              /* file area name                    */
    char     FileName[260];         /* attached file name                */
    char     Description[256];      /* file description                  */
    FTN_ADDR Origin;                /* original sender                   */
    FTN_ADDR From;                  /* immediate sender                  */
    FTN_ADDR To;                    /* destination                       */
    char     Password[32];          /* area password                     */
    uint32_t Crc;                   /* CRC-32 of file                    */
    int      HasCrc;                /* CRC field present?                */
} TicFile;


/*-----------------------------------------------------------------------*/
/* tic_parse() -- Parse a .TIC file into a TicFile struct                */
/*                                                                       */
/* TIC files are text-based metadata files that accompany file area       */
/* distributions in FidoNet. Each .TIC file describes one attached        */
/* file: its name, the area it belongs to, where it came from, an        */
/* optional CRC-32, and a description.                                    */
/*                                                                       */
/* Security: We reject filenames containing path separators (/ \ :) or   */
/* ".." to prevent directory traversal attacks. A malicious .TIC file     */
/* could otherwise overwrite system files by specifying paths like        */
/* "../../etc/passwd" in the File field.                                  */
/*                                                                       */
/* Returns 0 on success (valid TIC with both filename and area set),     */
/* -1 on any parse error or security violation.                          */
/*-----------------------------------------------------------------------*/

static int tic_parse(const char *Path, TicFile *Tic)
{
    FILE *f;                            /* TIC file handle               */
    char  Line[512];                    /* line read buffer              */

    memset(Tic, 0, sizeof(*Tic));
    qf_log(LOG_DEBUG, "tic_parse: reading %s", Path);

    f = fopen(Path, "r");
    if (!f) return -1;

    while (fgets(Line, sizeof(Line), f)) {
        char *p = Line;                 /* keyword pointer               */
        char *Val;                      /* value pointer                 */

        /* Strip trailing whitespace */
        {
            char *End = p + strlen(p) - 1;
            while (End > p && (*End == '\n' || *End == '\r' || *End == ' '))
                *End-- = '\0';
        }

        /* Find keyword-value split (first space) */
        Val = strchr(p, ' ');
        if (!Val) continue;
        *Val++ = '\0';
        while (*Val == ' ') Val++;

        if (strcasecmp(p, "Area") == 0)
            strncpy(Tic->Area, Val, sizeof(Tic->Area) - 1);
        else if (strcasecmp(p, "File") == 0)
            strncpy(Tic->FileName, Val, sizeof(Tic->FileName) - 1);
        else if (strcasecmp(p, "Desc") == 0)
            strncpy(Tic->Description, Val, sizeof(Tic->Description) - 1);
        else if (strcasecmp(p, "Origin") == 0)
            ftn_parse_addr(Val, &Tic->Origin);
        else if (strcasecmp(p, "From") == 0)
            ftn_parse_addr(Val, &Tic->From);
        else if (strcasecmp(p, "To") == 0)
            ftn_parse_addr(Val, &Tic->To);
        else if (strcasecmp(p, "Pw") == 0)
            strncpy(Tic->Password, Val, sizeof(Tic->Password) - 1);
        else if (strcasecmp(p, "CRC") == 0) {
            Tic->Crc = (uint32_t)strtoul(Val, NULL, 16);
            Tic->HasCrc = 1;
        }
    }

    fclose(f);

    /* Sanitize filename -- reject path separators and ".." */
    {
        const char *Scan;               /* filename scan pointer         */

        for (Scan = Tic->FileName; *Scan; Scan++) {
            if (*Scan == '/' || *Scan == '\\' || *Scan == ':') {
                qf_log(LOG_WARN, "TIC: rejecting filename with path chars: %s",
                       Tic->FileName);
                return -1;
            }
        }
        if (strstr(Tic->FileName, "..")) {
            qf_log(LOG_WARN, "TIC: rejecting filename with '..': %s",
                   Tic->FileName);
            return -1;
        }
    }

    return (Tic->FileName[0] && Tic->Area[0]) ? 0 : -1;
}


/*-----------------------------------------------------------------------*/
/* tic_scan_inbound() -- Scan inbound directory for .TIC files           */
/*                                                                       */
/* Walks the inbound directory looking for files ending in .TIC (case-    */
/* insensitive). Each one is parsed and logged. Count of valid TIC        */
/* files is returned.                                                     */
/*                                                                       */
/* Returns number of valid TIC files found.                              */
/*-----------------------------------------------------------------------*/

int tic_scan_inbound(const QfConfig *Cfg)
{
    int Count = 0;                      /* valid TIC files found         */

#ifndef _WIN32
    DIR           *d;                   /* directory handle               */
    struct dirent *Ent;                 /* directory entry                */

    d = opendir(Cfg->inbound);
    if (!d) return 0;

    while ((Ent = readdir(d)) != NULL) {
        const char *Name;               /* entry filename                */
        int         Len;                /* filename length               */
        TicFile     Tic;                /* parsed TIC data               */
        char        FullPath[520];      /* full path to .TIC file        */
        char        AddrBuf[64];        /* formatted address for log     */

        Name = Ent->d_name;
        Len  = (int)strlen(Name);

        /* Look for .TIC files (case-insensitive) */
        if (Len < 5) continue;
        if (strcasecmp(Name + Len - 4, ".tic") != 0) continue;

        snprintf(FullPath, sizeof(FullPath), "%s%c%s",
                 Cfg->inbound, PATH_SEP, Name);

        if (tic_parse(FullPath, &Tic) == 0) {
            ftn_format_addr(&Tic.From, AddrBuf, sizeof(AddrBuf));
            qf_log(LOG_INFO, "TIC: area=%s file=%s from=%s",
                   Tic.Area, Tic.FileName, AddrBuf);
            Count++;
        }
    }

    closedir(d);
#endif

    return Count;
}


/*-----------------------------------------------------------------------*/
/* tic_process() -- Process TIC files from inbound                       */
/*                                                                       */
/* Scans for TIC files, then either calls the configured external TIC     */
/* processor or logs a warning that no processor is configured.           */
/*                                                                       */
/* Returns 0 on success, or the external processor's exit code.          */
/*-----------------------------------------------------------------------*/

int tic_process(const QfConfig *Cfg)
{
    int Count;                          /* TIC files found               */

    Count = tic_scan_inbound(Cfg);
    if (Count == 0) return 0;

    qf_log(LOG_INFO, "TIC: %d file(s) to process", Count);

    if (Cfg->tic_proc[0]) {
        int Rc;                         /* external processor exit code  */

        qf_log(LOG_INFO, "Running TIC processor: %s", Cfg->tic_proc);
        Rc = system(Cfg->tic_proc);
        if (Rc != 0)
            qf_log(LOG_WARN, "TIC processor exited with code %d", Rc);
        return Rc;
    }

    qf_log(LOG_WARN, "No TIC processor configured -- files not processed");
    return 0;
}
