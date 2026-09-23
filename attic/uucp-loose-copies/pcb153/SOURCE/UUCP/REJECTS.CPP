/*===========================================================================*/
/* REJECTS.CPP                                                                */
/*                                                                           */
/* 15.4 REJECTS filter for UUIN.  Sender-address blacklist that lets sysops   */
/* bounce inbound e-mail or silently drop UUCP news from named addresses.    */
/*                                                                           */
/* Method notes:                                                             */
/*   - Reverse-engineered from UUIN.EXE 15.4b static string evidence         */
/*     (see PCBSRCV/026 CHANGES entry and SYSOP_154.TXT section 13).         */
/*   - File format specified in pcb154b/WHATSNEW item #10.                   */
/*   - Table structure: 16-entry fixed array (WHATSNEW MAX = 16).            */
/*                                                                           */
/* File format (ASCII, in PcbData.uucpPath):                                 */
/*   from_address, action                                                    */
/*   from_address2, action2                                                  */
/*   ...                                                                     */
/* Where action is "%" (bounce, e-mail only) or absent (silent drop).        */
/*                                                                           */
/* Written by: hexadecimal, v0.027                                           */
/*===========================================================================*/

#include <misc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "rejects.hpp"

/*---------------------------------------------------------------------------
 * Configuration constants.
 *---------------------------------------------------------------------------*/

#define REJECTS_MAX          16       /* per WHATSNEW: max 16 entries       */
#define REJECTS_ADDR_MAX    120       /* matches msgLFrom/replyto buffers   */
#define REJECTS_FILENAME    "REJECTS" /* verified in UUIN.EXE 15.4b @ 0x3d344 */

/*---------------------------------------------------------------------------
 * In-memory table.  Loaded once at UUIN startup; consulted per-message.
 *---------------------------------------------------------------------------*/

struct rejectEntry {
    char address[REJECTS_ADDR_MAX + 1];   /* lowercased for case-insensitive match */
    int  bounce;                          /* 1 = bounce (%), 0 = silent drop */
};

static rejectEntry rejects[REJECTS_MAX];
static int         nRejects   = 0;
static int         rejectsLoaded = 0;


/*---------------------------------------------------------------------------
 * strip_and_lower — normalize an address for comparison.
 * Strips leading/trailing whitespace and lowercases in place.
 *---------------------------------------------------------------------------*/

static void strip_and_lower(char *s)
{
    /* leading whitespace */
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);

    /* trailing whitespace, incl. CR/LF */
    int n = strlen(s);
    while (n > 0 && (s[n-1] == ' '  || s[n-1] == '\t' ||
                     s[n-1] == '\r' || s[n-1] == '\n'))
    {
        s[--n] = 0;
    }

    /* lowercase */
    strlwr(s);
}


/*---------------------------------------------------------------------------
 * loadRejects — read the REJECTS file into the in-memory table.  Called
 * once at UUIN startup by initRejects().  Silently succeeds if the file
 * doesn't exist (feature is opt-in — sysops only opt in by creating the
 * file).
 *---------------------------------------------------------------------------*/

static int loadRejects(const char *uucpPath)
{
    char path[300];
    strcpy(path, uucpPath);
    strcat(path, REJECTS_FILENAME);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        /* No REJECTS file — feature disabled, but not an error. */
        return 0;
    }

    char line[REJECTS_ADDR_MAX + 32];
    while (nRejects < REJECTS_MAX && fgets(line, sizeof(line), fp) != NULL) {
        /* Split at first comma.  Left side = address, right side = action. */
        char *comma = strchr(line, ',');
        if (comma == NULL) continue;     /* malformed line — skip */

        *comma = 0;
        char *action = comma + 1;

        /* Normalize the address (strip whitespace, lowercase). */
        strip_and_lower(line);
        if (line[0] == 0) continue;      /* blank address — skip */

        /* Store it.  Truncate if too long. */
        maxstrcpy(rejects[nRejects].address, line,
                  sizeof(rejects[nRejects].address));

        /* Check for the bounce indicator — first non-space char is '%'. */
        while (*action == ' ' || *action == '\t') action++;
        rejects[nRejects].bounce = (*action == '%') ? 1 : 0;

        nRejects++;
    }

    fclose(fp);
    return nRejects;
}


/*---------------------------------------------------------------------------
 * initRejects — public entry point.  Called once from UUIN.CPP main after
 * PcbData is loaded.  Idempotent — calling twice reloads the table.
 *---------------------------------------------------------------------------*/

int initRejects(const char *uucpPath)
{
    nRejects = 0;
    memset(rejects, 0, sizeof(rejects));

    int loaded = loadRejects(uucpPath);
    rejectsLoaded = 1;
    return loaded;
}


/*---------------------------------------------------------------------------
 * checkReject — is this sender on the REJECTS list?
 *
 * Returns:
 *    REJECT_NONE (0)  — sender is allowed
 *    REJECT_DROP (1)  — sender rejected; silently drop message
 *    REJECT_BOUNCE(2) — sender rejected; bounce message back (email only)
 *---------------------------------------------------------------------------*/

int checkReject(const char *fromAddress)
{
    if (!rejectsLoaded || nRejects == 0 || fromAddress == NULL) {
        return REJECT_NONE;
    }

    /* Normalize the incoming address for comparison. */
    char lookup[REJECTS_ADDR_MAX + 1];
    maxstrcpy(lookup, (char *)fromAddress, sizeof(lookup));
    strip_and_lower(lookup);
    if (lookup[0] == 0) return REJECT_NONE;

    /* Linear scan — the list is at most 16 entries so O(n) is fine. */
    for (int i = 0; i < nRejects; i++) {
        /* Substring match: WHATSNEW's example shows "annoying@bad.stor"
         * matching any address containing that exact string.  We compare
         * against the whole address as a substring; this catches things
         * like "annoying@bad.stor" appearing in a "From: Full Name
         * <annoying@bad.stor>" header where parseName() already extracted
         * just the addr portion, but also matches partial patterns like
         * "@bad.stor" which some sysops may want.
         */
        if (strstr(lookup, rejects[i].address) != NULL) {
            return rejects[i].bounce ? REJECT_BOUNCE : REJECT_DROP;
        }
    }

    return REJECT_NONE;
}


/*---------------------------------------------------------------------------
 * bounceEmail — send a bounce message back to the sender.  Uses
 * BOUNCE.TXT from the UUCP base path as the template body.
 *
 * Called from UUIN.CPP when checkReject() returned REJECT_BOUNCE and the
 * message is e-mail (not news).  For news, the caller must silently drop
 * — per WHATSNEW: "only email can be bounced news articles can only be
 * disregarded."
 *---------------------------------------------------------------------------*/

int bounceEmail(const char *fromAddress, const char *originalSubject,
                const char *uucpPath)
{
    char path[300];
    strcpy(path, uucpPath);
    strcat(path, "BOUNCE.TXT");

    /* Log the bounce attempt.  Log format matches PCBOARDM.EXE 15.4b:
     *   "Bounced Email -- %s (%s)"
     */
    extern void writeUucplog(const char *fmt, ...);   /* UUIN logging */
    writeUucplog("Bounced Email -- %s (%s)", fromAddress,
                 originalSubject ? originalSubject : "(no subject)");

    /* Read BOUNCE.TXT template.  If missing, use a compiled-in default so
     * the bounce still goes out (sysops shouldn't have to create the file
     * to get the feature).
     */
    const char *defaultBody =
        "Your message was rejected by this system's filter.\r\n"
        "\r\n"
        "This is an automated response.\r\n";

    char body[4096];
    body[0] = 0;

    FILE *fp = fopen(path, "r");
    if (fp != NULL) {
        int nread = fread(body, 1, sizeof(body) - 1, fp);
        body[nread] = 0;
        fclose(fp);
    } else {
        strcpy(body, defaultBody);
    }

    /* Delegate to UUOUT for the actual bounce dispatch.  UUOUT queues
     * outbound mail via the standard uucp spool mechanism.  For a bounce
     * we spool a message to `fromAddress` with subject "Bounced: <orig>"
     * and body = template.
     */
    extern int uucpQueueOutbound(const char *to, const char *subject,
                                 const char *body);
    char subject[240 + 1];
    if (originalSubject && originalSubject[0]) {
        buildstr(subject, "Bounced: ", originalSubject, NULL);
    } else {
        strcpy(subject, "Bounced Message");
    }

    return uucpQueueOutbound(fromAddress, subject, body);
}


/*---------------------------------------------------------------------------
 * Debug helper — dump the loaded REJECTS table.  Used by the -verbose
 * command-line flag to help sysops confirm their file was parsed correctly.
 *---------------------------------------------------------------------------*/

void dumpRejects(FILE *out)
{
    if (out == NULL) out = stdout;

    fprintf(out, "REJECTS filter: %d entries loaded\n", nRejects);
    for (int i = 0; i < nRejects; i++) {
        fprintf(out, "  [%2d] %s  %s\n",
                i,
                rejects[i].address,
                rejects[i].bounce ? "(bounce)" : "(drop)");
    }
    if (nRejects == 0) {
        fprintf(out, "  (no REJECTS file or file empty — filter inactive)\n");
    }
}
