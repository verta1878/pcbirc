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


// C++ class for implementing routines that track a list of non-duplicate
// items in an array.

#ifndef __UNIQUELIST__
#define __UNIQUELIST__

#ifdef  VMDATA
#ifndef VMDATA_INCLUDED
#include <vmdata.h>
#endif  // VMDATA_INCLUDED
#endif  // VMDATA


class uniquebase {
  public:
    bool         Allocated;

    uniquebase(unsigned Size) { NumNonDupes = 0; RecSize = Size; }
    ~uniquebase(void) { }

    // determine if an element already exists, if not, add it, no errors...
    bool pascal foundinlist(char *Str);

    // determine if element already exists, if not, add it, return 0
    // if successful or -1 if the element cannot be added
    int pascal putinlist(char *Str);

    // return number of items in list
    long pascal numitemsinlist(void) { return(NumNonDupes); };

    // check for element in list
    virtual bool pascal alreadyinlist(char *Str) = 0;

    // add element to list, return 0 if successful, -1 if error
    virtual int pascal addtolist(char *Str) = 0;

    // get an element from the list
    virtual char * operator[] (long i) = 0;

  protected:
    long         NumNonDupes;
    unsigned     RecSize;
};


class uniquelist : public uniquebase {
  public:
    // uniquelist constructor
    uniquelist(unsigned Size, unsigned Max);

    // uniquelist destructor
    ~uniquelist(void);

    // get an element from the list
    char * operator[] (long i) { return(Array + ((unsigned) i * RecSize)); }

  private:
    char        *Array;
    unsigned     MaxElements;

    // check for element in list
    bool pascal alreadyinlist(char *Str);

    // add element to list, return 0 if successful, -1 if error
    int pascal addtolist(char *Str);
};


#ifdef VMDATA
class uniqueunlimited : public uniquebase {
  public:
    // uniquelistunlimited constructor
    uniqueunlimited(unsigned Size);

    // uniquelistunlimited destructor
    ~uniqueunlimited(void);

    // get an element from the list
    char * operator[] (long i);

  private:
    VMDataSet    DataSet;

    // check for element in list
    bool pascal alreadyinlist(char *Str);

    // add element to list, return 0 if successful, -1 if error
    int pascal addtolist(char *Str);
};
#endif  // VMDATA

#endif  // __UNIQUELIST__
