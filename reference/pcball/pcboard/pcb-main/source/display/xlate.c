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


#include "project.h"
#pragma hdrstop

#include "pcbmacro.h"
#include "messages.h"
#include "event.h"

#ifdef LIB
#include "usersys.h"
#endif


static char *StartToken;

/********************************************************************
*
*  Function:  guardedmoreprompt()
*
*  Desc    :  A situation now exists where PCBoard tries to GUARD the top lines
*             of the screen where the Message Header exists when reading
*             messages with scrolling disabled.
*
*             The trouble is that the following conditions must be met:
*
*             1) When the screen fills up the moreprompt should automatically
*                clear the message body while protecting the header remains.
*
*             2) Yet, if an embedded @MORE@, @WAIT@ or @PAUSE@ is encountered,
*                this should NOT be enterpreted as a signal to clear the body
*                of the message!  Nor should any of these reset the line
*                counter, lest the message header now be forced to scroll off
*                the top of the screen!
*
*             3) However, it is possible for an @CLS@ to be embedded.  This
*                will clear the entire screen - including the header.  At this
*                point there is nothing left to guard so the scrolling should
*                be ENABLED - this will allow any potential 'pictures' to
*                display properly.
*
*             These ideas have been implemented in two parts as follows:
*
*             a) This guardedmoreprompt() is a front-end to the moreprompt()
*                function.  It is called when the @MORE@, @WAIT@ or @PAUSE@
*                macros are encountered.  Basically, they avoid clearing the
*                screen and they purposely keep the line counter unchanged.
*                This solves the problem in point #2 above.
*
*             b) The problem in point #3 is solved by disabing the any further
*                'guarding' of the header by setting Display.ClearScrnWhenFull
*                to FALSE down in the printxlated() function.
*/

#ifndef LIB
static void _NEAR_ LIBENTRY guardedmoreprompt(moretype Type) {
  char X;
  bool ClearScrnWhenFull;

  checkstack();
  X = Display.NumLinesPrinted;
  ClearScrnWhenFull = Display.ClearScrnWhenFull;
  Display.ClearScrnWhenFull = FALSE;
  moreprompt(Type);
  if (ClearScrnWhenFull) {
    Display.ClearScrnWhenFull = TRUE;
    Display.NumLinesPrinted = X;
  }
}
#endif


/********************************************************************
*
*  Function:  getvar()
*
*  Desc    :  This function searches for an environment variable using Str to
*             point to the name of the environment variable to find.
*
*             Str contains a value such as "@ENV=PHONE@" or "@ENV=PHONE:10R@".
*
*             This function first jumps to the character after the "=" and then
*             it strips off the trailing : or @ characters, thus reducing the
*             name of the environment variable down to just the root word (i.e.
*             PHONE in the above example).
*
*             The sysop uses a "SET @PHONE@=" statement to put the variable
*             into the environment.  The @'s are used to protect against
*             accessing just any environment variable (i.e. for security
*             reasons).
*
*             So, getvar() next adds @'s around the name (i.e. "@PHONE@") and
*             then calls getenv() to see if the environment variable exists.
*             If it does, it then copies the environment variable contents
*             into the string pointed to by Value.
*
*  Returns :  TRUE if the environment variable was found and Value filled in.
*             Otherwise FALSE.
*/

static bool _NEAR_ LIBENTRY getvar(char *Value, char *Str, int MaxLen) {
  char *p;
  char Temp[128];
  char Var[128];

  checkstack();
  if ((p = strchr(Str,'=')) != NULL) {
    maxstrcpy(Temp,p+1,MaxLen);
    if ((p = strchr(Temp,'@')) != NULL) {
      *p = 0;
      if ((p = strchr(Temp,':')) != NULL)
        *p = 0;
      strupr(Temp);
      sprintf(Var,"@%s@",Temp);
      if ((p = getenv(Var)) != NULL) {
        maxstrcpy(Value,p,128);
        return(TRUE);
      }
    }
  }
  return(FALSE);
}


/********************************************************************
*
*         Where  first indicates the user's first name, last indicates  the
*         user's last name, and domain is your system's domain name or UUCP
*         name.  If the user has middles names then those are also included
*         in the address.
*
*         Example:  John Brown Smith  = john.brown.smith@foobar.com
*
*         If  you have set that underbars be used instead of periods,  then
*         the above example would be:
*
*              john_brown_smith@foobar.com
*
*         BBS  user  names  can also contain illegal characters,  thus  the
*         user's name undergoes a transformation.  The following table show
*         that function.
*
*            BBS Character                RFC1137 Character(s)
*               (space)                        . (period)
*             . (period)                           #d#
*                  _                               #u#
*                  (                               #l#
*                  )                               #r#
*                  ,                               #m#
*                  :                               #c#
*                  \                               #b#
*                  #                               #h#
*                  =                               #e#
*                  /                               #s#
*                  >                               #o#
*                  <                               #p#
*                  *                               #i#
*            any extended             #xxx# (xxx is an ASCII code)
*              character
*
*         Example:
*            Enrique T. Rodriguez = enrique.t#d#.rodriguez@foobar.com
*/

static void _NEAR_ LIBENTRY inetname(char *Dest, char *Srce, int MaxLen) {
  int   CurLen;
  char *p;
  char  SrchStr[2];
  char  RepStr[6];

  // make room for the @domain characters.
  MaxLen -= (strlen(PcbData.uucpDomainName)+1);

  strcpy(Dest,Srce);                        // copy current name
  strlwr(Dest);                             // change to lowercase letters
  substitute(Dest,"#" ,"#h#",MaxLen);       // change # to #h#
  substitute(Dest,"." ,"#d#",MaxLen);       // change . to #d#
  substitute(Dest,"_" ,"#u#",MaxLen);       // change _ to #u#
  substitute(Dest,"(" ,"#l#",MaxLen);       // change ( to #l#
  substitute(Dest,")" ,"#r#",MaxLen);       // change ) to #r#
  substitute(Dest,"," ,"#m#",MaxLen);       // change , to #m#
  substitute(Dest,":" ,"#c#",MaxLen);       // change : to #c#
  substitute(Dest,"\\","#b#",MaxLen);       // change \ to #b#
  substitute(Dest,"=" ,"#e#",MaxLen);       // change = to #e#
  substitute(Dest,"/" ,"#s#",MaxLen);       // change / to #s#
  substitute(Dest,">" ,"#o#",MaxLen);       // change > to #o#
  substitute(Dest,"<" ,"#p#",MaxLen);       // change < to #p#
  substitute(Dest,"*" ,"#i#",MaxLen);       // change * to #i#

  // substitute high ascii characters
  for (p = Dest, CurLen = strlen(Dest); p < &Dest[CurLen]; p++) {
    if (*p > 127) {
      SrchStr[0] = *p;
      SrchStr[1] = 0;
      sprintf(RepStr,"#%d#",*p);
      substitute(Dest,SrchStr,RepStr,MaxLen);
      CurLen = strlen(Dest);
    }
  }

  change(Dest,' ',PcbData.uucpSeparator[0]); // change spaces to periods (or whatever is defined)
  addchar(Dest,'@');                         // add @
  strcat(Dest,PcbData.uucpDomainName);       // add domain name
}


/********************************************************************
*
*  Function:  expandtoken()
*
*  Desc    :  Expands a token in memory and returns the modified value in the
*             Temp string.
*
*  Notes   :  There are two switch statements.  If printxlated() calls us, the
*             PerformAction variable will be set to TRUE.  This will tell us
*             to clear the screen, change colors, and so on.  If PerformAction
*             is FALSE, then we are to only expand the macro, when appropriate,
*             and return back to the caller of this function.
*
*  Returns :  TRUE  = instructs printxlated() to print the Temp value
*             FALSE = indicates that no processing was performed on the Temp
*                     value and that printxlated() should not print it.
*
*/

static bool _NEAR_ LIBENTRY expandtoken(int Token, char *Temp, bool PerformAction) {
  char static SavedColor;
  unsigned    Time;
  int         Num;
  #ifndef LIB
  long        BytesLeft;
  #endif
  pcbtexttype Buf;

  checkstack();
  /* if called from printxlated(), then we want to act upon those macros */
  /* which perform an action, such as clearing the screen, changing the  */
  /* color, pausing the display, etc. As soon as the action is complete  */
  /* we will return back to the caller of this function.                 */

  if (PerformAction) {
    switch (Token) {
      case AUTOMORE        : Display.AutoMore = TRUE; return(FALSE);
      case BEEP            : bell(); return(FALSE);
      case CLREOL          : cleareol(); return(FALSE);
      case CLS             : printcls();
                             /* disable the msg header protection since the */
                             /* rest of the message might be a picture.     */
                             #ifndef LIB
                               Display.ClearScrnWhenFull = FALSE;
                             #endif
                             return(FALSE);
      case DELAY           :
                             #ifndef LIB
                             /* don't allow @delay@ if reading msgs */
                             if (! Status.MsgRead)
                             #endif
                               tickdelay(((unsigned) FindTokenColor * ONESECOND) / 10);
                             return(FALSE);
      #ifndef LIB
      case MORE            : guardedmoreprompt(MOREPROMPT); return(FALSE);
      case PAUSE           : Display.AutoMore = TRUE; guardedmoreprompt(MOREPROMPT); Display.AutoMore = FALSE; return(FALSE);
      #else
      case MORE            : moreprompt(MOREPROMPT); return(FALSE);
      case PAUSE           : Display.AutoMore = TRUE; moreprompt(MOREPROMPT); Display.AutoMore = FALSE; return(FALSE);
      #endif
      case POFF            : startdisplay(FORCENONSTOP);    Status.Poff = TRUE;  return(FALSE);
      case PON             : startdisplay(FORCECOUNTLINES); Status.Poff = FALSE; return(FALSE);
      case POS             :
                             #ifndef LIB
                               if (! Display.ShowOnScreen)
                                 Num = Display.NumCharsPrinted + 1;
                               else
                                 Num = awherex() + 1;
                             #else
                               Num = awherex() + 1;
                             #endif
                             if (FindTokenColor > Num) {
                               FindTokenColor -= (char) Num;
                               Temp[0] = 0;
                               break;
                             }
                             return(FALSE);
      case QOFF            : Display.Break = FALSE; return(FALSE);
      case QON             : Display.Break = TRUE;  return(FALSE);
      #ifndef LIB
      case WAIT            : guardedmoreprompt(PRESSENTER); return(FALSE);
      case WHO             : if (PcbData.Network)
                               displayusernet(0);
                             else
                               displaypcbtext(TXT_NONETWORKACTIVE,NEWLINE|LFBEFORE);
                             return(FALSE);
      #else
      case WAIT            : moreprompt(PRESSENTER); return(FALSE);
      #endif
      case XOFF            : Status.DisableColor = TRUE; return(FALSE);
      case XON             : Status.DisableColor = FALSE; return(FALSE);
      case XCOLORS         : if (Status.DisableColor) {
                               sprintf(Temp,"@X%02X",FindTokenColor);
                               return(TRUE);
                             }
                             switch (FindTokenColor) {
                               case 0x00: SavedColor = curcolor();    break;
                               case 0xFF: printcolor(SavedColor);     break;
                               default  : printcolor(FindTokenColor); break;
                             }
                             return(FALSE);
    }
  }

  /* If we aren't processing action macros, or if the macro found was not an */
  /* action macro, then we'll fall thru to this switch statement.  If it     */
  /* really *is* an action macro, we'll just ignore it.  Otherwise, we'll    */
  /* modify the Temp[] value "in place" and return back to the caller of     */
  /* this function.                                                          */

  switch (Token) {
    case ALIAS           : strcpy(Temp,UsersData.Alias); break;
    #ifndef LIB
    case BICPS           : lascii(Temp,XferVars.CPSboth); break;
    #endif
    case BOARDNAME       : strcpy(Temp,PcbData.BoardName); break;
    case BPS             : lascii(Temp,Asy.ConnectSpeed); break;
    #ifndef LIB
    case BYTECREDIT      : comma(Temp,((long)Status.KByteRatioCredits)*1024L); break;
    case BYTELIMIT       : if (Status.BytesRemaining == -1) {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           } else {
                             comma(Temp,(long) Status.MaxKBytesAllowed << 10);
                           }
                           break;
    case BYTERATIO       : byteratio(Temp);
                           break;
    case BYTESLEFT       : if ((BytesLeft = totalbytesleft()) == -1) {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           } else {
                             comma(Temp,BytesLeft);
                           }
                           break;
    #endif
    case CARRIER         : lascii(Temp,Asy.CarrierSpeed); break;
    case CITY            : strcpy(Temp,usercity()); break;
    #ifndef LIB
    case CONFNAME        : strcpy(Temp,Status.CurConf.Name); break;
    case CONFNUM         : comma(Temp,Status.Conference); break;
    case CURMSGNUM       : lascii(Temp,curmsgnum()); break;
    #endif
    #if defined(PCB152) && ! defined(LIB)
    case CREDLEFT        : if (Status.ActStatus == ACT_ENFORCE) {
                             calculatebalance();
                             creditcurrency(Temp,Status.Balance);
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    case CREDNOW         : if (Status.ActStatus != ACT_DISABLED) {
                             calculatebalance();
                             creditcurrency(Temp,UsersData.Account.StartThisSession - Status.Balance);
                           } else {
                             Temp[0] = '0';
                             Temp[1] = 0;
                           }
                           break;
    case CREDSTART       : if (Status.ActStatus != ACT_DISABLED) {
                             creditcurrency(Temp,UsersData.Account.StartingBalance);
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    case CREDUSED        : if (Status.ActStatus != ACT_DISABLED) {
                             calculatebalance();
                             creditcurrency(Temp,UsersData.Account.StartingBalance - Status.Balance);
                           } else {
                             Temp[0] = '0';
                             Temp[1] = 0;
                           }
                           break;
    #endif
    case DATAPHONE       : strcpy(Temp,UsersData.BusDataPhone); break;
    case DAYBYTES        : comma(Temp,UsersData.DailyDnldBytes); break;
    #ifndef LIB
    case DIRNAME         : if (Status.CurDirName != NULL)
                             maxstrcpy(Temp,Status.CurDirName,128);
                           else
                             Temp[0] = 0;
                           break;
    case DIRNUM          : if (Status.CurDirName != NULL)
                             lascii(Temp,Status.CurDirNum);
                           else
                             Temp[0] = 0;
                           break;
    #endif
    case DLBYTES         : dcomma(Temp,UsersData.TotDnldBytes); break;
    case DLFILES         : comma(Temp,UsersData.NumDownloads); break;
    case ENV             : if (getvar(Temp,StartToken,FindTokenEnd))
                             break;
                           return(FALSE);
    #ifndef LIB
    case EVENT           : nextevent(Temp); break;
    #endif
    case EXPDATE         : if (PcbData.SubscriptMode)
                             strcpy(Temp,juliantodate(UsersData.RegExpDate));
                           else
                             strcpy(Temp,"00-00-00");
                           countrydate(Temp);
                           break;
    case EXPDAYS         : if (PcbData.SubscriptMode && UsersData.RegExpDate != 0) {
                             if (UsersData.RegExpDate >= Status.JulianLogonDate)
                               comma(Temp,UsersData.RegExpDate - Status.JulianLogonDate);
                             else {
                               Temp[0] = '-';
                               comma(&Temp[1],- ((long) UsersData.RegExpDate - (long) Status.JulianLogonDate));
                             }
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    #ifndef LIB
    case FBYTES          : comma(Temp,numbytesflagged()); break;
    case FFILES          : comma(Temp,numfilesflagged()); break;
    case FNUM            : lascii(Temp,numfilesflagged()+1); break;
    case FILECREDIT      : comma(Temp,Status.FileRatioCredits); break;
    case FILERATIO       : fileratio(Temp); break;
    #endif
    case FIRSTU          : strcpy(Temp,Status.FirstName); strupr(Temp); break;
    case FIRST           : strcpy(Temp,Status.FirstName); break;
    #ifndef LIB
    case FREESPACE       : ucomma(Temp,diskfreespace(Status.CurConf.PrvUpldLoc)); break;
    case HIGHMSGNUM      : lascii(Temp,highmsgnum()); break;
    #endif
    case HOMEPHONE       : strcpy(Temp,UsersData.HomeVoicePhone); break;
    #ifndef LIB
    case INAME           : inetname(Temp,Status.DisplayName,128);
                           break;
    case INCONF          : makeconfstr(Temp,Status.Conference); break;
    case KBLEFT          : if (totalbytesleft() == -1) {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           } else {
                             comma(Temp,totalbytesleft() >> 10);
                           }
                           break;
    case KBLIMIT         : if (Status.BytesRemaining == -1 && Status.TotalKByteLimit == 0) {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           } else {
                             if (Status.TotalKByteLimit != 0 && Status.TotalKByteLimit < Status.MaxKBytesAllowed)
                               comma(Temp,Status.TotalKByteLimit);
                             else
                               comma(Temp,Status.MaxKBytesAllowed);
                           }
                           break;
    #ifdef PCBSTATS
    case LASTCALLERSYSTEM: lastcaller(Temp,FALSE); break;
    case LASTCALLERNODE  : lastcaller(Temp,TRUE); break;
    #else
    case LASTCALLERSYSTEM:
    case LASTCALLERNODE  : Temp[0] = 0; break;
    #endif
    #endif
    case LASTDATEON      : strcpy(Temp,juliantodate(UsersData.LastDateOn));
                           countrydate(Temp);
                           break;
    case LASTTIMEON      : strcpy(Temp,UsersData.LastTimeOn); break;
    case LOGDATE         : strcpy(Temp,Status.LogonDate); countrydate(Temp); break;
    case LOGTIME         : strcpy(Temp,Status.LogonTime); Temp[5] = 0; break;
    #ifndef LIB
    case LMR             : lascii(Temp,MsgReadPtr[Status.Conference]); break;
    case LOWMSGNUM       : lascii(Temp,lowmsgnum()); break;
    case MAXBYTES        : if (Status.TotalKByteLimit == 0) {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           } else
                             comma(Temp,Status.TotalKByteLimit*1024);
                           break;
    case MAXFILES        : if (Status.TotalFileLimit == 0) {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           } else
                             comma(Temp,Status.TotalFileLimit);
                           break;
    case MINLEFT         : ascii(Temp,xferminutesleft()); break;
    #endif
    case MSGLEFT         : comma(Temp,UsersData.MsgsLeft); break;
    case MSGREAD         : comma(Temp,UsersData.MsgsRead); break;
    case NOCHAR          : Temp[0] = NoChar; Temp[1] = 0;  break;
    case NODE            : if (PcbData.Network) {
                             ascii(Temp,PcbData.NodeNum);
                             break;
                           }
                           return(FALSE);
    #ifndef LIB
    case NUMBLT          : comma(Temp,numblts()); break;
    case NUMCALLS        : comma(Temp,(Status.CallerNumber != 0 ? Status.CallerNumber : numcallers()+1)); break;
    case NUMDIR          : Num = numdirs(Temp);  /* Temp gets filled with the name of DIR.LST */
                           comma(Temp,Num);      /* Replace contents of Temp now with number */
                           break;
    #endif
    case NUMCONF         : comma(Temp,PcbData.NumConf); break;
    case NUMTIMESON      : comma(Temp,UsersData.NumTimesOn); break;
    case OFFHOURS        : sprintf(Temp,"%s-%s",PcbData.AllowLowStrt,PcbData.AllowLowStop); break;
    case OPTEXT          : strcpy(Temp,Status.DisplayText); break;
    case POS             : break;  /* fall thru and let it expand the string */
    #ifndef LIB
    case PROLTR          : Temp[0] = UsersData.Protocol; Temp[1] = 0; break;
    case PRODESC         : strcpy(Temp,Protocols[UsersData.Protocol-'A'].Desc); break;
    case PWXDATE         : if (PasswordSupport) {
                             if (UsersData.PwrdHistory.ExpireDate != 0) {
                               strcpy(Temp,juliantodate(UsersData.PwrdHistory.ExpireDate));
                             } else {
                               getpcbtext(TXT_UNLIMITED,&Buf);
                               strcpy(Temp,Buf.Str);
                             }
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    case PWXDAYS         : if (PasswordSupport) {
                             if (UsersData.PwrdHistory.ExpireDate != 0) {
                               if (Status.JulianLogonDate >= UsersData.PwrdHistory.ExpireDate) {
                                 Temp[0] = '0';
                                 Temp[1] = 0;
                               } else
                                 ascii(Temp,UsersData.PwrdHistory.ExpireDate - Status.JulianLogonDate);
                             } else {
                               getpcbtext(TXT_UNLIMITED,&Buf);
                               strcpy(Temp,Buf.Str);
                             }
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    #endif
    #ifndef LIB
    case RATIOBYTES      : if (Status.ByteRatio != 0) {
                             decimal(Temp,Status.ByteRatio);
                             strcat(Temp,":1");
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    case RATIOFILES      : if (Status.FileRatio != 0) {
                             decimal(Temp,Status.FileRatio);
                             strcat(Temp,":1");
                           } else {
                             getpcbtext(TXT_UNLIMITED,&Buf);
                             strcpy(Temp,Buf.Str);
                           }
                           break;
    case RBYTES          : comma(Temp,XferVars.SuccessfulUpldBytes); break;
    case RCPS            : lascii(Temp,XferVars.CPSup); break;
    case REALNAME        : /* NOTE: it's really @REAL@ */ strcpy(Temp,UsersData.Name); break;
    case RFILES          : ascii(Temp,XferVars.SuccessfulUpldFiles); break;
    case SBYTES          : comma(Temp,XferVars.SuccessfulDnldBytes); break;
    case SCPS            : lascii(Temp,XferVars.CPSdn); break;
    #endif
    case SECLEVEL        : /* NOTE:  it's really @SECURITY@ */ ascii(Temp,Status.CurSecLevel); break;
    #ifndef LIB
    case SFILES          : ascii(Temp,XferVars.SuccessfulDnldFiles); break;
    #endif
    case SYSDATE         : datestr(Temp);
                           countrydate(Temp);
                           break;
    case SYSOPIN         : strcpy(Temp,PcbData.SysopStart); break;
    case SYSOPOUT        : strcpy(Temp,PcbData.SysopStop);  break;
    case SYSTIME         : timestr2(Temp); break;
    #ifndef LIB
    case TIMELIMIT       : ascii(Temp,Status.PwrdTimeAllowed); break;
    #endif
    case TIMELEFT        : Num = minutesleft();
                           if (Num < 0 && Control.WatchSessionClock)
                             loguseroff(ALOGOFF);
                           ascii(Temp,Num);
                           break;
    case TIMEUSED        : Time = currentminute();
                           if (Time < (unsigned) Status.LogonMinute)
                             Time += 1440;
                           ascii(Temp,Time-Status.LogonMinute);
                           break;
    case TOTALTIME       : Time = currentminute();
                           if (Time < (unsigned) Status.LogonMinute)
                             Time += 1440;
                           #ifdef LIB
                             ascii(Temp,Time-Status.LogonMinute+UsersData.ElapsedTimeOn);
                           #else
                             ascii(Temp,Time-Status.LogonMinute+Status.PrevTimeOn);
                           #endif
                           break;
    case UPBYTES         : dcomma(Temp,UsersData.TotUpldBytes); break;
    case UPFILES         : comma(Temp,UsersData.NumUploads); break;
    case USERNAME        : /* NOTE: it's really @USER@ */ strcpy(Temp,Status.DisplayName); break;
    case YESCHAR         : Temp[0] = YesChar; Temp[1] = 0; break;

    /* if it wasn't one of the above, then it may be an action macro, so */
    /* return now without performing any further processing.             */
    default              : return(FALSE);
  }

  /* From here down, FindTokenColor is a field length modifier - specifying */
  /* the length of the field, for left, right, and center justification.    */

  Num = strlen(Temp);

  /* if the string is already longer than FieldLen, or if FieldLen is        */
  /* maliciously set to something longer than 80 characters, then return now */

  if (FindTokenColor > 80)
    return(TRUE);

  if (Num > FindTokenColor) {
    /* 'T' means to TRUNCATE the field if it is longer than the fieldlength */
    if (FindTokenAttr == 'T') {
      Temp[FindTokenColor] = 0;
      return(TRUE);
    } else {
      /* otherwise, get out now */
      return(TRUE);
    }
  }

  FindTokenColor -= (char) Num;
  switch (FindTokenAttr) {
    case 'R': memmove(&Temp[FindTokenColor],Temp,Num+1);
              memset(Temp,' ',FindTokenColor);
              break;
    case 'C': Num = FindTokenColor;
              Num >>= 1;
              FindTokenColor -= (char) Num;
              memmove(&Temp[FindTokenColor],Temp,strlen(Temp)+1);
              memset(Temp,' ',FindTokenColor);
              FindTokenColor = (char) Num;
              /* fall through! */
    default : Num = strlen(Temp);
              memset(&Temp[Num],' ',FindTokenColor);
              Temp[Num+FindTokenColor] = 0;
              break;
  }
  return(TRUE);
}


/********************************************************************
*
*  Function:  printxlated()
*
*  Desc    :  Prints a string translating any special codes.
*
*  Notes   :  The findtoken() function returns a 0 value if no token was found.
*             Otherwise, it returns a number which is used by typedef enum
*             constants and a switch statement in order to determine which
*             token was found.  In printxlated(), this switch statement has
*             been moved out and is now found in the expandtoken() function.
*
*             To aid in the processing of the token, findtoken() returns a
*             couple of values via variables.  These variables are:
*
*                FindTokenStart   - An offset into the string where the first
*                                   @ character of the token is found.
*
*                FindTokenEnd     - An offset into the string where the last
*                                   @ character of the token is found.
*
*                FindTokenColor   - This used to be just a color value, now it
*                                   is used for a couple of purposes.  The
*                                   actual value is either the color value in
*                                   an @X macro (i.e. @X07), or a number value
*                                   following a colon (i.e. @FIRST:10@).
*
*                FindTokenAttr    - This value is used as a field attribute.
*                                   As an example, @FIRST:10R@, the letter 'R'
*                                   is the attribute, and it indicates that
*                                   the field should be right-aligned.  Other
*                                   values are L (left) and C (center).
*
*             In addition, the character at offset FindTokenStart is set to a
*             NULL-terminator by findtoken() so that everything up to that
*             point can just be printed.  (i.e. if "Hello @FIRST@" is
*             encountered, the string will contain "Hello "NULL"FIRST@" so
*             that "Hello " can be printed, followed by the translation of the
*             @FIRST@ macro).
*/

void LIBENTRY printxlated(char *Str) {
  int         Found;
  char        Temp[128];

  checkstack();
  while ((Found = findtoken(Str)) != 0) {
    StartToken = &Str[FindTokenStart];
    print(Str);
    *StartToken = '@';       /* restore the '@' in case we need it back */
    Str += FindTokenEnd;
    if (expandtoken(Found,Temp,TRUE))
      print(Temp);
  }
  print(Str);
}


/********************************************************************
*
*  Function:  printfoundtext()
*
*  Desc    :  displays found text in reverse video
*             calls printxlated() to print the string
*/

#ifndef LIB
void LIBENTRY printfoundtext(char *Str) {
  int  Ofs;
  int  Len;
  int  OldColor;
  char OldChar;

  checkstack();
  if (! Control.GraphicsMode) {
    printxlated(Str);
  } else {
    while (1) {
      if ((Ofs = searchfirst(Str,&Len)) == -1) {
        printxlated(Str);
        break;
      } else {
        {
          /* print everything up to the text that was found */
          OldChar = Str[Ofs];
          Str[Ofs] = 0;
          printxlated(Str);
          Str[Ofs] = OldChar;
        }
        {
          /* now print the text that was found while highlighting it */
          OldChar = Str[Ofs+Len];
          Str[Ofs+Len] = 0;
          OldColor = curcolor();
          printcolor((OldColor & 0xF0) != 0x70 ? 0x70 : 0x07);
          print(&Str[Ofs]);
          printcolor(OldColor);
          Str[Ofs+Len] = OldChar;
        }
        /* now skip over the text that was found, loop back to search for more */
        Str = &Str[Ofs+Len];
      }
    }
  }
}
#endif


void LIBENTRY xlatetext(char *New, char *Old) {
  int   Found;

  checkstack();
  *New = 0;
  while ((Found = findtoken(Old)) != 0) {
    StartToken = &Old[FindTokenStart];

    strcpy(New,Old);
    New += strlen(New);

    *StartToken = '@';       /* restore the '@' in case we need it back */
    Old += FindTokenEnd;

    if (expandtoken(Found,New,FALSE))
      New += strlen(New);
  }

  strcpy(New,Old);
}


int LIBENTRY strlenminuscolorcodes(char *Str) {
  int   Found;
  char  *Start;
  int   Len;

  checkstack();
  if ((Len = strlen(Str)) == 0)
    return(0);

  while ((Found = findtoken(Str)) != 0) {
    Start = &Str[FindTokenStart];
    *Start = '@';       /* restore the '@' in case we need it back */
    Str += FindTokenEnd;

    if (Found == XCOLORS)
      Len -= 4;
  }
  return(Len);
}
