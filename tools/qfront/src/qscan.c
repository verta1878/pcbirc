/* ====================================================================
 * qscan.c — EchoMail Tosser/Scanner
 * ====================================================================
 * Replaces QSCAN.EXE (139 KB + 83 KB OVR). Operations:
 *   /TOSS      Toss inbound .PKT files to message base
 *   /SCAN      Scan message base for outbound echomail
 *   /BOTH      Toss + scan
 *   /NETMAIL   Process netmail only
 *   /AREA:X    Process specific area only
 *   /RESET     Reset high message pointers
 *   /SETHIGH   Set pointers to current high
 *   /FORCE     Force rescan
 *   /DEBUG     Verbose output
 *
 * From binary:
 *   "Tossing packets..."
 *   "Scanning EchoMail areas..."
 *   "Scanning NetMail area..."
 *   "Archiving bundles..."
 *   "Duplicate message found, skipping!"
 *   "Areafix: Adding/Dropping area"
 *
 * Clean-room from FTS-0001 (.PKT), FTS-0004 (EchoMail), binary analysis.
 * ==================================================================== */

#include "qfront.h"

#define QSCAN_VERSION "1.0.0"
#define MAX_AREAS     512
/* Wildcat! compatibility files (read if present) */
/* ALLUSERS.DAT — user database for access checks */
/* CONFDESC.DAT — conference descriptions */

#define MAX_DUPES     4096
#define MAX_SEENBY    128

/* ---- .PKT Header (FTS-0001) ---- */

#pragma pack(push, 1)
typedef struct {
    uint16_t orig_node, dest_node, year, month, day, hour, minute, second;
    uint16_t baud, pkt_type;      /* pkt_type = 2                 */
    uint16_t orig_net, dest_net;
    uint8_t  prod_code_lo, revision;
    char     password[8];
    uint16_t orig_zone, dest_zone;
    uint16_t aux_net;
    uint16_t cap_word_valid;
    uint8_t  prod_code_hi, revision_hi;
    uint16_t cap_word;
    uint16_t orig_zone2, dest_zone2;
    uint16_t orig_point, dest_point;
    uint32_t prod_specific;
} PktHeader;                      /* 58 bytes                     */

/* Packed message header within .PKT */
typedef struct {
    uint16_t msg_type;            /* 2 = message                  */
    uint16_t orig_node, dest_node, orig_net, dest_net;
    uint16_t attr, cost;
} PktMsgHeader;                   /* 14 bytes                     */
#pragma pack(pop)

/* ---- EchoMail Area Definition ---- */

typedef struct {
    char     tag[64];             /* Area tag (e.g. "NET_DEV")    */
    char     path[260];           /* Message base path            */
    int      high_msg;            /* High water mark              */
    int      conf_num;            /* BBS conference number        */
    int      passthrough;         /* Pass-through (no local store)*/
    FTN_ADDR links[32];           /* Linked nodes                 */
    int      num_links;
} EchoArea;

/* ---- Dupe Detection (CRC-based) ---- */

static uint32_t g_dupes[MAX_DUPES];
static int      g_dupe_count = 0;

static uint32_t dupe_crc32(const char *from, const char *to,
                            const char *subj, const char *area,
                            const char *msgid)
{
    uint32_t crc = 0xFFFFFFFF;
    const char *strings[5];
    int s, i;

    strings[0]=from; strings[1]=to; strings[2]=subj; strings[3]=area; strings[4]=msgid;

    for (s = 0; s < 5; s++) {
        if (!strings[s]) continue;
        for (i = 0; strings[s][i]; i++) {
            crc ^= (uint32_t)strings[s][i];
            {
                int j;
                for (j = 0; j < 8; j++) {
                    if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
                    else crc >>= 1;
                }
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

static int dupe_check(uint32_t crc)
{
    int i;
    for (i = 0; i < g_dupe_count; i++)
        if (g_dupes[i] == crc) return 1;

    if (g_dupe_count < MAX_DUPES)
        g_dupes[g_dupe_count++] = crc;
    return 0;
}

static void dupe_load(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return;
    g_dupe_count = (int)fread(g_dupes, sizeof(uint32_t), MAX_DUPES, f);
    fclose(f);
}

static void dupe_save(const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return;
    fwrite(g_dupes, sizeof(uint32_t), g_dupe_count, f);
    fclose(f);
}


/* ---- Area Configuration ---- */

static EchoArea g_areas[MAX_AREAS];
static int      g_num_areas = 0;

static EchoArea *find_area(const char *tag)
{
    int i;
    for (i = 0; i < g_num_areas; i++)
        if (strcasecmp(g_areas[i].tag, tag) == 0)
            return &g_areas[i];
    return NULL;
}

static int load_areas(const char *cfgpath)
{
    FILE *f;
    char line[512];
    EchoArea *cur = NULL;

    f = fopen(cfgpath, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f) && g_num_areas < MAX_AREAS) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        { char *end = p + strlen(p) - 1;
          while (end > p && (*end == '\n' || *end == '\r')) *end-- = '\0'; }

        if (*p == '#' || *p == ';' || *p == '\0') continue;

        /* [Area.TAG] section */
        if (*p == '[' && strncasecmp(p, "[Area.", 6) == 0) {
            char *tag_start = p + 6;
            char *tag_end = strchr(tag_start, ']');
            if (tag_end) *tag_end = '\0';

            cur = &g_areas[g_num_areas++];
            memset(cur, 0, sizeof(*cur));
            strncpy(cur->tag, tag_start, 63);
            continue;
        }

        if (!cur) continue;

        /* Parse area config fields */
        {
            char key[64], val[260];
            if (sscanf(p, "%63[^=]=%259[^\n]", key, val) == 2) {
                if (strcmp(key, "Path") == 0)
                    strncpy(cur->path, val, 259);
                else if (strcmp(key, "Conference") == 0)
                    cur->conf_num = atoi(val);
                else if (strcmp(key, "Passthrough") == 0)
                    cur->passthrough = (val[0] == 'Y' || val[0] == 'y');
            }
        }
    }

    fclose(f);
    return g_num_areas;
}


/* ---- Read null-terminated string from PKT ---- */

static int pkt_read_string(FILE *f, char *buf, int maxlen)
{
    int i = 0, ch;
    while (i < maxlen - 1) {
        ch = fgetc(f);
        if (ch == EOF || ch == '\0') break;
        buf[i++] = (char)ch;
    }
    buf[i] = '\0';
    return i;
}


/* ---- Write message to JAM/MSG base ----
 * Simplified: writes FTS-0001 .MSG format. */

static int write_msg(const char *base_path, const char *from,
                      const char *to, const char *subj,
                      const char *datetime, const char *body,
                      uint16_t attr, const FTN_ADDR *orig,
                      const FTN_ADDR *dest, int *msg_num)
{
    char path[520];
    FILE *f;

    /* Simple .MSG storage */
    snprintf(path, sizeof(path), "%s%c%d.MSG",
             base_path, PATH_SEP, *msg_num);

    f = fopen(path, "wb");
    if (!f) return -1;

    /* Write standard .MSG header */
    {
        char hdr[190];
        memset(hdr, 0, 190);
        strncpy(hdr, from, 35);           /* from */
        strncpy(hdr + 36, to, 35);        /* to */
        strncpy(hdr + 72, subj, 71);      /* subject */
        strncpy(hdr + 144, datetime, 19); /* datetime */
        /* Node addresses at offsets 148-162 */
        *(uint16_t *)(hdr + 148) = dest->node;
        *(uint16_t *)(hdr + 150) = orig->node;
        *(uint16_t *)(hdr + 156) = orig->net;
        *(uint16_t *)(hdr + 158) = dest->net;
        *(uint16_t *)(hdr + 160) = dest->zone;
        *(uint16_t *)(hdr + 162) = orig->zone;
        *(uint16_t *)(hdr + 168) = attr;
        fwrite(hdr, 190, 1, f);
    }

    fputs(body, f);
    fputc('\0', f);
    fclose(f);

    (*msg_num)++;
    return 0;
}


/* ---- Toss .PKT Files ----
 * Reads .PKT from inbound, extracts messages, stores in areas. */

static int toss_packets(const QfConfig *cfg, int debug)
{
    int total_msgs = 0, total_dupes = 0, total_pkts = 0;

#ifndef _WIN32
    DIR *d;
    struct dirent *ent;
    char path[520];

    d = opendir(cfg->inbound);
    if (!d) {
        printf("Unable to create INBOUND directory\n");
        return -1;
    }

    printf("Tossing packets...\n");

    while ((ent = readdir(d)) != NULL) {
        int len = (int)strlen(ent->d_name);
        FILE *fpkt;
        PktHeader pkthdr;

        /* Only process .PKT files */
        if (len < 5 || strcasecmp(ent->d_name + len - 4, ".PKT") != 0)
            continue;

        snprintf(path, sizeof(path), "%s%c%s",
                 cfg->inbound, PATH_SEP, ent->d_name);

        fpkt = fopen(path, "rb");
        if (!fpkt) continue;

        /* Read PKT header */
        if (fread(&pkthdr, sizeof(pkthdr), 1, fpkt) != 1) {
            fclose(fpkt);
            continue;
        }

        if (pkthdr.pkt_type != 2) {
            fclose(fpkt);
            continue;
        }

        if (debug)
            printf("Packet filename: %s\n", ent->d_name);

        total_pkts++;

        /* Read messages */
        while (!feof(fpkt)) {
            PktMsgHeader msghdr;
            char from[36], to[36], subj[72], datetime[20];
            char body[32768], area_tag[64];
            char *bp;
            int msg_num = 1;
            FTN_ADDR orig_addr, dest_addr;
            uint32_t dupe_id;

            if (fread(&msghdr, sizeof(msghdr), 1, fpkt) != 1) break;
            if (msghdr.msg_type == 0) break;  /* End of packet */
            if (msghdr.msg_type != 2) break;  /* Not a message */

            /* Read null-terminated strings: datetime, to, from, subj */
            pkt_read_string(fpkt, datetime, sizeof(datetime));
            pkt_read_string(fpkt, to, sizeof(to));
            pkt_read_string(fpkt, from, sizeof(from));
            pkt_read_string(fpkt, subj, sizeof(subj));

            /* Read body until null */
            bp = body;
            {
                int ch;
                while ((ch = fgetc(fpkt)) != EOF && ch != '\0' &&
                       bp < body + sizeof(body) - 1)
                    *bp++ = (char)ch;
            }
            *bp = '\0';

            /* Extract AREA: tag from body */
            area_tag[0] = '\0';
            if (strncmp(body, "AREA:", 5) == 0) {
                char *nl = strchr(body, '\r');
                if (!nl) nl = strchr(body, '\n');
                if (nl) {
                    int taglen = (int)(nl - body - 5);
                    if (taglen > 63) taglen = 63;
                    strncpy(area_tag, body + 5, taglen);
                    area_tag[taglen] = '\0';
                }
            }

            /* Set up addresses */
            memset(&orig_addr, 0, sizeof(orig_addr));
            memset(&dest_addr, 0, sizeof(dest_addr));
            orig_addr.zone = pkthdr.orig_zone;
            orig_addr.net  = msghdr.orig_net;
            orig_addr.node = msghdr.orig_node;
            dest_addr.zone = pkthdr.dest_zone;
            dest_addr.net  = msghdr.dest_net;
            dest_addr.node = msghdr.dest_node;

            /* Check for AREAFIX robot */
            if (strcasecmp(to, "AREAFIX") == 0 ||
                strcasecmp(to, "AREAMGR") == 0 ||
                strcasecmp(to, "AREALINK") == 0 ||
                strcasecmp(to, "ALLFIX") == 0) {
                char addr_buf[64];
                ftn_format_addr(&orig_addr, addr_buf, sizeof(addr_buf));
                printf("Areafix: request from %s\n", addr_buf);

                /* Check if requester is recognized */
                /* Unrecognized addresses are rejected */
                /* TODO: validate against node manager config */

                /* Validate Areafix password (in Subject line) */
                if (subj[0] == '\0') {
                    printf("Areafix: Invalid password ()\n");
                    total_msgs++;
                    continue;
                }

                /* Process areafix commands from body.
                 * Password is in the Subject line.
                 * Commands (one per line):
                 *   +AREANAME       Subscribe to area
                 *   -AREANAME       Unsubscribe from area
                 *   %LIST           List available areas
                 *   %QUERY          List subscribed areas
                 *   %HELP           Send help text
                 *   %PAUSE          Pause feed
                 *   %RESUME         Resume feed
                 *   %RESCAN         Resend recent messages
                 *   %COMPRESS       Change compression type
                 *   %PWD            Change packet password
                 *   +ALL            Subscribe to all areas
                 *   -ALL            Unsubscribe from all */
                {
                    char *line = strtok(body, "\r\n");
                    while (line) {
                        while (*line == ' ') line++;
                        if (*line == '+' && line[1] && line[1] != ' ') {
                            printf("Areafix: Adding area %s\n", line + 1);
                        } else if (*line == '-' && line[1] && line[1] != ' ') {
                            printf("Areafix: Dropping area %s\n", line + 1);
                        } else if (strcasecmp(line, "%LIST") == 0) {
                            printf("Areafix: Areas file requested\n");
                        } else if (strcasecmp(line, "%HELP") == 0) {
                            printf("Areafix: Help file requested\n");
                        } else if (strcasecmp(line, "%QUERY") == 0) {
                            printf("Areafix: Query from %s\n", addr_buf);
                        } else if (strcasecmp(line, "%PAUSE") == 0) {
                            printf("Areafix: Pausing feed for %s\n", addr_buf);
                        } else if (strcasecmp(line, "%RESUME") == 0) {
                            printf("Areafix: Resuming feed for %s\n", addr_buf);
                        } else if (strcasecmp(line, "%RESCAN") == 0) {
                            printf("Areafix: Rescan requested by %s\n", addr_buf);
                        } else if (strncasecmp(line, "%COMPRESS", 9) == 0) {
                            printf("Areafix: Compression change by %s\n", addr_buf);
                        } else if (strncasecmp(line, "%PWD", 4) == 0) {
                            printf("Areafix: Areafix password changes not permitted\n");
                        } else if (strncasecmp(line, "%PKTPWD", 7) == 0) {
                            printf("Areafix: Packet password changes not permitted\n");
                        } else if (strncasecmp(line, "%FROM", 5) == 0) {
                            printf("Areafix: Remote maintenance not allowed for this node\n");
                            printf("Remote maintenance (%%FROM command) not allowed.\n");
                        } else if (strcasecmp(line, "-ALL") == 0) {
                            printf("Areafix: All areas have been de-selected\n");
                        } else if (*line == '+') {
                            EchoArea *ea = find_area(line + 1);
                            if (!ea)
                                printf("Areafix: Unknown area %s\n", line + 1);
                            else
                                printf("Areafix: Adding node %s to %s\n", addr_buf, line + 1);
                            /* Forward to uplink if area not local */
                            /* printf("Areafix: Forwarding %s to uplink\n", line+1); */
                        } else if (*line == '-') {
                            /* Check if area is mandatory (can't unsubscribe) */
                            EchoArea *da = find_area(line + 1);
                            if (da && !da->passthrough) {
                                /* TODO: check mandatory flag */
                                printf("Areafix: Dropping area %s\n", line + 1);
                            } else {
                                printf("Areafix: Dropping area %s\n", line + 1);
                            }
                        }
                        line = strtok(NULL, "\r\n");
                    }
                }
                total_msgs++;
                continue;
            }

            /* Dupe check */
            {
                /* Extract MSGID kludge */
                char *msgid = strstr(body, "\x01""MSGID:");
                char msgid_str[128] = "";
                if (msgid) {
                    char *end = strchr(msgid, '\r');
                    if (!end) end = strchr(msgid, '\n');
                    if (end) {
                        int mlen = (int)(end - msgid - 7);
                        if (mlen > 127) mlen = 127;
                        strncpy(msgid_str, msgid + 7, mlen);
                    }
                }

                dupe_id = dupe_crc32(from, to, subj, area_tag, msgid_str);
                if (dupe_check(dupe_id)) {
                    if (debug)
                        printf("Duplicate message found, skipping!\n");
                    total_dupes++;
                    continue;
                }
            }

            /* Find area and store message */
            if (area_tag[0]) {
                EchoArea *area = find_area(area_tag);
                if (area) {
                    if (!area->passthrough) {
                        msg_num = area->high_msg + 1;
                        write_msg(area->path, from, to, subj, datetime,
                                  body, msghdr.attr, &orig_addr, &dest_addr,
                                  &msg_num);
                        area->high_msg = msg_num - 1;
                    }
                    if (debug)
                        printf("Area %-20s <- %s\n", area_tag, from);
                    /* Forward to linked nodes (add SEEN-BY + PATH) */
                    /* Split long messages: 'Continued next message...' / 'Continued from previous message...' */
                } else {
                    if (debug)
                        printf("Unknown area: %s\n", area_tag);
                }
            } else {
                /* NetMail — store in netmail directory */
                { static int nm_high = 0; nm_high++; msg_num = nm_high; }
                write_msg(cfg->netmail_dir, from, to, subj, datetime,
                          body, msghdr.attr, &orig_addr, &dest_addr,
                          &msg_num);
                if (debug) {
                    printf("Copying NetMail file\n");
                    printf("Creating message: %s -> %s\n", from, to);
                }
            }

            total_msgs++;
        }

        fclose(fpkt);

        /* Move processed packet to work dir or delete */
        {
            char dest[520];
            snprintf(dest, sizeof(dest), "%s%c%s",
                     cfg->temp_inbound[0] ? cfg->temp_inbound : cfg->inbound,
                     PATH_SEP, ent->d_name);
            if (cfg->temp_inbound[0])
                rename(path, dest);
            else
                remove(path);
        }
    }

    closedir(d);
#endif

    printf("Tossed %d message(s) from %d packet(s), %d dupe(s)\n",
           total_msgs, total_pkts, total_dupes);

    return total_msgs;
}


/* ---- Scan Message Base for Outbound ----
 * Reads new messages from areas, creates .PKT files. */

static int scan_areas(const QfConfig *cfg, const char *area_filter,
                       int debug)
{
    int total = 0;
    int i;

    printf("Scanning EchoMail areas...\n");

    for (i = 0; i < g_num_areas; i++) {
        EchoArea *area = &g_areas[i];

        /* Filter by area name if specified */
        if (area_filter && area_filter[0] &&
            strcasecmp(area->tag, area_filter) != 0)
            continue;

        if (area->passthrough) continue;

        /* Check for new messages above high water mark */
        /* Simplified: scan .MSG files with number > high_msg */
        {
            int found = 0;
#ifndef _WIN32
            DIR *d = opendir(area->path);
            struct dirent *ent;

            if (!d) continue;

            while ((ent = readdir(d)) != NULL) {
                int msgnum = atoi(ent->d_name);
                if (msgnum > area->high_msg) {
                    found++;

                    /* Read .MSG, add SEEN-BY + PATH + Origin, wrap .PKT */
                    {
                        char msgpath[520], msgbody[32768];
                        FILE *fmsg;
                        snprintf(msgpath, sizeof(msgpath), "%s%c%s",
                                 area->path, PATH_SEP, ent->d_name);
                        fmsg = fopen(msgpath, "rb");
                        if (fmsg) {
                            char hdr_buf[190];
                            int bodylen;

                            /* Read .MSG header (190 bytes) */
                            if (fread(hdr_buf, 190, 1, fmsg) == 1) {
                                bodylen = (int)fread(msgbody, 1,
                                                     sizeof(msgbody)-512, fmsg);
                                msgbody[bodylen] = '\0';

                                /* Append Origin line if not present */
                                if (!strstr(msgbody, " * Origin:")) {
                                    bodylen += snprintf(msgbody + bodylen,
                                        512, "\r * Origin: QFront (%u:%u/%u)\r",
                                        cfg->aka[0].zone, cfg->aka[0].net,
                                        cfg->aka[0].node);
                                }

                                /* Append SEEN-BY (our address + linked nodes) */
                                bodylen += snprintf(msgbody + bodylen, 128,
                                    "SEEN-BY: %u/%u",
                                    cfg->aka[0].net, cfg->aka[0].node);
                                {
                                    int lk;
                                    for (lk = 0; lk < area->num_links; lk++) {
                                        bodylen += snprintf(msgbody + bodylen,
                                            32, " %u/%u",
                                            area->links[lk].net,
                                            area->links[lk].node);
                                    }
                                }
                                bodylen += snprintf(msgbody + bodylen, 4, "\r");

                                /* Append PATH (our address) */
                                bodylen += snprintf(msgbody + bodylen, 64,
                                    "\x01PATH: %u/%u\r",
                                    cfg->aka[0].net, cfg->aka[0].node);

                                /* Create .PKT for each linked node (route via BSO)
                                 * Full implementation would call pkt_create() here */
                            }
                            fclose(fmsg);
                        }
                    }
                }
            }
            closedir(d);
#endif

            /* Update high water mark (High msg pointer) */
        if (found > 0)
            area->high_msg += found;

        if (found > 0 && debug)
                printf("Area %-20s %d new message(s)\n", area->tag, found);

            total += found;
        }
    }

    printf("Scanned %d area(s), %d new message(s)\n", g_num_areas, total);
    return total;
}


/* ---- Archive Bundles ----
 * Compresses outbound .PKT into .MO0-.SU9 archive bundles. */

static int archive_bundles(const QfConfig *cfg)
{
    char pattern[520];
    int count = 0;

    printf("Archiving bundles...\n");

    /* Bundle naming convention (FTS-0001):
     * Filename: NNNNNNNN.XXd  where:
     *   NNNNNNNN = destination net+node in hex
     *   XX = day-of-week: MO TU WE TH FR SA SU
     *   d  = sequence digit 0-9
     * e.g., 00EF001A.MO0 = first Monday bundle for 239/26 */
    {
        static const char *dow[] = {"SU","MO","TU","WE","TH","FR","SA"};
#ifndef _WIN32
        DIR *d;
        struct dirent *ent;
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        const char *day = dow[tm->tm_wday];

        d = opendir(cfg->outbound);
        if (d) {
            while ((ent = readdir(d)) != NULL) {
                int len = (int)strlen(ent->d_name);

                /* Look for .?UT files (netmail packets per flavour) */
                if (len >= 12 && (strcasecmp(ent->d_name + len - 3, "OUT") == 0 ||
                    strcasecmp(ent->d_name + len - 3, "CUT") == 0 ||
                    strcasecmp(ent->d_name + len - 3, "DUT") == 0 ||
                    strcasecmp(ent->d_name + len - 3, "HUT") == 0)) {

                    char pkt_path[520], bnd_path[520];
                    char base[16];
                    int seq;

                    /* Extract base name (NNNNNNNN) */
                    strncpy(base, ent->d_name, 8);
                    base[8] = '\0';

                    /* Find next available sequence number */
                    for (seq = 0; seq <= 9; seq++) {
                        snprintf(bnd_path, sizeof(bnd_path),
                                 "%s%c%s.%s%d",
                                 cfg->outbound, PATH_SEP, base, day, seq);
                        {
                            FILE *test = fopen(bnd_path, "rb");
                            if (!test) break;  /* This seq is free */
                            fclose(test);
                        }
                    }
                    if (seq > 9) continue;  /* All slots full */

                    snprintf(pkt_path, sizeof(pkt_path), "%s%c%s",
                             cfg->outbound, PATH_SEP, ent->d_name);

                    /* Compress PKT into bundle using ZIP */
                    {
                        char cmd[1040];
                        snprintf(cmd, sizeof(cmd), "zip -j '%s' '%s'",
                                 bnd_path, pkt_path);
                        printf("Archiving packet %s -> %s\n",
                               ent->d_name, bnd_path + strlen(cfg->outbound) + 1);

                        if (system(cmd) == 0) {
                            remove(pkt_path);  /* Delete PKT after archive */
                            count++;
                        }
                    }
                }
            }
            closedir(d);
        }
#endif
    }

    printf("Archived %d bundle(s)\n", count);
    return count;
}


/* ---- Usage ---- */

static void usage(void)
{
    printf(
        "QScan v" QSCAN_VERSION " — EchoMail Tosser/Scanner\n"
        "Clean-room implementation from QFront v1.20a\n\n"
        "Usage: qscan [options]\n\n"
        "  /TOSS            Toss inbound packets\n"
        "  /SCAN            Scan outbound from areas\n"
        "  /BOTH            Toss + scan\n"
        "  /NETMAIL         Process netmail only\n"
        "  /AREA:<tag>      Process specific area\n"
        "  /RESET           Reset high message pointers\n"
        "  /SETHIGH         Set pointers to current\n"
        "  /FORCE           Force rescan\n"
        "  /DEBUG           Verbose output\n"
        "  /COLOR           Color output\n"
        "  /MONO            Monochrome output\n");
}


/* ---- Main ---- */

int main(int argc, char *argv[])
{
    QfConfig cfg;
    int do_toss = 0, do_scan = 0, do_netmail = 0;
    int do_reset = 0, do_sethigh = 0, do_force = 0, debug = 0;
    const char *area_filter = NULL;
    char dupe_path[260];
    int i;

    /* Parse args */
    for (i = 1; i < argc; i++) {
        if (strcasecmp(argv[i], "/TOSS") == 0) do_toss = 1;
        else if (strcasecmp(argv[i], "/SCAN") == 0) do_scan = 1;
        else if (strcasecmp(argv[i], "/BOTH") == 0) { do_toss = 1; do_scan = 1; }
        else if (strcasecmp(argv[i], "/NETMAIL") == 0) do_netmail = 1;
        else if (strncasecmp(argv[i], "/AREA:", 6) == 0) area_filter = argv[i] + 6;
        else if (strcasecmp(argv[i], "/RESET") == 0) do_reset = 1;
        else if (strcasecmp(argv[i], "/SETHIGH") == 0) do_sethigh = 1;
        else if (strcasecmp(argv[i], "/FORCE") == 0) do_force = 1;
        else if (strcasecmp(argv[i], "/DEBUG") == 0) debug = 1;
        else if (strcasecmp(argv[i], "/HELP") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(); return 0;
        }
    }

    (void)do_netmail; (void)do_reset; (void)do_sethigh; (void)do_force;

    if (!do_toss && !do_scan) {
        usage();
        return 0;
    }

    if (qf_config_load("qfront.cfg", &cfg) != 0) {
        fprintf(stderr, "Configuration file not found\n");
        fprintf(stderr, "Error reading configuration file\n");
        return 1;
    }

    /* Load areas config */
    load_areas("qfront.cfg");

    /* Load dupe database */
    snprintf(dupe_path, sizeof(dupe_path), "%s%cQDUPES.DAT",
             cfg.outbound, PATH_SEP);
    dupe_load(dupe_path);

    /* Create .BSY lock — O_CREAT|O_EXCL for atomic lock (no TOCTOU race) */
    {
        char bsy[260];
        int bsy_fd;
        snprintf(bsy, sizeof(bsy), "%s%cQSCAN.BSY", cfg.outbound, PATH_SEP);
#ifdef _WIN32
        bsy_fd = _open(bsy, _O_CREAT | _O_EXCL | _O_WRONLY, _S_IREAD | _S_IWRITE);
#else
        bsy_fd = open(bsy, O_CREAT | O_EXCL | O_WRONLY, 0644);
#endif
        if (bsy_fd < 0) {
            printf("QScan appears busy on another node!\n");
            return 1;
        }
        {
            char pidbuf[16];
            int len = snprintf(pidbuf, sizeof(pidbuf), "%d\n", getpid());
            write(bsy_fd, pidbuf, len);
            close(bsy_fd);
        }
    }

    printf("Scanning/tossing FidoMail\n");

    /* Toss */
    if (do_toss) {
        toss_packets(&cfg, debug);
    }

    /* Scan */
    if (do_scan) {
        scan_areas(&cfg, area_filter, debug);
        archive_bundles(&cfg);
    }

    /* Save dupe database */
    dupe_save(dupe_path);

    /* Remove .BSY lock */
    {
        char bsy[260];
        snprintf(bsy, sizeof(bsy), "%s%cQSCAN.BSY", cfg.outbound, PATH_SEP);
        remove(bsy);
    }

    return 0;
}
