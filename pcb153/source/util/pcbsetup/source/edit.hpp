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


// C++ class for implementing editing functions using VMData

#ifndef __EDITCLASS__
#define __EDITCLASS__

#ifndef H_SCREEN
#include <screen.h>
#endif

#ifndef H_DOSFUNC
#include <dosfunc.h>
#endif

#ifndef H_SCRNIO
#include <scrnio.h>
#endif

#ifndef VMDATA_INCLUDED
#include <vmdata.h>
#endif  // VMDATA_INCLUDED


enum { ALTI=FLAG1, ALTD, ALTR, ALT5, NEXTKEY};

typedef struct {
  long  Pos;
  void *Rec;
} scrnrectype;


class editclass {
  public:

    editclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys);
    ~editclass(void);

    void   pascal setupscreen(void);
    void   pascal insertrecord(long CurRec);
    void   pascal deleterecord(long CurRec);
    void   pascal addrecord(void *Rec);
    void * pascal getrecord(long RecNum);
    void   pascal updaterecord(void *Rec, long Pos);
    int    pascal load(char *Name);
    void   pascal paint(void);
    void   pascal displayscreen(long TopRec);
    void   pascal addexitkey(char KeyNum, int Flag, char *Desc);
    bool   pascal edit(void);

    void * operator[] (long RecNum) { return(getrecord(RecNum)); }

    virtual void pascal newrecord(void *Rec) = 0;
    virtual void pascal loadrecords(DOSFILE *File) = 0;
    virtual void pascal saverecords(DOSFILE *File) = 0;
    virtual void pascal showheaders(void) = 0;
    virtual void pascal showfooters(void);
    virtual void pascal displayrecord(long RecNum, int LineNum, void *Rec) = 0;
    virtual void pascal editrecord(void *Rec) = 0;
    virtual void pascal handlekeys(void *Rec);
    virtual void pascal save(void);

    bool      Allocated;     // TRUE if construction was successful
    VMDataSet DataSet;       // the actual dataset for the data being edited
    VMDataSet Idx;           // an index of positions within the dataset
    long      TotalRecs;     // total number of records in the main dataset
    long      TotalIdx;      // total number of records in the index
    int       CurNumLines;   // current number of lines on screen

    bool      NeedToDisplay; // TRUE if need to redisplay the screen
    long      CurRec;        // Current record number
    long      TopRec;        // Record number at the top of the screen
    int       Index;         // Index into screen display (top = 0)
    int       LineNum;       // Current line number on screen
    int       Column;        // Current column number (0-based)
    int       NumColumns;    // Number of columns on screen (0-based)
    char     *Title;         // Title shown on the screen
    bool      ExitEditor;    // TRUE means to we're done editing


    scrnrectype *Screen;

  protected:
    int         MaxScrnLines;
    int         LeftSide;
    int         RightSide;
    int         TopLine;
    int         KeepBottom;
    bool        Changed;
    char       *FileName;
    void       *SaveRec;
    int         RecSize;

  private:
    int           NumExitKeys;
    int           NextExitKey;
    char         *SaveKeyNum;
    int          *SaveKeyFlag;
    char        **KeyDesc;
    savescrntype  ScrnBuf;

    void pascal addidx(long NewPos);
    void pascal fillscreen(long TopRec);
};

#endif __EDITCLASS__
