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
  #define INCL_VIO
  #define INCL_DOSPROCESS
  #include <os2.h>

  #include <threads.h>
  #include <process.h>
  #include <string.h>
  #ifdef __BORLANDC__
    #include <inline.h>
  #endif
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    #include <malloc.h>
  #else
    #include <alloc.h>
  #endif
  #include <semafore.hpp>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif

#include "screen.h"

#ifdef TEST
#include <stdio.h>
#endif

#define FALSE 0
#define TRUE  1

#ifdef __OS2__
static bool HideScreen;
static unsigned long __far16 *Scrn_Addr;
void  *Scrn_Buf;
#else
long  Scrn_Addr;                   /* Address of Screen Memory (seg:ofs)   */
#endif

char  Scrn_Rtrc;                   /* True: Perform Screen Retrace         */
char  Scrn_Mode;                   /* True: Use Color, False: B&W          */
char  Scrn_ColorCard;              /* True: Color Card installed           */
char  Scrn_EGA;                    /* True: EGA Card installed             */
char  Scrn_Box;                    /* True: if allowed to draw boxes       */
char  Scrn_24Hour = 0;             /* True: 24 hour clock - can be changed */
char  Scrn_DateSeparator = '-';    /* i.e. 12-12-88  - can be changed      */
char  Scrn_BottomRow;              /* last row on screen (i.e. 24)         */
int   Scrn_SizeBytes;              /* number of bytes in screen buffer     */
short Scrn_Size16;                 /* number of 16-bit words in buffer     */
int   Scrn_Size32;                 /* number of 32-bit words in buffer     */
vidcardtype Scrn_Adapter;          /* type of video card installed         */

#ifdef __OS2__
char Scrn_X;
char Scrn_Y;
#endif


/********************************************************************
*
*  Function:  getvideotype()
*
*  Desc    :  performs a complete analysis of the type of video adapter that
*             is installed checking for MDA, CGA, EGA, VGA and MCGA.
*/

#ifndef __OS2__
void near pascal getvideotype(void) {
  char ALreg;
  char BLreg;
  char BHreg;
  int  BXreg;

  asm mov ax,1A00h      /* Attempt to call VGA Identify Adapter Function */
  asm int 10h
  asm mov ALreg,Al
  asm mov BLreg,Bl

  #ifdef TEST
    printf("Ax=1A00h, int 10h - results: AL=%02X, BL=%02X\n",ALreg,BLreg);
  #endif

  Scrn_Adapter = VID_NONE;
  Scrn_Mode    = VID_COLOR;

  if (ALreg == 0x1A) {    /* if AL changed from 0 to 0x1A - must be PS/2 bios */
    switch (BLreg) {
      case 0x00 : Scrn_Adapter = VID_NONE;  Scrn_Mode = VID_COLOR; break;
      case 0x01 : Scrn_Adapter = VID_MDA;   Scrn_Mode = VID_MONO;  break;
      case 0x02 : Scrn_Adapter = VID_CGA;   Scrn_Mode = VID_COLOR; break;
      case 0x04 : Scrn_Adapter = VID_EGA;   Scrn_Mode = VID_COLOR; break;
      case 0x05 : Scrn_Adapter = VID_EGA;   Scrn_Mode = VID_MONO;  break;
      case 0x07 : Scrn_Adapter = VID_VGA;   Scrn_Mode = VID_MONO;  break;
      case 0x08 : Scrn_Adapter = VID_VGA;   Scrn_Mode = VID_COLOR; break;
      case 0x0A :
      case 0x0C : Scrn_Adapter = VID_MCGA;  Scrn_Mode = VID_COLOR; break;
      case 0x0B : Scrn_Adapter = VID_MCGA;  Scrn_Mode = VID_MONO;  break;
      default   : Scrn_Adapter = VID_CGA;   Scrn_Mode = VID_COLOR; break;
    }
  } else {
    asm mov ah,12h      /* select Alternate Function service */
    asm mov bx,10h      /* BL=0x10 means return EGA information */
    asm int 10h
    asm mov BXreg,Bx

    #ifdef TEST
      printf("Ax=1200h, BX=0010h, int 10h - results: BX=%04X\n",BXreg);
    #endif

    if (BXreg != 0x10) {  /* BX changed means there's an EGA there */
      Scrn_Adapter = VID_EGA;
      asm mov ah,12h
      asm mov bl,10h
      asm int 10h
      asm mov BHreg,Bh
      if (BHreg == 0)
        Scrn_Mode = VID_COLOR;
      else
        Scrn_Mode = VID_MONO;
    } else {
      asm int 11h       /* we got this far, must be CGA or MDA */
      asm and al,30h
      asm shr al,1
      asm shr al,1
      asm shr al,1
      asm shr al,1
      asm mov ALreg,Al
      switch(ALreg) {
        case 1 :
        case 2 : Scrn_Adapter = VID_CGA;  Scrn_Mode = VID_COLOR; break;
        case 3 : Scrn_Adapter = VID_MDA;  Scrn_Mode = VID_MONO;  break;
        default: Scrn_Adapter = VID_NONE; Scrn_Mode = VID_COLOR; break;
      }
    }
  }

/*if (Scrn_Mode == VID_COLOR) { */
    switch (Scrn_Adapter) {
      case VID_CGA :
      case VID_EGA :
      case VID_VGA :
      case VID_MCGA: asm mov ah,0Fh
                     asm int 10h
                     asm mov ALreg,Al

                     #ifdef TEST
                       printf("Ax=0F00h, int 10h - results: AL=%02X\n",ALreg);
                     #endif

                     if (ALreg == 0 || ALreg == 2 || ALreg == 7) {
                       Scrn_Mode = VID_MONO;
                       if (ALreg == 7)
                         Scrn_Adapter = VID_MDA;
                     }
                     break;
    }
/*}*/
}
#endif  /* ifndef __OS2__ */


#ifdef __OS2__
static CMutexSemaphore MutexSemaphore;
static CUpdateSemaphore UpdateSemaphore;
static scrnupdttype volatile NeedToUpdateScrn;
static bool volatile ScreenLines[50];

#pragma argsused
#ifdef __WATCOMC__
static void THREADFUNC screenupdatethread(void *Ignore) {
#else
static void THREADFUNC screenupdatethread(void *Ignore) {
#endif
  bool  UpdateLines[50];
  char  CurX;
  char  CurY;
  int   UpdateScrn;
  int   Begin;
  int   End;
  int   X;
  int   Offset;
  int   Len;

  DosSetPriority(PRTYS_THREAD,PRTYC_REGULAR,0,0);  //lint !e534

  CurX = wherex();
  CurY = wherey();

  while (1) {
    // Wait until an event is posted signalling that we need to update the
    // screen, or wait for 1 second, whichever comes first.  The event is used
    // to signal "update in a hurry", whereas if an update is casual it will
    // be done when the interval expires.
    UpdateSemaphore.waitforevent();

    // NeedToUpdateScrn is used to determine if an update is even necessary
    // as well as a quick indicator of full screen update versus mixed line
    // update values.
    if ((! HideScreen) && NeedToUpdateScrn != UPDATE_NONE) {

      // To avoid stomping on another process that is trying to update the
      // ScreenLines variable, we wait for a mutex semaphore here to grab a
      // copy of ScreenLines, then we reset the value to 0 so that we avoid
      // refreshing the same lines again unless they happen to change again.
      MutexSemaphore.acquire();
        UpdateScrn       = NeedToUpdateScrn;
        NeedToUpdateScrn = UPDATE_NONE;
        memcpy(UpdateLines,(char *) ScreenLines,50);
        memset((char *) ScreenLines,0,50);
      MutexSemaphore.release();


      switch (UpdateScrn) {
        case SCRN_25LINES: memcpy((void __far16 *) Scrn_Addr,Scrn_Buf,4000);
                           VioShowBuf(0,(short) 4000,0);
                           break;
        case SCRN_50LINES: memcpy((void __far16 *) Scrn_Addr,Scrn_Buf,8000);
                           VioShowBuf(0,(short) 8000,0);
                           break;
        default:
        // Begin and End are the lines that need to be drawn.  Initially they
        // are set out of range to not display anything, then Begin and End
        // are changed to indicate which lines should be redrawn.
        Begin = End = -1;

        // scan all lines on the screen
        for (X = 0; X <= Scrn_BottomRow; X++) {
          if (UpdateLines[X]) {
            // Line X needs to be updated, update Begin only if we don't yet
            // know what the first line is, but extend the End line since we
            // know we're scanning forward
            if (Begin == -1)
              Begin = X;
            End = X;
          } else {
            // We know that Line X doesn't need to be updated, so we're either
            // done, or else we're skipping over some lines.  If Begin == -1
            // then we haven't found any lines to update yet.  If Begin != -1
            // then we may as well update the lines we've found so far and then
            // skip over those that don't need to be updated.
            if (Begin != -1) {
              Offset = Begin * 160;
              Len    = (End - Begin + 1) * 160;
              memcpy((char __far16 *) Scrn_Addr + Offset,(char *) Scrn_Buf + Offset,Len);
              VioShowBuf((short)Offset,(short)Len,0);
              Begin = End = -1;
            }
          }
        }

        // We're done scanning the lines, if there is anything left to update
        // then update it now.
        if (Begin != -1) {
          Offset = Begin * 160;
          Len    = (End - Begin + 1) * 160;
          memcpy((char __far16 *) Scrn_Addr + Offset,(char *) Scrn_Buf + Offset,Len);
          VioShowBuf((short)Offset,(short)Len,0);
          Begin = End = -1;
        }
      }
    }

    if (CurX != Scrn_X || CurY != Scrn_Y) {
      CurX = Scrn_X;
      CurY = Scrn_Y;
      VioSetCurPos(CurY,CurX,0);
    }

    UpdateSemaphore.reset();
  }
}


void LIBENTRY setscreenupdateinterval(int Interval) {
  UpdateSemaphore.enabletimer(Interval);
}


void LIBENTRY updatelines(scrnupdttype ScrnUpdate, int StartLine, int EndLine) {
  if ((! HideScreen) && ScrnUpdate != UPDATE_NONE) {
    // To avoid stomping on another process that is trying to update the
    // ScreenLines variable, we go into a critical section here to update the
    // ScreenLines variable.
    MutexSemaphore.acquire();
      // if EndLine < NeedToUpdateScrn, then we've already covered the area of
      // the screen that will need to be updated, so nothing needs to be done.
      // otherwise, we need to mark the lines that are to be updated.
      if (EndLine >= NeedToUpdateScrn) {
        if (ScrnUpdate == UPDATE_MIXED) {
          // for mixed lines, we have to manually set each line
          NeedToUpdateScrn = UPDATE_MIXED;
          for (; StartLine <= EndLine; StartLine++)
            ScreenLines[StartLine] = TRUE;
        } else {
          // for full screen updates, just indicate the size of the screen
          // and set each line to true
          NeedToUpdateScrn = ScrnUpdate;    // will indicate 25 or 50 lines
          memset((char *)  ScreenLines,TRUE,EndLine); // will be 25 or 50
        }
      }
    MutexSemaphore.release();
  }
}


void LIBENTRY updatelinesnow(void) {
  if (! HideScreen)
    UpdateSemaphore.postevent();
}


void LIBENTRY hidescreen(void) {
  int ScrnNum;

  // we save the current contents of the screen
  // then clear the physical screen
  // set a switch that prevents the thread from updating the physical screen
  // then restore the buffer so that the buffer contents remain in synch

  ScrnNum = memsavescreen();
  cls();
  updatelinesnow();
  mydelay(25);
  HideScreen = TRUE;
  setscreenupdateinterval(SEM_WAIT_NOTIMEOUT);
  memrestorescreen(ScrnNum,FREESCREEN);
}


void LIBENTRY unhidescreen(void) {
  HideScreen = FALSE;
  updatelines(UPDATE_25LINES,0,0);
  VioSetCurPos(Scrn_Y,Scrn_X,0);
  updatelinesnow();
}
#endif

/********************************************************************
 *
 *  Function:  getmode()
 *
 *  Parameter: Mode:  SCRN_DIRECT = write directly to the screen
 *
 *
 *  Determine: if a Color Screen is present,
 *             if it's an EGA,
 *             if Retrace Checking is necessary,
 *             if current mode is B/W or Color
 *             address of memory segment
 */

#ifdef __OS2__
int LIBENTRY getviolines(void) {
  VIOMODEINFO   VioModeInfo;

  VioModeInfo.cb = sizeof(VioModeInfo);
  VioGetMode((PVIOMODEINFO) &VioModeInfo,0);
  return(VioModeInfo.row);
}

void LIBENTRY setviolines(int NumLines) {
  VIOMODEINFO   VioModeInfo;

  VioModeInfo.cb = sizeof(VioModeInfo);
  VioGetMode((PVIOMODEINFO) &VioModeInfo,0);
  VioModeInfo.row = (short) NumLines;
  VioSetMode((PVIOMODEINFO) &VioModeInfo,0);
}
#endif

void LIBENTRY getmode(void) {
#ifdef __OS2__
//USHORT        BufSize;
  ULONG         BufSize;
  VIOMODEINFO   VioModeInfo;
  VIOCONFIGINFO VioConfigInfo;

  VioModeInfo.cb = sizeof(VioModeInfo);
  VioGetMode((PVIOMODEINFO) &VioModeInfo,0);

  VioConfigInfo.cb = sizeof(VioConfigInfo);
  VioGetConfig(VIO_CONFIG_CURRENT,&VioConfigInfo,0);

  #ifdef __BORLANDC__
    VioGetBuf((unsigned long __far16 *) &Scrn_Addr,(PUSHORT16) &BufSize,0);
  #elif defined(__WATCOMC__)
    VioGetBuf((unsigned long *) &Scrn_Addr,(unsigned short *) &BufSize,0);
  #endif

  Scrn_Buf       = malloc(8000);
  Scrn_Mode      = (char) (VioConfigInfo.display == MONITOR_MONOCHROME ? VID_MONO : VID_COLOR);
  Scrn_Rtrc      = FALSE;
  Scrn_BottomRow = (char) (VioModeInfo.row-1);
  Scrn_ColorCard = (char) (VioConfigInfo.adapter != DISPLAY_MONOCHROME);
  Scrn_EGA       = (char) (VioConfigInfo.adapter >= DISPLAY_EGA);

  switch(VioConfigInfo.adapter) {
    case DISPLAY_MONOCHROME: Scrn_Adapter = VID_MDA; break;
    case DISPLAY_CGA       : Scrn_Adapter = VID_CGA; break;
    case DISPLAY_EGA       : Scrn_Adapter = VID_EGA; break;
    case DISPLAY_VGA       : /* fall thru */
    default                : Scrn_Adapter = VID_VGA; break;
  }

#else /* ifdef __OS2__ */
  char ALreg;

  #ifdef TEST
    char *VidList[VID_MCGA+1] = {"NONE","MDA","CGA","EGA","VGA","MCGA"};
    char *ModeList[VID_COLOR+1] = {"MONO","COLOR"};
  #endif

  getvideotype();

  if (Scrn_Adapter == VID_MDA) {
    Scrn_Addr = 0xB0000000L;
    Scrn_ColorCard = FALSE;
    Scrn_EGA = FALSE;
  } else {
    Scrn_Addr = 0xB8000000L;
    Scrn_ColorCard = TRUE;
    Scrn_EGA = (Scrn_Adapter == VID_EGA || Scrn_Adapter == VID_VGA);
  }

  Scrn_Rtrc = (Scrn_Adapter == VID_CGA);

  asm xor ax,ax
  asm mov es,ax
  asm mov al,es:[0484h]
  asm mov ALreg,al

  Scrn_BottomRow = ALreg;
  if (Scrn_BottomRow < 24)
    Scrn_BottomRow = 24;

  #ifdef TEST
    printf("\n"
           "Summary: Adapter=%s\n"
           "         Mode=%s\n"
           "         Address=%08lX\n"
           "         Color=%s\n"
           "         Retrace=%s\n"
           "         Lines=%d\n",
           VidList[Scrn_Adapter],
           ModeList[Scrn_Mode],
           Scrn_Addr,
           (Scrn_ColorCard ? "Yes" : "No"),
           (Scrn_Rtrc ? "Yes" : "No"),
           Scrn_BottomRow+1);
  #endif

/***************************************************** no longer used ! *****
  Scrn_Addr = 0xB0000000;
  Scrn_Mode = VID_MONO;

  asm  Mov  Ah,0Fh
  asm  Int  10h
  asm  mov  ALreg,Al

  if (ALreg != 7) {
    Scrn_Addr = 0xB8000000;
    if (ALreg != 0 && ALreg != 2)
      Scrn_Mode = VID_COLOR;
  }
******************************************************************************/

  #ifndef TEST
    setdelay();                   /* set up the delay in case we need it */
  #endif
#endif /* ifdef __OS2__ */

  Scrn_Box       = TRUE;
  Scrn_SizeBytes = (Scrn_BottomRow+1) * 160;
  Scrn_Size16    = (short) (Scrn_SizeBytes >> 1);
  Scrn_Size32    = Scrn_Size16 >> 1;

  #ifdef __OS2__
    /* default to 1 second intervals for updating the screen */
    UpdateSemaphore.create(NULL);
    UpdateSemaphore.enabletimer(1000);
    MutexSemaphore.create(NULL);
    HideScreen = FALSE;
    startthread(screenupdatethread,8*1024,NULL);
  #endif
}


#ifdef TEST
void main(void) {
  getmode();
}
#endif
