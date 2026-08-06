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


#include <types.hpp>
#include "pcbmacro.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

typedef struct {
  char  Len;
  char *Macro;
} tabletype;

static tabletype Table[] = {
  { 0,""},
  { 4,"ALIAS"},
  { 7,"AUTOMORE"},
  { 3,"BEEP"},
  { 4,"BICPS"},
  { 8,"BOARDNAME"},
  { 2,"BPS"},
  { 9,"BYTECREDIT"},
  { 8,"BYTELIMIT"},
  { 8,"BYTERATIO"},
  { 8,"BYTESLEFT"},
  { 6,"CARRIER"},
  { 3,"CITY"},
  { 5,"CLREOL"},
  { 2,"CLS"},
  { 7,"CONFNAME"},
  { 6,"CONFNUM"},
  { 7,"CREDLEFT"},
  { 6,"CREDNOW"},
  { 8,"CREDSTART"},
  { 7,"CREDUSED"},
  { 8,"CURMSGNUM"},
  { 8,"DATAPHONE"},
  { 7,"DAYBYTES"},
  { 4,"DELAY"},
  { 6,"DIRNAME"},
  { 5,"DIRNUM"},
  { 6,"DLBYTES"},
  { 6,"DLFILES"},
  { 3,"ENV="},
  { 4,"EVENT"},
  { 6,"EXPDATE"},
  { 6,"EXPDAYS"},
  { 5,"FBYTES"},
  { 5,"FFILES"},
  { 9,"FILECREDIT"},
  { 8,"FILERATIO"},
  { 5,"FIRSTU"},
  { 4,"FIRST"},
  { 3,"FNUM"},
  { 8,"FREESPACE"},
  { 8,"HOMEPHONE"},
  { 9,"HIGHMSGNUM"},
  { 4,"INAME"},
  { 5,"INCONF"},
  { 5,"KBLEFT"},
  { 6,"KBLIMIT"},
  {13,"LASTCALLERNODE"},
  {15,"LASTCALLERSYSTEM"},
  { 9,"LASTDATEON"},
  { 9,"LASTTIMEON"},
  { 2,"LMR"},
  { 6,"LOGDATE"},
  { 6,"LOGTIME"},
  { 8,"LOWMSGNUM"},
  { 7,"MAXBYTES"},
  { 7,"MAXFILES"},
  { 6,"MINLEFT"},
  { 3,"MORE"},
  { 6,"MSGLEFT"},
  { 6,"MSGREAD"},
  { 5,"NOCHAR"},
  { 3,"NODE"},
  { 5,"NUMBLT"},
  { 7,"NUMCALLS"},
  { 6,"NUMCONF"},
  { 5,"NUMDIR"},
  { 9,"NUMTIMESON"},
  { 7,"OFFHOURS"},
  { 5,"OPTEXT"},
  { 4,"PAUSE"},
  { 3,"POFF"},
  { 2,"PON"},
  { 2,"POS"},
  { 5,"PROLTR"},
  { 6,"PRODESC"},
  { 6,"PWXDATE"},
  { 6,"PWXDAYS"},
  { 3,"QOFF"},
  { 2,"QON"},
  { 9,"RATIOBYTES"},
  { 9,"RATIOFILES"},
  { 5,"RBYTES"},
  { 3,"RCPS"},
  { 3,"REAL"},
  { 5,"RFILES"},
  { 5,"SBYTES"},
  { 3,"SCPS"},
  { 7,"SECURITY"},
  { 5,"SFILES"},
  { 6,"SYSDATE"},
  { 6,"SYSOPIN"},
  { 7,"SYSOPOUT"},
  { 6,"SYSTIME"},
  { 8,"TIMELIMIT"},
  { 7,"TIMELEFT"},
  { 7,"TIMEUSED"},
  { 8,"TOTALTIME"},
  { 6,"UPBYTES"},
  { 6,"UPFILES"},
  { 3,"USER"},
  { 3,"WAIT"},
  { 2,"WHO"},
  { 3,"XOFF"},
  { 2,"XON"},
  { 6,"YESCHAR"},
  { 9,"LASTENTRY"}  // this has to be the last line in the table
};

static char ValidHexChars[] = {
/*   0 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  10 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  20 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  30 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  40 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01,
/*  50 */  0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF,
/*  60 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
/*  70 */  0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  80 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/*  90 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 100 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 110 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 120 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 130 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 140 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 150 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 160 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 170 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 180 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 190 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 200 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 210 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 220 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 230 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 240 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
/* 250 */  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


int           FindTokenStart;
int           FindTokenEnd;
unsigned char FindTokenColor;
unsigned char FindTokenAttr;

#if defined(__OS2__) && defined(__BORLANDC__)
  // there is a bug BC-OS2 v2.0 that affects the following two lines of the
  // findtoken() function:
  //   FindTokenColor *= (char) 10;
  //   FindTokenColor += (char) (*p - '0');
  // to work around the bug, optimizations are temporarily disabled
  #pragma option -O-c
#endif
int LIBENTRY findtoken(char *Str) {
  char  C;
  char  Color;
  int   Offset;
  int   Len;
  char *p;
  char *q;
  char *s;

  s = Str;

  while (1) {
    C = *s++;
    if (C == 0)
      return(0);
    if (C != '@')
      continue;

    switch (*s) {
      case 'A': Offset = ALIAS;          break;
      case 'B': Offset = BEEP;           break;
      case 'C': Offset = CARRIER;        break;
      case 'D': Offset = DATAPHONE;      break;
      case 'E': Offset = ENV;            break;
      case 'F': Offset = FBYTES;         break;
      case 'G': continue;
      case 'H': Offset = HOMEPHONE;      break;
      case 'I': Offset = INAME;          break;
      case 'J': continue;
      case 'K': Offset = KBLEFT;         break;
      case 'L': Offset = LASTCALLERNODE; break;
      case 'M': Offset = MAXBYTES;       break;
      case 'N': Offset = NOCHAR;         break;
      case 'O': Offset = OFFHOURS;       break;
      case 'P': Offset = PAUSE;          break;
      case 'Q': Offset = QOFF;           break;
      case 'R': Offset = RATIOBYTES;     break;
      case 'S': Offset = SBYTES;         break;
      case 'T': Offset = TIMELIMIT;      break;
      case 'U': Offset = UPBYTES;        break;
      case 'V': continue;
      case 'W': Offset = WAIT;           break;
      case 'X': if ((Color = ValidHexChars[*(s+1)]) != (char) 0xFF) {
                  if ((C = ValidHexChars[*(s+2)]) != (char) 0xFF) {
                    FindTokenColor = (char) ((Color << 4) + C);
                    // Str[FindTokenEnd] will point to the next character after the last @
                    FindTokenEnd   = (int) (s - Str) + 3;
                    // Str[FindTokenStart] will point to the first @ character
                    FindTokenStart = FindTokenEnd - 4;
                    // NULL-terminate the string where the first @ was found
                    Str[FindTokenStart] = 0;
                    return(XCOLORS);
                  }
                }
                Offset = XOFF;           break;
      case 'Y': Offset = YESCHAR;        break;
      default : continue;
    }

    // Str[FindTokenStart] will point to the first @ character
    FindTokenStart = (int) (s - Str) - 1;

    loop:
    p = s;
    q = Table[Offset].Macro;
    if (*p != *q) {
      s++;
      continue;
    }

    for (p++, q++, Len = Table[Offset].Len; Len; p++, q++, Len--) {
      if (*p != *q) {
        Offset++;
        goto loop;
      }
    }

    endpoint:
    switch (*p) {
      default : if (Offset != ENV) {
                  s++;
                  continue;
                }
                // processing for @ENV= macro
                for (p++; 1; p++) {              //lint !e506
                  if (*p == '@' || *p == ':')
                    goto endpoint;
                  if (*p == 0) {
                    s++;
                    continue;
                  }
                }
                // it'll never get this far

      case '@': FindTokenAttr  = ' ';
                FindTokenColor = 0;
                goto done;
      case ':': FindTokenAttr  = ' ';
                FindTokenColor = 0;
                endloop:
                switch (*(++p)) {
                  case '@': goto done;
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9': FindTokenColor *= (char) 10;
                            FindTokenColor += (char) (*p - '0');
                            goto endloop;
                  case 'C':
                  case 'R':
                  case 'T': FindTokenAttr = *p;
                            goto endloop;
                  default : s++;
                            continue;  // go back to the top
                }
    }

    done:
    // Str[FindTokenEnd] will point to the next character after the last @
    FindTokenEnd = (int) (p - Str) + 1;
    // NULL-terminate the string where the first @ was found
    Str[FindTokenStart] = 0;
    return(Offset);
  }
}

#if defined(__OS2__) && defined(__BORLANDC__)
  // see above for comment on this option
  #pragma option -Oc
#endif
#ifdef TEST
#include <stdio.h>

void test(char *Str) {
  int  X;

  while ((X = findtoken(Str)) != 0) {
    printf("%s\n",Str);
    if (X == XCOLORS)
      printf("Color = @X%02X\n",FindTokenColor);
    else if (X == ENV)
      printf("Env = %-*.*s, FieldLen=%d, Attr=%c\n",FindTokenEnd-FindTokenStart,FindTokenEnd-FindTokenStart,Str+FindTokenStart+1, FindTokenColor, FindTokenAttr);
    else
      printf("Found='%s', FieldLen=%d, Attr=%c\n",Table[X].Macro, FindTokenColor, FindTokenAttr);
    Str[FindTokenStart] = '@';
    Str += FindTokenEnd;
  }
  printf("%s\n",Str);
}


void main(void) {
  test("this is a test");
  test("xxx @ALIAS@ yyy @FIRST:10@ zzz");
  test("xxx @USER:20C@ yyyy");
  test("xxx @HOMEPHONE:20R@ yyyy @DATAPHONE:15T@ zzz");
  test("xxx @X07 yyy");
  test("xxx @XF0 yyy");
  test("xxx @X78 yyy");
  test("xxx @Xa8 yyy");
  test("xxx @X8a yyy");
  test("xxx @ENV=PHONE:10@ yyy");
  test("[@POS:15@]");
  test("[              ]");
}
#endif
