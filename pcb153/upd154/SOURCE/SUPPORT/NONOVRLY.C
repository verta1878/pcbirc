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


/* this module exists merely to have a NON-OVERLAY segment of code which */
/* can execute portions of code which do not work properly under BC 5.0  */
/* but which *do* work properly with BC 3.1.                             */

#ifdef FIDO
#if __BORLANDC__ >= 0x500
#include "types.hpp"

#include "fidoque.hpp"
void LIBENTRY fido_addqueue(QUEUE_RECORD & rec) {
  cNEWQ Q;
  Q.addEntry(rec);
}

void LIBENTRY fido_scanqueue(void) {
  cNEWQ Q;
  Q.scanQueue();
}

#endif
#endif
