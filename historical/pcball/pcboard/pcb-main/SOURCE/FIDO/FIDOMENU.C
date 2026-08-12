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


#ifdef FIDO


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <dos.h>
#include <malloc.h>

#include <misc.h>
#include <bug.h>
#include <pcb.h>
#include <pcboard.h>
#include <pcboard.ext>
#include <messages.h>
#include <screen.h>
#include <users.h>
#include <ansi.h>

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include <pcbtoss.hpp>
#include <tossmisc.h>
#include "times.h"
#include "byte.h"
#include "fidoque.hpp"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define   NUM_MENUS   10


/*****************************************************************************
 Prototypes
 *****************************************************************************/
typedef   void (_NEAR_ *functype)(void);
static    void  _NEAR_ addFilesToQueue(char * Addr, char * files);
static    void  _NEAR_ send_file(void);
static    void  _NEAR_ do_poll_node(void);
static    void  _NEAR_ do_mail_scan(void);
static    void  _NEAR_ do_outbound_mail(void);
static    void  _NEAR_ LIBENTRY draw_fido_menu(void);
static    void  _NEAR_ do_fido_freq(void);
static    void  _NEAR_ do_nodelist_compile(void);
static    void  _NEAR_ vmqueue(void);
static    void  _NEAR_ toss_fido_mail(void);
static    void  _NEAR_ send_mail(void);
static    void  _NEAR_ tossNetmail(void);

#if __BORLANDC__ >= 0x500
// see \PROJ\PCB\SOURCE\SUPPORT\NONOVRLY.C for an explanation
void LIBENTRY fido_addqueue(QUEUE_RECORD & rec);
#endif


char    items[NUM_MENUS][30] = {
  "Poll     a Node.          ",
  "Request  a File.          ",
  "Transmit a File.          ",
  "Force Next Call.          ",
  "View/Modify Queue.        ",
  "Scan for Outbound Mail.   ",
  "Process  Inbound  Mail.   ",
  "Compile  Nodelist.        ",
  "Send Mail to a Node.      ",
  "Scan for Outbound Netmail ",
};

functype func[NUM_MENUS] = {
  do_poll_node,
  do_fido_freq,
  send_file,
  do_outbound_mail,
  vmqueue,
  do_mail_scan,
  toss_fido_mail,
  do_nodelist_compile,
  send_mail,
  tossNetmail,
};


static void _NEAR_ send_file(void)
{
QUEUE_RECORD  queue_rec;
char          mask_address[7]={6,0,'0','9',':','/','.'};
char          toAddr[25];
char          fileSpec[100];
int           screen;

  checkstack();
  memset(&queue_rec,0,sizeof(queue_rec));

  screen = memsavescreen();
  clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
  box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,ScrnColors->BoxColor,DOUBLE);
  fastprint(27,10,"       Node to Send to",ScrnColors->v14Color);
  fastprint(27,11,"                             ",0x01);
  agotoxy(27,11);
  inputfieldstr(toAddr,"_",0x07,sizeof(toAddr)-1,DEFAULTS,NOHELP,mask_address);

  if(toAddr[0]==NULL || toAddr[0]==' ')
  {
    memrestorescreen(screen,FREESCREEN);
    return;
  }

  default_address(toAddr,sizeof(toAddr));
  fastprint(27,11,toAddr,0x07);

  fastprint(23,13,"File Name to Send. (Wild cards OK)",ScrnColors->v14Color);
  fastprint(23,14,"                                  ",0x01);
  agotoxy(23,14);
  inputfieldstr(fileSpec,"_",0x07,sizeof(fileSpec)-1,UPCASE,NOHELP,mask_wildname);

  if(fileSpec[0]==NULL || fileSpec[0]==' ')
  {
    memrestorescreen(screen,FREESCREEN);
    return;
  }

  addFilesToQueue(toAddr,fileSpec);
  memrestorescreen(screen,FREESCREEN);
}


static void _NEAR_ addFilesToQueue(char * Addr, char * files)
{

  QUEUE_RECORD rec;
  ffblk        fileBlk;
  int          done;
  char         path[100];
  char *       nullIt = NULL;
  bool         noticeSent = FALSE;
  int          dosfindmax=1;
  #ifdef __OS2__
  int DirHandle = SYSTEMDIRHANDLE;
  #endif


  // Initialize date
  memset(&rec,0,sizeof(rec));
  maxstrcpy(path,files,sizeof(path));
  maxstrcpy(rec.nodestr,Addr,sizeof(rec.nodestr));
  rec.flag |= Q_FILESEND;
  rec.flag |= Q_NORMAL ;
  //rec.readOnly = TRUE;

  // Isolate path to filespec
  nullIt = strrchr(path,'\\');
  if(nullIt) *nullIt = '\x0';


  // Start finding the files and add them to the queue.
  dosfindmax = 1;
  done = dosfindfirst(files,&fileBlk,0,&dosfindmax PDIRHANDLE);

  cTOSSER * tosser = new(cTOSSER);
  while(tosser && !done)
  {
    sprintf(rec.filename,"%-1.*s\\%-1.13s",sizeof(path)-13,path,fileBlk.ff_name);
    // Limit Q's scope
    {
      #if __BORLANDC__ >= 0x500
        fido_addqueue(rec);
      #else
        cNEWQ Q;
        Q.addEntry(rec);
      #endif
    }
    if(!noticeSent)
    {
      tosser->send_file_notice(rec.nodestr,rec.filename);
      noticeSent =TRUE;

    }
    done = dosfindnext(&fileBlk,&dosfindmax PDIRHANDLE2);
  }
  if(tosser) delete(tosser);
}


static void _NEAR_ do_poll_node(void)
{
  checkstack();
  poll_node();
}

static void _NEAR_ do_outbound_mail(void)
{
  checkstack();
  send_outbound_mail(NULL,FROM_MENU);
}

static void _NEAR_ toss_fido_mail(void)
{
  checkstack();
  if(!OKtoImportExport()) return;
  toss_messages();
}

static void _NEAR_ do_mail_scan(void)
{
  checkstack();
  if(!OKtoImportExport()) return;
  copy_pcb_messages();
}


static void _NEAR_ do_fido_freq(void)
{
  checkstack();
  create_freq();
}


static void _NEAR_ do_nodelist_compile(void)
{
int   screen;
char  path[100];

  checkstack();
  strcpy(path,"PCBNLC.EXE");
  srchpath(path);
  screen=memsavescreen();
  //performshell(path," ",SHELLDIRECT);
  performshell(path," ",SHELLDIRECT,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,-1);
  memrestorescreen(screen,FREESCREEN);
}


static void _NEAR_ vmqueue(void)
{
cNEWQ Q;
  checkstack();
  Q.view();
}


void do_fido_menu(void)
{
char            input[6];
int             sel=0;
const int       ESC   = 27;
int             screen;

  checkstack();
  memset(input,0,sizeof(input));
  Status.FileXfer = TRUE;

  setcursor(CUR_BLANK);
  screen=memsavescreen();

  sel = 0;

/*
  /// Make sure no other nodes are processing mail
  if(!okaytohandlemail())
  {
    clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
    box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,ScrnColors->BoxColor,DOUBLE);
    fastprint(24,7,"Another node is processing mail.",ScrnColors->v14Color);
    //fastprint(25,8,"Cannot access Fido Queue.",ScrnColors->v14Color);
    fastprint(24,10,"Please wait for the other ",ScrnColors->v14Color);
    fastprint(24,11,"node to finish or abort ",ScrnColors->v14Color);
    fastprint(24,12,"processing on that node.",ScrnColors->v14Color);
    settimer(10,SIXTY_SECONDS);
    while(kbdhit(NOBUFFER) == 0 && !timerexpired(10))
      giveup();
    memrestorescreen(screen,FREESCREEN);
    Status.FileXfer = FALSE;
    setcursor(CUR_BLANK);
    return;
  }
*/

  draw_fido_menu();
  settimer(10,SIXTY_SECONDS);

  while(TRUE)
    {
      #ifdef __OS2__
        releasekbdblock(60000);
        sel = waitforkey();
      #else
        sel = kbdhit(NOBUFFER);
      #endif

      if(timerexpired(10)) break;

      if(sel == ESC) break;
      if(sel >= '1' && sel <= '1' + NUM_MENUS) sel += 16;
      if(sel >= 'a' && sel <= 'a' + NUM_MENUS) sel -= 32;
      if(  (sel >= 'A' && sel <= 'A' + NUM_MENUS))  // 49=ASCII char '1'
      {
         sel -= 'A';

         if(func[sel]!=NULL)  func[sel]();

         draw_fido_menu();
         settimer(10,SIXTY_SECONDS);
      }
      #ifndef __OS2__
        giveup();
      #endif
    }

  memrestorescreen(screen,FREESCREEN);
  setcursor(CUR_BLANK);
  Status.FileXfer = FALSE;
}


static void _NEAR_ LIBENTRY draw_fido_menu(void)
{
int   i=0;
char  buffer[35];

  checkstack();
  memset(buffer,0,sizeof(buffer));
  clsbox(22,5,57,19,0x11);
  box(22,5,57,19,ScrnColors->BoxColor,DOUBLE);
  fastcenter(6,"Sysop FIDO Menu",ScrnColors->v14Color);
  for(i=0;i<NUM_MENUS;i++)
    {
      sprintf(buffer,"%c) %s",(i+1+('A'-1)),items[i]);
      assert(strlen(buffer) < sizeof(buffer));
      fastprint(26,8+i,buffer,ScrnColors->BoxColor);
    }

  //fastprint(26,9+i,"Enter selection: ",ScrnColors->BoxColor);
  agotoxy(43,9+i);
  #ifdef __OS2__
    updatelinesnow();
  #endif
}


static void _NEAR_ send_mail(void)
{
QUEUE_RECORD q_rec[10],tmp;
char  buf[50];
int   last = 0,i=0,res = 0;
bool  dupe = FALSE;
//char  buffer[60];

    checkstack();
    while(TRUE)
    {
        for(i=0;i<10;i++) memset(&q_rec[i],0,sizeof(q_rec[i]));
        i=0;
        clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
        box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,ScrnColors->BoxColor,DOUBLE);
        fastprint(MNU_TOPX+3,MNU_TOPY+1,"Address's With Outgoing Packets",ScrnColors->v14Color);

        while(q_rec[9].nodestr[0] == '\x0')
        {
        cNEWQ Q;
          last = Q.getNextRecord(tmp,last+1);
          if(last == -1) break;
          for(int j=0;j<10;j++)
          {
            if(strcmp(tmp.nodestr,q_rec[j].nodestr)==0)
            {
              dupe = TRUE;
              break;
            }

          }

          if(!dupe)
          {
           memcpy(&q_rec[i],&tmp,sizeof(QUEUE_RECORD));
           dupe = FALSE;
           i++;
          }
          else
            dupe = FALSE;
        }
        for(i=0;i<10;i++)
        {
          if(q_rec[i].nodestr[0] != '\x0')
          {
            sprintf(buf,"%d) %s",i,q_rec[i].nodestr);
            fastprint(MNU_TOPX+3,MNU_TOPY+2+i,buf,ScrnColors->BoxColor);
          }
        }
        i = 0;

        settimer(10,SIXTY_SECONDS);
        res = 0;
        while(res == 0 && !timerexpired(10))
        {
          #ifdef __OS2__
            releasekbdblock(60000);
            res = waitforkey();
          #else
            giveup();
            res = kbdhit(NOBUFFER);
          #endif
        }
        if(timerexpired(10)) res = 'Q';

        if(res >=48  && res <= 59 && q_rec[res-48].nodestr[0] != '\x0')
          send_outbound_mail(q_rec[res-48].nodestr,FROM_MENU);

        if(res == 'Q' || res == 'q' || res == 27)
          return;
    }
}


static void _NEAR_ tossNetmail(void)
{
int screen=0;

        screen = memsavescreen();
        cTOSSER * t;
        t = new cTOSSER;
        if(t)
        {
           free_fido_memory();
           read_fido_config(ADDR_RECS);
           t->send_netmail();
           free_fido_memory();
           delete(t);
        }
        memrestorescreen(screen,FREESCREEN);

}
#endif
