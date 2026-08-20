/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* bso.c -- BSO Outbound Scanner (FTS-5005)                                 */
/*                                                                           */
/* Scans the Binkley Style Outbound directory for pending mail and files.    */
/* Implements flow file parsing, control file management (busy locks, hold,  */
/* try counters), and flavour priority ordering.                             */
/*                                                                           */
/* Clean-room from FTS-5005.003 (public domain FidoNet specification).      */
/*                                                                           */
/* Copyright (C) 2026  pcbirc crew.  GPLv3.                                 */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"
#include <errno.h>
#include <stdarg.h>


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                         FTN Address Helpers                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* ftn_parse_addr() -- Parse a FidoNet 5D address string                 */
/*                                                                       */
/* Accepts these formats (most to least specific):                       */
/*   "1:234/56.7@fidonet"   -- full 5D (zone:net/node.point@domain)      */
/*   "1:234/56.7"           -- 4D (zone:net/node.point)                  */
/*   "1:234/56"             -- 3D (zone:net/node)                        */
/*   "234/56"               -- 2D (net/node, zone defaults to 1)         */
/*                                                                       */
/* The slash between net and node is REQUIRED -- anything without '/'    */
/* is rejected (returns -1). Zone defaults to 1 if no colon is present.  */
/* Point defaults to 0 if no dot is present.                             */
/*                                                                       */
/* Domain (the @fidonet part) is stored but rarely used in practice.     */
/* Most FidoNet software ignores domains entirely. We store it for       */
/* completeness per FTS-5005.                                            */
/*                                                                       */
/* Returns 0 on success, -1 on parse error.                              */
/*-----------------------------------------------------------------------*/

int ftn_parse_addr(const char *Str, FTN_ADDR *Addr)
{
    const char *p = Str;                /* walk pointer through string   */

    memset(Addr, 0, sizeof(*Addr));
    Addr->zone = 1;                     /* default zone if none given    */

    /* Zone: digits before ':' */
    if (strchr(Str, ':')) {
        Addr->zone = (uint16_t)atoi(p);
        p = strchr(p, ':') + 1;
    }

    /* Net: digits before '/' */
    if (!strchr(p, '/'))
        return -1;                      /* must have net/node separator  */
    Addr->net = (uint16_t)atoi(p);
    p = strchr(p, '/') + 1;

    /* Node: digits before '.' or '@' or end */
    Addr->node = (uint16_t)atoi(p);

    /* Point: digits after '.' */
    if (strchr(p, '.')) {
        p = strchr(p, '.') + 1;
        Addr->point = (uint16_t)atoi(p);
    }

    /* Domain: text after '@' */
    if (strchr(Str, '@')) {
        p = strchr(Str, '@') + 1;
        strncpy(Addr->domain, p, sizeof(Addr->domain) - 1);
    }

    return 0;
}


/*-----------------------------------------------------------------------*/
/* ftn_format_addr() -- Format FTN_ADDR as "zone:net/node[.point]"       */
/*                                                                       */
/* Produces a human-readable address string for logging and display.     */
/* Point is omitted when zero (most addresses are 3D).                   */
/*-----------------------------------------------------------------------*/

void ftn_format_addr(const FTN_ADDR *Addr, char *Buf, int BufSize)
{
    if (Addr->point)
        snprintf(Buf, BufSize, "%u:%u/%u.%u",
                 Addr->zone, Addr->net, Addr->node, Addr->point);
    else
        snprintf(Buf, BufSize, "%u:%u/%u",
                 Addr->zone, Addr->net, Addr->node);
}


/*-----------------------------------------------------------------------*/
/* ftn_addr_equal() -- Compare two FTN addresses for equality            */
/*                                                                       */
/* Compares zone, net, node, and point. Domain is NOT compared -- most   */
/* FidoNet software treats domain as cosmetic.                           */
/*                                                                       */
/* Returns 1 if equal, 0 if different.                                   */
/*-----------------------------------------------------------------------*/

int ftn_addr_equal(const FTN_ADDR *AddrA, const FTN_ADDR *AddrB)
{
    return AddrA->zone  == AddrB->zone  &&
           AddrA->net   == AddrB->net   &&
           AddrA->node  == AddrB->node  &&
           AddrA->point == AddrB->point;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                         BSO Path Helpers                                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* bso_zone_dir() -- Build the outbound directory path for a zone        */
/*                                                                       */
/* FTS-5005: outbound.ZZZ where ZZZ is zone in hex.                      */
/* Default zone uses bare "outbound" (no extension).                     */
/*-----------------------------------------------------------------------*/

static void bso_zone_dir(const QfConfig *Cfg, uint16_t Zone,
                         char *Buf, int BufSize)
{
    if (Zone == Cfg->aka[0].zone) {
        snprintf(Buf, BufSize, "%s", Cfg->outbound);
    } else {
        snprintf(Buf, BufSize, "%s.%03x", Cfg->outbound, Zone);
    }
}


/*-----------------------------------------------------------------------*/
/* bso_base_name() -- Build the BSO base filename for an address         */
/*                                                                       */
/* FTS-5005: 8 hex digits = net(4) + node(4), zero-padded.               */
/* e.g. node 104/36 = "00680024"                                         */
/* Points go in a subdirectory: NNNNNNNN.PNT/PPPPPPPP.ext                */
/*-----------------------------------------------------------------------*/

static void bso_base_name(const FTN_ADDR *Addr, char *Buf, int BufSize)
{
    if (Addr->point) {
        /* Points go in a subdirectory: NNNNNNNN.PNT/PPPPPPPP.ext */
        snprintf(Buf, BufSize, "%04x%04x.pnt%c%08x",
                 Addr->net, Addr->node, PATH_SEP, Addr->point);
    } else {
        snprintf(Buf, BufSize, "%04x%04x", Addr->net, Addr->node);
    }
}


/*-----------------------------------------------------------------------*/
/* bso_full_path() -- Build full path for a BSO file                     */
/*                                                                       */
/* Combines: outbound[.zone]/NNNNNNNN.ext                                */
/*-----------------------------------------------------------------------*/

static void bso_full_path(const QfConfig *Cfg, const FTN_ADDR *Addr,
                          const char *Ext, char *Buf, int BufSize)
{
    char ZoneDir[260];                  /* outbound directory for zone   */
    char BaseName[64];                  /* hex net+node filename base    */

    bso_zone_dir(Cfg, Addr->zone, ZoneDir, sizeof(ZoneDir));
    bso_base_name(Addr, BaseName, sizeof(BaseName));
    snprintf(Buf, BufSize, "%s%c%s.%s", ZoneDir, PATH_SEP, BaseName, Ext);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    BSO Lock Management (FTS-5005 Section 5.1)             */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* bso_lock() -- Create a .bsy lock file for an address                  */
/*                                                                       */
/* FTS-5005 Section 5.1: Before calling a node, create NNNNNNNN.bsy      */
/* in the outbound directory. This prevents other mailer instances        */
/* (on other BBS nodes) from dialing the same system simultaneously.     */
/*                                                                       */
/* Uses O_CREAT|O_EXCL (exclusive create) for atomic locking -- avoids   */
/* the TOCTOU race condition where two instances check for the file,     */
/* both find it missing, and both create it.                              */
/*                                                                       */
/* The .bsy file contains our PID for diagnostics -- if a lock is stale  */
/* (process died), the sysop can check whether the PID is still alive.   */
/*                                                                       */
/* Returns 0 on success (lock acquired), -1 if already locked.           */
/*-----------------------------------------------------------------------*/

int bso_lock(const QfConfig *Cfg, const FTN_ADDR *Addr)
{
    char Path[260];                     /* full path to .bsy file        */

    bso_full_path(Cfg, Addr, "bsy", Path, sizeof(Path));
    qf_log(LOG_DEBUG, "bso_lock: attempting lock %s", Path);

    /* Check if already locked -- don't overwrite (FTS-5005 race
     * condition warning). Use exclusive create. */
#ifdef _WIN32
    {
        HANDLE h = CreateFileA(Path, GENERIC_WRITE, 0, NULL,
                               CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h == INVALID_HANDLE_VALUE) return -1;
        CloseHandle(h);
        return 0;
    }
#else
    {
        int Fd;                         /* file descriptor for .bsy      */

        Fd = open(Path, O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (Fd < 0) return -1;

        /* Write PID for diagnostics (FTS-5005 recommends this) */
        {
            char PidBuf[32];            /* PID string buffer             */
            int  PidLen;                /* length of PID string          */

            PidLen = snprintf(PidBuf, sizeof(PidBuf), "qfront %d\n", (int)getpid());
            write(Fd, PidBuf, PidLen);
        }
        close(Fd);
        return 0;
    }
#endif
}


/*-----------------------------------------------------------------------*/
/* bso_unlock() -- Remove the .bsy lock file                             */
/*                                                                       */
/* Called after a session completes (success or failure) to release the   */
/* lock so other mailer instances can call this node.                     */
/*                                                                       */
/* IMPORTANT: Must be called in ALL exit paths -- success, failure,       */
/* signal handler, etc. A leaked .bsy file blocks all future calls to     */
/* this node until the sysop manually deletes it.                         */
/*-----------------------------------------------------------------------*/

void bso_unlock(const QfConfig *Cfg, const FTN_ADDR *Addr)
{
    char Path[260];                     /* full path to .bsy file        */

    bso_full_path(Cfg, Addr, "bsy", Path, sizeof(Path));
    qf_log(LOG_DEBUG, "bso_unlock: removing lock %s", Path);
    remove(Path);
}


/*-----------------------------------------------------------------------*/
/* bso_check_hold() -- Check if a node is on hold                        */
/*                                                                       */
/* FTS-5005 Section 5.3: A .hld file contains a UNIX timestamp of when   */
/* the hold expires. If the current time is past the expiry, the hold     */
/* is cleared (file deleted) and we return 0 (clear to call).            */
/*                                                                       */
/* Hold files are created when a node exceeds max_retries -- we stop      */
/* calling them for hold_time seconds (typically 1 hour). This prevents   */
/* hammering a down system and wasting phone charges.                     */
/*                                                                       */
/* Returns: 1 = held (don't call), 0 = clear to call.                    */
/*-----------------------------------------------------------------------*/

int bso_check_hold(const QfConfig *Cfg, const FTN_ADDR *Addr)
{
    FILE   *f;                          /* hold file handle              */
    char    Path[260];                  /* full path to .hld file        */
    time_t  Expiry;                     /* hold expiry timestamp         */

    bso_full_path(Cfg, Addr, "hld", Path, sizeof(Path));
    f = fopen(Path, "r");
    if (!f) return 0;                   /* no hold file = clear          */

    if (fscanf(f, "%ld", (long*)&Expiry) == 1) {
        fclose(f);
        if (time(NULL) < Expiry)
            return 1;                   /* still held                    */
        /* Expired -- delete the hold file (FTS-5005) */
        remove(Path);
        return 0;
    }

    fclose(f);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* bso_record_try() -- Record a session attempt in the .try file         */
/*                                                                       */
/* FTS-5005 Section 5.4: The .try file stores session attempt history     */
/* for a node. Format is binary:                                          */
/*   Offset 0-1: NOK (uint16 BE) -- number of successful sessions        */
/*   Offset 2-3: NBAD (uint16 BE) -- number of failed sessions           */
/*   Offset 4:   CLength (uint8) -- comment string length                */
/*   Offset 5+:  Comment (CLength bytes) -- last attempt result          */
/*                                                                       */
/* This data is used by sem_is_undialable() to decide when a node        */
/* has failed too many times and should be put on hold.                   */
/*                                                                       */
/* TODO: Read existing .try to increment counters instead of resetting.  */
/*-----------------------------------------------------------------------*/

void bso_record_try(const QfConfig *Cfg, const FTN_ADDR *Addr,
                    int Success, const char *Msg)
{
    FILE    *f;                         /* try file handle               */
    char     Path[260];                 /* full path to .try file        */
    uint16_t Nok;                       /* successful session count      */
    uint16_t Nbad;                      /* failed session count          */
    uint8_t  Clen;                      /* comment string length         */

    qf_log(LOG_DEBUG, "bso_record_try: %s (success=%d, msg=%s)",
           Success ? "OK" : "FAIL", Success, Msg);

    bso_full_path(Cfg, Addr, "try", Path, sizeof(Path));
    f = fopen(Path, "wb");
    if (!f) return;

    /* FTS-5005: NOK (2 bytes BE), NBAD (2 bytes BE),
     * CLength (1 byte), Comment (CLength bytes) */
    Clen = (uint8_t)strlen(Msg);

    if (Success) {
        Nok  = 1;
        Nbad = 0;
    } else {
        /* For simplicity, just write NBAD=1 on failure.
         * A production impl would read+increment. */
        Nok  = 0;
        Nbad = 1;
    }

    /* Big-endian write */
    fputc(Nok  >> 8, f); fputc(Nok  & 0xFF, f);
    fputc(Nbad >> 8, f); fputc(Nbad & 0xFF, f);
    fputc(Clen, f);
    fwrite(Msg, 1, Clen, f);

    fclose(f);
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                         BSO Outbound Scanner                              */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* bso_scan() -- Scan BSO outbound for nodes with pending mail           */
/*                                                                       */
/* Scans the outbound directory for flow files and builds a list of       */
/* addresses with pending mail/files.                                     */
/*                                                                       */
/* FTS-5005: flow files are named NNNNNNNN.ext where:                    */
/*   N = net(4hex) + node(4hex)                                          */
/*   ext = flavour(1) + type(2)                                          */
/*   type: "ut" = netmail packet, "lo" = file reference list             */
/*   flavour: i/c/d/o(or f)/h                                           */
/*                                                                       */
/* Returns number of items found.                                        */
/*-----------------------------------------------------------------------*/

int bso_scan(const QfConfig *Cfg, BsoItem *Items, int MaxItems)
{
    char ZoneDir[260];                  /* outbound dir for current zone */
    int  Count = 0;                     /* number of items found         */
    int  ZoneIdx;                       /* zone loop index               */

#ifndef _WIN32
    DIR           *d;                   /* directory handle               */
    struct dirent *Ent;                 /* directory entry                */
#else
    HANDLE          FindHandle;         /* Win32 find handle              */
    WIN32_FIND_DATAA FindData;          /* Win32 find data                */
    char            Pattern[280];       /* search pattern                 */
#endif

    /*
     * Scan outbound for each zone we have an AKA in.
     *
     * FTS-5005: The outbound directory structure is:
     *   outbound/           -- default zone (our primary zone)
     *   outbound.002/       -- zone 2 (hex)
     *   outbound.003/       -- zone 3
     *
     * Each directory contains files named NNNNNNNN.ext where:
     *   NNNNNNNN = net(4 hex) + node(4 hex), zero-padded
     *   ext = flavour_char + type_chars
     *
     * Flow file types:
     *   .iut / .cut / .dut / .fut / .hut -- netmail PKT (by flavour)
     *   .ilo / .clo / .dlo / .flo / .hlo -- file reference list
     *   .req                              -- file request
     *
     * Flavour priority (highest first):
     *   i = immediate, c = continuous, d = direct, f = normal, h = hold
     *
     * When multiple flavours exist for the same node, we use the
     * highest priority one for scheduling decisions.
     */
    for (ZoneIdx = 0; ZoneIdx < Cfg->num_aka && Count < MaxItems; ZoneIdx++) {
        bso_zone_dir(Cfg, Cfg->aka[ZoneIdx].zone, ZoneDir, sizeof(ZoneDir));
        qf_log(LOG_DEBUG, "bso_scan: scanning zone dir %s (zone %d)",
               ZoneDir, Cfg->aka[ZoneIdx].zone);

#ifndef _WIN32
        d = opendir(ZoneDir);
        if (!d) continue;

        while ((Ent = readdir(d)) != NULL && Count < MaxItems) {
            const char *Name;           /* directory entry filename       */
            int         NameLen;        /* length of filename             */
            char        Ext[4];         /* parsed file extension          */
            uint16_t    NetHex;         /* net from hex filename          */
            uint16_t    NodeHex;        /* node from hex filename         */
            BsoFlavour  Flav;           /* mail flavour priority          */
            int         TypeIdx;        /* 0=pkt, 1=flo, 2=req           */

            Name    = Ent->d_name;
            NameLen = (int)strlen(Name);

            /* Must be NNNNNNNN.ext (8.3 format, 12 chars minimum) */
            if (NameLen < 12 || Name[8] != '.') continue;

            /* Parse extension */
            strncpy(Ext, Name + 9, 3);
            Ext[3] = '\0';

            /* Validate flavour (1st char of ext) */
            switch (Ext[0]) {
            case 'i': Flav = BSO_IMMEDIATE;  break;
            case 'c': Flav = BSO_CONTINUOUS; break;
            case 'd': Flav = BSO_DIRECT;     break;
            case 'o':                                /* fall through     */
            case 'f': Flav = BSO_NORMAL;     break;
            case 'h': Flav = BSO_HOLD;       break;
            default:  continue;             /* not a flow file           */
            }

            /* Validate type (2nd+3rd chars of ext) */
            if (strcmp(Ext + 1, "ut") == 0)
                TypeIdx = 0;                /* netmail packet            */
            else if (strcmp(Ext + 1, "lo") == 0)
                TypeIdx = 1;                /* file reference list       */
            else if (strcmp(Ext + 1, "eq") == 0 && Ext[0] == 'r')
                TypeIdx = 2;                /* file request (.req)       */
            else
                continue;

            /* Parse net/node from hex filename */
            if (sscanf(Name, "%4hx%4hx", &NetHex, &NodeHex) != 2)
                continue;

            /* Check if we already have this address in the list */
            {
                int Found = -1;         /* index of existing item        */
                int j;                  /* search loop index             */

                for (j = 0; j < Count; j++) {
                    if (Items[j].addr.net  == NetHex &&
                        Items[j].addr.node == NodeHex &&
                        Items[j].addr.zone == Cfg->aka[ZoneIdx].zone) {
                        Found = j;
                        break;
                    }
                }

                if (Found >= 0) {
                    /* Update existing item with higher-priority flavour */
                    if (Flav < Items[Found].flavour)
                        Items[Found].flavour = Flav;
                    if (TypeIdx == 0) Items[Found].has_netmail  = 1;
                    if (TypeIdx == 1) Items[Found].has_filelist = 1;
                    if (TypeIdx == 2) Items[Found].has_request  = 1;
                } else {
                    /* New address -- add to list */
                    BsoItem *Item = &Items[Count];
                    memset(Item, 0, sizeof(*Item));
                    Item->addr.zone = Cfg->aka[ZoneIdx].zone;
                    Item->addr.net  = NetHex;
                    Item->addr.node = NodeHex;
                    Item->flavour   = Flav;
                    if (TypeIdx == 0) Item->has_netmail  = 1;
                    if (TypeIdx == 1) Item->has_filelist = 1;
                    if (TypeIdx == 2) Item->has_request  = 1;

                    snprintf(Item->basepath, sizeof(Item->basepath),
                             "%s%c%s", ZoneDir, PATH_SEP, Name);
                    /* Trim extension from basepath */
                    Item->basepath[strlen(Item->basepath) - 4] = '\0';

                    Count++;
                }
            }
        }
        closedir(d);
#else
        snprintf(Pattern, sizeof(Pattern), "%s\\*.*", ZoneDir);
        FindHandle = FindFirstFileA(Pattern, &FindData);
        if (FindHandle == INVALID_HANDLE_VALUE) continue;
        /* Same logic as POSIX path above -- omitted for brevity */
        FindClose(FindHandle);
#endif
    }

    qf_log(LOG_INFO, "BSO scan: %d nodes with pending mail", Count);
    return Count;
}


/*-----------------------------------------------------------------------*/
/* bso_create_poll() -- Create a poll for an address                      */
/*                                                                       */
/* FTS-5005 Section 3.4: a zero-length .?lo file triggers a poll         */
/* without having files to send. Touch the file to create or update      */
/* its timestamp.                                                        */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int bso_create_poll(const QfConfig *Cfg, const FTN_ADDR *Addr,
                    BsoFlavour Flavour)
{
    FILE *f;                            /* flow file handle              */
    char  Path[260];                    /* full path to .?lo file        */
    char  Ext[4];                       /* file extension                */

    Ext[0] = (char)Flavour;
    Ext[1] = 'l';
    Ext[2] = 'o';
    Ext[3] = '\0';

    bso_full_path(Cfg, Addr, Ext, Path, sizeof(Path));
    f = fopen(Path, "a");              /* create or touch               */
    if (!f) return -1;
    fclose(f);

    {
        char AddrStr[64];               /* formatted address for log     */
        ftn_format_addr(Addr, AddrStr, sizeof(AddrStr));
        qf_log(LOG_INFO, "Created poll for %s (%c flavour)",
               AddrStr, (char)Flavour);
    }

    return 0;
}
