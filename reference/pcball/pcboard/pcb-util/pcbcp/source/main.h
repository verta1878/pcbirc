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

#define ID_WARNHIGHLIGHT      201
#define ID_WARNALARM          202
#define ID_WARNALL            203
#define ID_WARNMINUTES        204
#define ID_WARNEVENTS         205
#define ID_WARNDOORS          206
#define ID_WARNXFERS          207
#define ID_WARNRESTART        208
#define ID_WARNRESTARTMIN     209
#define ID_VERIFYCHANGE       210
#define ID_PAGEALARM          211
#define ID_PAGEFOREGROUND     212

#define ID_USERNAME           220
#define ID_USERCITY           221
#define ID_OPERATION          222
#define ID_MESSAGE            223
#define ID_STATUS             224
#define ID_UPDTTIME           225
#define ID_UPDTMIN            226

#define ID_FIRSTNODE          230
#define ID_LASTNODE           231

#define ID_NOTEBOOKCTRL       240
#define ID_PATHNAME           241
#define ID_WORKDIR            242
#define ID_PARAMETERS         243
#define ID_PORT               244
#define ID_RUNSETTINGS        245
#define ID_COPYFROM           246
#define ID_COPYTO             247
#define ID_UNDO               248
#define ID_HELP               249

#define ID_WINDOWED           250
#define ID_MINIMIZED          251
#define ID_FULLSCREEN         252
#define ID_DISABLED           253
#define ID_AUTOSTART          254
#define ID_RUNONCONNECT       255
#define ID_NODESETTINGS       256

#define ID_PROGRESSBOX        260

#define DID_LOGOFF            270
#define DID_DROPDOS           271
#define DID_DROPDOSNOW        272
#define DID_CLEARNODE         273
#define DID_RECYCLE           274

#define ID_COPYFROMNODE       280
#define ID_COPYTOFIRST        281
#define ID_COPYTOLAST         282

#define ID_MOUSEEDIT          290
#define ID_MOUSERUN           291

#define ID_ENABLESTARTRUN     300
#define ID_DISABLESTARTRUN    301
#define ID_ENABLECONNECTRUN   302
#define ID_DISABLECONNECTRUN  303

#define IDR_MAIN         1       /* resource identifier */
#define IDR_POPUPMENU    2

#define IDD_UPDATE       1
#define IDD_FONTSETTINGS 2
#define IDD_EDITRECORD   3
#define IDD_WARNINGS     4
#define IDD_NOTEBOOK     5
#define IDD_NODESETTINGS 6
#define IDD_RUNSETTINGS  7
#define IDD_NODELIMIT    8
#define IDD_PROGRESS     9
#define IDD_COPYFROM     10
#define IDD_COPYTO       11
#define IDD_MOUSE        12
#define IDD_STARTUP      13

#define ID_FONTCHAR      0x100
#define ID_FONTSCALE     0x101
#define ID_FONTFIXED     0x102
#define ID_NUMLINES      0x103
#define ID_FIRSTSHOW     0x104
#define ID_INTERVAL      0x200

#define IDM_DROPDOS     901
#define IDM_DROPDOSNOW  902
#define IDM_LOGOFF      903
#define IDM_DONOTHING   904
#define IDM_POPUPEDIT   905
#define IDM_NOTEBOOK    906
#define IDM_RUNNODES    907
#define IDM_CLEAR       908
#define IDM_RECYCLE     909

/*
 *  Standard Dialog box ids
 */
#define FILEOPEN        2000
#define FILESAVE        2001

#define OPTIONSDISPLAY  3001

#define MSGBOXID        1001
#define OBJECTID        1002

/*
 *  Menu item ids
 */
  #define IDM_FILE                            1000
  #define IDM_FILEOPEN                        1100
  #define IDM_FILELIMIT                       1200
  #define IDM_FILENODE                        1300
  #define IDM_FILEEXIT                        1400

  #define IDM_EDITRECORD                      2000

  #define IDM_ACTION                          3000
  #define IDM_ACTSCRN                         3100
  #define IDM_ACTSCRNDROPDOSNOW               3110
  #define IDM_ACTSCRNDROPDOS                  3120
  #define IDM_ACTSCRNLOGOFF                   3130
  #define IDM_ACTSCRNRECYCLE                  3140
  #define IDM_ACTSCRNCLEAR                    3150
  #define IDM_ACTALL                          3200
  #define IDM_ACTALLDROPDOSNOW                3210
  #define IDM_ACTALLDROPDOS                   3220
  #define IDM_ACTALLLOGOFF                    3230
  #define IDM_ACTALLRECYCLE                   3240
  #define IDM_ACTALLCLEAR                     3250
  #define IDM_ACTRUNNODES                     3300
  #define IDM_ACTRESTARTNODES                 3400
  #define IDM_ACTHIDENODES                    3500
  #define IDM_ACTSHOWNODES                    3600
  #define IDM_ACTARRANGENODES                 3700

  #define IDM_UPDATE                          4000
  #define IDM_UPDATENOW                       4100
  #define IDM_UPDATEINTERVAL                  4200

  #define IDM_OPTIONS                         5000
  #define IDM_OPTIONSFONT                     5100
  #define IDM_OPTIONSDISPLAY                  5200
  #define IDM_OPTIONSWARNINGS                 5300
  #define IDM_OPTIONSSTARTUP                  5400
  #define IDM_OPTIONSMOUSE                    5500
  #define IDM_OPTIONSSAVE                     5600

  #define IDM_HELP                            9000
  #define IDM_HELPUSINGHELP                   9100
  #define IDM_HELPGENERAL                     9200
  #define IDM_HELPKEYS                        9300
  #define IDM_HELPINDEX                       9400
  #define IDM_HELPTUTORIAL                    9500
  #define IDM_HELPPRODUCTINFO                 9600

/*
 *  Dialog item ids
 */
#define ID_OK               1
#define ID_CANCEL           2

/*
 *  Stringtable ids
 */
#define IDS_FILEOPENEXT             1
#define IDS_APPNAME                 2
#define IDS_HELPLIBRARYNAME         3
#define IDS_OPEN                    4
#define IDS_HELPWINDOWTITLE         5
#define IDS_SAVE                    6
#define IDS_OBJECTCLASS             7
#define IDS_FONTDLGTITLE            8
#define IDS_STATUSCHANGETITLE       9
#define IDS_STATUSCHANGEMSG        10
#define IDS_EDITNODESTATUS         11
#define IDS_SAMPLETEXT             12
#define IDS_PREVIEWTEXT            13
#define IDS_SETTINGSTEXT           14
#define IDS_LAUNCHING              15

/*
 *  Messagetable ids
 */
#define IDMSG_INITFAILED                1
#define IDMSG_MAINWINCREATEFAILED       2
#define IDMSG_CANNOTOPENUSERNETFILE     3
#define IDMSG_WRONGVERSION              4
#define IDMSG_CANNOTRUNCOLOR            5
#define IDMSG_CANNOTGETHPS              6
#define IDMSG_HELPLOADERROR             7
#define IDMSG_CANNOTLOADSTRING          8
#define IDMSG_HELPDISPLAYERROR          9
#define IDMSG_CANNOTLOADEXITLIST       10
#define IDMSG_CANNOTGETFILEINFO        11
#define IDMSG_CANNOTALLOCATEMEMORY     12
#define IDMSG_CANNOTREADFILE           13
#define IDMSG_CANNOTWRITETOFILE        14
#define IDMSG_CANNOTLOADFONTS          15
#define IDMSG_NODESWILLCLOSE           16
#define IDMSG_CONFIRMEXIT              17
#define IDMSG_FIRSTNODELESS            18
#define IDMSG_LIMITSERROR              19
#define IDMSG_COPYRANGEERROR           20
#define IDMSG_SAVECONFIGCHANGES        21
#define IDMSG_CHANGESMADE              22


#define UM_UPDATEBOX                   WM_USER + 1
#define UM_NEWPAGE                     WM_USER + 2
#define UM_STARTUP                     WM_USER + 3
#define UM_SEMERROR                    WM_USER + 4
/*
 *  Ids of messages used between threads
 */
#define TM_THREADINITFAILED         WM_USER+300
#define TM_UPDATETITLE              WM_USER+301

/*
 *  Other constants
 */
#define MESSAGELEN         128    /* maximum length for messages */
#define TITLESEPARATORLEN    4

/* return values for initialization routines */
#define RETURN_SUCCESS      0   /* successful return */
#define RETURN_ERROR        1   /* error return */
#define BEEP_WARN_FREQ      60  /* frequency of warning beep */
#define BEEP_WARN_DUR      100  /* duration of warning beep */


/*
 *  Included in the sample template is code for that creates a
 *  second thread that can be used for background processing.
 *  To use this thread, enable the BACKGROUND_THREAD constant.
 *  This will enable the routines that create and destroy the
 *  background processing thread.  The routines for the thread
 *  itself are located in the thrd.c file.
 */

#define BACKGROUND_THREAD
/***************************  End of main.h  ****************************/
