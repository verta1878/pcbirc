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

#include <messages.h>
#include <account.h>

#ifdef DBASE
  #undef ___USE_VAR___
  #include <dbase.hpp>
#endif


#ifdef PCB152
extern countrytype SystemCountry;

void LIBENTRY creditcurrency(char *Str, double Value) {
  char        *p;
  char         Decimal[10];
  char         Buffer[40];
  countrytype  Save;

  if (PcbData.ShowCurrency) {
    Str[0] = 0;

    // we want to show the currency in the HOST format, NOT the format that
    // the caller's country uses, so save the country information, then
    // replace it with the system format.

    Save = Country;
    Country = SystemCountry;

    if (!(Country.CurrencyFmt & 0x01)) { // Does the symbol go in front?
      strcpy(Str,Country.DollarSym);
      // Do we need a space after it?
      if (Country.CurrencyFmt & 0x02)
        addchar(Str,' ');
    }

    sprintf(Buffer,"%20.*lf",Country.SigDigits,Value);
    stripleft(Buffer,' ');
    if ((p = strchr(Buffer,'.')) != NULL) {
      strcpy(Decimal,p);
      *p = 0;
      Decimal[0] = Country.FractionSep[0];
    } else {
      Decimal[0] = 0;
    }

    commastr(Buffer,(bool) (Value < 0));
    strcat(Buffer,Decimal);

    if (Country.CurrencyFmt & 0x01) {   // Does the symbol go in back?
      // Do we need a space before it?
      if (Country.CurrencyFmt & 0x02)
        addchar(Buffer,' ');
      strcat(Buffer,Country.DollarSym);
    }

    strcat(Str,Buffer);

    // restor the country information
    Country = Save;
  } else
    dcomma(Str,Value);
}


static bool _NEAR_ LIBENTRY todayispeak(void) {
  bool     static LastRetVal;
  unsigned static LastDateChecked;
  unsigned TodaysDate;
  int      DayOfWeek;
  char    *p;
  char    *q;
  char     Str[10];
  char     Today[10];
  DOSFILE  File;

  TodaysDate = getjuliandate();
  if (LastDateChecked == TodaysDate)
    return(LastRetVal);

  LastDateChecked = TodaysDate;
  LastRetVal      = FALSE;

  DayOfWeek = dayofweektoday() - 1;
  if (! isset(&PcbData.PeakDays,DayOfWeek))
    return(FALSE);

  if (PcbData.HolidaysFile[0] == 0 || fileexist(PcbData.HolidaysFile) == 255) {
    LastRetVal = TRUE;
    return(TRUE);
  }

  if (dosfopen(PcbData.HolidaysFile,OPEN_READ|OPEN_DENYNONE,&File) == -1) {
    LastRetVal = TRUE;
    return(TRUE);
  }

  datestr(Today);
  LastRetVal = TRUE;
  while (dosfgets(Str,sizeof(Str),&File) != -1) {
    if (Str[0] > 0) {
      for (p = Str, q = Today; p < &Str[8]; p++, q++) {
        // X is a wildcard and matches any number
        if (*p != 'X' && *p != *q)
          goto bottom;
      }
      // if we got this far, it's a match... NO PEAK TODAY ... get out now
      LastRetVal = FALSE;
      break;
    }
bottom:;
  }

  dosfclose(&File);
  return(LastRetVal);
}


bool LIBENTRY currentlyinpeak(void) {
  int CurrentMinute;
  int PeakStart;
  int PeakEnd;

  if (Status.ActStatus == ACT_DISABLED)
    return(FALSE);

  PeakStart     = strtominutes(PcbData.PeakStart);
  PeakEnd       = strtominutes(PcbData.PeakEnd);
  CurrentMinute = currentminute();

  if (PeakEnd > PeakStart) {
    // PeakStart/PeakEnd does not cross midnight
    if (CurrentMinute < PeakStart || CurrentMinute > PeakEnd)
      return(FALSE);
  } else {
    // PeakStart/PeakEnd does cross midnight - check to see that it is NOT
    // in between the peakstart and midnight AND that it is NOT in between
    // midnight and the peakend, if this is the case, return FALSE indicating
    // that it is NOT currently peak time
    if (CurrentMinute > PeakEnd && CurrentMinute < PeakStart)
      return(FALSE);
  }

  if (! todayispeak())
    return(FALSE);

  return(TRUE);
}


static int _NEAR_ LIBENTRY calcpeakused(int CurrentMinute) {
  int Minute;
  int PeakStart;
  int PeakEnd;
  int PeakUsed;

  PeakUsed = 0;

  if (Status.ActStatus != ACT_DISABLED) {
    PeakStart = strtominutes(PcbData.PeakStart);
    PeakEnd   = strtominutes(PcbData.PeakEnd);

    if (PeakStart != 0 && PeakEnd != 0 && todayispeak()) {
      if (PeakEnd < PeakStart)
        PeakEnd += 1440;

      for (Minute = Status.LogonMinute, PeakUsed = 0; Minute < CurrentMinute; Minute++)
        if (Minute >= PeakStart && Minute <= PeakEnd)
          PeakUsed++;
    }
  }

  return(PeakUsed);
}


void LIBENTRY chargetimeonline(int CurrentMinute, int MinutesOnline) {
  int PeakUsed;

  if (Status.ActStatus != ACT_DISABLED) {
    PeakUsed = calcpeakused(CurrentMinute);

    if (PeakUsed != 0) {
      MinutesOnline -= PeakUsed;
      recordusage("TIME ONLINE","PEAK",AccountRates.ChargeForPeakTime,PeakUsed,&UsersData.Account.DebitTime);
    }

    recordusage("TIME ONLINE","",AccountRates.ChargeForTime,MinutesOnline,&UsersData.Account.DebitTime);
  }
}


#define bigger(x,y) if (x < y) x = y;

void LIBENTRY calculatebalance(void) {
  unsigned Time;
  unsigned MinutesOnline;
  unsigned PeakMinutes;
  double   Temp;
  double   TempTimeCharge;

  if (AccountSupport && Status.ActStatus != ACT_DISABLED) {
    TempTimeCharge = 0;

    if (AccountRates.ChargeForTime != 0 || AccountRates.ChargeForPeakTime != 0) {
      Time = currentminute();
      if (Time < (unsigned) Status.LogonMinute)
        Time += 1440;

      MinutesOnline  = Time - Status.LogonMinute;
      PeakMinutes    = calcpeakused(Time);
      MinutesOnline -= PeakMinutes;

      // If the caller didn't have sufficient credits to use up one minute,
      // he'd be logged off.  But that prevents him from using up the REST of
      // his credits so PCBoard couldn't give him the drop-sec-level.  The
      // line below works around this by giving him one extra minute to use
      // up credits and, potentially, go negative ... but at *least* it will
      // run the account dry so that the drop-sec-level can be used
      if (MinutesOnline > 0)
        MinutesOnline--;
      else if (PeakMinutes > 0)
        PeakMinutes--;

      TempTimeCharge = AccountRates.ChargeForTime * MinutesOnline +
                       AccountRates.ChargeForPeakTime * PeakMinutes;
    }

    if (PcbData.Concurrent) {
      Temp  = UsersData.Account.DebitTime + TempTimeCharge;
      bigger(Temp,UsersData.Account.DebitCall);
      bigger(Temp,UsersData.Account.DebitGroupChat);
      bigger(Temp,UsersData.Account.DebitTPU);
      bigger(Temp,UsersData.Account.DebitSpecial);
      bigger(Temp,UsersData.Account.DebitMsgRead);
      bigger(Temp,UsersData.Account.DebitMsgReadCapture);
      bigger(Temp,UsersData.Account.DebitMsgWrite);
      bigger(Temp,UsersData.Account.DebitMsgWriteEchoed);
      bigger(Temp,UsersData.Account.DebitMsgWritePrivate);
      bigger(Temp,UsersData.Account.DebitDownloadFile);
      bigger(Temp,UsersData.Account.DebitDownloadBytes);
      Temp = Temp
           -  UsersData.Account.CreditUploadFile
           -  UsersData.Account.CreditUploadBytes
           -  UsersData.Account.CreditSpecial;
    } else {
      Temp =  UsersData.Account.DebitTime + TempTimeCharge
           +  UsersData.Account.DebitCall
           +  UsersData.Account.DebitMsgRead
           +  UsersData.Account.DebitMsgReadCapture
           +  UsersData.Account.DebitMsgWrite
           +  UsersData.Account.DebitMsgWriteEchoed
           +  UsersData.Account.DebitMsgWritePrivate
           +  UsersData.Account.DebitDownloadFile
           +  UsersData.Account.DebitDownloadBytes
           +  UsersData.Account.DebitGroupChat
           +  UsersData.Account.DebitTPU
           +  UsersData.Account.DebitSpecial
           -  UsersData.Account.CreditUploadFile
           -  UsersData.Account.CreditUploadBytes
           -  UsersData.Account.CreditSpecial;
    }
    Status.Balance = UsersData.Account.StartingBalance - Temp - Status.TempCharge;
  }
}


bool LIBENTRY insufficientcredits(double Charge, double TempCharge) {
  if (Charge <= 0)
    return(FALSE);

  Status.TempCharge = TempCharge;
  calculatebalance();

  if (Status.ActStatus == ACT_ENFORCE) {
    if (Charge > Status.Balance) {
      creditcurrency(Status.DisplayText,Charge);
      displaypcbtext(TXT_INSUFCREDITS,NEWLINE|LFBEFORE|LOGIT|BELL);
      Status.TempCharge = 0;
      calculatebalance();
      return(TRUE);
    }
  }

  Status.TempCharge = 0;
  calculatebalance();
  return(FALSE);
}


void LIBENTRY checkaccountbalance(void) {
  long   CurMinLeft;
  double ActMinLeft;
  double PerMinCharge;
  double Save;

  if (Status.ActStatus == ACT_ENFORCE) {
    Save = Status.Balance;
    calculatebalance();

    // Scenario:  Caller has 50 credits left.  He logs on and is charged
    //            50 credits for the logon.  Now he has 0 credits left, but
    //            he *is* logged on.
    //
    // Problem:   The routines below will then reduce his access even if he
    //            NEVER does anything further to spend credits.  For example,
    //            if the caller reads messages (but is NOT charged for
    //            reading messages), afterwards this function will be called
    //            and it will see that he has 0 credits left and it will
    //            adjust his security level even though NO CREDIT SPENDING ACT
    //            was performed by the caller.
    //
    // Solution:  To work around this, we saved the current balance up above,
    //            the we calculate the current balance.  If both balances are
    //            the same, then it AVOIDS changing the caller's security.

    if (Status.Balance != Save) {
      if (Status.Balance <= 0) {
        if (Status.CurSecLevel != UsersData.Account.DropSecLevel && ! PcbData.IgnoreDropSecLevel) {
          {
            // since we ran out of credits, the credits *might* have been used
            // up by the time spent online.  If so, we need to record the
            // time spent *now* because if we wait until the caller is logged
            // off it'll be too late because Status.ActStatus will have been
            // changed to DISABLE the accounting features!
            unsigned    Time;

            Time = currentminute();
            if (Time < (unsigned) Status.LogonMinute)
              Time += 1440;

            chargetimeonline(Time,Time - Status.LogonMinute);
          }

          Status.CurSecLevel = UsersData.Account.DropSecLevel;
          checkdisplaystatus();
          displaypcbtext(TXT_CREDITEXCEEDED,NEWLINE|LFBEFORE|LOGIT|BELL);
          displaypcbtext(TXT_SECURITYCHANGED,NEWLINE|LOGIT);

          // updat the user information (this may change the security level)
          updateuserinformation();

          // also, the account status may have changed, if it is no longer
          // being enforced, then jump out now
          if (Status.ActStatus != ACT_ENFORCE)
            return;
        }
      }
    }

    PerMinCharge = (currentlyinpeak() ? AccountRates.ChargeForPeakTime : AccountRates.ChargeForTime) + Status.CurConf.ChargeTime;
    if (PerMinCharge > 0) {
      ActMinLeft = Status.Balance / PerMinCharge;
      CurMinLeft = minutesleft();
      if (CurMinLeft > ActMinLeft)
        subtime(((long) (CurMinLeft-ActMinLeft)) * 60,ACCTTIME);
    }
  }
}


long LIBENTRY minutesused(long StartTime) {
  long TimeSpent;
  long Minutes;

  if (Status.ActStatus == ACT_DISABLED)
    return(0);

  TimeSpent  = doselapsedtime(StartTime); // hundredths of a second
  TimeSpent /= (30 * 100);      /* half-minute intervals */
  Minutes    = TimeSpent / 2;   /* minutes */

  /* now round up to a minute if 30 seconds of the next minute was used */
  if (TimeSpent > Minutes * 2)
    Minutes++;

  return(Minutes);
}


#ifdef DBASE
static char * info [] = {
  "Date,D,8,0",
  "Time,C,5,0",
  "Name,C,25,0",
  "NodeNumber,N,5,0",
  "ConfNumber,N,5,0",
  "Activity,C,15,0",
  "SubAct,C,25,0",
  "UnitCost,N,14,4",
  "Quantity,N,9",
  "Value,N,14,4",
  NULL,
};


static void _NEAR_ LIBENTRY recordusageindbf(char *Activity, char *SubActivity, double UnitCost, long Quantity, double Value, char *Date, char *Time) {
  cDBF dbobj;
  char DDate[9];

  DDate[0] = '1';
  DDate[1] = '9';
  DDate[2] = Date[6];
  DDate[3] = Date[7];
  DDate[4] = Date[0];
  DDate[5] = Date[1];
  DDate[6] = Date[3];
  DDate[7] = Date[4];
  DDate[8] = 0;

  if (fileexist(PcbData.AccountTrack) == 255)
    dbobj.dbfCreate(PcbData.AccountTrack,FALSE,info);
  else
    dbobj.dbfOpen(PcbData.AccountTrack,FALSE);

  if (dbobj.inUse) {
    dbobj.start();
    dbobj.put("Date",DDate);
    dbobj.put("Time",Time);
    dbobj.put("Name",UsersData.Name);
    dbobj.put("NodeNumber",PcbData.NodeNum);
    dbobj.put("ConfNumber",(long) Status.Conference);
    dbobj.put("Activity",Activity);
    dbobj.put("SubAct",SubActivity);
    dbobj.put("UnitCost",UnitCost);
    dbobj.put("Quantity",Quantity);
    dbobj.put("Value",Value);
    dbobj.add();
    dbobj.dbfClose();
  }
}
#endif


static void _NEAR_ LIBENTRY recordusageinascii(char *Activity, char *SubActivity, double UnitCost, long Quantity, double Value, char *Date, char *Time) {
  int     X;
  DOSFILE File;
  char    Str[256];

  sprintf(Str,"%s %s %-25.25s %5u %5u %-15.15s %-25.25s %14.4lf %9ld %14.4lf",
          Date,
          Time,
          UsersData.Name,
          PcbData.NodeNum,
          Status.Conference,
          Activity,
          SubActivity,
          UnitCost,
          Quantity,
          Value);

  stripright(Str,' ');
  strcat(Str,"\r\n");

  // give it up to 10 tries to get the file written out to disk

  for (X = 0; X < 10; X++) {
    if (dosfopen(PcbData.AccountTrack,OPEN_RDWR|OPEN_DENYWRIT|OPEN_APPEND,&File) != -1) {
      dosfputs(Str,&File);   //lint !e534
      dosfclose(&File);
      return;
    }
  }
}


double LIBENTRY recordusage(char *Activity, char *SubActivity, double UnitCost, long Quantity, double *Storage) {
  char    Date[9];
  char    Time[6];
  double  Value;

  if (Status.ActStatus == ACT_DISABLED)
    return(0);

  Value = UnitCost * Quantity;

  if (Value == 0 && Status.ActStatus == ACT_ENFORCE)
    return(0);

  *Storage += Value;

  if (PcbData.AccountTrack[0] == 0)
    return(Value);

  if (Asy.Online == LOCAL && PcbData.ExcludeLocals)
    return(Value);

  timestr2(Time);
  datestr(Date);

  #ifdef DBASE
  if (strstr(PcbData.AccountTrack,".DBF") != NULL)
    recordusageindbf(Activity,SubActivity,UnitCost,Quantity,Value,Date,Time);
  else
  #endif
    recordusageinascii(Activity,SubActivity,UnitCost,Quantity,Value,Date,Time);

  return(Value);
}


void LIBENTRY chargeformessage(char MsgStatus, bool Echo, char *To) {
  char   *Activity;
  double *Storage;
  double  Rate;

  Rate = Status.CurConf.ChargeMsgWrite;

  if (MsgStatus == MSG_RCVR || MsgStatus == MSG_CMNT) {
    Rate += AccountRates.ChargeForMsgWritePrivate;
    Storage = &UsersData.Account.DebitMsgWritePrivate;
    Activity = "MSG WRITE PRIV";
  } else if (Echo) {
    Rate += AccountRates.ChargeForMsgWriteEchoed;
    Storage = &UsersData.Account.DebitMsgWriteEchoed;
    Activity = "MSG WRITE ECHO";
  } else {
    Rate += AccountRates.ChargeForMsgWrite;
    Storage = &UsersData.Account.DebitMsgWrite;
    Activity = "MSG WRITE";
  }

  if (Rate != 0) {
    recordusage(Activity,To,Rate,1,Storage);
    checkaccountbalance();
  }
}


void LIBENTRY chargetimeinconf(void) {
  long MinutesUsed;

  if (Status.ActStatus != ACT_DISABLED) {
    MinutesUsed = minutesused(Status.ConfJoinTime);
    if (MinutesUsed != 0) {
      recordusage("CONF TIME","",Status.CurConf.ChargeTime,MinutesUsed,&UsersData.Account.DebitTime);
      checkaccountbalance();
    }
  }
  Status.ConfJoinTime = dosgetlongtime();
}
#endif  /* ifdef PCB152 */
