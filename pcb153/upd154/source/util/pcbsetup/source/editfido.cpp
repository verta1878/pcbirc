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


#include <mem.h>
#include <malloc.h>
#include <misc.h>
#include "fconfig.h"
#include "editfido.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif

editfidoclass::~editfidoclass(void) {
  free_fido_memory();
}


int pascal editfidoclass::load(char *Name, void **pArray, unsigned *pCount, int flags) {
  editclass::FileName = Name;

  //if(fileexist(Name)==255)
  //  make_fido_config();

  if(!read_fido_config(flags))
    return(-1);

  Array      = pArray;
  ArrayCount = pCount;

  loadrecords(NULL);

  if (TotalRecs == 0) {
    newrecord(SaveRec);
    addrecord(SaveRec);
  }

  return(0);
}


void pascal editfidoclass::save(void) {
  saverecords(NULL);
  write_fido_config();
  free_fido_memory();
}


#pragma argsused
void pascal editfidoclass::loadrecords(DOSFILE *File) {
  int X;

  for (X = 0; X < *ArrayCount; X++) {
    memcpy(SaveRec,(char *) *Array + (X * RecSize),RecSize);
    addrecord(SaveRec);
  }
}


#pragma argsused
void pascal editfidoclass::saverecords(DOSFILE *File) {
  int    X;

  if(Array) free(*Array);
  if ((*Array = malloc((int) TotalRecs * RecSize)) == NULL)
    return;

  for (X = 0; X < TotalRecs; X++)
    memcpy((char *) *Array + (X * RecSize),getrecord(X+1),RecSize);

  *ArrayCount = X;
}
