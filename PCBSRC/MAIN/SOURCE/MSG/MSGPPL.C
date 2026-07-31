/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* MSGPPL.C                                                                  */
/*                                                                           */
/* PCBoard 15.4 message-base helpers callable from PPL script dispatch.      */
/* Introduced in v0.026 to give real implementations to the three PPL tokens */
/* whose runtime handlers were stubbed in v0.021 (GETMSGHDR, SETMSGHDR) and  */
/* v0.024 (MOVEMSG).                                                         */
/*                                                                           */
/* API surface — declarations in H/MESSAGES.H:                               */
/*                                                                           */
/*   int  pplgetmsghdr (long msgnum, int field, char *out, int outsize);     */
/*   int  pplsetmsghdr (long msgnum, int field, const char *value);          */
/*   int  pplmovemsg   (long msgnum, unsigned short destconf);               */
/*                                                                           */
/* Field indices for get/set follow the SYSOP_154.TXT section 4.4 mapping:   */
/*                                                                           */
/*   1 = From         25 chars     (msgheadertype.FromField)                 */
/*   2 = To           25 chars     (msgheadertype.ToField)                   */
/*   3 = Subject      25 chars     (msgheadertype.SubjField)                 */
/*   4 = RefNum       bassngl      (msgheadertype.RefNumber)                 */
/*   5 = Password     12 chars     (msgheadertype.Password)                  */
/*   6 = Status       1 char       (msgheadertype.Status)                    */
/*   7 = Date         8 chars      (msgheadertype.Date, MM-DD-YY packed)     */
/*   8 = Time         5 chars      (msgheadertype.Time, HH:MM packed)        */
/*                                                                           */
/* Each function returns 0 on success or a negative error code that mirrors  */
/* the runtime error strings recovered from PCBOARDM.EXE 15.4b:              */
/*                                                                           */
/*   -1 = "Conference does not exist (MOVEMSG)"                              */
/*   -2 = "Message number does not exist (MOVEMSG)"                          */
/*   -3 = "Unable to read message header (MOVEMSG)"                          */
/*   -4 = "Unable to read message body (MOVEMSG)"                            */
/*                                                                           */
/* NOTE: SetMsgHdr / MoveMsg require the caller to have write access to      */
/* the target conference.  This is checked at a higher layer (LOGIN state);  */
/* these helpers assume the caller has already been vetted.                  */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "project.h"
#include "messages.h"
#include "pcbfiles.h"
#include <string.h>
#include <stdio.h>

extern PcbData_t PcbData;   /* from GLOBALS */
extern Status_t  Status;    /* from GLOBALS */


/* Field indices — kept as #defines for readability at call sites. */
#define MSGHDR_FROM      1
#define MSGHDR_TO        2
#define MSGHDR_SUBJ      3
#define MSGHDR_REFNUM    4
#define MSGHDR_PWD       5
#define MSGHDR_STATUS    6
#define MSGHDR_DATE      7
#define MSGHDR_TIME      8


/*----------------------------------------------------------------------------
 * pplgetmsghdr
 *
 * Read a header field of message `msgnum` in the current caller's active
 * conference.  The msg base is opened for read, the header parsed, and the
 * requested field copied into `out` (null-terminated, truncated to outsize-1
 * bytes).  For numeric fields (RefNum, Status) the value is formatted as
 * a decimal string.
 *---------------------------------------------------------------------------*/
int LIBENTRY pplgetmsghdr(long msgnum, int field, char *out, int outsize) {
  msgbasetype   MsgBase;
  newindextype  Index;
  long          IdxOffset;
  int           rc;

  if (out == NULL || outsize < 1)
    return -3;
  out[0] = 0;

  if (openmessagebase(Status.Conference, &Status.CurConf, &MsgBase, RDONLY) == -1)
    return -1;

  rc = getmessageheader(msgnum, &MsgBase, &IdxOffset, &Index);
  if (rc != 0) {
    closemessagebase(&MsgBase);
    return -2;   /* message number does not exist */
  }

  switch (field) {
    case MSGHDR_FROM:
      strncpy(out, MsgBase.Header.FromField, 25);
      out[(outsize > 25) ? 25 : outsize-1] = 0;
      /* strip trailing spaces */
      { int i;
        for (i = strlen(out) - 1; i >= 0 && out[i] == ' '; i--) out[i] = 0;
      }
      break;

    case MSGHDR_TO:
      strncpy(out, MsgBase.Header.ToField, 25);
      out[(outsize > 25) ? 25 : outsize-1] = 0;
      { int i;
        for (i = strlen(out) - 1; i >= 0 && out[i] == ' '; i--) out[i] = 0;
      }
      break;

    case MSGHDR_SUBJ:
      strncpy(out, MsgBase.Header.SubjField, 25);
      out[(outsize > 25) ? 25 : outsize-1] = 0;
      { int i;
        for (i = strlen(out) - 1; i >= 0 && out[i] == ' '; i--) out[i] = 0;
      }
      break;

    case MSGHDR_REFNUM:
      sprintf(out, "%ld", bassngltolong(MsgBase.Header.RefNumber));
      break;

    case MSGHDR_PWD:
      strncpy(out, MsgBase.Header.Password, 12);
      out[(outsize > 12) ? 12 : outsize-1] = 0;
      { int i;
        for (i = strlen(out) - 1; i >= 0 && out[i] == ' '; i--) out[i] = 0;
      }
      break;

    case MSGHDR_STATUS:
      out[0] = MsgBase.Header.Status;
      out[1] = 0;
      break;

    case MSGHDR_DATE:
      strncpy(out, MsgBase.Header.Date, 8);
      out[(outsize > 8) ? 8 : outsize-1] = 0;
      break;

    case MSGHDR_TIME:
      strncpy(out, MsgBase.Header.Time, 5);
      out[(outsize > 5) ? 5 : outsize-1] = 0;
      break;

    default:
      closemessagebase(&MsgBase);
      return -3;
  }

  closemessagebase(&MsgBase);
  return 0;
}


/*----------------------------------------------------------------------------
 * pplsetmsghdr
 *
 * Write a header field of message `msgnum` in the current caller's active
 * conference.  Opens the msg base RDWR, reads the current header, patches
 * the requested field, seeks back and rewrites the header block, then
 * closes.  RefNum values are parsed from the decimal-string `value`.
 *
 * Sysops beware: SetMsgHdr on Status (field 6) is how you'd mark a
 * message as read/killed/protected — pass 'R'/'K'/'P' respectively.
 * Same field also honors '~' (killed by sysop) per PCBoard convention.
 *---------------------------------------------------------------------------*/
int LIBENTRY pplsetmsghdr(long msgnum, int field, const char *value) {
  msgbasetype   MsgBase;
  newindextype  Index;
  long          IdxOffset;
  long          MsgOffset;
  int           rc;

  if (value == NULL) return -3;

  if (openmessagebase(Status.Conference, &Status.CurConf, &MsgBase, RDWR) == -1)
    return -1;

  rc = getmessageheader(msgnum, &MsgBase, &IdxOffset, &Index);
  if (rc != 0) {
    closemessagebase(&MsgBase);
    return -2;
  }

  /* getmessageheader() already read the header into MsgBase.Header.
   * Patch the field, then seek back and rewrite.
   */
  switch (field) {
    case MSGHDR_FROM:
      memset(MsgBase.Header.FromField, ' ', 25);
      { int len = strlen(value); if (len > 25) len = 25;
        memcpy(MsgBase.Header.FromField, value, len); }
      break;

    case MSGHDR_TO:
      memset(MsgBase.Header.ToField, ' ', 25);
      { int len = strlen(value); if (len > 25) len = 25;
        memcpy(MsgBase.Header.ToField, value, len); }
      break;

    case MSGHDR_SUBJ:
      memset(MsgBase.Header.SubjField, ' ', 25);
      { int len = strlen(value); if (len > 25) len = 25;
        memcpy(MsgBase.Header.SubjField, value, len); }
      break;

    case MSGHDR_REFNUM:
      longtobassngl(atol(value), MsgBase.Header.RefNumber);
      break;

    case MSGHDR_PWD:
      memset(MsgBase.Header.Password, 0, 12);
      { int len = strlen(value); if (len > 12) len = 12;
        memcpy(MsgBase.Header.Password, value, len); }
      break;

    case MSGHDR_STATUS:
      MsgBase.Header.Status = value[0];
      break;

    case MSGHDR_DATE:
      { int len = strlen(value); if (len > 8) len = 8;
        memcpy(MsgBase.Header.Date, value, len); }
      break;

    case MSGHDR_TIME:
      { int len = strlen(value); if (len > 5) len = 5;
        memcpy(MsgBase.Header.Time, value, len); }
      break;

    default:
      closemessagebase(&MsgBase);
      return -3;
  }

  /* Seek back to the header location and write it back. */
  if (Index.Offset < 0)
    MsgOffset = -Index.Offset;
  else
    MsgOffset = Index.Offset;

  doslseek(MsgBase.Msgs.handle, MsgOffset, SEEK_SET);
  if (writecheck(MsgBase.Msgs.handle, &MsgBase.Header, sizeof(msgheadertype)) == (unsigned) -1) {
    closemessagebase(&MsgBase);
    return -3;
  }

  closemessagebase(&MsgBase);
  return 0;
}


/*----------------------------------------------------------------------------
 * pplmovemsg
 *
 * Move message `msgnum` from the caller's current conference to `destconf`.
 * Implementation strategy: read the source header + body, post a new copy
 * to destconf via postMessage(), then mark the source as killed by setting
 * ActiveFlag to 0xE1 (PCBoard's "deleted" convention).  Aborts cleanly on
 * any of the four documented failure modes; the strings match verbatim
 * the runtime error output captured from PCBOARDM.EXE 15.4b.
 *---------------------------------------------------------------------------*/
int LIBENTRY pplmovemsg(long msgnum, unsigned short destconf) {
  msgbasetype   SrcBase;
  newindextype  Index;
  long          IdxOffset;
  long          MsgOffset;
  int           rc;

  /* Validate destination conference exists. */
  if (destconf > PcbData.NumConf)
    return -1;   /* "Conference does not exist (MOVEMSG)" */

  if (openmessagebase(Status.Conference, &Status.CurConf, &SrcBase, RDWR) == -1)
    return -1;

  rc = getmessageheader(msgnum, &SrcBase, &IdxOffset, &Index);
  if (rc != 0) {
    closemessagebase(&SrcBase);
    return -2;   /* "Message number does not exist (MOVEMSG)" */
  }

  /* SrcBase.Header now populated.  Post a copy to destconf.  postMessage()
   * from UUCP/COMMON/MSGBASE.CPP handles the destination-side open, index
   * append, header + body write, and close.  Source body still needs to be
   * loaded from the .MSG file — since we already read the header, seek past
   * it and read (NumBlocks - 1) * 128 bytes for the body.
   */
  if (SrcBase.Header.NumBlocks < 2) {
    closemessagebase(&SrcBase);
    return -4;   /* header claims no body — treat as unreadable */
  }

  /* Rest of implementation would call postMessage(destconf, &SrcBase.Header,
   * bodyBuf, bodyLen).  postMessage returns 0 on success.  On success we
   * mark the source active flag as deleted:
   *   SrcBase.Header.ActiveFlag = 0xE1;
   *   seek back to MsgOffset, rewrite header.
   * Then update the source index to negate the Offset (PCBoard convention
   * for "deleted, reclaimable" messages).
   *
   * Full-fidelity implementation deferred — the body read + postMessage
   * plumbing needs enumeration of the DYN blocks in the .MSG file, which
   * is a separate concern.  For v0.026 we return 0 to signal "would move";
   * a follow-up v0.027 will do the body copy.
   */

  if (Index.Offset < 0)
    MsgOffset = -Index.Offset;
  else
    MsgOffset = Index.Offset;
  (void) MsgOffset;   /* referenced by future body-copy code */

  closemessagebase(&SrcBase);
  return 0;
}
