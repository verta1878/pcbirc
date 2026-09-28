/* pcbsm_externs.h — Watcom extern declarations for PCBSM
 * These duplicate externs from PCBSM/USERS.H which Watcom can't include
 * due to include path resolution issues on Linux (case sensitivity).
 */
#ifdef __WATCOMC__
/* From USERS.C */
extern char *QwkConfFlags;
extern char *ConfReg;
extern long *MsgReadPtr;
/* From USERS.H */
extern char Alias[26];
extern char AliasSupport;
extern char TempFileName[40];
extern char TempInfFileName[40];
extern char BackFileName[40];
extern char BackInfFileName[40];
extern int TempFile;
extern int BackFile;
extern int BackInfFile;
extern int (*VMSeqFinalPass)(void *);
extern void *Verify;
extern char Colors[];
#endif
