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


/******************************************************************************
                                PTSETUP.HPP
                          Written by: Stan Paulsen

                 Copyright (C) 1995 Clark Development, inc.

 This module contains prototypes related to processing the FIDONET.NA file

 ******************************************************************************/
#ifndef _PTSETUP_HPP
#define _PTSETUP_HPP
// Prototypes
void pascal ptsetup(void);
void pascal setupAreas(void);
void pascal regUsers(void);
void pascal setupAreasNA(void);
void pascal setupAreasMan(void);
void pascal addPArea(const char * tag);
#endif


