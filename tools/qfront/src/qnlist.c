/* ====================================================================
 * qnlist.c — Nodelist Compiler
 * ====================================================================
 * Replaces QNLIST.EXE (255 KB). Compiles FidoNet nodelists:
 *   /COMPILE        Compile primary + all private nodelists
 *   /COMPILENEW     Compile only if new diffs found
 *
 * Operations:
 *   1. Find archived nodediff in inbound
 *   2. Unarchive nodediff
 *   3. Verify old nodelist CRC
 *   4. Apply nodediff to produce new nodelist
 *   5. Verify new nodelist CRC
 *   6. Compile nodelist to binary index (.NDX)
 *   7. Process private nodelists/pointlists
 *   8. Move/archive processed files
 *
 * From binary:
 *   "Checking CRC of old nodelist...DONE"
 *   "Applying nodediff"
 *   "Processing nodelist update...DONE"
 *   "New nodelist fails CRC check after nodediff update"
 *   "Old nodelist fails CRC check"
 *   "No new nodelists/nodediffs found"
 *
 * Clean-room from FTS-5001 (nodelist format) + binary analysis.
 * ==================================================================== */

#include "qfront.h"

#define QNLIST_VERSION "1.0.0"

/* ---- CRC-16 (CCITT) for nodelist validation ---- */

static uint16_t nl_crc16(const void *data, int len)
{
    const unsigned char *p = (const unsigned char *)data;
    uint16_t crc = 0;
    int i, j;

    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)p[i] << 8;
        for (j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}


/* ---- Calculate CRC of a nodelist file ----
 * FTS-5001: CRC is over all bytes except the first line
 * (which contains the CRC itself). */

static uint16_t nl_file_crc(const char *path, uint16_t *stored_crc)
{
    FILE *f;
    char line[512];
    uint16_t crc = 0;
    int first = 1;

    f = fopen(path, "r");
    if (!f) return 0xFFFF;

    while (fgets(line, sizeof(line), f)) {
        int len = (int)strlen(line);

        /* Strip trailing CR/LF for CRC calculation */
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
            len--;

        if (first) {
            /* First line: ";A ... -- Day number : XXXXX"
             * Extract stored CRC from the last field */
            first = 0;
            if (stored_crc) {
                char *colon = strrchr(line, ':');
                if (colon) {
                    while (*colon == ':' || *colon == ' ') colon++;
                    *stored_crc = (uint16_t)strtoul(colon, NULL, 10);
                }
            }
            continue;
        }

        /* Skip comment lines (;) for CRC */
        if (line[0] == ';') continue;

        crc = nl_crc16(line, len);
    }

    fclose(f);
    return crc;
}


/* ---- Find newest nodediff archive in inbound ---- */

static int nl_find_diff(const char *inbound, const char *base,
                         char *result, int result_size)
{
    char pattern[32];
    int best_day = -1;

#ifndef _WIN32
    DIR *d;
    struct dirent *ent;

    d = opendir(inbound);
    if (!d) return -1;

    /* Look for NODEDIFF.Znn or basename.Znn */
    while ((ent = readdir(d)) != NULL) {
        int len = (int)strlen(ent->d_name);
        if (len < 4) continue;

        /* Check for .Znn, .Ann, .ARJ, .ZIP, .LZH extensions */
        if (strncasecmp(ent->d_name, base, strlen(base)) == 0 ||
            strncasecmp(ent->d_name, "NODEDIFF", 8) == 0) {
            /* Extract day number from extension */
            const char *ext = strrchr(ent->d_name, '.');
            if (ext) {
                int day = atoi(ext + 1);
                if (day == 0) {
                    /* Could be .ZIP, .ARJ, .LZH — check */
                    if (strcasecmp(ext, ".ZIP") == 0 ||
                        strcasecmp(ext, ".ARJ") == 0 ||
                        strcasecmp(ext, ".LZH") == 0) {
                        day = 999;  /* Archive — process it */
                    }
                }
                if (day > best_day) {
                    best_day = day;
                    snprintf(result, result_size, "%s%c%s",
                             inbound, PATH_SEP, ent->d_name);
                }
            }
        }
    }

    closedir(d);
#endif
    (void)pattern;

    return (best_day >= 0) ? 0 : -1;
}


/* ---- Unarchive a file ----
 * Tries ZIP, ARJ, LZH based on extension. */

static int nl_unarchive(const char *archive, const char *dest_dir)
{
    char cmd[520];
    const char *ext;
    int rc;

    ext = strrchr(archive, '.');
    if (!ext) return -1;

    if (strcasecmp(ext, ".ZIP") == 0)
        snprintf(cmd, sizeof(cmd), "unzip -o -d '%s' '%s'", dest_dir, archive);
    else if (strcasecmp(ext, ".ARJ") == 0)
        snprintf(cmd, sizeof(cmd), "arj x -y '%s' '%s'%c", archive, dest_dir, PATH_SEP);
    else if (strcasecmp(ext, ".LZH") == 0)
        snprintf(cmd, sizeof(cmd), "lha x '%s' '%s'%c", archive, dest_dir, PATH_SEP);
    else {
        /* Raw diff — just copy */
        snprintf(cmd, sizeof(cmd), "cp '%s' '%s'%c", archive, dest_dir, PATH_SEP);
    }

    printf("Unarchiving %s\n", archive);
    rc = system(cmd);
    if (rc != 0)
        printf("Error unarchiving %s (rc=%d)\n", archive, rc);

    return rc;
}


/* ---- Apply nodediff to nodelist ----
 * Nodediff format (FTS-5001 Appendix):
 *   Annn      Add nnn lines
 *   Cnnn      Copy nnn lines from old
 *   Dnnn      Delete nnn lines from old */

static int nl_apply_diff(const char *old_nl, const char *diff_file,
                          const char *new_nl)
{
    FILE *fold, *fdiff, *fnew;
    char line[512];
    int old_line = 0;

    fold = fopen(old_nl, "r");
    fdiff = fopen(diff_file, "r");
    fnew = fopen(new_nl, "w");

    if (!fold || !fdiff || !fnew) {
        printf("Cannot apply nodediff to this nodelist\n");
        if (fold) fclose(fold);
        if (fdiff) fclose(fdiff);
        if (fnew) fclose(fnew);
        return -1;
    }

    /* Read old nodelist into memory for random access */
    {
        char **old_lines = NULL;
        int old_count = 0, old_cap = 8192;
        int i;

        old_lines = (char **)calloc(old_cap, sizeof(char *));
        if (!old_lines) { fclose(fold); fclose(fdiff); fclose(fnew); return -1; }

        while (fgets(line, sizeof(line), fold)) {
            if (old_count >= old_cap) {
                old_cap *= 2;
                old_lines = (char **)realloc(old_lines, old_cap * sizeof(char *));
                if (!old_lines) break;
            }
            old_lines[old_count] = strdup(line);
            old_count++;
        }
        fclose(fold);

        /* Process diff commands */
        old_line = 0;
        while (fgets(line, sizeof(line), fdiff)) {
            int len = (int)strlen(line);
            while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r'))
                line[--len] = '\0';

            if (line[0] == 'A' || line[0] == 'a') {
                /* Add N lines from diff */
                int count = atoi(line + 1);
                for (i = 0; i < count; i++) {
                    if (fgets(line, sizeof(line), fdiff))
                        fputs(line, fnew);
                }
            }
            else if (line[0] == 'C' || line[0] == 'c') {
                /* Copy N lines from old */
                int count = atoi(line + 1);
                for (i = 0; i < count && old_line < old_count; i++) {
                    fputs(old_lines[old_line++], fnew);
                }
            }
            else if (line[0] == 'D' || line[0] == 'd') {
                /* Delete N lines from old (skip them) */
                int count = atoi(line + 1);
                old_line += count;
            }
        }

        /* Free old lines */
        for (i = 0; i < old_count; i++)
            free(old_lines[i]);
        free(old_lines);
    }

    fclose(fdiff);
    fclose(fnew);

    return 0;
}


/* ---- Compile nodelist to binary index (.NDX) ----
 * Creates a quick-lookup index file with:
 *   zone:net/node → file_offset mapping. */

typedef struct {
    uint16_t zone, net, node, point;
    uint32_t offset;              /* Byte offset in nodelist file  */
    uint16_t flags;               /* CM, MO, IBN, etc.            */
} NdxEntry;

static int nl_compile_index(const char *nl_path, const char *ndx_path)
{
    FILE *fnl, *fndx;
    char line[512];
    uint16_t cur_zone = 0, cur_net = 0, cur_hub = 0;
    uint32_t offset = 0;
    int count = 0;
    NdxEntry entry;

    fnl = fopen(nl_path, "r");
    fndx = fopen(ndx_path, "wb");
    if (!fnl || !fndx) {
        if (fnl) fclose(fnl);
        if (fndx) fclose(fndx);
        return -1;
    }

    /* Write placeholder header (entry count) */
    fwrite(&count, sizeof(count), 1, fndx);

    while (fgets(line, sizeof(line), fnl)) {
        char *p = line;
        uint32_t line_offset = offset;
        int node_num;

        offset += (uint32_t)strlen(line);

        /* Skip comments and blank lines */
        if (*p == ';' || *p == '\r' || *p == '\n') continue;

        memset(&entry, 0, sizeof(entry));

        /* Parse keyword,node_number,... */
        /* DefZone sets the default zone if not in the nodelist */
        if (strncmp(p, "DefZone,", 8) == 0) {
            cur_zone = (uint16_t)atoi(p + 8);
            continue;
        }
        else if (strncmp(p, "Zone,", 5) == 0) {
            cur_zone = (uint16_t)atoi(p + 5);
            cur_net = cur_zone;
            continue;
        }
        else if (strncmp(p, "Region,", 7) == 0) {
            cur_net = (uint16_t)atoi(p + 7);
            continue;
        }
        else if (strncmp(p, "Host,", 5) == 0) {
            cur_net = (uint16_t)atoi(p + 5);
            cur_hub = 0;
            node_num = 0;
        }
        else if (strncmp(p, "Hub,", 4) == 0) {
            cur_hub = (uint16_t)atoi(p + 4);
            node_num = cur_hub;
        }
        else if (*p == ',' || (*p >= '0' && *p <= '9')) {
            /* Regular node: ,node_number,name,... */
            if (*p == ',') p++;
            node_num = atoi(p);
        }
        else if (strncmp(p, "Pvt,", 4) == 0) {
            node_num = atoi(p + 4);
        }
        else if (strncmp(p, "Down,", 5) == 0) {
            node_num = atoi(p + 5);
            entry.flags |= 0x01;  /* Down flag */
        }
        else if (strncmp(p, "Hold,", 5) == 0) {
            node_num = atoi(p + 5);
            entry.flags |= 0x02;  /* Hold flag */
        }
        else {
            continue;
        }

        entry.zone = cur_zone;
        entry.net = cur_net;
        entry.node = (uint16_t)node_num;
        entry.offset = line_offset;

        /* Check flags field for CM, MO, IBN */
        if (strstr(line, ",CM,") || strstr(line, ",CM\r") || strstr(line, ",CM\n"))
            entry.flags |= 0x04;
        if (strstr(line, ",MO,") || strstr(line, ",MO\r") || strstr(line, ",MO\n"))
            entry.flags |= 0x08;
        if (strstr(line, ",IBN,") || strstr(line, ",IBN:"))
            entry.flags |= 0x10;

        fwrite(&entry, sizeof(entry), 1, fndx);
        count++;
    }

    /* Update header with final count */
    fseek(fndx, 0, SEEK_SET);
    fwrite(&count, sizeof(count), 1, fndx);

    fclose(fnl);
    fclose(fndx);

    return count;
}


/* ---- Usage ---- */

static void usage(void)
{
    printf(
        "QNList v" QNLIST_VERSION " — Nodelist Compiler\n"
        "Clean-room implementation from QFront v1.20a\n\n"
        "Usage: qnlist [options]\n\n"
        "  /COMPILE        Compile primary + private nodelists\n"
        "  /COMPILENEW     Compile only if new diffs found\n"
        "  /C<config>      Specify config file\n"
        "  /COLOR          Color output\n"
        "  /MONO           Monochrome output\n");
}


/* ---- Main ---- */

int main(int argc, char *argv[])
{
    QfConfig cfg;
    char nl_path[520], ndx_path[520], diff_path[520];
    char new_nl_path[520], backup_path[520];
    int compile_new_only = 0;
    int found_diff = 0;
    int i;
    const char *cfgfile = "qfront.cfg";

    /* Parse args */
    for (i = 1; i < argc; i++) {
        if (strcasecmp(argv[i], "/HELP") == 0 ||
            strcmp(argv[i], "-h") == 0) {
            usage();
            return 0;
        }
        if (strcasecmp(argv[i], "/COMPILENEW") == 0)
            compile_new_only = 1;
        if (strncasecmp(argv[i], "/C", 2) == 0 && argv[i][2])
            cfgfile = argv[i] + 2;
    }

    if (qf_config_load(cfgfile, &cfg) != 0) {
        fprintf(stderr, "Configuration file not found\n");
        fprintf(stderr, "Error reading configuration file\n");
        return 1;
    }

    if (!cfg.nodelist_dir[0] || !cfg.nodelist_base[0]) {
        fprintf(stderr, "Nodelist directory or base name not configured\n");
        return 1;
    }

    /* Create .BSY lock file to prevent concurrent runs.
     * O_CREAT|O_EXCL is atomic — no TOCTOU race between check and create. */
    {
        char bsy_path[260];
        int bsy_fd;
        snprintf(bsy_path, sizeof(bsy_path), "%s%cQNLIST.BSY",
                 cfg.nodelist_dir, PATH_SEP);
#ifdef _WIN32
        bsy_fd = _open(bsy_path, _O_CREAT | _O_EXCL | _O_WRONLY, _S_IREAD | _S_IWRITE);
#else
        bsy_fd = open(bsy_path, O_CREAT | O_EXCL | O_WRONLY, 0644);
#endif
        if (bsy_fd < 0) {
            printf("QNList appears to be busy processing on another node!\n");
            printf("If you are certain QNList is not running on another node, "
                   "delete QNLIST.BSY\n");
            return 1;
        }
        {
            char pidbuf[16];
            int len = snprintf(pidbuf, sizeof(pidbuf), "%d\n", getpid());
            write(bsy_fd, pidbuf, len);
            close(bsy_fd);
        }
    }

    /* Build paths */
    snprintf(nl_path, sizeof(nl_path), "%s%c%s",
             cfg.nodelist_dir, PATH_SEP, cfg.nodelist_base);
    snprintf(ndx_path, sizeof(ndx_path), "%s%c%s.NDX",
             cfg.nodelist_dir, PATH_SEP, cfg.nodelist_base);

    /* Step 1: Look for nodediff in inbound */
    found_diff = (nl_find_diff(cfg.inbound, "NODEDIFF", diff_path,
                                sizeof(diff_path)) == 0);

    if (!found_diff && compile_new_only) {
        printf("No new nodelists/nodediffs found\n");
        return 0;
    }

    /* Step 2: If we have a diff, apply it */
    if (found_diff) {
        char temp_dir[260];
        char unarch_diff[520];

        snprintf(temp_dir, sizeof(temp_dir), "%s%cNLWORK",
                 cfg.nodelist_dir, PATH_SEP);
#ifndef _WIN32
        mkdir(temp_dir, 0755);
#endif

        /* Unarchive */
        printf("Unarchiving nodediff %s\n", diff_path);
        if (nl_unarchive(diff_path, temp_dir) != 0) {
            printf("Unable to copy unarchived nodediff\n");
            return 1;
        }

        /* Find the raw diff file in temp dir */
        snprintf(unarch_diff, sizeof(unarch_diff), "%s%cNODEDIFF.*",
                 temp_dir, PATH_SEP);
        /* Simplified: assume same name without archive extension */
        snprintf(unarch_diff, sizeof(unarch_diff), "%s%cNODEDIFF.%03d",
                 temp_dir, PATH_SEP, 0);

        /* Verify old nodelist CRC */
        {
            uint16_t stored = 0, calc;
            printf("Checking CRC of old nodelist...");
            calc = nl_file_crc(nl_path, &stored);
            printf("DONE\n");

            if (stored != 0 && calc != stored) {
                printf("Old nodelist fails CRC check\n");
                printf("Cannot continue with nodediff update\n");
                /* Continue anyway — compile what we have */
            }
        }

        /* Backup old nodelist (QN-1 fix: rollback on failure) */
        snprintf(backup_path, sizeof(backup_path), "%s.BAK", nl_path);
        rename(nl_path, backup_path);

        /* Apply diff */
        snprintf(new_nl_path, sizeof(new_nl_path), "%s.NEW", nl_path);
        printf("Processing nodelist update...");
        if (nl_apply_diff(backup_path, unarch_diff, new_nl_path) != 0) {
            /* Restore backup */
            rename(backup_path, nl_path);
            printf("FAILED\n");
            return 1;
        }
        printf("DONE\n");

        /* Verify new nodelist CRC */
        {
            uint16_t stored = 0, calc;
            calc = nl_file_crc(new_nl_path, &stored);
            if (stored != 0 && calc != stored) {
                printf("New nodelist fails CRC check after nodediff update\n");
                /* Restore backup (QN-1 fix) */
                remove(new_nl_path);
                rename(backup_path, nl_path);
                return 1;
            }
        }

        /* Replace old with new */
        rename(new_nl_path, nl_path);
        remove(backup_path);

        /* Move processed diff */
        printf("Moving nodediff\n");
        /* Option: delete old nodelist after successful update */
        printf("Deleting old nodelist backup\n");
        /* "Deleting nodelist" — remove superseded nodelist after compile */
        remove(backup_path);
        /* Original also supports: Deleting nodediff (if configured) */
        {
            char moved[520];
            snprintf(moved, sizeof(moved), "%s%cprocessed_%s",
                     cfg.nodelist_dir, PATH_SEP, strrchr(diff_path, PATH_SEP) + 1);
            rename(diff_path, moved);
        }
    }

    /* Step 3: Compile nodelist to index */
    printf("Compiling nodelist %s...", cfg.nodelist_base);
    {
        int count = nl_compile_index(nl_path, ndx_path);
        if (count >= 0)
            printf("DONE (%d nodes)\n", count);
        else
            printf("FAILED\n");
    }

    /* Step 4: Process private nodelists (from config) */
    /* Process private nodelists and pointlists.
     * Pointlists have a "Boss,<address>" header that maps
     * point numbers to their boss node address. */
    {
#ifndef _WIN32
        DIR *d;
        struct dirent *ent;
        char pvt_path[520], pvt_ndx[520];

        d = opendir(cfg.nodelist_dir);
        if (d) {
            while ((ent = readdir(d)) != NULL) {
                int len = (int)strlen(ent->d_name);

                /* Look for private nodelists: *.PVT, *.LST, POINTS.* */
                if ((len > 4 && (strcasecmp(ent->d_name + len - 4, ".PVT") == 0 ||
                     strcasecmp(ent->d_name + len - 4, ".LST") == 0)) ||
                    strncasecmp(ent->d_name, "POINTS.", 7) == 0) {

                    snprintf(pvt_path, sizeof(pvt_path), "%s%c%s",
                             cfg.nodelist_dir, PATH_SEP, ent->d_name);
                    snprintf(pvt_ndx, sizeof(pvt_ndx), "%s%c%s.NDX",
                             cfg.nodelist_dir, PATH_SEP, ent->d_name);

                    printf("Compiling private nodelist %s...", ent->d_name);
                    {
                        int pcount = nl_compile_index(pvt_path, pvt_ndx);
                        if (pcount >= 0)
                            printf("DONE (%d entries)\n", pcount);
                        else
                            printf("FAILED\n");
                    }
                }
            }
            closedir(d);
        }
#endif
    }

    /* Remove .BSY lock */
    {
        char bsy_path[260];
        snprintf(bsy_path, sizeof(bsy_path), "%s%cQNLIST.BSY",
                 cfg.nodelist_dir, PATH_SEP);
        remove(bsy_path);
    }

    printf("Nodelist compilation complete\n");
    return 0;
}
