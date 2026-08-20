/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* route.c -- NetMail Routing Engine                                        */
/*                                                                           */
/* Resolves the actual dial target for a FidoNet address. Implements         */
/* routing rules equivalent to QFront's QTRANS.DAT:                          */
/*   - Direct (call node directly)                                          */
/*   - Via host (route through net host, node 0)                            */
/*   - Via hub (route through specified hub)                                */
/*   - Via address (route through specific node)                            */
/*   - Hold (don't dial, wait for pickup)                                   */
/*   - NoPoll (never initiate to this node)                                 */
/*                                                                           */
/* Clean-room from QFront documentation + FTS-5005.                          */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "qfront.h"

#define MAX_ROUTES 256                  /* max routing rules             */

/* ---- Route Rule Types ---- */
typedef enum {
    RT_DIRECT,                          /* call node directly            */
    RT_VIA_HOST,                        /* route through net host (/0)   */
    RT_VIA_HUB,                         /* route through hub             */
    RT_VIA,                             /* route through specific node   */
    RT_HOLD,                            /* hold -- wait for them to call */
    RT_ABSHOLD,                         /* absolute hold -- never send   */
    RT_NOPOLL                           /* never poll this node          */
} RouteType;

typedef struct {
    FTN_ADDR  Match;                    /* address pattern to match      */
    int       MatchZone;                /* match zone? (0=wildcard)      */
    int       MatchNet;                 /* match net? (0=wildcard)       */
    int       MatchNode;                /* match node? (0=wildcard)      */
    RouteType Type;                     /* how to route                  */
    FTN_ADDR  Via;                      /* route via this address        */
} RouteRule;

typedef struct {
    RouteRule Rules[MAX_ROUTES];        /* routing rule table            */
    int       Count;                    /* number of rules loaded        */
} RouteTable;

static RouteTable g_Routes;            /* global routing table          */


/*-----------------------------------------------------------------------*/
/* rt_parse_rule() -- Parse a routing rule from config line               */
/*                                                                       */
/* Config syntax (in qfront.cfg):                                        */
/*   Route <pattern> direct                                              */
/*   Route <pattern> via <address>                                       */
/*   Route <pattern> host          (route through net host)              */
/*   Route <pattern> hub <address> (route through hub)                   */
/*   Route <pattern> hold                                                */
/*   Route <pattern> abshold                                             */
/*   NoPoll <address>                                                    */
/*                                                                       */
/* Pattern: zone:net/node or zone:net/* or zone:* or *                   */
/* Wildcard: * means match any value for that position.                  */
/*                                                                       */
/* Returns 0 on success, -1 on parse error.                              */
/*-----------------------------------------------------------------------*/

static int rt_parse_rule(const char *Line, RouteRule *Rule)
{
    char Cmd[16];                       /* command keyword               */
    char Pattern[64];                   /* address pattern               */
    char Action[16];                    /* action keyword                */
    char ViaStr[64];                    /* via address string            */
    int  NumFields;                     /* sscanf field count            */

    memset(Rule, 0, sizeof(*Rule));

    NumFields = sscanf(Line, "%15s %63s %15s %63s", Cmd, Pattern, Action, ViaStr);
    if (NumFields < 2) return -1;

    /* NoPoll is a shortcut */
    if (strcmp(Cmd, "NoPoll") == 0 || strcmp(Cmd, "nopoll") == 0) {
        if (ftn_parse_addr(Pattern, &Rule->Match) != 0)
            return -1;
        Rule->MatchZone = 1;
        Rule->MatchNet  = 1;
        Rule->MatchNode = 1;
        Rule->Type = RT_NOPOLL;
        return 0;
    }

    if (strcmp(Cmd, "Route") != 0 && strcmp(Cmd, "route") != 0)
        return -1;
    if (NumFields < 3) return -1;

    /* Parse pattern with wildcards */
    Rule->MatchZone = 1;
    Rule->MatchNet  = 1;
    Rule->MatchNode = 1;

    {
        const char *p = Pattern;        /* pattern scan pointer          */
        char ZBuf[8]  = "";             /* zone text buffer              */
        char NBuf[8]  = "";             /* net text buffer               */
        char NoBuf[8] = "";             /* node text buffer              */

        if (strcmp(Pattern, "*") == 0) {
            /* Match everything */
            Rule->MatchZone = 0;
            Rule->MatchNet  = 0;
            Rule->MatchNode = 0;
        } else if (strchr(p, ':')) {
            /* Has zone */
            sscanf(p, "%7[^:]", ZBuf);
            p = strchr(p, ':') + 1;

            if (strcmp(ZBuf, "*") == 0)
                Rule->MatchZone = 0;
            else
                Rule->Match.zone = (uint16_t)atoi(ZBuf);

            if (strchr(p, '/')) {
                sscanf(p, "%7[^/]", NBuf);
                p = strchr(p, '/') + 1;
                strncpy(NoBuf, p, sizeof(NoBuf) - 1);

                if (strcmp(NBuf, "*") == 0)
                    Rule->MatchNet = 0;
                else
                    Rule->Match.net = (uint16_t)atoi(NBuf);

                if (strcmp(NoBuf, "*") == 0)
                    Rule->MatchNode = 0;
                else
                    Rule->Match.node = (uint16_t)atoi(NoBuf);
            } else {
                /* zone:* */
                Rule->MatchNet  = 0;
                Rule->MatchNode = 0;
            }
        } else {
            /* Just net/node */
            ftn_parse_addr(Pattern, &Rule->Match);
        }
    }

    /* Parse action */
    if (strcmp(Action, "direct") == 0)
        Rule->Type = RT_DIRECT;
    else if (strcmp(Action, "host") == 0)
        Rule->Type = RT_VIA_HOST;
    else if (strcmp(Action, "hold") == 0)
        Rule->Type = RT_HOLD;
    else if (strcmp(Action, "abshold") == 0)
        Rule->Type = RT_ABSHOLD;
    else if (strcmp(Action, "via") == 0) {
        if (NumFields < 4) return -1;
        Rule->Type = RT_VIA;
        if (ftn_parse_addr(ViaStr, &Rule->Via) != 0)
            return -1;
    }
    else if (strcmp(Action, "hub") == 0) {
        if (NumFields < 4) return -1;
        Rule->Type = RT_VIA_HUB;
        if (ftn_parse_addr(ViaStr, &Rule->Via) != 0)
            return -1;
    }
    else
        return -1;

    return 0;
}


/*-----------------------------------------------------------------------*/
/* rt_matches() -- Check if an address matches a rule pattern            */
/*                                                                       */
/* Compares zone/net/node with wildcards. A field with MatchXxx=0        */
/* matches any value (wildcard).                                         */
/*                                                                       */
/* Returns 1 if match, 0 if no match.                                    */
/*-----------------------------------------------------------------------*/

static int rt_matches(const RouteRule *Rule, const FTN_ADDR *Addr)
{
    if (Rule->MatchZone && Rule->Match.zone != Addr->zone)
        return 0;
    if (Rule->MatchNet && Rule->Match.net != Addr->net)
        return 0;
    if (Rule->MatchNode && Rule->Match.node != Addr->node)
        return 0;
    return 1;
}


/*-----------------------------------------------------------------------*/
/* rt_load() -- Load routing rules from config file                      */
/*                                                                       */
/* Reads Route and NoPoll lines from the config file. Rules are          */
/* stored in order -- first match wins during resolution.                */
/*                                                                       */
/* Returns 0 on success, -1 on error.                                    */
/*-----------------------------------------------------------------------*/

int rt_load(const char *CfgPath)
{
    FILE *f;                            /* config file handle            */
    char  Line[512];                    /* line read buffer              */

    g_Routes.Count = 0;

    f = fopen(CfgPath, "r");
    if (!f) return -1;

    while (fgets(Line, sizeof(Line), f)) {
        char      *p = Line;            /* line scan pointer             */
        RouteRule  Rule;                /* parsed rule                   */

        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n') continue;

        /* Only parse Route and NoPoll lines */
        if (strncmp(p, "Route",  5) != 0 &&
            strncmp(p, "route",  5) != 0 &&
            strncmp(p, "NoPoll", 6) != 0 &&
            strncmp(p, "nopoll", 6) != 0)
            continue;

        if (rt_parse_rule(p, &Rule) == 0 &&
            g_Routes.Count < MAX_ROUTES) {
            g_Routes.Rules[g_Routes.Count++] = Rule;
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Routing: %d rules loaded", g_Routes.Count);
    return 0;
}


/*-----------------------------------------------------------------------*/
/* rt_resolve() -- Resolve routing for an address                        */
/*                                                                       */
/* Walks the routing table in order. First match wins.                   */
/*                                                                       */
/* Returns:                                                              */
/*   0  = direct (call DestAddr directly, ViaAddr unchanged)             */
/*   1  = via (call *ViaAddr instead, contains the relay node)           */
/*  -1  = hold/abshold/nopoll (don't call)                               */
/*                                                                       */
/* If no rule matches, default is direct.                                */
/*-----------------------------------------------------------------------*/

int rt_resolve(const FTN_ADDR *DestAddr, FTN_ADDR *ViaAddr)
{
    int  i;                             /* rule loop index               */
    char DestBuf[64];                   /* formatted dest for log        */
    char ViaBuf[64];                    /* formatted via for log         */

    for (i = 0; i < g_Routes.Count; i++) {
        const RouteRule *Rule = &g_Routes.Rules[i];

        if (!rt_matches(Rule, DestAddr))
            continue;

        /* First match wins */
        switch (Rule->Type) {
        case RT_DIRECT:
            ftn_format_addr(DestAddr, DestBuf, sizeof(DestBuf));
            qf_log(LOG_DEBUG, "Route %s: direct", DestBuf);
            return 0;

        case RT_VIA:
            *ViaAddr = Rule->Via;
            ftn_format_addr(DestAddr, DestBuf, sizeof(DestBuf));
            ftn_format_addr(ViaAddr, ViaBuf, sizeof(ViaBuf));
            qf_log(LOG_DEBUG, "Route %s: via %s", DestBuf, ViaBuf);
            return 1;

        case RT_VIA_HOST:
            *ViaAddr = *DestAddr;
            ViaAddr->node = 0;          /* net host is node 0            */
            ftn_format_addr(DestAddr, DestBuf, sizeof(DestBuf));
            ftn_format_addr(ViaAddr, ViaBuf, sizeof(ViaBuf));
            qf_log(LOG_DEBUG, "Route %s: via host %s", DestBuf, ViaBuf);
            return 1;

        case RT_VIA_HUB:
            *ViaAddr = Rule->Via;
            ftn_format_addr(DestAddr, DestBuf, sizeof(DestBuf));
            ftn_format_addr(ViaAddr, ViaBuf, sizeof(ViaBuf));
            qf_log(LOG_DEBUG, "Route %s: via hub %s", DestBuf, ViaBuf);
            return 1;

        case RT_HOLD:
        case RT_ABSHOLD:
            ftn_format_addr(DestAddr, DestBuf, sizeof(DestBuf));
            qf_log(LOG_DEBUG, "Route %s: hold", DestBuf);
            return -1;

        case RT_NOPOLL:
            ftn_format_addr(DestAddr, DestBuf, sizeof(DestBuf));
            qf_log(LOG_DEBUG, "Route %s: nopoll", DestBuf);
            return -1;
        }
    }

    /* No matching rule -- default to direct */
    return 0;
}
