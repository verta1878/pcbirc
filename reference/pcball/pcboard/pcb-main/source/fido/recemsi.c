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
#include <dos.h>

#include <misc.h>
#include <dosfunc.h>
#include <newdata.h>
#include <pcboard.h>
#include <pcboard.ext>
#include <screen.h>
#include <users.h>

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include <tossmisc.h>
#include <tossmisc.h>

/* If we want to have MemCheck included, we need to have the environment	*/
/* variable DEBUG set.	Otherwise, MemCheck won't be compiled in.           */

#ifdef DEBUG
#include <memcheck.h>
#endif


/*****************************************************************************/
/* Global Variables 														 */

#ifdef TRAVIS
extern unsigned _stklen=8192;
#endif

static _FAR_ int					sendcount=0;
static _FAR_ Emsi					emsi;
//static _FAR_ THIS_ADDRESS 		  remote_address;

extern NADDRESS 			  *address;
extern unsigned int 		  num_akas;



bool attempt_emsi_handshake(void)
{
int 		screen;
const int	nodestrSize = 2048;
char		remote_nodestr[2048];
//char		  nodebuf[30];
char		logBuf[70];

  memset(remote_nodestr,0,nodestrSize);

  screen=memsavescreen();

  if(read_fido_config(ADDR_RECS|NODE_RECS)==FALSE)
	{
	  writeFidolog("Could not read config file. Aborting.",BLOCK);
	  resetuser();
	  return(FALSE);
	}

  cls();
  clsbox(19,16,61,21,0x11);
  box(19,16,61,21,0x1F,DOUBLE);
  fastprint(22,18,"Waiting for EMSI handshake from remote.",ScrnColors->BoxColor);
  if(!NoEmsi) sendstr("**EMSI_REQA77E",14);
  sendbyte(CR);
  writeFidolog("Beginning incoming mail transfer.",BLOCK_START);
  if(receive_emsi_handshake(remote_nodestr,nodestrSize,emsi)==TRUE)
	{
	  fastprint(22,18,"Received EMSI handshake from remote.   ",ScrnColors->BoxColor);
	  sprintf(logBuf,"Incoming connection to %s",emsi.name);
	  writeFidolog(logBuf,BLOCK);
	  fastprint(22,19,"Sending EMSI handshake to remote.     ",ScrnColors->BoxColor);
	  if(send_emsi_handshake(emsi,INBOUND)==TRUE)
		{

		  fastprint(22,19,"Sent EMSI handshake to remote.       ",ScrnColors->BoxColor);
		  TransferMessages(&sendcount,remote_nodestr,INBOUND,ZMODEM);
		  free_fido_memory();
		  memrestorescreen(screen,FREESCREEN);
		  resetuser();
		  return(TRUE);
		}
	}
  writeFidolog("Handshake Error! - EMSI Failure.",BLOCK);
  free_fido_memory();
  memrestorescreen(screen,FREESCREEN);
  resetuser();
  return(FALSE);
}

#ifdef RECEMSI

int wait_for_connect(void)
{
char		*tptr=NULL,*last=NULL,buffer[80];
bool		done=FALSE,ring_detected=FALSE;
int 		incoming=0,MaxLen=0;

  done=FALSE;
  ring_detected=FALSE;
  printf("waiting for call\n");
  MaxLen=80;
  memset(buffer,0,MaxLen);
  tptr=buffer;
  last=buffer+MaxLen-1;
  while(!done)
	{
	  if(kbdhit(NOBUFFER))
		{
		  if(getch()==ESC_KEY)
			{
			  printf("ESC pressed!  Aborting\n");
			  if(ring_detected)
				sendbyte(' ');
			  done=TRUE;
			  return(FALSE);
			}
		}
	  if((incoming=comminkey())!=-1)		/* check for incoming	*/
		{
		  if(incoming==13)
			printf("\r");
		  else if(incoming==10)
			printf("\n");
		  else
			printf("%c",incoming);
		  if(incoming!=LINE_FEED)			/* eat line feeds	*/
			{
			  *tptr=incoming;
			  if(tptr==last||*tptr==CARRIAGE_RETURN)	/* CR or last char? */
				{
				  if(strstr(buffer,"CONNECT"))
					{
					  done=TRUE;
					  settimer(4,FIVE_SECONDS);
					  while(!online()&&!timerexpired(4));
					  return(online());
					}
				  else if(strstr(buffer,"RING"))
					{
					  sendstr("ATA",3);
					  sendbyte(CARRIAGE_RETURN);
					  memset(buffer,0,79);
					  tptr=buffer;
					  ring_detected=TRUE;
					  settimer(4,THIRTY_SECONDS);
					}
				  else if(strstr(buffer,"NO CARRIER"))
					{
					  memset(buffer,0,79);
					  tptr=buffer;
					}
				  else
					{
					  tptr=buffer;
					  memset(buffer,0,MaxLen);
					}
				}
			  else
				tptr++;
			  assert(tptr >= buffer && tptr < (buffer + sizeof(buffer));
			}
		}
	  if(ring_detected)
		{
		  if(timerexpired(4))
			{
			  ring_detected=FALSE;
			  turnoffdtr();
			  settimer(4,TWO_SECONDS);
			  turnondtr();
			  memset(buffer,0,80);
			  tptr=buffer;
			}
		}
	}
  return(FALSE);
}

#endif

#endif
