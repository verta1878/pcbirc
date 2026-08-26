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


#ifdef __OS2__
  #include <stdlib.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
/*  #pragma  inline */
  #endif
  #include "model.h"
#endif

#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef __OS2__
static inline short _NEAR_ LIBENTRY srotl(uint Val, uint RotVal) {
  #ifdef __BORLANDC__
    return(_rotl(Val,RotVal));
  #else
    int LongVal = Val;
    LongVal = _rotl(LongVal,RotVal);
    Val = (LongVal & 0xFFFF) | (LongVal >> 16);
    return(Val);
  #endif
}

static inline ubyte _NEAR_ LIBENTRY crotl(ubyte Val, uint RotVal) {
  #ifdef __BORLANDC__
    return(_crotl(Val,RotVal));
  #else
    uint ShortVal = Val;
    ShortVal = _rotl(ShortVal,(RotVal & 7));
    Val = (ShortVal & 0xFF) | ((ShortVal & 0xFF00) >> 8);
    return(Val);
  #endif
}

static inline short _NEAR_ LIBENTRY srotr(uint Val, uint RotVal) {
  #ifdef __BORLANDC__
    return(_rotr(Val,RotVal));
  #else
    int LongVal = Val;
    LongVal = _rotr(LongVal,RotVal);
    Val = (LongVal & 0xFFFF) | (LongVal >> 16);
    return(Val);
  #endif
}

static inline ubyte _NEAR_ LIBENTRY crotr(ubyte Val, uint RotVal) {
  #ifdef __BORLANDC__
    return(_crotr(Val,RotVal));
  #else
    int LongVal = Val;
    LongVal = _rotr(LongVal,(RotVal & 7));
    Val = (LongVal & 0xFF) | ((LongVal & 0xFF000000) >> 24);
    return(Val);
  #endif
}
#endif


void LIBENTRY encrypt(char *Str, int Len) {
#ifdef __OS2__
  unsigned char   RotVal;
  unsigned short  Seed;
  unsigned short *p;
  int             X;

  Seed = 0xDB24;
  for (p = (unsigned short *) Str, X = Len >> 1; X > 0; X--, p++) {
    RotVal = (char) ((char) Seed + (char) X);
    Seed = (unsigned short) (srotl(*p,RotVal) ^ Seed);
    *p = Seed;
  }

  if (Len & 1) {
    Str = (char *) p;
    *Str = (char) (crotl(*Str,RotVal) ^ Seed);
  }

#else  /* ifdef __OS2__ */

#ifdef LDATA
  asm push  ds
  asm lds   si,Str
  asm les   di,Str
#else
  asm mov   si,Str
  asm mov   ax,ds
  asm mov   es,ax
  asm mov   di,si
#endif

  asm cld
  asm mov   bx,0DB24h
  asm mov   dx,Len
  asm shr   dx,1

top:
  asm or    dx,dx
  asm jz    done
  asm lodsw
  asm mov   cl,bl
  asm add   cl,dl
  asm rol   ax,cl
  asm xor   ax,bx
  asm stosw
  asm mov   bx,ax
  asm dec   dx
  asm jmp   short top

done:
  asm test  word ptr Len,1
  asm jz    exit
  asm lodsb
  asm rol   al,cl
  asm xor   al,bl
  asm stosb

exit:;
#ifdef LDATA
  asm pop   ds
#endif
#endif  /* ifdef __OS2__ */
}


void LIBENTRY decrypt(char *Str, int Len) {
#ifdef __OS2__
  unsigned char   RotVal;
  unsigned short  Temp;
  unsigned short  Seed;
  unsigned short *p;
  int             X;

  Seed = 0xDB24;
  for (p = (unsigned short *) Str, X = Len >> 1; X > 0; X--, p++) {
    Temp = (short) (*p ^ Seed);
    RotVal = (char) ((char) Seed + (char) X);
    Seed = *p;
    *p = srotr(Temp,RotVal);
  }

  if (Len & 1) {
    Str = (char *) p;
    *Str = crotr((unsigned char) (*Str ^ (unsigned char) Seed),RotVal);
  }

#else  /* ifdef __OS2__ */

#ifdef LDATA
  asm push  ds
  asm lds   si,Str
  asm les   di,Str
#else
  asm mov   si,Str
  asm mov   ax,ds
  asm mov   es,ax
  asm mov   di,si
#endif

  asm cld
  asm mov   bx,0DB24h
  asm mov   dx,Len
  asm shr   dx,1

top:
  asm or    dx,dx
  asm jz    done
  asm lodsw
  asm mov   cx,ax
  asm xor   ax,bx
  asm xchg  bx,cx
  asm add   cl,dl
  asm ror   ax,cl
  asm stosw
  asm dec   dx
  asm jmp   short top

done:
  asm test  word ptr Len,1
  asm jz    exit
  asm lodsb
  asm xor   al,bl
  asm ror   al,cl
  asm stosb

exit:;
#ifdef LDATA
  asm pop   ds
#endif
#endif  /* ifdef __OS2__ */
}



void LIBENTRY encrypt2(char *Str, int Len) {
#ifdef __OS2__
  unsigned char   RotVal;
  unsigned short  Seed;
  unsigned short  Temp;
  unsigned short *p;
  int             X;

  Seed = 0xDB24;
  for (p = (unsigned short *) Str, X = Len >> 1; X > 0; X--, p++) {
    Temp = (unsigned short) ((X & 0xFF) + ((X & 0xFF) << 8));
    RotVal = (char) ((char) Seed + (char) X);
    *p = srotl(((*p ^ Seed) ^ Temp),RotVal);
    Seed = *p;
  }

  if (Len & 1) {
    Str = (char *) p;
    *Str = (char) (crotl(*Str,RotVal) ^ Seed);
  }

#else  /* ifdef __OS2__ */

#ifdef LDATA
  asm push  ds
  asm lds   si,Str
  asm les   di,Str
#else
  asm mov   si,Str
  asm mov   ax,ds
  asm mov   es,ax
  asm mov   di,si
#endif

  asm cld
  asm mov   bx,0DB24h
  asm mov   dx,Len
  asm shr   dx,1

top:
  asm or    dx,dx
  asm jz    done
  asm lodsw
  asm xor   ax,bx
  asm xor   ah,dl
  asm xor   al,dl
  asm mov   cl,bl
  asm add   cl,dl
  asm rol   ax,cl
  asm stosw
  asm mov   bx,ax
  asm dec   dx
  asm jmp   short top

done:
  asm test  word ptr Len,1
  asm jz    exit
  asm lodsb
  asm rol   al,cl
  asm xor   al,bl
  asm stosb

exit:;
#ifdef LDATA
  asm pop   ds
#endif
#endif  /* ifdef __OS2__ */
}



void LIBENTRY decrypt2(char *Str, int Len) {
#ifdef __OS2__
  unsigned char   RotVal;
  unsigned short  Temp;
  unsigned short  Save;
  unsigned short  Seed;
  unsigned short *p;
  int             X;

  Seed = 0xDB24;
  for (p = (unsigned short *) Str, X = Len >> 1; X > 0; X--, p++) {
    Temp = (unsigned short) ((X & 0xFF) + ((X & 0xFF) << 8));
    RotVal = (char) ((char) Seed + (char) X);
    Save = *p;
    *p = (unsigned short) ((srotr(*p,RotVal) ^ Temp) ^ Seed);
    Seed = Save;
  }

  if (Len & 1) {
    Str = (char *) p;
    *Str = crotr((char) (*Str ^ (unsigned char) Seed),RotVal);
  }

#else  /* ifdef __OS2__ */

#ifdef LDATA
  asm push  ds
  asm lds   si,Str
  asm les   di,Str
#else
  asm mov   si,Str
  asm mov   ax,ds
  asm mov   es,ax
  asm mov   di,si
#endif

  asm cld
  asm mov   bx,0DB24h
  asm mov   dx,Len
  asm shr   dx,1
  asm push  bp

top:
  asm or    dx,dx
  asm jz    done
  asm lodsw
  asm mov   bp,ax
  asm mov   cl,bl
  asm add   cl,dl
  asm ror   ax,cl
  asm xor   al,dl
  asm xor   ah,dl
  asm xor   ax,bx
  asm stosw
  asm mov   bx,bp
  asm dec   dx
  asm jmp   short top

done:
  asm pop   bp
  asm test  word ptr Len,1
  asm jz    exit
  asm lodsb
  asm xor   al,bl
  asm ror   al,cl
  asm stosb

exit:;
#ifdef LDATA
  asm pop   ds
#endif
#endif  /* ifdef __OS2__ */
}


#define SUCKLEN   17
/* static char *Suck = "DECOMPILERS SUCK!";  CRYPTO.EXE */
static char Suck[SUCKLEN] = {0x8C,0x53,0xB8,0xA7,0x9E,0x0F,0x0A,0xCB,0x28,
  0x62,0x2D,0x50,0x7E,0x05,0x3D,0x4E,0x35};

void LIBENTRY encrypt3(char *Str, int Len) {
  int Recycle = SUCKLEN;
  for (char *p = Suck; Len; Str++, Len--) {
    *Str ^= *p + (char) Len;
    if (--Recycle)
      p++;
    else {
      p = Suck;
      Recycle = SUCKLEN;
    }
  }
}


#ifdef TEST
#include <stdio.h>
#include <string.h>

static FILE *f;

void pascal write(char *Str, int Len) {
  fwrite(Str,Len,1,f);
  fwrite("\r\n",2,1,f);
}

void pascal testit2(char *Str, int Len) {
/*encrypt(Str,Len); */
/*encrypt2(Str,Len); */
  encrypt3(Str,Len);
  write(Str,Len);
  decrypt3(Str,Len);
/*decrypt2(Str,Len); */
/*decrypt(Str,Len); */
  write(Str,Len);
  write("--------------------\r\n",22);
}

typedef struct {
  char Before[20];
  char Str[80];
  char After[20];
} memtest;

void pascal testit(char *Str, int Len) {
  memtest Test;
  memtest Save;

  memset(&Test,0,sizeof(Test));
  strcpy(Test.Str,Str);
  memcpy(&Save,&Test,sizeof(Test));

  write(Test.Str,Len);
/*
  encrypt(Test.Str,Len);
  write(Test.Str,Len);
  decrypt(Test.Str,Len);
  write(Test.Str,Len);

  if (memcmp(&Test,&Save,sizeof(Test)) != 0)
    printf("error in compare #1, %s\n",Save.Str);
*/

  testit2(Test.Str,Len);

  if (memcmp(&Test,&Save,sizeof(Test)) != 0)
    printf("error in compare #2, %s\n",Save.Str);
}


static char *Str1 = "THIS IS A TEST";
static char *Str2 = "DAVID TERRY";
static char *Str3 = "MARILYN TERRY";
static char *Str4 = "CASSIE TERRY";
static char *Str5 = "ANGELA TERRY";
static char *Str6 = "BRANDY TERRY";
static char *Str7 = "            ";
static char *Str8 = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static char *Str9 = "Hello PPl version 3.3!";

void main(int argc, char **argv) {
  if ((f = fopen("test","w+b")) == NULL)
    return;

  testit(Str1,strlen(Str1));
  testit(Str2,strlen(Str2));
  testit(Str3,strlen(Str3));
  testit(Str4,strlen(Str4));
  testit(Str5,strlen(Str5));
  testit(Str6,strlen(Str6));
  testit(Str7,strlen(Str7));
  testit(Str8,20);
  testit(Str9,strlen(Str9)+1);

  if (argc > 1) {
    for (int X = 2; X <= argc; X++)
      testit(argv[X-1],strlen(argv[X-1]));
  }

  fclose(f);
}
#endif
