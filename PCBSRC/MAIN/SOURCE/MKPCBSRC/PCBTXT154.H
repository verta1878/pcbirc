/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* pcbtext_154.h                                                             */
/*                                                                           */
/* Default PCBTEXT record table for PCBoard 15.4b.  Extracted from the       */
/* data section of MKPCBTXT.EXE (1997-04-15) by extract_pcbtext.py.          */
/*                                                                           */
/* IMPORTANT: this file preserves the STRING CONTENT recovered from the      */
/* original binary.  The exact TXT_ numeric assignment for records past      */
/* index 713 (the 15.3 baseline) is inferred from context — verify against   */
/* your specific runtime before shipping to users.                           */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef H_PCBTEXT_154
#define H_PCBTEXT_154

typedef struct {
    unsigned char  color;
    const char    *text;
} pcbtext_default_t;

#define TXT_NUMPROMPTS_154   50

static const pcbtext_default_t Pcbtext154Table[] = {
    { 0x00, "" },  /* record 0 — MKPCBTXT rewrites with version stamp */
    /* rec    1, src 0x07d7e */  { 0x00, "PCBoard version 14.5 & 15.0 & 15.2 & 15.3 & 15.4 PCBTEXT file" },
    /* rec    2, src 0x07dbc */  { 0x00, "Leave a comment for the sysop (Enter)=no" },
    /* rec    3, src 0x07de5 */  { 0x00, " Brand of CPU you are using" },
    /* rec    4, src 0x07e01 */  { 0x00, "Access Denied - Upcoming Event Pending ..." },
    /* rec    5, src 0x07e2c */  { 0x00, "Time Limit Exceeded." },
    /* rec    6, src 0x07e41 */  { 0x00, "Access Denied - Unauthorized name match on @USER@!" },
    /* rec    7, src 0x07e74 */  { 0x00, "Access Denied - You are Locked Out of this System!" },
    /* rec    8, src 0x07ea7 */  { 0x00, "Access Denied - Excessive Password Failures!" },
    /* rec    9, src 0x07ed4 */  { 0x00, "Access Denied - Refused to Complete Registration!" },
    /* rec   10, src 0x07f06 */  { 0x00, "Access Denied - Remote DOS Password Failure!" },
    /* rec   11, src 0x07f33 */  { 0x00, "Access Denied - @USER@ is in use on another Node!" },
    /* rec   12, src 0x07f65 */  { 0x00, "Sorry, @FIRST@, this is a `Closed' Board ..." },
    /* rec   13, src 0x07f92 */  { 0x00, "Security Violation(s) - Disconnecting ..." },
    /* rec   14, src 0x07fbc */  { 0x00, "There is presently no room for your text, @FIRST@ ..." },
    /* rec   15, src 0x07ff2 */  { 0x00, "A message to `ALL' cannot be protected, @FIRST@ ..." },
    /* rec   16, src 0x08026 */  { 0x00, "Callers must know p/w to read message!  Continue (Enter)=no" },
    /* rec   17, src 0x08062 */  { 0x00, "Protocol Unavailable - Modem Type Wrong!" },
    /* rec   18, src 0x0808b */  { 0x00, "Local Download Path" },
    /* rec   19, src 0x0809f */  { 0x00, "Password to Read Message (ENTER alone aborts)" },
    /* rec   20, src 0x080cd */  { 0x00, "Local Upload Path or Filename" },
    /* rec   21, src 0x080eb */  { 0x00, "Printer Off-Line ..." },
    /* rec   22, src 0x08100 */  { 0x00, "Enter the Security password desired" },
    /* rec   23, src 0x08124 */  { 0x00, "Reply to Msgs: (#), (#), (Enter)=abort" },
    /* rec   24, src 0x0816f */  { 0x00, "Selection not available in local mode." },
    /* rec   25, src 0x08196 */  { 0x00, "Temporary Sysop privileges removed ..." },
    /* rec   26, src 0x081bd */  { 0x00, "Temporary Sysop privileges granted this call only!" },
    /* rec   27, src 0x081f0 */  { 0x00, "Graphics mode is not available, @FIRST@ ..." },
    /* rec   28, src 0x0821c */  { 0x00, "Automatic Lockout completed ..." },
    /* rec   29, src 0x0823c */  { 0x00, "!|*|Y01000400|c0A|@0005RIPscrip |Y01000200|c0B|@3W0Fmode on|g0006" },
    /* rec   30, src 0x0827e */  { 0x00, "Text Entry is Full ..." },
    /* rec   31, src 0x08295 */  { 0x00, "ASCII transfer is not available on a binary file!" },
    /* rec   32, src 0x082c7 */  { 0x00, "NOTE: Transfer must end with a (Ctrl-Z)!" },
    /* rec   33, src 0x082f5 */  { 0x00, "NO CONNECT / @OPTEXT@" },
    /* rec   34, src 0x0830b */  { 0x00, "FULL COUNTDOWN" },
    /* rec   35, src 0x0831a */  { 0x00, "NO CARRIER" },
    /* rec   36, src 0x08325 */  { 0x00, "Disk Full - System presently unavailable!" },
    /* rec   37, src 0x0834f */  { 0x00, "ALL NODES ARE BUSY AT THIS TIME PLEASE TRY LATER" },
    /* rec   38, src 0x08384 */  { 0x00, "Uploading text file directly into the message editor..." },
    /* rec   39, src 0x083bc */  { 0x00, "~(Echo)" },
    /* rec   40, src 0x083d0 */  { 0x00, "Error reading Script Questionnaire" },
    /* rec   41, src 0x083f3 */  { 0x00, "Error reading Message base in use!" },
    /* rec   42, src 0x08416 */  { 0x00, "Unable to exit to DOS - no REMOTE.SYS file found!" },
    /* rec   43, src 0x08448 */  { 0x00, "No record available to update!" },
    /* rec   44, src 0x08467 */  { 0x00, "Messages Successfully Packed & Purified!" },
    /* rec   45, src 0x08490 */  { 0x00, "Enter `handle' to be used for Node Chat" },
    /* rec   46, src 0x084b8 */  { 0x00, "File Attachment: @OPTEXT@" },
    /* rec   47, src 0x084d2 */  { 0x00, "Enter a longer description of the file please!" },
    /* rec   48, src 0x08501 */  { 0x00, "No Script Questionnaires are available, @FIRST@ ..." },
    /* rec   49, src 0x08535 */  { 0x00, "(R) to re-enter your name or (C) to continue logon as a new user" },
    /* rec   50, src 0x08576 */  { 0x00, "Would you like to register with us, @FIRST@? (Enter)=yes" },
};

#endif  /* H_PCBTEXT_154 */
