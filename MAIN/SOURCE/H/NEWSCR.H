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
/*                                                                            */
/*                                  NEWSCR.H                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*     Header file for PCBoard to access the Version 15.0 Script Language     */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

#ifndef ___NEWSCR_H___

#define ___NEWSCR_H___

/******************************************************************************/

/* Function Interface Documentation */

/*

doScript accepts two parameters and returns a completion status:

    Import parameters:

        scrFile - The filename of the script to execute
        ansFile - The filename to append script answers to
                  (NULL for no answer filename)
        numToks - Count of waiting tokens from previous tokenize
                  function call

    Export parmeters:

        -2 - doScript called while another script executing
             (doScript is not re-entrant)
        -1 - Error occurred executing script (ansFile ignored)
         0 - No error occurred executing script, ansFile not appended to
             (ansFile either NULL or script exited via STOP statement)
         1 - No error occurred executing script, ansFile appended to


cleanupScript accepts no parameters and returns no parameters

*/

/******************************************************************************/

/* Function Prototypes */

// #ifdef __cplusplus
// extern "C" {
// #endif

int  LIBENTRY doScript      (char * scrFile, char * ansFile, int numToks);
void LIBENTRY cleanupScript (void);

// #ifdef __cplusplus
// }
// #endif

/******************************************************************************/

#endif

