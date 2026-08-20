/* ====================================================================
 * bso.c — Binkley-Style Outbound Scanner for PCBoard BinkP
 * ====================================================================
 * Scans BSO outbound directory for files to send to a given FTN node.
 * Handles .flo (file attach), .hut/.dut/.cut/.out (mail bundles),
 * and .req (file requests).
 *
 * BSO directory layout:
 *   outbound/                        Zone 1 (default)
 *   outbound.002/                    Zone 2
 *   outbound.003/                    Zone 3
 *   Within zone dir:
 *     NNNNNNNN.flo                   Attach file (net<<16|node)
 *     NNNNNNNN.out                   Normal mail bundle
 *     NNNNNNNN.cut                   Crash mail
 *     NNNNNNNN.dut                   Direct mail
 *     NNNNNNNN.hut                   Hold mail
 *     NNNNNNNN.req                   File request
 *     NNNNNNNN.bsy                   Node busy flag
 *   Point directories:
 *     NNNNNNNN.pnt/PPPPPPPP.flo      Point outbound
 *
 * Copyright (C) 2026 pcbrevival contributors
 * License: GPLv3
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __OS2__
 #define INCL_DOSFILEMGR
 #include <os2.h>
#elif defined(__NT__) || defined(__WINDOWS__)
 #include <windows.h>
 #include <io.h>
#else
 #include <dirent.h>
 #include <unistd.h>
#endif

#include "binkp.h"

/* ====================================================================
 * Internal state for BSO scanning
 * ==================================================================== */

#define MAX_OUTFILES 256

typedef struct {
    char path[260];         /* full path to file                       */
    char flavour;           /* 'c'rash, 'd'irect, 'n'ormal, 'h'old    */
    int  is_flo_entry;      /* came from a .flo file                   */
    int  sent;              /* already sent this session                */
} OutFile;

static OutFile g_outfiles[MAX_OUTFILES];
static int     g_outcount = 0;
static int     g_outcur   = 0;

/* ====================================================================
 * Build BSO hex filename for a node
 * ==================================================================== */

static void bso_hex_name(const FtnAddr *addr, char *hex8)
{
    unsigned long val;
    val = ((unsigned long)addr->net << 16) | (unsigned long)addr->node;
    sprintf(hex8, "%08lx", val);
}

/* Build zone outbound directory path */
static void bso_zone_dir(const char *base, const FtnAddr *addr,
                         char *buf, int bufsize)
{
    if (addr->zone <= 1)
        snprintf(buf, bufsize, "%s", base);
    else
        snprintf(buf, bufsize, "%s.%03x", base, addr->zone);
}

/* ====================================================================
 * Parse a .flo file — each line is a filename to send
 * ==================================================================== */

static int parse_flo(const char *flo_path, char flavour)
{
    FILE *fp;
    char line[512];

    fp = fopen(flo_path, "r");
    if (!fp)
        return -1;

    while (fgets(line, sizeof(line), fp) && g_outcount < MAX_OUTFILES) {
        char *p = line;
        int delete_after = 0;
        int truncate_after = 0;
        struct stat st;

        /* Trim trailing whitespace */
        {
            char *end = p + strlen(p) - 1;
            while (end >= p && (*end == '\r' || *end == '\n' || *end == ' '))
                *end-- = '\0';
        }

        /* Skip empty lines and comments */
        if (*p == '\0' || *p == ';')
            continue;

        /* Leading ^ = delete after send, # = truncate after send */
        if (*p == '^') { delete_after = 1; p++; }
        else if (*p == '#') { truncate_after = 1; p++; }

        /* Check file exists */
        if (stat(p, &st) != 0) {
            binkp_log(2, "FLO: skipping missing file: %s", p);
            continue;
        }

        strncpy(g_outfiles[g_outcount].path, p,
                sizeof(g_outfiles[g_outcount].path) - 1);
        g_outfiles[g_outcount].flavour = flavour;
        g_outfiles[g_outcount].is_flo_entry = 1;
        g_outfiles[g_outcount].sent = 0;
        g_outcount++;
    }

    fclose(fp);
    return 0;
}

/* ====================================================================
 * Scan outbound for a remote address
 * ==================================================================== */

int bso_scan_outbound(BinkpSession *s, const FtnAddr *remote)
{
    char zone_dir[260];
    char hex[12];
    char path[520];
    struct stat st;
    static const char *flavours = "cohd";   /* crash, out, hold, direct */
    static const char *fexts[]  = { ".cut", ".out", ".hut", ".dut" };
    int i;

    g_outcount = 0;
    g_outcur   = 0;

    bso_hex_name(remote, hex);
    bso_zone_dir(s->outbound, remote, zone_dir, sizeof(zone_dir));

    binkp_log(2, "Scanning BSO: %s for %s", zone_dir, hex);

    /* Check for .flo files (file attaches) — all flavours */
    for (i = 0; i < 4 && g_outcount < MAX_OUTFILES; i++) {
        snprintf(path, sizeof(path), "%s/%s.%clo", zone_dir, hex,
                 flavours[i]);
        if (stat(path, &st) == 0)
            parse_flo(path, flavours[i]);
    }

    /* Check for mail bundles (.cut, .out, .hut, .dut) */
    for (i = 0; i < 4 && g_outcount < MAX_OUTFILES; i++) {
        snprintf(path, sizeof(path), "%s/%s%s", zone_dir, hex, fexts[i]);
        if (stat(path, &st) == 0) {
            strncpy(g_outfiles[g_outcount].path, path,
                    sizeof(g_outfiles[g_outcount].path) - 1);
            g_outfiles[g_outcount].flavour = flavours[i];
            g_outfiles[g_outcount].is_flo_entry = 0;
            g_outfiles[g_outcount].sent = 0;
            g_outcount++;
        }
    }

    /* Check for .req (file request) */
    snprintf(path, sizeof(path), "%s/%s.req", zone_dir, hex);
    if (stat(path, &st) == 0) {
        binkp_log(1, "File request found: %s", path);
        /* TODO: implement FREQ handling */
    }

    /* Point outbound: zone_dir/NNNNNNNN.pnt/PPPPPPPP.* */
    if (remote->point > 0) {
        char pnt_dir[520];
        char pnt_hex[12];

        snprintf(pnt_dir, sizeof(pnt_dir), "%s/%s.pnt", zone_dir, hex);
        sprintf(pnt_hex, "%08x", (unsigned int)remote->point);

        for (i = 0; i < 4 && g_outcount < MAX_OUTFILES; i++) {
            snprintf(path, sizeof(path), "%s/%s.%clo",
                     pnt_dir, pnt_hex, flavours[i]);
            if (stat(path, &st) == 0)
                parse_flo(path, flavours[i]);
        }
        for (i = 0; i < 4 && g_outcount < MAX_OUTFILES; i++) {
            snprintf(path, sizeof(path), "%s/%s%s",
                     pnt_dir, pnt_hex, fexts[i]);
            if (stat(path, &st) == 0) {
                strncpy(g_outfiles[g_outcount].path, path,
                        sizeof(g_outfiles[g_outcount].path) - 1);
                g_outfiles[g_outcount].flavour = flavours[i];
                g_outfiles[g_outcount].is_flo_entry = 0;
                g_outfiles[g_outcount].sent = 0;
                g_outcount++;
            }
        }
    }

    binkp_log(1, "Found %d file(s) for %u:%u/%u.%u",
              g_outcount, remote->zone, remote->net,
              remote->node, remote->point);

    return g_outcount;
}

/* ====================================================================
 * Get next file to send
 * Returns 0 if file opened in s->out_file, -1 if no more files
 * ==================================================================== */

int bso_next_file(BinkpSession *s)
{
    struct stat st;
    char *basename;

    while (g_outcur < g_outcount) {
        OutFile *of = &g_outfiles[g_outcur];
        g_outcur++;

        if (of->sent)
            continue;

        if (stat(of->path, &st) != 0) {
            binkp_log(2, "BSO: file gone: %s", of->path);
            continue;
        }

        s->out_file.fp = fopen(of->path, "rb");
        if (!s->out_file.fp) {
            binkp_log(0, "BSO: cannot open: %s", of->path);
            continue;
        }

        /* Extract basename */
        basename = strrchr(of->path, '/');
        if (!basename) basename = strrchr(of->path, '\\');
        basename = basename ? basename + 1 : of->path;

        strncpy(s->out_file.name, basename, sizeof(s->out_file.name) - 1);
        s->out_file.size   = (long)st.st_size;
        s->out_file.time   = (long)st.st_mtime;
        s->out_file.offset = 0;

        of->sent = 1;

        binkp_log(2, "BSO: queued %s (%ld bytes, flavour '%c')",
                  s->out_file.name, s->out_file.size, of->flavour);
        return 0;
    }

    return -1;  /* no more files */
}

/* ====================================================================
 * Mark a file as successfully sent (remove/truncate BSO entries)
 * ==================================================================== */

void bso_mark_sent(BinkpSession *s, const char *filename)
{
    /* For now just log it — full .flo line removal is phase 2 */
    binkp_log(2, "BSO: marking sent: %s", filename);

    /* TODO:
     * - Remove sent .out/.cut/.dut/.hut files
     * - Remove/comment lines from .flo files
     * - Delete .bsy lock file at session end
     */
}
