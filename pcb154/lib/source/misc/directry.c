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


/* this module has not been ported to OS/2 */

#if !defined(__OS2__) && !defined(__WATCOMC__)
//#pragma inline
#include "model.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


typedef struct {
  char          Attrib;
  unsigned int  Time;
  unsigned int  Date;
  unsigned long Size;
  char          Name[13];
} dtalisttype;

#define NUMOFWORDS 11        /* number of words in dtalisttype */


int pascal directory(dtalisttype *List, char Mask[], int SearchAttribute, int MaxEntries) {
  int  Total;

  asm  Cld
  asm  Push Ds
  asm  Mov  Cx,Ds
  asm  Mov  word ptr Total,0      /* used to count total files found */

  asm  Mov  Ah,2Fh                /* get the address of the current DTA  */
  asm  Int  21h                   /*                                     */
  asm  Mov  Si,Bx                 /*                                     */
  asm  Add  Si,21                 /*                                     */
  asm  Mov  Bx,Es                 /* BX:SI = address of DTA + 21 bytes   */


#ifdef LDATA                      /* Point ES:DI to the array that will  */
  asm  Les  Di,List               /* be storing the names and attributes */
#else                             /* of the files that we find           */
  asm  Mov  Ax,Ds                 /*                                     */
  asm  Mov  Es,Ax                 /*                                     */
  asm  Mov  Di,List               /*                                     */
#endif                            /*                                     */


#ifdef LDATA
  asm  Push Ds
  asm  Lds  Dx,Mask
#else
  asm  Mov  Dx,offset Mask
#endif
  asm  Mov  Cx,SearchAttribute
  asm  Mov  Ah,4Eh                /* call FINDFIRST to find first file */
  asm  Int  21h
#ifdef LDATA
  asm  Pop  Ds
#endif
  asm  Jc   done


copytolist:

  asm  Push Ds
  asm  Mov  Ds,Bx                 /* DS:SI = address of DTA+21 bytes */
  asm  Mov  Cx,NUMOFWORDS
  asm  Rep  Movsw                 /* Copy DTA to List */
  asm  Sub  Si,NUMOFWORDS*2       /* reposition SI register */
  asm  Pop  Ds

  asm  Mov  Ax,Total              /* add 1 to the total */
  asm  Inc  Ax
  asm  Mov  Total,Ax
  asm  Cmp  Ax,MaxEntries         /* check to see if array is full */
  asm  Jge  done;                 /* get out now if it is          */

findmore:

  asm  Mov  Ah,4Fh                /* call FINDNEXT to find another */
  asm  Int  21h
  asm  Jnc  copytolist            /* get out now if no more found  */

done:

  asm  Pop  Ds
  return(Total);
}


#ifdef TEST
#include <alloc.h>
#include <stdio.h>
#include <string.h>

#define LIMIT    100
#define SRCHATTR 0x1F

void main(int argc, char **argv) {
  char        Mask[66];
  int         Total;
  dtalisttype *List;
  dtalisttype *p;

  if ((List = calloc(LIMIT,sizeof(dtalisttype))) == NULL)
    return;

  if (argc < 2)
    strcpy(Mask,"*.*");
  else
    strcpy(Mask,argv[1]);

  Total = directory(List,Mask,SRCHATTR,LIMIT);

  for (p = List; Total > 0; Total--, p++) {
    puts(p->Name);
  }

  free(List);
}
#endif
#endif /* ifdef __OS2__ */