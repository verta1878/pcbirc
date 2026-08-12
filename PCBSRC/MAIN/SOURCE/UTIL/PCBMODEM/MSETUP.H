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


#define   PCBVERSTR  "for PCBoard 15.22"
#define   MAIN_MENU  0
#define   NumEKeys   27
#define   TOTOPTS    5
#define   TOTMENUOPT 6
#define   MenuReserved   5
#define   ChTotal        30
#define   MakerTotal     10
#define   MenuItemTotal  (MenuReserved + ChTotal + MakerTotal)
#define   NUMOPTIONS     12
#define   CFGCMDS        5

char *MenuChoice [TOTMENUOPT] = {
"Add New Manufacturer & Modem",
"Modify Modem type",
"Update History file",
"Create PCBMODEM.ZIP",
"Modify MSETUP.CFG",
"Modify Default Configurations",
};

char             YN [] = {2,'Y','N'};
char       mask_num [] = {3, 0, '0', '9'};
char  mask_alphanum [] = {3, 0, ' ', '~'};

char    VerifyStr[10] = { VERIFYSTR };
char    Head1 [] = "PCBModem Setup Program";
char    Head2 [] = "Copyright (C) 1995  Clark Development Company, Inc.";
char    * ConfName;

// Used with the 'Options' information
typedef struct
{
  char  Modems   [ 40 ],
        PCBDat   [ 40 ],
        History  [ 40 ],
        Name     [ 15 ],
        Version  [ 10 ];
} cfgtype;

struct  MenuType  Menu[ChTotal];


void  pascal (*MenuFunc[MenuItemTotal])(void);
void  pascal initall(void);
void  pascal defaultmodem(void);
void  pascal updatehistory(void);
void  pascal createzip(void);
void  pascal changemodem(void);
void  pascal newsetup(void);
void  pascal mainmenu(void);
void  pascal editdata(bool isDefault);
void  pascal editcfg(void);
int   pascal chkdatafile(void);
