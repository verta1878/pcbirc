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
#include <string.h>
#include <pcb.h>
#include <misc.h>
#include "setup.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

extern pcbconftype WriteConf; /* this is pulled from DATAWRIT.C in PCB_x.LIB */

#ifdef FIDO
#define NUMSCRNS 47
#else
#define NUMSCRNS 32
#endif


char _FAR_ * _FAR_ ScreenName[NUMSCRNS] = {
  "Sysop Information",
  "File Locations",
  "Modem Information",
  "Node Configuration",
  "Event Setup",
  "Subscription",
  "Configuration Options",
  "Security Levels",
  "Accounting Configuration",
#ifdef FIDO
  "Fido Configuration",
#endif
  "UUCP Configuration",
  "Main Board Configuration",
  "Conferences",
  "Global Search & Replace",

  "System Files & Directories",
  "Configuration Files",
  "Display Files",
  "New User/Logon/off Questionnaires",

  "Modem Setup",
  "Configuration Switches",
  "Allowed Access Speeds",

  "Messages",
  "File Transfers",
  "System Control",
  "Configuration Switches",
  "Logging Options",
  "Limits",
  "Colors",
  "Function Keys",
  "OS/2 Settings",

  "Sysop Functions",
  "Sysop Commands",
  "User Commands",

#ifdef FIDO
  "Fido Configuration",               /* fidocfg.c  */
  "Tosser Configuration",             /* fidotoss.c */
  "Node Configuration",               /* fidonode.c */
  "System Address",                   /* fidothis.c */
  "EMSI Profile",                     /* fidoemsi.c */
  "File & Directory Configuration",   /* fidoinfo.c */
  "Archiver Configuration",           /* fidoarc.c  */
  "Phone Number Translation",         /* fidoph.c   */
  "Nodelist Configuration",           /* fidolist.c */
  "FREQ Path List",                   /* fidopath.c */
  "FREQ Restrictions",                /* fidofreq.c */
  "FREQ Magic Names",                 /* fidomgic.c */
  "FREQ Deny Nodelist",               /* fidonofr.c */
  "Origin Conference Range",          /* fidoorg.c */
#endif
};

char _FAR_ * _FAR_ Questions[FLDTOTAL] = {
  "Sysop's Name (when NOT using Real Name)         ",
  "Local Password (used at the Call Waiting screen)",
  "Require Local Password to Exit PCBoard          ",
  "Use Real Name (Yes=Use name found in USERS file)",
  "Use Graphics When Logged On Locally             ",

  " (1) View/Print Caller Log ",
  " (2) View/Print User List  ",
  " (3) Pack Renumber Messages",
  " (4) Recover Killed Message",
  " (5) List Message Headers  ",
  " (6) View Any File         ",
  " (7) User Maintenance      ",
  " (8) Pack User File        ",
  " (9) Exit to DOS remote    ",
  "(10) Shelled DOS functions ",
  "(11) View Other Nodes      ",
  "(12) Logoff Alternate Node ",
  "(13) View Alt Node Callers ",
  "(14) Drop Alt Node to DOS  ",
  "(15) Drop to DOS/Recycle   ",

  "Sysop Level (for Sysop Menu and F1-Temp-Sysop Upgrade)",
  "Level Needed to Read All Comments                     ",
  "Level Needed to Read All Mail Except Comments         ",
  "Level Needed to Copy or Move Messages Between Areas   ",
  "Level Needed to Enter @-Variables in Message Base     ",
  "Level Needed to Edit Any Message in the Message Base  ",
  "Level Needed to NOT Update Msg Read Status (R O cmd)  ",
  "Level Needed to Use the BROADCAST Command             ",
  "Level Needed to View the Private Upload Directory     ",
  "Level Needed to Enter Generic Messages (@USER@)       ",
  "Level Needed to Edit Message Headers                  ",
  "Level Needed to Protect/Unprotect a Message           ",
  "Level Needed to Overwrite Files on Uploads            ",
  "Level Needed to Set the Pack-Out Date on Messages     ",
  "Level Needed to See All Return Receipt Messages       ",

  "Name/Loc of Conference Data      ",
  "Name/Loc of User File            ",
  "Name/Loc of User Info File       ",
  "Name/Loc of Caller Log           ",
  "Name/Loc of Group CHAT File      ",
  "Location of Group CHAT .CAP Files",
  "Name/Loc of Statistics File      ",
  "Name/Loc of USERNET.XXX File     ",
  "Name/Loc of Transfer Summary File",
  "Name/Loc of Swap File            ",
  "Location of PCBTEXT Files        ",
  "Location of User Index Files     ",
  "Location of Temporary Work Files ",
  "Location of Help Files           ",
  "Location of Login Security Files ",
  "Location of Command Display Files",

  "Name/Loc of PWRD/Security File   ",
  "Name/Loc of FSEC File            ",
  "Name/Loc of UPSEC File           ",
  "Name/Loc of User Trashcan File   ",
  "Name/Loc of Protocol Data File   ",
  "Name/Loc of Multi-Lang. Data File",
  "Name/Loc of Color Definition File",
  "Name/Loc of Default CMD.LST File ",
  "Name/Loc of All-Files DLPATH.LST ",
  "Name/Loc of Upload File Trashcan ",
  "Batch file for viewing compressed files",
  "Filename EXTENSION for compressed files",
  "External command for copying files     ",

  "Name/Loc of WELCOME File         ",
  "Name/Loc of NEWUSER File         ",
  "Name/Loc of CLOSED File          ",
  "Name/Loc of WARNING File         ",
  "Name/Loc of EXPIRED File         ",
  "Name/Loc of Conference Join Menu ",
  "Name/Loc of Group Chat Intro File",
  "Name/Loc of Group Chat Menu      ",
  "Name/Loc of NOANSI Warning       ",

  "Name/Loc of New Reg Questionnaire",
  "Name/Loc of Answers to New Reg.  ",
  "Name/Loc of Logon  Script Quest. ",
  "Name/Loc of Logon  Script Answers",
  "Name/Loc of Logoff Script Quest. ",
  "Name/Loc of Logoff Script Answers",

  "Seconds to wait for carrier   ",
  "Comm Driver to use (A/C/F/O)  ",
  "Comm Port (0=NONE/Local Only) ",
  "Opening Baud Rate (300-115200)",
  "Lock in Opening Baud Rate     ",
  "Modem Initialization String #1",
  "Modem Initialization String #2",
  "Modem Off-Hook String         ",
  "Modem Answer String           ",
  "Modem Dialout String          ",
  "Max # of Redials on Busy      ",
  "Max # of Attempts to Connect  ",

  "Disable CTS/RTS Checking     ",
  "Disable RTS-Drop During Write",
  "Using a FastComm 9600        ",
  "Reset Modem While Idle       ",
  "Reset Modem During Recycle   ",
  "Modem Off-Hook During Recycle",
  "Modem Delay During Recycle   ",
  "Packet-Switch Network        ",
  "Verify CD-Loss               ",
  "Leave DTR Up at Drop to DOS  ",
  "Drop to DOS on Missed Connect",
  "Answer on True Ring Detect   ",
  "Number of Rings Required     ",
  "Monitor Missed Connections   ",
  "Force NON-16550 Usage        ",
  "Force 16550A Usage           ",
  "Share IRQs on MCA Buses      ",

  "Allow Callers at 7,E,1 ",
  "Lowest Desired Baud    ",
  "Allow Lower Speeds     ",
  "Begin Time             ",
  "End Time               ",
  "Security Level Override",

  "Board Name",
  "Origin    ",
  "Parallel Port Num (1-3,0)",
  "Running a Network / Multitasker System    ",
  "Node Number on the Network                ",
  "Float Node Number                         ",
  "Network Timeout on Errors (20-99 secs)    ",
  "Node Chat Frequency (once every 5-99 secs)",
  "Include City Field in WHO Display         ",
  "Show ALIAS Names in WHO Display           ",
  "Using Slave Cards (slows file access)     ",

  "Is a Timed Event Active              ",
  "Name/Location of EVENT.DAT           ",
  "Location of EVENT Files              ",
  "Minutes Prior to Suspend All Activity",
  "Disallow Uploads Prior to Event      ",
  "Minutes Prior to Disallow Uploads    ",

  "Enable Subscription Mode             ",
  "Default Subscription Length in Days  ",
  "Default `Expired' Security Level     ",
  "Warning Days Prior to Expiration     ",

  "Number of Highest Conference Desired    ",
  "Maximum Lines in the Message Editor     ",
  "Message Capture - Maximum Messages      ",
  "Message Capture - Max Per Conference    ",
  "Name of Capture File (blank=caller#)    ",
  "Name of QWK Packet (blank=capture name) ",
  "Stop Clock for Capture File Download    ",
  "Disable Message Scan Prompt             ",
  "Allow ESC Codes in Messages             ",
  "Allow Carbon-Copy Messages              ",
  "Validate TO: Name in Messages           ",
  "Force COMMENTS-to-the-Sysop into Main   ",
  "Double-Byte Characters (Foreign Systems)",
  "Create MSGS File if Missing             ",
  "Default to (Q)uick on Personal Mail Scan",
  "Default to Scan ALL Conferences at Login",
  "Prompt to Read Mail when Mail Waiting   ",

  "Log Caller Number to Disk ",
  "Log Connect String to Disk",
  "Log Security Level to Disk",

  "Disable 3-minute Screen Clear     ",
  "Disable Registration Edits        ",
  "Disable High-ASCII Filter         ",
  "Default to Graphics At Login      ",
  "Use Non-Graphics Mode Only        ",
  "Exclude Local Logins from Stats   ",
  "Exit to DOS After Each Call       ",
  "Eliminate Screen Snow in PCB      ",
  "Display NEWS Only if Changed      ",
  "Display User Info at Login        ",
  "Force INTRO Display on Join       ",
  "Pre-load PCBTEXT File             ",
  "Pre-load CNAMES File              ",
  "Scan for New Bulletins            ",
  "Swap Out During Shell             ",
  "Swap Out During $$LOGON/LOGOFF.BAT",
  "Create USERS.SYS for $$LOGON.BAT  ",
  "Capture GROUP CHAT Session to Disk",
  "Allow Handles in GROUP CHAT       ",

  "Disable NS Logon Feature       ",
  "Disable Password Check (DOS)   ",
  "Multi-Lingual Operation        ",
  "Disable Full Record Updating   ",
  "Allow Alias Change after Chosen",
  "Run System as a Closed Board   ",
  "Enforce Daily Time Limit       ",
  "Allow One Name Users           ",
  "Allow Password Failure Comment ",
  "Warning on Logoff Command      ",
  "Allow Local SHELL to DOS       ",
  "Use NEWASK+Standard Questions  ",
  "Skip Protocol when Registering ",
  "Skip Alias when Registering    ",
  "Read PWRD on Conference Join   ",
  "Confirm Caller Name / Address  ",
  "Auto-Reg in Public Conf        ",
  "Encrypt Users File             ",

  "F-Key #1 ",
  "F-Key #2 ",
  "F-Key #3 ",
  "F-Key #4 ",
  "F-Key #5 ",
  "F-Key #6 ",
  "F-Key #7 ",
  "F-Key #8 ",
  "F-Key #9 ",
  "F-Key #10",

  "Keyboard Timeout (in min, 0=disable)  ",
  "Max Number of Upload Description Lines",
  "Maximum Number of Lines in Scrollback ",
  "DOS Environment Size When Shelled Out ",
  "Number Days Before FORCED Password Change ",
  "Number Days to Warn Prior to FORCED Change",
  "Minimum Password Length                   ",
  "Allow Sysop Page Start Time",
  "Allow Sysop Page Stop Time ",

  "$$LOGON/$$LOGOFF Processing ",
  "External Protocols          ",
  "Door Applications           ",
  "Upload Verification Process ",
  "File Viewer                 ",
  "PCBQWK & PCBCMPRS Processing",
  "All Other Shells            ",

  "Normal Processing           ",
  "External Protocols          ",
  "PCBQWK & PCBCMPRS Processing",
  "Fido Import Processing      ",
  "Fido Export Processing      ",
  "All Other Shells            ",

  "",
  "",
  "",
  "",
  "",
  "",
  "",

  "Default Color (@X code format)    ",
  "Color for Message Header DATE Line",
  "Color for Message Header TO   Line",
  "Color for Message Header FROM Line",
  "Color for Message Header SUBJ Line",
  "Color for Message Header READ Line",
  "Color for Message Header CONF Line",

  "Disallow BATCH Uploads         ",
  "Promote to Batch Transfers     ",
  "Upload Credit for Time         ",
  "Upload Credit for Bytes        ",
  "Include `Uploaded By' in Desc. ",
  "Verify Files Uploaded          ",
  "Disable Drive Size Check       ",
  "Upload Buffer Size (4-64)      ",
  "List of Slow Drive Letters     ",
  "Slow Drive Batch File          ",
  "Stop Uploads when Free Space is less than",

  "A) Abandon Conference  ",
  "B) Bulletin Listings   ",
  "C) Comment to Sysop    ",
  "D) Download a File     ",
  "E) Enter a Message     ",
  "F) File Directory      ",
  "H) Help Functions      ",
  "I) Initial Welcome     ",
  "J) Join a Conference   ",
  "K) Kill a Message      ",
  "L) Locate File Name    ",
  "M) Mode (graphics)     ",
  "N) New Files Scan      ",
  "O) Operator Page       ",
  "P) Page Length         ",
  "Q) Quick Message Scan  ",
  "R) Read Messages       ",
  "S) Script Questionnaire",
  "T) Transfer Protocol   ",
  "U) Upload a File       ",
  "V) View Settings       ",
  "W) Write User Info.    ",
  "X) Expert Mode Toggle  ",
  "Y) Your Personal Mail  ",
  "Z) Zippy DIR Scan      ",
  "Group CHAT/CHAT Status ",
  "OPEN a DOOR            ",
  "TEST a File            ",
  "USER Search/Display    ",
  "WHO is On Another Node ",
  "Level Required for BATCH File Transfers    ",
  "Level Required to EDIT Your Own Messages   ",
  "Level Given to Users Who Agree  to Register",
  "Level Given to Users Who Refuse to Register",

#ifdef FIDO
  "Enable Fido Processing                   ",
  "Location of FIDO Configuration Files     ",
  "Import Immediately After File Transfers  ",
  "Allow Node to Process Incoming Packets   ",
  "Scan for Inbound Packets Frequency (min) ",
  "Allow Node to Export Mail                ",
  "Scan for Mail to Export Frequency (min)  ",
  "Allow Node to Dial Out                   ",
  "Scan for Outbound Packets Frequency (min)",
  "Default Zone                             ",
  "Default Net                              ",
  "Fido Logging Level (higher = more detail)",

  "Security Req'd for +C and +D Modifiers   ",
  "Create MSG with Outbound Packets Attached",
  "Enable Inbound Routing                   ",
  "Secure Netmail                           ",
  "Change SYSOP to FIDO_SYSOP on Import     ",
  "Check for Dupes using Message Path       ",
  "Check for Dupes using MSGID              ",
  "Number of Messages to Track for Dupes    ",
  "Generate Response Messages               ",
  "Enable PassThru's                        ",
  "Enable Areafix Forwarding                ",
  "Auto Add Fido Areas as PassThru's        ",
  "Re-Address Routed Packets                ",
  "Route Echo Mail                          ",

  "Incoming Packets       ",
  "Outgoing Packets       ",
  "Bad      Packets       ",
  "Nodelist Database      ",
  "Work Directory         ",
  "*.MSG Files            ",
  "PassThru Files         ",
  "Secure Netmail Packets ",
  "Message/Response Files ",

  "BBS Name  ",
  "SysOp Name",
  "City/State",
  "Phone     ",
  "Baud      ",
  "Flags     ",

  "Session Max Time  ",
  "Session Max Bytes ",
  "Daily Max Time    ",
  "Daily Max Bytes   ",
  "Allowed Nodes     ",
  "Min Allowed Baud  ",
#endif

  "Conf Name",
  "Public Conference          ",
  "Req. Security if Public",
  "Password to Join if Private",
  "Auto-Register Flags    ",
  "Number of Message Blocks   ",
  "Name/Loc of MSGS File      ",
  "Name/Loc of User's Menu    ",
  "Name/Loc of Sysop's Menu   ",
  "Name/Loc of NEWS File      ",
  "Name/Loc of Conf INTRO File",
  "Location for Attachments   ",
  "Public Upload Sort Type",
  "Name/Loc of Public Upld DIR",
  "Location of Public Uploads",
  "Private Upload Sort Type",
  "Name/Loc of Private Upld DIR",
  "Location of Private Uploads",
  "Name/Loc of Doors Menu",
  "Name/Loc of Doors Listing",
  "Name/Loc of Bulletins Menu",
  "Name/Loc of Bulletins Listing",
  "Name/Loc of Scripts Menu",
  "Name/Loc of Scripts Listing",
  "Name/Loc of DIR Menu",
  "Name/Loc of DIR Listing",
  "Name/Loc of Download Path Listing",

  "Auto-Rejoin into this Conf.    ",
  "Allow Viewing Conf. Members    ",
  "Make All Uploads Private       ",
  "Make All Messages Private      ",
  "Echo Mail in Conference        ",
  "Force Echo on All Messages     ",
  "Type of NetMail Conference     ",
  "Allow Internet (long) TO: Names",
  "Make Conference Read-Only      ",
  "Disallow Private Messages      ",
  "Place ORIGIN Info in Messages  ",
  "Prompt for ROUTE Info          ",
  "Allow Aliases to be used       ",
  "Show INTRO in `R A' Scan       ",
  "Maintain Old MSGS.NDX File     ",
  "Conf-Specific CMD.LST File     ",
  "Additional Conference Security ",
  "Additional Conference Time     ",
  "Level to Save File Attachment  ",
  "Level to Enter a Message       ",
  "Level to Request Return Receipt",
  "Level to Enter Carbon List Msgs",
  "Carbon Copy List Limit         ",
  "Charge Per Minute         ",
  "Charge Per Message Read   ",
  "Charge Per Message Written",
  "Last Message Exported     ",

  "Enable Accounting Features      ",
  "Display Money instead of Credits",
  "Concurrent Tracking of Charges  ",
  "Ignore Empty Security Level     ",
  "Peak Usage Start Time           ",
  "Peak Usage End Time             ",
  "Peak Days of Week               ",

  "Name/Loc of Peak Holidays List File   ",
  "Name/Loc of Account Configuration File",
  "Name/Loc of Account Tracking File     ",
  "Name/Loc of Account INFO File         ",
  "Name/Loc of Account WARNING File      ",
  "Name/Loc of Account LOGOFF File       ",

  "New User Starting Balance       ",
  "Balance Warning Level           ",
  "Per Logon                       ",
  "Per Minute Online               ",
  "Per Minute Online Peak Time     ",
  "Per Minute in Group Chat (Added)",
  "Per Message Read                ",
  "Per Message Captured (QWK/c/d/z)",
  "Per Message Written             ",
  "Per Message Written (Echoed)    ",
  "Per Message Written (Private)   ",
  "Per File Downloaded             ",
  "Per 1K-Bytes Downloaded         ",
  "Per File Uploaded               ",
  "Per 1K-Bytes Uploaded           ",

  "Organization",
  "Base UUCP Path           ",
  "Spool Directory          ",
  "Log File Directory       ",
  "Decompress Batch File    ",
  "Moderator List File      ",
  "UUCP Name                ",
  "UUCP Domain Name         ",
  "UUCP Email Host          ",
  "UUCP News Host           ",
  "News Distribution        ",
  "Time Zone Offset from GMT",
  "Name Separator           ",
  "Internet Email Conference",
  "Usenet Junk Conference   ",
  "Bang Domain              ",
  "Sub Domain               ",
  "High Ascii (S/R/C/N)     ",
};


extern "C" {
char * pascal getcontext(void *Ptr) {
  static char  Result[128];
  char        *Screen;
  char        *Quest;
  char         Temp[128];

/*
  Screen = ScreenName[1];
  if (Ptr == &PcbData.HlpLoc ) { Quest=Questions[FLDFILELOC1+ 0]; goto end; }
  if (Ptr == &PcbData.SecLoc ) { Quest=Questions[FLDFILELOC1+ 1]; goto end; }
  if (Ptr == &PcbData.ChatFile){ Quest=Questions[FLDFILELOC1+ 2]; goto end; }
  if (Ptr == &PcbData.TxtLoc ) { Quest=Questions[FLDFILELOC1+ 3]; goto end; }
  if (Ptr == &PcbData.NdxLoc ) { Quest=Questions[FLDFILELOC1+ 4]; goto end; }
  if (Ptr == &PcbData.UsrFile) { Quest=Questions[FLDFILELOC1+ 5]; goto end; }
  if (Ptr == &PcbData.ClrFile) { Quest=Questions[FLDFILELOC1+ 6]; goto end; }
  if (Ptr == &PcbData.CnfFile) { Quest=Questions[FLDFILELOC1+ 7]; goto end; }
  if (Ptr == &PcbData.PwdFile) { Quest=Questions[FLDFILELOC1+ 8]; goto end; }
  if (Ptr == &PcbData.FscFile) { Quest=Questions[FLDFILELOC1+ 9]; goto end; }
  if (Ptr == &PcbData.UscFile) { Quest=Questions[FLDFILELOC1+10]; goto end; }
  if (Ptr == &PcbData.TcnFile) { Quest=Questions[FLDFILELOC1+11]; goto end; }
  if (Ptr == &PcbData.WlcFile) { Quest=Questions[FLDFILELOC1+12]; goto end; }
  if (Ptr == &PcbData.NewFile) { Quest=Questions[FLDFILELOC1+13]; goto end; }
  if (Ptr == &PcbData.ClsFile) { Quest=Questions[FLDFILELOC1+14]; goto end; }
  if (Ptr == &PcbData.WrnFile) { Quest=Questions[FLDFILELOC1+15]; goto end; }
  if (Ptr == &PcbData.ExpFile) { Quest=Questions[FLDFILELOC1+16]; goto end; }

  Screen = ScreenName[2];
  if (Ptr == &PcbData.NetFile        ) { Quest=Questions[FLDFILELOC2+0]; goto end; }
  if (Ptr == &PcbData.CnfMenu        ) { Quest=Questions[FLDFILELOC2+1]; goto end; }
  if (Ptr == &PcbData.RegFile        ) { Quest=Questions[FLDFILELOC2+2]; goto end; }
  if (Ptr == &PcbData.AnsFile        ) { Quest=Questions[FLDFILELOC2+3]; goto end; }
  if (Ptr == &PcbData.TrnFile        ) { Quest=Questions[FLDFILELOC2+4]; goto end; }
  if (Ptr == &PcbData.DldFile        ) { Quest=Questions[FLDFILELOC2+5]; goto end; }
  if (Ptr == &PcbData.LogOffScr      ) { Quest=Questions[FLDFILELOC2+6]; goto end; }
  if (Ptr == &PcbData.LogOffAns      ) { Quest=Questions[FLDFILELOC2+7]; goto end; }
  if (Ptr == &PcbData.MultiLang      ) { Quest=Questions[FLDFILELOC2+8]; goto end; }
  if (Ptr == &PcbData.GroupChat      ) { Quest=Questions[FLDFILELOC2+9]; goto end; }
*/

  Screen = WriteConf.Name;
  if (Ptr == &WriteConf.MsgFile    ) { Quest=Questions[FLDCONFRNCE+ 6]; goto end; }
  if (Ptr == &WriteConf.UserMenu   ) { Quest=Questions[FLDCONFRNCE+ 7]; goto end; }
  if (Ptr == &WriteConf.SysopMenu  ) { Quest=Questions[FLDCONFRNCE+ 8]; goto end; }
  if (Ptr == &WriteConf.NewsFile   ) { Quest=Questions[FLDCONFRNCE+ 9]; goto end; }
  if (Ptr == &WriteConf.Intro      ) { Quest=Questions[FLDCONFRNCE+10]; goto end; }
  if (Ptr == &WriteConf.AttachLoc  ) { Quest=Questions[FLDCONFRNCE+11]; goto end; }
  if (Ptr == &WriteConf.UpldDir    ) { Quest=Questions[FLDCONFRNCE+13]; goto end; }
  if (Ptr == &WriteConf.PubUpldLoc ) { Quest=Questions[FLDCONFRNCE+14]; goto end; }
  if (Ptr == &WriteConf.PrivDir    ) { Quest=Questions[FLDCONFRNCE+16]; goto end; }
  if (Ptr == &WriteConf.PrvUpldLoc ) { Quest=Questions[FLDCONFRNCE+17]; goto end; }
  if (Ptr == &WriteConf.DrsMenu    ) { Quest=Questions[FLDCONFRNCE+18]; goto end; }
  if (Ptr == &WriteConf.DrsFile    ) { Quest=Questions[FLDCONFRNCE+19]; goto end; }
  if (Ptr == &WriteConf.BltMenu    ) { Quest=Questions[FLDCONFRNCE+20]; goto end; }
  if (Ptr == &WriteConf.BltNameLoc ) { Quest=Questions[FLDCONFRNCE+21]; goto end; }
  if (Ptr == &WriteConf.ScrMenu    ) { Quest=Questions[FLDCONFRNCE+22]; goto end; }
  if (Ptr == &WriteConf.ScrNameLoc ) { Quest=Questions[FLDCONFRNCE+23]; goto end; }
  if (Ptr == &WriteConf.DirMenu    ) { Quest=Questions[FLDCONFRNCE+24]; goto end; }
  if (Ptr == &WriteConf.DirNameLoc ) { Quest=Questions[FLDCONFRNCE+25]; goto end; }
  if (Ptr == &WriteConf.PthNameLoc ) { Quest=Questions[FLDCONFRNCE+26]; goto end; }
  return(NULL);   /* Ptr not found ... so return NULL */

end:
  // in case we run up on a long conference name, let's make sure it gets
  // truncated down to a smaller size
  sprintf(Temp,"%-25.25s",Screen);
  stripright(Temp,' ');

  sprintf(Result,"Screen: %s, Quest: %s",Temp,Quest);
  stripright(Result,' ');

  // one more attempt to protect the screen from blowing up...  if the result
  // string is too wide, truncate it now
  if (strlen(Result) > 70)
    Result[70] = 0;

  return(Result);
}
}
