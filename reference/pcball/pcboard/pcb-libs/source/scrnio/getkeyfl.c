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


#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>

void LIBENTRY getkeyflags(int HelpNum) {
  char C;
  char Ext;
  bool More;
  int  X;

  KeyFlags = 0;
  while (KeyFlags == 0) {
    C = inkey(&Ext,ShowClock);

    if (Ext) {
      More = TRUE;
      for (X = 0; X < NumExitKeys && More; X++) {
        if (C == ExitKeyNum[X]) {
          More = FALSE;
          KeyFlags = ExitKeyFlag[X];
        }
      }
      if (More) {
        switch (C) {
          case /* F1     */  59: if (HelpFile != 0) showhelp(HelpNum);
                                 break;
          case /* up     */  72: KeyFlags = UP; break;
          case /* pgup   */  73: KeyFlags = PGUP; break;
          case /* down   */  80: KeyFlags = DN; break;
          case /* pgdn   */  81: KeyFlags = PGDN; break;
          case /* c-pgdn */ 118: KeyFlags = CTRLPGDN; break;
          case /* c-pgup */ 132: KeyFlags = CTRLPGUP; break;
        }
      }
    } else {
      switch (C) {
        case  9: KeyFlags = TAB; break;
        case 13: KeyFlags = RET; break;
        case 27: KeyFlags = ESC; break;
      }
    }
  }
}
