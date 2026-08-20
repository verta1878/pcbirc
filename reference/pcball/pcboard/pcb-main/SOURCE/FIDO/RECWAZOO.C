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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <process.h>
#include <dos.h>
#include <assert.h>

#include <misc.h>
#include <bug.h>
#include <pcb.h>
#include <pcboard.h>
#include <pcboard.ext>
#include <messages.h>
#include <screen.h>
#include <users.h>

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include <tossmisc.h>
#include "times.h"
#include "byte.h"

/* If we want to have MemCheck included, we need to have the environment    */
/* variable DEBUG set.  Otherwise, MemCheck won't be compiled in.           */

#ifdef DEBUG
#include <memcheck.h>
#endif

/****************************************************************************/
/* Global Vars.                                                             */

static _FAR_ HELLO_T                  OurHello;           // Our hello structure
static _FAR_ HELLO_T                  RemoteHello;        // remote hello structure
static _FAR_ Emsi                     emsi;

extern NADDRESS               *address;
extern int                    num_akas;
extern _FAR_ EMSI_DATA        emsi_data;

/****************************************************************************
 Prototypes
 ****************************************************************************/

 static void _NEAR_ LIBENTRY fill_in_struct(void);
 static void _NEAR_ LIBENTRY convert_structure(void);
 static void _NEAR_ LIBENTRY logRemoteHello(void);
 static void _NEAR_ LIBENTRY logOurHello(void);
 static void _NEAR_ LIBENTRY getFidoUserPassword(char * password);

/****************************************************************************/
/* This function attempts to perform a WaZoo connection.  It follows the    */
/* protocol as it is described in FSC-0006.TXT dated 9/20/87.               */
/* The description has a state diagram which describes the connection       */
/* process.  This function simultates a state machine using the switch      */
/* statement. Returns TRUE if it was Fido FALSE if not.                     */

bool LIBENTRY Receiver_WaZoo_Init(void)
{
int           inchar=0,state=0,retry=0,sendcount=0;
char          nodestr[55];
bool          havesignal=FALSE;


  Asy.IgnoreCDLoss = FALSE;
  memset(nodestr,0,sizeof(nodestr));
  cls();

  state=4;

  fill_in_struct();

  while(cdstillup())
    {
      switch(state)
        {
          case 0:   retry=0;            /* initialize   */
                    sendbyte(ENQ);
                    if(PcbData.FidoLogLevel >= 4) writeFidolog("Yoohoo state 0 going to state 3",BLOCK);
                    state=3;
                   break;

          case 3:  if(Recv_Hello())
                   {

                    if(PcbData.FidoLogLevel >= 4) writeFidolog("Yoohoo state 3 going to state 4",BLOCK);
                     state=4;
                    }
                    else
                    {

                     if(PcbData.FidoLogLevel >= 4) writeFidolog("Yoohoo state 3 returning FALSE",BLOCK);
                     {
                      de_init_users();
                      return(FALSE);
                     }
                    }
                   break;

          case 4:   sendbyte(YOOHOO);     /* We now know it's Fido. Hankshake */
                    retry=0;              /* from remote was successfull.     */
                    havesignal=FALSE;
                    while(retry<3&&!havesignal&&cdstillup())
                      {
                        havesignal=FALSE;
                        settimer(9,TWENTY_SECONDS);
                        while(!havesignal&&!timerexpired(9)&&cdstillup())
                          {
                            if((inchar=comminkey())!=-1)
                              {
                                if(inchar==ENQ)
                                  {
                                    havesignal=TRUE;
                                    if(PcbData.FidoLogLevel >= 4) writeFidolog("Yoohoo state 4 going to state 5",BLOCK);
                                    state=5;
                                  }
                              }
                          }
                        if(!havesignal)
                          {
                            retry++;
                            if(retry<3)
                              {
                                while(comminkey()!=-1);
                                sendbyte(YOOHOO);
                              }
                          }
                      }
                    if(!havesignal)
                      {
                        //clearusernet();
                        if(PcbData.FidoLogLevel >= 4) writeFidolog("Yoohoo state 4 returning TRUE",BLOCK);
                        {
                          de_init_users();
                          return(TRUE);
                        }
                      }
                   break;

          case 5:   if(Send_Hello())
                    {
                     if(PcbData.FidoLogLevel >= 4) writeFidolog("Yoohoo state 5 going to state 6",BLOCK);
                      state=6;
                    }
                    else
                      {
                        de_init_users();
                        return(TRUE);
                      }
                   break;

          case 6:   convert_structure();
                    display_fido_info(emsi);

                    if(RemoteHello.my_zone!=0 && RemoteHello.my_point!=0)
                      sprintf(nodestr,"%d:%d/%d.%d",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node,RemoteHello.my_point);
                    else if(RemoteHello.my_zone!=0 && RemoteHello.my_point==0)
                      sprintf(nodestr,"%d:%d/%d",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node);
                    else if(RemoteHello.my_zone==0 && RemoteHello.my_point!=0)
                      sprintf(nodestr,"%d/%d.%d",RemoteHello.my_net,RemoteHello.my_node,RemoteHello.my_point);
                    else if(RemoteHello.my_zone==0 && RemoteHello.my_point==0)
                      sprintf(nodestr,"%d/%d",RemoteHello.my_net,RemoteHello.my_node);

                    TransferMessages(&sendcount,(char *)nodestr,INBOUND,ZMODEM);
                    //clearusernet();
                    de_init_users();
                    return(TRUE);
        }
    }
  //clearusernet();
  de_init_users();
  return(TRUE);
}

/****************************************************************************/
/* Fill in the Structure for the outbound WaZoo.                            */

static void  _NEAR_ LIBENTRY fill_in_struct(void)
{
NODE_REC    noderec;
char        buffer[80];
bool        useEMSIdata = FALSE;

  memset(&noderec,0,sizeof(NODE_REC));
  memset(buffer  ,0,sizeof(buffer));

  if(read_fido_config(ADDR_RECS) == FALSE) return;

  if(get_node(address[0].zone,address[0].net,address[0].node,&noderec)!=TRUE)
  {
      if(PcbData.FidoLogLevel == 2 || PcbData.FidoLogLevel >= 3)
      {
        sprintf(buffer,"Could not locate your address in nodelist. Using EMSI profile data.");
        writeFidolog(buffer,BLOCK);
      }
      useEMSIdata = TRUE;
  }
  else
  {
    stripright(noderec.BBS_name,' ');
    stripright(noderec.Sysop_name,' ');
  }

  OurHello.signal='o';
  OurHello.hello_version=1;
  OurHello.product=W_PRODUCT_CODE;
  OurHello.product_maj=atoi(VERSION_MAJOR);
  OurHello.product_min=atoi(VERSION_MINOR);

  // If my address is not in the nodelist, then use the info from the EMSI_DATA
  if(useEMSIdata)
  {
     maxstrcpy(OurHello.bbs_name,emsi_data.BBS_Name,sizeof(OurHello.bbs_name));
     maxstrcpy(OurHello.sysop,emsi_data.Sysop,sizeof(OurHello.sysop));
  }
  else
  {
     maxstrcpy(OurHello.bbs_name,noderec.BBS_name,sizeof(OurHello.bbs_name));
     maxstrcpy(OurHello.sysop,noderec.Sysop_name,sizeof(OurHello.sysop));
  }
  OurHello.my_zone=address[0].zone;
  OurHello.my_net=address[0].net;
  OurHello.my_node=address[0].node;
  OurHello.my_point=address[0].point;
  if(OurHello.my_password[0] == '\x0') maxstrcpy(OurHello.my_password," ",sizeof(OurHello.my_password));
  memset(OurHello.reserved2,0,8);
  OurHello.capabilities=ZED_ZIPPER | WZ_FREQ;
  //if(INBOUND) OurHello.capabilities |=WZ_FREQ;
  memset(OurHello.reserved3,0,12);
  free_fido_memory();
}

/****************************************************************************/
/* Convert the required fields from the hello packet to the Emsi structure  */
/* So the display_fido_info() function will work.                           */

static void _NEAR_ LIBENTRY convert_structure(void)
{

  if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3) writeFidolog("Converting structure",BLOCK);
  if (RemoteHello.my_zone == 0)
  {
    if (RemoteHello.my_point == 0)
      sprintf(emsi.addr,"%d/%d",RemoteHello.my_net,RemoteHello.my_node);
    else
      sprintf(emsi.addr,"%d/%d.%d",RemoteHello.my_net,RemoteHello.my_node,RemoteHello.my_point);
  }
  else
  {
    if (RemoteHello.my_point == 0)
      sprintf(emsi.addr,"%d:%d/%d",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node);
    else
      sprintf(emsi.addr,"%d:%d/%d.%d",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node,RemoteHello.my_point);
  }

  if(get_product_name(RemoteHello.product,emsi.prodname)==FALSE)
    sprintf(emsi.prodname,"Unknown code: %-1.35X",RemoteHello.product);

  sprintf(emsi.prodver,"%-1.4d.%-1.4d",RemoteHello.product_maj,RemoteHello.product_min);
  maxstrcpy(emsi.name,RemoteHello.bbs_name,sizeof(emsi.name));
  maxstrcpy(emsi.sysop,RemoteHello.sysop,sizeof(emsi.sysop));
  memcpy(emsi.pw,RemoteHello.my_password,8);
}

/****************************************************************************/
/* Get the name of the product that goes with the FTSC product code.        */

bool LIBENTRY get_product_name(int code,char *name)
{
DOSFILE   prodfile;
int       i=0;
char      *c=NULL,buffer[100];
char      prodFilePath[100];

  memset(buffer,0,sizeof(buffer));

  maxstrcpy(prodFilePath,PROD_FILE,sizeof(prodFilePath));

  srchpath(prodFilePath);

  //if(dosfopen(PROD_FILE,OPEN_READ|OPEN_DENYWRIT,&prodfile)==-1)
  if(dosfopen(prodFilePath,OPEN_READ|OPEN_DENYWRIT,&prodfile)==-1)
    return(FALSE);

  while(1)
    {
      if(read_line(&prodfile,buffer,sizeof(buffer))==-1)
        {
          dosfclose(&prodfile);
          return(FALSE);
        }

      if(buffer[0]!=';')
        {
          c=strtok(buffer,",");
          if(c) i=iHexDec(c);

          if(code==i)
            {
              c=strtok(NULL,",");

              if(c==NULL)
              {
                dosfclose(&prodfile);
                return(FALSE);
              }
              else
                maxstrcpy(name,c,50);

              dosfclose(&prodfile);
              return(TRUE);
            }
        }
    }
}

/****************************************************************************/
/* Send our hello structure to the remote system.  Function returns wether  */
/* or not we were able to successfully send the hello structure.            */

bool LIBENTRY Send_Hello(void)
{
unsigned short      crc=0;
int                 incoming_char=0,retrycount=0;
bool                done=0,dropthrough=0;
unsigned char       highbyte=' ',lowbyte=' ';
char                *buffer=NULL;

  done=FALSE;
  retrycount=0;
  crc=0;
  buffer=(char *)&OurHello;
  crc=fido_updcrc(crc,(unsigned char *)buffer,HELLO_SIZE);
  highbyte=(unsigned char)(crc>>8);      /* get MSB of CRC */
  lowbyte=(unsigned char)(crc&0xff);     /* get LSB of CRC */
  settimer(11,TWO_MINUTES);
  while(!done&&cdstillup() && !timerexpired(11))
    {
      while(comminkey()!=-1 && !timerexpired(11));                   /* clear incoming buffer    */
      if(PcbData.FidoLogLevel >= 4) writeFidolog("Sending yoohoo header.",BLOCK);
      sendbyte(HEADER);
      sendstr((char *)&OurHello,HELLO_SIZE);    /* send hello packet    */
      if(PcbData.FidoLogLevel >= 4) writeFidolog("Sending yoohoo block.",BLOCK);
      sendbyte(highbyte);                       /* send MSB of CRC  */
      sendbyte(lowbyte);                        /* send LSB of CRC  */
      dropthrough=FALSE;
      while(!dropthrough  && cdstillup() && !timerexpired(11))   /* wait until we get something interesting  */
        {
          while((incoming_char=comminkey())==-1 && cdstillup() &&  gettimer(4)>0  && !timerexpired(11));
          if(incoming_char!=-1)
            {
              switch(incoming_char)
                {
                  case ACK:   done=TRUE;                /* is good! */
                  case '?':
                  case ENQ:   dropthrough=TRUE;         /* is bad - again */
                             break;
                  default :   /* keep watching */
                             break;
                }
            }
        }
      if(++retrycount==10)
        done=TRUE;
    }
  return(incoming_char==ACK);
}


/****************************************************************************/
/* Get a block of data in from the modem, calculating the CRC on the block  */
/* as we receive information.                                               */
/* Function returns whether or not we got a block of size blocksize from the*/
/* remote end.                                                              */
/*                                                                          */

bool LIBENTRY get_block(char *block,int blocksize)
{
int         size=0,incoming_char=0;
bool        haveheader,havejunk;
char        incomingBuffer[256];

  memset(incomingBuffer,0,sizeof(incomingBuffer));
  size=0;
  haveheader=FALSE;
  havejunk=FALSE;
  settimer(9,SIXTY_SECONDS);
  sendbyte(ENQ);
  //settimer(10,TWENTY_SECONDS);

  while(size < blocksize && !timerexpired(9) && cdstillup())
    {
      if((incoming_char=comminkey())!=-1)
        {
          if(haveheader)                    /* getting block */
            {
              *(incomingBuffer+size) = (char)incoming_char;
              if(++size >= blocksize) break;
              settimer(10,TEN_SECONDS);
            }
          else if(incoming_char==0x1f)      /* is it the header? */
            {
              haveheader=TRUE;
              settimer(10,TEN_SECONDS);
            }
          else if(!havejunk)                /* are we getting junk? */
            {
              settimer(10,TEN_SECONDS);
              havejunk=TRUE;
            }
          if(timerexpired(10))                  /* timeout */
            {
              size=0;
              haveheader=FALSE;
              havejunk=FALSE;
              sendbyte('?');
            }
            giveup();
        }
    }
  if(size == blocksize)
  {
    memcpy(block,incomingBuffer,blocksize);
    getFidoUserPassword(OurHello.my_password);
    logOurHello();
    logRemoteHello();
    return TRUE;
  }
  return FALSE;
}


/****************************************************************************/
/* Read in a 2-byte CRC value from the remote end.                          */
/*                                                                          */

unsigned short LIBENTRY get_crc(void)
{
bool            havehighbyte=FALSE;
int             inbyte=0;

union
{
    unsigned short crc;
    char crcbuf[2];
} ucrc;

  ucrc.crc = 0;
  settimer(9,TEN_SECONDS);
  havehighbyte=FALSE;

  while(!timerexpired(9) && cdstillup())
    {
      if((inbyte = comminkey()) != -1)
        {
          if(!havehighbyte)             /* store high byte  */
            {
              ucrc.crcbuf[1]=inbyte;
              havehighbyte=TRUE;
            }
          else                         // Store low byte
            {
              ucrc.crcbuf[0]=inbyte;
              return(ucrc.crc);
            }
        }
    }
    return(0);
}


/****************************************************************************/
/* Receive the Hello block from the remote system.  Function returns whether*/
/* or not we were able to get a correct block from the remote system.       */
/*                                                                          */

bool LIBENTRY Recv_Hello(void)
{
int             retries;
unsigned short  rec_crc,crc;
  openlog();
  retries=0;
  while(retries<10 && cdstillup())
    {
      tickdelay(THREE_SECONDS);
      if(PcbData.FidoLogLevel >= 4) writeFidolog("Getting hello block from remote.",BLOCK);
      if(get_block((char *)&RemoteHello,128))       /* get the block */
        {
          if(PcbData.FidoLogLevel >= 4) writeFidolog("Got Hello. Checking CRC.",BLOCK);
          rec_crc=get_crc();                        /* get remote CRC   */
          crc=fido_updcrc(0,(unsigned char *)&RemoteHello,sizeof(RemoteHello));
          if(rec_crc==crc)                          /* compare CRC's    */
            {
              if(PcbData.FidoLogLevel >= 4) writeFidolog("CRC passed.",BLOCK);
              sendbyte(ACK);
              Status.FoundFido = TRUE;
              return(TRUE);
            }
          else
            {
              if(PcbData.FidoLogLevel >= 4) writeFidolog("CRC failed.",BLOCK);
              sendbyte('?');
              retries++;
            }
        }
        retries++;
    }
  return(FALSE);
}

static void _NEAR_ LIBENTRY logOurHello(void)
{
  char logBuf[65];

    writeFidolog("Yoohoo handshake information :Local",BLOCK);
    sprintf(logBuf," BBS Name: %-1.50s",OurHello.bbs_name);
    writeFidolog(logBuf,BLOCK);
    sprintf(logBuf," Sysop Name: %-1.50s",OurHello.sysop);
    writeFidolog(logBuf,BLOCK);
    sprintf(logBuf," Password: %-1.8s",OurHello.my_password);
    writeFidolog(logBuf,BLOCK);
}

static void _NEAR_ LIBENTRY logRemoteHello(void)
{
  char logBuf[100];

    writeFidolog("Yoohoo handshake information :Remote",BLOCK);
    sprintf(logBuf," BBS Name: %-1.50s",RemoteHello.bbs_name);
    writeFidolog(logBuf,BLOCK);
    sprintf(logBuf," Sysop Name: %-1.50s",RemoteHello.sysop);
    writeFidolog(logBuf,BLOCK);
    sprintf(logBuf," Password: %-1.8s",RemoteHello.my_password);
    writeFidolog(logBuf,BLOCK);
}

static void _NEAR_ LIBENTRY getFidoUserPassword(char * password)
{
  char userName[100];
  char tmp[20];

    sprintf(userName,"~FIDO~%d:%d/%d",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node);
    if(RemoteHello.my_point != 0)
    {
        strcat(userName,".");
        strcat(userName,itoa(RemoteHello.my_point,tmp,10));
    }

    if(tempuseralloc(FALSE) == -1) return;
    long userNum = finduser(userName);
    if(userNum > 0 && gettemprecord(userNum) != -1)
    {
      memcpy(password,TempData->Password,8);
      stripright(password,' ');
    }
    tempuserdealloc();
}


#endif
