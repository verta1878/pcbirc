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


#ifndef ___CNAMEIDX_H___
#define ___CNAMEIDX_H___

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

/******************************************************************************/

/* Constants for use by opencnamesidx */
/* CREATE_IDX used by buildcnamesidx, READ_IDX for getconfbyname */
#define     CREATE_IDX  TRUE
#define     READ_IDX    FALSE

    /*--------------------------------------------------------------------*/

/* Structure used by this module and (pointer) returned by findconfbystr */
#pragma pack(1)
typedef struct {
    char     Name [ 60 + 1 ];
    uint     Num;
} cnamesidxtype;
#pragma pack()

    /*--------------------------------------------------------------------*/

/* These are used internally by the index functions */
extern int           IdxHand;
extern long          IdxRecs;

    /*--------------------------------------------------------------------*/

/* #ifdef __cplusplus */
/* extern "C" { */
/* #endif */

/* These are used with buildcnamesidx & getconfbyname */
void            LIBENTRY opencnamesidx   ( bool Create );
void            LIBENTRY closecnamesidx  ( void );

/* This expects the index to be closed on entry & it will be closed on exit */
void            LIBENTRY buildcnamesidx  ( void );

/* This expects the index to be open on entry & it will be open on exit */
long            LIBENTRY getconfbyname   ( char * Name );

/* These are used with findconfbystr */
void            LIBENTRY fopencnamesidx  ( void );
void            LIBENTRY fclosecnamesidx ( void );

/* This expects the index to be open on entry & it will be open on exit */
/* NOTE: Str must be uppercase or this function will never find a match */
cnamesidxtype * LIBENTRY findconfbystr   ( char * Str );


/* findconfbyname() searches for a partial name match based on the begining of */
/* the name (i.e. if you type 'pcb' it will find 'pcboard' but it will */
/* not find alt.bbs.pcboard - it returns a value of ((unsigned) -1) to indicate */
/* that the conference was not found, otherwise it returns the INDEX RECORD */
/* NUMBER (for use with getconfbyrec()) together with the Rec structure which */
/* is filled in with the conference name and conference number. */
unsigned LIBENTRY findconfbyname(char * Name, cnamesidxtype * Rec);

/* getconfbyrec() reads an INDEX RECORD according to the number that is passed */
/* in and returns with the Rec structure filled in, or it returns -1 if an */
/* error occurs (0 if everything is okay). */
int LIBENTRY getconfbyrec(unsigned RecNum, cnamesidxtype * Rec);

/* #ifdef __cplusplus */
/* } */
/* #endif */

/******************************************************************************/

#endif

