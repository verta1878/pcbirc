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


#ifndef H_ANSI
#define H_ANSI

#ifndef _FAR_
  #ifdef __OS2__
    #define _FAR_
  #else
    #define _FAR_ far
  #endif
#endif

enum {UPDT_DFLTINTERVAL = 500, UPDT_SLOWINTERVAL = 1000, UPDT_FASTINTERVAL = 100};

/* in OS/2 these functions are in a C source module, but under DOS they are */
/* in assembly, so we can't mangle the names if C++ and DOS is used.        */

#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif

void LIBENTRY bell(void);

/* prototypes from ANSI.ASM for DOS or ANSI.C for OS/2 */
void LIBENTRY toggleoff(char _FAR_ *ScrnBuf);
void LIBENTRY toggleon(char _FAR_ *ScrnBuf);
char LIBENTRY curcolor(void);
char LIBENTRY awherex(void);
char LIBENTRY awherey(void);
void LIBENTRY agotoxy(char X, char Y);
void LIBENTRY asetcolor(char NewColor);
int  LIBENTRY colorstr(char *Str, char NewColor);
void LIBENTRY doesccodes(void);
void LIBENTRY noesccodes(void);
void LIBENTRY ansi(char *Str);
void LIBENTRY setlimits(char NumLines);
void LIBENTRY scrolloff(void);
void LIBENTRY reenablescroll(void);
void LIBENTRY viewbuff(int TopLine);
void LIBENTRY tagem(int X1, int Y1, int X2, int Y2, char Attr);
void LIBENTRY gettagged(char _FAR_ *Str,int X1, int X2, int Y, char Attr);

#ifdef __OS2__
  void LIBENTRY scrollon(char *Buffer, char **Offset, int NumLines);
  void LIBENTRY changeupdateinterval(int Interval);
#else
  void LIBENTRY scrollon(char _FAR_ *Buffer, int _FAR_ *Offset, int NumLines);
  #define changeupdateinterval(i)
#endif

#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif

#endif  /* ifdef H_ANSI */
