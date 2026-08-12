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


#include <alloc.h>
#include <string.h>
#include <scrnio.h>
#include "unique.hpp"
#ifdef DEBUG
#include <memcheck.h>
#define mallochk malloc
#endif


// determine if an element already exists, if not, add it, no errors...
bool  uniquebase::foundinlist(char *Str) {
  if (Allocated) {
    if (alreadyinlist(Str))
      return(TRUE);

    addtolist(Str);     //lint !e534
  }
  return(FALSE);
}


// determine if element already exists, if not, add it, return 0
// if successful or -1 if the element cannot be added
int  uniquebase::putinlist(char *Str) {
  if (Allocated) {
    if (alreadyinlist(Str))
      return(0);

    return(addtolist(Str));
  }
  return(-1);
}



// uniquelist constructor
uniquelist::uniquelist(unsigned Size, unsigned Max) : uniquebase(Size) {
  Allocated = FALSE;

  if ((long) Max * (long) Size < 65535U) {
    if ((Array = (char *) mallochk(Max * Size)) != NULL) {
      Allocated = TRUE;
      MaxElements = Max;
    }
  }
}


// uniquelist constructor
uniquelist::~uniquelist(void) {
  if (Allocated) {
    free(Array);
    Allocated = FALSE;
  }
}


// check for element in list
bool  uniquelist::alreadyinlist(char *Str) {
  int  Count;
  char *p;

  if (Str[0] == 0)
    return(TRUE);

  for (Count = 0, p = Array; Count < NumNonDupes; Count++, p += RecSize) {
    if (stricmp(p,Str) == 0)
      return(TRUE);
  }
  return(FALSE);
}

// add element to list, return 0 if successful, -1 if error
int  uniquelist::addtolist(char *Str) {
  if (NumNonDupes < MaxElements) {
    strcpy(Array + (int) (NumNonDupes * RecSize),Str);
    NumNonDupes++;
    return(0);
  }
  return(-1);
}


#ifdef VMDATA
// uniquelistunlimited constructor
uniqueunlimited::uniqueunlimited(unsigned Size) : uniquebase(Size) {
  Allocated = TRUE;
  VMInitRec(&DataSet,NULL,0,Size);
}


// uniquelist constructor
uniqueunlimited::~uniqueunlimited(void) {
  if (Allocated) {
    VMDone(&DataSet);
    Allocated = FALSE;
  }
}


// check for element in list
bool  uniqueunlimited::alreadyinlist(char *Str) {
  long  Count;
  char *p;

  if (Str[0] == 0)
    return(TRUE);

  for (Count = 1; Count <= NumNonDupes; Count++) {
    p = (char *) VMRecordGetByIndex(&DataSet,Count,NULL);
    if (stricmp(p,Str) == 0)
      return(TRUE);
  }
  return(FALSE);
}

// add element to list, return 0 if successful, -1 if error
int  uniqueunlimited::addtolist(char *Str) {
  char *p;

  p = (char *) VMRecordCreate(&DataSet,RecSize,NULL,NULL);
  strcpy(p,Str);
  NumNonDupes++;
  return(0);
}

// get an element from the list
char * uniqueunlimited::operator[] (long i) {
  return((char *) VMRecordGetByIndex(&DataSet,i+1,NULL));
}
#endif // VMDATA



#ifdef TEST
#include <stdio.h>

void main(void) {
  uniquelist       Small(80,3);
  #ifdef VMDATA
  uniqueunlimited  Big(80);
  VMDataStartUp("VMDATA.$$$",64,64,VM_FALSE);
  #endif

  Small.putinlist("ABC");
  Small.putinlist("ABC");
  printf("Items in list %ld\n",Small.numitemsinlist());
  printf("Should only be 1 because a second, non-unique, item was passed\n");

  Small.putinlist("DEF");
  Small.putinlist("GHI");
  Small.putinlist("JKL");
  printf("\nItems in list %ld\n",Small.numitemsinlist());
  printf("Should only be 3 because the constructor was passed 80,3\n");


  #ifdef VMDATA
  Big.putinlist("ABC");
  Big.putinlist("ABC");
  printf("\nItems in list %ld\n",Big.numitemsinlist());
  printf("Should only be 1 because a second, non-unique, item was passed\n");

  Big.putinlist("DEF");
  Big.putinlist("GHI");
  Big.putinlist("JKL");
  printf("\nItems in list %ld\n",Big.numitemsinlist());
  printf("Should be 4 - list is unlimited\n");
  #endif
}

#endif
