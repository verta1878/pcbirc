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
#include <mem.h>
#include <stdio.h>
#include <screen.h>
#include <ctype.h>
#include <dos.h>

#include "misc.h"
#include "bug.h"
#include "pcb.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "ansi.h"

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include "tossmisc.h"
#include "times.h"
#include "fidoque.hpp"


#ifdef DEBUG
#include <memcheck.h>
#endif

extern struct ffblk DTA;

/******************************************************************************/
/*																			  */
/*								  NEWQ.CPP									  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*					QUEUE Class written for FIDO queue functionality		  */
/*																			  */
/*============================================================================*/
/*																			  */
/*								  Written by								  */
/*								 Stan Paulsen								  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*				   Copyright (C) 1994 - Clark Development Company, Inc. 	  */
/*																			  */
/******************************************************************************/


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: Constructor														  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Initialize the cNEWQ class object.								  */
/*																			  */
/*============================================================================*/
/* Parameters: None 														  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : Uninitialized cNEWQ object								  */
/*																			  */
/* Post Conditions: Initialized   cNEWQ object								  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Special Notes: Considering making the Qfile DOSFILE structure static 	  */
/* to allow multiple instances working on the same file.					  */
/*																			  */
/******************************************************************************/


bool   cNEWQ::connect	= FALSE;

cNEWQ::~cNEWQ(void)
{

}

void cNEWQ::scanQueue(void)
{
QUEUE_RECORD rec,mtrec;
long allowFlags;
char logBuf[65];


  checkstack();
  memset(&mtrec,0,sizeof(mtrec));
  QSize = totRecs();

	//get_event_info(" ");
	if(!PcbData.FidoDialOut) return;

	// Limit the scope of cSEND
	{
	  cSEND s;
	  s.process();
	}
	for(int i = 1;i<=QSize;i++)
	{
		rec = getRec(i);
		if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;

		if( !(rec.flag&Q_POLL) && !(rec.flag&Q_POLLED) && !(rec.flag&Q_FILEREQ) && !(rec.flag&Q_FREQED) && fileexist(rec.filename) == 255)
		{
		   removeEntry(rec.filename);
		   continue;
		}

		if(rec.readOnly) continue;

		allowFlags = get_event_info(rec.nodestr);

		// if the record is a completed poll and no POLL is active for this poll or there are no events active and the poll is completed, remopve the poll
		if( (rec.flag & Q_POLLED || rec.flag & Q_POLL) && !(allowFlags & Q_POLL) )
		{
			 removePoll(rec.nodestr);
			 continue;
		}


		if( ((rec.flag & Q_POLL) || (rec.flag & Q_POLLED)) && allowFlags == 0)
		{
			removePoll(rec.nodestr);
			continue;
		}

		// if the record is a completed FREQ and no FREQ is active for this poll or there are no events active and the poll is completed, remopve the poll
		if( (rec.flag & Q_FREQED || rec.flag & Q_FILEREQ) && !(allowFlags & Q_FILEREQ) )
		{
			 char * file = rec.filename;
			 if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel  >= 3)
			 {
				sprintf(logBuf,"removing FREQ to %s.",rec.nodestr);
				writeFidolog(logBuf,BLOCK_END);
			 }

			 removeFREQ(file);
			 continue;
		}

		// If the file exists, but it is 0 bytes, remove the entry.
		if(fileexist(rec.filename) != 255 && DTA.ff_fsize == 0) removeEntry(rec.filename);

		if(rec.flag & Q_POLL && allowFlags & Q_POLL)
		  continue;
		if(rec.flag & Q_POLLED && allowFlags & Q_POLL)
		  continue;

		// $$$ If it is already hold then don't modify it
		if( (rec.flag & Q_HOLD) && (allowFlags&Q_HOLD) || (rec.failedConnects >= PcbData.MaxTries))
		{
		   rec.flag |= Q_HOLD;
		   rec.flag &= ~Q_OUTBOUND;
		   rec.flag &= ~Q_CRASH;
		   rec.flag &= ~Q_NORMAL;

		   // Attempt to modify record and report results
		   if( !modifyRec(rec,i) )
		   {
			 tickdelay(TWOSECONDS);
			 if( !modifyRec(rec,i) )
			 {
			   sprintf(logBuf,"Couldn't modify rec in queue. #%u To: %s File: %s",i,rec.nodestr,findstartofname(rec.filename));
			   writeFidolog(logBuf,BLOCK);
			 }
		   }
		   else if(PcbData.FidoLogLevel >= 4)
		   {
			   sprintf(logBuf,"Modified rec in queue. #%u To: %s File: %s",i,rec.nodestr,findstartofname(rec.filename));
			   writeFidolog(logBuf,BLOCK);
		   }
		   continue;
		}
		// If this is a poll record, or a crash is defined or it's a crash record and not a hold record  and not a finished poll
		if( ((rec.flag&Q_FILEREQ) || (rec.flag&Q_POLL) || (allowFlags&Q_CRASH) || (rec.flag & Q_CRASH)) && (!(rec.flag & Q_POLLED)))
		{
		   if(rec.flag & Q_POLLED) continue;
		   rec.flag |= Q_OUTBOUND;
		   rec.flag |= Q_CRASH;
		   rec.flag &= ~Q_HOLD;
		   rec.flag &= ~Q_NORMAL;

		}
		if( (rec.flag & Q_OUTBOUND || rec.flag & Q_CRASH) && ! (allowFlags & Q_CRASH) && ! (rec.flag & Q_POLL) && !(allowFlags&Q_FILEREQ))
		{
		   rec.flag &= ~Q_OUTBOUND;
		   rec.flag &= ~Q_CRASH;
		   rec.flag |=	Q_NORMAL;
		}

		modifyRec(rec,i);
	}
}
/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: findRec virtual inherited										  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: find a record in the queue based on address 					  */
/*																			  */
/*============================================================================*/
/* Parameters: str: address string to search for							  */
/* Return Val: record number												  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : Initialized queue record ready to be added				  */
/*																			  */
/* Post Conditions: Queue record added or failure reported					  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes:															  */
/*																			  */
/*																			  */
/******************************************************************************/
unsigned int cNEWQ::findRec(const char * str)
{
QUEUE_RECORD rec;
unsigned int i,trecs = totRecs();

   for(i=1;i<=trecs;i++)
   {
	 rec = getRec(i);
	 if(strcmp(rec.nodestr,str)==0) return i;
   }
   return 0;
}



/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: addEntry 														  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Add a new record to the queue file								  */
/*																			  */
/*============================================================================*/
/* Parameters: a QUEUE_RECORD structure passed by reference 				  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : Initialized queue record ready to be added				  */
/*																			  */
/* Post Conditions: Queue record added or failure reported					  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes:															  */
/*																			  */
/*																			  */
/******************************************************************************/
bool cNEWQ::addEntry(QUEUE_RECORD &  rec)
{
QUEUE_RECORD trec;
unsigned int trecnum;
char		 logBuf[100];

  unsigned long allowFlags;

	checkstack();

	if(isDupe(rec)) return FALSE;

	// See if this record needs to be flagged as immediate outbound
	if(!(rec.flag & Q_POLL) && !(rec.flag & Q_FILEREQ))
	{

	  //Make sure the file exists before adding to queue (POLLS and FREQS don't apply)
	  if(fileexist(rec.filename) == 255) return FALSE;

	  allowFlags = get_event_info(rec.nodestr);
	  if((allowFlags&Q_POLL || allowFlags&CRASH || allowFlags&Q_CRASH) && !(allowFlags&Q_HOLD))
		rec.flag |= Q_OUTBOUND;
	  else if(allowFlags&Q_HOLD && !rec.readOnly)
	  {
		rec.flag &= ~Q_OUTBOUND;
		rec.flag &= ~Q_CRASH;
		rec.flag &= ~Q_NORMAL;
		rec.flag |= Q_HOLD;
	  }

	}
	else
	  rec.flag |= Q_OUTBOUND;

	// If there running a network, try to lock the queue file.
	if(PcbData.Network)
	{
	}

	if(rec.flag & Q_POLL && (PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3))
	{
		char buf[50];
		sprintf(buf,"Adding POLL to %-1.48s to queue.",rec.nodestr);
		writeFidolog(buf,BLOCK_END);
	}

	trecnum = findRec(rec.nodestr);
	if(trecnum > 0)
	{
	  trec = getRec(trecnum);
	  rec.failedConnects = trec.failedConnects;
	}

	// Add record to queue, report results
	if( !addRec(rec) )
	{
	  tickdelay(TWOSECONDS);
	  if( !addRec(rec) )
	  {
		sprintf(logBuf,"Couldn't add rec to queue. To: %s File: %s",rec.nodestr,findstartofname(rec.filename));
		writeFidolog(logBuf,BLOCK);
	  }
	}
	else if(PcbData.FidoLogLevel >= 4)
	{
		sprintf(logBuf,"Added rec to queue. To: %s File: %s",rec.nodestr,findstartofname(rec.filename));
		writeFidolog(logBuf,BLOCK);
	}

	return TRUE;
}
/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: isDupe	(Virtual)												  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Check for duplicate Records in queue							  */
/*																			  */
/*============================================================================*/
/* Parameters: rec: record to chack against 								  */
/* Return Val: Dupe status													  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : Queue entry with slot marked as being used				  */
/*																			  */
/* Post Conditions: Slot marled as unused and first byte in record = NULL	  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: Marking the first byte as NULL is redundant.				  */
/*																			  */
/*																			  */
/******************************************************************************/

bool cNEWQ::isDupe(QUEUE_RECORD & rec)
{
unsigned int trecs = totRecs();
unsigned int i;
QUEUE_RECORD trec;

  for(i=1;i<= trecs;i++)
  {
	trec = getRec(i);
	if(strcmp(rec.filename,trec.filename) == 0 &&
	   strcmp(rec.nodestr,trec.nodestr)   == 0)  return TRUE;
  }
  return FALSE;
}


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: removeEntry (overloaded) 										  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Remove a record from the queue file 							  */
/*																			  */
/*============================================================================*/
/* Parameters: Record number, or slot to mark as being empty				  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : Queue entry with slot marked as being used				  */
/*																			  */
/* Post Conditions: Slot marled as unused and first byte in record = NULL	  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: Marking the first byte as NULL is redundant.				  */
/*																			  */
/*																			  */
/******************************************************************************/
bool cNEWQ::removeEntry(int slotNum)
{

	char logBuf[65];

	QUEUE_RECORD rec;

	checkstack();
	QSize = totRecs();
	if(slotNum < 1 || slotNum > QSize )
	{
	   packFile();
	   return FALSE;
	}


	// Set that bit to 0
	// lock header **A must!**
	//if(PcbData.Network)
	//	Qfile.lockchk(sizeof(QUEUE_HEADER)-1,1);

	rec = getRec(slotNum);

	// If the file is marked as Q_KILLSENT, delete it
	if(rec.flag & Q_KILLSENT)
	{
	  strupr(rec.filename);
	  // Is it a echo mail packet? If so truncate to 0 bytes.
	  if(strstr(rec.filename,".MO") ||
		 strstr(rec.filename,".TU") ||
		 strstr(rec.filename,".WE") ||
		 strstr(rec.filename,".TH") ||
		 strstr(rec.filename,".FR") ||
		 strstr(rec.filename,".SA") ||
		 strstr(rec.filename,".SU"))
	  {
		cDOSFILE tmp;
		 if(tmp.openChk(rec.filename,OPEN_RDWR | OPEN_DENYRDWR) == 0)
		 {
		   tmp.trunc(0);
		   tmp.close();
		   sprintf(logBuf,"File %s truncated for %s",findstartofname(rec.filename),rec.nodestr);
		 }
		 else
		   sprintf(logBuf,"Unable to open and truncate File %s.",findstartofname(rec.filename));

		 if(PcbData.FidoLogLevel >= 4) writeFidolog(logBuf,BLOCK);
	  }
	  else
	  {
		 // If another queue entry needs to send this file, don't delete it
		 if( checkForFiles(rec.filename) == FALSE)
		 {
		   unlink(rec.filename);

		   if(fileexist(rec.filename) != 255)
		   {
				sprintf(logBuf,"File %s cannot be deleted for %s",findstartofname(rec.filename),rec.nodestr);
				writeFidolog(logBuf,BLOCK);
		   }
		   else
		   {
			  sprintf(logBuf,"File %s Deleted for %s.",findstartofname(rec.filename),rec.nodestr);
			  writeFidolog(logBuf,BLOCK);
		   }
		 }
	  }

	}
	else
	if( (rec.flag & Q_POLL) || (rec.flag & Q_POLLED) )
	{
		long allowFlags = get_event_info(rec.nodestr);
		if( (allowFlags & Q_POLL) ) return FALSE;
		if(PcbData.FidoLogLevel ==1 || PcbData.FidoLogLevel >= 3)
		{
		  sprintf(logBuf,"Removing POLL to %s.",rec.nodestr);
		  writeFidolog(logBuf,BLOCK_END);
		}
	}

	// Attempt to remove file and report results
	if( !removeRec(slotNum) )
	{
	  tickdelay(TWOSECONDS);	 // Retry in 2 seconds
	  if( !removeRec(slotNum) )
	  {
		sprintf(logBuf,"Couldn't remove rec from queue. #%u To: %s File: %s",slotNum,rec.nodestr,findstartofname(rec.filename));
		writeFidolog(logBuf,BLOCK);
	  }
	}
	else if(PcbData.FidoLogLevel >= 4)
	{
	   sprintf(logBuf,"Removed from queue #%u To: %s File: %s",slotNum,rec.nodestr,findstartofname(rec.filename));
	   writeFidolog(logBuf,BLOCK);
	}
	packFile();
	return FALSE;
}


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: removeEntry	(overloaded)										  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Remove a record from the queue file 							  */
/*																			  */
/*============================================================================*/
/* Parameters: File string accociated with a queue entry					  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : Queue entry with slot marked as being used				  */
/*																			  */
/* Post Conditions: Slot marled as unused and first byte in record = NULL	  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: Marking the first byte as NULL is redundant.				  */
/*																			  */
/*																			  */
/******************************************************************************/
bool cNEWQ::removeEntry(char * file)
{
  checkstack();
  if(strstr(file,".REQ")!=NULL)
	return(removeFREQ(file));
  return removeEntry(getRecordNumber(file,1));
}

/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: addPoll															  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Add a poll entry to the queue file								  */
/*																			  */
/*============================================================================*/
/* Parameters: Node string correspondin to the node to be polled			  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An existing queue file									  */
/*																			  */
/* Post Conditions: A poll record added or failure reported 				  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: POLLS are only unique by node string. 					  */
/*																			  */
/*																			  */
/******************************************************************************/
bool cNEWQ::addPoll(char * node,bool readStatus)
{
	QUEUE_RECORD rec;
	checkstack();
	memset(&rec,0,sizeof(rec));
	maxstrcpy(rec.filename,"POLL",sizeof(rec.filename));
	maxstrcpy(rec.nodestr,node,sizeof(rec.nodestr));
	rec.readOnly = readStatus;
	rec.flag=Q_POLL|Q_CRASH|Q_OUTBOUND;
	return (addEntry(rec));
}


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: removePoll														  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Remove a poll record from queue file							  */
/*																			  */
/*============================================================================*/
/* Parameters: Node string correspondin to the node to be polled			  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An existing queue file									  */
/*																			  */
/* Post Conditions: A poll record added or failure reported 				  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: POLLS should not be removed unless there is no event		  */
/* defined for that poll. If a poll event is active and the poll has been	  */
/* completed, it should be marked as 'done'.                                  */
/******************************************************************************/
bool cNEWQ::removePoll(char * node)
{
 QUEUE_RECORD rec;
 long allowFlags = 0;

	checkstack();
	allowFlags = get_event_info(node);
	if( !(allowFlags & POLL))
	  return removeEntry(getPollRecord(node));
	else
	{

	 int recNum = getPollRecord(node);
	  if(recNum != -1)
	  {
		rec = getRec(recNum);
		rec.flag &= ~Q_OUTBOUND;
		rec.flag &= ~Q_CRASH;
		rec.flag &= ~Q_NORMAL;
		rec.flag |= Q_POLLED;
		rec.flag |= Q_HOLD;

		modifyRec(rec,recNum);
		return TRUE;
	  }
		return FALSE;
	}
}

int cNEWQ::getPollRecord(char * node)
{
  QUEUE_RECORD rec,mtrec;

	checkstack();
	QSize = totRecs();
	memset(&rec,0,sizeof(rec));
	memset(&mtrec,0,sizeof(mtrec));
	for(int i=1;i<=QSize;i++)
	{
		rec = getRec(i);
		if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;
		if(( ( rec.flag & Q_POLL) || (rec.flag & Q_POLLED)) && (strcmp(node,rec.nodestr) == 0))
		  return i;
	}
	return -1;
}


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: removePoll														 */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Remove a poll record from queue file							  */
/*																			  */
/*============================================================================*/
/* Parameters: Node string correspondin to the node to be polled			  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An existing queue file									  */
/*																			  */
/* Post Conditions: A poll record added or failure reported 				  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: POLLS should not be removed unless there is no event		  */
/* defined for that poll. If a poll event is active and the poll has been	  */
/* completed, it should be marked as 'done'.                                  */
/******************************************************************************/
bool cNEWQ::removePoll(int recNum)
{
	checkstack();
	return removeEntry(recNum);
}

/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: addFREQ															  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Add a FREQ entry to the queue file								  */
/*																			  */
/*============================================================================*/
/* Parameters: Node string correspondin to the node to be FREQed			  */
/*			   Filename to FREQ from the node								  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An existing queue file									  */
/*																			  */
/* Post Conditions: A FREQ record added or failure reported 				  */
/*																			  */
/*----------------------------------------------------------------------------*/
/******************************************************************************/

bool cNEWQ::addFREQ(char *nodestr, char *filename)
{
DOSFILE 				reqfile;
uint					zone,net,node,point;
char					nodebuf[25];
char					logBuf[65];
QUEUE_RECORD			queue_record;
extern _FAR_ DIRECTORIES  directory_info;

  checkstack();
  if(read_fido_config(0) == FALSE) return FALSE;

  memset(nodebuf	  ,1,sizeof(nodebuf));
  memset(&queue_record,0,sizeof(QUEUE_RECORD));

  maxstrcpy(queue_record.nodestr,nodestr,sizeof(queue_record.nodestr));

  queue_record.flag=Q_FILEREQ|Q_KILLSENT|Q_CRASH;

  maxstrcpy(nodebuf,queue_record.nodestr,sizeof(nodebuf));
  fido_nodestr_to_int(nodebuf,zone,net,node,point);

  directory_info.outgoing_packets[sizeof(directory_info.outgoing_packets)-1]=NULL;
  stripright(directory_info.outgoing_packets,' ');
  sprintf(queue_record.filename,"%s%04X%04X.REQ",directory_info.outgoing_packets,net,node);

  if(fileexist(queue_record.filename)!=255)
  {
  char line[100];
	 if(dosfopen(queue_record.filename,OPEN_RDWR|OPEN_DENYNONE/*WRIT*/,&reqfile)==-1)
	   return(FALSE);
	 while( dosfgets(line,sizeof(line),&reqfile) != -1)
	 {
	   if(strcmp(line,filename) == 0)
	   {
		 dosfclose(&reqfile);
		 return FALSE;
	   }
	 }
	 dosfputs(filename,&reqfile);
	 dosfwrite("\r",1,&reqfile);
	 dosfclose(&reqfile);
	}
  else
	{
	 if(dosfopen(queue_record.filename,OPEN_RDWR|OPEN_CREATE|OPEN_DENYNONE/*WRIT*/,&reqfile)==-1)
		return(FALSE);

	  dosfputs(filename,&reqfile);
	  dosfwrite("\r",1,&reqfile);
	  dosfclose(&reqfile);
	}
	if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
	{
	   sprintf(logBuf,"Adding FREQ to %s.",queue_record.nodestr);
	   writeFidolog(logBuf,BLOCK_END);
	}
  return(addEntry(queue_record));
}

/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: removeFREQ														  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Remove a FREQ record from queue file							  */
/*																			  */
/*============================================================================*/
/* Parameters: Node string correspondin to the node to be polled			  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An existing queue file									  */
/*																			  */
/* Post Conditions: A poll record added or failure reported 				  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: POLLS should not be removed unless there is no event		  */
/* defined for that poll. If a poll event is active and the poll has been	  */
/* completed, it should be marked as 'done'.                                  */
/******************************************************************************/

bool cNEWQ::removeFREQ(char *file)
{
QUEUE_RECORD rec;

  checkstack();
  int recNum = getFREQRecord(file);
  if(recNum==-1)
	return(FALSE);

  rec = getRec(recNum);

  if(!(get_event_info(rec.nodestr) & Q_FILEREQ))
	return(removeEntry(recNum));
  else
	{
	  if(recNum != -1)
		{
		  rec = getRec(recNum);

		  rec.flag &= ~Q_OUTBOUND;
		  rec.flag &= ~Q_CRASH;
		  rec.flag &= ~Q_NORMAL;
		  rec.flag |= Q_HOLD | Q_FREQED;
		  modifyRec(rec,recNum);
		  return TRUE;
		}
	  return FALSE;
	}
}

int cNEWQ::getFREQRecord(char * file)
{
  QUEUE_RECORD rec,mtrec;

	checkstack();
	QSize = totRecs();
	memset(&rec,0,sizeof(rec));
	memset(&mtrec,0,sizeof(mtrec));
	for(int i=1;i<=QSize;i++)
	{
		rec = getRec(i);
		if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;
		if((rec.flag & Q_FILEREQ || rec.flag & Q_FREQED) && (strcmpi(file,rec.filename) == 0))
		  return i;
	}
	return -1;
}


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: removePoll														 */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: Remove a poll record from queue file							  */
/*																			  */
/*============================================================================*/
/* Parameters: Node string correspondin to the node to be polled			  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An existing queue file									  */
/*																			  */
/* Post Conditions: A poll record added or failure reported 				  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Special Notes: POLLS should not be removed unless there is no event		  */
/* defined for that poll. If a poll event is active and the poll has been	  */
/* completed, it should be marked as 'done'.                                  */
/******************************************************************************/

bool cNEWQ::removeFREQ(int recNum)
{
	checkstack();
	return removeEntry(recNum);
}

/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: readRecord														 */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: read a specified record number from the queue file				  */
/*																			  */
/*============================================================================*/
/* Parameters: rec : QUEUE_RECORD structure passed by reference.			  */
/*			recNum : The record number to read from the file. This corresponds*/
/*					 the the slot number.									  */
/* Return Val: Success or Fail. 											  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An uninitialized QUEUE_FILE structure					  */
/*																			  */
/* Post Conditions: An uninitialized QUEUE_FILE structure containing		  */
/*					the desired record information. 						  */
/*----------------------------------------------------------------------------*/
/* Special Notes:															  */
/******************************************************************************/
bool cNEWQ::readRecord(QUEUE_RECORD & rec, int recNum)
{
QUEUE_RECORD mtrec;
  memset(&mtrec,0,sizeof(mtrec));

  rec = getRec(recNum);
  return (memcmp(&mtrec,&rec,sizeof(rec)) == 0);
}


/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: getRecordNumber													 */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: To return record/slot number accociated with a filename 		  */
/*																			  */
/*============================================================================*/
/* Parameters: file: Filename to search queue for							  */
/* Parameters:field: Which filed(filename/nodestr) to search on 			  */
/* Return Val: record number corresponding to the search criteria			  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : A qurious programmer wanting to know what queue entry	  */
/*					is associated with the filename/nodestr passed in		 */
/* Post Conditions: A satisfied programmer knowing what record number		  */
/*					corresponds to his data 								  */
/*----------------------------------------------------------------------------*/
/* Special Notes:															  */
/******************************************************************************/
int cNEWQ::getRecordNumber(char * file,int field)
{
	QUEUE_RECORD rec,mtrec;

	checkstack();
	memset(&rec,0,sizeof(rec));
	memset(&mtrec,0,sizeof(mtrec));

	QSize = totRecs();
	for(int i=1;i<=QSize;i++)
	{
		  rec = getRec(i);
		  if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;
		  switch(field)
		  {
			  case 1: if(strcmpi(rec.filename,file)==0) return i;
			  case 2: if(strcmpi(rec.nodestr,file)==0) return i;
		  }
	}
	return -1;
}



/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: getNextOutbound													  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: get the next queue record marked for immediate processing		  */
/*																			  */
/*============================================================================*/
/* Parameters:lastRead: The record number of the last outbound flagged record */
/*				rec: An uninitialized QUEUE_RECORD structure to be filled.	  */
/* Return Val: None 														  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An unitialized QUEUE_RECORD structure and a know last read*/
/* Post Conditions: Initialized structure containing the next outbound record */
/*----------------------------------------------------------------------------*/
/* Special Notes:															  */
/******************************************************************************/
int cNEWQ::getNextOutbound(QUEUE_RECORD & rec, int lastRead)
{
QUEUE_RECORD mtrec;

	checkstack();
	memset(&mtrec,0,sizeof(mtrec));
	if(lastRead <= 0) lastRead = 1;

	QSize = totRecs();
	for(int i=lastRead;i<=QSize;i++)
	{
		rec= getRec(i);
		if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;

		if( (rec.flag & Q_OUTBOUND && rec.flag & Q_CRASH) && (rec.failedConnects < PcbData.MaxTries) ) return i;
	}
	memset(&rec,0,sizeof(rec));
	return -1;
}


int cNEWQ::getNextRecord(QUEUE_RECORD & rec, int lastRead)
{
QUEUE_RECORD mtrec;

	// update header
	checkstack();
	memset(&rec,0,sizeof(rec));
	memset(&mtrec,0,sizeof(mtrec));
	if(lastRead <= 0) lastRead = 1;
	QSize = totRecs();
	for(int i=lastRead;i<=QSize;i++)
	{
	   rec = getRec(i);
	   if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;
	   return i;
	}
	return -1;
}
uint  cNEWQ::getOutboundCount(void)
{
	checkstack();
	return totRecs();
}

/******************************************************************************/
/*																			  */
/*								  CLASS cNEWQ								  */
/* Member: View 															  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/* Purpose: View Queue file 												  */
/*																			  */
/*============================================================================*/
/* Return Val: None 														  */
/*																			  */
/*============================================================================*/
/* Pre Conditions : An unviewed QUEUE										  */
/* Post Conditions: A viewed queue											  */
/*----------------------------------------------------------------------------*/
/* Special Notes:															  */
/******************************************************************************/
void cNEWQ::view(void)
{
 QUEUE_RECORD q_rec;
 int i = 1,count = 0;
 char * chPtr1 = NULL;
 char buffer[81],flagBuf[50];
 char sel,flag;
char		  mask_address[7]={6,0,'0','9',':','/','.'};

  checkstack();

  scanQueue();
  do
  {

	  clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
	  box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,ScrnColors->BoxColor,DOUBLE);
	  fastprint(28,6,"View/Modify Outbound Queue",ScrnColors->v14Color);

	  if(totRecs() == 0)
	  {
		fastprint(26,9,"Queue is empty. Hit a key...",ScrnColors->BoxColor);

		bool keyboard = FALSE;
		fgetbyte(SIXTYSECONDS,keyboard);
		return;
	  }

	  if( (i = getNextRecord(q_rec,i)) == -1)
	  {
		count = 1;
		i = getNextRecord(q_rec,0);
		if(i == -1) return;
	  }
	  else
		count++;
	  if(i == 1) count = 1;
	  chPtr1 = strrchr(q_rec.filename,'\\');
	  sprintf(buffer,"%d",count);
	  fastprint(53,7,buffer,ScrnColors->BoxColor);
	  sprintf(buffer,"Filename : %s ",chPtr1 == NULL ? q_rec.filename : ++chPtr1);
	  fastprint(26,8,buffer,ScrnColors->BoxColor);
	  sprintf(buffer,"Address  : %s",q_rec.nodestr);
	  fastprint(26,9,buffer,ScrnColors->BoxColor);

	  if(q_rec.readOnly)		   fastprint(24,7,"R/O",ScrnColors->BoxColor);
	  if(q_rec.flag & Q_KILLSENT)  fastprint(29,7,"K/S",ScrnColors->BoxColor);

	  strcpy(flagBuf,": ");
	  if(q_rec.flag & Q_POLL)
			   strcat(flagBuf,"POLL ");
	  if(q_rec.flag & Q_FILEREQ)
			   strcat(flagBuf,"FREQ ");
	  if((q_rec.flag & Q_POLLED) || (q_rec.flag & Q_FREQED))
			   strcat(flagBuf,"(done) ");
	  if(q_rec.flag & Q_CRASH)
			   strcat(flagBuf,"CRASH ");
	  if(q_rec.flag & Q_HOLD)
			   strcat(flagBuf,"HOLD ");
	  if(q_rec.flag & Q_NORMAL)
			   strcat(flagBuf,"NORMAL ");
	  if(q_rec.flag & Q_FILESEND)
			   strcat(flagBuf,"SEND-FILE ");

	  sprintf(buffer,"F) Flag  %s",flagBuf);
	  fastprint(26,11,buffer,ScrnColors->BoxColor);
	  fastprint(26,12,"S) Send This Packet",ScrnColors->BoxColor);

	  fastprint(26,13,"D) Delete ",ScrnColors->BoxColor);
	  fastprint(26,14,"N) Next ",ScrnColors->BoxColor);
	  fastprint(26,15,"P) Previous ",ScrnColors->BoxColor);

	  if(q_rec.failedConnects >= PcbData.MaxTries)
		sprintf(buffer,"[MAX] %d",q_rec.failedConnects);
	  else
		sprintf(buffer,"      %d",q_rec.failedConnects);
	  fastprint(26,16,"C) Clear Failed Connects ",ScrnColors->BoxColor);
	  fastprint(49,16,buffer,ScrnColors->BoxColor);
	  fastprint(26,17,"R) Re-Address Entry ",ScrnColors->BoxColor);

	  fastprint(26,18,"Enter selection : ",ScrnColors->BoxColor);
	  agotoxy(43,18);
	  settimer(11,SIXTY_SECONDS);
	  modemoffhook();
	  do
	  {

		if(i <= 0) i = totRecs();
		#ifdef __OS2__

		  updatelinesnow();
		  releasekbdblock(SIXTYSECONDS);
		  sel = waitforkey();

	  #else

		  sel = kbdhit(NOBUFFER);

	  #endif
	  }while(sel <= 0 && !timerexpired(11));
	  if(sel <= 0) sel = 'Q';
	  switch (sel)
	  {
		case 'F':
		case 'f':
				  fastprint(26,17,"Enter New Value : ",ScrnColors->BoxColor);
				  fastprint(26,18,"1-Crash 2-Hold 3-NORMAL",ScrnColors->BoxColor);
				  agotoxy(44,17);
				  settimer(2,SIXTY_SECONDS);
				  do
				  {

					#ifdef __OS2__

					  updatelinesnow();
					  releasekbdblock(60000);
					  flag = waitforkey();

					#else

					  giveup();
					  flag = kbdhit(NOBUFFER);

					#endif

				  }while(flag <= 0 && !timerexpired(2));

				  switch (flag)
				  {
					case '1' : q_rec.flag |= Q_CRASH;
							   q_rec.flag |= Q_OUTBOUND;
							   q_rec.flag &= ~Q_HOLD;
							   q_rec.flag &= ~Q_NORMAL;
							   q_rec.readOnly = TRUE;
									 break;

					case '2' : q_rec.flag &= ~Q_CRASH;
							   q_rec.flag &= ~Q_OUTBOUND;
							   q_rec.flag |= Q_HOLD;
							   q_rec.flag &= ~Q_NORMAL;
							   q_rec.readOnly = TRUE;
									 break;

					case '3' : q_rec.flag &= ~Q_CRASH;
							   q_rec.flag &= ~Q_HOLD;
							   q_rec.flag &= ~Q_OUTBOUND;
							   q_rec.flag |= Q_NORMAL;
							   q_rec.readOnly = TRUE;
									 break;
					default  : break;

				  }
				   // Write the new flag values to the file record
				   modifyRec(q_rec,i);

				   break;
		case 'N':
		case 'n':
					i++;
					break;
		case 'D':
		case 'd':

					fastprint(28,19,"Delete this entry? (Y/N)",ScrnColors->v14Color);
					settimer(3,SIXTY_SECONDS);
					do
					{
					  #ifdef __OS2__
						updatelinesnow();
						releasekbdblock(60000);
						sel = waitforkey();
					  #else
						sel = kbdhit(NOBUFFER);
					  #endif
					}while(sel == 0 && !timerexpired(3));

					sel = toupper(sel);
					if(sel == 'Y')
					{
					  if( (q_rec.flag & Q_POLL) || (q_rec.flag & Q_POLLED) )
						removePoll(q_rec.nodestr);
					  else if( (q_rec.flag & Q_FILEREQ) || (q_rec.flag & Q_FREQED) )
						removeFREQ(q_rec.filename);
					  else
						removeEntry(i);
					  i++;
					}
					break;
		case 'P':
		case 'p':
					i--;
					count--;
					if(i <= 0 || count <= 0) i = count = totRecs();
					count--;
					break;
		case  0 :
		case 27 :
		case 'Q':
		case 'q':
					break;
		case 'S':
		case 's':
					// make call, modify or remove poll entry
					close();
					send_outbound_mail(q_rec.nodestr,FROM_MENU);
					open();

					if((q_rec.flag & Q_POLL) && connect)
					  removePoll(q_rec.nodestr);
					modemoffhook();
					break;

		case 'C':
		case 'c':
					forEachMatch(q_rec.nodestr,CLEARCOUNT);
					break;

		case 'R':
		case 'r':
					  char	 newAddress[50];
					  clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
					  box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,ScrnColors->BoxColor,DOUBLE);
					  fastprint(27,10,"Old Address: ",ScrnColors->v14Color);
					  fastprint(40,10,q_rec.nodestr,ScrnColors->BoxColor);
					  fastprint(27,12,"Enter new address",ScrnColors->BoxColor);
					  fastprint(27,13,"                        ",0x01);
					  agotoxy(27,13);
					  inputfieldstr(newAddress,"_",0x07,
									sizeof(newAddress)-1,DEFAULTS,NOHELP,mask_address);
					  default_address(newAddress,sizeof(newAddress));
					  fastprint(27,13,newAddress,0x07);
					  fastprint(27,15,"Add change to queue?  (Y/N)",ScrnColors->BoxColor);
					  settimer(3,SIXTY_SECONDS);
					  #ifdef __OS2__
						updatelinesnow();
						releasekbdblock(60000);
						sel = waitforkey();
					  #else
						while( (sel = kbdhit(NOBUFFER)) == 0 && !timerexpired(3));
					  #endif

					  sel = toupper(sel);
					  if(sel == 'Y')
					  {
						maxstrcpy(q_rec.nodestr,newAddress,sizeof(q_rec.nodestr));
						modifyRec(q_rec,i);
					  }
					  break;

		default:
					count--;
					break;
	  }
  }while(sel != 'Q' && sel != 'q' && sel != 27);
  //compress();
  closecallerlog();
}

void cNEWQ::forEachMatch(unsigned zone,unsigned net,unsigned node,int action)
{
	char			addr[30];
	QUEUE_RECORD	rec,mtrec;

	checkstack();
	memset(&rec,0,sizeof(rec));
	memset(&mtrec,0,sizeof(mtrec));
	sprintf(addr,"%u:%u/%u",zone,net,node);

	QSize = totRecs();
	for(int i=1;i<=QSize;i++)
	{
		rec = getRec(i);
		if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;
		if(strstr(rec.nodestr,addr))
		{
		  switch(action)
		  {
			case ADD1FAIL:	  rec.failedConnects++;
							  break;

			case CLEARCOUNT:  rec.failedConnects =0;
							  break;
		  }
		  modifyRec(rec,i);
		}
	}

}

void cNEWQ::forEachMatch(char * addr,int action)
{
	QUEUE_RECORD	rec,mtrec;

	checkstack();
	memset(&rec,0,sizeof(rec));
	memset(&mtrec,0,sizeof(mtrec));

	QSize = totRecs();
	for(int i=1;i<=QSize;i++)
	{
		rec = getRec(i);
		if(memcmp(&mtrec,&rec,sizeof(mtrec)) == 0) continue;

		if(strstr(rec.nodestr,addr))
		{
		  switch(action)
		  {
			case ADD1FAIL:
			  rec.failedConnects++;
			  break;


			case CLEARCOUNT:
			  rec.failedConnects =0;
			  break;
		  }
		  modifyRec(rec,i);
		}
	}

}

bool cNEWQ::checkForFiles(const char * filename)
{

uint nrecs = totRecs();
QUEUE_RECORD trec;
uint count = 0;

  for(uint i = 1;i<nrecs;i++)
  {
	trec = getRec(i);
	if(strcmpi(trec.filename,filename) == 0) count++;
  }
  if(count > 1) return TRUE;
  else			return FALSE;
}

bool LIBENTRY outboundRec(void)
{
cNEWQ		   Q;
QUEUE_RECORD   rec;
int 		   ret=0;

  ret = Q.getNextOutbound(rec,1);
  return (!(ret == -1));
}

void LIBENTRY scanQueue(void)
{
  cNEWQ 	   Q;

  get_event_info(" ");   // Add any freq poll or send events to queue
  Q.scanQueue();
}





#endif




