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


/***************************************************************************
 *
 * Copyright (c) 1993   All Rights Reserved
 * Clark Development Company, Inc.
 *
 * $Revision:   1.7  $
 *
 * $Log:   E:/tc/pcbpack/vcs/help.c_v  $
 * 
 *    Rev 1.7   20 Sep 1995 18:43:14   DWT
 * Changed to LIBENTRY
 *
 *    Rev 1.6   22 Mar 1995 15:26:06   DWT
 * Fixed the /RENUMBER shortcut.
 *
 *    Rev 1.5   04 Feb 1995 15:08:46   DWT
 * Updated help...
 *
 *    Rev 1.4   19 Apr 1994 11:51:36   DWT
 * Modified the way command line help is printed in order to accent the appearance
 * of abbreviations.
 *
 *    Rev 1.3   30 Nov 1993 10:36:00   DWT
 * Added /THRESHOLD help
 *
 *    Rev 1.2   11 Nov 1993 10:43:56   DWT
 * changed to LIBENTRY
 *
 *    Rev 1.1   13 Oct 1993 09:25:06   DWT
 * changed the way memcheck is used
 *
 *    Rev 1.0   19 Aug 1993 19:32:38   LDZ
 * Initial revision.
 *
 ***************************************************************************/

#include <ctype.h>
#include <screen.h>
#include "pcbpack.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

void LIBENTRY highprint(int X, int Y, char *Str) {
  int VidOffset;

  for (VidOffset = xyvalue(X,Y); ; X++, Str++, VidOffset += 2) {
    if (*Str == 0)
      return;
    if (*Str == ':')
      break;
    if (*Str == '/' || isupper(*Str))
      fastputch(VidOffset,*Str,ScreenColor->Intense);
    else
      fastputch(VidOffset,(char) toupper(*Str),ScreenColor->Normal);
  }

  for (; *Str; X++, Str++, VidOffset += 2)
    fastputch(VidOffset,*Str,ScreenColor->Normal);
}


/***************************************************************************
 *** Display help for the user *********************************************/
void LIBENTRY ShowHelp(void) {
/*** show the first screen and then pause waiting for an ESC to be *********/
/*** pressed to abort, or any other key pressed to continue ****************/

    fastprint(0,  0, "Command Line:", ScreenColor->Intense);
    fastprint(15, 0, "PCBPACK /AR[EA]:nn;nn-nn [options]", ScreenColor->Help);
    highprint( 4, 2,     "/ARea:nn"         );
    fastprint(22, 2,                       "Specify which areas to process (/AREA:ALL = All areas)", ScreenColor->Normal);
    highprint( 4, 3,     "/CAp:<filename>"  );
    fastprint(22, 3,                       "Specify capture file <filename>", ScreenColor->Normal);
    highprint( 4, 4,     "/CRc:nn"          );
    fastprint(22, 4,                       "Keep a CRC database for nn number of days", ScreenColor->Normal);
    highprint( 4, 5,     "/DaTe:mmddyy"     );
    fastprint(22, 5,                       "Purge messages older than given date (MM/DD/YY).", ScreenColor->Normal);
    highprint( 4, 6,     "/DaYs:nn"         );
    fastprint(22, 6,                       "Purge messages older than nn number of days.", ScreenColor->Normal);
    highprint( 4, 7,     "/FAst"            );
    fastprint(22, 7,                       "Display a minimum of information to the user.", ScreenColor->Normal);
    highprint( 4, 8,     "/IGnore"          );
    fastprint(22, 8,                       "Ignore Imbedded Packout Dates", ScreenColor->Normal);
    highprint( 4, 9,     "/INdex"           );
    fastprint(22, 9,                       "Create index files for messages.", ScreenColor->Normal);
    highprint( 4,10,     "/KEep[:nn]"       );
    fastprint(22,10,                       "Keep PRIVATE messages that haven't yet been received.", ScreenColor->Normal);
    highprint( 4,11,     "/KillBak"         );
    fastprint(22,11,                       "Remove backup files after processing.", ScreenColor->Normal);
    highprint( 4,12,     "/KillDups"        );
    fastprint(22,12,                       "Remove unecessary duplicate messages.", ScreenColor->Normal);
    highprint( 4,13,     "/MAxmsgs:nn"      );
    fastprint(22,13,                       "Maximum number of messages to keep.", ScreenColor->Normal);
    highprint( 4,14,     "/MinMsgs:nn"      );
    fastprint(22,14,                       "Minimum number of messages to keep.", ScreenColor->Normal);
    highprint( 4,15,     "/NoCaller"        );
    fastprint(22,15,                       "No output will be written to the callers log.", ScreenColor->Normal);
    highprint( 4,16,     "/OLdindex"        );
    fastprint(22,16,                       "Maintain \"old-style\" index files.", ScreenColor->Normal);
    highprint( 4,17,     "/PUrge[:nn]"      );
    fastprint(22,17,                       "Purge PRIVATE messages that have been received..", ScreenColor->Normal);
    highprint( 4,18,     "/RAnge:low-high"  );
    fastprint(22,18,                       "Remove a given range of messages.", ScreenColor->Normal);
    highprint( 4,19,     "/REnumber:nn"     );
    fastprint(22,19,                       "Renumber message base beginning at 'nn'.", ScreenColor->Normal);
    highprint( 4,20,     "/RePort"          );
    fastprint(22,20,                       "Generate REPORT for specified /AREA:", ScreenColor->Normal);
    highprint( 4,21,     "/THreshold:nn"    );
    fastprint(22,21,                       "Minimum Active Messages Pack Threshold", ScreenColor->Normal);
    highprint( 4,22,     "/UpCase"          );
    fastprint(22,22,                       "Convert all SUBJECT fields to upper case.", ScreenColor->Normal);
    highprint( 4,23,     "/UPdate"          );
    fastprint(22,23,                       "Update IDX files for specified /AREA:", ScreenColor->Normal);



/****
    /AREA:nn        Specify which areas to be processed

    /CAP:<file>     Specify output to a capture file and capture file name

    /CRC:nn         Keep CRC database for nn number of days

    /DATE:mmddyy    Purge all messages dated older than "date"

    /DAYS:nn        Purge all messages older than nn number of days

    /DEBUG:nn       Specify a debugging output level (debugging only)

    /FAST           Minimal display output (synonym for /QUIET)

    /FILE:<file>    Specify which PCBOARD.DAT file to use

    /FIX            Repair problems with high and low message numbers being
                    inaccurate as well as number of active
                    (synonym for /REPAIR)

    /FORCE          Force PCBPack to run with nodes up (obsolete)

    /GAP[:nn]       Tell PCBPack to remove any "gaps" between messages
                    (special)

    /HELP           Display the help screen

    /INDEX          Tell PCBPack to only update the indices

    /KEEP:nn        Keep unreceived private messages newer than nn number of
                    days

    /KILLALL        Remove ALL messages leaving only the header statistics

    /KILLBAK        Remove BAK files after processing

    /KILLDUPS       Remove duplicate messages from an area

    /MAXMSGS:nn     Tell PCBPack to keep only nn number of messages

    /MINMSGS:nn     Tell PCBPack to keep AT LEAST nn number of messages

    /NOCALLER       Log NO information to the callers log

    /NOSIZE         Perform no drive size check (obsolete)

    /OLDINDEX       Keep old NDX files up to date

    /PURGE:nn       Remove received private messages nn number of days old

    /QUIET          Minimal display output (synonym for /FAST)

    /RANGE          Remove a range of messages from an area

    /RENUMBER:nn    Renumber a message base starting at nn

    /REPAIR         Repair problems with high and low message number being
                    inaccurate as well as number of active
                    (synonym for /FIX)

    /REPORT         Report on the high, low, and active messages in an area

    /TIMEOUT        Specify a different timeout value for areas that are in
                    use

    /THRESHOLD:nn   Specifies the minimum number of active messages before a
                    pack will occur on a conference

    /UPCASE         Convert TO, FROM, and SUBJECT lines to upper case

    /UPDATE         Update new IDX files from message area information

 ****/
}

