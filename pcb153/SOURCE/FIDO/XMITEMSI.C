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
#include <dos.h>
#include <assert.h>

#include "misc.h"
#include "bug.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "screen.h"
#include "users.h"
#ifdef __OS2__
#include <sem.hpp>
#endif

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include "tossmisc.h"
#ifdef TOSSCLASS
#include "pcbtoss.hpp"
#endif
#include "times.h"

#include "fidoque.hpp"

#ifdef DEBUG
#include <memcheck.h>
#endif


extern			NADDRESS				*address;
extern _FAR_	DIRECTORIES 			directory_info;
extern _FAR_	EMSI_DATA				emsi_data;
extern			unsigned int			num_akas;
static			unsigned short			crcval;
static _FAR_	HELLO_T 				OurHello;			// Our hello structure
static _FAR_	HELLO_T 				RemoteHello;		// remote hello structure
static _FAR_	NODE_REC				noderec;
static			char					outbound_address[25];
static			bool					wrotebusy=FALSE;



static int	_NEAR_ LIBENTRY detect_handshake(void);
static int	_NEAR_ LIBENTRY WaZoo_Init(void);
static int	_NEAR_ LIBENTRY connect_to_system(char *phone_num);
static void _NEAR_ LIBENTRY display_wazoo(void);
static void _NEAR_ initstruct(Emsi &emsi);
static bool _NEAR_ LIBENTRY check_for_outbound_mail(void);
static bool _NEAR_ LIBENTRY Send_Hello1(void);
static bool _NEAR_ LIBENTRY get_block1(char *block,int blocksize);
static bool _NEAR_ LIBENTRY ReceiveHello(void);
static unsigned short _NEAR_ LIBENTRY get_crc1(void);

//cNEWQ  Q;


/****************************************************************************/
/* Send the Mail.															*/

void send_outbound_mail(char *addr, int howCalled)
{
uint			line_status=0/*,MaxLen=0*/,zone=0,net=0,node=0,point;
char			*tptr=NULL;
int 			sendcount=0;
int 			dialCount=0;
int 			screen;
long			recno;
char			nodebuf[30];
THIS_ADDRESS	remote_address;
char			logBuf[80];
char			buffer[80];
const int		nodestrSize = 2048;
char			remote_nodestr[2048];
Emsi			emsi;
cXLATEPHONE 	Phone;

  memset(remote_nodestr,0,nodestrSize);
  memset(buffer   ,0,sizeof(buffer));
  memset(nodebuf  ,0,sizeof(nodebuf));
  assert(stackavail() > 2000);

  screen=memsavescreen();

  init_users();

  openlog();

  {
  cNEWQ Q;
	if(howCalled != FROM_MENU) Q.scanQueue();
  }

  if(strcmpi(PcbData.ModemPort,"NONE")==0 || Asy.ComPortNumber==0)
  {

	 memrestorescreen(screen,FREESCREEN);
	 closecallerlog();
	 resetuser();
	 sprintf(logBuf,"Could not open modem port. Aborting!");
	 writeFidolog(logBuf,BLOCK);
	 fastprint(8,6,logBuf,0x0C);
	 tickdelay(THREESECONDS);
	 return;
  }

  if(addr==NULL)
  {
	 showmodemdata("CHECKING FOR MAIL TO PROCESS...");
	 if(check_for_outbound_mail()==FALSE)
	 {
		memrestorescreen(screen,FREESCREEN);
		closecallerlog();
		resetuser();
		return;
	 }
  }
  else
	 maxstrcpy(outbound_address,addr,sizeof(outbound_address));

  cls();

  if(read_fido_config(ADDR_RECS|NODE_RECS|XLAT_RECS)==FALSE)
  {
	 memrestorescreen(screen,FREESCREEN);
	 resetuser();
	 return;
  }

  maxstrcpy(nodebuf,outbound_address,sizeof(nodebuf)-1);
  fido_nodestr_to_int(nodebuf,zone,net,node,point);

  if(get_node(zone,net,node,&noderec)==FALSE)
  {
	 sprintf(logBuf,"Could not locate address: %u:%u/%u in nodelist. Aborting!",zone,net,node);
	 writeFidolog(logBuf,BLOCK);
	 fastprint(8,6,logBuf,0x0C);
	 tickdelay(THREESECONDS);
	 free_fido_memory();
	 resetuser();
	 memrestorescreen(screen,FREESCREEN);
	 return;
  }

  sprintf(buffer,"Beginning mail run with %s",noderec.BBS_name);
  writeFidolog(buffer,BLOCK_START);
  noderec.BBS_name[sizeof(noderec.BBS_name)-1]=NULL;
  noderec.location[sizeof(noderec.location)-1]=NULL;
  noderec.Sysop_name[sizeof(noderec.Sysop_name)-1]=NULL;
  noderec.phone[sizeof(noderec.phone)-1]=NULL;

  stripright(noderec.BBS_name,' ');
  stripright(noderec.location,' ');
  stripright(noderec.Sysop_name,' ');
  stripright(noderec.phone,' ');

  // If there is a phone number in the users record-dataphone field
  // Copy it into the noderec.phone string.

  memset(UsersData.Password,0,sizeof(UsersData.Password));
  make_emsi_dat(&emsi,UsersData.Password);
  if((recno=find_fido_rec(nodebuf,FALSE,FALSE,NULL))!=-1)
  {
	 get_fido_rec(recno);
	 if(UsersData.Password[0]!=NULL && UsersData.Password[0]!=' ')
		make_emsi_dat(&emsi,UsersData.Password);

	 if(UsersData.BusDataPhone[0] != NULL && UsersData.BusDataPhone[0] != ' ')
	 {
		writeFidolog("Overriding nodelist Phone with user record phone.",BLOCK);
		maxstrcpy(buffer,UsersData.BusDataPhone,sizeof(buffer));
		buffer[sizeof(UsersData.BusDataPhone)-1] = '\x0';
		stripright(buffer,' ');
		if(strlen(buffer) <= sizeof(noderec.phone))
		  strcpy(noderec.phone,buffer);

		//translate_phone(noderec.phone);
		Phone.findAndReplace(noderec.phone);
	 }
	 else
		//translate_phone(noderec.phone);
		Phone.findAndReplace(noderec.phone);
  }
  else
	//translate_phone(noderec.phone);
	Phone.findAndReplace(noderec.phone);

  //Draw dialing box
  clsbox(15,16,65,22,0x11);
  box(15,16,65,22,0x1F,DOUBLE);
  sprintf(logBuf,"Dialing: %s",noderec.phone);
  writeFidolog(logBuf,BLOCK);
  fastprintmove(40-(strlen(logBuf)/2),17,logBuf,ScrnColors->BoxColor);
  fastprintmove(31,21,"Press ESC to abort.",ScrnColors->BoxColor);

  // Init and send modedm dial string
  initcommwindow(HIDE);

  wrotebusy = FALSE;
  for(dialCount = 0;dialCount < PcbData.NumRedials; dialCount++)
  {
	char  modemStr[37];
	maxstrcpy(modemStr,PcbData.ModemInit,sizeof(modemStr)-1);
	addchar(modemStr,'\r');
	sendmodemwaitokay(modemStr,FALSE);
	if( dialCount >= 1) fastprintmove(33,20,"Redialing...   ",ScrnColors->BoxColor);
	tickdelay(ONESECOND);

	line_status=connect_to_system(noderec.phone);
	if(line_status==BUSY)
	{
	  tickdelay(THREESECONDS);
	  continue;
	}

	if(line_status == NO_CONNECTION)
	{
	cNEWQ Q;
	  Q.forEachMatch(noderec.zone,noderec.net,noderec.node,ADD1FAIL);
	  break;
	}

	if(line_status==USER_ABORT || (line_status != BUSY && line_status != CONNECTED)) break;
	if(line_status==CONNECTED)
	{
	  {
	  cNEWQ Q;
		Q.forEachMatch(noderec.zone,noderec.net,noderec.node,ADD1FAIL);
	  }
	  sprintf(logBuf,"Connected to %s",noderec.BBS_name);
	  writeFidolog(logBuf,BLOCK);
	  fastprintmove(40 - (strlen(logBuf)/2),18,logBuf,ScrnColors->BoxColor);
	  tickdelay(QUARTERSECOND);
	  sendbyte(CR);
	  tickdelay(QUARTERSECOND);
	  sendbyte(CR);
	  tickdelay(QUARTERSECOND);
	  sendbyte(CR);

	  memset(buffer,0,sizeof(buffer));
	  tptr=buffer;


	  switch(detect_handshake())
	  {
		case EMSI_CONNECT:
			   writeFidolog("EMSI detected.  Attempting Handshake.",BLOCK);
			   //tickdelay(ONESECOND);
			   clearinbuf();
			   sendstr("**EMSI_INQC816",14);
			   sendbyte(CR);

			   // Log EMSI packet
			   if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
			   {
				  writeFidolog("Sending EMSI packet",BLOCK);
				  logEMSI(emsi);
			   }
			   if(send_emsi_handshake(emsi,OUTBOUND)==TRUE)
			   {
				  if(cdstillup() == FALSE)
					 writeFidolog("Carrier dropped by remote system. Handshake will fail.",BLOCK);

				  if(receive_emsi_handshake(remote_nodestr,nodestrSize,emsi)==TRUE)
				  { if((tptr=strchr(remote_nodestr,' '))!=NULL) {
				  *tptr=NULL;
				  maxstrcpy(nodebuf,remote_nodestr,sizeof(nodebuf)-1);
				  *tptr=' '; } else
				  maxstrcpy(nodebuf,remote_nodestr,sizeof(nodebuf)-1);

					 fido_nodestr_to_int(nodebuf,
										 remote_address.this_zone,
										 remote_address.this_net,
										 remote_address.this_node,
										 remote_address.this_point);

					 writeFidolog("EMSI Handshake successful",BLOCK);
					 TransferMessages(&sendcount,remote_nodestr,OUTBOUND,ZMODEM);
					 // Limit Q's scope
					 {
					 cNEWQ Q;
					   Q.connect = TRUE;
					 }
					 free_fido_memory();
					 Disconnect();
					 memrestorescreen(screen,FREESCREEN);
					 resetuser();
					 return;
				  }
				  else
				  {
					 writeFidolog("send emsi handshake returned FALSE",BLOCK);
					 writeFidolog("EMSI Handshake failure!",BLOCK);
					 free_fido_memory();
					 Disconnect();
					 memrestorescreen(screen,FREESCREEN);
					 resetuser();
					 return;
				  }
			   }
			   else
			   {
					 writeFidolog("EMSI Handshake failure!",BLOCK);
					 memrestorescreen(screen,FREESCREEN);
					 free_fido_memory();
					 Disconnect();
					 closecallerlog();
					 resetuser();
					 return;
			   }

		case FIDO_CONNECT:	// Fallthough
		case WAZOO_CONNECT:
					 initstruct(emsi);
					 switch(WaZoo_Init())
					 {
					   case NOT_OPUS:
							 writeFidolog("Connection Failed!",BLOCK);
							 break;

					   case FIDO_SYS:
							 writeFidolog("Fido connection made!",BLOCK);
							 TransferMessages(&sendcount,outbound_address,OUTBOUND,XMODEM);
							 break;

					   case WAZOO_CONNECT:
							 writeFidolog("WaZoo connection made!",BLOCK);
							 display_wazoo();
							 TransferMessages(&sendcount,outbound_address,OUTBOUND,ZMODEM);
							 break;
					 }
					 free_fido_memory();
					 setcursor(CUR_NORMAL);
					 memrestorescreen(screen,FREESCREEN);
					 Disconnect();
					 closecallerlog();
					 resetuser();
					 return;

		}// Switch

		break; // Connected so break out of redial for loop

	}// If status == connected
  }// Dialcount for

  free_fido_memory();
  setcursor(CUR_NORMAL);
  memrestorescreen(screen,FREESCREEN);
  Disconnect();
  if(addr != NULL)
	sprintf(buffer,"Ending mail run with %s ",addr);
  else
	sprintf(buffer,"Ending mail run.");
  writeFidolog(buffer,BLOCK_END);
  closecallerlog();
  resetuser();
  return;
}

/*****************************************************************************/
/* Detect the handshake from the remote.									 */

static int _NEAR_ LIBENTRY detect_handshake(void) {
  int			  incoming=0,MaxLen=79;
  char			  *tptr=NULL,*last=NULL,buffer[500];
  bool			  look=FALSE;

  memset(buffer,0,sizeof(buffer));
  tptr=buffer;
  last=tptr+MaxLen;

  settimer(4,FIVESECONDS);
  settimer(9,ONEMINUTE);

  clearinbuf();
  if(!NoEmsi)
  {
	 sendstr("**EMSI_INQC816",14);
	 sendstr("**EMSI_INQC816",14);
  }

  if(!NoWazoo) sendbyte(YOOHOO);
  if(!NoFTS1)  sendbyte(TSYNCH);

  sendbyte(CR);

  do
  {
	if (kbdhit(NOBUFFER) == 0x1B) return(NOT_OPUS);
	#ifdef __OS2__
	if(InBytes == 0)
	{
	  WaitKeySem.waitforevent(10000);
	  WaitKeySem.reset();
	}
	#endif
	incoming = comminkey();
	if (incoming != -1) 	 /* check for incoming */
	{
	  if (incoming != 10)
	  {
		if (look)
		{
		  if ( (incoming == ENQ || incoming == NAK || incoming == 'C') && !NoWazoo)
			return(WAZOO_CONNECT);
		}

		*tptr = incoming;
		if (tptr == last || *tptr == CARRIAGE_RETURN)
		{
		  *tptr = NULL;
		  if (strstr(buffer,"**EMSI_REQA77E") != NULL && !NoEmsi)
			return(EMSI_CONNECT);
		  tptr = buffer;   // Dump this string, we don't need it anymore
		} else
		  tptr++;  // Got a char, but not the CR or full buffer
	  }
	}
	if (timerexpired(4))
	{
	 clearinbuf();
	  if(!NoEmsi)
	  {
		 sendstr("**EMSI_INQC816",14);
		 sendstr("**EMSI_INQC816",14);
	  }

	  if(!NoWazoo) sendbyte(YOOHOO);
	  if(!NoFTS1)  sendbyte(TSYNCH);

	  sendbyte(CR);
	  settimer(4,FIVESECONDS);
	  look=TRUE;
	}
	#ifndef __OS2__
	//giveup();
	#endif
  } while (cdstillup() && !timerexpired(9));

  return(NOTHING_THERE);
}

/*****************************************************************************/
/* Display the remote WaZoo system's information.                            */

static void _NEAR_ LIBENTRY display_wazoo(void)
{
Emsi	emsi;

  memset(&emsi,0,sizeof(emsi));

  if(RemoteHello.my_zone!=0 && RemoteHello.my_point!=0)
  {
	 sprintf(emsi.addr,"%u:%u/%u.%u",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node,RemoteHello.my_point);
	 assert(strlen(emsi.addr) < sizeof(emsi.addr));
  }
  else if(RemoteHello.my_zone!=0 && RemoteHello.my_point==0)
  {
	 sprintf(emsi.addr,"%u:%u/%u",RemoteHello.my_zone,RemoteHello.my_net,RemoteHello.my_node);
	 assert(strlen(emsi.addr) < sizeof(emsi.addr));
  }
  else if(RemoteHello.my_zone==0 && RemoteHello.my_point!=0)
  {
	 sprintf(emsi.addr,"%u/%u.%u",RemoteHello.my_net,RemoteHello.my_node,RemoteHello.my_point);
	 assert(strlen(emsi.addr) < sizeof(emsi.addr));
  }
  else if(RemoteHello.my_zone==0 && RemoteHello.my_point==0)
  {
	 sprintf(emsi.addr,"%u/%u",RemoteHello.my_net,RemoteHello.my_node);
	 assert(strlen(emsi.addr) < sizeof(emsi.addr));
  }

  if(get_product_name(RemoteHello.product,emsi.prodname)==FALSE)
  {
	 sprintf(emsi.prodname,"Unknown code: %X",RemoteHello.product);
	 assert(strlen(emsi.prodname) < sizeof(emsi.prodname));
  }

  sprintf(emsi.prodver,"%d.%d",RemoteHello.product_maj,RemoteHello.product_min);
  assert(strlen(emsi.prodver) < sizeof(emsi.prodver));
  maxstrcpy(emsi.name,RemoteHello.bbs_name,sizeof(emsi.name));
  maxstrcpy(emsi.sysop,RemoteHello.sysop,sizeof(emsi.sysop));
  display_fido_info(emsi);
}

/*****************************************************************************/
/* Check for outbound mail. If there is mail to send, return TRUE.			 */

static bool _NEAR_ LIBENTRY check_for_outbound_mail(void)
{
int 			queues	= 0;
int 			i;
char			*tptr;
static int		lastone = 0;
char			temp[100];
QUEUE_RECORD	q_rec;

  memset(&q_rec ,0,sizeof(QUEUE_RECORD));
  assert(stackavail() > 2000);

  // limit Q's scope
  {
  cNEWQ Q;
	if((queues=Q.getOutboundCount()) == 0) return(FALSE);
  }

  if(queues>0)
  {
	 // Limit Q's scope
	 {
	 cNEWQ Q;
	   if( (lastone = Q.getNextOutbound(q_rec,lastone+1)) ==-1)
		  if( (lastone = Q.getNextOutbound(q_rec,1)) == -1)  return(FALSE);
	 }

	 if(q_rec.flag&Q_FILEREQ)
	 {
		if(q_rec.flag&Q_FREQED)
		   return(FALSE);
	 }

	 if(!(q_rec.flag&Q_POLL) && !(q_rec.flag & Q_POLLED))
	 {
	 char naddr[25];

		for(i=0;i<num_akas && address != NULL;i++)
		{
		   if(address[i].point == 0) sprintf(naddr,"%u:%u/%u",address[i].zone,address[i].net,address[i].node);
		   else 					 sprintf(naddr,"%u:%u/%u.%u",address[i].zone,address[i].net,address[i].node,address[i].point);

		   if(strcmp(q_rec.nodestr,naddr)==0)
		   {
		   cNEWQ Q;
			  Q.removeEntry(q_rec.filename);
			  return(FALSE);
		   }
		}
		if(fileexist(q_rec.filename)!=255)
		{
		   maxstrcpy(outbound_address,q_rec.nodestr,sizeof(outbound_address)-1);
		   return(TRUE);
		}
		else
		{
		cNEWQ Q;
		   Q.removeEntry(q_rec.filename);
		   if((tptr=strrchr(q_rec.filename,'\\'))!=NULL)
		   {
			  tptr++;
			  sprintf(temp,"File: %s Not found. Removeing from queue.",tptr);
			  writeFidolog(temp,BLOCK);
		   }
		   return(FALSE);
		}
	 }
	 else
	 {
	  char naddr[25];

	   for(i=0;i<num_akas && address != NULL;i++)
	   {

		   if(address[i].point == 0) sprintf(naddr,"%u:%u/%u",address[i].zone,address[i].net,address[i].node);
		   else 					 sprintf(naddr,"%u:%u/%u",address[i].zone,address[i].net,address[i].node);

		   if(strcmp(q_rec.nodestr,naddr)==0)
		   {
		   cNEWQ Q;
			  Q.removePoll(q_rec.nodestr);
			  return(FALSE);
		   }
	   }
	   if(q_rec.flag & Q_POLLED)
		  return(FALSE);

	   maxstrcpy(outbound_address,q_rec.nodestr,sizeof(outbound_address)-1);
	   return(TRUE);
	 }
  }
  lastone = 0;
  return(FALSE);
}

/****************************************************************************/
/* Get the CRC. 															*/

static unsigned short _NEAR_ LIBENTRY get_crc1(void)
{
bool			done=FALSE,havehighbyte=FALSE;
int 			inbyte;

union
{
  short crc;
  char crcbuf[2];
}ucrc;

  memset(ucrc.crcbuf,0,sizeof(ucrc.crcbuf));
  assert(stackavail() > 2000);

  settimer(4,FIVESECONDS);
  done=FALSE;
  havehighbyte=FALSE;
  while(!done&&!timerexpired(4)&&cdstillup())
  {
	#ifdef __OS2__
	  if(InBytes == 0)
	  {
		WaitKeySem.waitforevent(10000);
		WaitKeySem.reset();
	  }
	#endif
	inbyte = comminkey();
	if(inbyte !=-1)
	{
	   if(!havehighbyte)			 /* store high byte  */
	   {
		  ucrc.crcbuf[1]=inbyte;
		  havehighbyte=TRUE;
	   }
	   else 						// Store low byte
	   {
		  ucrc.crcbuf[0]=inbyte;
		  done=TRUE;
	   }
	}
  }
  if(done)
	return(ucrc.crc);					   /* did we get it?   */
  else
	return(0);
}

/****************************************************************************/
/* Send hello packet for WaZoo. 											*/

static bool _NEAR_ LIBENTRY Send_Hello1(void)
{
unsigned short		crc= 0;
int 				incoming_char=0,retrycount=0;
bool				done=FALSE,dropthrough=FALSE;
unsigned char		highbyte=0,lowbyte=0;

  checkstack();
  settimer(4,FOURTY_SECONDS);

  crc=fido_updcrc(crc,(unsigned char *)&OurHello,sizeof(OurHello));
  highbyte=(unsigned char)(crc>>8); 	 /* get MSB of CRC */
  lowbyte=(unsigned char)(crc&0xff);	 /* get LSB of CRC */
  while(!done&&cdstillup())
  {
	 //clearinbuf();
	 sendbyte(HEADER);
	 sendstr((char *)&OurHello,HELLO_SIZE);    /* send hello packet    */
	 sendbyte(highbyte);					   /* send MSB of CRC  */
	 sendbyte(lowbyte); 					   /* send LSB of CRC  */
	 dropthrough=FALSE;
	 while(!dropthrough)   /* wait until we get something interesting  */
	 {

	  #ifdef __OS2__
	  if(InBytes == 0)
	  {
		WaitKeySem.waitforevent(10000);
		WaitKeySem.reset();
	  }
	  #endif
		while( (incoming_char = comminkey()) !=-1 && cdstillup())
		{
		   switch(incoming_char)
		   {
			   case ACK:  return(TRUE);
			   case '?':
			   case ENQ:   dropthrough=TRUE;		 /* is bad - again */
						   break;
			   default :   /* keep watching */
						   break;
		   }
		   if(timerexpired(4))
		   {
			  dropthrough=TRUE;
			  done=TRUE;
		   }
		}
		if(timerexpired(4))
		{
		   dropthrough=TRUE;
		   done=TRUE;
		}
	 }
	 if(++retrycount==10 || timerexpired(4)) done=TRUE;
  }
  return(incoming_char==ACK);
}

/****************************************************************************/
/* Get a block from the modem.												*/

static bool _NEAR_ LIBENTRY get_block1(char *block,int blocksize)
{
char			*tptr=NULL;
int 			size=0,incoming_char=0;
unsigned short	crc=0;
bool			haveheader=FALSE,havejunk=FALSE,done=FALSE;

  tptr=block;
  crc=0;
  size=0;
  haveheader=FALSE;
  havejunk=FALSE;
  done=FALSE;
  assert(block != NULL);
  assert(stackavail() > 2000);

  settimer(4,TWO_MINUTES);
  sendbyte(ENQ);
  while(size<blocksize&&!timerexpired(4)&&cdstillup()&&!done)
  {
	  #ifdef __OS2__
		if(InBytes == 0)
		{
		  WaitKeySem.waitforevent(10000);
		  WaitKeySem.reset();
		}
	  #endif
	  incoming_char = comminkey();

	 if(incoming_char!=-1)
	 {
		if(haveheader)					  /* getting block */
		{
		   *tptr=(char)incoming_char;
		   crc=fido_updcrc(crc,(unsigned char *)tptr,1);	/* update CRC on incoming block */
		   crcval=crc;
		   tptr++;
		   size++;
		   settimer(8,FIVESECONDS);
		}
		else if(incoming_char==0x1f)	  /* is it the header? */
		{
		   haveheader=TRUE;
		   settimer(4,THIRTY_SECONDS);
		   settimer(8,FIVESECONDS);
		   crc=0;
		   crcval=0;
		}
		else if(!havejunk)				  /* are we getting junk? */
		{
		   settimer(4,FIVESECONDS);
		   havejunk=TRUE;
		}
		if(timerexpired(8)) 				 /* timeout */
		{
		   tptr=block;
		   size=blocksize;
		   haveheader=FALSE;
		   havejunk=FALSE;
		   settimer(4,TWO_MINUTES);
		   sendbyte('?');
		}
	 }
   }
  return(size==blocksize);
}

/****************************************************************************/
/* Receive the hello packet.												*/

static bool _NEAR_ LIBENTRY ReceiveHello(void)
{
int 			retries=0;
unsigned short	rec_crc=0,crc=0;
char			logBuf[65];

  checkstack();
  retries=0;
  while(retries<10 && cdstillup())
  {
	 if(PcbData.FidoLogLevel >= 4) writeFidolog("Getting hello block from remote.",BLOCK);
	 if(get_block1((char *)&RemoteHello,128))		/* get the block */
	 {
		crc=0;
		if(PcbData.FidoLogLevel >= 4) writeFidolog("Got Hello. Checking CRC.",BLOCK);
		rec_crc=get_crc1(); 					   /* get remote CRC   */
		crc=fido_updcrc(crc,(unsigned char *)&RemoteHello,sizeof(RemoteHello));
		if(rec_crc==crc)						  /* compare CRC's    */
		{
		   if(PcbData.FidoLogLevel >= 4) writeFidolog("CRC passed.",BLOCK);
		   sendbyte(ACK);
		   sendbyte(YOOHOO);
		   return(TRUE);
		}
		else
		{
		   if(PcbData.FidoLogLevel >= 4)
		   {
			 sprintf(logBuf,"CRC Failed. Got %u, calculated %u.",rec_crc,crc);
			 writeFidolog(logBuf,BLOCK);
		   }
		   sendbyte('?');
		   retries++;
		}
	 }
  }
  return(FALSE);
}

/*****************************************************************************/
/* Init WaZoo transfer. 													 */

static int _NEAR_ LIBENTRY WaZoo_Init(void)
{
int 		inchar=0,state=0;

  assert(stackavail() > 2000);
  state=0;
  while(cdstillup())
  {
	 if(kbdhit(NOBUFFER) == 0x1B) return(NOT_OPUS);

	  switch(state)
	  {
		  case 0:	state=2;
					break;

		  case 2:	settimer(9,TWOMINUTES); 	 /* initialize timers	 */
					state=3;
					break;
		  case 3:	settimer(10,FIVESECONDS);
					state=4;
					break;
		  case 4:
					//clearinbuf();
					if(!NoWazoo) sendbyte(YOOHOO);
					if(!NoFTS1)  sendbyte(TSYNCH);
					sendbyte(CR);
					settimer(8,FIVESECONDS);

					#ifdef __OS2__
					if(InBytes == 0)
					{
					  WaitKeySem.waitforevent(10000);
					  WaitKeySem.reset();
					}
					#endif

					while( (inchar = comminkey() ) == -1 && cdstillup())
					   giveup();

					switch(inchar)		/* look for a reply */
					{
						case ENQ:	 state=8;
									 break;

						case 'C':
						case NAK:	 return(FIDO_SYS);

						case 0x01:	 state=7;
									 break;

						default:	 state=4;
									 break;
					}
					break;

		  case 7:	settimer(8,THREE_SECONDS);

					#ifdef __OS2__
					if(InBytes == 0)
					{
					  WaitKeySem.waitforevent(10000);
					  WaitKeySem.reset();
					}
					#endif

					while( (inchar = comminkey()) == -1 && !timerexpired(8)&&!timerexpired(9) && cdstillup())
						 giveup();

					switch(inchar)		/* Fido system? */
					{
						case ENQ:	 state=8;
									 break;

						case 'C':    state=4;
									 break;

						case NAK:	 state=4;
									 break;

						case 0x01:	 state=7;
									 break;

						case 0xFE:	 return(FIDO_SYS);

						default:	 settimer(8,THREE_SECONDS);
									 break;
					}
					break;

		  case 8:	if(Send_Hello1())		 /* Attempt WaZoo connect */
					  state=9;
					else
					  state=2;
					break;

		  case 9:	settimer(8,FIVE_SECONDS);

					#ifdef __OS2__
					if(InBytes == 0)
					{
					  WaitKeySem.waitforevent(10000);
					  WaitKeySem.reset();
					}
					#endif

					while( (inchar = comminkey() ) ==-1 && !timerexpired(8)&&!timerexpired(9))
					  giveup();

					switch(inchar)			/* did we do good?	*/
					{
						case YOOHOO:	state=10;
										break;
						default:		state=2;
										break;
					}
					break;
		  case 10:	if(ReceiveHello())			/* get remote struct	*/
					  return(WAZOO_CONNECT);
					else
					  state=2;
					break;
		}
	}
  return(NOT_OPUS);
}

/****************************************************************************/
/* Init the structure for WaZoo trnasfers.									*/

static void _NEAR_ initstruct(Emsi &emsi)
{
char		  nodebuf[30];
uint  zone;
uint  net;
uint  node;
uint  point;
int 		  i;
int 		  matched_aka;

  maxstrcpy(nodebuf,outbound_address,sizeof(nodebuf));
  fido_nodestr_to_int(nodebuf,zone,net,node,point);

  for(i=0,matched_aka=0;i<num_akas;i++)
  {
	 if(address[i].zone==zone) matched_aka=i;

	 if(address[i].zone==zone && address[i].net==net)
	 {
		matched_aka=i;
		break;
	 }
  }

  memset(&OurHello,0,sizeof(HELLO_T));			/* zero everything out */
  OurHello.signal='o';
  OurHello.hello_version=1;
  OurHello.product=W_PRODUCT_CODE;
  OurHello.product_maj=atoi(VERSION_MAJOR);
  OurHello.product_min=atoi(VERSION_MINOR);
  maxstrcpy(OurHello.bbs_name,emsi_data.BBS_Name,sizeof(OurHello.bbs_name));
  maxstrcpy(OurHello.sysop,emsi_data.Sysop,sizeof(OurHello.sysop));
  OurHello.my_zone=address[matched_aka].zone;
  OurHello.my_net=address[matched_aka].net;
  OurHello.my_node=address[matched_aka].node;
  OurHello.my_point=address[matched_aka].point;
  memcpy(OurHello.my_password,emsi.pw,sizeof(OurHello.my_password));
  memset(OurHello.reserved2,0,sizeof(OurHello.reserved2));
  OurHello.capabilities=ZED_ZIPPER | ZED_ZAPPER | WZ_FREQ | Y_DIETIFNA;
  memset(OurHello.reserved3,0,sizeof(OurHello.reserved3));
}

/*****************************************************************************/
/* Call phone number and connect to the system. Return what happened.		 */

static int _NEAR_ LIBENTRY connect_to_system(char *phone_num)
{
  char		  *tptr=NULL,*last=NULL;
  int		  incoming=0,MaxLen=0,line_status=0;
  char		  buffer[80];
  char		  string[100];

  memset(buffer,0,sizeof(buffer));
  assert(phone_num != NULL);
  assert(stackavail() > 2000);

  MaxLen=sizeof(buffer)-1;
  tptr=buffer;
  last=tptr+MaxLen;
  settimer(9,TWOMINUTES);
  clearinbuf();

  // If there is a string in the setup, build it else use ATDT
  buildstr(string,PcbData.ModemDial,phone_num,"\r",NULL);
  initcommwindow(HIDE);
  slowsendtomodem(string);


  while(!timerexpired(9) )
  {
	 if(kbdhit(NOBUFFER) == ESC_KEY)
	 {
		Disconnect();
		line_status=USER_ABORT;
		break;
	 }

	  #ifdef __OS2__
	  if(InBytes == 0)
	  {
		WaitKeySem.waitforevent(10000);
		WaitKeySem.reset();
	  }
	  #endif
	  incoming = comminkey();

	 if(incoming!=-1)		 /* check for incoming	 */
	 {
		if(incoming!=10)				  /* eat line feeds   */
		{
		   assert(tptr >= buffer && tptr < (buffer + sizeof(buffer)));
		   *tptr=incoming;
		   if(tptr==last||*tptr==CARRIAGE_RETURN)	 /* CR or last char? */
		   {
			  *tptr='\0';
			  if (strstr(buffer,"CONNECT") != NULL)
			  {
				  fastprintmove(40-(strlen(buffer)/2),19,buffer,ScrnColors->BoxColor);
				  settimer(10,FIVE_SECONDS);		 /* check for connect */
				  writeFidolog(buffer,BLOCK);
				  while(!online()&&!timerexpired(10));
				  if (online())
				  {
					  CDokay = 128;
					  line_status=CONNECTED;
					  processconnectstring(buffer);
				  }
				  else
					  line_status=NO_CONNECTION;
				  break;
			  }
			  else if (strstr(buffer,"BUSY") != NULL)
			  { 							  /* check for busy */
				  fastprintmove(33,20,"Line is Busy!",ScrnColors->BoxColor);

				  if(!wrotebusy)
				  {
					writeFidolog("Line was busy.",BLOCK);
					wrotebusy = TRUE;
				  }

				  tickdelay(ONESECOND);
				  line_status=BUSY;
				  break;
			  }
			  else if (strstr(buffer,"NO CARRIER") != NULL || strstr(buffer,"NO DIAL") != NULL)
			  {
				  fastprintmove(33,20,"NO CONNECTION",ScrnColors->BoxColor);
				  writeFidolog("No Connection.",BLOCK);
				  line_status=NO_CONNECTION;	/* check for error */
				  break;
			  }
			  else
			  {
				  tptr=buffer;
				  memset(buffer,0,MaxLen);		/* otherwise keep it */
			  }
		   }
		   else
			  tptr++;
		   assert(tptr >= buffer && tptr < (buffer + sizeof(buffer)));
		}
	 }
	 #ifndef __OS2__
	 else giveup();
	 #endif
  }
  if(line_status==CONNECTED && !online())
	line_status=NO_CONNECTION;
  return(line_status);
}


void logEMSI(Emsi & info)
{

char			logBuf[100];

	sprintf(logBuf,"Address: %-1.50s",info.addr);
	writeFidolog(logBuf,BLOCK);

	sprintf(logBuf,"Sysop: %-1.50s",info.sysop);
	writeFidolog(logBuf,BLOCK);

	sprintf(logBuf,"City: %-1.50s",info.city);
	writeFidolog(logBuf,BLOCK);

	sprintf(logBuf,"Phone: %-1.50s",info.phone);
	writeFidolog(logBuf,BLOCK);

	if(PcbData.FidoLogLevel >= 3)
	{
		sprintf(logBuf,"Password: %-1.50s",info.pw);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Line: %-1.50s",info.line);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Codes: %-1.50s",info.codes);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Prodcode: %-1.50s",info.prodcode);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Prodname: %-1.50s",info.prodname);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Prodver: %-1.50s",info.prodver);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Prodserial: %-1.50s",info.prodserial);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Name: %-1.50s",info.name);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"baud: %-1.50s",info.baud);
		writeFidolog(logBuf,BLOCK);
		sprintf(logBuf,"Flags: %-1.50s",info.flags);
		writeFidolog(logBuf,BLOCK);
	}
}

#endif
