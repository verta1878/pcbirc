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


/******************************************************************************/
#ifndef FIDONET_HPP
#define FIDONET_HPP
#include <dir.h>

//Prototypes


#define NUMQS 7

typedef struct
{
  char         NALoc[MAXDIR];
  char         MSGLoc[MAXDIR];
  unsigned int bConf;
  unsigned int TemplateConf;
  bool         ovWr;
  bool         UseATag;
  bool         UseDesc;
}FIELDS;

void pascal fidonet(void);
void pascal processNa(FIELDS & f, bool passthru);
static void near pascal processLine(const char * line,bool passthru,FIELDS & f);
void areasBBS(void);

#endif
