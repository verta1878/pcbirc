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


// C++ class for implementing DIR.IDX functions

#ifndef __IDXCLASS__
#define __IDXCLASS__

#define NAMESIZE   8
#define EXTSIZE    3
#define FILESIZE   NAMESIZE + EXTSIZE

typedef struct {
  char     Name[NAMESIZE];
  char     Ext[EXTSIZE];
  unsigned Date;
  unsigned LoOffset;
  char     HiOffset;
} filerectype;

typedef struct {
  long     FileSize;
  unsigned FileTime;
  unsigned FileDate;
  unsigned Latest;
  char     Reserved[6];
} idxheadertype;


class diridxclass {
  public:

    diridxclass(void);
    ~diridxclass(void) { }

    // call either create() or append() to open.
    bool pascal create(char *DirFileName);
    bool pascal append(char *DirFileName);
    void pascal close(void);

    bool pascal fill(char *FileName, char *Str, long OffsetIntoDir);
    int  pascal write(void);
    bool pascal needtoresync(void);

  private:
    bool           Opened;
    idxheadertype  IdxHeader;
    filerectype    FileRec;
    DOSFILE        IdxFile;
    char           DirName[66];
    char           IdxName[66];
};


void pascal indexchanged(void);

#endif // #ifndef __IDXCLASS__
