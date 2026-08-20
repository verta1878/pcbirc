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


#include    <string.h>

#if __BORLANDC__ < 0x500
typedef char bool;
#endif

bool pascal dosreservedword(char *Name) {
   return(strstr(Name,"CLOCK$") != NULL ||
          (memcmp(Name,"COM",3) == 0 && (Name[3] >= '1' && Name[3] <= '4') &&
             (Name[4] == ':' || Name[4] == 0)) ||
          (memcmp(Name,"LPT",3) == 0 && (Name[3] >= '1' && Name[3] <= '3') &&
             (Name[4] == ':' || Name[4] == 0)) ||
          strcmp(Name,"AUX") == 0 ||
          strcmp(Name,"PRN") == 0 ||
          strcmp(Name,"CON") == 0);
}

