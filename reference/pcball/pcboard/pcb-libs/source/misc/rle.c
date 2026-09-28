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


#include <misc.h>
#ifndef __OS2__
  #include <model.h>
  #pragma inline
#endif

unsigned LIBENTRY doRLE(char *Dest, char *Srce, unsigned Len) {
#ifdef __OS2__
  unsigned Count;
  int      CountZeroes;
  char     Byte;

  for (Count = 0; Len > 0; ) {
    Byte = *Srce++;
    *Dest++ = Byte;
    Count++;
    Len--;
    if (Byte == 0) {
      for (CountZeroes = 1; *Srce == 0 && Len > 0 && CountZeroes < 255; Len--, Srce++)
        CountZeroes++;
      *Dest++ = (char) CountZeroes;
      Count++;
    }
  }

  return(Count);

#else  /* ifdef __OS2__ */

  asm  mov   dx,Len     /* get the length of the srce (input) */
  asm  or    dx,dx      /* check for zero length srce */
  asm  jnz   begin      /* begin processing if not zero */
  return(0);            /*   else return zero bytes in dest */

begin:
#ifdef LDATA
  asm  push  ds
  asm  les   di,Dest    /* load source and destination registers */
  asm  lds   si,Srce
#else
  asm  mov   ax,ds
  asm  mov   es,ax
  asm  mov   di,Dest
  asm  mov   si,Srce
#endif

  asm  cld
  asm  xor   bx,bx      /* zero a counter to count bytes in dest (output) */

top:
  asm  lodsb            /* get byte from srce, increment srce ptr */
  asm  stosb            /* copy to dest, increment dest ptr */
  asm  inc   bx         /* increment dest counter */
  asm  dec   dx         /* decrement srce counter */
  asm  or    al,al      /* is it a ZERO byte? */
  asm  je    RLE        /*   yes, go count zeroes */

  asm  or    dx,dx      /* check to see if srce bytes are finished */
  asm  jz    done       /*   yes, get out now */
  asm  jmp   short top  /*   no, loop back up for more bytes */

RLE:
  asm  xor   cx,cx      /* initialize counter to zero */
RLEloop:
  asm  inc   cx         /* count the zero */
  asm  or    dx,dx      /* have we read in the last byte? */
  asm  jz    store      /*   yes, go store the count */
  asm  lodsb            /* get the next byte from srce */
  asm  or    al,al      /* is it a zero? */
  asm  jz    RLEloop    /*   yes, go back up and scan for more */
  asm  dec   si         /*   no, we overscanned one byte, go back one */
store:
  asm  mov   es:[di],cl /* store the count of zeroes in dest */
  asm  inc   di         /* increment dest ptr */
  asm  inc   bx         /* increment the dest counter */
  asm  dec   cx         /* "un" count the last byte we scanned */
  asm  sub   dx,cx      /* decrement srce counter by # of zero bytes */
  asm  or    dx,dx      /* check to see if srce bytes are finished */
  asm  jnz   top        /*   no, loop back up for more bytes */

done:
#ifdef LDATA
  asm  pop   ds
#endif
  asm  mov   ax,bx      /* put total bytes in dest in return register */
  return(_AX);
#endif  /* ifdef __OS2__ */
}


unsigned LIBENTRY unRLE(char *Dest, char *Srce, unsigned Len) {
#ifdef __OS2__
  char     Byte;
  unsigned Count;
  unsigned Zeroes;

  for (Count = 0; Len > 0; ) {
    Byte = *Srce++;
    Len--;
    *Dest++ = Byte;
    Count++;
    if (Byte == 0) {
      Zeroes = *Srce++ - 1;
      Count += Zeroes;
      for (; Zeroes; Zeroes--)
        *Dest++ = 0;
      Len--;
    }
  }

  return(Count);

#else  /* ifdef __OS2__ */

  asm  mov   dx,Len     /* get the length of the srce (input) */
  asm  or    dx,dx      /* check for zero length srce */
  asm  jnz   begin      /* begin processing if not zero */
  return(0);            /*   else get out without doing anything */

begin:
#ifdef LDATA
  asm  push  ds
  asm  les   di,Dest    /* load source and destination registers */
  asm  lds   si,Srce
#else
  asm  mov   ax,ds
  asm  mov   es,ax
  asm  mov   di,Dest
  asm  mov   si,Srce
#endif

  asm  cld
  asm  xor   cx,cx      /* make sure all of cx (hi/lo bytes) is zeroed */
  asm  xor   bx,bx      /* keep a count of bytes in output buffer */

top:
  asm  lodsb            /* get byte from srce, increment srce ptr */
  asm  stosb            /* copy to dest, increment dest ptr */
  asm  inc   bx         /* count the byte that we just stored */
  asm  dec   dx         /* decrement srce counter */
  asm  or    al,al      /* is it a ZERO byte? */
  asm  je    unRLE      /*   yes, go expand the zeroes */
  asm  or    dx,dx      /* check to see of srce bytes are finished */
  asm  jnz   top        /*   no, loop back up for more bytes */

done:
#ifdef LDATA
  asm  pop   ds
#endif
  asm  mov   ax,bx      /* return the number of bytes in the output */
  return(_AX);

unRLE:                  /* remember cx was initialized to zero */
  asm  mov   cl,[si]    /* get the count of zeroes */
  asm  dec   cl         /* don't count the first zero, already copied it */
  asm  add   bx,cx      /* count the zeroes we're going to be adding */
  asm  rep   stosb      /* store that many zeroes in dest */
  asm  inc   si         /* point to next character in srce */
  asm  dec   dx         /* decrement srce counter */
  asm  or    dx,dx      /* check to see of srce bytes are finished */
  asm  jnz   top;       /* if there is more to do, loop back up */
  goto done;            /* otherwise, we're done */
#endif  /* ifdef __OS2__ */
}


#ifdef TEST
#include <mem.h>
#ifdef __BORLANDC__
  #include <alloc.h>
#else
  #include <malloc.h>
#endif
#include <stdio.h>
#include <dosfunc.h>

/* do a whole buffer at a time */
void pascal test1(char *Srce, char *Dest, char *Compare, unsigned SrceLen) {
  unsigned DestLen;

  /* go compress it */
  DestLen = doRLE(Dest,Srce,SrceLen);

  /* then uncompress it */
  unRLE(Srce,Dest,DestLen);

  /* print the results */
  printf("SrceLen = %3d, DestLen = %3d, RLE/unRLE was ... %s\n",
         SrceLen,
         DestLen,
         memcmp(Srce,Compare,SrceLen) == 0 ? "Successful!" : "Miserable!");
}


/* split the buffer up and process it in chunks */
void pascal test2(char *Srce, char *Dest, char *Compare, unsigned SrceLen) {
  char Buffer[4];
  int  InBytes;
  int  BufBytes;
  int  CompressedBytes;
  char *p;
  char *Walk;

  /* compress it first, doing a little bit at a time, walking the Walk */
  /* pointer thru the Dest buffer while we do it, meanwhile the p pointer */
  /* is what walks through the Srce buffer */

  Walk = Dest;
  for (InBytes = 0, p = Srce; InBytes < SrceLen; ) {
    for (BufBytes = 0; BufBytes < sizeof(Buffer); ) {
      Buffer[BufBytes] = *p;
      p++;
      InBytes++;
      BufBytes++;
      if (InBytes >= SrceLen)
        break;
    }
    Walk += doRLE(Walk,Buffer,BufBytes);
  }


  CompressedBytes = (int) (Walk - Dest);

  /* for debugging purposes, initialize the whole thing with a value */
  memset(Srce,0xFC,SrceLen);

  Walk = Srce;

  for (InBytes = 0, p = Dest; InBytes < CompressedBytes; ) {
    for (BufBytes = 0; BufBytes < sizeof(Buffer)-1; ) {
      Buffer[BufBytes] = *p;
      p++;
      InBytes++;
      BufBytes++;
      if (InBytes >= CompressedBytes)
        break;
    }
    if (*(p-1) == 0) {
      Buffer[BufBytes] = *p;
      p++;
      InBytes++;
      BufBytes++;
    }
    Walk += unRLE(Walk,Buffer,BufBytes);
  }

  /* print the results */
  printf("SrceLen = %3d, DestLen = %3d, RLE/unRLE was ... %s\n",
         SrceLen,
         CompressedBytes,
         memcmp(Srce,Compare,SrceLen) == 0 ? "Successful!" : "Miserable!");
}


void pascal alltests(char *Srce, unsigned SrceLen, char *TestName) {
  unsigned  AllocLen;
  char     *Dest;
  char     *Temp;

  /* make room for worst case scenario */
  AllocLen = SrceLen * 2;

  if ((Dest = (char *) malloc(AllocLen)) != NULL) {

    /* for debugging purposes, initialize the whole thing with a value */
    memset(Dest,0xFF,AllocLen);

    if ((Temp = (char *) malloc(AllocLen)) != NULL) {

      /* for debugging purposes, initialize the whole thing with a value */
      memset(Temp,0xFE,AllocLen);

      /* make a temporary copy of the original and use it instead so that    */
      /* we can make sure that the srce doesn't get clobbered during testing */
      memcpy(Temp,Srce,SrceLen);

      printf("%s - Single  : ",TestName);
      test1(Temp,Dest,Srce,SrceLen);

      printf("%s - Multiple: ",TestName);
      test2(Temp,Dest,Srce,SrceLen);
      free(Temp);
    }
    free(Dest);
  }
}


char Test1[6]   = {0,1,2,3,4,5};
char Test2[7]   = {0,0,1,2,3,4,5};
char Test3[8]   = {0,0,0,1,2,3,4,5};
char Test4[6]   = {5,4,3,2,1,0};
char Test5[7]   = {5,4,3,2,1,0,0};
char Test6[8]   = {5,4,3,2,1,0,0,0};
char Test7[8]   = {0,1,0,1,0,1,0,1};
char Test8[9]   = {0,1,0,1,0,1,0,1,0};
char Test9[9]   = {1,2,3,0,0,0,3,2,1};
char Test10[9]  = {1,2,0,0,0,0,0,2,1};
char Test11[13] = {0,0,1,2,0,0,0,0,0,2,1,0,0};
char Test12[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};


void main(void) {
  alltests("DAVID TERRY",12,"STRING");
  alltests(Test1 ,sizeof(Test1) ,"TEST1");
  alltests(Test2 ,sizeof(Test2) ,"TEST2");
  alltests(Test3 ,sizeof(Test3) ,"TEST3");
  alltests(Test4 ,sizeof(Test4) ,"TEST4");
  alltests(Test5 ,sizeof(Test5) ,"TEST5");
  alltests(Test6 ,sizeof(Test6) ,"TEST6");
  alltests(Test7 ,sizeof(Test7) ,"TEST7");
  alltests(Test8 ,sizeof(Test8) ,"TEST8");
  alltests(Test9 ,sizeof(Test9) ,"TEST9");
  alltests(Test10,sizeof(Test10),"TEST10");
  alltests(Test11,sizeof(Test11),"TEST11");
  alltests(Test12,sizeof(Test12),"TEST12");
}
#endif
