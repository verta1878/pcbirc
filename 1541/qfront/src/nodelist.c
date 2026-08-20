/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* nodelist.c -- FidoNet Nodelist Parser (FTS-5001)                         */
/*                                                                           */
/* Parses raw NODELIST.* text files for address-to-info lookup.              */
/* Handles Zone/Region/Host/Hub/Pvt/Down/Hold/Boss entry types.              */
/* Extracts phone number, flags, speed, sysop name, system name.             */
/*                                                                           */
/* Clean-room from FTS-5001 (public FidoNet specification).                  */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"
#include <ctype.h>

#define NL_MAX_NODES 65536              /* max nodes in memory index     */


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                          Nodelist Entry                                   */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct {
    FTN_ADDR Addr;                      /* zone:net/node address         */
    char     System[64];                /* system/BBS name               */
    char     Location[40];              /* city, state                   */
    char     Sysop[36];                 /* sysop name                    */
    char     Phone[40];                 /* phone number or IP address    */
    uint32_t Speed;                     /* max baud rate                 */
    char     Flags[128];                /* capability flags string       */
    int      IsDown;                    /* node is DOWN                  */
    int      IsHold;                    /* node is HOLD                  */
    int      IsPvt;                     /* node is PVT (unlisted phone)  */
    int      IsCM;                      /* has CM flag (continuous mail) */
    int      IsMO;                      /* has MO flag (modem only)      */
    int      HasIBN;                    /* has IBN flag (BinkP capable)  */
    char     IbnHost[128];              /* IBN hostname:port if present  */
} NlEntry;

typedef struct {
    NlEntry *Entries;                   /* array of parsed entries       */
    int      Count;                     /* number of entries             */
    int      Capacity;                  /* allocated capacity            */
} NlDatabase;


/*-----------------------------------------------------------------------*/
/* nl_parse_flags() -- Extract capability flags from flags string        */
/*                                                                       */
/* Flags are comma-separated tokens in FTS-5001 format.                  */
/*                                                                       */
/* Key flags:                                                            */
/*   CM  -- Continuous Mail (available 24 hours)                         */
/*   MO  -- Modem Only (no IP sessions)                                  */
/*   LO  -- Listed Only                                                  */
/*   IBN -- BinkP capable (IBN[:host:port])                              */
/*   INA -- IP address (INA:hostname)                                    */
/*   XX  -- Node does not accept calls                                   */
/*   XA  -- Supports all protocols                                       */
/*   XW  -- Supports FTS-0001                                            */
/*-----------------------------------------------------------------------*/

static void nl_parse_flags(NlEntry *Ent)
{
    const char *p = Ent->Flags;         /* flags string scan pointer     */
    char        Token[64];              /* current flag token            */
    int         Ti;                     /* token character index         */

    Ent->IsCM    = 0;
    Ent->IsMO    = 0;
    Ent->HasIBN  = 0;
    Ent->IbnHost[0] = '\0';

    while (*p) {
        /* Skip commas and whitespace */
        while (*p == ',' || *p == ' ') p++;
        if (!*p) break;

        /* Extract token */
        Ti = 0;
        while (*p && *p != ',' && Ti < 63)
            Token[Ti++] = *p++;
        Token[Ti] = '\0';

        /* Match known flags */
        if (strcmp(Token, "CM") == 0)
            Ent->IsCM = 1;
        else if (strcmp(Token, "MO") == 0)
            Ent->IsMO = 1;
        else if (strncmp(Token, "IBN", 3) == 0) {
            Ent->HasIBN = 1;
            /* IBN:host:port or just IBN (use default 24554) */
            if (Token[3] == ':')
                strncpy(Ent->IbnHost, Token + 4, sizeof(Ent->IbnHost) - 1);
        }
        else if (strncmp(Token, "INA:", 4) == 0) {
            /* INA:hostname -- use as BinkP host if no IBN host */
            if (!Ent->IbnHost[0])
                strncpy(Ent->IbnHost, Token + 4, sizeof(Ent->IbnHost) - 1);
        }
    }
}


/*-----------------------------------------------------------------------*/
/* nl_parse_line() -- Parse one nodelist line into an NlEntry            */
/*                                                                       */
/* FTS-5001 format:                                                      */
/*   keyword,number,name,location,sysop,phone,speed,flags                */
/*                                                                       */
/* Keywords: Zone, Region, Host, Hub, Pvt, Hold, Down, Boss, or empty.   */
/* Empty keyword = normal node under current net.                        */
/*                                                                       */
/* Example:                                                              */
/*   Zone,1,North_America,Dallas_TX,Sysop,-Unpublished-,9600,CM,XA,IBN  */
/*   Host,234,SomeNet,Chicago_IL,John_Doe,1-312-555-1234,9600,CM        */
/*   ,56,MyBBS,Chicago_IL,Jane_Smith,1-312-555-5678,9600,CM,IBN          */
/*                                                                       */
/* Returns 0 on success, -1 on comment/empty/parse error.                */
/*-----------------------------------------------------------------------*/

static int nl_parse_line(const char *Line, NlEntry *Ent,
                         uint16_t *CurZone, uint16_t *CurNet)
{
    char        Keyword[16] = "";       /* entry type keyword            */
    char        NumBuf[16];             /* node number buffer            */
    char        NameBuf[64];            /* system name buffer            */
    char        LocBuf[40];             /* location buffer               */
    char        SysBuf[36];             /* sysop name buffer             */
    char        PhoneBuf[40];           /* phone number buffer           */
    char        SpeedBuf[16];           /* baud rate buffer              */
    char        FlagsBuf[256];          /* flags string buffer           */
    const char *p = Line;               /* line scan pointer             */
    char       *Dst;                    /* current field dest pointer    */
    int         Field;                  /* current field index           */

    memset(Ent, 0, sizeof(*Ent));

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

    Field = 0;
    while (*p && Field <= 7) {
        int Len = 0;                    /* chars read into field         */
        int MaxLen;                     /* max field length              */

        switch (Field) {
        case 0: Dst = Keyword;  MaxLen = 15;  break;
        case 1: Dst = NumBuf;   MaxLen = 15;  break;
        case 2: Dst = NameBuf;  MaxLen = 63;  break;
        case 3: Dst = LocBuf;   MaxLen = 39;  break;
        case 4: Dst = SysBuf;   MaxLen = 35;  break;
        case 5: Dst = PhoneBuf; MaxLen = 39;  break;
        case 6: Dst = SpeedBuf; MaxLen = 15;  break;
        case 7: Dst = FlagsBuf; MaxLen = 255; break;
        default: Dst = NULL;    MaxLen = 0;   break;
        }

        if (Field < 7) {
            /* Read until comma or end of line */
            while (*p && *p != ',' && *p != '\n' && *p != '\r' && Len < MaxLen)
                Dst[Len++] = *p++;
            Dst[Len] = '\0';
            if (*p == ',') p++;         /* skip comma                    */
        } else {
            /* Field 7+ = rest of line (flags, comma-separated) */
            while (*p && *p != '\n' && *p != '\r' && Len < MaxLen)
                Dst[Len++] = *p++;
            Dst[Len] = '\0';
        }

        Field++;
    }

    if (Field < 7)
        return -1;                      /* not enough fields             */

    /* Convert underscores to spaces in name/location/sysop */
    for (Dst = NameBuf; *Dst; Dst++) if (*Dst == '_') *Dst = ' ';
    for (Dst = LocBuf;  *Dst; Dst++) if (*Dst == '_') *Dst = ' ';
    for (Dst = SysBuf;  *Dst; Dst++) if (*Dst == '_') *Dst = ' ';

    /* Process keyword to determine address context */
    if (strcmp(Keyword, "Zone") == 0) {
        *CurZone = (uint16_t)atoi(NumBuf);
        *CurNet  = (uint16_t)atoi(NumBuf);
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurZone;
        Ent->Addr.node = 0;
    }
    else if (strcmp(Keyword, "Region") == 0) {
        *CurNet = (uint16_t)atoi(NumBuf);
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurNet;
        Ent->Addr.node = 0;
    }
    else if (strcmp(Keyword, "Host") == 0) {
        *CurNet = (uint16_t)atoi(NumBuf);
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurNet;
        Ent->Addr.node = 0;
    }
    else if (strcmp(Keyword, "Hub") == 0 || strcmp(Keyword, "Boss") == 0 ||
             Keyword[0] == '\0') {
        /* Hub, Boss, or normal node -- under current net */
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurNet;
        Ent->Addr.node = (uint16_t)atoi(NumBuf);
    }
    else if (strcmp(Keyword, "Pvt") == 0) {
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurNet;
        Ent->Addr.node = (uint16_t)atoi(NumBuf);
        Ent->IsPvt = 1;
    }
    else if (strcmp(Keyword, "Down") == 0) {
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurNet;
        Ent->Addr.node = (uint16_t)atoi(NumBuf);
        Ent->IsDown = 1;
    }
    else if (strcmp(Keyword, "Hold") == 0) {
        Ent->Addr.zone = *CurZone;
        Ent->Addr.net  = *CurNet;
        Ent->Addr.node = (uint16_t)atoi(NumBuf);
        Ent->IsHold = 1;
    }
    else {
        return -1;                      /* unknown keyword               */
    }

    /* Store parsed fields */
    strncpy(Ent->System,   NameBuf,  sizeof(Ent->System) - 1);
    strncpy(Ent->Location, LocBuf,   sizeof(Ent->Location) - 1);
    strncpy(Ent->Sysop,    SysBuf,   sizeof(Ent->Sysop) - 1);
    strncpy(Ent->Phone,    PhoneBuf, sizeof(Ent->Phone) - 1);
    strncpy(Ent->Flags,    FlagsBuf, sizeof(Ent->Flags) - 1);
    Ent->Speed = (uint32_t)atol(SpeedBuf);

    /* Parse flags for capabilities */
    nl_parse_flags(Ent);

    return 0;
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                    Nodelist Database Operations                           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


/*-----------------------------------------------------------------------*/
/* nl_open() -- Open and parse a nodelist file                           */
/*                                                                       */
/* Reads the entire nodelist into memory. Starts with 4096 entry         */
/* capacity and doubles as needed, up to NL_MAX_NODES.                   */
/*                                                                       */
/* Returns allocated NlDatabase pointer, or NULL on error.               */
/*-----------------------------------------------------------------------*/

NlDatabase *nl_open(const char *Path)
{
    FILE       *f;                      /* nodelist file handle           */
    NlDatabase *Db;                     /* database being built          */
    char        Line[512];              /* line read buffer              */
    uint16_t    CurZone = 1;            /* current zone context          */
    uint16_t    CurNet  = 1;            /* current net context           */

    f = fopen(Path, "r");
    if (!f) {
        qf_log(LOG_ERROR, "Cannot open nodelist: %s", Path);
        return NULL;
    }

    Db = (NlDatabase *)calloc(1, sizeof(NlDatabase));
    if (!Db) { fclose(f); return NULL; }

    Db->Capacity = 4096;
    Db->Entries  = (NlEntry *)calloc(Db->Capacity, sizeof(NlEntry));
    if (!Db->Entries) { free(Db); fclose(f); return NULL; }

    while (fgets(Line, sizeof(Line), f)) {
        NlEntry Ent;                    /* parsed entry                  */

        if (nl_parse_line(Line, &Ent, &CurZone, &CurNet) == 0) {
            /* Grow array if needed */
            if (Db->Count >= Db->Capacity) {
                int      NewCap;        /* new capacity                  */
                NlEntry *NewBuf;        /* reallocated buffer            */

                NewCap = Db->Capacity * 2;
                if (NewCap > NL_MAX_NODES) break;

                NewBuf = (NlEntry *)realloc(Db->Entries,
                                             NewCap * sizeof(NlEntry));
                if (!NewBuf) break;

                Db->Entries  = NewBuf;
                Db->Capacity = NewCap;
            }

            Db->Entries[Db->Count++] = Ent;
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Nodelist loaded: %d entries from %s", Db->Count, Path);
    return Db;
}


/*-----------------------------------------------------------------------*/
/* nl_lookup() -- Find a node in the parsed nodelist database            */
/*                                                                       */
/* Linear scan -- acceptable for nodelists up to ~65K nodes.             */
/* A production implementation would use a hash table or binary search   */
/* on the pre-compiled .NDX index for O(1) / O(log n) lookup.           */
/*                                                                       */
/* Matches on zone:net/node only. Point matching is not implemented     */
/* because points are typically in separate pointlists with Boss         */
/* headers (FTS-5002).                                                   */
/*                                                                       */
/* Returns a pointer to the NlEntry, or NULL if not found.               */
/*-----------------------------------------------------------------------*/

const NlEntry *nl_lookup(const NlDatabase *Db, const FTN_ADDR *Addr)
{
    int  i;                             /* search index                  */
    char AddrBuf[64];                   /* formatted address for log     */

    if (!Db) return NULL;

    ftn_format_addr(Addr, AddrBuf, sizeof(AddrBuf));
    qf_log(LOG_DEBUG, "nl_lookup: searching for %s in %d entries",
           AddrBuf, Db->Count);

    for (i = 0; i < Db->Count; i++) {
        if (Db->Entries[i].Addr.zone == Addr->zone &&
            Db->Entries[i].Addr.net  == Addr->net &&
            Db->Entries[i].Addr.node == Addr->node) {
            qf_log(LOG_DEBUG, "nl_lookup: found %s -- \"%s\" by %s "
                   "(cm=%d down=%d hold=%d ibn=%d)",
                   AddrBuf, Db->Entries[i].System, Db->Entries[i].Sysop,
                   Db->Entries[i].IsCM, Db->Entries[i].IsDown,
                   Db->Entries[i].IsHold, Db->Entries[i].HasIBN);
            return &Db->Entries[i];
        }
    }

    qf_log(LOG_DEBUG, "nl_lookup: %s not found", AddrBuf);
    return NULL;
}


/*-----------------------------------------------------------------------*/
/* nl_find_host() -- Find a node's net host (for routing)                */
/*                                                                       */
/* Net host is always node 0 of the same net. Used for routing           */
/* decisions when no direct route is configured.                         */
/*-----------------------------------------------------------------------*/

const NlEntry *nl_find_host(const NlDatabase *Db, const FTN_ADDR *Addr)
{
    FTN_ADDR HostAddr;                  /* net host address              */

    HostAddr.zone  = Addr->zone;
    HostAddr.net   = Addr->net;
    HostAddr.node  = 0;                 /* net host is always node 0     */
    HostAddr.point = 0;

    return nl_lookup(Db, &HostAddr);
}


/*-----------------------------------------------------------------------*/
/* nl_close() -- Close and free nodelist database                        */
/*-----------------------------------------------------------------------*/

void nl_close(NlDatabase *Db)
{
    if (Db) {
        free(Db->Entries);
        free(Db);
    }
}


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*                      Accessors for qfront.c                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

int nl_entry_is_cm(const NlEntry *E)   { return E ? E->IsCM   : 0; }
int nl_entry_is_down(const NlEntry *E) { return E ? E->IsDown : 0; }
int nl_entry_is_hold(const NlEntry *E) { return E ? E->IsHold : 0; }

const char *nl_entry_ibn_host(const NlEntry *E)
{
    return (E && E->IbnHost[0]) ? E->IbnHost : "";
}
