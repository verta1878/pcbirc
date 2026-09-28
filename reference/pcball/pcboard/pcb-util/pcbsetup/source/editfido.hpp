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


#ifndef __EDITCLASS__
#include "edit.hpp"
#endif

#ifndef __EDITFIDOCLASS__
#define __EDITFIDOCLASS__


// The editfidoclass is slightly different from editclass, mostly in the way
// that the information is loaded into memory and saved back out.

class editfidoclass : public editclass {
  public:
    editfidoclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) { }

    ~editfidoclass(void);
    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal handlekeys(void *Rec) { editclass::handlekeys(Rec); }
    int  pascal load(char *Name, void **pArray, unsigned *pCount, int flags);
    void pascal save(void);

//  virtual void pascal loadrecords(void) = 0;
//  virtual void pascal saverecords(void) = 0;

  protected:
    void      **Array;        // pointer to pointer to allocated memory
    unsigned   *ArrayCount;   // pointer to counter for # records in array
};

#endif
