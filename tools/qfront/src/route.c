/* ====================================================================
 * route.c — NetMail Routing Engine
 * ====================================================================
 * Resolves the actual dial target for a FidoNet address.
 * Implements routing rules equivalent to QFront's QTRANS.DAT:
 *   - Direct (call node directly)
 *   - Via host (route through net host, node 0)
 *   - Via hub (route through specified hub)
 *   - Via address (route through specific node)
 *   - Hold (don't dial, wait for pickup)
 *   - NoPoll (never initiate to this node)
 *
 * Route types match QFCONFIG binary strings:
 *   "Direct to target"
 *   "Route through target's host"
 *   "Route through target's hub"
 *   "Route through another node"
 *   "Hold for target"
 *   "Absolute hold"
 *
 * Clean-room from QFront documentation + FTS-5005.
 * ==================================================================== */

#include "qfront.h"

#define MAX_ROUTES 256

/* ---- Route Rule ---- */
typedef enum {
    RT_DIRECT,                    /* Call node directly            */
    RT_VIA_HOST,                  /* Route through net host (/0)   */
    RT_VIA_HUB,                  /* Route through hub              */
    RT_VIA,                       /* Route through specific node   */
    RT_HOLD,                      /* Hold — wait for them to call  */
    RT_ABSHOLD,                   /* Absolute hold — never send    */
    RT_NOPOLL                     /* Never poll this node          */
} RouteType;

typedef struct {
    FTN_ADDR match;               /* Address pattern to match      */
    int      match_zone;          /* Match zone? (0=wildcard)      */
    int      match_net;           /* Match net? (0=wildcard)       */
    int      match_node;          /* Match node? (0=wildcard)      */
    RouteType type;               /* How to route                  */
    FTN_ADDR via;                 /* Route via this address         */
} RouteRule;

typedef struct {
    RouteRule rules[MAX_ROUTES];
    int       count;
} RouteTable;

static RouteTable g_routes;


/* ---- Parse a Route Rule ----
 *
 * Config syntax (in qfront.cfg):
 *   Route <pattern> direct
 *   Route <pattern> via <address>
 *   Route <pattern> host                (route through net host)
 *   Route <pattern> hub <address>       (route through hub)
 *   Route <pattern> hold
 *   Route <pattern> abshold
 *   NoPoll <address>
 *
 * Pattern: zone:net/node or zone:net/* or zone:* or *
 * Wildcard: * means match any value for that position.
 *
 * Examples:
 *   Route 1:234/56 direct               Call 1:234/56 directly
 *   Route 1:234/* via 1:234/0           Route all of net 234 via host
 *   Route 2:* via 2:5020/0             Route all zone 2 via 2:5020/0
 *   Route 1:100/* host                  Route net 100 through its host
 *   NoPoll 1:234/99                     Never poll this node
 */
static int rt_parse_rule(const char *line, RouteRule *rule)
{
    char cmd[16], pattern[64], action[16], via_str[64];
    int n;

    memset(rule, 0, sizeof(*rule));

    n = sscanf(line, "%15s %63s %15s %63s", cmd, pattern, action, via_str);
    if (n < 2) return -1;

    /* NoPoll is a shortcut */
    if (strcmp(cmd, "NoPoll") == 0 || strcmp(cmd, "nopoll") == 0) {
        if (ftn_parse_addr(pattern, &rule->match) != 0)
            return -1;
        rule->match_zone = 1;
        rule->match_net  = 1;
        rule->match_node = 1;
        rule->type = RT_NOPOLL;
        return 0;
    }

    if (strcmp(cmd, "Route") != 0 && strcmp(cmd, "route") != 0)
        return -1;

    if (n < 3) return -1;

    /* Parse pattern with wildcards */
    rule->match_zone = 1;
    rule->match_net  = 1;
    rule->match_node = 1;

    /* Check for wildcards in pattern */
    {
        const char *p = pattern;
        char zbuf[8] = "", nbuf[8] = "", nobuf[8] = "";

        if (strcmp(pattern, "*") == 0) {
            /* Match everything */
            rule->match_zone = 0;
            rule->match_net  = 0;
            rule->match_node = 0;
        } else if (strchr(p, ':')) {
            /* Has zone */
            sscanf(p, "%7[^:]", zbuf);
            p = strchr(p, ':') + 1;

            if (strcmp(zbuf, "*") == 0)
                rule->match_zone = 0;
            else
                rule->match.zone = (uint16_t)atoi(zbuf);

            if (strchr(p, '/')) {
                sscanf(p, "%7[^/]", nbuf);
                p = strchr(p, '/') + 1;
                strncpy(nobuf, p, sizeof(nobuf) - 1);

                if (strcmp(nbuf, "*") == 0)
                    rule->match_net = 0;
                else
                    rule->match.net = (uint16_t)atoi(nbuf);

                if (strcmp(nobuf, "*") == 0)
                    rule->match_node = 0;
                else
                    rule->match.node = (uint16_t)atoi(nobuf);
            } else {
                /* zone:* */
                rule->match_net  = 0;
                rule->match_node = 0;
            }
        } else {
            /* Just net/node */
            ftn_parse_addr(pattern, &rule->match);
        }
    }

    /* Parse action */
    if (strcmp(action, "direct") == 0)
        rule->type = RT_DIRECT;
    else if (strcmp(action, "host") == 0)
        rule->type = RT_VIA_HOST;
    else if (strcmp(action, "hold") == 0)
        rule->type = RT_HOLD;
    else if (strcmp(action, "abshold") == 0)
        rule->type = RT_ABSHOLD;
    else if (strcmp(action, "via") == 0) {
        if (n < 4) return -1;
        rule->type = RT_VIA;
        if (ftn_parse_addr(via_str, &rule->via) != 0)
            return -1;
    }
    else if (strcmp(action, "hub") == 0) {
        if (n < 4) return -1;
        rule->type = RT_VIA_HUB;
        if (ftn_parse_addr(via_str, &rule->via) != 0)
            return -1;
    }
    else
        return -1;

    return 0;
}


/* ---- Check if an Address Matches a Rule Pattern ---- */

static int rt_matches(const RouteRule *rule, const FTN_ADDR *addr)
{
    if (rule->match_zone && rule->match.zone != addr->zone)
        return 0;
    if (rule->match_net && rule->match.net != addr->net)
        return 0;
    if (rule->match_node && rule->match.node != addr->node)
        return 0;
    return 1;
}


/* ---- Load Routing Rules from Config ---- */

int rt_load(const char *cfgpath)
{
    FILE *f;
    char line[512];

    g_routes.count = 0;

    f = fopen(cfgpath, "r");
    if (!f) return -1;

    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        RouteRule rule;

        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n') continue;

        /* Only parse Route and NoPoll lines */
        if (strncmp(p, "Route", 5) != 0 &&
            strncmp(p, "route", 5) != 0 &&
            strncmp(p, "NoPoll", 6) != 0 &&
            strncmp(p, "nopoll", 6) != 0)
            continue;

        if (rt_parse_rule(p, &rule) == 0 &&
            g_routes.count < MAX_ROUTES) {
            g_routes.rules[g_routes.count++] = rule;
        }
    }

    fclose(f);

    qf_log(LOG_INFO, "Routing: %d rules loaded", g_routes.count);
    return 0;
}


/* ---- Resolve Routing for an Address ----
 *
 * Walks the routing table in order. First match wins.
 * Returns:
 *   0  = direct (call dest_addr directly, dest unchanged)
 *   1  = via (call *via_addr instead, contains the relay node)
 *  -1  = hold/abshold/nopoll (don't call)
 *
 * If no rule matches, default is direct. */

int rt_resolve(const FTN_ADDR *dest, FTN_ADDR *via_addr)
{
    int i;
    char dbuf[64], vbuf[64];

    for (i = 0; i < g_routes.count; i++) {
        const RouteRule *rule = &g_routes.rules[i];

        if (!rt_matches(rule, dest))
            continue;

        /* First match wins */
        switch (rule->type) {
        case RT_DIRECT:
            ftn_format_addr(dest, dbuf, sizeof(dbuf));
            qf_log(LOG_DEBUG, "Route %s: direct", dbuf);
            return 0;

        case RT_VIA:
            *via_addr = rule->via;
            ftn_format_addr(dest, dbuf, sizeof(dbuf));
            ftn_format_addr(via_addr, vbuf, sizeof(vbuf));
            qf_log(LOG_DEBUG, "Route %s: via %s", dbuf, vbuf);
            return 1;

        case RT_VIA_HOST:
            *via_addr = *dest;
            via_addr->node = 0;  /* Net host is node 0           */
            ftn_format_addr(dest, dbuf, sizeof(dbuf));
            ftn_format_addr(via_addr, vbuf, sizeof(vbuf));
            qf_log(LOG_DEBUG, "Route %s: via host %s", dbuf, vbuf);
            return 1;

        case RT_VIA_HUB:
            *via_addr = rule->via;
            ftn_format_addr(dest, dbuf, sizeof(dbuf));
            ftn_format_addr(via_addr, vbuf, sizeof(vbuf));
            qf_log(LOG_DEBUG, "Route %s: via hub %s", dbuf, vbuf);
            return 1;

        case RT_HOLD:
        case RT_ABSHOLD:
            ftn_format_addr(dest, dbuf, sizeof(dbuf));
            qf_log(LOG_DEBUG, "Route %s: hold", dbuf);
            return -1;

        case RT_NOPOLL:
            ftn_format_addr(dest, dbuf, sizeof(dbuf));
            qf_log(LOG_DEBUG, "Route %s: nopoll", dbuf);
            return -1;
        }
    }

    /* No matching rule — default to direct */
    return 0;
}
