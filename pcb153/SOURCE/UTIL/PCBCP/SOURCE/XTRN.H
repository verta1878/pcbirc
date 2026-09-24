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

#include <types.hpp>

extern HWND  hwndMainFrame;
extern HWND  hwndMain;
extern HWND  hwndVertScroll;
extern HAB   hab;
extern HMQ   hmq;
extern HDC   hdcMain;
extern CHAR  szAppName[];
extern CHAR  szLaunch[];
extern CHAR  szUntitled[];
extern BOOL  fPrintEnabled;
extern BOOL  fHelpEnabled;
extern HWND  hwndEditDlg;
extern RECTL rclDesktop;

extern HPS  hpsMain;
extern int  fontWidth;
extern int  fontHeight;
extern int  fontDescend;
extern int  fontProportional;
extern int  fontKerning;
extern int  numWinLines;
extern int  yTop;
extern int  xRight;
extern int  SelectedNode;
extern int  SelectedNodeFirst;
extern int  SelectedNodeLast;
extern bool MainIsMinimized;

enum {RUN_DISABLED=0,RUN_STARTUP,RUN_ONCONNECT};

typedef struct {
  int  NumLines;           // if FixedFont==FALSE, specifies # of lines on screen
  int  UpdateInterval;     // # of seconds in between screen updates
  int  FirstShow;          // first node to be shown on screen
  int  FirstNode;          // first node to monitor or configure
  int  LastNode;           // last node to monitor or configure
  int  WarnSeconds;        // # of seconds before decided a node may be locked up
  int  WarnEventSeconds;   // # of seconds before warning an event is old
  int  WarnDoorSeconds;    // # of seconds before warning a door is old
  int  WarnXferSeconds;    // # of seconds before warning a file transfer is old
  int  WarnRestartSeconds; // # of seconds before warning a file transfer is old
  bool FixedFont;          // TRUE=use fixed font size, FALSE=use scalable font
  bool ShowOld;            // TRUE=highlight nodes that may be locked up
  bool WarnOld;            // TRUE=sound alarm if a node may be lockedup
  bool MonitorAll;         // TRUE=monitor all nodes (not just those on screen)
  bool RestartInactive;    // TRUE=restart nodes that are inactive for 1 min.
  bool VerifyChanges;      // TRUE=verify status changes with pop-up window
  bool PageAlarmEnabled;   // TRUE=page alarm is enabled
  bool PageBringToFore;    // TRUE=bring paging node to the foreground
  bool ClickToRun;         // TRUE=doubleclick to launch or switch-to a node
  bool DisableStartRun;    // TRUE=disable auto-run of nodes at startup
  bool DisableConnectRun;  // TRUE=disable auto-run of nodes on connection
} settingstype;

typedef struct {
  char PathName[128];
  char Parameters[128];
  char WorkDir[128];
  char PortNum;
  char AutoStart;       // 0=Disabled, 1=Run at startup, 2=Run on connect
  char WindowType;      // 0=Windowed, 1=Minimized, 2=Full Screen
} nodeinfotype;

extern settingstype Settings;

#pragma pack(1)
typedef struct {
  unsigned short Version;     /* PCBoard version number */
  unsigned short NumOfNodes;  /* Number of nodes supported in the USERNET.DAT */
  unsigned short SizeOfRec;   /* The size of each node record */
} usernethdrtype;
#pragma pack()

#pragma pack(1)
typedef union {
  long DateTime;
  struct {
    uint Time;            /* number of seconds past midnight divided by 2 */
    uint Date;            /* julian date */
  } Split;
} updttype;
#pragma pack()

#pragma pack(1)
typedef struct {
  char     Status;         /* node status */
  bool     MailWaiting;    /* true if msg posted */
  uint     Pager;          /* node number of pager */
  char     Name[26];       /* caller's name */
  char     City[25];       /* caller's city */
  char     Operation[49];  /* current operational text */
  char     Message[80];    /* broadcast message text */
  char     Channel;        /* channel number of pager */
  updttype LastUpdate;     /* julian date + (# of seconds past midnight / 2) */
} nodetype;
#pragma pack()

#define USERNETFLAGSIZE ((UserNetHeader.NumOfNodes+7)/8)
#define USERNETSTART    (sizeof(usernethdrtype) + (USERNETFLAGSIZE*2))
#define USERNETFLAGS    (sizeof(usernethdrtype) + USERNETFLAGSIZE)

extern int            UserNetFile;
extern usernethdrtype UserNetHeader;
extern int            MaxNodes;
extern nodetype      *NodeArray;
extern nodetype      *SaveArray;
extern int            SkipScreenUpdate;
extern nodeinfotype  *NodeInfo;
extern int           *SessionIds;
extern int           *Semaphores;
extern bool          *StopNodes;
extern char           ComSpec[128];

#define NUMOPTIONS 22
extern char *Options[NUMOPTIONS];

/*
 *  Entry point declarations
 */

/* from main.c */
int main(VOID);
MRESULT EXPENTRY MainWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
ULONG   LIBENTRY MessageBox(HWND hwndOwner, ULONG idMsg, ULONG idTitle, ULONG fsStyle, BOOL fBeep);

/* from init.c */
char *  LIBENTRY loadstring(int ID, char *Buf, int MaxLen);
BOOL    LIBENTRY Init(VOID);
MRESULT LIBENTRY InitMainWindow(HWND hwnd, MPARAM mp1, MPARAM mp2);
VOID    LIBENTRY ExitProc(USHORT usTermCode);
void    LIBENTRY setprofilefont(void);
void    LIBENTRY setprofilesize(void);
bool    LIBENTRY isprofilesizeloaded(void);
void    LIBENTRY readnodeprofile(void);
void    LIBENTRY writenodeprofile(void);
void    LIBENTRY readprofile(void);
void    LIBENTRY writeprofile(void);

/* from file.c */
void LIBENTRY setmenuitems(void);
int  LIBENTRY openusernetfile(char *FileName);
void LIBENTRY closeusernetfile(void);
VOID LIBENTRY FileNew(MPARAM mp2);
VOID LIBENTRY FileOpen(MPARAM mp2);
VOID LIBENTRY FileSave(MPARAM mp2);
VOID LIBENTRY FileSaveAs(MPARAM mp2);
VOID LIBENTRY WriteFileToDisk(HFILE hf);
BOOL LIBENTRY GetFileName(VOID);
VOID LIBENTRY UpdateTitleText(HWND hwnd, char *Text, int NodeNum);
MRESULT EXPENTRY FAR TemplateOpenFilterProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

/* from edit.c */
// VOID EditUndo(MPARAM mp2);
// VOID EditCut(MPARAM mp2);
// VOID EditCopy(MPARAM mp2);
// VOID EditPaste(MPARAM mp2);
// VOID EditClear(MPARAM mp2);

/* from user.c */
void    LIBENTRY setscrollbar(void);
void    LIBENTRY setspinbuttonvalue(HWND hwnd, int ID, int Value);
void    LIBENTRY setspinbutton(HWND hwnd, int ID, int High, int Low, int Current);
ULONG   LIBENTRY getspinbuttonvalue(HWND hwnd, int ID);
void    LIBENTRY relatespinbutton(HWND hwnd, MPARAM mp1, int LowID, int HighID);
VOID    LIBENTRY UserCommand(MPARAM mp1, MPARAM mp2);
MRESULT UserWndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
VOID    LIBENTRY InitMenu(MPARAM mp1, MPARAM mp2);
VOID    LIBENTRY EnableMenuItem(HWND hwndMenu, USHORT idItem, BOOL fEnable);
VOID    LIBENTRY EditRecord(VOID);
void    LIBENTRY centerwindow(HWND hwnd);
void    LIBENTRY getfontparams(void);
int     LIBENTRY getstatusnum(int Status);
char    LIBENTRY getstatusletter(int Num);
char *  LIBENTRY getstatustext(int Status);

/* from pnt.c */
VOID LIBENTRY MainPaint(HWND hwnd);

/* from help.c */
VOID LIBENTRY InitHelp(VOID);
VOID LIBENTRY HelpIndex(VOID);
VOID LIBENTRY HelpGeneral(VOID);
VOID LIBENTRY HelpUsingHelp(VOID);
VOID LIBENTRY HelpKeys(VOID);
VOID LIBENTRY HelpTutorial(VOID);
VOID LIBENTRY HelpProductInfo(VOID);
VOID LIBENTRY DisplayHelpPanel(ULONG idPanel);
VOID LIBENTRY DestroyHelpInstance(VOID);

/* from thrd.c */
BOOL    LIBENTRY CreateBackgroundThread(VOID);
VOID    LIBENTRY DestroyBackgroundThread(VOID);
BOOL    LIBENTRY PostBkThreadMsg(ULONG msg, MPARAM mp1, MPARAM mp2);
MRESULT LIBENTRY SendBkThreadMsg(ULONG msg, MPARAM mp1, MPARAM mp2);
void    LIBENTRY restartnodes(void);
void    LIBENTRY hidenodes(void);
void    LIBENTRY shownodes(void);
void    LIBENTRY arrangenodes(void);
bool    LIBENTRY areanynodesrunning(void);
void    LIBENTRY runorswitchtoselectednodes(void);
void    LIBENTRY selectandconfirmchange(HWND hwnd, bool AllNodes, char Status);
void    LIBENTRY writeselectednode(nodetype *Buf);
void    LIBENTRY writeselectednodes(char Status);
void    LIBENTRY deselectnodes(void);

/***************************  End of xtrn.h  ****************************/
