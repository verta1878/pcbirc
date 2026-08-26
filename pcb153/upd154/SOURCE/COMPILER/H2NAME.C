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


/***************************************************************
 *
 *     H2NAME.C - Jim Kyle
 *
 *  Compile only with large memory model:
 *        tcc -ml h2name
 *        cl -AL h2name.c
 *
 ***************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>
#ifdef __TURBOC__
#include <mem.h>
#else
#include <memory.h>
#endif

#ifndef MK_FP
#define MK_FP(s,o) ((void far *)\
    (((unsigned long)(s) << 16) | (unsigned)(o)))
#endif

char * pascal h2name( unsigned psp, int h )
{ static char name[15];  /* will hold file's name           */
  signed char far * htbl;
  unsigned far *ptr, nmofs;
  char far *sptr;
  int sftn, sftsize;
  union REGS regs;
  struct SREGS sregs;

  _fmemset( name, 0, 15 ); /* blank out the static name       */
  
  /* create pointer to handle table (JFT)                   */
  htbl = *((signed char far * far *) MK_FP(psp, 0x34));

  regs.h.ah = 0x52;      /* set up initial SFT pointer      */
  segread( &sregs );
  intdosx( &regs, &regs, &sregs );
  ptr = *((unsigned far * far *) MK_FP( sregs.es, regs.w.bx + 4 ));

  switch( _osmajor )     /* switch sizes, offsets for ver   */
    { case 2: sftsize = 0x28;
              nmofs = 4; /* offset of 11-byte name area     */
              break;

      case 3: sftsize = 0x35;
              nmofs = 0x20;   /* offset of name area        */
              break;

      case 4: 
      case 5:
      case 6: sftsize = 0x3B;
              nmofs = 0x20;   /* offset of name area        */
              break;

      default: return name;   /* returns null string        */
    }

  if (htbl[h] >= 0)           /* now if handle is valid...  */
    { sftn = htbl[h];         /* get index into SFT list    */
      while ( ((unsigned long)(ptr) & 0xFFFF) != 0xFFFF )
        { if (ptr[2] > sftn)  /* then target is here        */
            { sptr = (char *)&ptr[3];
              while (sftn--)  /* so skip down to it         */
                sptr += sftsize;
              _fmemcpy( name, &sptr[nmofs], 11 );
              return name;    /* found and copied, done     */
            }
          sftn -= ptr[2];     /* not here, reduce index     */
          ptr = (unsigned int far *) MK_FP( ptr[1], ptr[0] );
        }
    }
  strcpy( name, "UNKNOWN" );
  return name;                /* reached only by error      */
}

#ifdef ___TEST___

void main( int argc, char *argv[] )
{ 
    unsigned psp;
    int max_files;
    int i;
    
    if (argc < 2)
        psp = _psp;              /* display files for this program */
    else
        sscanf(argv[1], "%X", &psp); /* take PSP from command line */
    
    if (_osmajor >= 3)
        max_files = *((unsigned far *) MK_FP(psp, 0x32));
    else
        max_files = 20;
    
    fprintf(stderr, "Files for %04X\n", psp);
    
    for (i=0; i<max_files; i++)
        fprintf(stderr, "%2d ==> %s\n", i, h2name(psp, i));
}

#endif

