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


#if defined(__OS2__) || defined(__WATCOMC__)
  #define INCL_DOSNLS
  #include <os2.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
    #include <stdlib.h>
  #else
//  #pragma inline
  #endif
  #include "model.h"
#endif

#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "country.h"
#include "tables.h"   /* this header has DEFAULT tables already built so */
                      /* that strupr(), etc, will work WITHOUT having to */
                      /* first call getcountryspecs()                    */

#ifdef DEBUG
#include <memcheck.h>
#endif

/* NOTE:  The call to getcountryspecs() allows you to pass two values.  They */
/* are the country code and code page.  However, be aware that a -1 can be   */
/* passed for Code Page and it MAY NOT MEAN what you expect it to mean.  The */
/* following defines the actions taken:                                      */
/*                                                                           */
/*   GetCountry Values:      0   Get the default country code                */
/*                          >0   Get the specified country code              */
/*                                                                           */
/*   GetCodePage Values:     0   Get the default codepage                    */
/*                          >0   Get the specified codepage                  */
/*                          -1   Skip the COUNTRY CODE specifics for upper & */
/*                               lowercase high ascii tables.                */


countrytype Country;

#if !defined(__OS2__) && !defined(__WATCOMC__)
  typedef struct {
    int  TableSize;       /* how many of the 128 chars are used */
    char Chars[128];      /* characters in conversion table     */
  } chartabletype;

  typedef struct {
    char Code;
    chartabletype far *Ptr;
  } countrytabletype;

  countrytabletype Table;
#endif

unsigned int  CurrentCountry;
unsigned int  CodePage;
/*
unsigned char LowerCase[256];
unsigned char UpperCase[256];
unsigned char Collate[256];
*/


void LIBENTRY getcountryspecs(int GetCountry, int GetCodePage) {
  #if defined(__OS2__) || defined(__WATCOMC__)
    COUNTRYCODE CountryCode;
    COUNTRYINFO CountryBuffer;
    ULONG       Length;
    APIRET      rc;
    char        CopyBuf[256];
  #endif

  int  X;

  for (X = 0; X < 256; X++)
    LowerCase[X] = UpperCase[X] = (char) X;
  for (X = 'A'; X <= 'Z'; X++)
    LowerCase[X] = (char) (X + 32);
  for (X = 'a'; X <= 'z'; X++)
    UpperCase[X] = (char) (X - 32);

  #if defined(__OS2__) || defined(__WATCOMC__)
    CountryCode.country  = GetCountry;
    CountryCode.codepage = GetCodePage;

    rc = DosQueryCtryInfo(sizeof(CountryBuffer),&CountryCode,&CountryBuffer,&Length);
    if (rc != 0)
      goto defaultcountry;

    memcpy(Country.DollarSym,CountryBuffer.szCurrency,5);
    Country.DateFormat     = (int) CountryBuffer.fsDateFmt;
    Country.CurrencyFmt    = CountryBuffer.fsCurrencyFmt;
    Country.SigDigits      = CountryBuffer.cDecimalPlace;
    Country.TimeFmt        = CountryBuffer.fsTimeFmt;
    Country.ThousandSep[0] = CountryBuffer.szThousandsSeparator[0];
    Country.ThousandSep[1] = 0;
    Country.FractionSep[0] = CountryBuffer.szDecimal[0];
    Country.FractionSep[1] = 0;
    Country.DateSep[0]     = CountryBuffer.szDateSeparator[0];
    Country.DateSep[1]     = 0;
    Country.TimeSep[0]     = CountryBuffer.szTimeSeparator[0];
    Country.TimeSep[1]     = 0;
    Country.DataSep[0]     = CountryBuffer.szDataSeparator[0];
    Country.DataSep[1]     = 0;

    CurrentCountry = CountryCode.country;
    CodePage = CountryCode.codepage;

  #else
    if (_osmajor < 3)
      goto defaultcountry;
    if (_osmajor == 3 && _osminor < 30)
      goto defaultcountry;

    /* get the country dependent information */
    if (GetCountry >= 255) {
      asm mov al,0ffh
      asm mov bx,word ptr GetCountry
    } else {
      asm mov al,byte ptr GetCountry
    }

    #ifdef LDATA
    asm push ds
    asm mov  dx, seg Country
    asm mov  ds, dx
    #endif

    asm mov ah,38h
    asm mov dx,offset Country
    asm int 21h

    #ifdef LDATA
    asm pop ds
    #endif

    asm mov X,Bx                    /* playing games with X because MSC */
    CurrentCountry = X;             /* uses a different segment!        */

    asm jnc getcodepage
    asm jmp defaultcountry

    getcodepage:
    asm mov ax,6601h
    asm int 21h
    asm mov X,Bx                    /* playing games with X because MSC */
    CodePage = X;                   /* uses a different segment!        */
  #endif  /* ifdef __OS2__ */

  getupcasetable:
  if (GetCodePage != 0) {
    if (GetCodePage == -1)
      goto defaultconv;

    X = CodePage = GetCodePage;

    #if !defined(__OS2__) && !defined(__WATCOMC__)
      /* does this need to be convered to OS/2 code??? */

      asm mov bx,X                  /* get desired code page info      */
      asm mov ax,6602h
      asm int 21h
    #endif
  }

  #if defined(__OS2__) || defined(__WATCOMC__)
    CountryCode.country  = CurrentCountry;
    CountryCode.codepage = CodePage;

    for (X = 0; X <= 127; X++)
      CopyBuf[X] = (char) (X + 128);

    rc = DosMapCase(128,&CountryCode,CopyBuf);
    if (rc != 0)
      goto defaultconv;

    memcpy(UpperCase+128,CopyBuf,128);

  #else  /* ifdef __OS2__ */
    X = CodePage;
    asm mov bx,X                  /* get desired code page info      */
    X = CurrentCountry;
    asm mov dx,X                  /* get current country information */

    asm mov ax, seg Table
    asm mov es, ax

    asm mov ax, 6502h             /* get uppercase table             */
    asm mov cx, 5                 /* size of Table is 5 bytes        */
    asm mov di, offset Table
    asm int 21h

    asm jnc copycasetable
    asm jmp defaultconv

    copycasetable:
    farmemcpy(&UpperCase[128],Table.Ptr->Chars,Table.Ptr->TableSize);
  #endif  /* ifdef __OS2__ */


  for (X = 128; X < 255; X++) {
    if (UpperCase[X] >= 128 && X != UpperCase[X])
      LowerCase[UpperCase[X]] = (char) X;
  }


  getupcollatetable:
  #if defined(__OS2__) || defined(__WATCOMC__)

    for (X = 0; X <= 255; X++)
      CopyBuf[X] = (char) X;

    rc = DosQueryCollate(256,&CountryCode,CopyBuf,&Length);
    if (rc != 0)
      goto defaultcollate;

    memcpy(Collate,CopyBuf,256);

  #else   /* ifdef __OS2__ */
    X = CodePage;
    asm mov bx,X                  /* get desired code page info      */
    X = CurrentCountry;
    asm mov dx,X                  /* get current country information */

    asm mov ax, seg Table
    asm mov es, ax

    asm mov ax, 6506h               /* get collate table               */
    asm mov dx, -1                  /* get current country information */
    asm mov cx, 5                   /* size of Table is 5 bytes        */
    asm mov dx, offset Table
    asm int 21h

    asm jnc copycollatetable
    asm jmp defaultcollate

    copycollatetable:
    farmemcpy(Collate,Table.Ptr->Chars,Table.Ptr->TableSize);
  #endif   /* ifdef __OS2__ */

  return;  /* all done */


/****  defaults in case the country information is not available ****/

defaultcountry:
  CurrentCountry = 1;
  memset(&Country,0,sizeof(Country));
  Country.DollarSym[0] = '$';
  Country.ThousandSep[0] = ',';
  Country.FractionSep[0] = '.';
  Country.DateSep[0] = '/';
  Country.TimeSep[0] = ':';
  Country.SigDigits = 2;
  Country.DataSep[0] = ',';

defaultconv:
  /* for compatibility with standard C functions, do not set these if the */
  /* country to get is a -1                                               */

  if (GetCodePage != -1) {
    LowerCase['Ä'] = 'á';  UpperCase['á'] = 'Ä';
    LowerCase['ö'] = 'Å';  UpperCase['Å'] = 'ö';
    LowerCase['ê'] = 'Ç';  UpperCase['Ç'] = 'ê';
    LowerCase['é'] = 'Ñ';  UpperCase['Ñ'] = 'é';
    LowerCase['è'] = 'Ü';  UpperCase['Ü'] = 'è';
    LowerCase['ô'] = 'î';  UpperCase['î'] = 'ô';
    LowerCase['•'] = '§';  UpperCase['§'] = '•';

    UpperCase['É'] = 'A';
    UpperCase['Ö'] = 'A';
    UpperCase['†'] = 'A';
    UpperCase['à'] = 'E';
    UpperCase['â'] = 'E';
    UpperCase['ä'] = 'E';
    UpperCase['ã'] = 'I';
    UpperCase['å'] = 'I';
    UpperCase['ç'] = 'I';
    UpperCase['°'] = 'I';
    UpperCase['ì'] = 'O';
    UpperCase['ï'] = 'O';
    UpperCase['¢'] = 'O';
    UpperCase['ñ'] = 'U';
    UpperCase['ó'] = 'U';
    UpperCase['£'] = 'U';
    UpperCase['ò'] = 'Y';
  }

defaultcollate:
  for (X = 0; X < 256; X++)
    Collate[X] = (char) X;        /* default to normal ascii weightings      */
  for (X = 97; X < 123; X++)
    Collate[X] = (char) (X - 32); /* weight lowercase letters = to uppercase */

  /* for compatibility with standard C functions, do not set these if the */
  /* country to get is a -1                                               */

  if (GetCodePage != -1) {
    Collate[225] = 83;
    memcpy(&Collate[128],"CUEAAAACEEEIIIAAEAAOOOUUYOU$$$$$AIOUNN¶ß?©™´¨!\"\"",48);
  }
}
