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


// Error codes for PCBModem and MSetup program
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
// Borland C/C++ includes
#include <stdio.h>
#include <string.h>

// CDC includes
#include <misc.h>

#define  ERRTOTAL  17

char * Errors[ERRTOTAL] = {
  "Press any key to continue",
  "writing MSETUP.CNF",
  "seeking to modem",
  "reading modem data",
  "seeking",
  "identifing MODEMS.DAT structure",
  "reading manufacturers",
  "creating MSETUP.CNF",
  "creating MODEMS.DAT",
  "writing header info",
  "writing manuf data",
  "writing modem data",
  "opening MODEMS.DAT",
  "PCBOARD.DAT is missing",
  "rename batch file",
  "creating batch file",
  "removing TMP.FIL"
};

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
