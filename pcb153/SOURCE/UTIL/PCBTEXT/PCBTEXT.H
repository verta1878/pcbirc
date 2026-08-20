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


#define TEXT_ARRAY 751       /* remember, 1 more than total records */

typedef struct {             /* data structure for elements in string array */
  char cNewflag;
  char cJust;
  char cLen;
  char cColor;
  char *sptStr;
} pcbtexttype;

extern pcbtexttype Array[TEXT_ARRAY];

#define PCB_BLUE          9
#define PCB_ANSIBLUE      4

#define PCB_GREEN        10
#define PCB_ANSIGREEN     2

#define PCB_CYAN         11
#define PCB_ANSICYAN      6

#define PCB_RED          12
#define PCB_ANSIRED       1

#define PCB_MAGENTA      13
#define PCB_ANSIMAGENTA   5

#define PCB_YELLOW       14
#define PCB_ANSIYELLOW    3

#define PCB_WHITE        15
#define PCB_ANSIWHITE     7

#define PCB_NOCHANGE      0
