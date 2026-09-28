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


#ifndef H_COUNTRY
#define H_COUNTRY

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned size_t;
#endif

#pragma pack(1)
typedef struct {
  int  DateFormat;
  char DollarSym[5];
  char ThousandSep[2];
  char FractionSep[2];
  char DateSep[2];
  char TimeSep[2];
  char CurrencyFmt;
  char SigDigits;
  char TimeFmt;
  char (_FAR_ *upchar)(char Al);
  char DataSep[2];
  char Reserved[10];
} countrytype;
#pragma pack()

extern countrytype Country;

extern unsigned int  CurrentCountry;
extern unsigned int  CodePage;
extern unsigned char LowerCase[256];
extern unsigned char UpperCase[256];
extern unsigned char Collate[256];

/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

void   LIBENTRY getcountryspecs(int GetCountry, int GetCodePage);
int    LIBENTRY memfcmp(const void *s1, const void *s2, size_t n);
char * LIBENTRY countrydate(char *DateStr);
char * LIBENTRY countrydate2(char *DateStr);
char * LIBENTRY uncountrydate(char *DateStr);
char * LIBENTRY uncountrydate2(char *DateStr);
char * LIBENTRY dateformat(void);
char * LIBENTRY dateformat2(void);
char * LIBENTRY comma(char *Buffer, long Num);
char * LIBENTRY ucomma(char *Buffer, unsigned long Num);
char * LIBENTRY dcomma(char *Buffer, double Num);

/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/

#endif  /* ifndef H_COUNTRY */
