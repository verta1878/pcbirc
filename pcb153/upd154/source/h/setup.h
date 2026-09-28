/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#define H_SETUP

#include <types.hpp>

#ifndef H_SCREEN
#include <screen.h>
#endif

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))

#define PROGRAM    "pcbsm"

#define RAND 0
#define TEXT 1

#define NUMSYSOP       5
#define NUMSYSOPFUN   15
#define NUMSYSOPCOM   15
#define NUMSYSTFILES  16
#define NUMCONFFILES  13
#define NUMDISPFILES   9
#define NUMQUESFILES   6
#define NUMMODEM      12
#define NUMMODEMSWI   17
#define NUMMODEMACC    6
#define NUMNODE       11
#define NUMEVENT       6
#define NUMSUB         4
#define NUMMSGOPT     17
#define NUMLOGOPT      3
#define NUMSYSOPT     19
#define NUMCTRLOPT    18
#define NUMFUNCKEYS   10
#define NUMLIMITS      9
#define NUMOS2OPT     20
#define NUMCOLORDEFS   7
#define NUMXFERS      11
#define NUMLEVELS     34
#ifdef FIDO
#define NUMFIDOCFG    12
#define NUMFIDOTOSSER 14
#define NUMFIDOLOC     9
#define NUMFIDOAREA    4
#define NUMFIDOEMSI    6
#define NUMFIDOFREQ    6
#endif
#define NUMCONFRNCE   27
#define NUMADDCONF    27
#define NUMACCTDEF    13
#define NUMACCTRATES  15
#define NUMUUCP       18

#define FLDSYSOP       0
#define FLDSYSOPFUN    (FLDSYSOP     + NUMSYSOP)
#define FLDSYSOPCOM    (FLDSYSOPFUN  + NUMSYSOPFUN)
#define FLDSYSTFILES   (FLDSYSOPCOM  + NUMSYSOPCOM)
#define FLDCONFFILES   (FLDSYSTFILES + NUMSYSTFILES)
#define FLDDISPFILES   (FLDCONFFILES + NUMCONFFILES)
#define FLDQUESFILES   (FLDDISPFILES + NUMDISPFILES)
#define FLDMODEM       (FLDQUESFILES + NUMQUESFILES)
#define FLDMODEMSWI    (FLDMODEM     + NUMMODEM)
#define FLDMODEMACC    (FLDMODEMSWI  + NUMMODEMSWI)
#define FLDNODE        (FLDMODEMACC  + NUMMODEMACC)
#define FLDEVENT       (FLDNODE      + NUMNODE)
#define FLDSUB         (FLDEVENT     + NUMEVENT)
#define FLDMSGOPT      (FLDSUB       + NUMSUB)
#define FLDLOGOPT      (FLDMSGOPT    + NUMMSGOPT)
#define FLDSYSOPT      (FLDLOGOPT    + NUMLOGOPT)
#define FLDCTRLOPT     (FLDSYSOPT    + NUMSYSOPT)
#define FLDFUNCKEYS    (FLDCTRLOPT   + NUMCTRLOPT)
#define FLDLIMITS      (FLDFUNCKEYS  + NUMFUNCKEYS)
#define FLDOS2OPT      (FLDLIMITS    + NUMLIMITS)
#define FLDCOLORS      (FLDOS2OPT    + NUMOS2OPT)
#define FLDXFERS       (FLDCOLORS    + NUMCOLORDEFS)
#define FLDLEVELS      (FLDXFERS     + NUMXFERS)
#ifdef FIDO
#define FLDFIDOCFG     (FLDLEVELS    + NUMLEVELS)
#define FLDFIDOTOSSER  (FLDFIDOCFG   + NUMFIDOCFG)
#define FLDFIDOLOC     (FLDFIDOTOSSER+ NUMFIDOTOSSER)
#define FLDFIDOEMSI    (FLDFIDOLOC   + NUMFIDOLOC)
#define FLDFIDOFREQ    (FLDFIDOEMSI  + NUMFIDOEMSI)
#define FLDCONFRNCE    (FLDFIDOFREQ  + NUMFIDOFREQ)
#else
#define FLDCONFRNCE    (FLDLEVELS    + NUMLEVELS)
#endif
#define FLDADDCONF     (FLDCONFRNCE  + NUMCONFRNCE)
#define FLDACCTDEF     (FLDADDCONF   + NUMADDCONF)
#define FLDACCTRATES   (FLDACCTDEF   + NUMACCTDEF)
#define FLDUUCP        (FLDACCTRATES + NUMACCTRATES)
#define FLDTOTAL       (FLDUUCP      + NUMUUCP)

typedef enum {EDIT_NONE=0,EDIT_CHECK7=1,EDIT_CHECK8=2,EDIT_GRAPHICS=4} edittype;

void pascal init(void);
void pascal sysop(void);

void pascal systemfiles(void);
void pascal configurationfiles(void);
void pascal displayfiles(void);
void pascal questionnairefiles(void);

#ifdef FIDO
void pascal fidosetup(void);
void pascal fidotosser(void);
#endif

void pascal functionkeys(void);
void pascal modem(void);
void pascal modemaccess(void);
void pascal modemswitches(void);
void pascal account(void);
void pascal uucp(void);
void pascal node(void);
void pascal event(void);
void pascal subscription(void);
void pascal loggingoptions(void);
void pascal messageoptions(void);
void pascal controloptions(void);
void pascal systemoptions(void);
void pascal transferoptions(void);
void pascal os2options(void);
void pascal limits(void);
void pascal colors(void);
void pascal sysopfunctions(void);
void pascal sysopcommands(void);
void pascal levels(void);
void pascal mainbrd(void);
void pascal conferences(void);
void pascal conffunc(void);
int  pascal checktime(char *Str, int HelpNum);
void pascal editdirlist(char *Name);
void pascal editcmd(char *Name);
void pascal editscript(char *Name);
void pascal editfsec(int Select);
void pascal editblt(char *Name, char *FldTitle);
void pascal editdoor(char *Name);
char pascal editexit(char *Str);
void pascal editpath(char *Name, int FileNum);
void pascal editprot(void);
void pascal editpwrd(void);
void pascal edittcan(void);
void pascal editlang(void);
void pascal editfiletcan(void);
void pascal editmodfile(void);

int  pascal externaledit(int Edit);
bool pascal notokaytoedit(char *Name, savescrntype *ScrnBuf, char *NumPtr, int *FlagPtr, int NumKeys);
void pascal editrestore(savescrntype *ScrnBuf, char *NumPtr, int *FlagPtr, int NumKeys, void *List);
void pascal editor(char *FileName, bool Graphics);

void pascal searchandreplace(void);

#ifdef __cplusplus
extern "C" {
#endif
char * pascal getcontext(void *Ptr);
#ifdef __cplusplus
}
#endif

void pascal showeditmsg(int Before, char *Msg);
void pascal showeditmsg2(int Before, char *Msg);
int  pascal numrandrecords(char *FileName, int RecSize);
int  pascal numtextrecords(char *FileName);

void pascal findfilename(int *Len, int *ExtLen);
int backslash(int Before);
int checkfiles6(int Before);
int checkfiles7(int Before);
int checkfiles8(int Before);
int checkfiles8n(int Before);
int checkseclevel(int Before);

int checkspeed(int Before);
int jumpcmdfile(int Before);

int pascal datevalid(char *Date);

void pascal editeventlist(char *Name);
void pascal editdays(char *Name);
void pascal editverblist(char *Name);
int  pascal fileselect(int X, int Y, int NumItems, char List[64][20], int Width, int Heighth);

void pascal shelltoprogram(char *Program, char *Param);

void pascal pack_config_file(void);
