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


#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <screen.h>
#include <dosfunc.h>
#include <country.h>
#include <system.h>

#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

clocktype ShowClock;
bool UpdateBox;
bool UpdateKbdStatus;
char HelpName[66];
char ColorCnf[66];
int  HelpFile;
char *HelpVerifyStr;      /* override this in an actual application */
char Colors[NUMCOLORS];
char Color[NUMCOLORS];
char BandW[NUMCOLORS];

/* moved to SYSTEM.LIB */
/* #ifndef __OS2__ */
/* char far *KbdStatus;         /* pointer to Keyboard status byte */ */
/* #endif */

char DefColor1[NUMCOLORS] =
{ 0x01,   /* OUTBOX      */
  0x03,   /* STATUS      */
  0x0C,   /* HEADING     */
  0x04,   /* MENUBOX     */
  0x0E,   /* MENUTITLE   */
  0x0A,   /* MENUSELECT  */
  0x3E,   /* MENUBAR     */
  0x4E,   /* MENUDESC    */
  0x07,   /* MENUUNAVAIL */
  0x30,   /* MENUUNBAR   */
  0x0A,   /* QUESTION    */
  0x03,   /* ANSWER      */
  0x4F,   /* EDIT        */
  0x07,   /* DISPLAY     */
  0x60,   /* DESC        */
  0x20,   /* HELPBOX     */
  0x2F,   /* HELPTITLE   */
  0x2E,   /* HELPSUB     */
  0x20,   /* HELPTEXT    */
  0x4F,   /* HELPDESC    */
  0x0F,   /* WHITE       */
  0x70,   /* SCALECOLOR1 */
  0x0F};  /* SCALECOLOR2 */

char DefColor2[NUMCOLORS] =
{ 0x13,   /* OUTBOX      */
  0x16,   /* STATUS      */
  0x1E,   /* HEADING     */
  0x14,   /* MENUBOX     */
  0x1F,   /* MENUTITLE   */
  0x1A,   /* MENUSELECT  */
  0x3E,   /* MENUBAR     */
  0x4E,   /* MENUDESC    */
  0x17,   /* MENUUNAVAIL */
  0x30,   /* MENUUNBAR   */
  0x1A,   /* QUESTION    */
  0x13,   /* ANSWER      */
  0x4F,   /* EDIT        */
  0x17,   /* DISPLAY     */
  0x30,   /* DESC        */
  0x24,   /* HELPBOX     */
  0x2F,   /* HELPTITLE   */
  0x2E,   /* HELPSUB     */
  0x20,   /* HELPTEXT    */
  0x4E,   /* HELPDESC    */
  0x1F,   /* WHITE       */
  0x71,   /* SCALECOLOR1 */
  0x2B};  /* SCALECOLOR2 */

char DefBandW[NUMCOLORS] =
{ 0x07,   /* OUTBOX      */
  0x07,   /* STATUS      */
  0x0F,   /* HEADING     */
  0x07,   /* MENUBOX     */
  0x0F,   /* MENUTITLE   */
  0x07,   /* MENUSELECT  */
  0x70,   /* MENUBAR     */
  0x70,   /* MENUDESC    */
  0x08,   /* MENUUNAVAIL */
  0x78,   /* MENUUNBAR   */
  0x0F,   /* QUESTION    */
  0x07,   /* ANSWER      */
  0x70,   /* EDIT        */
  0x07,   /* DISPLAY     */
  0x70,   /* DESC        */
  0x70,   /* HELPBOX     */
  0x70,   /* HELPTITLE   */
  0x70,   /* HELPSUB     */
  0x70,   /* HELPTEXT    */
  0x07,   /* HELPDESC    */
  0x0F,   /* WHITE       */
  0x70,   /* SCALECOLOR1 */
  0x7F};  /* SCALECOLOR2 */

/********************************************************************
*
*  Function:  savecolors()
*
*  Save the colors that will be used by the system.
*/

void LIBENTRY savecolors(char C[], char B[]) {
  DOSFILE File;

  if (ColorCnf[0] == 0 || dosfopen(ColorCnf,OPEN_RDWR|OPEN_DENYNONE,&File) == -1)
    return;

  dosfwrite(C,NUMCOLORS,&File);
  dosfwrite(B,NUMCOLORS,&File);
  dosfclose(&File);
}


/********************************************************************
*
*  Function:  readcolors()
*
*  Read the colors that will be used by the system.
*/

void LIBENTRY readcolors(void) {
  int     Len;
  DOSFILE File;

  Len = 0;
  memset(&File,0,sizeof(File));

  if (ColorCnf[0] != 0 && dosfopen(ColorCnf,OPEN_READ|OPEN_DENYNONE,&File) != -1) {
    Len = (int) dosfseek(&File,0,SEEK_END);
    dosfseek(&File,0,SEEK_SET);
  }

  if (Len != NUMCOLORS*2) {
    dosfclose(&File);
    savecolors(DefColor1,DefBandW);
    memcpy(Color,DefColor1,NUMCOLORS);
    memcpy(BandW,DefBandW ,NUMCOLORS);
  } else {
    dosfread(Color,NUMCOLORS,&File);
    dosfread(BandW,NUMCOLORS,&File);
    dosfclose(&File);
  }
}


void closehelpfile(void) {
  if (HelpFile > 0)
    dosclose(HelpFile);
}


/********************************************************************
*
*  Function:  initscrnio()
*
*  This function MUST be called before any other scrnio function may be used
*/

void LIBENTRY initscrnio(void) {
  char *p;
  char teststr[10];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  /* default to color before the array gets loaded in case the file open */
  /* gets an error and the error message needs to be displayed */
  memcpy(Colors,DefColor1,NUMCOLORS);

  getmode();

  getcountryspecs(0,0);

/*  _fmode = O_BINARY; */

  Novell = FALSE;
  DisableGiveup = FALSE;

  p = getenv("PCB");
  if (p != NULL) {
    strupr(p);
    if (strstr(p,"NMT") != NULL)
      Novell = TRUE;
    if (strstr(p,"/NOGIVEUP") != NULL)
      DisableGiveup = TRUE;
    if (strstr(p,"/COLOR") != NULL)
      Scrn_Mode = VID_COLOR;
    if (strstr(p,"/MONO") != NULL)
      Scrn_Mode = VID_MONO;
  }

  checkmultitaskers();

  readcolors();
  if (Scrn_Mode == VID_COLOR)
    p = Color;
  else
    p = BandW;
  memcpy(Colors,p,NUMCOLORS);

  Scrn_24Hour = TRUE;

  ShowClock = CLOCK;
  if ((p = getenv("CLOCK")) != NULL)
    if (*p == 'N' || *p == 'n')
      ShowClock = NOCLOCK;

  UpdateBox       = TRUE;
  UpdateKbdStatus = TRUE;

  if ((p = getenv("BOX")) != NULL)
    if (*p == 'N' || *p == 'n') {
      UpdateBox       = FALSE;
      UpdateKbdStatus = FALSE;
      Scrn_Box        = FALSE;
      ShowClock       = NOCLOCK;
    }

  /* turn off the INSERT key */
  setkbdstatus((uint) (getkbdstatus() & ~INSERT));

  fastprint(0,0," ",0x7F);
  clscolor(0x07);

  if (HelpName[0] != 0) {
    if ((HelpFile = dosopen(HelpName,OPEN_READ|OPEN_DENYNONE POS2ERROR)) != -1) {
      dosread(HelpFile,teststr,sizeof(teststr) POS2ERROR);
      if (memcmp(teststr,HelpVerifyStr,sizeof(teststr)) != 0) {
        dosclose(HelpFile);
        HelpFile = -1;        /* use "-1" to indicate a help file mismatch */
      } atexit(closehelpfile);
    } else HelpFile = 0;
  } else
    HelpFile = 0;


  #ifdef __OS2__
    initshowstatus();
  #endif
}

