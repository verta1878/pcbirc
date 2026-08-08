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


#include "pcbtools.h"

#pragma warn -rvl
#pragma warn -par
int LIBENTRY wordbackward(char *Str, int Pos) { return 0;
}
int LIBENTRY wordforward(char *Str, int Pos, int FieldLen) { return 0;
}
void LIBENTRY inputfield(char *Buffer, int PcbTextNum, int MaxLen, displaytype DisplayCtrl, int HelpNum, char *Mask) {
}
void LIBENTRY inputfieldstr(char *Buffer, char *Prompt, int Color, int MaxLen, displaytype DisplayCtrl, int HelpNum, char *Mask) {
}
long LIBENTRY inputfieldlong(long Default, char *Prompt, int Color, int MaxLen, displaytype DisplayCtrl, int HelpNum) { return 0;
}
int LIBENTRY inputfieldint(int Default, char *Prompt, int Color, int MaxLen, displaytype DisplayCtrl, int HelpNum) { return 0;
}
#pragma warn +rvl
#pragma warn +par
