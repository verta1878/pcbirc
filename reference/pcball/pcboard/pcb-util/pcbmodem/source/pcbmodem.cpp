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


// (c) 1995  Clark Development Company, Inc.  All Rights Reserved.
// Revision:  2.00
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// BC includes
#define gotoxy bor_gotoxy
#define wherex bor_wherex
#define wherey bor_wherey
#include <conio.h>
#undef gotoxy
#undef wherex
#undef wherey

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <dir.h>
#include <string.h>
#include <errno.h>


// PCBoard includes
#include <system.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <help.h>
#include <pcbtools.h>

// Local includes
#include "common.h"
#include "modems.h"

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
#define   NumEKeys      27
#define   NumOfOptions  11

DOSFILE   DataFile;

static DOSFILE  Outfile,
                Infile;

static  char  initstr     [ 20 ],
              PortStr     [  7 ],
              NodeFile    [ 40 ],
              Temp        [ 80 ],
              Base        [  7 ],
              Baud3       [  7 ],
              PCBDat_File [ 40 ],
              PCBOld_File [ 40 ];

char   TempStr       [ 120 ],
       MFile         [  60 ],
       ExitKeyNum    [NumEKeys];

char   Mans  [190 ][ 15 ],
       Names [ 54 ][ 25 ];

static char ComPort,
            CommDrv,
            IrqLocal,
            LocalComPort,
            BaseLocal;

static  char  mask_hex [] = {9, 0, '0', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

static  int  k,
             Irq,
             IntPort,
             Help,
             count,
             count2;

int    ManCount,
       DefaultNum,
       NumExitKeys = NumEKeys,
       ExitKeyFlag [ NumEKeys ];

static unsigned BASE;

unsigned  CurrentModem,
          y;

static  long  BaudRate;

long    Offset,
        ModemdataOffset;

static  bool  Fax,
              NoUpdate,
              V42,
              OK,
              Dat_File,
              SetPCB;

bool  local;

char VerifyStr[10] = { VERIFYSTR };

typedef struct {
   char *Word;
} wordtype;

wordtype static Options[NumOfOptions] = {
    {"/MODEM:"}, {"/PORT:"}, {"/BASE:"}, {"/IRQ:"}, {"/V42"},
    {"/FAX"},    {"/BAUD:"}, {"/FILE:"}, {"/MP:"},  {"/NOUPDATE"},
    {"/NODEBAT:"}
};

enum {P_MODEM,P_PORT,P_BASE,P_IRQ,P_V42,P_FAX,P_BAUD,P_FILE,P_MP,P_NO,P_NB};

headertype     Header;
modemdatatype  Modemdata;
manufdatatype  * Manufdata;

char    Head1 [] = "PCBModem";
char    Head2 [] = "Copyright (C) 1995  Clark Development Company, Inc.";

static void  dsp_prms(void);
static void  newmodeminfo(void);
static void  writepcbline(void);
static void  pascal merror(int Key);
static void  pascal show(char *Str2);
static void  pcboard(void);
static void  dosversion(void);
static void  commandline(void);
static void  modify(void);
static int   getinfo(void);
static int   printinfo(void);
static int   writeinfo(void);
static int   pascal option(wordtype *CmdOptions, char *CmdStr, int NumOptions);
static bool  pascal checktoken(char *Pointer);

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int main(int argc, char **argv)
{
  Manufdata = (manufdatatype *) malloc(sizeof(manufdatatype) * MaxModem);
  clrscr();
  printf("Initializing --- Please wait...");
  OK = TRUE;
  initprogram();

  if (dosfopen(MFile, READ, &DataFile) == -1)
    quit(13,"main");

  createarray(TRUE);
  title();

  SetPCB = Help = Dat_File = 0;
  strcpy(PCBDat_File, "");

  if (argc == 2 || argc == 3)
  {
   for (k=1; k < argc; k++)
    {
     strupr(argv[k]);
     if (memcmp("/NODEBAT:",argv[k],9) == 0)
      {
       memcpy(NodeFile, argv[k]+9, strlen(argv[k])-9);
       SetPCB=1;
       Help = 2;
      }
     if (memcmp("/FILE:",argv[k],6) == 0)
      {
       memcpy(PCBDat_File, argv[k]+6, strlen(argv[k])-17);
       Dat_File = 1;
       Help = 2;
      }
     if (memcmp("/?",argv[k],2) == 0)
      {
       Help = 1;
       dsp_prms();
       anykey();
       quit(0,"");
      }
    }
  }

  if (argc > 1 && Help != 2) {
    local = 1;
    CommDrv = 'A';
    for (count=1; count<argc; count++) {
      strupr(argv[count]);
      switch(option(Options, argv[count], NumOfOptions)) {
        case P_MODEM    :  CurrentModem = atoi(argv[count]+7);
                           break;
        case P_PORT     :  LocalComPort = atoi(argv[count]+6);
                           break;
        case P_BASE     :  BaseLocal = 1;
                           memcpy(Base, argv[count]+6, 4);
                           break;
        case P_IRQ      :  IrqLocal = 1;
                           Irq = atoi(argv[count]+5);
                           break;
        case P_V42      :  V42 = 1;
                           break;
        case P_FAX      :  Fax = 1;
                           break;
        case P_BAUD     :  BaudRate = atol(&argv[count][1]+5);
                           break;
        case P_FILE     :  Dat_File = 1;
                           memcpy(PCBDat_File, argv[count]+6, strlen(argv[count])-17);
                           break;
        case P_MP       :  memcpy(TempStr, argv[count]+4, strlen(argv[count])-4);
                           CommDrv = TempStr[0];
                           break;
        case P_NO       :  NoUpdate = 1;
                           break;
        case P_NB       :  memcpy(NodeFile, argv[count]+9, strlen(argv[count])-9);
                           SetPCB=1;
                           break;
      }
    }

    if (!LocalComPort || !CurrentModem) {
      dsp_prms();
      goto EndPCBModem;
    }

    if (!IrqLocal && !BaseLocal && (LocalComPort>2 && CommDrv=='A')) {
      dsp_prms();
      goto EndPCBModem;

    }

    qsort(Manufdata, Header.NumOfModems, sizeof(manufdatatype), sortmodemnum);

    for (y=0; y<Header.NumOfModems; y++) {
      if (Manufdata[y].Number == CurrentModem)
        goto FoundModem;
    }

    gotoxy(3,5);
    printf("\nModem is not found in the list.\n");
    goto EndPCBModem;

FoundModem:
    CurrentModem = y;
    getmdmdata(FALSE);
    commandline();
    if (printinfo() != -1)
      if (writeinfo() != -1)
        if (!NoUpdate)
          modify();

EndPCBModem:
    setcursor(CUR_NORMAL);
    free(Manufdata);
    dosfclose(&DataFile);
    exit(EXIT_FAILURE);
  }

NoCmdLine:
  if ((k = selectmodem()) == 0)
    newmodeminfo();

  if (k < 1)
    quit(0,"");

  if (getinfo() == -1)
    goto NoCmdLine;

  if (printinfo() == -1)
    goto NoCmdLine;

  if (writeinfo() != -1)
    pcboard();

  goto NoCmdLine;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void pascal show(char *Str2)
{
  // Delay used for timing purpose.

  delay(30);
  cputs(Str2);     //lint !e534
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void dosversion(void)
{
  static char Major;

  asm {
    mov ah, 30h
    int 21h
    mov Major, al
  }

  if (Major >= 20)
    PcbData.OS2Driver = 1;
  else
    PcbData.OS2Driver = 0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void pascal merror(int Key)
{
  closemodem(TRUE);
  boxcls(8, 17, 70, 23, Colors[MENUBOX], DOUBLE);

  if (Key == 2) {
    fastcenter(19, "Initialization Error...Invalid intelligent port", Colors[MENUSELECT]);
    fastcenter(20, "or missing intelligent port driver.", Colors[MENUSELECT]);
  } else if (Key == 3) {
    fastcenter(19, "Initialization Error...No modem present", Colors[MENUSELECT]);
    fastcenter(20, "at specified location or bad UART.", Colors[MENUSELECT]);
  } else {
    fastcenter(19, "Initialization Error...Modem is not responding", Colors[MENUSELECT]);
    fastcenter(20, "Please Check Your Configuration.", Colors[MENUSELECT]);
  }

  anykey();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void  newmodeminfo(void)
{
  title();
  fastprint(6, 5, "If you find that your modem is not listed among those found here in", Colors[MENUSELECT]);
  fastprint(6, 6, "PCBModem, please call our support BBS at (801) 261-8976 and inquire", Colors[MENUSELECT]);
  fastprint(6, 7, "about how to obtain the latest release of PCBModem.", Colors[MENUSELECT]);
  fastprint(6, 9, "To have support for your modem added to PCBModem you may either", Colors[MENUSELECT]);
  fastprint(6, 10, "FAX us a copy of your AT Commands (quick reference) and S-Registers", Colors[MENUSELECT]);
  fastprint(6, 11, "or send us a photocopy via U.S. Mail.", Colors[MENUSELECT]);
  fastprint(6, 13, "FAX:", Colors[EDIT]);
  fastprint(24, 13, "(801) 261-8987", Colors[QUESTION]);
  fastprint(6, 15, "U.S. Mail:", Colors[EDIT]);
  fastprint(24, 15, "Clark Development Company, Inc.", Colors[QUESTION]);
  fastprint(24, 16, "3950 South 700 East, Suite 303", Colors[QUESTION]);
  fastprint(24, 17, "Murray, Utah  84107", Colors[QUESTION]);
  anykey();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static int  getinfo(void)
{
  title();
  BaudRate = Modemdata.Baud;
  y = 5;
  if (BaudRate > 9600L) {
    setcursor(CUR_BLANK);
    boxcls(5, 4, 73, 9, Colors[MENUBOX], DOUBLE);
    fastprint(7, 5, "Note:  Some multitaskers or networks will not work correctly with", Colors[MENUSELECT]);
    fastprint(7, 6, "the port opening speed greater than 9600.  If you experience", Colors[MENUSELECT]);
    fastprint(7, 7, "problems with data loss or CRC errors (when transferring files),", Colors[MENUSELECT]);
    fastprint(7, 8, "lower your port opening speed to 9600 baud.", Colors[MENUSELECT]);
    y = 11;
  }

  do {
    BaudRate = Modemdata.Baud;

    if (BaudRate > 57600L)
      BaudRate = 57600L;

    sprintf(TempStr, "Enter the port opening speed (%lu max)", Modemdata.Baud);
    inputnum(5, y, 6, TempStr, &BaudRate, vLONG, PCBMODEMHLP+2);

    if (KeyFlags == ESC)
      return(-1);
  } while (BaudRate > Modemdata.Baud || (BaudRate != 300L && BaudRate != 1200L &&
    BaudRate != 2400L && BaudRate != 4800L && BaudRate != 9600L &&
    BaudRate != 19200L && BaudRate != 38400L && BaudRate != 57600L &&
    BaudRate != 115200L));

  Modemdata.Baud = BaudRate;
  title();
  CommDrv = 'A';
  y = 6;
  boxcls(3, 4, 76, 7, Colors[MENUBOX], SINGLE);
  fastprint(5, 5, "A = UART based com ports", Colors[MENUSELECT]);
  fastprint(5, 6, "F = Fossil driven (X00, BNU, G00)", Colors[MENUSELECT]);
  fastprint(45, 5, "C = COMMDRV (Digiboard, Arnet)", Colors[MENUSELECT]);
  fastprint(45, 6, "O = OS/2 (SIO, OS/2 drivers)", Colors[MENUSELECT]);
  do {
    inputnum(5, y+2, 1, "Communications driver to use (A, F, C, O)", &CommDrv, vCHAR, PCBMODEMHLP+3);

  if (KeyFlags == ESC)
    return(-1);

  } while (CommDrv != 'A' && CommDrv != 'F' && CommDrv != 'C' && CommDrv != 'O');

  if (BaudRate > 38400L && (CommDrv == 'F' || CommDrv =='C'))
    Modemdata.Baud = 38400L;

  if (CommDrv != 'A') {
    IntPort = 1;
    inputnum(5, y+4, 2, "What port number will you be using", &IntPort, vINT, PCBMODEMHLP+4);

    if (KeyFlags == ESC)
      return(-1);

  } else {
    ComPort = '1';
    inputnum(5, y+4, 1, "What COM port number will you be using", &ComPort, vCHAR, PCBMODEMHLP+5);

    if (KeyFlags == ESC)
      return(-1);

    if (ComPort != '1' && ComPort != '2')
    {
      switch(ComPort)
      {
        case '4' : Irq = 3;
                   strcpy(Base, "2E8");
                   break;
        default  : Irq = 4;
                   strcpy(Base, "3E8");
                   break;
      }
      inputnum(51, y+4, 2, "IRQ", &Irq, vINT, PCBMODEMHLP+6);

      if (KeyFlags == ESC)
        return(-1);

      inputstr(62, y+4, 4, "BASE", Base, Base, mask_hex, INPUT_CAPS, PCBMODEMHLP+7);
      stripright(Base,' ');

      if (KeyFlags == ESC)
        return(-1);
    }
  }

  if (Modemdata.V42)
  {
    inputnum(5, y+6, 1, "Would you like to enable V.42bis (Y/N)", &Modemdata.V42, vBOOL, PCBMODEMHLP+8);
    if (KeyFlags == ESC)
      return(-1);
  }

  if (Modemdata.CallID)
  {
    inputnum(5, y+6, 1, "Would you like to enable caller ID (Y/N)", &Modemdata.Fax, vBOOL, PCBMODEMHLP+8);
    if (KeyFlags == ESC)
      return(-1);
  }

  if (!Modemdata.Lock && Modemdata.Baud <= 9600L)
  {
    inputnum(5, y+6, 1, "Do you want to Lock the Opening Baud Rate (Y/N)", &Modemdata.Lock, vBOOL, PCBMODEMHLP+11);
    if (KeyFlags == ESC)
      return(-1);
  }

  if (Modemdata.Fax)
  {
    inputnum(5, y+6, 1, "Would you like to enable adaptive fax answering (Y/N)", &Modemdata.Fax, vBOOL, PCBMODEMHLP+8);
    if (KeyFlags == ESC)
      return(-1);
  }

  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static int  printinfo(void)
{
  if (!Modemdata.Eprom)
    return 0;

  title();
  sprintf(TempStr, "Modem Selected:  %u", Manufdata[CurrentModem].Number);
  fastprint(3, 4, TempStr, Colors[MENUBOX]);
  sprintf(TempStr,"Description:  %s %s", Manufdata[CurrentModem].Manufs, Manufdata[CurrentModem].Name);
  fastprint(6, 5, TempStr, Colors[MENUSELECT]);

  if (CommDrv != 'A') {
    sprintf(TempStr, "Port Selected:  PORT%i  (", IntPort);
    if (CommDrv == 'F')
      strcat(TempStr,"Fossil)");
    else if (CommDrv == 'C')
      strcat(TempStr,"Commdrv)");
    else
      strcat(TempStr,"OS/2)");
  } else {
    sprintf(TempStr, "Port Selected:  COM%c", ComPort);
  }

  fastprint(4, 6, TempStr, Colors[MENUSELECT]);

  if (CommDrv == 'A') {
    if (ComPort != '1' && ComPort != '2') {
      sprintf(TempStr, "IRQ: %-2d    BASE: %-4s", Irq, Base);
      fastprint(27, 6, TempStr, Colors[MENUSELECT]);
    }
  }

  sprintf(TempStr, "Opening Speed:  %ld", BaudRate);
  fastprint(4, 7, TempStr, Colors[MENUSELECT]);

  if (Modemdata.Factory)
    sprintf(TempStr, "Using Factory:  &F%c", Modemdata.Factory+48);
  else
    sprintf(TempStr, "Using Factory:  &F");

  fastprint(4, 8, TempStr, Colors[MENUSELECT]);
  fastprint(9, 10, "Comments:", 14);
  fastprint(20, 10, Modemdata.Cmnt1, Colors[MENUSELECT]);
  fastprint(20, 11, Modemdata.Cmnt2, Colors[MENUSELECT]);
  fastprint(20, 12, Modemdata.Cmnt3, Colors[MENUSELECT]);
  fastprint(4, 14, "NRAM Settings:", 14);
  fastprint(20, 14, Modemdata.Send1, Colors[MENUSELECT]);
  fastprint(20, 15, Modemdata.Send2, Colors[MENUSELECT]);
  fastprint(20, 16, Modemdata.Send3, Colors[MENUSELECT]);

  if (Modemdata.Eprom && !local) {
    inputnum(3, 22, 1, "Do you wish to proceed with the initialization (Y/N)", &OK, vBOOL, PCBMODEMHLP+9);
    if (!OK || KeyFlags == ESC)
      return -1;
  }
  return 1;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static int writeinfo(void)
{
  window(4, 18, 78, 23);
  boxcls(2, 18, 78, 23, 0, BLANK);
  textcolor(LIGHTGRAY);

  switch (CommDrv) {
    case 'A' : sprintf(PortStr, "COM%c", ComPort);
               if (ComPort != '1' && ComPort != '2') {
                 BASE = hextoint(Base);
               } else {
                 if (ComPort == '1') {
                   Irq = 4;
                   BASE = 0x3F8;
                 } else {
                   Irq = 3;
                   BASE = 0x2F8;
                 }
               }
               break;
    case 'C' : sprintf(PortStr, "PORT%d", IntPort);
               break;
    case 'F' : sprintf(PortStr, "PORT%dF", IntPort);
               break;
    case 'O' : sprintf(PortStr, "PORT%d", IntPort);
               dosversion();
               break;
  }

openport:
 sprintf(TempStr,"%ld  ",BaudRate);
 fastprint(20,7,TempStr,Colors[MENUSELECT]);

 if (CommDrv != 'A') {
   if (initmodem(PortStr, IntPort, 0, 0, BaudRate, show) == -1) {
     merror(2);
     return(-1);
   }
  } else {
    sprintf(PortStr, "COM%c", ComPort);
    if (initmodem(PortStr, ComPort, Irq, BASE, BaudRate, show) == -1) {
      merror(3);
      return(-1);
    }
  }

  if (Modemdata.Factory)
    sprintf(initstr, "AT&F%cE1\r", Modemdata.Factory+48);
  else
    sprintf(initstr, "AT&FE1\r");

  if (modemcommand(initstr, WAITFOROKAY) == -1) {
    closemodem(TRUE);
    if (BaudRate <= 38400L)
      BaudRate = BaudRate / 2L;
    else
      BaudRate = 38400L;
    if (BaudRate < 1200L) {
      merror(1);
      return(-1);
    }
    goto openport;
  }

  strcpy(TempStr, Modemdata.Send1);
  strcat(TempStr, "\r");

  if (modemcommand(TempStr, WAITFOROKAY) == -1) {
    merror(1);
    return(-1);
  }

  if ((strcmp(Modemdata.Send2, "")) != 0) {
    strcpy(TempStr, Modemdata.Send2);
    strcat(TempStr, "\r");
    if (modemcommand(TempStr, WAITFOROKAY) == -1) {
      merror(1);
      return(-1);
    }
  }

  if ((strcmp(Modemdata.Send3, "")) != 0) {
    strcpy(TempStr, Modemdata.Send3);
    strcat(TempStr, "\r");
    if (modemcommand(TempStr, WAITFOROKAY) == -1) {
      merror(1);
      return(-1);
    }
  }

  strcpy(TempStr, "AT");
  strcat(TempStr, (Modemdata.Lock) ? Modemdata.Hspd : Modemdata.Lspd);
  strcat(TempStr, (Modemdata.V42)  ? "&A3&K3" : "");
  if (Manufdata[CurrentModem].Number == 178)
    strcat(TempStr, "\r");
  else
    strcat(TempStr, "&W\r");

  if (modemcommand(TempStr, WAITFOROKAY) == -1) {
    merror(1);
    return(-1);
  }

  closemodem(TRUE);

  if (!local) {
    setcursor(CUR_BLANK);
    boxcls(8, 20, 70, 23, 2, DOUBLE);
    fastcenter(21, "Modem Successfully Initialized", Colors[MENUSELECT]);
    anykey();
  }

  return(0);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static  void  pcboard(void)
{
  if (CommDrv != 'A') {
    sprintf(PortStr, "PORT%i", IntPort);
    if (CommDrv == 'F')
      strcat(PortStr, "F");
  } else {
    sprintf(PortStr, "COM%c", ComPort);
  }

  if (SetPCB)
    writepcbline();

  title();
  fastprint(2, 4, "Your PCBOARD.DAT file must now be modified to reflect the modem selected.", Colors[MENUSELECT]);
  fastprint(2, 5, "The items below are the proper settings which should be used:", Colors[MENUSELECT]);
  fastprint(9, 7, "Modem Selected:", 14);
  sprintf(TempStr, "#%u (%s %s)",
          Manufdata[CurrentModem].Number,
          Manufdata[CurrentModem].Manufs,
          Manufdata[CurrentModem].Name);
  fastprint(26, 7, TempStr, 13);
  fastprint(8, 8, "Seconds to wait:", 14);
  sprintf(TempStr, "%d", Modemdata.Seconds);
  fastprint(26, 8, TempStr, Colors[MENUSELECT]);
  fastprint(10, 9, "Port Selected:", 14);
  fastprint(26, 9, PortStr, Colors[MENUSELECT]);

  if (ComPort != '1' && ComPort != '2' && CommDrv == 'A') {
    sprintf(TempStr,"IRQ: %-2d  BASE: %-4s",Irq, Base);
    fastprint(30, 9, TempStr, Colors[MENUSELECT]);
  }

  fastprint(5, 10, "Port Opening Speed:", 14);
  sprintf(TempStr, "%ld", BaudRate);
  fastprint(26, 10, TempStr, Colors[MENUSELECT]);
  fastprint(9, 12, "Init String #1:", 14);
  sprintf(TempStr, "%s", Modemdata.Init1);
  fastprint(26, 12, TempStr, Colors[MENUSELECT]);

  if (Modemdata.Fax) {
    fastprint(9, 13, "Init String #2:", 14);
    sprintf(TempStr, "%s", Modemdata.Init2);
    fastprint(26, 13, TempStr, Colors[MENUSELECT]);
  }

  fastprint(6, 14, "'Off-Hook' String:", 14);
  sprintf(TempStr, "%s", Modemdata.Offhook);
  fastprint(26, 14, TempStr, Colors[MENUSELECT]);
  fastprint(15, 15, "Comments:", Colors[MENUSELECT]);
  fastprint(26, 15, Modemdata.Cmnt1, 13);
  fastprint(26, 16, Modemdata.Cmnt2, 13);
  fastprint(26, 17, Modemdata.Cmnt3, 13);
  fastprint(2, 19, "Would you like us to reflect the above changes in the PCBOARD.DAT?  If so,", Colors[MENUSELECT]);
  fastprint(2, 20, "we will rename your current PCBOARD.DAT file as PCBDAT.OLD followed by", Colors[MENUSELECT]);
  fastprint(2, 21, "creating a new PCBOARD.DAT file.", Colors[MENUSELECT]);
  inputnum(2, 23, 1, "Do you wish to create the new file (Y/N)", &OK, vBOOL, PCBMODEMHLP+10);

  if (KeyFlags == ESC || !OK)
    return;

  modify();
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void  writepcbline(void)
{
  static char   TokenStr[255];
  static char * Pointer;
  static int    NumTokens;
  static char   tFile[66];

  dosfclose(&Infile);
  dosfclose(&Outfile);
  strcpy(TokenStr,NodeFile);

  Pointer = strrchr(TokenStr,'\\');
  if (Pointer != NULL)
  {
   *(Pointer+1) = 0;
   strcpy(tFile,TokenStr);
   strcat(tFile,"tmp.fil");
  } else
   strcpy(tFile,"tmp.fil");

  if (remove(tFile) != 0)
    if (errno == EACCES)
      quit(17,"writepcbline-1a");

  if (rename(NodeFile, tFile) == -1)
    quit(15,"writepcbline-1b");

  if (dosfopen(tFile, OPEN_READ|OPEN_DENYNONE, &Infile) == -1)
    if (rename(tFile, NodeFile) == -1)
      quit(15,"writepcbline-2");

  if (dosfopen(NodeFile, OPEN_WRIT|OPEN_CREATE|OPEN_DENYNONE, &Outfile) == -1)
    quit(16,"writepcbline-3");

  while (dosfgets(Temp, sizeof(Temp), &Infile) != -1) {
    strupr(Temp);
    if (strncmp(Temp, "SET PCB=", 8) != 0) {
      dosfputs(Temp, &Outfile);    //lint !e534
    } else {
      dosfputs("SET PCB=", &Outfile);     //lint !e534
      strcpy(TokenStr, Temp+8);

      for (NumTokens = tokenizestr(TokenStr); NumTokens > 0; NumTokens--) {
        Pointer = getnexttoken();
        if (!checktoken(Pointer)) {
          dosfputs(Pointer, &Outfile);     //lint !e534
          dosfputs(" ", &Outfile);         //lint !e534
        }
      }

      sprintf(Temp, "/BAUD:%ld ", BaudRate);
      dosfputs(Temp, &Outfile);         //lint !e534
      sprintf(Temp,"/%s: ", PortStr);
      dosfputs(Temp, &Outfile);         //lint !e534

      if (CommDrv != 'A') {
        if (PcbData.OS2Driver && CommDrv == 'O')
          dosfputs("/OS ", &Outfile);   //lint !e534
      } else {
        if (ComPort != '1' && ComPort != '2') {
          sprintf(Temp, "/BASE:%s ", Base);
          dosfputs(Temp, &Outfile);     //lint !e534
          sprintf(Temp, "/IRQ:%d ", Irq);
          dosfputs(Temp, &Outfile);     //lint !e534
        }
      }
    }
    dosfputs("\r\n", &Outfile);   //lint !e534
   }
   dosfclose(&Infile);
   dosfclose(&Outfile);
   if (Dat_File) {
     modify();
     remove("tmp.fil");          //lint !e534
     remove("pcbdat.old");       //lint !e534
   }
   quit(0,"");
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static bool pascal checktoken(char *Pointer)
{
  if (memcmp(Pointer, "/OS", 3) == 0 ||
      memcmp(Pointer, "/COM", 4) == 0  ||
      memcmp(Pointer, "/IRQ", 4) == 0  ||
      memcmp(Pointer, "/PORT", 5) == 0 ||
      memcmp(Pointer, "/BASE", 5) == 0 ||
      memcmp(Pointer, "/BAUD", 5) == 0)
    return(TRUE);
  return(FALSE);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void  modify(void)
{
  static int    z;
  static char * s;

  dosfclose(&Infile);
  dosfclose(&Outfile);

  if (Dat_File == 0)
  {
    s = searchpath("PCBOARD.DAT");
    if (s != NULL) memcpy(PCBDat_File, s, strlen(s)-11);
    else {
      s = getenv("PCBDAT");
      if (s != NULL) memcpy(PCBDat_File, s, strlen(s)-11);
      else PCBDat_File[0] = 0;
    }
  }
  strcat(PCBDat_File, "PCBOARD.DAT");
  strcat(PCBOld_File, "PCBDAT.OLD");

  if (dosfopen(PCBDat_File, OPEN_READ|OPEN_DENYNONE, &Infile) == -1)
     quit(14,"modify-1");  // Unable to open PCBOARD.DAT

  count = 0;
  while ((dosfgets(TempStr, sizeof(TempStr), &Infile)) != -1)
     count++;
  dosfclose(&Infile);

  if (copyfile(PCBDat_File, PCBOld_File, 2) == -1)
        quit(14,"modify-2");  // Unable to COPY PCBOARD.DAT -> PCBDAT.OLD

  if ((dosfopen(PCBOld_File, OPEN_READ|OPEN_DENYNONE, &Infile)) == -1)
        quit(14,"modify-3");  // Unable to read in PCBDAT.OLD

  if ((dosfopen(PCBDat_File, OPEN_WRIT|OPEN_CREATE|OPEN_DENYNONE, &Outfile)) == -1)
        quit(14,"modify-4");  // Unable to create new PCBOARD.DAT

  for (count2 = 1; count2 <= count; count2++) {
     if (dosfgets(TempStr, sizeof(TempStr), &Infile) == -1)
        break;

        switch(count2) {
          /* Seconds to Wait for Carrier     */
          case 51  :  sprintf(TempStr, "%d", Modemdata.Seconds);
                                  z = dosfputs(TempStr, &Outfile);
                                  break;

          /* Port Number */
          case 52  :  z = dosfputs(PortStr, &Outfile);
                                  break;

          /* Port Speed */
          case 53  :  ltoa(BaudRate, Baud3, 10);
                                  z = dosfputs(Baud3, &Outfile);
                                  break;

          /* Lock Port Speed */
          case 54  :  if (Modemdata.Lock || BaudRate > 9600L)
                                        z = dosfputs("-1", &Outfile);
                                  else
                                        z = dosfputs("0", &Outfile);
                                   break;

           /* Modem Initialization String */
           case 55  :  z = dosfputs(Modemdata.Init1, &Outfile);
                                   break;

           /* Modem Off-Hook String */
           case 56  :  z = dosfputs(Modemdata.Offhook, &Outfile);
                                   break;

           /* Disable CTS/RTS checking */
           case 57  :  z = dosfputs("0", &Outfile);
                                   break;

           /* Number of Rings before answering the phone */
       case 157 :  if (Modemdata.CallID)
                     z = dosfputs("2", &Outfile);
                   else
                     z = dosfputs("1", &Outfile);
                   break;

       /* IRQ # if not COM1 or COM2 */
       case 158 :  itoa(Irq, TempStr, 10);
                   if (ComPort == '1') {
                     z = dosfputs("4", &Outfile);
                                   } else if (ComPort == '2') {
                                         z = dosfputs("3", &Outfile);
                                   } else
                                         z = dosfputs(TempStr, &Outfile);
                                   break;

           /* BASE address if not COM1 or COM2 */
           case 159 :  switch(ComPort) {
                                         case '1' :  z = dosfputs("3F8", &Outfile); break;
                                         case '2' :  z = dosfputs("2F8", &Outfile); break;
                                         default  :  z = dosfputs(Base, &Outfile);  break;
                                   }
                                   break;

           /* Leave DTR up at drop to DOS */
           case 160 :  z = dosfputs("Y", &Outfile);
                                   break;

           /* Set to -1 if using OS/2 with API Intelligent card driver */
           case 272 :  if (PcbData.OS2Driver && CommDrv == 'O')
                                         z = dosfputs("-1", &Outfile);
                                   else
                                         z = dosfputs("0", &Outfile);
                                   break;

           /* Initialize string #2 */
           case 313 :  if (Modemdata.Fax || Modemdata.CallID)
                                         z = dosfputs(Modemdata.Init2, &Outfile);
                                   else
                                         z = dosfputs("", &Outfile);
                                   break;

           /* For non-PCBModem specific lines. Simply write what was read */
           default  :  z = dosfputs(TempStr, &Outfile);
                                   break;

        }
        z = dosfputs("\r\n", &Outfile);      /* Return and feed to next line */

        if (z == -1) {
          dosfclose(&Outfile);
          dosfclose(&Infile);
          z = copyfile(PCBOld_File, PCBDat_File, 2);
          quit(0,"");
        }
  }
  dosfclose(&Infile);
  dosfclose(&Outfile);                /* Close file and reset variables */
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void  commandline(void)
{
  ComPort = LocalComPort+48;

  if (!BaudRate)
        BaudRate = Modemdata.Baud;

  if (BaudRate > Modemdata.Baud)
        BaudRate = Modemdata.Baud;

  if (CommDrv != 'A')
        IntPort = LocalComPort;

  if (V42 && Modemdata.V42)
        Modemdata.V42 = 1;
  else
        Modemdata.V42 = 0;

  if (Fax && Modemdata.Fax)
        Modemdata.Fax = 1;
  else
        Modemdata.Fax = 0;
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static void  dsp_prms(void)
{
   if (!Help)
         fastprint(5, 4, "Invalid Number of Command Line Switches",Colors[MENUSELECT]);

   fastprint(3, 6,  "PCBModem /parameters",Colors[MENUSELECT]);
   fastprint(5, 7,  "/PORT:x        'x' is 1 - 8 for COMx",Colors[MENUSELECT]);
   fastprint(5, 8,  "/BAUD:xxxx     'xxxx' is the opening port rate",Colors[MENUSELECT]);
   fastprint(5, 9,  "/FAX           To enable adaptive fax answering of modem",Colors[MENUSELECT]);
   fastprint(5, 10, "/IRQ:xx        IRQ number for COM3 - COM8",Colors[MENUSELECT]);
   fastprint(5, 11, "/BASE:xxx      BASE address for COM3 - COM8",Colors[MENUSELECT]);
   fastprint(5, 12, "/MODEM:xxx     'xxx' is the modem number",Colors[MENUSELECT]);
   fastprint(5, 13, "/V42           To enable V42bis for USRobotic modems",Colors[MENUSELECT]);
   fastprint(5, 14, "/MP:x          'x' is the type of Multiport Driver (C,F,O)",Colors[MENUSELECT]);
   fastprint(5, 15, "/NOUPDATE      Will not update the PCBOARD.DAT file",Colors[MENUSELECT]);
   fastprint(5, 16, "/NODEBAT:file  Batch file to update SET PCB= parameters",Colors[MENUSELECT]);
   fastprint(5, 17, "/FILE:file     Name/loc of PCBOARD.DAT to update",Colors[MENUSELECT]);
   gotoxy(3,18);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
static int pascal option(wordtype *CmdOptions, char *CmdStr, int NumOptions)
{
  int      Number,
           Len,
           WordLen;

  wordtype *p;

  for (Number = 0, p = CmdOptions; Number < NumOptions; Number++, p++) {
    for (Len = 0; Len <= 3; Len++)
      if (CmdStr[Len] != p->Word[Len])
        goto next;
    Len++;
    WordLen = strlen(p->Word);
    if (Len >= WordLen || (Len >= 3 && WordLen >= 3))
      return(Number);
next:;
  }
  return(-1);
}

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
