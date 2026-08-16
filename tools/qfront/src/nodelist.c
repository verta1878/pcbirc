/* ====================================================================
 * nodelist.c — FidoNet Nodelist Parser (FTS-5001)
 * ====================================================================
 * Parses raw NODELIST.* text files for address→info lookup.
 * Handles Zone/Region/Host/Hub/Pvt/Down/Hold entry types.
 * Extracts phone number, flags, speed, sysop name, system name.
 *
 * Clean-room from FTS-5001 (public FidoNet specification).
 * ==================================================================== */

#include "qfront.h"
#include <ctype.h>

/* Max nodes we can index in memory */
#define NL_MAX_NODES    65536

/* ---- Nodelist Entry ---- */
typedef struct {
    FTN_ADDR addr;                /* Zone:Net/Node address        */
    char     system[64];          /* System/BBS name              */
    char     location[40];        /* City, State                  */
    char     sysop[36];           /* Sysop name                   */
    char     phone[40];           /* Phone number or IP address   */
    uint32_t speed;               /* Max baud rate                */
    char     flags[128];          /* Capability flags string      */
    int      is_down;             /* Node is DOWN                 */
    int      is_hold;             /* Node is HOLD                 */
    int      is_pvt;              /* Node is PVT (unlisted phone) */
    int      is_cm;               /* Has CM flag (Continuous Mail) */
    int      is_mo;               /* Has MO flag (Modem Only)     */
    int      has_ibn;             /* Has IBN flag (BinkP capable) */
    char     ibn_host[128];       /* IBN hostname:port if present */
} NlEntry;

/* ---- Nodelist Database ---- */
typedef struct {
    NlEntry *entries;             /* Array of parsed entries      */
    int      count;               /* Number of entries            */
    int      capacity;            /* Allocated capacity           */
} NlDatabase;


/* ---- Parse Flags String ----
 * Extracts capability flags from the FTS-5001 flags field.
 * Flags are comma-separated tokens after the 5th field.
 *
 * Key flags from binary analysis + FTS-5001:
 *   CM  — Continuous Mail (available 24 hours)
 *   MO  — Modem Only (no IP sessions)
 *   LO  — Listed Only
 *   IBN — BinkP capable (IBN[:port])
 *   INA — IP address (INA:hostname)
 *   XX  — Node does not accept calls
 *   XA  — Supports all protocols
 *   XW  — Supports FTS-0001
 */
static void nl_parse_flags(NlEntry *ent)
{
    const char *p = ent->flags;
    char token[64];
    int ti;

    ent->is_cm  = 0;
    ent->is_mo  = 0;
    ent->has_ibn = 0;
    ent->ibn_host[0] = '\0';

    while (*p) {
        /* Skip commas and whitespace */
        while (*p == ',' || *p == ' ') p++;
        if (!*p) break;

        /* Extract token */
        ti = 0;
        while (*p && *p != ',' && ti < 63)
            token[ti++] = *p++;
        token[ti] = '\0';

        /* Match known flags */
        if (strcmp(token, "CM") == 0)
            ent->is_cm = 1;
        else if (strcmp(token, "MO") == 0)
            ent->is_mo = 1;
        else if (strncmp(token, "IBN", 3) == 0) {
            ent->has_ibn = 1;
            /* IBN:host:port or just IBN (use default 24554) */
            if (token[3] == ':')
                strncpy(ent->ibn_host, token + 4, sizeof(ent->ibn_host) - 1);
        }
        else if (strncmp(token, "INA:", 4) == 0) {
            /* INA:hostname — use as BinkP host if no IBN host */
            if (!ent->ibn_host[0])
                strncpy(ent->ibn_host, token + 4, sizeof(ent->ibn_host) - 1);
        }
    }
}


/* ---- Parse One Nodelist Line ----
 *
 * FTS-5001 format:
 *   keyword,number,name,location,sysop,phone,speed,flags
 *
 * Keywords: Zone, Region, Host, Hub, Pvt, Hold, Down, Boss, or empty.
 * Empty keyword = normal node under current net.
 *
 * Example:
 *   Zone,1,North_America,Dallas_TX,Rob_Kittredge,-Unpublished-,9600,CM,XA,IBN
 *   Host,234,SomeNet,Chicago_IL,John_Doe,1-312-555-1234,9600,CM
 *   ,56,MyBBS,Chicago_IL,Jane_Smith,1-312-555-5678,9600,CM,IBN:mybbs.com
 */
static int nl_parse_line(const char *line, NlEntry *ent,
                         uint16_t *cur_zone, uint16_t *cur_net)
{
    char keyword[16] = "";
    char numbuf[16], namebuf[64], locbuf[40], sysbuf[36];
    char phonebuf[40], speedbuf[16], flagsbuf[256];
    const char *p = line;
    char *dst;
    int field;

    memset(ent, 0, sizeof(*ent));

    /* Skip comment lines (starting with ;) */
    while (*p == ' ' || *p == '\t') p++;
    if (*p == ';' || *p == '\0' || *p == '\n' || *p == '\r')
        return -1;

    /* Parse comma-separated fields.
     * Field 0: keyword (may be empty)
     * Field 1: number
     * Field 2: system name
     * Field 3: location
     * Field 4: sysop name
     * Field 5: phone
     * Field 6: speed
     * Field 7+: flags (rest of line) */

    field = 0;
    while (*p && field <= 7) {
        int len = 0;
        int maxlen;

        switch (field) {
        case 0: dst = keyword;  maxlen = 15; break;
        case 1: dst = numbuf;   maxlen = 15; break;
        case 2: dst = namebuf;  maxlen = 63; break;
        case 3: dst = locbuf;   maxlen = 39; break;
        case 4: dst = sysbuf;   maxlen = 35; break;
        case 5: dst = phonebuf; maxlen = 39; break;
        case 6: dst = speedbuf; maxlen = 15; break;
        case 7: dst = flagsbuf; maxlen = 255; break;
        default: dst = NULL; maxlen = 0; break;
        }

        if (field < 7) {
            /* Read until comma or end of line */
            while (*p && *p != ',' && *p != '\n' && *p != '\r' && len < maxlen)
                dst[len++] = *p++;
            dst[len] = '\0';
            if (*p == ',') p++;  /* Skip comma */
        } else {
            /* Field 7+ = rest of line (flags, comma-separated) */
            while (*p && *p != '\n' && *p != '\r' && len < maxlen)
                dst[len++] = *p++;
            dst[len] = '\0';
        }

        field++;
    }

    if (field < 7)
        return -1;                /* Not enough fields            */

    /* Convert underscores to spaces in name/location/sysop */
    for (dst = namebuf; *dst; dst++) if (*dst == '_') *dst = ' ';
    for (dst = locbuf;  *dst; dst++) if (*dst == '_') *dst = ' ';
    for (dst = sysbuf;  *dst; dst++) if (*dst == '_') *dst = ' ';

    /* Process keyword to determine address context */
    if (strcmp(keyword, "Zone") == 0) {
        *cur_zone = (uint16_t)atoi(numbuf);
        *cur_net = (uint16_t)atoi(numbuf);  /* Zone = net initially */
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_zone;
        ent->addr.node = 0;
    }
    else if (strcmp(keyword, "Region") == 0) {
        *cur_net = (uint16_t)atoi(numbuf);
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_net;
        ent->addr.node = 0;
    }
    else if (strcmp(keyword, "Host") == 0) {
        *cur_net = (uint16_t)atoi(numbuf);
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_net;
        ent->addr.node = 0;
    }
    else if (strcmp(keyword, "Hub") == 0 || strcmp(keyword, "Boss") == 0 ||
             keyword[0] == '\0') {
        /* Hub, Boss, or normal node — under current net */
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_net;
        ent->addr.node = (uint16_t)atoi(numbuf);
    }
    else if (strcmp(keyword, "Pvt") == 0) {
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_net;
        ent->addr.node = (uint16_t)atoi(numbuf);
        ent->is_pvt = 1;
    }
    else if (strcmp(keyword, "Down") == 0) {
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_net;
        ent->addr.node = (uint16_t)atoi(numbuf);
        ent->is_down = 1;
    }
    else if (strcmp(keyword, "Hold") == 0) {
        ent->addr.zone = *cur_zone;
        ent->addr.net  = *cur_net;
        ent->addr.node = (uint16_t)atoi(numbuf);
        ent->is_hold = 1;
    }
    else {
        return -1;                /* Unknown keyword              */
    }

    /* Store parsed fields */
    strncpy(ent->system,   namebuf,  sizeof(ent->system) - 1);
    strncpy(ent->location, locbuf,   sizeof(ent->location) - 1);
    strncpy(ent->sysop,    sysbuf,   sizeof(ent->sysop) - 1);
    strncpy(ent->phone,    phonebuf, sizeof(ent->phone) - 1);
    strncpy(ent->flags,    flagsbuf, sizeof(ent->flags) - 1);
    ent->speed = (uint32_t)atol(speedbuf);

    /* Parse flags for capabilities */
    nl_parse_flags(ent);

    return 0;
}


/* ---- Open and Parse a Nodelist ---- */

NlDatabase *nl_open(const char *path)
{
    FILE *f;
    NlDatabase *db;
    char line[512];
    uint16_t cur_zone = 1, cur_net = 1;

    f = fopen(path, "r");
    if (!f) {
        qf_log(LOG_ERROR, "Cannot open nodelist: %s", path);
        return NULL;
    }

    db = (NlDatabase *)calloc(1, sizeof(NlDatabase));
    if (!db) { fclose(f); return NULL; }

    db->capacity = 4096;
    db->entries = (NlEntry *)calloc(db->capacity, sizeof(NlEntry));
    if (!db->entries) { free(db); fclose(f); return NULL; }

    while (fgets(line, sizeof(line), f)) {
        NlEntry ent;

        if (nl_parse_line(line, &ent, &cur_zone, &cur_net) == 0) {
            /* Grow array if needed */
            if (db->count >= db->capacity) {
                int newcap = db->capacity * 2;
                NlEntry *newbuf;
                if (newcap > NL_MAX_NODES) break;
                newbuf = (NlEntry *)realloc(db->entries,
                                            newcap * sizeof(NlEntry));
                if (!newbuf) break;
                db->entries = newbuf;
                db->capacity = newcap;
            }

            db->entries[db->count++] = ent;
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Nodelist loaded: %d entries from %s", db->count, path);
    return db;
}


/* ---- Lookup a Node ---- */

/*-----------------------------------------------------------------------*/
/* nl_lookup() — Find a node in the parsed nodelist database             */
/*                                                                         */
/* Linear scan — acceptable for nodelists up to ~65K nodes.             */
/* A production implementation would use a hash table or binary search   */
/* on the pre-compiled .NDX index for O(1) / O(log n) lookup.           */
/*                                                                         */
/* Matches on zone:net/node only. Point matching is not implemented     */
/* because points are typically in separate pointlists with Boss headers.*/
/*                                                                         */
/* Returns a pointer to the NlEntry, or NULL if not found.               */
/*-----------------------------------------------------------------------*/

const NlEntry *nl_lookup(const NlDatabase *db, const FTN_ADDR *addr)
{
    int i;
    char buf[64];

    if (!db) return NULL;

    ftn_format_addr(addr, buf, sizeof(buf));
    qf_log(LOG_DEBUG, "nl_lookup: searching for %s in %d entries",
           buf, db->count);

    for (i = 0; i < db->count; i++) {
        if (db->entries[i].addr.zone == addr->zone &&
            db->entries[i].addr.net  == addr->net &&
            db->entries[i].addr.node == addr->node) {
            qf_log(LOG_DEBUG, "nl_lookup: found %s — \"%s\" by %s "
                   "(cm=%d down=%d hold=%d ibn=%d)",
                   buf, db->entries[i].system, db->entries[i].sysop,
                   db->entries[i].is_cm, db->entries[i].is_down,
                   db->entries[i].is_hold, db->entries[i].has_ibn);
            return &db->entries[i];
        }
    }

    qf_log(LOG_DEBUG, "nl_lookup: %s not found", buf);
    return NULL;
}

/* ---- Find a Node's Net Host (for routing) ---- */

const NlEntry *nl_find_host(const NlDatabase *db, const FTN_ADDR *addr)
{
    FTN_ADDR host_addr;
    host_addr.zone = addr->zone;
    host_addr.net  = addr->net;
    host_addr.node = 0;           /* Net host is always node 0    */
    host_addr.point = 0;
    return nl_lookup(db, &host_addr);
}


/* ---- Close and Free ---- */

void nl_close(NlDatabase *db)
{
    if (db) {
        free(db->entries);
        free(db);
    }
}

/* ---- Accessors for qfront.c ---- */

int nl_entry_is_cm(const NlEntry *e)    { return e ? e->is_cm : 0; }
int nl_entry_is_down(const NlEntry *e)  { return e ? e->is_down : 0; }
int nl_entry_is_hold(const NlEntry *e)  { return e ? e->is_hold : 0; }
const char *nl_entry_ibn_host(const NlEntry *e)
{
    return (e && e->ibn_host[0]) ? e->ibn_host : "";
}
