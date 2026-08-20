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


#include <dir.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include <dosfunc.h>
#include <country.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#include "rules.h"

#ifdef DEBUG
#define  MC_NO_XFER_SIZE
#include <memcheck.h>
#endif

enum { F2=FLAG1, F3, F4, F5, F6, F7, F8, F9, ALT5, ALTA, ALTB, ALTD, ALTF,
       ALTG, ALTI, ALTJ, ALTL, ALTM, ALTN, ALTO, ALTP, ALTR, ALTS, ALTT, ALTU,
       ALTV, ALTX, ALTY, ALTZ, HOME
};

enum {NOVICE, EXPERT, TEXTDUPE};

typedef enum {
  SHOWALL, NAMEMATCHES, TEXTMATCHES, RULEMATCHES, KEEPMATCHES, MOVEMATCHES,
  DELETEMATCHES, ONDISKMATCHES, INDIRMATCHES, SELECTEDMATCHES,
  NONSELECTEDMATCHES, TOTALMATCHES
} filtertype;

const int MAXLINES = 43;
const int EXPCONF  =  3;
const int EXPDIR   =  8;
const int EXPNAME  = 13;
const int EXPDATE  = 26;
const int NOVNAME  = 10;
const int NOVDATE  = 24;
const int BOTHDESC = 34;

const int ROLLUPSIZE = 4096;

bool ForcedUpdate;

/*****************************************************************************/

typedef struct {
  LineType   Line;
  long       Pos;
  union {
  parenttype Parent;
  childtype  Child;
  };
} scrnlintype;

typedef struct {
  LineType   Line;
  int        LastPos;
  bool       MoveDown;
} cursortype;

static cursortype Cursor;

/*****************************************************************************/

typedef struct {
  VMDataSet *pParentsOnly;    // points to ParentsOnly or ParentsOnlyMatches
  VMDataSet *pCombined;       // points to Combined or CombinedMatches
  long      *pTotalLines;     // points to TotalLines or TotalMatches
  long      *pTotalParents;   // points to TotalParents or TotalMatchingParents
} virtualsettype;

static virtualsettype  AllFiles;
static virtualsettype  Matches;
static virtualsettype *Current;

/*****************************************************************************/

// unsigned LoScanNum;   // used for scanning a range of entries to see if they
// unsigned HiScanNum;   // should be marked as dupes before processing

static int NumLines;     // Current number of lines on screen
static int BottomLine;   // Line number where file size is shown

static unsigned CurCnf;
static unsigned CurDir;

// ScrnRecs pre-loads and holds a screenful of records at a time
static scrnlintype ScrnRecs[MAXLINES];
static bool        ShowParentsOnly;
static long        NumTagged;
static long        SaveParentPos;
static long        SaveChildPos;
static long        ParentRecNum;
static long        CombinedRecNum;
static long        TotalMatches;
static long        TotalMatchingParents;
static char        SrchForm[13];
static char        SearchInput[256];
static filtertype  Filter;
static char       *DPath;
static char       *PathList;

static VMDataSet   ParentsOnlyMatches;
static VMDataSet   CombinedMatches;
static VMDataSet   Rules;
static long        TotalRules;

/******************************************************************************
 The following lines are used to create the Move Window
******************************************************************************/
static int checkconfrange(int Before);  /* prototype */
static int checkdirrange(int Before);   /* prototype */
static int checkoffline(int Before);    /* prototype */

#define NUMFIELDS 5

typedef struct {
  unsigned Cnf;
  unsigned Dir;
  char     CnfName[61];
  char     DirDesc[36];
  bool     ShowOnScreen;
} newloctype;


static newloctype New;

static unsigned TempCnf;
static unsigned TempDir;
static int      NewDays;
static char     MoveFlag;
static bool     Offline;
static bool     ExpertMode;
static char     ConfStr[6];


static char *MatchStrs[TOTALMATCHES] = {
  "",
  "Filname",
  "Description",
  "Rules",
  "Unmarked",
  "Move/Copy",
  "Deleted",
  "On-Disk-Only",
  "In-DIR-Only",
  "Non-Selected",
};


static FldType MoveQuestions[NUMFIELDS] = {
 { vUNSIGNED,ALLNUM ,FEDIT+0       ,10,16, 5,CLEAR     ,"Conference Number ","",&TempCnf ,checkconfrange },
 { vUNSIGNED,ALLNUM ,FEDIT+1       ,10,17, 5,CLEAR     ,"Directory Number  ","",&TempDir ,checkdirrange  },
 { vINT     ,ALLNUM ,FILEMOVEMENT+0,10,18, 5,CLEAR     ,"Days After Upload ","",&NewDays ,NULL           },
 { vCHAR    ,ALLCHAR,FILEMOVEMENT+5,10,19, 1,NOCLEARFLD,"Move or Copy (M/C)","",&MoveFlag,NULL           },
 { vBOOL    ,YESNO  ,FILEMOVEMENT+6,10,20, 1,NOCLEARFLD,"Move File Offline ","",&Offline ,checkoffline   }
};

static FldType ProtectedQuestions[NUMFIELDS-1] = {
 { vUNSIGNED,ALLNUM ,FEDIT+1       ,10,17, 5,CLEAR     ,"Directory Number  ","",&TempDir ,checkdirrange  },
 { vINT     ,ALLNUM ,FILEMOVEMENT+0,10,18, 5,CLEAR     ,"Days After Upload ","",&NewDays ,NULL           },
 { vCHAR    ,ALLCHAR,FILEMOVEMENT+5,10,19, 1,NOCLEARFLD,"Move or Copy (M/C)","",&MoveFlag,NULL           },
 { vBOOL    ,YESNO  ,FILEMOVEMENT+6,10,20, 1,NOCLEARFLD,"Move File Offline ","",&Offline ,checkoffline   }
};

/*****************************************************************************/


static void near pascal showloc(void) {
  char Str[36];

  if (New.ShowOnScreen) {
    clsbox(38,16,73,17,Colors[DISPLAY]);
    if (! Protected) {
      maxstrcpy(Str,New.CnfName,sizeof(Str));
      fastprint(38,16,Str,Colors[DISPLAY]);
    }
    fastprint(38,17,New.DirDesc,Colors[DISPLAY]);
  }
}


static void near pascal setnewloc(unsigned NewCnf, unsigned NewDir) {
  DirListType Rec;
  pcbconftype Conf;

  TempCnf = NewCnf;
  TempDir = NewDir;

  if (New.Cnf != NewCnf) {
    New.Cnf = NewCnf;
    New.Dir = 0xFFFF;  // force it to re-read the DIR.LST file
    if (New.Cnf == 0xFFFF) {
      strcpy(New.CnfName,"Offline Directory");
    } else {
      getconfrecord(NewCnf,&Conf);
      strcpy(New.CnfName,Conf.Name);
    }
  }

  if (New.Dir != NewDir) {
    New.Dir = NewDir;

    if (finddirpath(NewCnf,NewDir,&Rec) == -1)
      New.DirDesc[0] = 0;
    else
      strcpy(New.DirDesc,Rec.DirDesc);
  }

  showloc();
}


static void near pascal showfilter(void) {
  char Str[40];

  clsbox(1,1,28,1,Colors[HEADING]);

  if (Filter != SHOWALL) {
    sprintf(Str,"%s Filter Enabled",MatchStrs[Filter]);
    fastprint(1,1,Str,Colors[HEADING] + 0x80);
  }
}


static void near pascal resetfilter(void) {
  Filter = SHOWALL;
  Current = &AllFiles;
  clsbox(1,1,28,1,Colors[HEADING]);
  if (TotalMatches != 0) {
    VMDone(&CombinedMatches);
    VMDone(&ParentsOnlyMatches);
    TotalMatches = 0;
    TotalMatchingParents = 0;
  }
}


static void near pascal updaterecord(scrnlintype *s) {
  if (s->Line == DIRLINE || s->Line == TEXTLINE)
    VMWrite(&Parents,&s->Parent,s->Pos,sizeof(s->Parent));
  else
    VMWrite(&Children,&s->Child,s->Pos,sizeof(s->Child));
}



static void near pascal updatecombined(long RecNum, LineType Line) {
  combinedtype *combined;

  combined = (combinedtype *) VMRecordGetByIndex(Current->pCombined,RecNum,NULL);
  combined->Line = Line;
  VMRecordChanged(Current->pCombined);
}



static long near pascal findposinparentsonly(virtualsettype *Set, long Pos) {
  long             X;
  parentsonlytype *p;

  for (X = 1; X <= *Set->pTotalParents; X++) {
    p = (parentsonlytype *) VMRecordGetByIndex(Set->pParentsOnly,X,NULL);
    if (p->Pos == Pos)
      return(X);
  }
  return(-1);
}


static void near pascal insertinparentsonly(virtualsettype *Set, long CurPos, long NewPos) {
  long             X;
  parentsonlytype *p;
  parentsonlytype  pNew;
  parentsonlytype  pSave;

  pNew.Pos = NewPos;

  if (CurPos != VM_INVALID_POS) {
    X = findposinparentsonly(Set,CurPos);
    for (X++; X <= *Set->pTotalParents; X++) {
      p = (parentsonlytype *) VMRecordGetByIndex(Set->pParentsOnly,X,NULL);
      pSave = *p;
      *p = pNew;
      pNew = pSave;
      VMRecordChanged(Set->pParentsOnly);
    }
  }

  (*Set->pTotalParents)++;
  if (*Set->pTotalParents > VMRecordCount(Set->pParentsOnly))
    p = (parentsonlytype *) VMRecordCreate(Set->pParentsOnly,sizeof(parentsonlytype),NULL,NULL);
  else
    p = (parentsonlytype *) VMRecordGetByIndex(Set->pParentsOnly,*Set->pTotalParents,NULL);

  *p = pNew;
  VMRecordChanged(Set->pParentsOnly);
}


static long near pascal findposincombined(virtualsettype *Set, long Pos, LineType Line) {
  long          X;
  combinedtype *c;

  for (X = 1; X <= *Set->pTotalLines; X++) {
    c = (combinedtype *) VMRecordGetByIndex(Set->pCombined,X,NULL);
    if (c->Pos == Pos && c->Line == Line)
      return(X);
  }
  return(-1);
}


static void near pascal insertincombined(virtualsettype *Set, long CurPos, long NewPos, LineType CurLine, LineType NewLine) {
  long          X;
  long          Y;
  combinedtype *p;
  combinedtype  pNew;
  combinedtype  pSave;

  pNew.Pos  = NewPos;
  pNew.Line = NewLine;

  if (CurPos != VM_INVALID_POS) {
    Y = findposincombined(Set,CurPos,CurLine) + 1;
    if (NewLine == DIRLINE) {
      for (; Y <= *Set->pTotalLines; Y++) {
        p = (combinedtype *) VMRecordGetByIndex(Set->pCombined,Y,NULL);
        if (p->Line != DUPELINE)
          break;
      }
    }


    for (X = Y; X <= *Set->pTotalLines; X++) {
      p = (combinedtype *) VMRecordGetByIndex(Set->pCombined,X,NULL);
      pSave = *p;
      *p = pNew;
      pNew = pSave;
      VMRecordChanged(Set->pCombined);
    }
  }

  (*Set->pTotalLines)++;
  if (*Set->pTotalLines > VMRecordCount(Set->pCombined))
    p = (combinedtype *) VMRecordCreate(Set->pCombined,sizeof(combinedtype),NULL,NULL);
  else
    p = (combinedtype *) VMRecordGetByIndex(Set->pCombined,*Set->pTotalLines,NULL);

  *p = pNew;
  VMRecordChanged(Set->pCombined);
}


static void near pascal deletefromparentsonly(virtualsettype *Set, long Pos) {
  long             X;
  long             CurRec;
  parentsonlytype *p;
  parentsonlytype  pNew;
  parentsonlytype  pSave;

  CurRec = findposinparentsonly(Set,Pos);
  memset(&pNew,0,sizeof(pNew));

  for (X = *Set->pTotalParents; X >= CurRec; X--) {
    p = (parentsonlytype *) VMRecordGetByIndex(Set->pParentsOnly,X,NULL);
    pSave = *p;
    *p = pNew;
    pNew = pSave;
    VMRecordChanged(Set->pParentsOnly);
  }

  (*Set->pTotalParents)--;
}


static void near pascal deletefromcombined(virtualsettype *Set, long Pos, LineType Line) {
  long          X;
  long          CurRec;
  combinedtype *p;
  combinedtype  pNew;
  combinedtype  pSave;

  CurRec = findposincombined(Set,Pos,Line);
  memset(&pNew,0,sizeof(pNew));

  for (X = *Set->pTotalLines; X >= CurRec; X--) {
    p = (combinedtype *) VMRecordGetByIndex(Set->pCombined,X,NULL);
    pSave = *p;
    *p = pNew;
    pNew = pSave;
    VMRecordChanged(Set->pCombined);
  }

  (*Set->pTotalLines)--;
}


/********************************************************************
*
*  Function: deleteline()
*
*  Desc    :
*
*/

static void near pascal deleteline(scrnlintype *s) {
  long        Pos;
  long        PrevPos;
  long        NextPos;
  long        ParentPos;
  childtype  *c;
  parenttype *p;

  if (s->Line == DUPELINE) {
    // This child may have older or younger siblings, check for the next
    // and previous siblings and adjust their NextChildPos and PrevChildPos
    // values in order to remove this child from the chain
    c = (childtype *) VMRecordGetByPos(&Children,s->Pos);
    NextPos   = c->NextChildPos;
    PrevPos   = c->PrevChildPos;
    ParentPos = c->ParentPos;

    // if this child wasn't the last, the point then next child's previous
    // pointer to this child's previous sibling
    if (NextPos != VM_INVALID_POS) {
      c = (childtype *) VMRecordGetByPos(&Children,NextPos);
      c->PrevChildPos = PrevPos;
      VMRecordChanged(&Children);
    }

    // if the PrevChildPos is invalid, then this was the first child, so
    // set the parent's FirstChildPos equal to the NextChildPos
    if (PrevPos == VM_INVALID_POS) {
      p = (parenttype *) VMRecordGetByPos(&Parents,ParentPos);
      p->Fields.FirstChildPos = NextPos;
      VMRecordChanged(&Parents);
    } else {
      // otherwise, update the previous child's NextChildPos to take this
      // child out of the chain
      c = (childtype *) VMRecordGetByPos(&Children,PrevPos);
      c->NextChildPos = NextPos;
      VMRecordChanged(&Children);
    }

    // Now delete the child from the combined index
    deletefromcombined(&AllFiles,s->Pos,DUPELINE);
    if (Filter != SHOWALL)
      deletefromcombined(&Matches,s->Pos,DUPELINE);
  } else {
    deletefromparentsonly(&AllFiles,s->Pos);
    deletefromcombined(&AllFiles,s->Pos,s->Line);
    if (Filter != SHOWALL) {
      deletefromparentsonly(&Matches,s->Pos);
      deletefromcombined(&Matches,s->Pos,s->Line);
    }
    // if we deleted a parent that has children, then kill the children too
    if (s->Line == DIRLINE) {
      Pos = s->Parent.Fields.FirstChildPos;
      while (Pos != VM_INVALID_POS) {
        c = (childtype *) VMRecordGetByPos(&Children,Pos);
        NextPos = c->NextChildPos;
        deletefromcombined(&AllFiles,Pos,DUPELINE);
        if (Filter != SHOWALL)
          deletefromcombined(&Matches,Pos,DUPELINE);
        Pos = NextPos;
      }
    }
  }
}



static void near pascal wordwrap(scrnlintype *s, char *Str) {
  char *p;
  char *Source;

  switch (s->Line) {
    case DIRLINE :  Source = s->Parent.Fields.FDesc;
                    padstr(Source,' ',sizeof(s->Parent.Fields.FDesc)-1);
                    break;
    case TEXTLINE:  Source = s->Parent.Str;
                    padstr(Source,' ',sizeof(s->Parent.Str)-1);
                    break;
    case DUPELINE:  Source = s->Child.Str;
                    if ((p = strchr(Source,'|')) != NULL) {
                      Source = p+2;  // point to first char after |+space
                      padstr(Source,' ',sizeof(s->Child.Str)-1 - (int) (Source - s->Child.Str));
                    } else
                      padstr(Source,' ',sizeof(s->Child.Str)-1);
                    break;
    default      :  return;
  }

  addchar(Str,' ');

  if (Input.Ch != ' ') {
    p = strrchr(Source,' ');
    if (p != NULL) {
      *p = 0;
      strcat(Str,p+1);
    }
    addchar(Str,Input.Ch);
  }

  if (Cursor.MoveDown) {
    Cursor.LastPos = strlen(Str);
    Cursor.Line = DUPELINE;
  }
}


static char * near pascal lastspace(char *Str, int LastColumn) {
  int X;
  for (X = LastColumn, Str += LastColumn; *Str != ' ' && X; X--, Str--);
  if (X == 0)
    return(NULL);
  return(Str);
}


static bool near pascal joinline(scrnlintype *s) {
  LineType      SourceLine;
  unsigned      Len;
  unsigned      MaxLen;
  char         *p;
  char         *Target;
  char         *Source;
  parenttype   *parent;
  childtype    *child;
  combinedtype *combined;
  long          Pos;
  long          NextRec;

  switch (s->Line) {
    case DIRLINE :  if ((Pos = s->Parent.Fields.FirstChildPos) == VM_INVALID_POS) {
                      beep();
                      return(FALSE);
                    }

                    Target = s->Parent.Fields.FDesc;
                    MaxLen = sizeof(s->Parent.Fields.FDesc)-1;
                    stripright(Target,' ');
                    Len = strlen(Target);

                    SourceLine = DUPELINE;
                    child  = (childtype *) VMRecordGetByPos(&Children,Pos);
                    Source = child->Str;

                    if ((p = strchr(Source,'|')) != NULL) {
                      p++;
                      if (*p == ' ')
                        p++;
                      Source = p;
                    }
                    break;

    case TEXTLINE:  Target = s->Parent.Str;
                    MaxLen = sizeof(s->Parent.Str)-1;
                    stripright(Target,' ');
                    Len  = strlen(Target);

                    NextRec = findposincombined(Current,s->Pos,TEXTLINE) + 1;
                    if (NextRec > *Current->pTotalLines) {
                      beep();
                      return(FALSE);
                    }

                    combined = (combinedtype *) VMRecordGetByIndex(Current->pCombined,NextRec,NULL);
                    if (combined->Line != TEXTLINE) {
                      beep();
                      return(FALSE);
                    }

                    SourceLine = TEXTLINE;
                    Pos = combined->Pos;
                    parent = (parenttype *) VMRecordGetByPos(&Parents,Pos);
                    Source = parent->Str;
                    break;

    case DUPELINE:  if ((Pos = s->Child.NextChildPos) == VM_INVALID_POS) {
                      beep();
                      return(FALSE);
                    }

                    Target = s->Child.Str;
                    MaxLen = sizeof(s->Child.Str)-1;
                    stripright(Target,' ');
                    Len  = strlen(Target);
                    if ((p = strchr(Target,'|')) != NULL)
                      Target = p+1;

                    SourceLine = DUPELINE;
                    child  = (childtype *) VMRecordGetByPos(&Children,Pos);
                    Source = child->Str;

                    if ((p = strchr(Source,'|')) != NULL) {
                      p++;
                      if (*p == ' ')
                        p++;
                      Source = p;
                    }
                    break;
    default      :  return(FALSE);
  }

  if (Len+1 < MaxLen) {
    addchar(Target,' ');
    Len++;
    if (strlen(Source) < MaxLen - Len) {
      strcat(Target,Source);
      Source[0] = 0;
    } else if ((p = lastspace(Source,MaxLen-Len)) != NULL) {
      *p = 0;
      strcat(Target,Source);
      strcpy(Source,p+1);
    } else {
      beep();
      return(FALSE);
    }

    updaterecord(s);

    switch (SourceLine) {
      case TEXTLINE:  VMWrite(&Parents,parent,Pos,sizeof(parenttype)); break;  //lint !e644 parent is initialized
      case DUPELINE:  VMWrite(&Children,child,Pos,sizeof(childtype));  break;  //lint !e644 child is initialized
    }
    return(TRUE);
  }

  return(FALSE);
}


static void near pascal reflow(long CurPos, bool Recurse) {
  bool       Reflowed;
  unsigned   CurrentLen;
  unsigned   NextLen;
  childtype  CurrentChild;
  childtype *NextChild;
  char      *p;
  char      *q;

  CurrentChild = * (childtype *) VMRecordGetByPos(&Children,CurPos);
  if (CurrentChild.NextChildPos == VM_INVALID_POS)
    return;

  CurrentLen = strlen(CurrentChild.Str);

  NextChild = (childtype *) VMRecordGetByPos(&Children,CurrentChild.NextChildPos);
  if ((p = strchr(NextChild->Str,'|')) == NULL)  // nothing to reflow
    return;

  p++;
  stripright(p,' ');
  NextLen = strlen(p);
  if (NextLen == 0)      // nothing to reflow
    return;

  Reflowed = FALSE;
  if (CurrentLen + NextLen < sizeof(CurrentChild.Str)-1) {
    CurrentChild.Str[CurrentLen] = ' ';
    strcpy(&CurrentChild.Str[CurrentLen+1],p+1);
    *p = 0;
    VMRecordChanged(&Children);
    VMWrite(&Children,&CurrentChild,CurPos,sizeof(childtype));
    Reflowed = TRUE;
  } else {
    q = lastspace(p,sizeof(CurrentChild.Str) - CurrentLen - 1);

    // this routine is UN-TESTED (5/16/94)
    if (q != NULL) {
      if (q == p)    // if they are equal, then this is the space immediately
        q++;         // after the vertical bar, move forward one position

      *q = 0;
      CurrentChild.Str[CurrentLen] = ' ';
      strcpy(&CurrentChild.Str[CurrentLen+1],p+1);
      memmove(p+1,q+1,strlen(q+1)+1);
      VMRecordChanged(&Children);
      VMWrite(&Children,&CurrentChild,CurPos,sizeof(childtype));
      Reflowed = TRUE;
    }
  }

  // these routine are UN-TESTED (5/16/94)
  if (Reflowed) {
    reflow(CurrentChild.NextChildPos,TRUE);

    if (! Recurse) {
      // now we need to delete any blank lines from the bottom

      setkbdstatus(getkbdstatus() | INSERT);
    }
  }
}



static void near pascal removeblankline(long CurPos) {
  childtype   *Child;
  char        *p;
  scrnlintype  Rec;

  // find the last child
  while (TRUE) {
    Child = (childtype *) VMRecordGetByPos(&Children,CurPos);
    if (Child->NextChildPos == VM_INVALID_POS)
      break;
    CurPos = Child->NextChildPos;
  }

  // check to see if this line is blank and, if so, remove it
  stripright(Child->Str,' ');
  if ((p = strchr(Child->Str,'|')) == NULL || strlen(p+1) == 0) {
    Rec.Line  = DUPELINE;
    Rec.Pos   = CurPos;
    Rec.Child = *Child;
    deleteline(&Rec);
  }
}


static long near pascal insertchild(scrnlintype *s, bool AutoInsert, bool Flow, childtype *Default) {
  long        Pos;
  long        NextChildPos;
  childtype  *c;

  c = (childtype *) VMRecordCreate(&Children,sizeof(childtype),&Pos,NULL);
  *c = *Default;

  if (AutoInsert) {
    if (Flow)
      wordwrap(s,c->Str);
  } else {
    Cursor.LastPos = Work.Indent + 1;
    Cursor.Line = DUPELINE;
  }

  if (s->Line == DUPELINE) {
    c->ParentPos    = s->Child.ParentPos;
    c->NextChildPos = s->Child.NextChildPos;
    c->PrevChildPos = s->Pos;
    s->Child.NextChildPos = Pos;
  } else {
    c->ParentPos    = s->Pos;
    c->NextChildPos = s->Parent.Fields.FirstChildPos;
    c->PrevChildPos = VM_INVALID_POS;
    s->Parent.Fields.FirstChildPos = Pos;
  }

  NextChildPos = c->NextChildPos;
  if (NextChildPos != VM_INVALID_POS) {
    c = (childtype *) VMRecordGetByPos(&Children,c->NextChildPos);
    c->PrevChildPos = Pos;
    VMRecordChanged(&Children);
  }

  updaterecord(s);  // update the current record
  insertincombined(&AllFiles,s->Pos,Pos,s->Line,DUPELINE);
  if (Filter != SHOWALL)
    insertincombined(&Matches,s->Pos,Pos,s->Line,DUPELINE);

  if (Flow && Pos != VM_INVALID_POS) {
    reflow(Pos,FALSE);
    if (NextChildPos != VM_INVALID_POS /* Input.Ch != ' ' */)
      removeblankline(Pos);
  }

  return(Pos);
}


static long near pascal insertparent(scrnlintype *s, parenttype *Default) {
  parenttype *p;
  long        Pos;

  p = (parenttype *) VMRecordCreate(&Parents,sizeof(parenttype),&Pos,NULL);
  *p = *Default;

  insertinparentsonly(&AllFiles,(s->Line == DUPELINE ? s->Child.ParentPos : s->Pos),Pos);
  insertincombined(&AllFiles,s->Pos,Pos,s->Line,Default->Line);

  if (Filter != SHOWALL) {
    insertinparentsonly(&Matches,(s->Line == DUPELINE ? s->Child.ParentPos : s->Pos),Pos);
    insertincombined(&Matches,s->Pos,Pos,s->Line,Default->Line);
  }
  return(Pos);
}



static void near pascal copyline(scrnlintype *s) {
  long        Pos;
  long        ParentPos;
  parenttype  Parent;
  childtype   Child;
  scrnlintype ScrnRec;

  switch (s->Line) {
    case DUPELINE:  Child = s->Child;
                    insertchild(s,FALSE,FALSE,&Child);     //lint !e534
                    break;
    case TEXTLINE:  Parent = s->Parent;
                    insertparent(s,&Parent);               //lint !e534
                    break;
    case DIRLINE :  Parent = s->Parent;
                    Parent.Fields.FirstChildPos = VM_INVALID_POS;
                    ParentPos = insertparent(s,&Parent);

                    ScrnRec.Pos    = ParentPos;
                    ScrnRec.Line   = DIRLINE;
                    ScrnRec.Parent = Parent;

                    Pos = s->Parent.Fields.FirstChildPos;

                    while (Pos != VM_INVALID_POS) {
                      Child = *(childtype *) VMRecordGetByPos(&Children,Pos);

                      Pos = Child.NextChildPos;
                      Child.ParentPos = ParentPos;
                      Child.PrevChildPos = VM_INVALID_POS;
                      Child.NextChildPos = VM_INVALID_POS;

                      ScrnRec.Pos   = insertchild(&ScrnRec,FALSE,FALSE,&Child);
                      ScrnRec.Line  = DUPELINE;
                      ScrnRec.Child = Child;
                    }
                    break;
  }
}


/********************************************************************
*
*  Function: insertline()
*
*  Desc    : Insert a line into the DIR file being edited
*
*/

static void near pascal insertline(scrnlintype *s, bool AutoInsert, bool Flow) {
  childtype   Child;
  parenttype  Parent;

  if (AutoInsert) {
    MsgData.Answer[0] = 'C';  // we're word wrapping onto a DUPELINE
  } else {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox   = TRUE;
    MsgData.Save      = TRUE;
    MsgData.Msg1      = "Insert a pre-filled text line containing";
    MsgData.Line1     = Scrn_BottomRow-10;
    MsgData.Color1    = Colors[QUESTION];
    MsgData.Msg2      = "A) Nothing                        ";
    MsgData.Line2     = Scrn_BottomRow-8;
    MsgData.Color2    = Colors[QUESTION];
    MsgData.Msg3      = "B) File description guidelines    ";
    MsgData.Line3     = Scrn_BottomRow-7;
    MsgData.Color3    = Colors[QUESTION];
    MsgData.Msg4      = "C) Secondary description indicator";
    MsgData.Line4     = Scrn_BottomRow-6;
    MsgData.Color4    = Colors[QUESTION];
    MsgData.Quest     = "Insert type";
    MsgData.QuestLine = Scrn_BottomRow-4;
    MsgData.Answer[0] = 'C';
    MsgData.Answer[1] = 0;
    MsgData.Mask      = ABC;
    showmessage();
  }

  if (KeyFlags != ESC) {
    switch(MsgData.Answer[0]) {
      case 'B': if (s->Line == DUPELINE && s->Child.NextChildPos != VM_INVALID_POS) {
                  // don't let them try to insert a parent line right in the
                  // middle of a set of children
                  beep();
                  KeyFlags = NOTHING;
                  return;
                }

                if (s->Line == DIRLINE && s->Parent.Fields.FirstChildPos != VM_INVALID_POS) {
                  // don't let them try to insert a parent line right in the
                  // middle of a set of children
                  beep();
                  KeyFlags = NOTHING;
                  return;
                }

                sprintf(Parent.Str,"³úfilenameú³ ³úsizeú³  %s  ³úúúúúúúúúdescription for this fileúúúúúúúúú³",dateformat());
                Parent.Line = TEXTLINE;
                insertparent(s,&Parent);    //lint !e534
                Cursor.LastPos = 0;
                Cursor.Line = TEXTLINE;
                break;
      case 'C': if (s->Line == TEXTLINE) {
                  // don't let them try to add a child line after a text line
                  beep();
                  KeyFlags = NOTHING;
                  return;
                }

                memset(Child.Str,' ',Work.Indent-1);
                Child.Str[Work.Indent-1] = '|';
                Child.Str[Work.Indent] = 0;
                insertchild(s,AutoInsert,Flow,&Child);   //lint !e534
                break;
      default : Parent.Str[0] = 0;
                Parent.Line = TEXTLINE;
                insertparent(s,&Parent);    //lint !e534
                Cursor.LastPos = 0;
                Cursor.Line = TEXTLINE;
                break;
    }
  }

  KeyFlags = NOTHING;
}



void static near pascal copyfromlinebuf(long ParentPos) {
  bool          Ans;
  int           X;
  parenttype   *p;
  childtype    *c;
  long          NextChildPos;
  scrnlintype   s;
  savescrntype  ScrnBuf;

  savescreen(&ScrnBuf);
  boxcls(14,Scrn_BottomRow-18,66,Scrn_BottomRow-1,Colors[HEADING],SINGLE);
  fastcenter(Scrn_BottomRow-16,"Contents of Pre-Written Description",Colors[DISPLAY]);

  for (X = 0; X < DizCounter; X++)
    fastprint(18,X+Scrn_BottomRow-14,DizBuf[X],Colors[ANSWER]);

  Ans = TRUE;
  inputnum(23,Scrn_BottomRow-3,1,"Replace current description",&Ans,vBOOL,0);

  if (KeyFlags != ESC && Ans) {

    // first replace the description in the parent line
    p = (parenttype *) VMRecordGetByPos(&Parents,ParentPos);
    strcpy(p->Fields.FDesc,DizBuf[0]);

    s.Line       = DIRLINE;
    s.Pos        = ParentPos;
    s.Parent     = *p;
    NextChildPos = p->Fields.FirstChildPos;
    updaterecord(&s);

    // then if there are any more lines left from the FILE_ID.DIZ, begin
    // replacing any chilren that may already exist.
    for (X = 1; X < DizCounter && NextChildPos != VM_INVALID_POS; X++) {
      c = (childtype *) VMRecordGetByPos(&Children,NextChildPos);
      if (strstr(c->Str,"Uploaded by:") != NULL)
        break;
      memset(c->Str,' ',Work.Indent+1);
      c->Str[Work.Indent-1] = '|';
      strcpy(&c->Str[Work.Indent+1],DizBuf[X]);
      VMRecordChanged(&Children);
      s.Line       = DUPELINE;
      s.Pos        = NextChildPos;
      s.Child      = *c;
      NextChildPos = c->NextChildPos;
    }

    // if all of the existing children have been replaced and there
    // are *still* more lines from the FILE_ID.DIZ, then begin inserting new
    // lines and adding descriptions into them.  once done, we can assume that
    // all existing description lines are gone

    if (X < DizCounter) {
      for (; X < DizCounter; X++) {
        insertline(&s,TRUE,FALSE);
        NextChildPos = (s.Line == DIRLINE ? s.Parent.Fields.FirstChildPos : s.Child.NextChildPos);
        c = (childtype *) VMRecordGetByPos(&Children,NextChildPos);
        memset(c->Str,' ',Work.Indent+1);
        c->Str[Work.Indent-1] = '|';
        strcpy(&c->Str[Work.Indent+1],DizBuf[X]);
        VMRecordChanged(&Children);
        s.Line  = DUPELINE;
        s.Pos   = NextChildPos;
        s.Child = *c;
      }

      // force the clean-up to be bypassed since it won't be needed because
      // all existing description lines have been replaced.
      NextChildPos = VM_INVALID_POS;
    }

    // now to clean up... if there are any lines left over from the original
    // description, they should be removed now

    while (NextChildPos != VM_INVALID_POS) {
      c = (childtype *) VMRecordGetByPos(&Children,NextChildPos);
      if (strstr(c->Str,"Uploaded by:") != NULL)
        break;
      s.Line  = DUPELINE;
      s.Pos   = NextChildPos;
      s.Child = *c;
      NextChildPos = c->NextChildPos;
      deleteline(&s);
    }
  }

  restorescreen(&ScrnBuf);
  KeyFlags = NOTHING;
}



void static near pascal togglekeepstatus(parentfieldstype *p, KeepType Status) {
  if (p->Keep != Status)
    p->Keep = Status;
  else if (Status == COPYONLY)
    p->Keep = MOVE;
  else {
    if (p->NCnfNum != CurCnf || p->NDirNum != CurDir)
      p->Keep = MOVE;
    else
      p->Keep = KEEP;
  }
}


static void near pascal showheading(void) {
  clsbox(2,3,27,3,Colors[QUESTION]);
  if (ExpertMode) {
    if (! Protected)
      fastprint( 4, 3,"Cnf",Colors[QUESTION]);
    fastprint( 9, 3,"Dir File Name    Date",Colors[QUESTION]);
  } else {
    fastprint( 2, 3,"Status",Colors[HEADING]);
    fastprint(10, 3,"File Name     Date  ",Colors[QUESTION]);
  }
  fastprint(34,3,"Description",Colors[QUESTION]);
}


static void near pascal displaybottomline(void) {
  fastprint(1,BottomLine,"ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ",Colors[QUESTION]);
}


/********************************************************************
*
*  Function: showkeepstatus()
*
*  Desc    : Shows one of the following status symbols:
*
*            "   "  = KEEP;     Keep the file where it is
*            "MOV"  = MOVE;     Move the file to a new conf and/or directory
*            "DEL"  = DELETE;   Delete the file (both description & file)
*            "CPY"  = COPYONLY; Copy the file (but leave it where it is)
*            "REM"  = REMOVE;   Remove the file but leave description
*            "SEL"  = TAGGED;   The file is tagged for moving/copying
*/

static void near pascal showkeepstatus(parenttype *p, int LineNum) {
  char static *KeepStatus[TAGGED+1] = { "   ", "MOV", "DEL", "CPY", "REM", "SEL" };
  char static *ExpertKeepStatus[TAGGED+1] = { " ", "M", "D", "C", "R", "S" };
  KeepType     Status;
  int          Pos;
  unsigned     Date;
  int          Days;
  char        *q;
  char         Str[30];

  Status = p->Fields.Keep;
  Days   = p->Fields.Days;

  if (ExpertMode) {
    q   = ExpertKeepStatus[Status];
    Pos = 2;
  } else {
    q   = KeepStatus[Status];
    Pos = 5;
  }
  fastprint(Pos,LineNum,q,Colors[HEADING]);

  if (Days != 0 && (Status == MOVE || Status == COPYONLY)) {
    dtoc(p->Fields.NDate,Str);
    Date = datetojulian(Str) + Days;
    sprintf(Str,"´ Days:%4d / %s Ã",Days,countrydate(juliantodate(Date)));
    fastprint(20,BottomLine,Str,Colors[QUESTION]);
  }
}


/********************************************************************
*
*  Function: showexiststatus()
*
*  Desc    : Shows one of the following status symbols:
*
*            "DSK"  = DIRECT;   Exists only in the hard disk subdirectory
*            "DIR"  = LISTING;  Exists only in the DIR file listing
*            "   "  = BOTH;     Exists both on disk and in directory
*/

static void near pascal showexiststatus(ExistType Status, int LineNum) {
  static char *ExistStatus[BOTH+1] = { "DSK", "DIR", "   " };
  static char *ExpertExistStatus[BOTH+1] = { "D", "L", " " };
  char *p;

  if (ExpertMode) {
    p   = ExpertExistStatus[Status];
  } else {
    p   = ExistStatus[Status];
  }
  fastprint(1,LineNum,p,Colors[DISPLAY]);
}


static void near pascal getdatestr(char *Str, int Date) {
  dtoc(Date,Str);
  if (isdigit(Str[0]))
    countrydate(Str);
  if (ExpertMode) {
    switch (Str[0]) {
      case 'D': strcpy(Str,"DELETE"); break;
      case 'O': strcpy(Str,"OFFLNE"); break;
      default : Str[2] = Str[3];
                Str[3] = Str[4];
                Str[4] = Str[6];
                Str[5] = Str[7];
                Str[6] = 0;
                break;
    }
  }
}


static bool near pascal matchnamefilter(parenttype *p) {
  char SrchFile[13];

  // examine only DIR lines
  if (p->Line != DIRLINE)
      return(FALSE);

  strcpy(SrchFile,p->Fields.NName);
  stripright(SrchFile,' ');
  formatwild(SrchFile);
  return(equalwilds(SrchForm,SrchFile));
}


// copy parent and all children into one text buffer, then return with the
// buffer filled and a return value equal to the length of the text in it.

static int near pascal rolluptext(char *Buffer, parenttype *p) {
  int        Len;
  int        More;
  childtype *c;
  char      *q;
  char      *r;
  long       Pos;
  char       Temp[10];

  if (p->Line == TEXTLINE) {
    strcpy(Buffer,p->Str);
    return(strlen(Buffer));
  }

  sprintf(Buffer,"%s %s %s %ld ",
          p->Fields.NName,
          p->Fields.FDesc,
          countrydate(dtoc(p->Fields.NDate,Temp)),
          p->Fields.FSize);

  Len = strlen(Buffer);

  if (p->Fields.FirstChildPos != VM_INVALID_POS) {
    Pos = p->Fields.FirstChildPos;
    q = &Buffer[Len];

    // accumulate all of the child descriptions into one buffer

    while (Pos != VM_INVALID_POS && Len < ROLLUPSIZE-10) {
      c = (childtype *) VMRecordGetByPos(&Children,Pos);
      for (r = c->Str; *r == ' '; r++);         // skip past leading spaces
      maxstrcpy(q,r,ROLLUPSIZE - (Len+10));
      stripright(q,' ');   // strip excessive trailing spaces
      More = strlen(q);
      q += More;
      *q = ' ';            // add one space back in as a separator
      q++;
      Len += More + 1;     // adjust Len by More plus 1 for the separator
      Pos  = c->NextChildPos;
    }
  }

  return(Len);
}


static bool near pascal matchtextfilter(parenttype *p) {
  int    Len;
  char   Temp[ROLLUPSIZE];

  Len = rolluptext(Temp,p);
  return(parsersearch(Temp,Len,SearchInput,FALSE,0));
}


static bool near pascal matchrules(parenttype *p) {
  int         Len;
  rulestype  *r;
  long        X;
  char        SrchFile[13];
  char        Temp[ROLLUPSIZE];

  // examine only DIR lines, and only process records which are not already
  // marked for movement
  if (p->Line != DIRLINE || p->Fields.Keep != KEEP)
    return(FALSE);

  // don't automatically process files which were requested to remain private
  if (p->Fields.FDesc[0] == '/')
    return(FALSE);

  // first time thru the loop, Len = -1, this signifies that we need to roll up
  // the text into a single buffer, then we can avoid re-rolling it up on
  // subsequent loops
  Len = -1;

  // The filename needs to be converted to a format that will be comparable
  // to a wildcard.  Blank it first, so that we can detect when later runs
  // thru the loop won't require creating the format again.
  SrchFile[0] = 0;

  // If we ever have to find the file on disk, then we want to make sure it
  // hapens only once.  So blank out SrchDskPath first, then we can detect
  // during later runs if it has already been filled in.
  SrchDskPath[0] = 0;

  for (X = 1; X <= TotalRules; X++) {
    r = (rulestype *) VMRecordGetByIndex(&Rules,X,NULL);

    // ignore this record if the target conf+dir is the same as the
    // current conference and directory
    if (r->Dir == CurDir && r->Cnf == CurCnf)
      continue;  // loop around for another rule

    // *.* gets converted to a null string so that we can quickly match all
    // files if necessary, otherwise, check for a match on the filespec
    if (r->Name[0] != 0) {

      // if this is our first time thru, convert the file name into a
      // format that we can use to compare wildcards
      if (SrchFile[0] == 0) {
        strcpy(SrchFile,p->Fields.NName);
        stripright(SrchFile,' ');
        formatwild(SrchFile);
      }
      if (! equalwilds(r->Name,SrchFile))
        continue;
    }

    // no search criteria means match ALL files
    if (r->Criteria[0] != 0) {
      // initialize the boyer-moore search tables, get out if there is an
      // error in the token stream
      strcpy(SrchText,r->Criteria);
      if (tokenscan(SrchText,SearchInput,FALSE) <= 0)
        continue;

      // roll the description up into a single buffer if we haven't already
      if (Len == -1)
        Len = rolluptext(Temp,p);

      // check to see if the current file matches the search criteria
      if (! parsersearch(Temp,Len,SearchInput,FALSE,0)) {
        stopsearch();
        continue;
      }

      stopsearch();
    }

    // *.* gets converted to a null string so that we can quickly match all
    // files if necessary, otherwise, check for a match on the filespec inside
    if (r->InSpec[0] != 0) {
      // if we haven't found it already, go find it now
      if (SrchDskPath[0] == 0) {
        if (verifyexist('Q',DPath,PathList,p->Fields.FName) == 255)
          continue;
      }
      // then, check inside the file to see if the filespec exists inside of
      // the found file (which verifyexist() placed in SrchDskPath)
      if (! wildcardinsidefile(SrchDskPath,r->InSpec))
        continue;
    }

    // if we got this far, then we found a match!  So record that the file
    // will be moved, where it will be moved to, and in how many days.
    p->Fields.Keep    = MOVE;
    p->Fields.NCnfNum = r->Cnf;
    p->Fields.NDirNum = r->Dir;
    p->Fields.Days    = r->Days;
    VMRecordChanged(&Parents);
    return(TRUE);
  }

  SrchText[0] = 0;
  return(FALSE);
}


static bool near pascal matchmove(parenttype *p) {
  return(p->Line == DIRLINE && (p->Fields.Keep == MOVE || p->Fields.Keep == COPYONLY));
}


static bool near pascal matchnotmove(parenttype *p) {
  return(p->Line == DIRLINE && p->Fields.Keep == KEEP);
}


static bool near pascal matchdelete(parenttype *p) {
  return(p->Line == DIRLINE && (p->Fields.Keep == DELETE || p->Fields.Keep == REMOVE));
}


static bool near pascal matchondisk(parenttype *p) {
  return(p->Line == DIRLINE && p->Fields.Exist == DIRECT);
}


static bool near pascal matchinlist(parenttype *p) {
  return(p->Line == DIRLINE && p->Fields.Exist == LISTING);
}


static bool near pascal matchselected(parenttype *p) {
  return(p->Line == DIRLINE && p->Fields.Keep == TAGGED);
}


static bool near pascal matchnonselected(parenttype *p) {
  return(p->Line == DIRLINE && p->Fields.Keep != TAGGED);
}


static void near pascal scanning(void) {
  boxcls(24,Scrn_BottomRow-8,53,Scrn_BottomRow-4,Colors[OUTBOX],SINGLE);
  fastprintmove(27,Scrn_BottomRow-6,"Scanning ... Please Wait",Colors[HEADING]);
}


static void near pascal findmatches(filtertype SetFilter, savescrntype *ScrnBuf) {
  bool near pascal  (*filter)(parenttype *p);
  parenttype        *p;
  childtype         *c;
  combinedtype      *combined;
  parentsonlytype   *parentsonly;
  long               ParentPos;
  long               ChildPos;
  long               RecNum;
  char               ShowStr[30];

  resetfilter();

  switch (SetFilter) {
    case NAMEMATCHES        :  filter = matchnamefilter;   break;
    case TEXTMATCHES        :  filter = matchtextfilter;   break;
    case RULEMATCHES        :  filter = matchrules;        break;
    case KEEPMATCHES        :  filter = matchnotmove;      break;
    case MOVEMATCHES        :  filter = matchmove;         break;
    case DELETEMATCHES      :  filter = matchdelete;       break;
    case ONDISKMATCHES      :  filter = matchondisk;       break;
    case INDIRMATCHES       :  filter = matchinlist;       break;
    case SELECTEDMATCHES    :  filter = matchselected;     break;
    case NONSELECTEDMATCHES :  filter = matchnonselected;  break;
    default                 :  return;
  }

  scanning();

  TotalMatches = TotalMatchingParents = 0;
  VMInitRec(&CombinedMatches,NULL,0,sizeof(combinedtype));
  VMInitRec(&ParentsOnlyMatches,NULL,0,sizeof(parentsonlytype));

  for (RecNum = 1; RecNum <= TotalParents; RecNum++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,RecNum,NULL);
    p = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);

    sprintf(ShowStr,"File %6ld of %6ld",RecNum,TotalParents);
    fastprint(58,1,ShowStr,Colors[DISPLAY]);

    if (! filter(p))
      continue;

    TotalMatches++;
    TotalMatchingParents++;

    if (! BatchMode) {
      ParentPos = parentsonly->Pos;

      parentsonly = (parentsonlytype *) VMRecordCreate(&ParentsOnlyMatches,sizeof(parentsonlytype),NULL,NULL);
      parentsonly->Pos = ParentPos;

      combined = (combinedtype *) VMRecordCreate(&CombinedMatches,sizeof(combinedtype),NULL,NULL);
      combined->Pos  = ParentPos;
      combined->Line = p->Line;

      if (p->Line == DIRLINE) {
        ChildPos  = p->Fields.FirstChildPos;
        while (ChildPos != VM_INVALID_POS) {
          combined = (combinedtype *) VMRecordCreate(&CombinedMatches,sizeof(combinedtype),NULL,NULL);
          combined->Pos  = ChildPos;
          combined->Line = DUPELINE;
          c = (childtype *) VMRecordGetByPos(&Children,ChildPos);
          ChildPos = c->NextChildPos;
          TotalMatches++;
        }
      }
    }
  }

  if (TotalMatchingParents == 0) {
    VMDone(&CombinedMatches);
    VMDone(&ParentsOnlyMatches);
    if (! BatchMode) {
      beep();
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.AutoBox = TRUE;
      MsgData.Msg1    = " There Were No Matches Found!";
      MsgData.Line1   = Scrn_BottomRow - 6;
      MsgData.Color1  = Colors[HEADING];
      showmessage();
      restorescreen(ScrnBuf);
      clsbox(1,1,28,1,Colors[HEADING]);
    }
  } else {
    Filter  = SetFilter;
    Current = &Matches;
    restorescreen(ScrnBuf);
    showfilter();
  }
}


static bool near pascal editfindname(void) {
  savescrntype  ScrnBuf;

  savescreen(&ScrnBuf);
  boxcls(3,5,76,12,Colors[OUTBOX],SINGLE);
  fastprint( 5, 7,"You may locate any file within this listing by entering a single"     ,Colors[QUESTION]);
  fastprint( 5, 8,"file name or by using DOS wild card conventions (e.g. *.TXT, A?.T?T).",Colors[QUESTION]);
  inputstr(5,10,12,"File Search Specification",SrchCriteria,SrchCriteria,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
  stripright(SrchCriteria,' ');
  restorescreen(&ScrnBuf);

  if (SrchCriteria[0] == 0 || KeyFlags == ESC) {
    KeyFlags = NOTHING;
    return(FALSE);
  } else {
    massagesrchcriteria();
    strcpy(SrchForm,SrchCriteria);
    formatwild(SrchForm);
    findmatches(NAMEMATCHES,&ScrnBuf);
  }

  KeyFlags = NOTHING;
  return(Filter != SHOWALL);
}


static bool near pascal editfindtext(void) {
  savescrntype  ScrnBuf;

  savescreen(&ScrnBuf);
  boxcls(2,5,77,12,Colors[OUTBOX],SINGLE);
  fastprint( 4, 7,"You may locate any text within this listing by typing in text as you",Colors[QUESTION]);
  fastprint( 4, 8,"would for a PCBoard Zippy Dir Scan.",Colors[QUESTION]);
  inputstr(4,10,52,"Enter Search Text",SrchText,SrchText,ALLCHAR,INPUT_CAPS|INPUT_CLEAR,0);
  stripright(SrchText,' ');
  restorescreen(&ScrnBuf);

  if (SrchText[0] == 0 || KeyFlags == ESC || tokenscan(SrchText,SearchInput,FALSE) <= 0) {
    KeyFlags = NOTHING;
    return(FALSE);
  } else {
    strupr(SrchText);
    findmatches(TEXTMATCHES,&ScrnBuf);
  }

  stopsearch();
  KeyFlags = NOTHING;
  return(Filter != SHOWALL);
}


static bool near pascal setfilter(void) {
  char static   Mask[10] = {9, 'L','Z','M','D','U','S','N','R','K'};
  char          Ans[2];
  savescrntype  ScrnBuf;

  savescreen(&ScrnBuf);
  boxcls(9,3,70,19,Colors[OUTBOX],SINGLE);
  fastcenter(4,"Set Search Filter",Colors[HEADING]);
  fastprint(12, 7,"L - Locate by Filename or by Wildcard",Colors[QUESTION]);
  fastprint(12, 8,"Z - Locate by Description",Colors[QUESTION]);
  fastprint(12, 9,"M - Display Files Marked For Moving or Copying",Colors[QUESTION]);
  fastprint(12,10,"D - Display Files Marked For Deleting",Colors[QUESTION]);
  fastprint(12,11,"U - Display Un-Marked Files",Colors[QUESTION]);
  fastprint(12,12,"S - Display Selected Files,",Colors[QUESTION]);
  fastprint(12,13,"N - Display non-Selected Files",Colors[QUESTION]);
  fastprint(12,14,"R - Display Files Found in DIR but Missing from Disk",Colors[QUESTION]);
  fastprint(12,15,"K - Display Files Found on Disk but Missing Description",Colors[QUESTION]);
  fastprint(55, 7,"(ALT-L)",Colors[HEADING]);
  fastprint(55, 8,"(ALT-Z)",Colors[HEADING]);
  fastprint(45,17,"Press ESC to Exit",Colors[DISPLAY]);

  Ans[0] = 0;
  inputstr(12,17,1,"Enter Filter Selection",Ans,Ans,Mask,INPUT_CAPS,0);
  restorescreen(&ScrnBuf);

  if (KeyFlags == ALTL)
    Ans[0] = 'L';

  if (KeyFlags == ALTZ)
    Ans[0] = 'Z';

  if (KeyFlags == ESC || Ans[0] == 0) {
    KeyFlags = NOTHING;
    return(FALSE);
  }

  KeyFlags = NOTHING;
  switch(Ans[0]) {
    case 'L' : return(editfindname());
    case 'Z' : return(editfindtext());
    case 'M' : findmatches(MOVEMATCHES,&ScrnBuf);
               break;
    case 'D' : findmatches(DELETEMATCHES,&ScrnBuf);
               break;
    case 'U' : findmatches(KEEPMATCHES,&ScrnBuf);
               break;
    case 'S' : findmatches(SELECTEDMATCHES,&ScrnBuf);
               break;
    case 'N' : findmatches(NONSELECTEDMATCHES,&ScrnBuf);
               break;
    case 'R' : findmatches(INDIRMATCHES,&ScrnBuf);
               break;
    case 'K' : findmatches(ONDISKMATCHES,&ScrnBuf);
               break;
  }

  return(Filter != SHOWALL);
}


void pascal applyrulesinit(char *DskPath, char *PathListName, unsigned BoardNumber, unsigned DirNumber) {
  DPath    = DskPath;
  PathList = PathListName;
  CurCnf   = BoardNumber;
  CurDir   = DirNumber;
}


int pascal applyrulesfile(char *FileName, savescrntype *ScrnBuf) {
  rulestype  *p;
  DOSFILE     File;
  rulestype   Rule;

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(-1);

  VMInitRec(&Rules,NULL,0,sizeof(rulestype));
  scanning();

  TotalRules = 0;
  memset(&Rule,0,sizeof(Rule));
  while (readonerule(&File,&Rule) != -1) {
    p = (rulestype *) VMRecordCreate(&Rules,sizeof(rulestype),NULL,NULL);
    *p = Rule;

    strcpy(SrchCriteria,p->Name);
    massagesrchcriteria();
    if (SrchCriteria[0] == 0 || strcmp(SrchCriteria,"*.*") == 0)
      p->Name[0] = 0;
    else {
      strcpy(p->Name,SrchCriteria);
      formatwild(p->Name);
    }

    if (p->InSpec[0] == 0 || strcmp(p->InSpec,"*.*") == 0)
      p->InSpec[0] = 0;
    else
      formatwild(p->InSpec);

    strupr(p->Criteria);
    TotalRules++;
    memset(&Rule,0,sizeof(Rule));
  }
  dosfclose(&File);

  findmatches(RULEMATCHES,ScrnBuf);

  VMDone(&Rules);
  return(0);
}


static bool near pascal applyrules(void) {
  char          Name[66];
  savescrntype  ScrnBuf;

  strcpy(Name,Work.RulesName);
  savescreen(&ScrnBuf);

top:
  boxcls(3,Scrn_BottomRow-9,76,Scrn_BottomRow-1,Colors[HEADING],SINGLE);
  fastcenter(Scrn_BottomRow-7,"Enter Filename for Auto-Selection Rules",Colors[DISPLAY]);
  fastcenter(Scrn_BottomRow-3," Press ENTER to Apply Rules - OR - Press F2 to Edit Rules ",Colors[DESC]);
  inputstr(5,Scrn_BottomRow-5,59,"Filename",Name,Name,ALLFILE,INPUT_CAPS,0);

  if (KeyFlags == ESC || Name[0] == 0) {
    restorescreen(&ScrnBuf);
    KeyFlags = NOTHING;
    return(FALSE);
  }

  strcpy(Work.RulesName,Name);
  if (KeyFlags == F2) {
    editrulesfile(Name);
    restorescreen(&ScrnBuf);
    setupscale(NumLines-1,177,5);
    goto top;
  }

  KeyFlags = NOTHING;
  restorescreen(&ScrnBuf);

  if (applyrulesfile(Name,&ScrnBuf) == -1)
    return(FALSE);

  return(Filter != SHOWALL);
}


static parenttype * getfirstdirline(void) {
  parenttype      *p;
  parentsonlytype *parentsonly;
  long             Count;

  for (Count = 1; Count <= *Current->pTotalParents; Count++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,Count,NULL);
    p = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);
    if (p->Line == DIRLINE)
      return(p);
  }
  return(p);
}


static void near pascal fillscreenrecs(long RecNum) {
  int                X;
  parenttype        *p;
  childtype         *c;
  combinedtype      *combined;
  parentsonlytype   *parentsonly;

  memset(ScrnRecs,0,sizeof(ScrnRecs));

  for (X = 0; X < NumLines; RecNum++) {
    if (ShowParentsOnly) {
      if (RecNum > *Current->pTotalParents)
        return;

      parentsonly = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,RecNum,NULL);
      p = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);
      ScrnRecs[X].Pos    = parentsonly->Pos;
      ScrnRecs[X].Line   = p->Line;
      ScrnRecs[X].Parent = *p;

    } else {
      if (RecNum > *Current->pTotalLines)
        return;

      combined = (combinedtype *) VMRecordGetByIndex(Current->pCombined,RecNum,NULL);
      if (combined->Line == DIRLINE || combined->Line == TEXTLINE) {
        p = (parenttype *) VMRecordGetByPos(&Parents,combined->Pos);
        ScrnRecs[X].Parent = *p;
      } else {
        c = (childtype *) VMRecordGetByPos(&Children,combined->Pos);
        ScrnRecs[X].Child = *c;
      }
      ScrnRecs[X].Line = combined->Line;
      ScrnRecs[X].Pos  = combined->Pos;
    }

    X++;
  }
}



/********************************************************************
*
*  Function: displayline()
*
*  Desc    : Displays the table pointed to by *q on the screen.
*
*/

static void near pascal displayline(long RecNum) {
  scrnlintype *s;
  int          X;
  int          LineNum;
  char         Temp[40];

  fillscreenrecs(RecNum);
  clsbox(1,5,78,BottomLine-1,Colors[OUTBOX]);

  for (X = 0, s = ScrnRecs, LineNum = 5; X < NumLines; X++, LineNum++, s++) {
    switch (s->Line) {
      case DIRLINE : showkeepstatus(&s->Parent,LineNum);
                     showexiststatus(s->Parent.Fields.Exist,LineNum);
                     if (ExpertMode) {
                       if (! Protected) {
                         if (s->Parent.Fields.NCnfNum == 0xFFFF)
                           strcpy(Temp,"X   ");
                         else
                           sprintf(Temp,"%4u",s->Parent.Fields.NCnfNum);
                         fastprint(EXPCONF,LineNum,Temp,Colors[ANSWER]);
                       }
                       sprintf(Temp,"%4u",s->Parent.Fields.NDirNum);
                       fastprint(EXPDIR,LineNum,Temp,Colors[ANSWER]);
                       fastprint(EXPNAME,LineNum,s->Parent.Fields.NName,Colors[ANSWER]);
                       getdatestr(Temp,s->Parent.Fields.NDate);
                       fastprint(EXPDATE,LineNum,Temp,Colors[ANSWER]);
                     } else {
                       fastprint(NOVNAME,LineNum,s->Parent.Fields.NName,Colors[ANSWER]);
                       getdatestr(Temp,s->Parent.Fields.NDate);
                       fastprint(NOVDATE,LineNum,Temp,Colors[ANSWER]);
                     }
                     fastprint(BOTHDESC,LineNum,s->Parent.Fields.FDesc,Colors[ANSWER]);
                     break;
      case DUPELINE: fastprint(1,LineNum,s->Child.Str,Colors[DISPLAY]);
                     break;
      case TEXTLINE: if (s->Pos != VM_INVALID_POS)
                       fastprint(1,LineNum,s->Parent.Str,Colors[DISPLAY]);
                     break;
    }
  }
}


int pascal checkconf(unsigned CnfNum) {
  char Str[80];

  if (CnfNum == 0xFFFF) {
    Offline = TRUE;
    return(0);
  }

  if (CnfNum > PcbData.NumConf) {
    beep();
    sprintf(Str,"Valid conference numbers range from 0 to %d.",PcbData.NumConf);
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save      = TRUE;
    MsgData.AutoBox   = TRUE;
    MsgData.Msg1      = Str;
    MsgData.Line1     = Scrn_BottomRow-6;
    MsgData.Color1    = Colors[QUESTION];
    showmessage();
    return(-1);
  }
  Offline = FALSE;
  return(1);
}


static int checkconfrange(int Before) {
  int RetVal;

  if (Before)
    return(0);

  if (KeyFlags == F2) {
    editdirlist(TempCnf,TempDir,TRUE,TRUE);
    setupscale(NumLines-1,177,5);
    TempCnf = NewBoardNumber;
    TempDir = NewDirNumber;
    KeyFlags = NOTHING;
  }

  if ((RetVal = checkconf(TempCnf)) == -1)
    return(-1);

  setnewloc(TempCnf,TempDir);
  return(RetVal);
}


int pascal checkdir(unsigned CnfNum, unsigned DirNum) {
  unsigned    HighNum;
  unsigned    NumTextDirs;
  char        Str[80];
  pcbconftype Conf;

  getconfrecord(CnfNum,&Conf);
  NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
  HighNum = NumTextDirs + (Conf.UpldDir[0] == 0 ? 0 : 1);

  if (DirNum > HighNum) {
    beep();
    sprintf(Str,"Valid directory numbers for conference %d range from 0 to %d.",CnfNum,HighNum);
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save      = TRUE;
    MsgData.AutoBox   = TRUE;
    MsgData.Msg1      = Str;
    MsgData.Line1     = Scrn_BottomRow-6;
    MsgData.Color1    = Colors[QUESTION];
    showmessage();
    return(-1);
  }
  return(1);
}


static int checkdirrange(int Before) {
  int RetVal;

  if (Before)
    return(0);

  if (KeyFlags == F2) {
    editdirlist(TempCnf,TempDir,TRUE,TRUE);
    setupscale(NumLines-1,177,5);
    TempCnf = NewBoardNumber;
    TempDir = NewDirNumber;
    KeyFlags = NOTHING;
  }

  if (Offline || New.Cnf == 0xFFFF) {
    Offline = TRUE;
    setnewloc(0xFFFF,0);
    return(0);
  }

  if ((RetVal = checkdir(TempCnf,TempDir)) == -1)
    return(-1);

  setnewloc(TempCnf,TempDir);
  return(RetVal);
}


static int checkoffline(int Before) {
  if (Before)
    return(0);

  if (KeyFlags == F2) {
    editdirlist(TempCnf,TempDir,TRUE,TRUE);
    setupscale(NumLines-1,177,5);
    setnewloc(NewBoardNumber,NewDirNumber);
    KeyFlags = NOTHING;
  }

  if (Offline) {
    setnewloc(0xFFFF,0);
  } else if (New.Cnf == 0xFFFF) {
    setnewloc(CurCnf,CurDir);
  }

  return(1);
}


static void near pascal convertdirtotext(scrnlintype *p) {
  bool    Save;
  char    Temp[10];
  char    Str[81];

  if (p->Line != DIRLINE)
    return;

  SaveParentPos   = p->Pos;
  SaveChildPos    = p->Parent.Fields.FirstChildPos;

  Save = ExpertMode;
  ExpertMode = FALSE;
  getdatestr(Temp,p->Parent.Fields.NDate);     /* get full date format in novice mode */
  ExpertMode = Save;

  sprintf(Str,"%-12.12s%9ld  %s  %s",
              p->Parent.Fields.NName,
              p->Parent.Fields.FSize,
              Temp,
              p->Parent.Fields.FDesc);

  strcpy(p->Parent.Str,Str);
  p->Line = p->Parent.Line = TEXTLINE;
}


static void near pascal converttexttodir(scrnlintype *s, int ConfNum, int DirNum) {
  char        Buffer[81];
  char        Temp[20];
  unsigned    TestValue;
  unsigned    BufLen;

  if (s->Line == DIRLINE)
    return;

  strcpy(Buffer,s->Parent.Str);
  BufLen = strlen(Buffer);
  memcpy(Temp,&Buffer[23],8);
  Temp[8] = 0;

  if (isdigit(Temp[0]))
    uncountrydate(Temp);

  if (Buffer[0] != ' ' && BufLen >= 30 && (TestValue = ctod2(Temp)) != 0) {
    memset(&s->Parent.Fields,0,sizeof(s->Parent.Fields));
    s->Parent.Fields.FDate   =
    s->Parent.Fields.NDate   = TestValue;
    s->Parent.Fields.NCnfNum = ConfNum;
    s->Parent.Fields.NDirNum = DirNum;
    s->Parent.Fields.Exist   = BOTH;
    s->Parent.Fields.Keep    = KEEP;

    // if this is the same parent that we saved earlier,
    // and if the SaveChildPos value is valid,
    // then restore the first child position and the number of children
    if (s->Pos == SaveParentPos)
      s->Parent.Fields.FirstChildPos = SaveChildPos;

    memcpy(s->Parent.Fields.FName,Buffer,12);  s->Parent.Fields.FName[12] = 0;
    stripright(s->Parent.Fields.FName,' ');
    strcpy(s->Parent.Fields.NName,s->Parent.Fields.FName);
    memcpy(Temp,&Buffer[12],9);  Temp[9] = 0;
    s->Parent.Fields.FSize = atol(Temp);
    if (BufLen > 32)
      memcpy(s->Parent.Fields.FDesc,&Buffer[33],45);  s->Parent.Fields.FDesc[45] = 0;

    s->Line = s->Parent.Line = DIRLINE;
  }
}


static bool near pascal changelocation(parentfieldstype *p) {
  unsigned     OldCnf;
  unsigned     OldDir;
  unsigned     OldDays;
  KeepType     OldKeep;
  savescrntype ScrnBuf;

  setnewloc(p->NCnfNum,p->NDirNum);

  OldCnf   = NewBoardNumber = p->NCnfNum;
  OldDir   = NewDirNumber   = p->NDirNum;
  OldDays  = NewDays        = p->Days;
  OldKeep  = p->Keep;
  MoveFlag = (OldKeep == COPYONLY ? 'C' : 'M');

  if (New.Cnf == 0xFFFF) {
    Offline = TRUE;
    ConfStr[0] = 'X';
    ConfStr[1] = 0;
  } else {
    Offline = FALSE;
    ltoa(New.Cnf,ConfStr,10);
  }

  savescreen(&ScrnBuf);
  boxcls(3,12,75,23,Colors[OUTBOX],SINGLE);
  fastcenter(13,"Select a new location by changing the conference or directory numbers",Colors[DISPLAY]);
  fastcenter(14,"Press the F2 function key to display a list of directories",Colors[DISPLAY]);
  fastcenter(22,"Press ESC to Return",Colors[DISPLAY]);

  New.ShowOnScreen = TRUE;
  showloc();
  if (Protected) {
    initquest(ProtectedQuestions,NUMFIELDS-1);
    readscrn(ProtectedQuestions,NUMFIELDS-2,0,"","",1,NOCLEARFLD);
    freeanswers(ProtectedQuestions,NUMFIELDS-1);
  } else {
    initquest(MoveQuestions,NUMFIELDS);
    readscrn(MoveQuestions,NUMFIELDS-1,0,"","",1,NOCLEARFLD);
    freeanswers(MoveQuestions,NUMFIELDS);
  }
  restorescreen(&ScrnBuf);
  New.ShowOnScreen = FALSE;

  if (Offline || TempCnf == 0xFFFF)
    setnewloc(0xFFFF,0);
  else
    setnewloc(TempCnf,TempDir);

  KeyFlags = NOTHING;
  p->NCnfNum = New.Cnf;
  p->NDirNum = New.Dir;

  if (New.Dir != CurDir || New.Cnf != CurCnf) {
    p->Keep = (MoveFlag == 'C' ? COPYONLY : MOVE);
    p->Days = NewDays;
  } else {
    p->Keep = KEEP;
    p->Days = 0;
  }

  return(OldDir != New.Dir || OldCnf != New.Cnf || OldKeep != p->Keep || OldDays != p->Days);
}


static void near pascal updatetagged(long TopParentNum, long BottomParentNum, parentfieldstype *p) {
  unsigned         NewCnf;
  unsigned         NewDir;
  KeepType         NewKeep;
  parentsonlytype *Index;
  parenttype      *Parent;
  long             X;

  NewCnf  = p->NCnfNum;
  NewDir  = p->NDirNum;
  NewKeep = p->Keep;
  NewDays = p->Days;

  for (X = TopParentNum; X <= BottomParentNum; X++) {
//  Index = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,X,NULL);
    Index = (parentsonlytype *) VMRecordGetByIndex(AllFiles.pParentsOnly,X,NULL);
    Parent = (parenttype *) VMRecordGetByPos(&Parents,Index->Pos);

    if (Parent->Fields.Keep == TAGGED) {
      Parent->Fields.NCnfNum = NewCnf;
      Parent->Fields.NDirNum = NewDir;
      Parent->Fields.Keep    = NewKeep;
      Parent->Fields.Days    = NewDays;
      VMWrite(&Parents,Parent,Index->Pos,sizeof(parenttype));
    }
  }
}


static void near pascal removetags(long TopParentNum, long BottomParentNum) {
  parentsonlytype *Index;
  parenttype      *Parent;
  long             X;

  for (X = TopParentNum; X <= BottomParentNum; X++) {
//  Index = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,X,NULL);
    Index = (parentsonlytype *) VMRecordGetByIndex(AllFiles.pParentsOnly,X,NULL);
    Parent = (parenttype *) VMRecordGetByPos(&Parents,Index->Pos);

    if (Parent->Fields.Keep == TAGGED) {
      togglekeepstatus(&Parent->Fields,TAGGED);
      VMWrite(&Parents,Parent,Index->Pos,sizeof(parenttype));
    }
  }
}


static void near pascal indicatemustmove(void) {
  beep();
  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox   = TRUE;
  MsgData.Save      = TRUE;
  MsgData.Msg1      = "You must first specify that the file is to be MOVED before you can";
  MsgData.Line1     = Scrn_BottomRow-7;
  MsgData.Color1    = Colors[DISPLAY];
  MsgData.Msg2      = "override the operation turning the MOVE into a COPY only procedure";
  MsgData.Line2     = Scrn_BottomRow-6;
  MsgData.Color2    = Colors[DISPLAY];
  showmessage();
}


static bool near pascal gotonextparent(scrnlintype *s, long *Top, int *Index, bool MovingDown) {
  long             SaveTop;
  long             CurRec;
  long             NewRec;
  parentsonlytype *p;

  SaveTop = *Top;
  CurRec  = findposinparentsonly(Current,s->Pos);
  NewRec  = CurRec + (MovingDown ? +1 : -1);

  if (NewRec < 1 || NewRec > TotalParents)
    return(FALSE);

  if (! ShowParentsOnly) {
    p = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,CurRec,NULL);
    CurRec = findposincombined(Current,p->Pos,DIRLINE);
    p = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,NewRec,NULL);
    NewRec = findposincombined(Current,p->Pos,DIRLINE);
    if (NewRec < 1)
      return(FALSE);
  }

  if (MovingDown) {
    if (NewRec - CurRec >= NumLines) {
      *Top   = NewRec - 1;
      *Index = 0;
    } else {
      *Top   = CurRec - 1;
      *Index = (int) (NewRec - CurRec);
    }
  } else {
    if (CurRec - NewRec >= NumLines) {
      *Top = NewRec - 1;
      *Index = 0;
    } else if (CurRec >= NumLines) {
      *Top   = CurRec - NumLines;
      *Index = NumLines - (int) (CurRec - NewRec) - 1;
    } else if (NewRec > NumLines) {
      *Top   = NewRec - NumLines;
      *Index = NumLines - 1;
    } else {
      *Top   = 0;
      *Index = (int) NewRec - 1;
    }
  }

  if (*Top < 0) {
    *Index -= (int) *Top;
    *Top = 0;
  }

  return(SaveTop != *Top);
}


static void near pascal selectallfiles(void) {
  parenttype      *p;
  parentsonlytype *parentsonly;
  long             Count;

  for (Count = 1; Count <= *Current->pTotalParents; Count++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(Current->pParentsOnly,Count,NULL);
    p = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);
    p->Fields.Keep = TAGGED;
    VMRecordChanged(&Parents);
  }

  NumTagged = *Current->pTotalParents;
}



static void near pascal checktotallines(void) {
  parenttype  Parent;
  scrnlintype Rec;

  if (TotalLines == 0) {
    Parent.Line = TEXTLINE;
    memset(Parent.Str,0,sizeof(Parent.Str));
    Rec.Line = TEXTLINE;
    Rec.Pos  = VM_INVALID_POS;  // force the new record into the first slot
    insertparent(&Rec,&Parent); //lint !e534
  }
}



static void near pascal setscreenlines(void) {
  NumLines   = Scrn_BottomRow - (ExpertMode ? 6 : 11);
  BottomLine = NumLines + 5;
  clsbox(1,Scrn_BottomRow-3,78,Scrn_BottomRow-1,Colors[DESC]);
  displaybottomline();

  Work.BigScreen = (NumLines > 25);

  setupscale(NumLines-1,177,5);

  // display help lines
  if (! ExpertMode) {
    fastprintv(79,Scrn_BottomRow-6,"ºººººº",Colors[OUTBOX]);
    fastprint(1,Scrn_BottomRow-5," F2=Select DIR   F9=Delete File   ALT-M=Move File  -I=Insert Line  -B=Sort    ",Colors[DESC]);
    fastprint(1,Scrn_BottomRow-4," F3/F4=Up/Lower  F10=Undo Line       -V=View File  -D=Delete Line  -O=DOS     ",Colors[DESC]);
    fastprint(1,Scrn_BottomRow-3," F5/F6=Text/File ALT-S=Select One    -G=Go Rules   -R=Repeat Line  -P=Process ",Colors[DESC]);
    fastprint(1,Scrn_BottomRow-2," F7=Remove(0)    ALT-A=Select All    -Y=Primaries  -N=Disk Space   -T=Today   ",Colors[DESC]);
    fastprint(1,Scrn_BottomRow-1," F8=Copy Only    ALT-F-Filter (Find) -X=Expert     -5=50/25-Line   -U=Use DIZ ",Colors[DESC]);
  }
}


static void near pascal drawscreen(char *Desc) {
  char Str[40];

  clscolor(Colors[OUTBOX]);
  generalscreen("Edit DIR File","");
  sprintf(Str,"%30.30s",Desc);
  fastprint(48,3,Str,Colors[QUESTION]);
  showheading();
  fastprint( 1, 4,"ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ",Colors[QUESTION]);
  setscreenlines();
  showfilter();
}


static int near pascal numcolumns(void) {
  if (ExpertMode) {
    if (Protected)
      return(2);
    return(3);
  } 
  return(2);
}



/********************************************************************
*
*  Function: editdir()
*
*  Desc    :
*
*  Calls   :
*/

int pascal editdir(char *Desc, char *DskPath, char *PathListName, unsigned BoardNumber, unsigned DirNumber) {
  static char     *ExtractErrors[5] = {"Error Opening File or Invalid Archive","WARNING! DOS RESERVED WORD","Error Extracting Description","Error Reading Extracted Description", "Archive does not contain FILE_ID.DIZ"};
  bool             NeedToDisplay;
  bool             Updated;
  bool             HasChanged;
  bool             MovingDown;
  char             SortType;
  int              Y;
  int              Index;
  int              Temp;
  unsigned         SaveData;
  int              Okay;
  fonttype         SaveFont;
  char            *Str;
  char            *q;
  scrnlintype     *s;
  parenttype      *p;
  long             Top;
  long             CurParent;
  long             TopParentTag;
  long             BottomParentTag;
  long             CurrentTotal;
  char             Today[15];
  int              Column[TEXTDUPE+1];
  parentfieldstype SaveFields;
  char             SaveStr[256];

  DPath    = DskPath;
  PathList = PathListName;
  ForcedUpdate = FALSE;
  NumTagged = 0;

  TopParentTag = BottomParentTag = -1;
  ExpertMode = Work.ExpertMode;
  MovingDown = TRUE;
  ShowParentsOnly = Work.ShowParentsOnly;

  // initialize AllFiles pointers
  AllFiles.pParentsOnly  = &ParentsOnly;
  AllFiles.pCombined     = &Combined;
  AllFiles.pTotalLines   = &TotalLines;
  AllFiles.pTotalParents = &TotalParents;

  // initialize Matches pointers
  Matches.pParentsOnly   = &ParentsOnlyMatches;
  Matches.pCombined      = &CombinedMatches;
  Matches.pTotalLines    = &TotalMatches;
  Matches.pTotalParents  = &TotalMatchingParents;

  // Initially point Current to AllFiles, changes to Matches when displaying
  // matches on screen
  Current = &AllFiles;

  New.Cnf = 0xFFFE;
  New.Dir = 0xFFFE;
  New.ShowOnScreen = FALSE;
  setnewloc(BoardNumber,DirNumber);

  CurCnf = BoardNumber;
  CurDir = DirNumber;
  Filter = SHOWALL;

  checktotallines();

  SaveFont = getfont();
  setfont(Work.BigScreen ? SMALLFONT : BIGFONT);
  drawscreen(Desc);


//LoScanNum  = 0xFFFFU;
//HiScanNum  = 0;
  HasChanged = FALSE;
  Top = Index = Column[EXPERT] = Column[NOVICE] = Column[TEXTDUPE] = 0;
  Cursor.Line = TEXTLINE;
  Cursor.LastPos = 0;
  displayline(1);
  NeedToDisplay = FALSE;

  ExitKeyNum[0]  = 60;  ExitKeyFlag[0]  = F2;
  ExitKeyNum[1]  = 61;  ExitKeyFlag[1]  = F3;
  ExitKeyNum[2]  = 62;  ExitKeyFlag[2]  = F4;
  ExitKeyNum[3]  = 63;  ExitKeyFlag[3]  = F5;
  ExitKeyNum[4]  = 64;  ExitKeyFlag[4]  = F6;
  ExitKeyNum[5]  = 65;  ExitKeyFlag[5]  = F7;
  ExitKeyNum[6]  = 66;  ExitKeyFlag[6]  = F8;
  ExitKeyNum[7]  = 67;  ExitKeyFlag[7]  = F9;
  ExitKeyNum[8]  = 32;  ExitKeyFlag[8]  = ALTD;
  ExitKeyNum[9]  = 33;  ExitKeyFlag[9]  = ALTF;
  ExitKeyNum[10] = 23;  ExitKeyFlag[10] = ALTI;
  ExitKeyNum[11] = 38;  ExitKeyFlag[11] = ALTL;
  ExitKeyNum[12] = 50;  ExitKeyFlag[12] = ALTM;
  ExitKeyNum[13] = 24;  ExitKeyFlag[13] = ALTO;
  ExitKeyNum[14] = 25;  ExitKeyFlag[14] = ALTP;
  ExitKeyNum[15] = 31;  ExitKeyFlag[15] = ALTS;
  ExitKeyNum[16] = 20;  ExitKeyFlag[16] = ALTT;
  ExitKeyNum[17] = 47;  ExitKeyFlag[17] = ALTV;
  ExitKeyNum[18] = 45;  ExitKeyFlag[18] = ALTX;
  ExitKeyNum[19] = 44;  ExitKeyFlag[19] = ALTZ;
  ExitKeyNum[20] = 21;  ExitKeyFlag[20] = ALTY;
  ExitKeyNum[21] = 48;  ExitKeyFlag[21] = ALTB;
  ExitKeyNum[22] =124;  ExitKeyFlag[22] = ALT5;
  ExitKeyNum[23] = 30;  ExitKeyFlag[23] = ALTA;
/*ExitKeyNum[24] = 71;*/ExitKeyFlag[24] = HOME;  // Activate the ExitKeyNum on the line where it is needed
  ExitKeyNum[25] = 22;  ExitKeyFlag[25] = ALTU;
  ExitKeyNum[26] = 19;  ExitKeyFlag[26] = ALTR;
  ExitKeyNum[27] = 34;  ExitKeyFlag[27] = ALTG;
  ExitKeyNum[28] = 36;  ExitKeyFlag[28] = ALTJ;
  ExitKeyNum[29] = 49;  ExitKeyFlag[29] = ALTN;

  KeyFlags = 0;
  setcursor(CUR_NORMAL);
  showkeystatus();


  while (KeyFlags != ESC) {
    if (ShowParentsOnly) {
      CurrentTotal = *Current->pTotalParents;
      ParentRecNum = Top + Index + 1;
      scale(ParentRecNum-1,CurrentTotal - 1);
      sprintf(SaveStr,"File %6ld of %6ld",ParentRecNum,CurrentTotal);
    } else {
      CurrentTotal = *Current->pTotalLines;
      CombinedRecNum = Top + Index + 1;
      scale(CombinedRecNum-1,CurrentTotal - 1);
      sprintf(SaveStr,"Line %6ld of %6ld",CombinedRecNum,CurrentTotal);
    }
    fastprint(58,1,SaveStr,Colors[DISPLAY]);

    if (NeedToDisplay)
      displayline(Top+1);

    NeedToDisplay = FALSE;
    s = &ScrnRecs[Index];
    Y = Index+5;
    Updated = FALSE;
    displaybottomline();

    switch (s->Line) {
      case DIRLINE :
           sprintf(SaveStr,"´ Size:%8ld Ã",s->Parent.Fields.FSize);
           fastprint(2,BottomLine,SaveStr,Colors[QUESTION]);
           showkeepstatus(&s->Parent,Y);
           showexiststatus(s->Parent.Fields.Exist,Y);
           if (ExpertMode) {
             if (Protected)
               Column[EXPERT]++;
             switch (Column[EXPERT]) {
               case 0:
                    SaveData = s->Parent.Fields.NCnfNum;
                    do {
                      if (s->Parent.Fields.NCnfNum == 65535U) {
                        ConfStr[0] = 'X'; ConfStr[1] = 0;
                      } else ltoa(s->Parent.Fields.NCnfNum,ConfStr,10);
                      inputstr(EXPCONF,Y, 4,"",ConfStr,ConfStr,ALLCONF,INPUT_CAPS|INPUT_CLEAR,FEDIT+0);
                      if (ConfStr[0] == 'X') {
                        setnewloc(0xFFFF,0);
                        s->Parent.Fields.NCnfNum = 0xFFFF;
                        s->Parent.Fields.NDirNum = 0;
                        Okay = 0;
                      } else {
                        setnewloc(atoi(ConfStr),s->Parent.Fields.NDirNum);
                        s->Parent.Fields.NCnfNum = New.Cnf;
                        sprintf(ConfStr,"%4u",New.Cnf);
                        fastprint(EXPCONF,Y,ConfStr,Colors[ANSWER]);
                        Okay = checkconf(New.Cnf);
                      }
                    } while (Okay == -1);

                    if (s->Parent.Fields.NCnfNum != SaveData) {
                      Updated = TRUE;
                      if (s->Parent.Fields.Keep != DELETE) {
                        if (s->Parent.Fields.NCnfNum != BoardNumber || s->Parent.Fields.NDirNum != DirNumber)
                          s->Parent.Fields.Keep = (s->Parent.Fields.Keep == COPYONLY ? COPYONLY : MOVE);
                        else
                          s->Parent.Fields.Keep = KEEP;
                      }
                      showkeepstatus(&s->Parent,Y);
                    }
                    break;
               case 1:
                    SaveData = s->Parent.Fields.NDirNum;
                    setnewloc(s->Parent.Fields.NCnfNum,s->Parent.Fields.NDirNum);
                    do {
                      inputnum(EXPDIR,Y,4,"",&s->Parent.Fields.NDirNum,vUNSIGNED,FEDIT+1);
                      setnewloc(s->Parent.Fields.NCnfNum,s->Parent.Fields.NDirNum);
                      Okay = checkdir(New.Cnf,New.Dir);
                    } while (Okay == -1);
                    if (s->Parent.Fields.NDirNum != SaveData) {
                      Updated = TRUE;
                      if (s->Parent.Fields.Keep != DELETE) {
                        if (s->Parent.Fields.NCnfNum != BoardNumber || s->Parent.Fields.NDirNum != DirNumber)
                          s->Parent.Fields.Keep = (s->Parent.Fields.Keep == COPYONLY ? COPYONLY : MOVE);
                        else
                          s->Parent.Fields.Keep = KEEP;
                      }
                      showkeepstatus(&s->Parent,Y);
                    }
                    break;
               case 2:
                    strcpy(SaveStr,s->Parent.Fields.NName);
                    inputstr(EXPNAME,Y,12,"",s->Parent.Fields.NName,s->Parent.Fields.NName,ALLNAME,INPUT_CAPS,FEDIT+2);
                    stripright(s->Parent.Fields.NName,' ');
                    Updated = strcmp(SaveStr,s->Parent.Fields.NName);
                    if (Updated)
                      ForcedUpdate = TRUE;
                    break;
               case 3:
                    stripright(s->Parent.Fields.FDesc,' ');
                    strcpy(SaveStr,s->Parent.Fields.FDesc);
                    if (SaveStr[0] == 0)
                      Cursor.LastPos = 0;
                    if (Cursor.Line != DIRLINE) {
                      if (Cursor.LastPos >= 33)
                        Cursor.LastPos -= 33;
                    }
                    Cursor.MoveDown = FALSE;
                    Cursor.LastPos = inputline(BOTHDESC,Y,45,Cursor.LastPos,s->Parent.Fields.FDesc,ALLCHAR,INPUT_WRAP,FEDIT+4);
                    Cursor.Line = DIRLINE;
                    Cursor.MoveDown = (KeyFlags == WORDWRAP && Cursor.LastPos == 45);
                    if (KeyFlags == RET)
                      Cursor.LastPos = 0;

                    stripright(s->Parent.Fields.FDesc,' ');
                    Updated = strcmp(s->Parent.Fields.FDesc,SaveStr);
                    if (Updated)
                      ForcedUpdate = TRUE;
                    break;
             }
             if (Protected)
               Column[EXPERT]--;
           } else {
             switch (Column[NOVICE]) {
               case 0:
                    strcpy(SaveStr,s->Parent.Fields.NName);
                    inputstr(NOVNAME,Y,12,"",s->Parent.Fields.NName,s->Parent.Fields.NName,ALLNAME,INPUT_CAPS,FEDIT+2);
                    stripright(s->Parent.Fields.NName,' ');
                    Updated = strcmp(SaveStr,s->Parent.Fields.NName);
                    if (Updated)
                      ForcedUpdate = TRUE;
                    break;
               case 1:
                    getdatestr(SaveStr,s->Parent.Fields.NDate);
                    SaveData = s->Parent.Fields.NDate;
                    do {
                      inputstr(NOVDATE,Y,8,"",SaveStr,SaveStr,ALLDATE,INPUT_CAPS|INPUT_CLEAR,FEDIT+3);
                      if (isdigit(SaveStr[0])) {
                        formatdate(SaveStr);
                        if (strcmp(SaveStr,"00000000") == 0)
                          strcpy(SaveStr,"01-01-80");
                        uncountrydate(SaveStr);
                      }
                      Okay = ((s->Parent.Fields.NDate = ctod2(SaveStr)) != 0);
                      if (! Okay)
                        beep();
                    } while (! Okay);
                    if (s->Parent.Fields.NDate != SaveData) {
                      Updated = TRUE;
                      ForcedUpdate = TRUE;
                      getdatestr(SaveStr,s->Parent.Fields.NDate);
                      fastprint(NOVDATE,Y,SaveStr,Colors[ANSWER]);
                    }
                    break;
               case 2:
                    stripright(s->Parent.Fields.FDesc,' ');
                    strcpy(SaveStr,s->Parent.Fields.FDesc);
                    if (SaveStr[0] == 0)
                      Cursor.LastPos = 0;
                    if (Cursor.Line != DIRLINE) {
                      if (Cursor.LastPos >= 33)
                        Cursor.LastPos -= 33;
                    }
                    Cursor.MoveDown = FALSE;
                    Cursor.LastPos = inputline(BOTHDESC,Y,45,Cursor.LastPos,s->Parent.Fields.FDesc,ALLCHAR,INPUT_WRAP,FEDIT+4);
                    Cursor.Line = DIRLINE;
                    Cursor.MoveDown = (KeyFlags == WORDWRAP && Cursor.LastPos == 45);
                    if (KeyFlags == RET)
                      Cursor.LastPos = 0;

                    stripright(s->Parent.Fields.FDesc,' ');
                    Updated = strcmp(s->Parent.Fields.FDesc,SaveStr);
                    if (Updated)
                      ForcedUpdate = TRUE;
                    break;
             }
           }
           break;

      case DUPELINE:
      case TEXTLINE:
           if (Cursor.Line == DIRLINE)
             Cursor.LastPos += 33;
           if (s->Line == DUPELINE) {
             p = (parenttype *) VMRecordGetByPos(&Parents,s->Child.ParentPos);
             sprintf(SaveStr,"´ Size:%8ld Ã",p->Fields.FSize);
             fastprint(2,BottomLine,SaveStr,Colors[QUESTION]);

             Str = s->Child.Str;
             q = strchr(Str,'|');
             Temp = (int) (q - Str) + 2;
             if (Cursor.LastPos < Temp)
               Cursor.LastPos = Temp;
           } else {
             Str = s->Parent.Str;
             Cursor.LastPos = 0;
           }
           stripright(Str,' ');
           strcpy(SaveStr,Str);
           Cursor.MoveDown = FALSE;
           ExitKeyNum[24] = 71;  // needed to handle HOME properly
           Cursor.LastPos = inputline(1,Y,78,Cursor.LastPos,Str,ALLCHAR,INPUT_WRAP,FEDIT+5);
           ExitKeyNum[24] = 0;   // disable it now
           Cursor.Line = s->Line;
           Cursor.MoveDown = (KeyFlags == WORDWRAP && Cursor.LastPos == 78);
           if (KeyFlags == RET)
             Cursor.LastPos = 0;

           stripright(Str,' ');
           fastprint(1,Y,Str,Colors[DISPLAY]);
           Updated = strcmp(Str,SaveStr);

//         if (strchr(Str,'|') != NULL && strchr(SaveStr,'|') == NULL) {
//           if (CurLine < LoScanNum) LoScanNum = CurLine;
//           if (CurLine > HiScanNum) HiScanNum = CurLine;
//         }
           break;
    }

    if (KeyFlags == DN || KeyFlags == RET)
      MovingDown = TRUE;
    else if (KeyFlags == UP)
      MovingDown = FALSE;

    if (Updated) {
      updaterecord(s);
      HasChanged = TRUE;
    }

    switch (KeyFlags) {
      case WORDWRAP:
//         if (Input.Ch == ' ')
//           Cursor.LastPos = 0;
//         else
//           insertline(s,TRUE,TRUE);

           if (ShowParentsOnly) {
             // oh oh, we're in Parents-Only mode, need to switch so that the line we are
             // about to insert will be visible on screen
             Top   = findposincombined(Current,s->Pos,s->Line) - 1;
             Index = 0;
             ShowParentsOnly = FALSE;
           }

           insertline(s,TRUE,TRUE);

           NeedToDisplay = TRUE;
           HasChanged    = TRUE;
           if (Cursor.MoveDown) {
             KeyFlags = DN;
             CurrentTotal = (ShowParentsOnly ? *Current->pTotalParents : *Current->pTotalLines);
             processcursorkeys(&Index,&Top,&Column[(ExpertMode ? EXPERT : NOVICE)],CurrentTotal,numcolumns(),NumLines);
           }
           break;

      case HOME:
           // This is a special option which only affects DUPELINEs because,
           // when HOME is pressed, instead of just homing the cursor, it
           // exits the field.  Then, right here, we set the cursor
           // position to 0 and let the DUPLINE processing fix it so that
           // the cursor is to the right of the vertical bar.
           Cursor.LastPos = 0;
           break;

      case ESC:
           if (Filter != SHOWALL) {
             resetfilter();
             NeedToDisplay = TRUE;
             KeyFlags = NOTHING;
             break;
           }

           if (NumTagged != 0) {
             beep();
             memset(&MsgData,0,sizeof(MsgData));
             MsgData.AutoBox   = TRUE;
             MsgData.Save      = TRUE;
             MsgData.Msg1      = "You have SELECTED one or more files.";
             MsgData.Line1     = Scrn_BottomRow-6;
             MsgData.Color1    = Colors[HEADING];
             MsgData.Quest     = "De-select the files";
             MsgData.QuestLine = Scrn_BottomRow-4;
             MsgData.Answer[0] = 'N';
             MsgData.Answer[1] = 0;
             MsgData.Mask      = YESNO;
             showmessage();

             if (KeyFlags == ESC) {
               KeyFlags = NOTHING;
               break;
             }

             if (MsgData.Answer[0] == 'Y') {
               removetags(TopParentTag,BottomParentTag);
               TopParentTag = BottomParentTag = -1;
               NumTagged = 0;
               NeedToDisplay = TRUE;
             }
             break;
           }

           if (Updated || HasChanged) {
             switch (editexit("Exit DIR File Editor")) {
               case 'Y': KeyFlags = ESC;                         break;
               case 'A': KeyFlags = ESC;     HasChanged = FALSE; break;
               case 'N': KeyFlags = NOTHING;                     break;
             }
           }
           break;

      case F2:
           if (s->Line == DIRLINE) {
             editdirlist(s->Parent.Fields.NCnfNum,s->Parent.Fields.NDirNum,TRUE,TRUE);
             setupscale(NumLines-1,177,5);
             if (s->Parent.Fields.NCnfNum != NewBoardNumber || s->Parent.Fields.NDirNum != NewDirNumber) {
               s->Parent.Fields.NCnfNum = NewBoardNumber;
               s->Parent.Fields.NDirNum = NewDirNumber;
               if (s->Parent.Fields.Keep != DELETE) {
                 if (s->Parent.Fields.NCnfNum != BoardNumber || s->Parent.Fields.NDirNum != DirNumber)
                   s->Parent.Fields.Keep = (s->Parent.Fields.Keep == COPYONLY ? COPYONLY : MOVE);
                 else
                   s->Parent.Fields.Keep = KEEP;
               }
               updaterecord(s);
               HasChanged = TRUE;
               NeedToDisplay = TRUE;
             }
             KeyFlags = NOTHING;
           }
           break;

      case F3:
      case F4:
           if (s->Line == DUPELINE)
             q = s->Child.Str;
           else if (s->Line == TEXTLINE)
             q = s->Parent.Str;
           else {
             q = NULL;
             if (ExpertMode) {
               switch (Column[EXPERT]) {
                 case 2: q = s->Parent.Fields.NName; break;
                 case 3: q = s->Parent.Fields.FDesc; break;
               }
             } else {
               switch (Column[NOVICE]) {
                 case 0: q = s->Parent.Fields.NName; break;
                 case 2: q = s->Parent.Fields.FDesc; break;
               }
             }
           }
           if (q != NULL) {
             if (KeyFlags == F3)
               strupr(q);
             else
               strlwr(q);

             ForcedUpdate = TRUE;
             HasChanged = TRUE;
             updaterecord(s);
           }
           break;

      case F5:
           convertdirtotext(s);
           updaterecord(s);
           updatecombined(CombinedRecNum,s->Line);
           ForcedUpdate  = TRUE;
           HasChanged    = TRUE;
           NeedToDisplay = TRUE;
           break;

      case F6:
           converttexttodir(s,BoardNumber,DirNumber);
           updaterecord(s);
           updatecombined(CombinedRecNum,s->Line);
           ForcedUpdate  = TRUE;
           HasChanged    = TRUE;
           NeedToDisplay = TRUE;
           break;

      case F7:
           if (s->Line == DIRLINE) {
             togglekeepstatus(&s->Parent.Fields,REMOVE);
             ForcedUpdate = TRUE;
             HasChanged = TRUE;
             switch (s->Parent.Fields.Keep) {
               case REMOVE: s->Parent.Fields.NDate = 0xFFFE;
                            break;
               case KEEP  : s->Parent.Fields.NDate = s->Parent.Fields.FDate;
                            break;
             }
             updaterecord(s);
             getdatestr(SaveStr,s->Parent.Fields.NDate);
             fastprint((ExpertMode ? 26 : 24),Y,SaveStr,Colors[ANSWER]);
             showkeepstatus(&s->Parent,Y);
           }
           break;

      case F8:
           if (s->Line == DIRLINE) {
             if (s->Parent.Fields.NDirNum != DirNumber || s->Parent.Fields.NCnfNum != BoardNumber) {
               togglekeepstatus(&s->Parent.Fields,COPYONLY);
               ForcedUpdate = TRUE;
               HasChanged = TRUE;
               updaterecord(s);
               showkeepstatus(&s->Parent,Y);
             } else
               indicatemustmove();
           }
           break;

      case F9:
           if (s->Line == DIRLINE) {
             togglekeepstatus(&s->Parent.Fields,DELETE);
             ForcedUpdate = TRUE;
             HasChanged = TRUE;
             updaterecord(s);
             showkeepstatus(&s->Parent,Y);
           }
           break;

      case ALT5:
           if (getfont() == BIGFONT) {
             setfont(SMALLFONT);
             if (getfont() == BIGFONT)  // font did not change, so
               break;                   // get out now
           } else {
             setfont(BIGFONT);
           }
           drawscreen(Desc);
           if (Index > NumLines - 1)
             Index = NumLines - 1;
           NeedToDisplay = TRUE;
           break;

      case ALTA:
           selectallfiles();
           TopParentTag    = 1;
           BottomParentTag = *AllFiles.pTotalParents;
           p               = getfirstdirline();
           SaveFields      = p->Fields;
           NeedToDisplay   = TRUE;
           break;

      case ALTB:
           memset(&MsgData,0,sizeof(MsgData));
           MsgData.AutoBox   = TRUE;
           MsgData.Save      = TRUE;
           MsgData.Msg1      = "In-Memory Sort";
           MsgData.Msg2      = "Sort orders:   1 = File Name (Ascending)    3 = File Name (Descending)";
           MsgData.Msg3      = "0 = No Sort    2 = File Date (Ascending)    4 = File Date (Descending)";
           MsgData.Line1     = Scrn_BottomRow-9;
           MsgData.Line2     = Scrn_BottomRow-7;
           MsgData.Line3     = Scrn_BottomRow-6;
           MsgData.Color1    = Colors[HEADING];
           MsgData.Color2    = Colors[DISPLAY];
           MsgData.Color3    = Colors[DISPLAY];
           MsgData.Quest     = "Enter Sort Order (0 or ESC to cancel)";
           MsgData.QuestLine = Scrn_BottomRow-4;
           MsgData.Answer[0] = '0';
           MsgData.Answer[1] = 0;
           MsgData.Mask      = ALLNUM;
           showmessage();

           SortType = MsgData.Answer[0] - '0';
           if (KeyFlags == ESC || SortType < 1 || SortType > 4) {
             KeyFlags = NOTHING;
             break;
           }

           if (Filter != SHOWALL) {
             resetfilter();
             NeedToDisplay = TRUE;
           }

           NeedToDisplay |= sortinplace(SortType,TRUE);
           drawscreen(Desc);

           if (NeedToDisplay) {
             if (ShowParentsOnly)
               Top = findposinparentsonly(Current,s->Pos) - 1;
             else
               Top = findposincombined(Current,s->Pos,s->Line) - 1;
             Index = 0;
           }
           break;

      case ALTD:
           deleteline(s);
           CurrentTotal = (ShowParentsOnly ? *Current->pTotalParents : *Current->pTotalLines);
           if (CurrentTotal == 0) {
             if (Filter != SHOWALL)
               resetfilter();
             if (TotalLines == 0)
               checktotallines();
           }
           if (Top+Index+1 > CurrentTotal) {
             KeyFlags = UP;
             processcursorkeys(&Index,&Top,&Column[TEXTDUPE],CurrentTotal,0,NumLines);
           }
           ForcedUpdate  = TRUE;
           NeedToDisplay = TRUE;
           HasChanged    = TRUE;
           break;

      case ALTF:
           NeedToDisplay = setfilter();
           if (NeedToDisplay) {
             Top   = 0;
             Index = 0;
           }
           break;

      case ALTG:
           if (applyrules()) {
             NeedToDisplay = HasChanged = TRUE;
             Top   = 0;
             Index = 0;
           }
           break;

      case ALTI:
           insertline(s,FALSE,FALSE);
           ForcedUpdate  = TRUE;
           NeedToDisplay = TRUE;
           HasChanged    = TRUE;
           KeyFlags      = DN;
           CurrentTotal  = (ShowParentsOnly ? *Current->pTotalParents : *Current->pTotalLines);
           processcursorkeys(&Index,&Top,&Column[(ExpertMode ? EXPERT : NOVICE)],CurrentTotal,numcolumns(),NumLines);
           break;

      case ALTJ:
           if (joinline(s)) {
             NeedToDisplay = TRUE;
             HasChanged    = TRUE;
           }
           break;

      case ALTL:
           NeedToDisplay = editfindname();
           if (NeedToDisplay) {
             Top   = 0;
             Index = 0;
           }
           break;

      case ALTM:
           if (s->Line == DIRLINE)
             p = &s->Parent;
           else if (s->Line == DUPELINE)
             p = (parenttype *) VMRecordGetByPos(&Parents,s->Child.ParentPos);
           else
             break;

           if (NumTagged == 0) {
             NeedToDisplay = changelocation(&p->Fields);
             if (NeedToDisplay) {
               updaterecord(s);
               if (TopParentTag != -1)
                 updatetagged(TopParentTag,BottomParentTag,&p->Fields);
               HasChanged = TRUE;
             }
           } else {
             changelocation(&SaveFields);   //lint !e534
             updatetagged(TopParentTag,BottomParentTag,&SaveFields);
             TopParentTag = BottomParentTag = -1;
             NumTagged = 0;
             NeedToDisplay = HasChanged = TRUE;
           }
           break;

      case ALTN:
           showfreespace();
           break;

      case ALTO:
           shelltodos();
           break;

      case ALTP:
           KeyFlags     = ESC;
           ForcedUpdate = TRUE;
           HasChanged   = TRUE;
           break;

      case ALTR:
           copyline(s);
           ForcedUpdate  = TRUE;
           NeedToDisplay = TRUE;
           HasChanged    = TRUE;
           KeyFlags      = DN;
           CurrentTotal  = (ShowParentsOnly ? *Current->pTotalParents : *Current->pTotalLines);
           processcursorkeys(&Index,&Top,&Column[(ExpertMode ? EXPERT : NOVICE)],CurrentTotal,numcolumns(),NumLines);
           break;

      case ALTS:
           if (s->Line == DIRLINE) {
//           CurParent = findposinparentsonly(Current,s->Pos);
             CurParent = findposinparentsonly(&AllFiles,s->Pos);
             if (TopParentTag == -1 || CurParent < TopParentTag)
               TopParentTag = CurParent;
             if (BottomParentTag == -1 || CurParent > BottomParentTag)
               BottomParentTag = CurParent;
             togglekeepstatus(&s->Parent.Fields,TAGGED);
             NumTagged += (s->Parent.Fields.Keep == TAGGED ? +1 : -1);
             HasChanged = TRUE;
             updaterecord(s);
             SaveFields = s->Parent.Fields;
             showkeepstatus(&s->Parent,Y);
             NeedToDisplay = gotonextparent(s,&Top,&Index,MovingDown);
           }
           break;

      case ALTT:
           if (s->Line == DIRLINE) {
             datestr(Today);
             s->Parent.Fields.NDate = TodaysDate = ctod2(Today);
             ForcedUpdate = TRUE;
             HasChanged = NeedToDisplay = TRUE;
             updaterecord(s);
           }
           break;

      case ALTU:
           if (s->Line == DIRLINE)
             p = &s->Parent;
           else if (s->Line == DUPELINE)
             p = (parenttype *) VMRecordGetByPos(&Parents,s->Child.ParentPos);
           else
             break;

           if (findfile(p->Fields.FName,DskPath,PathListName) != -1) {
             if ((Temp = extractdiz(SrchDskPath)) == 0) {
               copyfromlinebuf(s->Line == DIRLINE ? s->Pos : s->Child.ParentPos);
               ForcedUpdate = TRUE;
               HasChanged = NeedToDisplay = TRUE;
             } else {
               memset(&MsgData,0,sizeof(MsgData));
               MsgData.AutoBox   = TRUE;
               MsgData.Save      = TRUE;
               MsgData.Msg1      = ExtractErrors[Temp-1];
               MsgData.Line1     = Scrn_BottomRow-6;
               MsgData.Color1    = Colors[QUESTION];
               showmessage();
               KeyFlags = NOTHING;
             }
           }
           break;


      case ALTV:
           switch (s->Line) {
             case DIRLINE :
                  if (findfile(s->Parent.Fields.FName,DskPath,PathListName) != -1)
                    fileview(SrchDskPath);
                  break;

             case DUPELINE:
                  p = (parenttype *) VMRecordGetByPos(&Parents,s->Child.ParentPos);
                  if (findfile(p->Fields.FName,DskPath,PathListName) != -1)
                    fileview(SrchDskPath);
                  break;

             case TEXTLINE:
                  beep();
                  break;
           }
           break;

      case ALTX:
           if (ExpertMode) {
             switch (Column[EXPERT]) {
               case 0:
               case 1:
               case 2: Column[NOVICE] = 0; break;
               case 3: Column[NOVICE] = 2; break;
             }
             ExpertMode = FALSE;
           } else {
             switch (Column[NOVICE]) {
               case 0:
               case 1: Column[EXPERT] = 2; break;
               case 2: Column[EXPERT] = 3; break;
             }
             ExpertMode = TRUE;
           }
           showheading();
           setscreenlines();
           if (Index > NumLines - 1)
             Index = NumLines - 1;
           NeedToDisplay = TRUE;
           break;

      case ALTY:
           if (ShowParentsOnly) {
             Top   = findposincombined(Current,s->Pos,s->Line) - 1;
             Index = 0;
             ShowParentsOnly = FALSE;
           } else {
             Top   = findposinparentsonly(Current,s->Line == DUPELINE ? s->Child.ParentPos : s->Pos) - 1;
             Index = 0;
             ShowParentsOnly = TRUE;
           }
           NeedToDisplay = TRUE;
           break;

      case ALTZ:
           NeedToDisplay = editfindtext();
           if (NeedToDisplay) {
             Top   = 0;
             Index = 0;
           }
           break;

      default:
           switch (s->Line) {
             case DIRLINE : NeedToDisplay = processcursorkeys(&Index,&Top,&Column[(ExpertMode ? EXPERT : NOVICE)],CurrentTotal,numcolumns(),NumLines); break;
             case DUPELINE:
             case TEXTLINE: NeedToDisplay = processcursorkeys(&Index,&Top,&Column[TEXTDUPE],CurrentTotal,0,NumLines); break;
           }
           break;
    }

  }

  if (getfont() != SaveFont) {
    setfont(SaveFont);
//  drawscreen(Desc);
    clscolor(Colors[OUTBOX]);
    generalscreen("Edit DIR File","");
  }

  memset(ExitKeyNum,0,30);

  if (HasChanged)
    return(TRUE);
  else
    return(FALSE);
}
