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



/****************************************************************************
*                               CONVERT.CPP
*                         Written by:Stan Paulsen
*                     Copyright Clark Development 1995
*
*
* This is a utility to convert the file PCBFIDO.CFG into component parts
* March 1995: Convert from version 2 to version 3
*****************************************************************************/
#ifndef CONVERT_HPP
#define CONVERT_HPP
// Prototypes

void pascal convert(void);


typedef struct
{
  char          incoming_packets[MAXDIR];
  char          outgoing_packets[MAXDIR];
  char          outgoing_msg[MAXDIR];
  char          bad_packets[MAXDIR];
  char          nodelist_path[MAXDIR];
  char          work_directory[MAXDIR];
} OLDDIRECTORIES;
#endif
