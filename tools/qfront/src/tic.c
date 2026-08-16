/* ====================================================================
 * tic.c — TIC File Processor
 * ====================================================================
 * Scans the inbound directory for .TIC files, parses them, and
 * either processes them directly or calls an external TIC processor
 * (htick, pcbtic, etc.).
 *
 * TIC format is a simple keyword-value text file:
 *   Area <areaname>
 *   Origin <zone:net/node>
 *   From <zone:net/node>
 *   To <zone:net/node>
 *   File <filename>
 *   Desc <description>
 *   CRC <hex_crc32>
 *   Path <zone:net/node> <unix_timestamp>
 *   Seenby <zone:net/node>
 *   Pw <password>
 *
 * Clean-room from published TIC specification.
 * ==================================================================== */

#include "qfront.h"

typedef struct {
    char     area[64];            /* File area name               */
    char     filename[260];       /* Attached file name           */
    char     description[256];    /* File description             */
    FTN_ADDR origin;              /* Original sender              */
    FTN_ADDR from;                /* Immediate sender             */
    FTN_ADDR to;                  /* Destination                  */
    char     password[32];        /* Area password                */
    uint32_t crc;                 /* CRC-32 of file               */
    int      has_crc;             /* CRC field present?           */
} TicFile;


/* ---- Parse a TIC File ---- */

/*-----------------------------------------------------------------------*/
/* tic_parse() — Parse a .TIC file into a TicFile struct                 */
/*                                                                         */
/* TIC files are text-based metadata files that accompany file area      */
/* distributions in FidoNet. Each .TIC file describes one attached       */
/* file: its name, the area it belongs to, where it came from, an       */
/* optional CRC-32, and a description.                                   */
/*                                                                         */
/* Security: We reject filenames containing path separators (/ \ :) or  */
/* ".." to prevent directory traversal attacks. A malicious .TIC file    */
/* could otherwise overwrite system files by specifying paths like       */
/* "../../etc/passwd" in the File field.                                  */
/*                                                                         */
/* Returns 0 on success (valid TIC with both filename and area set),     */
/* -1 on any parse error or security violation.                          */
/*-----------------------------------------------------------------------*/

static int tic_parse(const char *path, TicFile *tic)
{
    FILE *f;
    char line[512];

    memset(tic, 0, sizeof(*tic));
    qf_log(LOG_DEBUG, "tic_parse: reading %s", path);

    f = fopen(path, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        char *val;

        /* Strip trailing whitespace */
        {
            char *end = p + strlen(p) - 1;
            while (end > p && (*end == '\n' || *end == '\r' || *end == ' '))
                *end-- = '\0';
        }

        /* Find keyword-value split (first space) */
        val = strchr(p, ' ');
        if (!val) continue;
        *val++ = '\0';
        while (*val == ' ') val++;

        if (strcasecmp(p, "Area") == 0)
            strncpy(tic->area, val, sizeof(tic->area) - 1);
        else if (strcasecmp(p, "File") == 0)
            strncpy(tic->filename, val, sizeof(tic->filename) - 1);
        else if (strcasecmp(p, "Desc") == 0)
            strncpy(tic->description, val, sizeof(tic->description) - 1);
        else if (strcasecmp(p, "Origin") == 0)
            ftn_parse_addr(val, &tic->origin);
        else if (strcasecmp(p, "From") == 0)
            ftn_parse_addr(val, &tic->from);
        else if (strcasecmp(p, "To") == 0)
            ftn_parse_addr(val, &tic->to);
        else if (strcasecmp(p, "Pw") == 0)
            strncpy(tic->password, val, sizeof(tic->password) - 1);
        else if (strcasecmp(p, "CRC") == 0) {
            tic->crc = (uint32_t)strtoul(val, NULL, 16);
            tic->has_crc = 1;
        }
    }

    fclose(f);
    /* Sanitize filename — reject path separators and ".." */
    {
        const char *p;
        for (p = tic->filename; *p; p++) {
            if (*p == '/' || *p == '\\' || *p == ':') {
                qf_log(LOG_WARN, "TIC: rejecting filename with path chars: %s", tic->filename);
                return -1;
            }
        }
        if (strstr(tic->filename, "..")) {
            qf_log(LOG_WARN, "TIC: rejecting filename with '..': %s", tic->filename);
            return -1;
        }
    }
    return (tic->filename[0] && tic->area[0]) ? 0 : -1;
}


/* ---- Scan Inbound for TIC Files ---- */

int tic_scan_inbound(const QfConfig *cfg)
{
    int count = 0;

#ifndef _WIN32
    DIR *d;
    struct dirent *ent;

    d = opendir(cfg->inbound);
    if (!d) return 0;

    while ((ent = readdir(d)) != NULL) {
        const char *name = ent->d_name;
        int len = (int)strlen(name);
        TicFile tic;
        char fullpath[520];
        char addr_buf[64];

        /* Look for .TIC files (case-insensitive) */
        if (len < 5) continue;
        if (strcasecmp(name + len - 4, ".tic") != 0) continue;

        snprintf(fullpath, sizeof(fullpath), "%s%c%s",
                 cfg->inbound, PATH_SEP, name);

        if (tic_parse(fullpath, &tic) == 0) {
            ftn_format_addr(&tic.from, addr_buf, sizeof(addr_buf));
            qf_log(LOG_INFO, "TIC: area=%s file=%s from=%s",
                   tic.area, tic.filename, addr_buf);
            count++;
        }
    }

    closedir(d);
#endif

    return count;
}


/* ---- Process TIC Files ----
 * Calls external TIC processor if configured.
 * Falls back to logging only. */

int tic_process(const QfConfig *cfg)
{
    int count;

    count = tic_scan_inbound(cfg);
    if (count == 0) return 0;

    qf_log(LOG_INFO, "TIC: %d file(s) to process", count);

    if (cfg->tic_proc[0]) {
        int rc;
        qf_log(LOG_INFO, "Running TIC processor: %s", cfg->tic_proc);
        rc = system(cfg->tic_proc);
        if (rc != 0)
            qf_log(LOG_WARN, "TIC processor exited with code %d", rc);
        return rc;
    }

    qf_log(LOG_WARN, "No TIC processor configured — files not processed");
    return 0;
}
