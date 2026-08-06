/*===========================================================================*/
/* REJECTS.HPP — public interface for the 15.4 UUIN sender-blacklist filter. */
/*                                                                           */
/* See REJECTS.CPP for implementation notes.                                 */
/*===========================================================================*/

#ifndef REJECTS_HPP
#define REJECTS_HPP

#include <stdio.h>

/* checkReject() return codes. */
#define REJECT_NONE     0    /* sender allowed — deliver normally */
#define REJECT_DROP     1    /* sender rejected — silently drop */
#define REJECT_BOUNCE   2    /* sender rejected — bounce (email only) */

/* Load the REJECTS file into memory.  Call once at UUIN startup.
 * Returns number of entries loaded (0 = file absent, filter disabled). */
int initRejects(const char *uucpPath);

/* Consult the loaded list.  Called per-message after From: is parsed. */
int checkReject(const char *fromAddress);

/* Send a bounce message.  For REJECT_BOUNCE on email only. */
int bounceEmail(const char *fromAddress, const char *originalSubject,
                const char *uucpPath);

/* Debug dump — print the loaded table to `out` (or stdout if NULL). */
void dumpRejects(FILE *out);

#endif /* REJECTS_HPP */
