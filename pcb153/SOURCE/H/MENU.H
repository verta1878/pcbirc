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


/****************************************************************************/
/*                                                                          */
/*                                  MENU.H                                  */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                         PCBoard 15.0 Menu System                         */
/*                                                                          */
/*==========================================================================*/
/*                                                                          */
/*                                Written by                                */
/*                            Scott Dale Robison                            */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/*                                                                          */
/*           Copyright (C) 1993 - Clark Development Company, Inc.           */
/*                                                                          */
/****************************************************************************/

#ifndef ___MENU_H___

#define ___MENU_H___

/****************************************************************************/

/* Usage Information */

/*

chkMenuList - If "MNU#####.$$$" exists sets mnuListExist, otherwise clears

doMenuList  - Looks for a file in the current directory named
              "MNU#####.$$$" (where ##### is the node number) and uses it
              to restore the state of the menu structure at the time of
              door selection

              Imports - nothing

              Exports - error code

                  -2 - a script is being processed
                       (doMenuList and doMenu are disabled during
                       PPE processing)

                  -1 - a menu is already being processed
                       (doMenuList and doMenu are not re-entrant)

                   0 - no error


doMenu      - Takes a single file name parameter for the menu
              configuration file to use

              Imports - MNU filename to process
                        Number of command line tokens

              Exports - error code

                  -2 - a script is being processed
                       (doMenuList and doMenu are disabled during
                       PPE processing)

                  -1 - a menu is already being processed
                       (doMenuList and doMenu are not re-entrant)

                   0 - no error


clearMenu   - Cleans up after the menu module (free's memory at present)

*/

/****************************************************************************/

/* Extern Variables */

extern char mnuListExist;
extern char mnuExit;
extern char displayMNU;

/****************************************************************************/

/* Function Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

void LIBENTRY chkMenuList ( void );
int  LIBENTRY doMenuList  ( void );
int  LIBENTRY doMenu      ( char * f, int nt );
void LIBENTRY clearMenu   ( void );

#ifdef __cplusplus
}
#endif

/****************************************************************************/

#endif

