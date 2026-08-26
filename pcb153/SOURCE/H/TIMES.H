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


#ifndef TIMES_DOT_H
#define TIMES_DOT_H

/*****************************************************************************/
/* Timer values                                                              */
/* These are mapped to David Terry's platform safe macros. */

#define         ONE_SECOND              ONESECOND
#define         TWO_SECONDS             TWOSECONDS
#define         THREE_SECONDS           THREESECONDS
#define         FIVE_SECONDS            FIVESECONDS
#define         TEN_SECONDS             TENSECONDS
#define         FIFTEEN_SECONDS         (TENSECONDS + FIVESECONDS)
#define         TWENTY_SECONDS          TWENTYSECONDS
#define         THIRTY_SECONDS          THIRTYSECONDS
#define         FOURTY_SECONDS          (TWENTYSECONDS + TWENTYSECONDS)
#define         SIXTY_SECONDS           SIXTYSECONDS
#define         TWO_MINUTES             TWOMINUTES

#endif

