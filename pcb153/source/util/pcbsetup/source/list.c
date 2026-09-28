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


#include <mem.h>
#include "unique.hpp"

extern "C" {
bool pascal allocatelist(int Size);
bool pascal foundinlist(char *Str);
void pascal deallocatelist(void);
}


static uniqueunlimited *List;

bool pascal allocatelist(int Size) {
  List = new uniqueunlimited(Size);

  return (List != NULL && List->Allocated);
}

bool pascal foundinlist(char *Str) {
  if (List != NULL && List->Allocated) {
    if (List->foundinlist(Str))
      return(TRUE);
    List->putinlist(Str);
  }
  return(FALSE);
}

void pascal deallocatelist(void) {
  if (List != NULL && List->Allocated)
    delete List;
}
