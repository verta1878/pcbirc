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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <misc.h>
#include <mem.h>
#include "pcbtext.h"

#define CURRENTVERSION 0
#define NEWVERSION     1

#define NumEKeys 3
#define EDIT 0
#define SEARCH 1
#define FILENAMELEN 45

int NumExitKeys = NumEKeys;
int ExitKeyFlag[NumEKeys];
char ExitKeyNum[NumEKeys];

int CurrentVersion;

enum { F2=FLAG1, F3, F4 };

static int     EditFlag;
static DOSFILE fptOut;


/*==========================================================
*  FUNCTION: int fnFilesize( char *sptName )
*
*       This function determines the size of the pcbtext file
*       named by user.
*
*  INPUT PARAMETERS: ptr to name of the file to examine.
*
*       RETURNS: number of strings contained in the pcbtext file.
*---------------------------------------------------------*/
int fnFileSize(unsigned *iptFlSz) {
  int iFileFlag;
  char sBuff[81];

  dosfseek(&fptOut, 0L, SEEK_SET);
  dosfread( sBuff, 80, &fptOut);
  if (strstr(sBuff,"14.5") != NULL)
    CurrentVersion = 145;
  if (strstr(sBuff,"15.0") != NULL)
    CurrentVersion = 150;
  if (strstr(sBuff,"15.2") != NULL)
    CurrentVersion = 152;
  if (strstr(sBuff,"15.3") != NULL)
    CurrentVersion = 153;
  if (strstr(sBuff,"15.4") != NULL)
    CurrentVersion = 154;

  if (CurrentVersion < 152)
    iFileFlag = NEWVERSION;
  else
    iFileFlag = CURRENTVERSION;

  *iptFlSz = (unsigned) (dosfseek(&fptOut,0,SEEK_END) / 80); /* # of records */
  return( iFileFlag );
}


/*==========================================================
*  FUNCTION: void fnWriteChanges(  )
*
*       This function writes strings that have been changed to
*       an existing     pcbtext file.
*
*  INPUT PARAMETERS: none.
*
*       RETURNS: none.
*---------------------------------------------------------*/
void fnWriteChanges(unsigned iFlSize) {
  unsigned iNdx1;
  char     Buf[100];

  dosfseek(&fptOut, 0L , SEEK_SET);

  for ( iNdx1 = 0; iNdx1 < iFlSize; iNdx1++ ) {
    if ( Array[iNdx1].cNewflag > CurrentVersion ) {
      dosfseek(&fptOut, (iNdx1*80L), SEEK_SET);
      sprintf(Buf,"%d%-79s",Array[iNdx1].cColor, Array[iNdx1].sptStr);
      dosfwrite(Buf,80,&fptOut);
    }
  }
  dosfseek(&fptOut, 0L , SEEK_SET);
}


void fnWriteHeader(void) {
  char Buf[100];

  dosrewind(&fptOut);
  sprintf(Buf,"%d%-79s",Array[0].cColor,Array[0].sptStr);
  dosfwrite(Buf,80,&fptOut);
  dosrewind(&fptOut);
}


/*==========================================================
*  FUNCTION: void fnWriteNew(  unsigned int iFileSize )
*
*       This function appends the new strings to the end of the
*       existing        PCBTEXT file.
*
*  INPUT PARAMETERS: int iFlSize - size of existing pcbtext file.
*
*       RETURNS: none.
*---------------------------------------------------------*/
void fnWriteNew(unsigned iFlSize) {
  unsigned iNdx1;
  char     Buf[100];

  dosfseek(&fptOut, (iFlSize*80L), SEEK_SET );
  for (iNdx1 = iFlSize; iNdx1 < TEXT_ARRAY; iNdx1++) {
    sprintf(Buf,"%d%-79s",Array[iNdx1].cColor, Array[iNdx1].sptStr);
    dosfwrite(Buf,80,&fptOut);
  }
}


/*==========================================================
*  FUNCTION: void fnWrite_pcb( struct *array )
*
*       This function writes the default string array to the
*       output file.
*
*  INPUT PARAMETERS: ptr to an array of structures.
*
*       RETURNS: void
*---------------------------------------------------------*/
void fnWrite_pcb( pcbtexttype *Array ) {
  unsigned iNdx1;
  char     Buf[100];

  for (iNdx1 = 0; iNdx1 < TEXT_ARRAY; iNdx1++) {
    sprintf(Buf,"%d%-79s",Array[iNdx1].cColor, Array[iNdx1].sptStr);
    dosfwrite(Buf,80,&fptOut);
  }
}


/*===========================================================
*  FUNCTION:    fnFileOpen( char *sFile );
*
*       This function requests user to input the filename and
*       either opens an existing file or creates a new one if
*       the file doesn't exist.
*
*  INPUT PARAMETERS:    sFile - Ptr to string which will receive
*       filename.
*
*       RETURNS: none.
*----------------------------------------------------------*/
void fnFileOpen( char *sFile ) {
  bool        iQuery;
  unsigned    uiFilesize = 0;
  char static csFileMask[] =  { 15, ' ', '.', ':','-', '\\', '_', 0, 'A', 'Z', 0, 'a', 'z', 0, '0', '9' };

  boxcls( 0, 3, FILENAMELEN+33, 5, Colors[OUTBOX] , HDOUBLE );
  fastprint( 1, 4, "Enter filename to edit/convert: ", Colors[QUESTION] );
  KeyFlags = NOTHING;
  if (sFile[0] == 0) {
    inputstr( 33, 4, FILENAMELEN, "", sFile , sFile, csFileMask, INPUT_CAPS|INPUT_CLEAR, 0 );
    stripright( sFile, ' ' );
  } else {
    fastprint(33,4,sFile,Colors[ANSWER]);
  }

  if ( (sFile[0] == NULL && KeyFlags == RET) || KeyFlags == ESC ) {
    cls();
    exit (0);
  }

  iQuery = FALSE;
  if (fileexist(sFile) == 255) {
    inputnum( 1, 6, 1, "File not found. Create? (Y/N)", &iQuery, vBOOL, 0 );
    if ( iQuery == 0 ) {
      cls();
      exit (0);
    }
  }

  if (dosfopen(sFile ,OPEN_RDWR|OPEN_DENYNONE,&fptOut) != -1) {
    if (iQuery) {
      fastprint( 36, 6, "Creating File.", Colors[QUESTION] );
      fnWrite_pcb( Array );    /* make call to write strings to new file */
      clsbox( 36, 6, 64, 6, 1 );
    } else {
      fnFileSize(&uiFilesize);
      if (CurrentVersion >= 150) {
        if ( uiFilesize < TEXT_ARRAY ) {
            fnWriteChanges( uiFilesize );
            fnWriteNew( uiFilesize );
        } else {
          fnWriteHeader();
        }
        return;
      } else {
        fastprint( 12,  8, "File name given is not a Version 15.4 PCBTEXT file.",  Colors[QUESTION] );
        fastprint( 12,  9, "You can create a v15.4 File using a different name,",  Colors[QUESTION] );
        fastprint( 12, 10, "or this program will convert the existing file to",    Colors[QUESTION] );
        fastprint( 12, 11, "version 15.4. This is a destructive process because",  Colors[QUESTION] );
        fastprint( 12, 12, "your existing file will be altered. Be sure to make",  Colors[QUESTION] );
        fastprint( 12, 13, "a back-up of this file (particularly in the case of",  Colors[QUESTION] );
        fastprint( 12, 14, "foreign language translations) so that you can refer", Colors[QUESTION] );
        fastprint( 12, 15, "to it later if the need arises.",                      Colors[QUESTION] );
        inputnum( 12, 17, 1, "Type (Y)es to convert the file or (N)o to QUIT now.", &iQuery, vBOOL, 0 );
        if( iQuery == 0 ) {
          cls();
          exit(1);
        } else {
          fnWriteChanges( uiFilesize );
          fnWriteNew( uiFilesize );
          clsbox( 0, 7, 79, 20, 1 );
          return;
        }
      }
    }
  }
}


/*==========================================================
*  FUNCTION:    fnEditline( char *sTempbuff, int iNdx1 )
*
*  This function displays the current default-line with the
*       current file-line and allows editing of the file-line.
*       Also displays the current record number, record length,
*       and justification. Also sets up the keyboard help display.
*
*  INPUT PARAMETERS:    sTempbuff - ptr to default string to edit.
*                    iNdx1 - offset of the string in array.
*
*       RETURNS:     TRUE if changed, FALSE otherwise.
*---------------------------------------------------------*/
bool fnEditline( char *sTempbuff, unsigned iNdx1 ) {
  char EDITMASK[] = { 6, 0, 32, 126, 0, 128, 254 };
  char csNdx[4], csLength[3], csJustify[7], csDrawline[82];
  char csJustBuffer[81];
  char SaveBuffer[81];

  if (dosfread(sTempbuff,80,&fptOut) != 80) {
    fastprint( 2, 21, "Premature End-of-File, Aborting!", Colors[QUESTION] );
    exit (0);
  }

  memcpy(&sTempbuff[0], &sTempbuff[1], 79 );
  sTempbuff[Array[iNdx1].cLen] = 0;
  itoa( iNdx1 , csNdx, 10 );                      /* this code converts the index */
  itoa( Array[iNdx1].cLen, csLength, 10 );

  if (Array[iNdx1].cJust == 'R')                /* and justification fields to  */
    strcpy(csJustify,"Right");
  else
    strcpy(csJustify,"Left");        /* to ascii strings for display */

  boxcls( 0, 7, 79, 12, Colors[OUTBOX] , HDOUBLE );
  fastprint( 4, 8, "Record No.:    ", Colors[DISPLAY] );
  fastprint( 17, 8, csNdx, Colors[DISPLAY] );
  fastprint( 30, 8, "Record Length:   ", Colors[DISPLAY] );
  fastprint( 46, 8, csLength, Colors[DISPLAY] );
  fastprint( 56, 8, "Justification:    ", Colors[DISPLAY] );
  fastprint( 72, 8, csJustify, Colors[DISPLAY] );
  memset( csDrawline, '\xc4', 81 );   /*--------------------------------*/
  csDrawline[0] = '\xc3';             /* Build line with '-' char       */
  csDrawline[79] = '\xb4';            /* and put proper chars at ends   */
  csDrawline[80] = 0;                 /*--------------------------------*/
  fastprint( 0, 9, csDrawline, Colors[OUTBOX] );/* prints horiz. line */
  fastprint( 1, 10, Array[iNdx1].sptStr, 15 );
  if (EditFlag == EDIT ) {
    ExitKeyNum[0] = 60;     ExitKeyFlag[0] = F2;
    ExitKeyNum[1] = 61;     ExitKeyFlag[1] = F3;
    ExitKeyNum[2] = 62;     ExitKeyFlag[2] = F4;
    boxcls( 0, 14, 79, 21, Colors[OUTBOX], SINGLE );
    fastprint( 5, 15, " <ESC>       = Save & Quit.          <F2>        = Text-Search."       ,Colors[QUESTION] );
    fastprint( 5, 16, " <F3>        = Jump to Record #      <F4>        = Restore Default."   ,Colors[QUESTION] );
    fastprint( 5, 17, " <Up>        = Back 1 record.        <Dn>        = Ahead 1 record."    ,Colors[QUESTION] );
    fastprint( 5, 18, " <PgUp>      = Back 10 records.      <PgDn>      = Ahead 10 records."  ,Colors[QUESTION] );
    fastprint( 5, 19, " <Ctrl-PgUp> = Begin. of file.       <Ctrl-PgDn> = End of File."       ,Colors[QUESTION] );
    fastprint( 5, 20, "Use the tilde (~) character to add hard-spaces to the end of a string.",Colors[DISPLAY]  );
  } else
    fastprint( 1, 13, "Edit the line, then <ESC> or <ENTER> to return to SEARCH.",Colors[QUESTION] );

  memcpy(SaveBuffer,sTempbuff,80);

  KeyFlags = NOTHING;
  inputstr( 1, 11, Array[iNdx1].cLen,"", sTempbuff,sTempbuff, EDITMASK, INPUT_DEFAULT, 0 );

  if(Array[iNdx1].cJust == 'R') {
    strcpy( csJustBuffer, sTempbuff );
    stripboth( csJustBuffer, ' ' );
    sprintf( sTempbuff, "%*s%s", Array[iNdx1].cLen-strlen(csJustBuffer),"", csJustBuffer );
  }

  memset( ExitKeyNum, 0, NumEKeys );
  return(memcmp(SaveBuffer,sTempbuff,80) != 0);
}

/*==========================================================
*  FUNCTION:    void fnWriteline( char *sTempbuff, int iNdx1);
*
*       This function writes the current edit-line to the current
*       output file. It is executed upon any keystroke.
*
*  INPUT PARAMETERS:    sTempbuff - ptr to default string to edit.
*                    iNdx1 - offset of the string in array.
*       RETURNS:        none.
*---------------------------------------------------------*/
void fnWriteline( char *sTempbuff, unsigned iNdx1 ) {
  char Buf[100];

  dosrewind(&fptOut);
  dosfseek(&fptOut,iNdx1 * 80L,SEEK_SET);
  sprintf(Buf,"%d%-79s", Array[iNdx1].cColor, sTempbuff);
  dosfwrite(Buf,80,&fptOut);
  dosflush(&fptOut);
}


/*==========================================================
*  FUNCTION:    void fnSearch( int *iNdx1 )
*
*       This code searches the current file (users or default) for
*       a string that matches the one provided by user input.  If
*       a match is found, asks user to continue search or to edit
*       the string.
*
*  INPUT PARAMETERS: *iNdx1 - ptr to string offset. Set to 1
*       at call.
*
*       RETURNS:        none.
*---------------------------------------------------------*/
void fnSearch( unsigned *iNdx1 ) {
  char sTest[41], sTestsave[41];
  char sBuff[81];
  char sBuff2[81];
  char sItoStr[4];
  char cCh;
  char csTestMask[]       =       { 6, 0, 32, 126, 0, 128, 254 };


  EditFlag = SEARCH;
  clsbox( 0, 7, 79, 20, 1 );
  boxcls( 0, 7, 79, 12, Colors[OUTBOX] , DOUBLE );
  fastprint( 1, 8, "Enter search string: ", Colors[QUESTION] );
  inputstr( 23, 8, 40, "","", sTest, csTestMask, INPUT_CLEAR, 0 );
  stripright( sTest, ' ' );
  strcpy( sTestsave, sTest );

  for ( *iNdx1 = 1; *iNdx1 < TEXT_ARRAY ; (*iNdx1)++) {
    itoa( *iNdx1, sItoStr, 10 );
    memset( sBuff, '\0', 81 );
    memset( sBuff2, '\0', 81 );
    if (dosfread(sBuff, 80, &fptOut) != 80) {
      fastprint( 2, 21, "Premature End-of-File, Aborting!",Colors[QUESTION] );
      exit (0);
    }

    memcpy(&sBuff[0], &sBuff[1], 79 );
    sBuff[Array[*iNdx1].cLen] = 0;
    strcpy( sBuff2, sBuff );
    strupr( sBuff );
    strupr( sTest );
    if (strstr( sBuff, sTest ) != NULL ) {
      fastprint( 1, 9, "RECORD # FOUND:    ", Colors[QUESTION] );
      fastprint( 17, 9, sItoStr, Colors[ANSWER] );
      fastprint(  1, 10, sBuff2, Colors[ANSWER] );
      KeyFlags = NOTHING;
      while (KeyFlags != ESC && KeyFlags != RET) {
        ExitKeyNum[0] = 60;  ExitKeyFlag[0] = F2;
        inputnum( 1, 11, 1, "<ENTER> to find next, <F2> to edit this line, <ESC> to end search.",&cCh, vCHAR, 0 );

        if ( KeyFlags == F2 ) {
          KeyFlags = NOTHING;
          dosfseek(&fptOut, -80L, SEEK_CUR);
          clsbox( 0, 12, 79, 21, 01 );
          memset( ExitKeyNum, 0, NumEKeys );

          if (fnEditline( sBuff2, *iNdx1 ))
            fnWriteline( sBuff2, *iNdx1 );

          KeyFlags = NOTHING;
          clsbox( 0, 7, 79, 20, 1 );
          boxcls( 0, 7, 79, 12, Colors[OUTBOX] , DOUBLE );
          fastprint( 1, 8, "Enter search string: ", Colors[QUESTION] );
          fastprint( 23, 8, sTestsave, Colors[ANSWER] );
          fastprint( 1, 9, "RECORD # FOUND:    ", Colors[QUESTION] );
          fastprint( 17, 9, sItoStr, Colors[ANSWER] );
          fastprint(  1, 10, sBuff2, Colors[ANSWER] );
          continue;
        }

        if (KeyFlags == RET) {
          clsbox( 1, 10, 78, 10, 01 );
          clsbox( 0, 13, 79, 21, 01 );
          continue;
        }

        if (KeyFlags == ESC) {
          KeyFlags = NOTHING;
          dosfseek(&fptOut, 80L, SEEK_SET);
          clsbox( 0, 12, 79, 21, 01 );
          *iNdx1 = 1;
          EditFlag = EDIT;
          memset( ExitKeyNum, 0, NumEKeys );
          return;
        }
      }
    }
  }
  fastprint( 1, 21, "String Not Found.", Colors[QUESTION] );
  dosfseek(&fptOut, 80L, SEEK_SET);
  *iNdx1 = 1;
  EditFlag = EDIT;
  memset( ExitKeyNum, 0, NumEKeys );

}


void fnNewRecNum ( unsigned *iNdx1 ) {
 unsigned NewNum;
 unsigned Num;

  clsbox(1,8,20,8,Colors[QUESTION]);
  NewNum = Num = *iNdx1;
  inputnum(  1, 8, 3, "Record Number", &NewNum, vINT, 0 );
  if (KeyFlags != ESC) {
    if (NewNum < 1)
      Num = 1;
    else if (NewNum > TEXT_ARRAY-1)
      Num = TEXT_ARRAY-1;
    else
      Num = NewNum;
  }
  dosfseek(&fptOut, Num * 80L , SEEK_SET);
  *iNdx1 = Num;
  KeyFlags = NOTHING;
}


void main(int argc, char **argv) {
  char      sTempbuff[81];
  char      sFile[FILENAMELEN+1];
  unsigned  iNdx1;
  bool      Upgrade;
  int       X;
  char      *p;

  initscrnio();
  ShowClock = NOCLOCK;
  UpdateKbdStatus = FALSE;
  cls();
  boxcls( 0, 0, 79, 2, Colors[OUTBOX], HDOUBLE );
  fastcenter( 1, "PCBTEXT File Generator/Editor", Colors[QUESTION]);
  boxcls( 0, 22, 79, 24, Colors[OUTBOX], HDOUBLE );
  fastcenter(23, "Copyright (C) 1988-1997 Clark Development Company, Inc.", Colors[QUESTION]);

  sFile[0]     = 0;
  sTempbuff[0] = 0;
  iNdx1        = 0;
  Upgrade      = FALSE;

  if (argc > 1) {
    maxstrcpy(sFile,argv[1],sizeof(sFile));
    strupr(sFile);
  }

  for (X = 2; X < argc; X++) {
    maxstrcpy(sTempbuff,argv[X],sizeof(sTempbuff));
    strupr(sTempbuff);
    if (stricmp(sTempbuff,"/UPGRADE") == 0)
      Upgrade = TRUE;
    else if ((p = strstr(sTempbuff,"/I:")) != NULL) {
      iNdx1 = atoi(p+3);
      if (iNdx1 > TEXT_ARRAY-1)
        iNdx1 = 0;
    } else if (iNdx1 != 0)
      maxstrcpy(sTempbuff,argv[X],sizeof(sTempbuff));
  }

  if (sFile[0] != 0 && iNdx1 != 0 && fileexist(sFile) == 255) {
    sprintf(sTempbuff,"%s does not exist",sFile);
    cls();
    fastprint(0,0,sTempbuff,0x0F);
    goto exit;
  }

  fnFileOpen( sFile );

  if (Upgrade && sFile[0] != 0 && fptOut.handle > 0) {
    sprintf(sTempbuff,"%s has been upgraded",sFile);
    cls();
    fastprint(0,0,sTempbuff,0x0F);
    goto exit;
  }

  if (iNdx1 != 0 && sTempbuff[0] != 0 && sFile[0] != 0 && fptOut.handle > 0) {
    dosfseek(&fptOut,iNdx1*80L,SEEK_SET);
    fnWriteline(sTempbuff,iNdx1);
    sprintf(sTempbuff,"Record #%d has been upgraded in %s.",iNdx1,sFile);
    cls();
    fastprint(0,0,sTempbuff,0x0F);
    goto exit;
  }


  dosfseek(&fptOut, 80L, SEEK_SET);
  iNdx1 = 1;


  do {      /* This is the main keyboard handling code.  It dispatches    */
            /* the program flow to the appropriate function.              */

    if (fnEditline(sTempbuff,iNdx1))
      fnWriteline(sTempbuff,iNdx1);

/*  clsbox( 0, 21, 79, 21, 1); */
    switch ( KeyFlags ) {
      case RET     :
      case DN      : if (iNdx1 < (TEXT_ARRAY - 1))
                       iNdx1++;
                     break;

      case UP      : if (iNdx1 > 1)
                       iNdx1--;
                     break;

      case PGDN    : if (iNdx1 > (TEXT_ARRAY - 11))
                       iNdx1 = TEXT_ARRAY - 1;
                     else
                       iNdx1 += 10;
                     break;

      case PGUP    : if (iNdx1 < 11)
                       iNdx1 = 1;
                     else
                       iNdx1 -= 10;
                     break;

      case CTRLPGUP: iNdx1 = 1;
                     break;

      case CTRLPGDN: iNdx1 = TEXT_ARRAY - 1;
                     break;

      case F2      : dosrewind(&fptOut);
                     dosfseek(&fptOut, 80L, SEEK_SET );
                     iNdx1 = 1;
                     fnSearch(&iNdx1);
                     break;

      case F3      : fnNewRecNum(&iNdx1);
                     break;

      case F4      : strcpy( sTempbuff, Array[iNdx1].sptStr );
                     fnWriteline( sTempbuff, iNdx1 );
                     break;
    }

    dosfseek(&fptOut,iNdx1 * 80L,SEEK_SET);
  } while ( KeyFlags != ESC );

  memset( ExitKeyNum, 0, NumEKeys );
  cls();

exit:
  gotoxy(0,1);
  dosfclose (&fptOut);
}
