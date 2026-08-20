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


/****************************************************************************
*
*				   Pcboard's FIDO import/export class
*						  Author: Stan Paulsen
*					Copyright Clark Development 1995
*
*  Member functions related to importing and exporting FIDO messages
*
*
*
*****************************************************************************/

#ifdef FIDO


#include <string.h>
#include <dos.h>
#include <io.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "system.h"
#include "misc.h"
#include "pcb.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "messages.h"
#include "screen.h"
#include "users.h"

#include "defines.h"
#include "structs.h"
#include "prototyp.h"

#include "fidoque.hpp"
#include "fidomsg.hpp"
#include "seenby.hpp"
#include "passthru.hpp"

#include "umwf.hpp"
#include "pcbmsgs.hpp"
#include "pcbtoss.hpp"
#include "dupechec.hpp"


#ifdef DEBUG
#include <memcheck.h>
#endif


extern	   unsigned int 				num_areas;
extern	   unsigned int 				node_count;
extern	   unsigned int 				num_akas;
extern	   unsigned int 				ConfigFileVer;
extern _FAR_ ARCHIVERS					  archiver_info;
extern _FAR_ DIRECTORIES				  directory_info;
extern	   NADDRESS 					*address;
extern	   NODE_T						*node_list;
extern	   struct						ffblk DTA;
extern	   long 						lastMsgNum;
extern	   void 	  LIBENTRY			capMsgInfo(unsigned short ConfNum, long MsgNum, long MsgOffset);
//extern cNEWQ							  Q;
//extern	 char						  ExtHdrFunctions[EXTHDR_TOTAL][EXTFUNCLEN];

static	_FAR_  char 					logBuf[70];
/*********************************************************************
 Prototypes
 *********************************************************************/

#if __BORLANDC__ >= 0x500
// see \PROJ\PCB\SOURCE\SUPPORT\NONOVRLY.C for an explanation
void LIBENTRY fido_addqueue(QUEUE_RECORD & rec);
void LIBENTRY fido_scanqueue(void);
#endif


cTOSSER::cTOSSER(void)
{

  memset(zeros,0,2);
  memset(&fido_msg_hdr,0,sizeof(fido_msg_hdr));
  memset(&this_area,0,sizeof(this_area));
  memset(&this_node,0,sizeof(this_node));
  memset(&fido_header_type2,0,sizeof(fido_header_type2));
  memset(&fido_header,0,sizeof(fido_header));
  memset(&confrec,0,sizeof(confrec));
  memset(sendingTo,0,sizeof(sendingTo));
  memset(&pkt_file,0,sizeof(pkt_file));
  memset(pkt_name,0,sizeof(pkt_name));
  memset(messagebuff,0,sizeof(messagebuff));
  memset(tear_line,0,sizeof(tear_line));
  memset(origin_line,0,sizeof(origin_line));


  for(int i=0;i<100;i++)
  {
	path[i]   = NULL;
	seenby[i] = NULL;
  }

  netmail	 = FALSE;
  add_tear	 = FALSE;
  add_origin = FALSE;
  add_msgid  = FALSE;
  dupe		 = FALSE;
  sendlist	 = FALSE;
  sendhelp	 = FALSE;
  sendquery  = FALSE;
  sendunlink = FALSE;
  direct	 = FALSE;
  hold		 = FALSE;
  crash 	 = FALSE;
  cerr		 = FALSE;
  dontecho	 = FALSE;
  banner_disp= TRUE;

  conf		= matched_aka = dest_zone = dest_net   = dest_node = dest_point = 0;
  orig_zone = 0;
  orig_net	= 0;
  orig_node = 0;
  orig_point= 0;
  pkt_type	= 0;
  totattimp = 1;
  grandtotalimported = 0;
  msg_count = 0;
  pkt_count = 0;
  numimp	= 0;

  maxstrcpy(weekdays[0],"SU",3);
  maxstrcpy(weekdays[1],"MO",3);
  maxstrcpy(weekdays[2],"TU",3);
  maxstrcpy(weekdays[3],"WE",3);
  maxstrcpy(weekdays[4],"TH",3);
  maxstrcpy(weekdays[5],"FR",3);
  maxstrcpy(weekdays[6],"SA",3);

}

cTOSSER::~cTOSSER(void)
{
  message.close();
  message.freeMsgBuf();
}


/****************************************************************************/
/* Send a netmail message saying that a file was sent.						*/
/* This function is called by itself rather then with the rest of the tosser*/
/* thus it contains a call to read_fido_config. 							*/

void LIBENTRY cTOSSER::send_file_notice(char *nodestr, char *filename)
{
char				nodebuf[30];
char				datebuf[10];
char				timebuf[6];
char				yearbuf[6];
uint				zone,net,node,point;
systimetype 		timerec;
sysdatetype 		daterec;

  checkstack();
  getsysdate(&daterec);
  getsystime(&timerec);

  if(read_fido_config(ADDR_RECS)==FALSE) return;

  if(open_pkt_file()==TRUE)
	{
	  netmail=TRUE;
	  if(write_pkt_header(nodestr)==TRUE)
		{
		  fido_msg_hdr.Packet_Type=0x0002;

		  maxstrcpy(nodebuf,nodestr,sizeof(nodebuf));
		  fido_nodestr_to_int(nodebuf,zone,net,node,point);

		  fido_msg_hdr.Orig_Node=address[matched_aka].node;
		  fido_msg_hdr.Dest_Node=node;
		  fido_msg_hdr.Orig_Net=address[matched_aka].net;
		  fido_msg_hdr.Dest_Net=net;
		  fido_msg_hdr.Attribute=MSG_FILEATT|MSG_PRIVATE;
		  sprintf(yearbuf,"%d",daterec.Year);
		  memcpy(yearbuf,yearbuf+2,3);
		  sprintf(datebuf,"%02d-%02d-%s",daterec.Month,daterec.Day,yearbuf);
		  sprintf(timebuf,"%02d:%02d",timerec.Hours,timerec.Minutes);
		  translate_date(fido_msg_hdr.DateTime,datebuf,timebuf,sizeof(fido_msg_hdr.DateTime));

		  maxstrcpy(fido_msg_hdr.To_User,"SysOp",sizeof(fido_msg_hdr.To_User));
		  maxstrcpy(fido_msg_hdr.From_User,"PCBoard Fido Sub-System",sizeof(fido_msg_hdr.From_User));
		  maxstrcpy(fido_msg_hdr.Subject,findstartofname(filename),sizeof(fido_msg_hdr.Subject));

		  dosfwrite(&fido_msg_hdr,FIDO_FIXED_SIZE,&pkt_file);
		  dosfputs(fido_msg_hdr.To_User,&pkt_file);
		  dosfwrite(zeros,1,&pkt_file);
		  dosfputs(fido_msg_hdr.From_User,&pkt_file);
		  dosfwrite(zeros,1,&pkt_file);
		  dosfputs(fido_msg_hdr.Subject,&pkt_file);
		  dosfwrite(zeros,1,&pkt_file);
		  dosfputs("\rThis message was created automatically by PCBoard to inform you of the\rAttached File shown in the subject of this message.\r",&pkt_file);
		  dosfwrite(zeros,1,&pkt_file);
		  dosfwrite(zeros,2,&pkt_file);
		  dosfclose(&pkt_file);
		  archive_pkt();
		  netmail=FALSE;
		}
	}
  free_fido_memory();
}

/****************************************************************************/
/* Make a list of people I'm sending this to for SEEN-BY lines.             */
/*																			*/
/* Paramaters: char *list - the buffer to store it in, malloc() array.		*/
/*			   int len	  - the length of the array from malloc().			*/
/*			   int conf   - the conference number we are looking for.		*/
/* Return value 		  - how big the array is now or -1 on failure.		*/

char* LIBENTRY cTOSSER::make_outbound_list(uint &len, int conf)
{
long			recno=0;
char			buffer[26],tmp[26];
char			*tmplist=NULL,*list=NULL;
uint			zone,net=0,node=0,point;
size_t			size=0;

  checkstack();
  memset(buffer,0,sizeof(buffer));
  memset(tmp   ,0,sizeof(tmp));

  if(tempuseralloc(FALSE)==-1)	 return(NULL);

  net=node=0;

  if((list=(char *)malloc(len))==NULL)
  {
	writeFidolog("Could not allocate memory for outbound list",BLOCK);
	return(NULL);
  }

  memset(list,0,len);

  init_users();  /* Initialize array for the Fido user record index records. */

  find_fido_rec(NULL,TRUE,TRUE,NULL);

  makeMyAkaList(list,len,SEENBYS);

  // Now add all the users we will be exporting this message to to the list.
  while((recno=find_fido_rec(NULL,TRUE,FALSE,NULL))!=-1)
	{
	  if(gettemprecord(recno) == -1) continue;
	  if(strchr(TempData->Name,'.')==NULL)
		{
		  if(isset(&TempReg[REG],conf) && isset(&TempReg[USR],conf))
			{
			  maxstrcpy(buffer,TempData->Name,sizeof(buffer));
			  size=(sizeof(buffer)-6);
			  memcpy(tmp,buffer+6,size);
			  fido_nodestr_to_int(tmp,zone,net,node,point);

			  sprintf(buffer," %u/%u",net,node);
			  if((strlen(list)+strlen(buffer)) >= len)
				{
				  len=(strlen(list)+strlen(buffer));
				  if((tmplist=(char *)realloc(list,(len+1)))!=NULL)
					list=tmplist;
				  else
					{
					  list=NULL;
					  writeFidolog("re-allocation failed PCBT 228",BLOCK);
					  tempuserdealloc();

					  return(NULL);
					}
				}
			  strcat(list,buffer);
			}
		}
	}
  tempuserdealloc();	/* deallocate temp user data structures. */

  return(list);
}



/*****************************************************************************/
/* Get the correct area record from the file.																													 */

bool  LIBENTRY	cTOSSER::get_area_record(char *areaname)
{
unsigned int   recnum=0;
NAREA_STRUCT   area;

  checkstack();
  assert(stackavail() > 2000);
  memset(&this_area,0,sizeof(AREA_STRUCT));
  memset(&area,0,sizeof(NAREA_STRUCT));

  {
  cAREAS areas;
  recnum = areas.findRec(areaname);
	if(recnum == 0) return FALSE;
	area = areas.getRec(recnum);
	areas.updateLastActiveDate(area.AreaTag);
  }
  fillThisArea(area,this_area);
  return TRUE;
}

void LIBENTRY cTOSSER::fillThisArea(NAREA_STRUCT & area,AREA_STRUCT & thisarea)
{
cORIGINS	   origins;
cAKAS		   akas;
ORIGIN		   org;
NADDRESS	   addr;

  if(area.AreaTag[0] == '\x0') return;

  if(area.AkaIndex == 0 || area.AkaIndex > akas.totRecs()) area.AkaIndex = 1;
  addr	 = akas.getRec(area.AkaIndex);
  if(area.OriginIndex == 0 || area.OriginIndex > origins.totRecs()) area.OriginIndex = 1;
  org	 = origins.getRec(area.OriginIndex);

  maxstrcpy(thisarea.Area_Name,area.AreaTag,sizeof(thisarea.Area_Name));
  maxstrcpy(thisarea.origin,org.origin,sizeof(thisarea.origin));

  if(addr.zone != 0 && addr.point == 0) sprintf(thisarea.default_aka,"%u:%u/%u",addr.zone,addr.net,addr.node);
  else if(addr.zone != 0)				sprintf(thisarea.default_aka,"%u:%u/%u.%u",addr.zone,addr.net,addr.node,addr.point);

  thisarea.PCB_Conference = area.ConfNum;
  thisarea.HighAscii	  = area.HighAscii;
}

/*****************************************************************************/
/* Copy the message body from PCBoard to Fido format.																					 */

bool   LIBENTRY  cTOSSER::transfer_body(char *list)
{
int 		i=0;

  checkstack();

  if(netmail!=TRUE)
  {
	if(add_fido_control_lines(list)==FALSE)
	{
	  for(i=0;path[i]!=NULL;i++)   freeAndNull(&path[i]);
	  for(i=0;seenby[i]!=NULL;i++) freeAndNull(&seenby[i]);
	  return(FALSE);
	}
  }

  if(write_fido_message()==FALSE)
  {
	writeFidolog("Could not write message to packet.",BLOCK);

	if(netmail!=TRUE)
	{
	  for(i=0;path[i]!=NULL;i++) freeAndNull(&path[i]);
	  for(i=0;seenby[i]!=NULL;i++) freeAndNull(&seenby[i]);
	}
	return(FALSE);
  }

  if(netmail!=TRUE)
  {
	for(i=0;path[i]!=NULL;i++) freeAndNull(&path[i]);
	for(i=0;seenby[i]!=NULL;i++) freeAndNull(&seenby[i]);
  }
  return(TRUE);
}

/*****************************************************************************/
/* Add the fido control lines.												 */

bool   LIBENTRY  cTOSSER::add_fido_control_lines(char *list)
{
char	  nodebuf[25];
char	  *haspath=NULL,*hasorigin=NULL,*hastearline=NULL,*hasmsgid=NULL,*p=NULL;

  checkstack();
  memset(nodebuf,0,sizeof(nodebuf));

  maxstrcpy(nodebuf,this_area.default_aka,sizeof(nodebuf));
  stripright(nodebuf,' ');

  if(this_area.default_aka[0]!=0 && this_area.default_aka[0]!=0x20)
	{
	  stripright(this_area.default_aka,sizeof(nodebuf));
	  maxstrcpy(nodebuf,this_area.default_aka,sizeof(nodebuf));
	}
  else
  {
	//maxstrcpy(nodebuf,address[0].nodestr,sizeof(nodebuf));
	if(address[0].point == 0) sprintf(nodebuf,"%u:%u/%u",address[0].zone,address[0].net,address[0].node);
	else  sprintf(nodebuf,"%u:%u/%u.%u",address[0].zone,address[0].net,address[0].node,address[0].point);
  }

  // Find out if the message has MSGID, PATH, ORIGIN or tearline info
  for(p = message.msgBody;*p !='\x0';p++)
  {
	switch(*p)
	{
	  case 0x01:
		  if(memcmp(p+1,"PATH:",5)==0 && *(p-1) == LineSeparator) haspath = p;
		  else if(memcmp(p+1,"MSGID:",6)==0 && *(p-1) == LineSeparator) hasmsgid = p;
		  break;
	  case 0xE3:
		  if(memcmp(p+1," * Origin:",10)==0) hasorigin = p;
		  else if(memcmp(p+1,"---",3)==0)    hastearline = p;
		  break;
	}
  }
  if(!hastearline)				 add_tear = TRUE;
  if(!hasorigin)			   add_origin = TRUE;
  if(add_origin && !hasmsgid) add_msgid   = TRUE; // Only add MSGID if you add the origin as well.


  if(!hastearline)
  {
	char tearBuf[36];
	maxstrcpy(tearBuf,Status.Version,sizeof(tearBuf));
	#ifdef BETA

	   char * nullPtr = strrchr(tearBuf,'(');
	   *nullPtr = '\x0';
	   strcat(tearBuf,"Beta");

	#endif
	sprintf(tear_line,"\r--- %-1.35s",tearBuf);
	add_tear=TRUE;
  }

  if(!hasorigin)
  {
	if(this_area.origin[0]==NULL||this_area.origin[0]=='\x20')
	{
	  stripright(PcbData.Origin,' ');
	  sprintf(origin_line," * Origin: %s (%s)",PcbData.Origin,nodebuf);
	}
	else
	{
	  stripright(this_area.origin,' ');
	  sprintf(origin_line," * Origin: %s (%s)",this_area.origin,nodebuf);
	}
	add_origin=TRUE;
  }

  if(add_seen_by(list)==FALSE)
	return(FALSE);

  if(add_path(haspath)==FALSE)
	return(FALSE);

  return(TRUE);
}

/*****************************************************************************/
/* Append our address to the path line. Add new lines and realloc as required*/

bool LIBENTRY  cTOSSER::add_path(char * haspath)
{
char	*p=NULL,*q=NULL,*ptr=NULL,addr[50];
int 	i=0,j=0;
NADDRESS rec;

  checkstack();
  memset(addr,0,sizeof(addr));
  //memset(path,0,sizeof(path));
  assert(stackavail() > 2000);

  findMyPathAddr(sendingTo,rec);
  if(rec.point != 0 && !haspath) return TRUE;

/* If there aren't any PATH lines then add one with our address in it. */

  if(haspath == NULL)
  {
	if((path[0]=(char *)malloc(100))==NULL)
	{
	   writeFidolog("Could not allocate memory for string. add_path()",BLOCK);
	   return(FALSE);
	}
	memset(path[0],0,100);
	sprintf(path[0],"\x01PATH: %u/%u",rec.net,rec.node);
	path[1]=NULL;
	return(TRUE);
  }

/* Copy the PATH lines into the array or pointers. */

  for(i=0,p=haspath;*p!=LineSeparator && *p !='\x0';i++)
	{
	  if((path[i]=(char *)malloc(100))==NULL)
		{
		  writeFidolog("Could not allocate memory for string. add_path()",BLOCK);
		  for(i=0;path[i]!=NULL;i++)
			{
			  free(path[i]);
			  path[i]=NULL;
			}
		  return(FALSE);
		}
	  memset(path[i],0,100);
	  for(j=0,q=path[i];*p!=LineSeparator && j<100;p++,j++)
		q[j]=*p;

	  q[j]=NULL;
	}
  path[i]=NULL;

  if(rec.point != 0) return TRUE;

/* Add our address to the PATH line. If the last Net in the line matches then */
/* don't add the net to the line.                                             */

  i--;
  sprintf(addr," %u/%u",rec.net,rec.node);  // Don't add points to path

  if(strlen(path[i])+strlen(addr)>74)
	{
	  i++;
	  if((path[i]=(char *)malloc(100))==NULL)
		{
		  writeFidolog("Could not allocate memory for string. add_path()",BLOCK);
		  for(i=0;path[i]!=NULL;i++)
			{
			  free(path[i]);
			  path[i]=NULL;
			}
		  return(FALSE);
		}
	  memset(path[i],0,100);
	  path[i+1]=NULL;
	  sprintf(path[i],"\x01PATH: %u/%u",rec.net,rec.node);
	}
  else
	{
	  if((p=strrchr(path[i],'/'))!=NULL)
		{
		  *p=NULL;
		  if((q=strrchr(path[i],' '))!=NULL)
			{
			  q++;
			  if((ptr=strchr(addr+1,'/'))!=NULL)
				{
				  if(memcmp(q,addr+1,int(ptr-(addr+1)))==0)
					sprintf(addr," %u",rec.node);
				}
			}
		  *p='/';
		}
	  strcat(path[i],addr);
	}
  return(TRUE);
}

/*****************************************************************************/
/* Strip the SEEN-BY lines from the message and create the array of pointers */
/* to strings for the SEEN-BY processor.									 */

bool LIBENTRY  cTOSSER::add_seen_by(char *list)
{
char	*p=NULL,*q=NULL,*ptr=NULL;
int 	i=0,j=0;

  checkstack();
  assert(stackavail() > 2000);

  memset(seenby,0,sizeof(seenby));

/* If there are no SEEN-BY lines then make one with our address(s) in it.		*/

  if((p=strstr(message.msgBody,"\xE3\x01SEEN-BY:"))==NULL)
	{
	  if((seenby[0]=(char *)malloc(100))==NULL)
		{
		  writeFidolog("Could not allocate memory for Seenbys. ",BLOCK);
		  for(i=0;seenby[i]!=NULL;i++) freeAndNull(&seenby[i]);
		  dupe=FALSE;
		  return(FALSE);
		}

	  // generate a list of all my akas that match the address we are exporting to
	  // This code needs to be tested.
	  char akalst[70];
	  memset(akalst,0,sizeof(akalst));
	  makeMyAkaList(akalst,sizeof(akalst),SEENBYS);
	  sprintf(seenby[0],"SEEN-BY: %-*s",60,akalst);
	  seenby[1]=NULL;

	  /* Add all the other nodes in list to the SEEN-BY lines.					   */

	  cSEENBY seen(seenby,list,sendingTo);

	  if(seen.isDupe()==TRUE)
		{
		  if(dest_point==0 && orig_point == 0)
			{
			  for(j=0;seenby[j]!=NULL;j++)
				{
				  free(seenby[j]);
				  seenby[j]=NULL;
				}
			  dupe=TRUE;
			  return(FALSE);
			}
		}
	  dupe=FALSE;
	  return(TRUE);
	}

  ptr=message.msgBody;


  /* If there are SEEN-BY lines then copy them into the array for processing. */

  for(i=0;(p=strstr(ptr,"\xE3\x01SEEN-BY:"))!=NULL;i++)
  {
	  ++p;
	  if((seenby[i]=(char *)malloc(100))==NULL)
	  {
		 writeFidolog("Could not allocate memory for Seenbys. ",BLOCK);
		 for(i=0;seenby[i]!=NULL;i++) freeAndNull(&seenby[i]);
		 dupe=FALSE;
		 return(FALSE);
	  }
	  q = seenby[i];
	  for(j=0;*p!=LineSeparator && j<99 ;p++,j++) q[j]=*p;
	  q[j]=NULL;
	  ptr=p;
  }

  seenby[i+1]=NULL;

  /* Add all the other nodes in list to the SEEN-BY lines.					   */

  cSEENBY seen(seenby,list,sendingTo);

  if(seen.isDupe()==TRUE)
	{
	  if(dest_point==0)
		{
		  for(j=0;seenby[j]!=NULL;j++)
			{
			  free(seenby[j]);
			  seenby[j]=NULL;
			}
		  dupe=TRUE;
		  return(FALSE);
		}
	}
  dupe=FALSE;
  return(TRUE);
}

/*****************************************************************************/
/* Write the fido message out.												 */

bool   LIBENTRY  cTOSSER::write_fido_message(void)
{
char	buffer1[50];
char	*line=NULL;
int 	i=0;

  checkstack();
  memset(buffer1,0,sizeof(buffer1));

  // If message is from SYOP replace with real name and proper the case of from and to
  strupr(fido_msg_hdr.From_User);
  if(strcmp(fido_msg_hdr.From_User,"SYSOP")==0 && dest_point == 0) getSysopNam(fido_msg_hdr.From_User);
  proper(fido_msg_hdr.To_User);
  proper(fido_msg_hdr.From_User);

/* Write out the header. To, From, and Subject are NULL-Terminated variable  */
/* length strings.															 */

  if(dosfwrite(&fido_msg_hdr,FIDO_FIXED_SIZE,&pkt_file)==-1) return(FALSE);
  if(dosfputs(fido_msg_hdr.To_User,&pkt_file)==-1)			 return(FALSE);
  if(dosfwrite(zeros,1,&pkt_file)==-1)						 return(FALSE);
  if(dosfputs(fido_msg_hdr.From_User,&pkt_file)==-1)		 return(FALSE);
  if(dosfwrite(zeros,1,&pkt_file)==-1)						 return(FALSE);
  if(dosfputs(fido_msg_hdr.Subject,&pkt_file)==-1)			 return(FALSE);
  if(dosfwrite(zeros,1,&pkt_file)==-1)						 return(FALSE);

  stripright(this_area.Area_Name,' ');
  strupr(this_area.Area_Name);
  if(strcmpi(this_area.Area_Name,"NETMAIL")!=0)
  {
	sprintf(buffer1,"AREA:%s\x0D",this_area.Area_Name);
	dosfputs(buffer1,&pkt_file);
	dosfwrite("\r",1,&pkt_file);
  }
  else
  {

	 /* We are now working with Netmail, so generate the INTL addressing line and */
	 /* the Point addressing lines if required. 	*/


	  FUSERS akauser,destuser;
	  destuser.zone = dest_zone;
	  destuser.net = dest_net;
	  destuser.node = dest_node;
	  destuser.point = dest_point;

	  if(getPrimAkaForNet(akauser,destuser))
		sprintf(buffer1,"\x01INTL %u:%u/%u %u:%u/%u",dest_zone,dest_net,dest_node,akauser.zone,akauser.net,akauser.node);
	  else
	  {
		sprintf(buffer1,"\x01INTL %u:%u/%u %u:%u/%u",dest_zone,dest_net,dest_node,address[matched_aka].zone,address[matched_aka].net,address[matched_aka].node);
		akauser.zone  = address[matched_aka].zone;
		akauser.net   = address[matched_aka].net;
		akauser.node  = address[matched_aka].node;
		akauser.point = address[matched_aka].point;
	  }

	  if(strstr(message.msgBody,"\x01INTL ") == NULL)
	  {
		dosfputs(buffer1,&pkt_file);
		dosfwrite("\r",1,&pkt_file);
	  }

	  if(akauser.point != 0 && strstr(message.msgBody,"\xE3\x01""FMPT") == NULL)
	  {
		sprintf(buffer1,"\x01""FMPT %u",akauser.point);
		dosfputs(buffer1,&pkt_file);
		dosfwrite("\r",1,&pkt_file);
	  }

	  if(fido_header_type2.DestPoint!=0 && strstr(message.msgBody,"\xE3\x01""TOPT") == NULL)
	  {
		sprintf(buffer1,"\x01TOPT %u",fido_header_type2.DestPoint);
		dosfputs(buffer1,&pkt_file);
		dosfwrite("\r",1,&pkt_file);
	  }
	}

  if(add_msgid) dosfputs(genMSGID(this_area.default_aka,this_area.PCB_Conference),&pkt_file);
  add_msgid = FALSE;

  /* Write out the message body. Don't write lines with Kludge info in them. */

  while((line=message.getLine())!=NULL)
	{

	  if((strstr(line,"SEEN-BY:")==NULL) && (strstr(line,"\x01PATH:")==NULL))
		{
		  dosfputs(xlateFidoText(line,this_area.HighAscii),&pkt_file);
		  dosfwrite("\r",1,&pkt_file);
		}
	}

  if(add_tear==TRUE)
	{
	  dosfputs(tear_line,&pkt_file);
	  dosfwrite("\r",1,&pkt_file);
	  add_tear=FALSE;
	}

  if(add_origin==TRUE)
	{
	  dosfputs(origin_line,&pkt_file);
	  dosfwrite("\r",1,&pkt_file);
	  add_origin=FALSE;
	}

/* Add kludge lines. */

  if(netmail!=TRUE)
	{
	  for(i=0;seenby[i]!=NULL;i++)
		{
		  dosfputs(seenby[i],&pkt_file);
		  dosfwrite("\r",1,&pkt_file);
		}

	  for(i=0;path[i]!=NULL;i++)
		{
		  dosfputs(path[i],&pkt_file);
		  dosfwrite("\r",1,&pkt_file);
		}
	}
  dosfwrite(zeros,1,&pkt_file);
  return(TRUE);
}

/*****************************************************************************/
/* Write the PKT file header.																																													 */

bool   LIBENTRY  cTOSSER::write_pkt_header(char *nodestr)
{
systimetype 				timerec;
sysdatetype 				daterec;
size_t						size=0;
int 						i;
uint						zone=0,net=0,node=0,point=0;
uint						rzone=0,rnet=0,rnode=0,rpoint=0;
char						buffer[26],tmp[26],routedaddr[26];
long						userno;
bool						readdress=PcbData.FidoReAddress;

  checkstack();

  getsysdate(&daterec);
  getsystime(&timerec);

  dosfseek(&pkt_file,0,SEEK_SET);  // Go to the begining of the file, PKT header

  init_users();



  if(nodestr==NULL)
  {
	get_noderec();
	maxstrcpy(buffer,UsersData.Name,sizeof(buffer));
	size=(sizeof(buffer)-6);
	memcpy(tmp,buffer+6,size);
	fido_nodestr_to_int(tmp,zone,net,node,point);
	dest_zone=zone;
	dest_net=net;
	dest_node=node;
	dest_point=point;
  }
  else if(netmail && readdress)  // Do I need to re-address this re-routed packet?
  {
	maxstrcpy(routedaddr,nodestr,sizeof(routedaddr));
	re_route(routedaddr);
	fido_nodestr_to_int(routedaddr,rzone,rnet,rnode,rpoint);
	fido_nodestr_to_int(nodestr,zone,net,node,point);
	if((userno=find_fido_rec(nodestr,FALSE,FALSE,NULL))!=-1)
	get_fido_rec(userno);
	dest_zone=rzone;
	dest_net=rnet;
	dest_node=rnode;
	dest_point=rpoint;
  }
  else
  {
	maxstrcpy(buffer,nodestr,sizeof(buffer));
	fido_nodestr_to_int(buffer,zone,net,node,point);
	fill_in_node_struct(nodestr);
	if((userno=find_fido_rec(nodestr,FALSE,FALSE,NULL))!=-1)
	  get_fido_rec(userno);
	dest_zone=zone;
	dest_net=net;
	dest_node=node;
	dest_point=point;
  }


/* Match one of our AKAs as close as possible. Zone or Zone/Net for use in	 */
/* the rest of the program to identify ourself as close as we can to the	 */
/* destination site.														 */

  FUSERS  duser,suser;
  duser.zone  = zone;
  duser.net   = net;
  duser.node  = node;
  duser.point = point;

  getPrimAkaForNet(suser,duser);


  for(i=0,matched_aka=0;i<num_akas;i++)
  {
	if(address[i].zone==suser.zone) matched_aka=i;
	if(address[i].zone==suser.zone && address[i].net==suser.net) matched_aka=i;
	if(address[i].zone==suser.zone && address[i].net==suser.net && address[i].node==suser.node)
	{
		matched_aka=i;
		break;
	}
  }

  if(this_node.pkt_type==0)
	this_node.pkt_type=2;

  if(this_node.pkt_type==2)
	{
	  orig_node=fido_header_type2.Orig_Node=address[matched_aka].node;
	  fido_header_type2.Dest_Node=dest_node;
	  fido_header_type2.Year=daterec.Year;
	  fido_header_type2.Month=daterec.Month-1; // Month is 0 based
	  fido_header_type2.Day=daterec.Day;
	  fido_header_type2.Hour=timerec.Hours;
	  fido_header_type2.Minute=timerec.Minutes;
	  fido_header_type2.Second=timerec.Seconds;
	  fido_header_type2.Baud=0;
	  fido_header_type2.Packet_Type=0x0002;
	  fido_header_type2.Dest_Net=dest_net;
	  orig_net=fido_header_type2.Orig_Net=address[matched_aka].net;
	  orig_point=fido_header_type2.OrigPoint=address[matched_aka].point;
	  orig_zone=fido_header_type2.Orig_Zone=address[matched_aka].zone;
	  fido_header_type2.ProtCode=PROTCODE;
	  fido_header_type2.SerialNo=1;
	  memset(fido_header_type2.Password,0,8);

	  UsersData.SysopComment[sizeof(UsersData.SysopComment)-1]=NULL;
	  stripright(UsersData.SysopComment,' ');

	  if(!netmail || (netmail && readdress))
	  {
		if(strlen(UsersData.SysopComment)<=8)
		  memcpy(fido_header_type2.Password,UsersData.SysopComment,strlen(UsersData.SysopComment));
		else
		  memcpy(fido_header_type2.Password,UsersData.SysopComment,8);
	  }

	  fido_header_type2.OrigZone=address[matched_aka].zone;
	  fido_header_type2.DestZone=dest_zone;
	  fido_header_type2.Orig_Zone=address[matched_aka].zone;
	  fido_header_type2.Dest_Zone=dest_zone;
	  fido_header_type2.OrigPoint=address[matched_aka].point;
	  fido_header_type2.DestPoint=dest_point;
	  fido_header_type2.ProdData=0;
	  fido_header_type2.CW=0x0001;
	  fido_header_type2.CWValid=0x0100;

	  if(dosfwrite(&fido_header_type2,sizeof(FIDO_PACKET_HEADER_TYPE2),&pkt_file)==-1)
		{
		  dosfclose(&pkt_file);

		  return(FALSE);
		}
	}
  else if(this_node.pkt_type==1)
	{
	  orig_node=fido_header.Orig_Node=address[matched_aka].node;
	  fido_header.Dest_Node=dest_node;
	  fido_header.Year=daterec.Year;
	  fido_header.Month=daterec.Month;
	  fido_header.Day=daterec.Day;
	  fido_header.Hour=timerec.Hours;
	  fido_header.Minute=timerec.Minutes;
	  fido_header.Second=timerec.Seconds;
	  fido_header.Baud=0;
	  fido_header.Packet_Type=0x0002;
	  fido_header.Dest_Net=net;
	  orig_net=fido_header.Orig_Net=address[matched_aka].net;
	  orig_zone=fido_header.Orig_Zone=address[matched_aka].zone;
	  fido_header.ProtCode=PROTCODE;
	  fido_header.SerialNo=1;
	  memset(fido_header.Password,0,8);

	  UsersData.SysopComment[sizeof(UsersData.SysopComment)-1]=NULL;
	  stripright(UsersData.SysopComment,' ');

	  if(strlen(UsersData.SysopComment)<=8)
		memcpy(fido_header.Password,UsersData.SysopComment,strlen(UsersData.SysopComment));
	  else
		memcpy(fido_header.Password,UsersData.SysopComment,8);

	  fido_header.Orig_Zone=address[matched_aka].zone;
	  fido_header.Dest_Zone=dest_zone;


	  if(dosfwrite(&fido_header,sizeof(FIDO_PACKET_HEADER),&pkt_file)==-1)
		{
		  dosfclose(&pkt_file);

		  return(FALSE);
		}
	}

  dest_zone=zone;
  dest_net=net;
  dest_node=node;
  dest_point=point;

  return(TRUE);
}
/****************************************************************************/
/* Copy PCBoard message header info to Fido Packet format.																		*/

bool   LIBENTRY  cTOSSER::transfer_msg_header(void)
{
int 		  extHdrCnt=0;
uint		  zone,net=0,node=0,point;
size_t		  size=0;
char		  nodebuf[26],tmp[26],hdrbuf[80],temp[35];

  checkstack();
  memset(nodebuf,0,sizeof(nodebuf));
  memset(tmp	,0,sizeof(tmp));
  memset(hdrbuf ,0,sizeof(hdrbuf));

  fido_msg_hdr.Packet_Type=0x0002;

  maxstrcpy(nodebuf,UsersData.Name,sizeof(nodebuf));
  size=(sizeof(nodebuf)-6);
  memcpy(tmp,nodebuf+6,size);
  fido_nodestr_to_int(tmp,zone,net,node,point);

  fido_msg_hdr.Orig_Node=address[matched_aka].node;
  fido_msg_hdr.Dest_Node=node;
  fido_msg_hdr.Orig_Net=address[matched_aka].net;
  fido_msg_hdr.Dest_Net=net;

  if(message.msgHdr.status!=' ' && message.msgHdr.status!='-')
	fido_msg_hdr.Attribute=MSG_PRIVATE;
  else
	fido_msg_hdr.Attribute=0;

  fido_msg_hdr.Msg_Cost=0;

  translate_date(fido_msg_hdr.DateTime,message.msgHdr.dmsg,message.msgHdr.tmsg,sizeof(fido_msg_hdr.DateTime));

  memcpy(temp,message.msgHdr.tname,sizeof(message.msgHdr.tname));
  temp[sizeof(message.msgHdr.tname)]=NULL;
  stripright(temp,' ');

  maxstrcpy(fido_msg_hdr.To_User,temp,sizeof(fido_msg_hdr.To_User));

  memcpy(temp,message.msgHdr.fname,sizeof(message.msgHdr.tname));
  temp[sizeof(message.msgHdr.fname)]=NULL;
  stripright(temp,' ');
  maxstrcpy(fido_msg_hdr.From_User,temp,sizeof(fido_msg_hdr.From_User));

  extHdrCnt=message.getExtHdrs();
  if(extHdrCnt!=0)
	{
	  // Handle extended subject header
	  memset(hdrbuf,0,sizeof(hdrbuf));
	  message.getExtHdrs(ExtHdrFunctions[EXTHDR_SUBJECT],hdrbuf);
	  if(hdrbuf[0]==0)
	  {
		  memcpy(temp,message.msgHdr.subj,sizeof(message.msgHdr.subj));
		  temp[sizeof(message.msgHdr.subj)]=NULL;
		  stripright(temp,' ');
		  maxstrcpy(fido_msg_hdr.Subject,temp,sizeof(fido_msg_hdr.Subject));
	  }
	  else
		  maxstrcpy(fido_msg_hdr.Subject,hdrbuf,sizeof(fido_msg_hdr.Subject));

	  memset(hdrbuf,0,sizeof(hdrbuf));
	  message.getExtHdrs(ExtHdrFunctions[EXTHDR_ATTACH],hdrbuf);

	  if(hdrbuf[0]!=0)
	  {
	  bool oktoattach = FALSE;
		// handle file attach extended header
		{
		cAREAS areas;
		NAREA_STRUCT area;
		   area = areas.getAreaRec(this_area.PCB_Conference);
		   oktoattach = area.AllowFileAttach == 'Y';
		}
		if(oktoattach)
		{
		char uniqfile[14];
		char realfile[14];
		char realpath[MAXFLEN];
		char uniqpath[MAXFLEN];
		char *walk=hdrbuf;
		char *end=walk;
		QUEUE_RECORD qrec;

			memset(&qrec,0,sizeof(qrec));
			// Get the actual attach name
			walk = eatSpaces(walk);
			end = walk;
			end = eatNonSpaces(end);
			maxstrcpy(realfile,walk,size_t(end-walk+1) < sizeof(uniqfile) ? size_t(end-walk+1) : sizeof(uniqfile));

			walk = end;
			walk = eatSpaces(walk);
			walk = eatNonSpaces(walk);
			walk = eatSpaces(walk);
			end  = walk;
			end  = eatNonSpaces(end);

			maxstrcpy(uniqfile,walk,(size_t(end-walk+1) < sizeof(realfile) ? size_t(end-walk+1) : sizeof(realfile)));

			sprintf(realpath,"%s%s",directory_info.outgoing_packets,realfile);
			sprintf(uniqpath,"%s%s",confrec.AttachLoc,uniqfile);
			TK_CopyFile(uniqpath,realpath);
			maxstrcpy(qrec.filename,realpath,sizeof(qrec.filename));
			qrec.flag=Q_KILLSENT|Q_NORMAL;
			if(dest_point == 0) sprintf(qrec.nodestr,"%u:%u/%u",dest_zone,dest_net,dest_node);
			else		   sprintf(qrec.nodestr,"%u:%u/%u.%u",dest_zone,dest_net,dest_node,dest_point);

			fido_msg_hdr.Attribute |= MSG_FILEATT;
			maxstrcpy(fido_msg_hdr.Subject,realfile,sizeof(fido_msg_hdr.Subject));
			// Limit Q's scope
			{
              #if __BORLANDC__ >= 0x500
                fido_addqueue(qrec);
              #else
                cNEWQ Q;
                Q.addEntry(qrec);
              #endif
			}
		}
	  }// oktoattach

	}
  else
	{
	  memcpy(temp,message.msgHdr.subj,sizeof(message.msgHdr.subj));
	  temp[sizeof(message.msgHdr.subj)]=NULL;
	  stripright(temp,' ');
	  maxstrcpy(fido_msg_hdr.Subject,temp,sizeof(fido_msg_hdr.Subject));
	}
  return(TRUE);
}

/*****************************************************************************/
/* Get the correct area record from the file.																													 */
/* Search for area to match the conference number.							 */
bool   LIBENTRY  cTOSSER::get_area_number(int arearec)
{
NAREA_STRUCT area;

  checkstack();

  {
  cAREAS areas;
	area = areas.getAreaRec(arearec);
  }
  if(area.AreaTag[0] == '\x0') return FALSE;

  fillThisArea(area,this_area);

  // Update last activity date for this area
  {
  cAREAS areas;
	areas.updateLastActiveDate(area.AreaTag);
  }

  return TRUE;
}

/*****************************************************************************/
/* Copy all untossed messages to .PKT file. 								 */

void LIBENTRY  cTOSSER::copy_messages(uint confnum,char *list)
{
long  highnum=0;
long  lownum =0;

  checkstack();

  if((lownum=message.lowMsgNum())==-1L)   return;
  if((highnum=message.highMsgNum())==-1L) return;

  if(MsgReadPtr[confnum]<lownum)	MsgReadPtr[confnum]=lownum;
  else								MsgReadPtr[confnum]++;	 // get past LMR.

  for(;MsgReadPtr[confnum] <= highnum;MsgReadPtr[confnum]++)
  {
	  long idxnum = message.readIdx(MsgReadPtr[confnum]);

	  if(idxnum >= 0L)
	  {
		  clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);
		  sprintf(messagebuff," Scanning message: %ld.",MsgReadPtr[confnum]);
		  fastprint(START_COL,MESSAGES,messagebuff,0x0b);

		  if(message.idxRec.off<=0)
		  {
			if(PcbData.FidoLogLevel >=6) writeFidolog("Incorrect Index Offset. Skipping message.",BLOCK);
			continue;
		  }
		  if(message.readHdr(message.idxRec.off)!=0)
		  {
			if(PcbData.FidoLogLevel >=6) writeFidolog("Invalid message header. Skipping message.",BLOCK);
			continue;
		  }

		  if(!NoEchoFlag && message.msgHdr.echo!='E')
		  {
			if(PcbData.FidoLogLevel >= 5) writeFidolog("Message not flagged for echo.",BLOCK);
			continue;
		  }
		  if(confrec.PrivMsgs==FALSE)
		  {
			if(!netmail && !allowPrvMsgs(this_area.Area_Name) && message.msgHdr.status!=' ' && message.msgHdr.status!='-')
			{
			  if(PcbData.FidoLogLevel >= 5) writeFidolog("Private message not exported.",BLOCK);
			  continue;
			}
		  }

		  if(message.readBody(0U)!=0)
		  {
			if(PcbData.FidoLogLevel == 2 || PcbData.FidoLogLevel >= 5) writeFidolog("Could not read message body.",BLOCK);
			continue;
		  }
		  message.msgBody[message.msgSize] = '\x0';
		  transfer_msg_header();
		  if(transfer_body(list)==FALSE)
		  {
			if(dupe==TRUE) dupe=FALSE;
			if(PcbData.FidoLogLevel == 255) writeFidolog("Message already seen by this site.",BLOCK);
			continue;
		  }

		  clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);
		  sprintf(messagebuff," Transfered message: %ld.",MsgReadPtr[confnum]);
		  clearLine(START_COL,MESSAGES);
		  fastprint(START_COL,MESSAGES,messagebuff,0x0b);
		  msg_count++;
		  pkt_count++;
	  }
	  else if(PcbData.FidoLogLevel >= 7)
		  writeFidolog("Invalid Message Number.",BLOCK);

  }
  if(msg_count!=0)
	{
	cAREAS	  areas;

	  clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);
	  sprintf(messagebuff," %ld Msg(s)from area: %-1.40s.",msg_count,this_area.Area_Name);
	  clearLine(START_COL,MESSAGES);
	  fastprint(START_COL,MESSAGES,messagebuff,0x0b);
	  if(PcbData.FidoLogLevel >= 2) writeFidolog(messagebuff,BLOCK);
	  msg_count=0;
	}
  else
	{
	  clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);
	  sprintf(messagebuff," No messages to export: area %-1.40s",this_area.Area_Name);
	  clearLine(START_COL,MESSAGES);
	  fastprint(START_COL,MESSAGES,messagebuff,0x0b);
	}
  return;
}

/*****************************************************************************/
/* Get Node record from the Config File.																																				 */
/* Uses information in the this_area struct to see if it got the right one.  */

bool	LIBENTRY  cTOSSER::get_noderec(void)
{
uint		  i=0;
uint		  zone=0,net=0,node=0,point=0;
char		  buffer[26],tmp[26];
size_t		  size=0;

  checkstack();
  memset(buffer,0,sizeof(buffer));
  memset(tmp   ,0,sizeof(tmp));

  maxstrcpy(buffer,UsersData.Name,sizeof(buffer));
  size=(sizeof(buffer)-6);
  memcpy(tmp,buffer+6,size);
  fido_nodestr_to_int(tmp,zone,net,node,point);

  for(i=0;i<node_count;i++)
  {
	  memcpy(&this_node,&node_list[i],sizeof(NODE_T));
	  fido_nodestr_to_int(this_node.nodestr,this_node.zone,this_node.net,this_node.node,this_node.point);
	  if(this_node.zone==zone&&this_node.net==net&&this_node.node==node&&this_node.point==point)
		return(TRUE);
  }

  return(FALSE);
}

/*****************************************************************************/
/* Archive .PKT File.. if there is one for this node, add the PKT to the ZIP */

void LIBENTRY  cTOSSER::archive_pkt(void)
{
QUEUE_RECORD	  queue_record;
bool			  found_node	 = FALSE;
bool			  needtomovefile = FALSE;
int 			  errorlevel	 = 0;
bool			  routeEchoMail  = PcbData.FidoRouteEchoMail;
char			  buffer[200],workfile[200],workbuf[80],params[100],arc[100];
char			  fullName[100],destAddr[50],srcAddr[50];
sysdatetype 	  daterec;

  // Initializations
  checkstack();
  memset(buffer  ,0,sizeof(buffer));
  memset(workfile,0,sizeof(workfile));
  memset(workbuf ,0,sizeof(workbuf));
  memset(fullName ,0,sizeof(fullName));
  memset(params  ,0,sizeof(params));
  memset(&queue_record,0,sizeof(QUEUE_RECORD));

  cls();

  // Build source and destination address strings
  sprintf(fullName,"%s%04.04X%04.04X",directory_info.outgoing_packets,uint(orig_net-dest_net),uint(+orig_node+orig_point-dest_node-dest_point));
  sprintf(destAddr,"%d:%d/%d",dest_zone,dest_net,dest_node);
  sprintf(srcAddr,"%d:%d/%d",orig_zone,orig_net,orig_node);

  // Add points to address.
  if(orig_point != 0)
  {
	  char pntBuf[10];
	  sprintf(pntBuf,".%d",orig_point);
	  strcat(srcAddr,pntBuf);
  }
  if(dest_point != 0)
  {
	  char pntBuf[10];
	  sprintf(pntBuf,".%d",dest_point);
	  strcat(destAddr,pntBuf);
  }

  // If this is an echo mail packet
  if(netmail!=TRUE)
  {
	char   day[3];
	int    fileNum = 0;

	  getsysdate(&daterec);
	  strcpy(day,weekdays[daterec.DayOfWeek]);

	  found_node=get_noderec();
	  strcpy(buffer,directory_info.outgoing_packets);
	  stripright(buffer,' ');

	  // Generate a packet name with incremented extension if needed
	  makeUniqueFZipName(fullName);

	  maxstrcpy(workfile,findstartofname(fullName),sizeof(workfile));

	  sprintf(workbuf,"00000000.%s%d",day,fileNum);


	  // This comparison makes sure the packet is not 0000000 eg addressed to yourself
	  if(memcmp(workfile,workbuf,8)!=0)
	  {
		strcat(buffer,workfile);
		assert(strlen(buffer) < sizeof(buffer));
		int archiveindex = 0;
		if(found_node==TRUE)
		  archiveindex = this_node.archiver_index;

		stripright(archiver_info.archivers[archiveindex],' ');
		stripright(archiver_info.archiver_switches[archiveindex],' ');
		sprintf(params,"%s %s %s",archiver_info.archiver_switches[archiveindex],buffer,pktname);
		assert(strlen(params) < sizeof(params));
		maxstrcpy(arc,archiver_info.archivers[archiveindex],sizeof(arc));

		// Make sure archiver is in the path
		if(strrchr(arc,'.') == NULL) strcat(arc,".EXE");

		if(srchpath(arc) == -1)
		{
		  sprintf(messagebuff,"%s not found in path! Cannot archive.",arc);
		  writeFidolog(messagebuff,BLOCK);
		  needtomovefile = TRUE;
		}
		else if(strlen(arc)+strlen(params) < 100)
		{
		  closemodem(FALSE);

		  errorlevel=performshell(arc,params,SHELLVIACOMMAND,PcbData.PriorityCompress,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,-1);

		  reopenport();

		  #ifdef __OS2__
		  errorlevel = 0;
		  #endif

		  // ifdef is because return value is undefined under os2
		  #ifndef __OS2__
		  if(errorlevel !=0 )
		  {
			clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);
			sprintf(messagebuff,"Archiver failed, errorlevel: %d.",errorlevel);
			clearLine(START_COL,STATUS_LINE);
			clearLine(START_COL,LAST_ERROR);
			fastprint(START_COL,STATUS_LINE,messagebuff,0x0d);
			fastprint(START_COL,LAST_ERROR,messagebuff,0x0d);
			writeFidolog(messagebuff,BLOCK);
		  }
		  else
		  #endif
		  {
		   cFIDOMSG MSG;
			if(PcbData.FidoCreateMsg) MSG.createFile(srcAddr,destAddr,buffer);
			unlink(pktname);
		  }

		}// Calling archiver block
		else
		{
		  writeFidolog("Arguments to archiver too long for command line.",BLOCK);
		  writeFidolog("  Reduce the length of one or more of the following...",BLOCK);
		  writeFidolog("     Fido out directory",BLOCK);
		  writeFidolog("     Fido work directory",BLOCK);
		  writeFidolog("     Fido archiver path",BLOCK);
		  writeFidolog("Adding packet to queue unarchived.",BLOCK);
		  errorlevel = TRUE;
		  needtomovefile = TRUE;
		}

	  }// If filename != 0000000


	  if(errorlevel!=0)  needtomovefile = TRUE;

	}// if netmail != true
	else
	  needtomovefile = TRUE;

	// If the pkt file was not archived (netmail or failed archive attempt)
	if(needtomovefile)
	{
	  sprintf(fullName,"%s%s",directory_info.outgoing_packets,pkt_name);
	  sprintf(workfile,"%s%s",directory_info.work_directory,pkt_name);
	  if(fileexist(workfile) != 255) pcbmovefile(workfile,fullName);
	  else	return;
	}

	maxstrcpy(queue_record.filename,fullName,sizeof(queue_record.filename));
	maxstrcpy(queue_record.nodestr,destAddr,sizeof(queue_record.nodestr));
	queue_record.flag=Q_KILLSENT|Q_NORMAL;
	if(routeEchoMail) re_route(queue_record.nodestr);
	// Limit Q's scope
	{
      #if __BORLANDC__ >= 0x500
        fido_addqueue(queue_record);
      #else
        cNEWQ Q;
        Q.addEntry(queue_record);
      #endif
	}
	//memrestorescreen(screen,FREESCREEN);
}

/*****************************************************************************/
/* Open .PKT File for this node.																																												 */

bool   LIBENTRY  cTOSSER::open_pkt_file(void)
{
systimetype 			  timerec;
sysdatetype 			  daterec;
char					  temp_file[MAXFLEN];

  checkstack();
  assert(stackavail() > 2000);

  directory_info.work_directory[sizeof(directory_info.work_directory)-1]=NULL;
  directory_info.outgoing_packets[sizeof(directory_info.outgoing_packets)-1]=NULL;

  stripright(directory_info.work_directory,' ');
  stripright(directory_info.outgoing_packets,' ');

  getsysdate(&daterec);
  getsystime(&timerec);

  while(1)
  {
	  timerec.Seconds++;

	  sprintf(pkt_name,"%02d%02d%02d%02X.PKT",daterec.Day,timerec.Hours,timerec.Minutes,timerec.Seconds);
	  assert(strlen(pkt_name) < sizeof(pkt_name));

	  sprintf(pktname,"%s%s",directory_info.work_directory,pkt_name);
	  sprintf(temp_file,"%s%s",directory_info.outgoing_packets,pkt_name);

	  assert(strlen(pktname) < sizeof(pktname));
	  assert(strlen(temp_file) < sizeof(temp_file));

	  if(fileexist(pktname)==255 && fileexist(temp_file)==255) break;
  }

  if(dosfopen(pktname,OPEN_CREATE|OPEN_RDWR|OPEN_DENYWRIT,&pkt_file)==-1)
  {
	  message.close();
	  dosfclose(&pkt_file);
	  return(FALSE);
  }

  return(TRUE);
}

/*****************************************************************************/
/* Fill in the this_node structure for use in the program with netmail. 	 */

void LIBENTRY  cTOSSER::fill_in_node_struct(char *nodestr)
{
  checkstack();
  assert(stackavail() > 2000);
  assert(nodestr != NULL);
  maxstrcpy(this_node.nodestr,nodestr,sizeof(this_node.nodestr));
  fido_nodestr_to_int(nodestr,this_node.zone,this_node.net,
					  this_node.node,this_node.point);

  this_node.pkt_type=2;
  this_node.archiver_index=-1;
}

/*****************************************************************************/
/* Create outbound netmail messages 										 */

void LIBENTRY  cTOSSER::send_netmail(void)
{
static long    highnum=0;
static long    lownum=0;
bool		   first=TRUE,error=FALSE,logged=FALSE,opened=FALSE;
unsigned int   confnum=0,i=0;
uint		   zone=0,net=0,node=0,point=0;
char		   *ptr=NULL;
char		   *tptr=NULL;
char		   nodestr[26],oldnode[26],tmpnode[26];
char		   to_buf[100];
char		   temp[100];

  checkstack();
  netmail=TRUE;
  msg_count=0;
  memset(nodestr,1,sizeof(nodestr));
  memset(oldnode,0,sizeof(oldnode));
  memset(to_buf,0,sizeof(to_buf));

  cls();
  if(get_area_record("NETMAIL")==FALSE)
  {
	sprintf(messagebuff,"No Area Tag information for Netmail conference.");
	clearLine(START_COL,STATUS_LINE);
	clearLine(START_COL,LAST_ERROR);
	fastprint(START_COL,STATUS_LINE,messagebuff,0x0d);
	fastprint(START_COL,LAST_ERROR,messagebuff,0x0d);
	tickdelay(ONESECOND);
	writeFidolog(messagebuff,BLOCK);
	netmail=FALSE;
	return;
  }

  confnum=this_area.PCB_Conference;
  getconfrecord(confnum,&confrec);


  // If the conference name,msgfile does not exist the return
  if(confrec.Name[0] == '\x0'          ||
	 confrec.MsgFile[0]  == '\x0'      ||
	 fileexist(confrec.MsgFile) == 255 ||
	 message.open(confrec.MsgFile) != 0 )
	 {
	   message.close();
	   return;
	 }

  // Get the conference loqw message number
  if((lownum=message.lowMsgNum())==-1)
  {
	netmail=FALSE;
	message.close();
	return;
  }

  // Get conference high message number
  if((highnum=message.highMsgNum())==-1)
  {
	netmail=FALSE;
	message.close();
	return;
  }

  if(confrec.ExportPtr<lownum)
	confrec.ExportPtr=lownum;
  else if(confrec.ExportPtr>highnum)
	confrec.ExportPtr=highnum+1;

  print_banner();

  // For each message in the netmail conference
  for(;confrec.ExportPtr<=highnum;confrec.ExportPtr++)
  {
	 memset(&fido_msg_hdr,0,sizeof(fido_msg_hdr));
	 memset(temp,0,sizeof(temp));

	 if(message.readIdx(confrec.ExportPtr)==confrec.ExportPtr)
	 {
	   if(message.readHdr(message.idxRec.off)!=0)  continue;
	   if(message.readBody(0U)!=0)					continue;

	   message.msgBody[message.msgSize] = '\x0';
	   if(!logged)
	   {
		 writeFidolog("Beginning NETMAIL export.",BLOCK_START);
		 logged = TRUE;
	   }

	   memset(to_buf,0,sizeof(to_buf));
	   assert(sizeof(message.msgHdr.tname) <= sizeof(temp));
	   memcpy(temp,message.msgHdr.tname,sizeof(message.msgHdr.tname));
	   temp[sizeof(message.msgHdr.tname)]=NULL;
	   stripright(temp,' ');
	   maxstrcpy(to_buf,temp,sizeof(to_buf));
	   assert(sizeof(message.msgHdr.subj) <= sizeof(temp));
	   memcpy(temp,message.msgHdr.subj,sizeof(message.msgHdr.subj));
	   temp[sizeof(message.msgHdr.subj)]=NULL;
	   stripright(temp,' ');
	   maxstrcpy(fido_msg_hdr.Subject,temp,sizeof(fido_msg_hdr.Subject));
	   stripright(to_buf,' ');

	   check_netmail_str(to_buf,sizeof(to_buf));

	   if((ptr=strrchr(to_buf,'@'))!=NULL)
	   {
		  ptr++;
		  if((tptr=strchr(ptr,' '))!=NULL)  *tptr=NULL;
		  maxstrcpy(nodestr,ptr,sizeof(nodestr));
		  maxstrcpy(tmpnode,nodestr,sizeof(tmpnode));
		  fido_nodestr_to_int(tmpnode,zone,net,node,point);
		  if(tptr!=NULL) *tptr=' ';
		  if(myAKA(zone,net,node,point)) continue;
	   }
	   else
	   {
		  while((ptr=message.getLine())!=NULL)
		  {
			 if(*ptr!=0x01)
			 {
				if(*ptr=='(')
				{
				   ptr++;
				   i=0;
				   error=FALSE;
				   while(*ptr!=')' && *ptr != LineSeparator && *ptr != '\x0')
				   {
					  if(*ptr==LineSeparator)
					  {
						 error=TRUE;
						 break;
					  }

					  temp[i]=*ptr;
					  i++;
					  ptr++;
				   }
				   temp[i]=NULL;
				   if(!strchr(temp,':') || !strchr(temp,'/') || *ptr == '\x0' || *ptr == LineSeparator)
					  error = TRUE;
				}
				else
				  error = TRUE;
			 break;
			 }
		  }
		  if(error==TRUE)
		  {
			 clsbox(START_COL,MESSAGES,79,MESSAGES,0x00);
			 sprintf(messagebuff,"Netmail Message #%ld not sent: Improperly addressed.",confrec.ExportPtr);
			 clearLine(START_COL,MESSAGES);
			 clearLine(START_COL,LAST_ERROR);
			 fastprint(START_COL,MESSAGES,messagebuff,0x0b);
			 fastprint(START_COL,LAST_ERROR,messagebuff,0x0b);
			 writeFidolog(messagebuff,BLOCK);
			 continue;
		  }
		  else
		  {
			 maxstrcpy(nodestr,temp,sizeof(nodestr));
			 strcat(to_buf,"@");
			 strcat(to_buf,temp);
			 fido_nodestr_to_int(temp,zone,net,node,point);
			 if(myAKA(zone,net,node,point)) continue;
		  }
	   }
	   if(first==TRUE || strcmp(nodestr,oldnode)!=0)
	   {
		  if(first!=TRUE)
		  {
			 if(msg_count>0)
			 {
				dosfwrite(zeros,2,&pkt_file);
				dosfclose(&pkt_file);
				add_netmail();
			 }
			 else
			 {
				dosfclose(&pkt_file);
				unlink(pktname);
			 }
		  }
		  if(open_pkt_file()==FALSE) continue;
		  opened=TRUE;
		  write_pkt_header(nodestr);
		  maxstrcpy(oldnode,nodestr,sizeof(oldnode));
		  first=FALSE;
		  msg_count=0;
		  direct=FALSE;
		  crash=FALSE;
		  hold=FALSE;
	   }
	   clsbox(START_COL,MESSAGES,79,MESSAGES,0x00);
	   sprintf(messagebuff,"Sending netmail to: %s.",nodestr);
	   clearLine(START_COL,MESSAGES);
	   fastprint(START_COL,MESSAGES,messagebuff,0x0b);
	   writeFidolog(messagebuff,BLOCK);

	   if(transfer_netmail(to_buf,nodestr)==FALSE) continue;

	   if(EnableMSG && !PcbData.FidoCreateMsg)
	   {

		cFIDOMSG MSG;
		  msgheadertype hdr;
		  memcpy(&hdr,&message.msgHdr,sizeof(hdr));
		  if(!MSG.preventFile) MSG.MsgtoFile(message.msgHdr.number);
		  message.resetLine();
	   }

	   msg_count++;
	 }

  }

  if(msg_count>0)
  {
	 dosfwrite(zeros,2,&pkt_file);
	 dosfclose(&pkt_file);
	 add_netmail();
  }
  else if (opened)
  {
	dosfclose(&pkt_file);
	unlink(pktname);
  }
  putconfrecord(this_area.PCB_Conference,&confrec);
  if(logged)
  {
	 writeFidolog("Ending NETMAIL export.",BLOCK_END);
	 logged = FALSE;
  }
  netmail=FALSE;
  message.close();
  return;
}

/*****************************************************************************/
/* Add netmail packet to the queue. 										 */

void LIBENTRY  cTOSSER::add_netmail(void)
{
char		  pktname[100];
char		  workfile[100];
char		  workbuf[100];
QUEUE_RECORD  queue_record;
long		  allowflags=0;
//cNEWQ 		queue;

  checkstack();
  memset(&queue_record,0,sizeof(queue_record));

  sprintf(workfile,"%s%s",directory_info.outgoing_packets,pkt_name);
  sprintf(workbuf,"%s%s",directory_info.work_directory,pkt_name);
  pcbmovefile(workbuf,workfile);
  maxstrcpy(pktname,workfile,sizeof(pktname));
  maxstrcpy(queue_record.filename,workfile,sizeof(queue_record.filename));

  if(dest_point==0)
	sprintf(queue_record.nodestr,"%u:%u/%u",dest_zone,dest_net,dest_node);
  else
	sprintf(queue_record.nodestr,"%u:%u/%u.%u",dest_zone,dest_net,dest_node,dest_point);


  // add record according to event info
  allowflags = get_event_info(queue_record.nodestr);


  if(allowflags & Q_CRASH)		queue_record.flag = Q_CRASH | Q_OUTBOUND;
  else if(allowflags & Q_HOLD)	queue_record.flag = Q_HOLD;
  else							queue_record.flag = Q_NORMAL;

  queue_record.flag |= Q_KILLSENT;


  // If +C or +D was specified, orverride3 event info and make it read only.
  if(hold==TRUE)
	{
	  queue_record.flag|=Q_HOLD;
	  queue_record.flag&=~Q_OUTBOUND;
	  queue_record.flag&=~Q_CRASH;
	  queue_record.flag&=~Q_NORMAL;
	  queue_record.readOnly = TRUE;
	}
  if(crash==TRUE)
	{
	  queue_record.flag|=Q_CRASH;
	  queue_record.flag|=Q_OUTBOUND;
	  queue_record.flag&=~Q_HOLD;
	  queue_record.flag&=~Q_NORMAL;
	  queue_record.readOnly = TRUE;
	}
  if(!direct) re_route(queue_record.nodestr);
  else		  queue_record.readOnly = TRUE;
  // Limit Q's scope
  {
    #if __BORLANDC__ >= 0x500
      fido_addqueue(queue_record);
    #else
      cNEWQ Q;
      Q.addEntry(queue_record);
    #endif
  }
  if(PcbData.FidoCreateMsg)
  {
	cFIDOMSG MSG;
	char destAddr[50];
	char srcAddr[50];
	char tmp[10];
	maxstrcpy(destAddr,queue_record.nodestr,sizeof(destAddr));
	sprintf(srcAddr,"%u:%u/%u",orig_zone,orig_net,orig_node);
	if(orig_point != 0)
	{
	  sprintf(tmp,".%u",orig_point);
	  strcat(srcAddr,tmp);
	}
	MSG.createFile(srcAddr,destAddr,queue_record.filename);
  }
}

/*****************************************************************************/
/* Transfer the header for Netmail messages. PCB->FIDO						 */

bool LIBENTRY  cTOSSER::transfer_netmail(char *to_buf,char *nodestr)
{
char		  nodebuf[26];
char		  *ptr=NULL;
char		  *tptr=NULL;
char		  tmp = NULL;
char		  temp[30];
char		  commands[30];
uint		  zone,net=0,node=0,point;

  checkstack();
  memset(nodebuf,0,sizeof(nodebuf));
  assert(to_buf != NULL);
  assert(stackavail() > 2000);

  memset(fido_msg_hdr.DateTime,0x01,20);  // Can't have a bunch of NULLS in
										  // the message header, will look
  fido_msg_hdr.Packet_Type=0x0002;		  // like the end of the PKT.

  if((ptr=strrchr(to_buf,'@'))!=NULL)
	{
	  *ptr=NULL;
	  ptr++;
	  if((tptr=strchr(ptr,' '))!=NULL)
		{
		  tmp=*tptr;
		  *tptr=NULL;
		  maxstrcpy(commands,tptr+1,sizeof(commands));
		  strupr(commands);
		  if(strstr(commands,"+D")!=NULL)
			{

			 long userNum = 1;
				if(tempuseralloc(FALSE) == -1)
				{
				  writeFidolog("Not enough memory to allocate temp user.",BLOCK);
				  return FALSE;
				}
				strupr(message.msgHdr.fname);
				if(memcmp(SysopName,message.msgHdr.fname,25) == 0)
				  userNum = 1;
				else
				  userNum = finduser(message.msgHdr.fname);

				if(userNum >=0 && gettemprecord(userNum) != -1)
				{
				  if(TempData->SecurityLevel >= PcbData.FidoCrashSec)
				  {
					  direct=TRUE;
					  if(msg_count>0)
						{
						  dosfwrite(zeros,2,&pkt_file);
						  dosfclose(&pkt_file);
						  add_netmail();
						}
					  else
						{
						  dosfclose(&pkt_file);
						  unlink(pktname);
						}
					  if(open_pkt_file() == FALSE) return FALSE;
					  write_pkt_header(nodestr);

				  }
				}
				tempuserdealloc();
			}
		  if(strstr(commands,"+C")!=NULL)
		  {
		   long userNum = 0;
			if(tempuseralloc(FALSE) == -1)
			{
			  writeFidolog("Not enough memory to allocate temp user.",BLOCK);
			  return FALSE;
			}
			strupr(message.msgHdr.fname);
			if(memcmp(SysopName,message.msgHdr.fname,25) == 0)
			  userNum = 1;
			else
			  userNum = finduser(strupr(message.msgHdr.fname));
			if(userNum >=1 && gettemprecord(userNum) != -1)
			{
			  if(TempData->SecurityLevel >= PcbData.FidoCrashSec)
				crash=TRUE;
			}
			tempuserdealloc();
		  }
		  if(strstr(commands,"+H")!=NULL)
			hold=TRUE;
		  char * newaddrptr=NULL;
		  if( (newaddrptr = strchr(commands,'[') ) != NULL)
			matched_aka = getNewAddr(matched_aka,newaddrptr);

		}
	  maxstrcpy(nodebuf,ptr,sizeof(nodebuf));
	  sprintf(UsersData.Name,"~FIDO~%s",nodebuf);
	  maxstrcpy(fido_msg_hdr.To_User,to_buf,sizeof(fido_msg_hdr.To_User));
	  if(tptr!=NULL)
		*tptr=tmp;
	}
  else
	return(FALSE);

  fido_nodestr_to_int(nodebuf,zone,net,node,point);
  fido_msg_hdr.Orig_Node=address[matched_aka].node;
  fido_msg_hdr.Dest_Node=node;
  fido_msg_hdr.Orig_Net=address[matched_aka].net;
  fido_msg_hdr.Dest_Net=net;
  fido_msg_hdr.Attribute|=MSG_PRIVATE;
  fido_msg_hdr.Msg_Cost=0;
  translate_date(fido_msg_hdr.DateTime,message.msgHdr.dmsg,message.msgHdr.tmsg,sizeof(fido_msg_hdr.DateTime));

  memcpy(temp,message.msgHdr.fname,sizeof(message.msgHdr.tname));
  temp[sizeof(message.msgHdr.fname)]=NULL;
  stripright(temp,' ');
  maxstrcpy(fido_msg_hdr.From_User,temp,sizeof(fido_msg_hdr.From_User));

  if(write_fido_message()==FALSE)
	return(FALSE);

  return(TRUE);
}

/*****************************************************************************/
/* check for extended headers																																													 */

void LIBENTRY  cTOSSER::check_netmail_str(char *to,int to_len)
{
unsigned int  i,inc,inc_old;
uint		  zone,net,node,point;
char		  *ptr	= NULL;
char		  *tptr = NULL;
char		  tmp	= NULL;
char		  Attach_file[14];
char		  Act_Attach_file[14];
//cNEWQ 		queue;
QUEUE_RECORD  queue_rec;
char		  hdr_buf[100];
char		  Source[100];
char		  tmpnode[40];
char		  Destination[100];
bool		  same=FALSE;

  checkstack();
  memset(hdr_buf,0,sizeof(hdr_buf));
  memset(&queue_rec,0,sizeof(queue_rec));
  memset(Attach_file,0,sizeof(Attach_file));
  memset(Act_Attach_file,0,sizeof(Act_Attach_file));
  memset(Source,0,sizeof(Source));
  memset(Destination,0,sizeof(Destination));

  assert(stackavail() > 2000);

  if(message.getExtHdrs()!=0)
	{
	  memset(hdr_buf,0,sizeof(hdr_buf));
	  message.getExtHdrs(ExtHdrFunctions[EXTHDR_SUBJECT],hdr_buf);
	  if(hdr_buf[0]!=0)
		maxstrcpy(fido_msg_hdr.Subject,hdr_buf,sizeof(fido_msg_hdr.Subject));

	  memset(hdr_buf,0,sizeof(hdr_buf));
	  message.getExtHdrs(ExtHdrFunctions[EXTHDR_TO],hdr_buf);
	  if(hdr_buf[0]!=0)
		maxstrcpy(to,hdr_buf,to_len);

	  if((ptr=strrchr(to,'@'))!=NULL)
		{
		  ptr++;
		  if((tptr=strchr(ptr,' '))!=NULL)
			*tptr=NULL;
		  maxstrcpy(tmpnode,ptr,sizeof(tmpnode));
		  fido_nodestr_to_int(tmpnode,zone,net,node,point);
		  if(tptr!=NULL)
			*tptr=' ';

		  for(i=0;i<num_akas;i++)
			{
			  if(address[i].zone==zone &&
				 address[i].net==net &&
				 address[i].node==node &&
				 address[i].point==point)

				same=TRUE;
			}
		}

	  if(same!=TRUE)
		{
		  memset(hdr_buf,0,sizeof(hdr_buf));
		  message.getExtHdrs(ExtHdrFunctions[EXTHDR_ATTACH],hdr_buf);
		  if(hdr_buf[0]!=0)
			{
			  inc=0;
			  while(1)	   // Get attach filename according to PCBoard
				{
				  if(hdr_buf[inc++]==32)
					break;
				}
			  memcpy(Attach_file,hdr_buf,inc-1); // Place in string
			  inc_old=inc+1;
			  while(1)
				{
				  if(hdr_buf[inc++]==32)	// Get filesize of file: Not needed, skipped.
					break;
				}
			  inc_old=inc;
			  while(1)		 // Get actual name of filename
				{
				  if(hdr_buf[inc++]==32)
					break;
				}
			  memcpy(Act_Attach_file,hdr_buf+inc_old,inc-1-inc_old);
			  sprintf(Source,"%s%s",confrec.AttachLoc,Act_Attach_file);
			  directory_info.outgoing_packets[sizeof(directory_info.outgoing_packets)-1]=NULL;
			  stripright(directory_info.outgoing_packets,' ');
			  sprintf(Destination,"%s%s",directory_info.outgoing_packets,Attach_file);
			  TK_CopyFile(Source,Destination);
			  maxstrcpy(queue_rec.filename,Destination,sizeof(queue_rec.filename));
			  queue_rec.flag=Q_KILLSENT|Q_NORMAL;

			  if((ptr=strrchr(to,'@'))!=NULL)
				{
				  ptr++;
				  maxstrcpy(queue_rec.nodestr,ptr,sizeof(queue_rec.nodestr));
				  if((tptr=strchr(ptr,' '))!=NULL)
					{
					  tmp=*tptr;
					  *tptr=NULL;
					}
				  if(tptr!=NULL)
					*tptr=tmp;
				}

			  stripright(to,' ');
			  // Limit Q's scope
			  {
				if(!direct) re_route(queue_rec.nodestr);
                #if __BORLANDC__ >= 0x500
                  fido_addqueue(queue_rec);
                #else
                  cNEWQ Q;
                  Q.addEntry(queue_rec);
                #endif
			  }
			  maxstrcpy(fido_msg_hdr.Subject,findstartofname(Attach_file),sizeof(fido_msg_hdr.Subject));
			  fido_msg_hdr.Attribute|=MSG_FILEATT;
			}
		}
	}
}

/*****************************************************************************/
/* Copy PCBoard messages to Fido format.																																				 */

bool LIBENTRY  cTOSSER::exportMessages(void)
{
uint			   i=0,len=0;
long			  recno=0,offset=0;
bool			  files_open=FALSE;
bool			  abort = FALSE;
char			  *ptr,*list;
int 			  screen;
bool			  beginLog	 = FALSE;
bool			  loggedName = FALSE;
char			  userName[100];

#ifdef MG
  cFIDOMSG			MSG;
#endif

  checkstack();
  setcursor(CUR_BLANK);
  list=NULL;
  // Limit Q's scope
  {
    #if __BORLANDC__ >= 0x500
      fido_scanqueue();
    #else
      cNEWQ Q;
      Q.scanQueue();
    #endif
  }
  offset=0;
  len=128;
  screen=memsavescreen();
  openlog();
  files_open=FALSE;
  assert(stackavail() > 2000);

  init_users();

  if(read_fido_config(NODE_RECS|ADDR_RECS)==FALSE)
  {
	 memfreescreen(screen);
	 return(FALSE);
  }
  find_fido_rec(NULL,TRUE,TRUE,NULL);
  send_netmail();
  if(EnableMSG && !PcbData.FidoCreateMsg)
  {

	MSG.start_at_1MSG();
	MSG.preventFile = TRUE;
	send_netmail();
	MSG.preventFile = FALSE;
  }

  // While there is a ~FIDO~ user
  while(abort == FALSE && (recno=find_fido_rec(NULL,TRUE,FALSE,&offset))!=-1)
  {
	 memset(pktname,0,sizeof(pktname));
	 memset(&pkt_file,0,sizeof(pkt_file));
	 loggedName = FALSE;
	 get_fido_rec(recno);

	 ptr = strrchr(UsersData.Name,'~');
	 if(ptr) maxstrcpy(userName,++ptr,sizeof(userName));
	 else	 maxstrcpy(userName,UsersData.Name,sizeof(userName));

	 if(UsersData.DeleteFlag!=TRUE)
	 {
		cls();
		print_banner();
		for(i=1;i<=PcbData.NumConf;i++)
		{
			 char * p=strrchr(UsersData.Name,'~');
			 if(p) ++p;
			 else	 p = UsersData.Name;
			 sprintf(logBuf,"Exporting to user %s from conference %u                       ",p,i);
			 fastprint(0,STATUS_LINE-1,logBuf,0x0f);
			if(isset(&ConfReg[REG],i) && isset(&ConfReg[USR],i))
			{

			   if(i % 10 == 0 && kbdhit(NOBUFFER) == ESC_KEY)
			   {
				 abort = TRUE;
				 writeFidolog("Export aborted by user.",BLOCK);
				 fastprint(0,STATUS_LINE+3,"",0x0f);
				 abort = TRUE;
				 freeAndNull(&list);
				 break;
			   }
			   getconfrecord(i,&confrec);
			   if(confrec.ConfType != FIDO_CONFERENCE || confrec.Name[0] == '\x0' || confrec.MsgFile[0] == '\x0') continue;

			   if(message.open(confrec.MsgFile) != 0)
			   {
				  freeAndNull(&list);
				  sprintf(logBuf,"Could not open message base %s,conf %u",confrec.MsgFile,i);
				  writeFidolog(logBuf,BLOCK);
				  message.close();
				  continue;
			   }

			   long hiMsgNum = message.highMsgNum();

			   if(hiMsgNum != -1L)
			   {
				  if(hiMsgNum > MsgReadPtr[i])
				  {
					 list=make_outbound_list(len,i);
					 if(files_open!=TRUE)
					 {
						if(open_pkt_file()==FALSE)
						{
						   free(list);
						   list=NULL;
						   message.close();
						   continue;
						}

						write_pkt_header(userName);
						files_open=TRUE;
					 }
					 if(get_area_number(i)==FALSE)
					 {
						message.close();
						sprintf(logBuf," [%-1.40s] No Area Tag for this conference.",confrec.Name);
						//fastprint(START_COL,STATUS_LINE,messagebuff,0x0d);
						clearLine(START_COL,LAST_ERROR);
						fastprint(START_COL,LAST_ERROR,messagebuff,0x0d);
						writeFidolog(logBuf,BLOCK);
						free(list);
						list=NULL;
						continue;
					 }

					 stripright(this_area.Area_Name,' ');
					 if( (strcmpi(this_area.Area_Name,"NETMAIL")==0) ||
								 (confrec.Name[0] == '\x0')          ||
								 (confrec.MsgFile[0]  == '\x0' ))
					 {
					   freeAndNull(&list);
					   message.close();
					   continue;
					 }
					 if(!beginLog)
					 {
						sprintf(logBuf,"Beginning FIDO Message Export");
						writeFidolog(logBuf,BLOCK_START);  // should be done only if there is mail to export
						beginLog = TRUE;
					 }
					 if(!loggedName)
					 {
						char * ptrName = strrchr(UsersData.Name,'~');
						if(ptrName != NULL)
						   maxstrcpy(sendingTo,++ptrName,sizeof(sendingTo));
						sprintf(logBuf,"Exporting to node %s   ",sendingTo);
						writeFidolog(logBuf,BLOCK);
						loggedName = TRUE;
					 }
					 print_banner();
					 sprintf(logBuf,"Exporting from area: %s",this_area.Area_Name);
					 clearLine(START_COL+1,STATUS_LINE);
					 fastprint(START_COL+1,STATUS_LINE,logBuf,0x0d);

					 copy_messages(i,list);
					 message.close();
					 MsgReadPtr[i]=hiMsgNum;
				  }
				  else
				  {
					 MsgReadPtr[i]=hiMsgNum;
					 message.close();
				  }
			   }
			   if(list!=NULL)
			   {
				  free(list);
				  list=NULL;
			   }
			}
		}
	 }

	 if(pkt_count!=0)
	 {
		dosfwrite(zeros,2,&pkt_file);
		dosfclose(&pkt_file);
		archive_pkt();
		//if(UsersData.Name[0] != '\x0')
		//{
		//	convertdatatoread(&UsersData,&UsersRead);
		//	putuserrecord(IGNOREDIRTY);
		//}
		pkt_count=0;
		files_open=FALSE;
	 }
	 else if (files_open)
	 {
		dosfclose(&pkt_file);
		unlink(pktname);
		files_open=FALSE;
	 }
	 if(UsersData.Name[0] != '\x0')
	 {
	   convertdatatoread(&UsersData,&UsersRead);
	   putuserrecord(IGNOREDIRTY);
	 }
  }
  offset = 0;
  bool donepassthru = FALSE;


/*
  if(PcbData.FidoEnablePassThru)
  {
	cPTINFO PT;
	  PT.updateUsers();
	  PT.delOldUsers();
  }
*/

  // Export passthroughs
  while(PcbData.FidoEnablePassThru && abort == FALSE && (recno=find_fido_rec(NULL,TRUE,FALSE,&offset))!=-1)
  {
	 get_fido_rec(recno);
	 ptr = strrchr(UsersData.Name,'~');
	 if(ptr) maxstrcpy(userName,++ptr,sizeof(userName));
	 else	 maxstrcpy(userName,UsersData.Name,sizeof(userName));

	 if(exportPassthru(userName)) archive_pkt();
	 else						  unlink(pktname);
	 donepassthru = TRUE;
  }

  // Delete passthru message file
  if(donepassthru)
  {
	cPASSTHROUGH p;
	  p.removeFile();
  }

  if(beginLog)
  {
	 sprintf(logBuf,"Ending FIDO Message Export");
	 writeFidolog(logBuf,BLOCK_END);  // should be done only if there is mail to export
	 beginLog = FALSE;
  }
  closecallerlog();
  resetuser();

  if(list) freeAndNull(&list);

  {
    bool keyboard = FALSE;
    fgetbyte(SIXSECONDS,keyboard);
  }
  memrestorescreen(screen,FREESCREEN);
  free_fido_memory();
  message.freeMsgBuf();
  message.close();
  setcursor(CUR_BLANK);
  return(TRUE);
}

/****************************************************************************/
/* This function determines what kind of archiver was used to compress the		*/
/* file specified by filename.																																													*/

int LIBENTRY cTOSSER::archiver_type(char *filename)
{
int 			file_handle=0;
char			testbuffer[5];
#ifdef __OS2__
  os2errtype Os2Error;
#endif

  checkstack();
  memset(testbuffer,0,sizeof(testbuffer));
  assert(stackavail() > 2000);

  if((file_handle=dosopen(filename,OPEN_READ|OPEN_DENYNONE POS2ERROR))!=-1)
	{
	/* check for PKZIP signature			*/

	  if(dosread(file_handle,testbuffer,2 POS2ERROR)!=-1)
		{
		  if(testbuffer[0]=='P'&&testbuffer[1]=='K')
			{
			  dosclose(file_handle);
			  return(ZIP_ARCH);
			}
		}
	  else
		{
		  dosclose(file_handle);
		  return(ERROR);
		}

	/* check for ARJ signature		*/

	  doslseek(file_handle,0L,SEEK_SET);
	  if(dosread(file_handle,testbuffer,2 POS2ERROR)!=-1)
		{
		  if(testbuffer[0]==0x60&&testbuffer[1]==0xEA)
			{
			  dosclose(file_handle);
			  return(ARJ_ARCH);
			}
		}
	  else
		{
		  dosclose(file_handle);
		  return(ERROR);
		}

	/* check for ARC signature		*/

	  doslseek(file_handle,0L,SEEK_SET);
	  if(dosread(file_handle,testbuffer,1 POS2ERROR)!=-1)
		{
		  if(testbuffer[0]==0x1A)
			{
			  dosclose(file_handle);
			  return(ARC_ARCH);
			}
		}
	  else
		{
		  dosclose(file_handle);
		  return(ERROR);
		}

	/* check for LZH signature		*/

	  doslseek(file_handle,3L,SEEK_SET);
	  if(dosread(file_handle,testbuffer,2 POS2ERROR)!=-1)
		{
		  if(testbuffer[0]=='l'&&testbuffer[1]=='h')
			{
			  dosclose(file_handle);
			  return(LZH_ARCH);
			}
		}
	  else
		{
		  dosclose(file_handle);
		  return(ERROR);
		}
	  dosclose(file_handle);
	  return(ARCH_UNKNOWN);
	}
  else
	return(ERROR);
}

/****************************************************************************/
/* This function unarchives filename										*/

bool LIBENTRY cTOSSER::unarchive(char *filename,int type)
{
char	params[100];
char	unarc[100];

  checkstack();
  memset(params,0,sizeof(params));
  assert(stackavail() > 2000);

  archiver_info.unarchivers[type][sizeof(archiver_info.unarchivers[type])-1]=NULL;
  archiver_info.unarchiver_switches[type][sizeof(archiver_info.unarchiver_switches)-1]=NULL;
  directory_info.work_directory[sizeof(directory_info.work_directory)-1]=NULL;

  stripright(archiver_info.unarchivers[type],' ');
  stripright(archiver_info.unarchiver_switches[type],' ');
  stripright(directory_info.work_directory,' ');

  maxstrcpy(unarc,archiver_info.unarchivers[type],sizeof(unarc));
  sprintf(params,"%s %s %s",archiver_info.unarchiver_switches[type],filename,directory_info.work_directory);
  assert(strlen(params) < sizeof(params));
  closemodem(FALSE);
  if(strrchr(unarc,'.') == NULL)
	strcat(unarc,".EXE");
  if(srchpath(unarc)== -1)
	{
	  sprintf(messagebuff,"%-1.40s Not found in path. Cannot unarchive.",unarc);
	  writeFidolog(messagebuff,BLOCK);
	}
  else
	{
	  cls();
	  //if(performshell(unarc,params,SHELLDIRECT)==0)
	  if(performshell(unarc,params,SHELLVIACOMMAND,PcbData.PriorityCompress,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,-1)==0)
		{
		  reopenport();
		  return(TRUE);
		}
	  else
		{
		  reopenport();
		  // ifdef is because return value is undefined under os2
		  #ifdef __OS2__
			return(TRUE);
		  #else
			return(FALSE);
		  #endif
		}
	}
  return(FALSE);
}

/****************************************************************************/
/* Look for files to toss. Unarchives found mail.																										*/

void LIBENTRY  cTOSSER::look_for_files(void)
{
int 			  i=0,type;
ffblk			  file_block;
char			  filename[80],oldname[80],newname[80];
int 			  screen;
int 			  dosfindmax=1;
int 			  done;
#ifdef __OS2__
int 			  DirHandle = MAKEDIRHANDLE;
#endif

  checkstack();
  memset(filename,0,sizeof(filename));

  stripright(directory_info.incoming_packets,' ');

  sprintf(filename,"%s*.PKT",directory_info.incoming_packets);


  done = dosfindfirst(filename,&file_block,0,&dosfindmax PDIRHANDLE);
  while(!done)
  {
	 sprintf(oldname,"%s%s",directory_info.incoming_packets,file_block.ff_name);
	 sprintf(newname,"%s%s",directory_info.work_directory,file_block.ff_name);
	 pcbmovefile(oldname,newname);

	 dosfindmax = 1;
	 done = dosfindnext(&file_block,&dosfindmax PDIRHANDLE2);
  }
  #ifdef __OS2__
	dosclosedirhandle(DirHandle);
  #endif

  for(i=0;i<7;i++)
  {
	  sprintf(filename,"%s*.%s?",directory_info.incoming_packets,weekdays[i]);
	  assert(strlen(filename) < sizeof(filename));

	  dosfindmax = 1;

	  #ifdef __OS2__
		DirHandle = SYSTEMDIRHANDLE;
	  #endif

	  if(dosfindfirst(filename,&file_block,0,&dosfindmax PDIRHANDLE)==0)
		{
		  if(file_block.ff_fsize!=0)
			{
			  print_banner();
			  banner_disp=FALSE;
			  maxstrcpy(filename,directory_info.incoming_packets,sizeof(filename)-14);
			  strcat(filename,file_block.ff_name);
			  type=archiver_type(filename);
			  switch(type)
				{
				  case ZIP_ARCH:
				  case ARC_ARCH:
				  case ARJ_ARCH:
				  case LZH_ARCH:
								  screen=memsavescreen();
								  if(unarchive(filename,type)==TRUE) unlink(filename);
								  else
									{
									  sprintf(oldname,"%s%s",directory_info.incoming_packets,file_block.ff_name);
									  sprintf(newname,"%s%s",directory_info.bad_packets,file_block.ff_name);
									  pcbmovefile(oldname,newname);
									  sprintf(messagebuff,"File: %-1.40s could not be unarchived. ",file_block.ff_name);
									  writeFidolog(messagebuff,BLOCK);
									  sprintf(messagebuff,"File: %-1.40s moved to bad packets dir.",file_block.ff_name);
									  writeFidolog(messagebuff,BLOCK);
									}
								  {
								    bool keyboard = FALSE;
								    fgetbyte(SIXSECONDS,keyboard);
								  }
								  memrestorescreen(screen,FREESCREEN);
								  break;

				  case ARCH_UNKNOWN:
									  sprintf(oldname,"%s%s",directory_info.incoming_packets,file_block.ff_name);
									  sprintf(newname,"%s%s",directory_info.work_directory,file_block.ff_name);
									  pcbmovefile(oldname,newname);
									  sprintf(messagebuff,"File: %-1.40s could not be unarchived.",file_block.ff_name);
									  writeFidolog(messagebuff,BLOCK);
									  sprintf(messagebuff,"File: %-1.40s moved to bad packets dir.",file_block.ff_name);
									  writeFidolog(messagebuff,BLOCK);
									  break;
				}
			  i--;			  // Found one with this extension, look for another.
			}
		}
	}
}

/*****************************************************************************/
/* Scan buffer for the AREA: keyword... get it so we know what to do.					 */

char * LIBENTRY cTOSSER::get_area(char *buffer,char *areabuf)
{
char	*tptr=NULL,*ptr;
assert(stackavail() > 2000);


  checkstack();
  if((tptr=strstr(buffer,"AREA:"))!=NULL)
	{
	  tptr+=5;
	  if((ptr=strchr(tptr,'\x0D'))==NULL)
		{
		  strcpy(areabuf,"NETMAIL");
		  return(areabuf);
		}

	  *ptr=NULL;
	  strcpy(areabuf,tptr);
	  *ptr='\x0D';

	  if(isascii(areabuf[0]) && areabuf[0]!=' ')
	  {
		strupr(areabuf);
		return(areabuf);
	  }

	  strcpy(areabuf,"NETMAIL");
	  return(areabuf);
	}
  if( ( strstr(buffer,"\x0D\x01SEEN-BY:") || strstr(buffer,"\x0D * ORIGIN:"))) strcpy(areabuf,"BAD");
  else	strcpy(areabuf,"NETMAIL");
  return(areabuf);
}

/*****************************************************************************/
/* Toss a message into PCBoard																																													 */

void LIBENTRY  cTOSSER::toss_message(char *buffer,msgbasetype *MsgBase)
{
uint				dzone,dnet,dnode,dpoint=0,tzone,tnet,tnode,tpoint=0;
char				*ptr=NULL,*tptr=NULL;
char				Date[10],Time[10],UserName[30],FromName[30],PcbSubj[30];
char				ExtSubj[65],ExtFrom[65],ExtTo[65],fromaddress[25],toaddress[25];
char				filebuf[100];
bool				nm=FALSE,addr;
msgheadertype		Header;
int 				rv=0;

  totattimp++;
  assert(stackavail() > 2000);
  checkstack();
  memset(Date	 ,0,sizeof(Date));
  memset(Time	 ,0,sizeof(Time));
  memset(UserName,0,sizeof(UserName));
  memset(FromName,0,sizeof(FromName));
  memset(PcbSubj ,0,sizeof(PcbSubj));
  memset(ExtSubj ,0,sizeof(ExtSubj));
  memset(ExtFrom ,0,sizeof(ExtFrom));
  memset(ExtTo	 ,0,sizeof(ExtTo));

  if(PcbData.FidoCheckDupePath || PcbData.FidoCheckDupeMsgId)
  {
  unsigned int dzone = 0;
  bool		   dupe = FALSE;
	cDUPECHECK dup;
	if(pkt_type == 2) dzone = fido_header_type2.Dest_Zone;
	else			  dzone = fido_header.Dest_Zone;

	if(PcbData.FidoCheckDupeMsgId)	   dupe = dup.checkID(buffer,&confrec);
	else if(PcbData.FidoCheckDupePath && !PcbData.FidoCheckDupeMsgId) dupe = dup.checkPath(buffer,dzone);
	if(dupe)
	{
	   sprintf(logBuf,"Skipping duplicate mesasage %u.",totattimp);
	   if(PcbData.FidoLogLevel >= 2) writeFidolog(logBuf,BLOCK);
	   clearLine(START_COL,MESSAGES);
	   fastprint(START_COL,MESSAGES,logBuf,0x0b);
	   return;
	}
  }


  // Prepare message body for pcboard message base.
  // replace carraige returs with line separators etc...
  prepBodyForPCB(buffer);
  if(!buffer) return;

  // Convert FIDO DATETIME to pcboard date and time formats
  translate_date_time(fido_msg_hdr.DateTime,Date,Time);


  maxstrcpy(UserName,fido_msg_hdr.To_User,sizeof(UserName));
  maxstrcpy(FromName,fido_msg_hdr.From_User,sizeof(FromName));
  maxstrcpy(PcbSubj,fido_msg_hdr.Subject,sizeof(PcbSubj));

  // If there is a file attach then get the info and create an extended header
  if(fido_msg_hdr.Attribute&MSG_FILEATT && !isEmptyMessage(buffer))
  {
   char filename[MAXFLEN];
	maxstrcpy(filebuf,fido_msg_hdr.Subject,sizeof(filebuf));
	if((ptr=strchr(filebuf,' '))!=NULL) *ptr=NULL;
	sprintf(filename,"%s%s",directory_info.incoming_packets,filebuf);
	createAttach(this_area.PCB_Conference,filename,FALSE);
  }

  // If the subject is too long create an extended header
  if(strlen(fido_msg_hdr.Subject)>sizeof(PcbSubj))
  {
	maxstrcpy(ExtSubj,fido_msg_hdr.Subject,sizeof(ExtSubj));
	buildextheader(EXTHDR_SUBJECT,ExtSubj,HDR_SUBJ);
  }

  if(strcmpi(this_area.Area_Name,"NETMAIL")==0)
  {
	  nm=TRUE;
	  addr=FALSE;

	  // Search the message body for the INTL kludge line. If it is there then
	  // Retrieve the infomation
	  if((tptr=strstr(buffer,"\x01INTL"))!=NULL)
	  {
		addr=TRUE;
		getINTLinfo(buffer,toaddress,sizeof(toaddress),fromaddress,sizeof(fromaddress));
		fido_nodestr_to_int(toaddress,tzone,tnet,tnode,tpoint);
		fido_nodestr_to_int(fromaddress,dzone,dnet,dnode,dpoint);
		sprintf(ExtTo,"%-1.34s@%-1.25s",fido_msg_hdr.To_User,toaddress);
		sprintf(ExtFrom,"%-1.34s@%-1.25s",fido_msg_hdr.From_User,fromaddress);
		buildextheader(EXTHDR_FROM,ExtFrom,HDR_FROM);
		buildextheader(EXTHDR_TO,ExtTo,HDR_TO);
	  }

	  if(addr==FALSE)
	  {
		getPointAddr(buffer,dpoint,tpoint);

		if(pkt_type==2)
		{
		  if(tpoint==0)
			sprintf(ExtTo  ,"%1.40s@%u:%u/%u",fido_msg_hdr.To_User,uint(fido_header_type2.DestZone),uint(fido_msg_hdr.Dest_Net),uint(fido_msg_hdr.Dest_Node));
		  else
			sprintf(ExtTo  ,"%1.40s@%u:%u/%u.%u",fido_msg_hdr.To_User,uint(fido_header_type2.DestZone),uint(fido_msg_hdr.Dest_Net),uint(fido_msg_hdr.Dest_Node),uint(tpoint));

		  if(dpoint==0)
			sprintf(ExtFrom,"%1.40s@%u:%u/%u",fido_msg_hdr.From_User,uint(fido_header_type2.OrigZone),uint(fido_msg_hdr.Orig_Net),uint(fido_msg_hdr.Orig_Node));
		  else
			sprintf(ExtFrom,"%1.40s@%u:%u/%u.%u",fido_msg_hdr.From_User,uint(fido_header_type2.OrigZone),uint(fido_msg_hdr.Orig_Net),uint(fido_msg_hdr.Orig_Node),uint(dpoint));

			  assert(strlen(ExtFrom) < sizeof(ExtFrom));
			  assert(strlen(ExtTo) < sizeof(ExtTo));
			  buildextheader(EXTHDR_FROM,ExtFrom,HDR_FROM);
			  buildextheader(EXTHDR_TO,ExtTo,HDR_TO);
		}
		else if(pkt_type==1)
		{

		   if(tpoint==0)
			 sprintf(ExtTo	,"%s@%u:%u/%u",fido_msg_hdr.To_User,address[0].zone,fido_msg_hdr.Dest_Net,fido_msg_hdr.Dest_Node);
		   else
			 sprintf(ExtTo	,"%s@%u:%u/%u.%u",fido_msg_hdr.To_User,address[0].zone,fido_msg_hdr.Dest_Net,fido_msg_hdr.Dest_Node,tpoint);

		   if(dpoint==0)
			 sprintf(ExtFrom,"%s@%u:%u/%u",fido_msg_hdr.From_User,address[0].zone,fido_msg_hdr.Orig_Net,fido_msg_hdr.Orig_Node);
		   else
			 sprintf(ExtFrom,"%s@%u:%u/%u.%u",fido_msg_hdr.From_User,address[0].zone,fido_msg_hdr.Orig_Net,fido_msg_hdr.Orig_Node,dpoint);

		   assert(strlen(ExtFrom) < sizeof(ExtFrom));
		   assert(strlen(ExtTo) < sizeof(ExtTo));
		   buildextheader(EXTHDR_FROM,ExtFrom,HDR_FROM);
		   buildextheader(EXTHDR_TO,ExtTo,HDR_TO);
		}
	  }
	  tptr=buffer;
  }
  else if((tptr=strstr(buffer,"AREA:"))!=NULL && strcmpi(this_area.Area_Name,"BAD")!=0)
  {
	  tptr+=5;
	  while(*tptr!=LineSeparator && *tptr != '\x0')
		tptr++;
	  tptr++;
  }

  checkDateFormat(Date);
  checkTimeFormat(Time);
  if(!tptr) tptr = buffer;

  if(strlen(tptr) >= 0x8000 - 0x100)
  {
	  tptr[0x8000-0x400] = '\x0';
	  dontecho = TRUE;
  }


  if(nm==TRUE || confrec.PrivMsgs==TRUE || fido_msg_hdr.Attribute&MSG_PRIVATE)
	makemsgheader(&Header,MSG_RCVR,strlen(tptr),Date,Time,UserName,0,0,
					"",' ',FromName,PcbSubj,"", !(NoEchoFlag || dontecho) );
  else
	  makemsgheader(&Header,MSG_PBLC,strlen(tptr),Date,Time,UserName,0,0,
					"",' ',FromName,PcbSubj,"",!(NoEchoFlag || dontecho) );

  Header.NetTag = '*';
  if(!isEmptyMessage(tptr))
  {
	// Replace all fido messages addressed to SYSOP with FIDO_SYSOP
	if(PcbData.FidoSysopChange && memicmp(Header.ToField,SysopName,25) == 0)
	  memcpy(Header.ToField,"FIDO_SYSOP",10);
	numimp++;
	rv = savetomsgbase(this_area.PCB_Conference,&confrec,MsgBase,&Header,tptr,0,0,capMsgInfo);
	if(rv == -1)
	{
	   writeFidolog("Could not save message. Possible full disk condition.",BLOCK);
	   cerr = TRUE;
	}
	else		 cerr = FALSE;


	if(EnableMSG && !PcbData.FidoCreateMsg)
	{
	   cFIDOMSG MSG;
	   if(nm && !MSG.preventFile) MSG.MsgtoFile(lastMsgNum);
	}
  }
  if(dontecho)
  {
	 dontecho = FALSE;
	 sprintf(logBuf,"Message truncated. Conf: %u Msg:%ld",this_area.PCB_Conference,lastMsgNum);
	 writeFidolog(logBuf,BLOCK);
  }
  freehdrs();
  addUMWF(fido_msg_hdr.To_User,this_area.PCB_Conference);
  if(strcmpi(fido_msg_hdr.To_User,"AREAFIX")==0 && nm==TRUE)
  {
	//closemessagebase(MsgBase);
	handle_areafix(buffer,ExtFrom);
	//openmessagebase(MsgBase);
  }
}

/******************************************************************************/
/* Handle an AreaFix request.												  */

void LIBENTRY  cTOSSER::handle_areafix(char *buffer,char *from)
{
bool		  changed;
char		  *ptr;
char		  *tptr;
char		  *oldptr;
long		  recno;
AREA_STRUCT   temp_area;
cDOSFILE	  result_file;
char		  password[70];
char		  commands[70];
char		  temp[200];
FUSERS		  user;

  checkstack();
  memset(password,0,sizeof(password));
  memset(commands,0,sizeof(commands));
  memset(temp,0,sizeof(temp));
  memcpy(&temp_area,&this_area,sizeof(AREA_STRUCT));
  sendlist=FALSE;
  sendhelp=FALSE;
  sendquery=FALSE;
  sendunlink=FALSE;
  changed=FALSE;

  init_users();

  prepAfix(result_file);

  find_fido_rec(NULL,TRUE,TRUE,NULL);

  maxstrcpy(temp,from,sizeof(temp));
  ptr=strrchr(temp,'@');
  if(ptr == NULL) return;
  ptr++;

  fido_nodestr_to_int(ptr,user.zone,user.net,user.node,user.point);

  if((recno=find_fido_rec(ptr,TRUE,FALSE,NULL))!=-1)
	{
	  get_fido_rec(recno);
	  if((tptr=strchr(fido_msg_hdr.Subject,' '))!=NULL)
		{
		  *tptr=NULL;
		  maxstrcpy(password,fido_msg_hdr.Subject,sizeof(password));
		  *tptr=' ';
		  maxstrcpy(commands,tptr,sizeof(commands));
		}
	  else
		maxstrcpy(password,fido_msg_hdr.Subject,sizeof(password));

	  UsersData.UserComment[sizeof(UsersData.UserComment)-1]=NULL;
	  stripright(UsersData.UserComment,' ');

	  if(strcmpi(UsersData.UserComment,password)==0)
		{
		  oldptr=buffer;
		  while((ptr=strchr(oldptr,LineSeparator))!=NULL)
			{
			  *ptr=NULL;
			  maxstrcpy(temp,oldptr,sizeof(temp));

			  if(strstr(temp,"---")!=NULL)
				{
				  *ptr=LineSeparator;
				  break;
				}

			  if(temp[0] != '\x0' && temp[0]!='*' && temp[0]!='\xFE' && temp[0] != '\x01')
				process_areafix_command(temp,&changed,result_file);

			  *ptr=LineSeparator;
			  oldptr=ptr;
			  oldptr++;
			}
		}
	  else
		{
		  writeFidolog("AreaFix password was incorrect, aborting request.",BLOCK);
		  sprintf(temp,"Areafix password: %s is incorrect.",password);
		  genResponseMsg(user,AFIXFAIL,temp);
		}
	}

  strupr(commands);
  if((strstr(commands,"-L")!=NULL) || sendlist==TRUE)   genResponseMsg(user,AFIXLIST,NULL);

  if((strstr(commands,"-H")!=NULL) || sendhelp==TRUE)   genResponseMsg(user,AFIXHELP,NULL);

  if((strstr(commands,"-Q")!=NULL) || sendquery==TRUE)  genResponseMsg(user,AFIXAVAL,NULL);

  if((strstr(commands,"-U")!=NULL) || sendunlink==TRUE) genResponseMsg(user,AFIXRESPONSE,NULL);

  if(changed==TRUE && UsersData.Name[0] != '\x0')
  {
	convertdatatoread(&UsersData,&UsersRead);
	putuserrecord(IGNOREDIRTY);
  }
  memcpy(&this_area,&temp_area,sizeof(AREA_STRUCT));
  char failfile[100];
  sprintf(failfile,"%sFAILS.DAT",directory_info.messages);
  if(fileexist(failfile) != 255) genResponseMsg(user,AFIXFAIL,"End of failure notice");  ;
  cleanUpAfix(user,result_file);
}

/******************************************************************************/
/* Process the command for AreaFix. 										  */

void LIBENTRY  cTOSSER::process_areafix_command(char *command, bool *changed,cDOSFILE & result_file)
{
char	temp[100];
char	resMsg[60];
char	*ptr;
char	*theCommand;
FUSERS user;
char * nameptr=NULL;

  nameptr = strrchr(UsersData.Name,'~');
  if(nameptr)fido_nodestr_to_int(++nameptr,user.zone,user.net,user.node,user.point);

  checkstack();
  ptr = theCommand = command;
  strupr(theCommand);

  //if(*theCommand != '-' && *theCommand  != '%' && *theCommand  != '+') ++theCommand ,++ptr;
  if(*theCommand == ' ') ++theCommand,++ptr;

  switch(theCommand[0])
	{
	  case	'-':
				  if(get_area_record(theCommand+1)==TRUE)
					{
					  if(isset(&ConfReg[REG],this_area.PCB_Conference) && isset(&ConfReg[USR],this_area.PCB_Conference))
						{
						  unsetbit(&ConfReg[USR],this_area.PCB_Conference);
						  sprintf(temp,"Removed: %s\r\n",this_area.Area_Name);
						  result_file.puts(temp);
						  *changed=TRUE;
						}
					}
				  // If it's a passthru area, remove the user from that area
				  else if(PcbData.FidoEnablePassThru)
				  {
				  cPTINFO PT;
					if(PT.seekToArea(theCommand+1))
					  PT.removeUserFromArea(user,theCommand+1);


					// If there are no more users registerd in this area
					// Send request to hub to turn off area
					if(PcbData.FidoEnableAreaFix && PT.countUsersInArea(theCommand+1) == 0 )
					  forwardRequest(theCommand+1,FALSE);
				  }
				  break;

	  case	'%':
				  if(strcmpi(theCommand+1,"LIST")==0)
					sendlist=TRUE;
				  else if(strcmpi(theCommand+1,"HELP")==0)
					sendhelp=TRUE;
				  else if(strcmpi(theCommand+1,"QUERY")==0)
					sendquery=TRUE;
				  else if(strcmpi(theCommand+1,"UNLINKED")==0)
					sendunlink=TRUE;
				  else if(strcmpi(theCommand+1,"+ALL")==0)
					areafixAddRemoveAll(changed,TRUE);
				  else if(strcmpi(theCommand+1,"-ALL")==0)
					areafixAddRemoveAll(changed,FALSE);
				  else if(memcmp(theCommand+1,"RESCAN",6) == 0)
				  {
					doRescan(theCommand+1);
					*changed = TRUE;
				  }
				  break;

	  case	'+':  ptr++;
	  default:
				  // If the area exists as a regular PCBoard conference and
				  // The user has access, add them to it
				  if(get_area_record(ptr)==TRUE)
				  {
					  if(isset(&ConfReg[REG],this_area.PCB_Conference) && !isset(&ConfReg[USR],this_area.PCB_Conference))
						{
						 pcbconftype crec;
						  setbit(&ConfReg[USR],this_area.PCB_Conference);
						  getconfrecord(this_area.PCB_Conference,&crec);
						  if(fileexist(crec.MsgFile) != 255 && message.open(crec.MsgFile) ==0)
						  {
							MsgReadPtr[this_area.PCB_Conference] = message.highMsgNum();
							message.close();

							sprintf(temp,"Added: %s\r\n",this_area.Area_Name);
							result_file.puts(temp);
							*changed=TRUE;
						  }
						}
						else
						{
						  sprintf(resMsg,"Already registered or do not have access to area %s. ",ptr);
						  writeFailsToFile(resMsg);
						}
				  }
				  // IF passthrus are enabled and auto add passthurs is enabled
				  // add the passthru area and forward the request if forwarding
				  // is enabled
				  else if(PcbData.FidoEnablePassThru)
				  {
					 cPTINFO PT;
					 if(PcbData.FidoEnablePassThru && PT.seekToArea(ptr) && !PT.Err()) PT.putUserInArea(user,ptr);
					 else if(PcbData.FidoEnableAreaFix && PcbData.FidoEnablePassThru)
					 {
					   sprintf(logBuf,"Auto Adding passthru area %s.",ptr);
					   writeFidolog(logBuf,BLOCK);
					   forwardRequest(ptr,TRUE);
					   if(!PT.Err())
					   {
						 PT.addArea(ptr);
						 PT.putUserInArea(user,ptr);
					   }
					 }
					 else
					 {
						char failStr[100];
						sprintf(failStr,"Attempted to add area %-1.20s, but it does not exist.",theCommand+1);
						writeFailsToFile(failStr);
					 }
				  }
				  // else if just forwarding is enabled then just forward it
				  else if(PcbData.FidoEnableAreaFix) forwardRequest(ptr,TRUE);
				  else
				  {
					char failStr[100];
					sprintf(failStr,"Attempted to add area %-1.20s, but it does not exist.",theCommand+1);
					writeFailsToFile(failStr);
				  }

				  break;
				}
}

/******************************************************************************/
/* Turn on ALL available echos for this node.								  */

void LIBENTRY  cTOSSER::areafixAddRemoveAll(bool *changed,bool add)
{
NAREA_STRUCT  area;
unsigned int  numrecs=0;
char		  addname[MAXFLEN];
char		  username[25],*userptr=NULL;
char		  msgBuf[70];
char		* areaname;
unsigned int  i=0;
cDOSFILE	  rfile;
FUSERS		  user

  checkstack();

  // init user struct
  userptr = strrchr(UsersData.Name,'~');
  if(userptr == NULL) return;
  ++userptr;
  maxstrcpy(username,userptr,sizeof(username));
  fido_nodestr_to_int(username,user.zone,user.net,user.node,user.point);

  // Init file to hold areas added for later use with genResponseMsg
  maxstrcpy(addname,directory_info.messages,sizeof(addname));
  strcat(addname,"AFIX.ADD");
  if(fileexist(addname) != 255) unlink(addname);

  if(fileexist(addname) == 255 && rfile.open(addname,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE) != 0) return;
  else if (fileexist(addname) != 255 && rfile.open(addname,OPEN_RDWR |	OPEN_DENYNONE) != 0) return;


  // Limit areas scope
  {
  cAREAS areas;
  numrecs = areas.totRecs();
  // Add all PCBoard FIDO conferences
  for(i=1;i<=numrecs;i++)
  {

	area = areas.getRec(i);

	if(add && isset(&ConfReg[REG],area.ConfNum) && !isset(&ConfReg[USR],area.ConfNum))
	{
	   setbit(&ConfReg[USR],area.ConfNum);
	   sprintf(msgBuf,"Added: %s \r\n",area.AreaTag);
	   rfile.puts(msgBuf);
	   *changed = TRUE;
	}
	else if(add)
	{
	   sprintf(msgBuf,"Already registerd or do not have access to area %s. Not added.\r\n",area.AreaTag);
	   rfile.puts(msgBuf);
	}

	if(!add)
	{
	  if(isset(&ConfReg[REG],area.ConfNum) && isset(&ConfReg[USR],area.ConfNum))
	  {
		unsetbit(&ConfReg[USR],area.ConfNum);
		sprintf(msgBuf,"Removed: %s \r\n",area.AreaTag);
		rfile.puts(msgBuf);
		*changed=TRUE;
	  }
	}

  }
  }

  i = 1;

 if(PcbData.FidoEnablePassThru)
 {
  cPTINFO		PT;
  // Process passthru areas.
  while( (areaname = PT.seekToAreaNum(i++) ) != NULL)
  {
	// If adding all areas
	if(add)
	{
	   if(! PT.userInArea(areaname,user) )
	   {
		  PT.putUserInArea(user,areaname);
		  sprintf(msgBuf,"Added: %s \r\n",areaname);
		  rfile.puts(msgBuf);
		  *changed = TRUE;
	   }
	}
	// Else deleta all areas
	else
	{
	   if( PT.userInArea(areaname,user) )
	   {
		  PT.removeUserFromArea(user,areaname);
		  sprintf(msgBuf,"Removed: %s \r\n",this_area.Area_Name);
		  rfile.puts(msgBuf);
		  *changed=TRUE;
	   }
	}
  }
 }
  rfile.close();
}

/*****************************************************************************/
/* Read in the Fido message body from the .PKT. Size buffer as required.	 */

char * LIBENTRY cTOSSER::read_fido_body(char * msg, int buflen, bool & end)
{
int 		  length=0;
long		  seekback=0;
char		  buf[2048];
char		  tch = ' ';

  checkstack();

  memset(msg,0x20,buflen);
  msg[0]='\x0';
  msg[buflen-1] = '\x0';

  while(TRUE)
  {
	  memset(buf,0,sizeof(buf));
	  length=dosfread(buf,sizeof(buf)-1,&pkt_file);
	  if(length == 0)
	  {
		end = TRUE;
		break;
	  }

	  // If the buffer isn't full yet, add buf to it
	  // If it is full wee need to seek to the end of the message
	  if( (strlen(msg) + strlen(buf)) < buflen) strcat(msg,buf);
	  else
	  {
		if(strlen(buf) < length) dosfseek(&pkt_file,size_t(length-strlen(buf)),SEEK_CUR);
		else
		{
		   while(tch != '\x0') dosfread(&tch,1,&pkt_file);
		   dontecho = TRUE;
		}

		break;
	  }

	  // If I read in 2k, but an end of message in somewhere in that 2k
	  // reset file pointer to beginning of the next message
	  if(strlen(buf) < length)
	  {
		 seekback = length-strlen(buf);
		 dosfseek(&pkt_file,-seekback+1,SEEK_CUR);
		 break;
	  }
	}
	return msg;
}

/*****************************************************************************/
/* Toss the .PKT file into PCBoard. 																																							 */

bool  LIBENTRY	cTOSSER::toss_packet(char *filename)
{
bool		  end=FALSE,result=FALSE,update=FALSE,cupdate=FALSE,nobad=FALSE,passthru=FALSE;
int 		  buflen=32767;
unsigned long num=1;
char		  *buffer=NULL;
char		  areabuf[60],areabak[60];
char		  addr[100];
msgbasetype   MsgBase;
bool		  secureNetmail = PcbData.FidoSecure;


  // Initializations
  checkstack();
  netmail=FALSE;
  memset(areabuf,0,sizeof(areabuf));
  memset(areabak,1,sizeof(areabak));
  memset(&MsgBase,'Z',sizeof(MsgBase));

  // Make sure the FIDO users are initialized
  init_users();

  clearLine(START_COL,STATUS_LINE);
  fastprint(START_COL,STATUS_LINE,"Importing Messages.",0x0d);
  clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);


  // Open the packet file
  if(dosfopen(filename,OPEN_DENYNONE|OPEN_RDWR,&pkt_file)==-1)
	return(FALSE);

  // read in packet header
  if(dosfread(&fido_header_type2,sizeof(fido_header_type2),&pkt_file)==-1)
  {
	dosfclose(&pkt_file);
	return(FALSE);
  }

  // Determine if it is type one or type two FIDO packet
  pkt_type = determineFidoPktType(fido_header_type2);

  // If it is type 1 the re-read in fido type 1 header
  if(pkt_type == 1)
	if(readType1PktHdr(fido_header,pkt_file) == FALSE)
	  return FALSE;


  // get packet info includeing

  if(pkt_type==2)
  {
	if(fido_header_type2.Orig_Net==-1)
	  fido_header_type2.Orig_Net=fido_header_type2.auxnet;

	if(fido_header_type2.OrigPoint != 0)
	  sprintf(addr,"%u:%u/%u.%u",fido_header_type2.OrigZone,fido_header_type2.Orig_Net,fido_header_type2.Orig_Node,fido_header_type2.OrigPoint);
	else
	  sprintf(addr,"%u:%u/%u",fido_header_type2.OrigZone,fido_header_type2.Orig_Net,fido_header_type2.Orig_Node);

	if(checkPacketPassword(addr,fido_header_type2.Password) == FALSE)
	{
	  dosfclose(&pkt_file);
	  moveFileToDir(filename,directory_info.bad_packets,"Bad Packets");
	  sprintf(logBuf,"Packet file: %-1.13s has an invalid password.",findstartofname(filename));
	  writeFidolog(logBuf,BLOCK);
	  freeAndNull(&buffer);
	  return(TRUE);
	}
  }
  else if(pkt_type==1)
  {
	if(fido_header.Orig_Zone!=0)
	  sprintf(addr,"%u:%u/%u",fido_header.Orig_Zone,fido_header.Orig_Net,fido_header.Orig_Node);
	else
	  sprintf(addr,"%u:%u/%u",address[0].zone,fido_header.Orig_Net,fido_header.Orig_Node);

	if(checkPacketPassword(addr,fido_header.Password) == FALSE)
	{
	  dosfclose(&pkt_file);
	  moveFileToDir(filename,directory_info.bad_packets,"Bad Packets");
	  sprintf(logBuf,"Packet file: %-1.13s has an invalid password.",findstartofname(filename));
	  writeFidolog(logBuf,BLOCK);
	  freeAndNull(&buffer);
	  return(TRUE);
	}
  }
  sprintf(messagebuff,"From: %s          ",addr);
  fastprint(43,MESSAGES-2,messagebuff,0x0b);

  if(PcbData.FidoEnableRouting && !(handle_routing(filename))) return FALSE;


  buffer = (char *) bmalloc(buflen);
  if(!buffer)
  {
	sprintf(logBuf,"Out of memory. Please increase available memory to PCBoard.");
	clearLine(START_COL,STATUS_LINE);
	clearLine(START_COL,LAST_ERROR);
	fastprint(START_COL,STATUS_LINE,logBuf,0x0d);
	fastprint(START_COL,LAST_ERROR,logBuf,0x0d);
	writeFidolog(logBuf,BLOCK);
	return FALSE;
  }

  long recno = find_fido_rec(addr,FALSE,FALSE,NULL);
  if(recno > 0) get_fido_rec(recno);


  while(1)
	{
	  nobad = FALSE;
	  memset(buffer,' ',buflen);
	  buffer[buflen-1] = '\x0';
	  // Now we are past the packet header and will start to read individual
	  // message headers
	  if(readFidoMessageHeader(fido_msg_hdr,pkt_file) == FALSE) break;


	  // We've read in the message header, now get the message body
	  if((read_fido_body(buffer,buflen,end))==NULL)
	  {
		//Something went wrong so free the message buffer
		//freeAndNull(&buffer);
		bfree(buffer);
		buffer = NULL;

		if(end!=TRUE)
		{
		  maxstrcpy(logBuf,"Could not allocate memory for message body. Aborting.",sizeof(logBuf));
		  clearLine(START_COL,STATUS_LINE);
		  clearLine(START_COL,LAST_ERROR);
		  fastprint(START_COL,STATUS_LINE,logBuf,0x0d);
		  fastprint(START_COL,LAST_ERROR,logBuf,0x0d);
		}
		else
		{
		  maxstrcpy(logBuf,"Unexpected end of packet. Jumping to next packet.",sizeof(logBuf));
		  fastprint(START_COL,STATUS_LINE,logBuf,0x0d);
		  fastprint(START_COL,LAST_ERROR,logBuf,0x0d);
		  if(update==TRUE) updateFidoUserRecord();
		  return(TRUE);
		}
		writeFidolog(logBuf,BLOCK);
		if(update==TRUE) updateFidoUserRecord();
		return(FALSE);
	  }


	  get_area(buffer,areabuf);

	  if(strcmp(areabuf,areabak)!=0)
	  {
		closemessagebase(&MsgBase);
		if(PcbData.FidoLogLevel >= 2 && numimp > 0)
		{
		  if(areabak[0] == 1) sprintf(messagebuff,"%d messages imported to area %-1.40s.",numimp,areabuf);
		  else	sprintf(messagebuff,"%d messages imported to area %-1.40s.",numimp,areabak);
		  writeFidolog(messagebuff,BLOCK);
		}
		if(cupdate==TRUE)
		{
		  cupdate=FALSE;
		  MsgReadPtr[conf]=MsgBase.Stats.HighMsgNum;
		}

		num += numimp;
		numimp = 0;

		maxstrcpy(areabak,areabuf,sizeof(areabak));

		// get area record for this areatag
		if((result=get_area_record(areabuf))!=FALSE)
		{
		  if(strcmpi(this_area.Area_Name,"NETMAIL")==0) netmail=TRUE;
		  else	netmail=FALSE;
		}
		// If the arearecord didn't exist, check to see if we need to autoadd it.
		else
		{
		   netmail=FALSE;
		   if(PcbData.FidoEnablePassThru)
		   {
		   cPTINFO PT;
			 bool areaExists = PT.seekToArea(areabuf);
			 if(areaExists == FALSE && PcbData.FidoAutoAdd && (PT.Err() == FALSE) )
			 {
			   PT.addArea(areabuf);
			   sprintf(logBuf,"Adding Passthru Area %s",areabuf);
			   writeFidolog(logBuf,BLOCK);
			 }
			 else if(areaExists == FALSE)
			 {
				 sprintf(logBuf,"Could not find tag %s. Tossing to BAD.",areabuf);
				 if(PcbData.FidoLogLevel >= 3) writeFidolog(logBuf,BLOCK);

				 if( (result = get_area_record("BAD")) == FALSE)  nobad = TRUE;
				 else				  maxstrcpy(areabuf,"BAD",sizeof(areabuf));
			 }
		   }

		}

		if( (netmail && secureNetmail && UsersData.Name[0] == '\x0') || (!netmail && UsersData.Name[0] == '\x0') )
		{
		  dosfclose(&pkt_file);
		  if(netmail && secureNetmail && UsersData.Name[0] == '\x0')
			moveFileToDir(filename,directory_info.securemail,"Secure Mail");
		  else
			moveFileToDir(filename,directory_info.bad_packets,"Bad");

		  sprintf(messagebuff,"User record for node: %-1.25s can't be found.",addr);


		  clearLine(START_COL,STATUS_LINE);
		  clearLine(START_COL,LAST_ERROR);
		  fastprint(START_COL,STATUS_LINE,messagebuff,0x0d);
		  fastprint(START_COL,LAST_ERROR,messagebuff,0x0d);
		  writeFidolog(messagebuff,BLOCK);
		  bfree(buffer);
		  buffer = NULL;
		  if(update==TRUE) updateFidoUserRecord();
		  return(FALSE);
		}

		if(result==FALSE || (netmail==FALSE && !(isset(&ConfReg[REG],this_area.PCB_Conference))))
		{

		  bool bad = FALSE;
		  if(strcmp(areabuf,"BAD") == 0) bad = TRUE;
		  if(result==TRUE && !bad)
		  {
			sprintf(logBuf,"Node: %s does not have access to conference: %-1.40s.",addr,areabuf);
			clearLine(START_COL,STATUS_LINE);
			clearLine(START_COL,LAST_ERROR);
			fastprint(START_COL,STATUS_LINE,logBuf,0x0d);
			fastprint(START_COL,LAST_ERROR,logBuf,0x0d);
			writeFidolog(logBuf,BLOCK);
		  }
		  else if (PcbData.FidoEnablePassThru && !bad)
		  {
			cPTINFO 	  ptinfo;
			cPASSTHROUGH  pt;

		   if(ptinfo.seekToArea(areabuf))
		   {
			pt.addMsg(&fido_msg_hdr,buffer);
			passthru = TRUE;
			if(PcbData.FidoLogLevel >= 2) writeFidolog("Adding message to passthru file.",BLOCK);
		   }
		   maxstrcpy(areabuf,"Passthru",sizeof(areabuf));
		   areabak[0]='\x0';
		   sprintf(messagebuff,"Importing message to passthru file.");
		   clearLine(START_COL,MESSAGES);
		   fastprint(START_COL,MESSAGES,messagebuff,0x0b);
		   continue;

		  }
		  else if(!passthru && get_area_record("BAD")==FALSE)
			nobad=TRUE;
		  else
		  {
			nobad=FALSE;
			strcpy(areabuf,"BAD");
			strcpy(areabak,"BAD");
		  }
		}
		if(nobad==FALSE)
		{
		  conf=this_area.PCB_Conference;
		  getconfrecord(conf,&confrec);
		  if(confrec.Name[0] == '\x0' || fileexist(confrec.MsgFile) == 255 || openmessagebase(conf,&confrec,&MsgBase,RDWRLOCK)==-1)
		  {
			sprintf(logBuf,"Could not open message base for: %-1.20s",this_area.Area_Name);
			writeFidolog(logBuf,BLOCK);
			clearLine(START_COL,STATUS_LINE);
			clearLine(START_COL,LAST_ERROR);
			fastprint(START_COL,STATUS_LINE,logBuf,0x0d);
			fastprint(START_COL,LAST_ERROR,logBuf,0x0d);
			memset(areabak,0,sizeof(areabak));
			continue;
		  }

		  if(confrec.ConfType != 5)  continue;

		  if(MsgBase.Stats.HighMsgNum == (MsgReadPtr[conf]) )
		  {
			update=TRUE;
			cupdate=TRUE;
		  }
		}
	  }
	  clsbox(START_COL,MESSAGES,79,MESSAGES,0x00);

	  if(nobad==FALSE)
	  {
		sprintf(messagebuff,"Importing message %ld to area: %-1.40s",totattimp,this_area.Area_Name);
	  }
	  else
		sprintf(messagebuff,"Throwing away message %ld. No BAD conference found.",num);

	  clearLine(START_COL,MESSAGES);
	  fastprint(START_COL,MESSAGES,messagebuff,0x0b);

	  if(nobad==FALSE && buffer) toss_message(buffer,&MsgBase);
	  if(cerr) break;


	  if(end==TRUE)
	  {
		if(cupdate==TRUE)
		{
		  cupdate=FALSE;
		  MsgReadPtr[conf]=MsgBase.Stats.HighMsgNum;
		  MsgReadPtr[conf]++;
		}
		break;
	  }
	}
  if(nobad==FALSE) closemessagebase(&MsgBase);

  sprintf(logBuf,"%d messages imported from this packet.",num);
  writeFidolog(logBuf,BLOCK);
  //freeAndNull(&buffer);
  bfree(buffer);
  buffer = NULL;
  grandtotalimported+=num;

  if(update==TRUE) updateFidoUserRecord();
  if(!cerr) return(TRUE);
  else		return(FALSE);
}

/*****************************************************************************/
/* Save the rest of the messages in the packet. 							 */

void LIBENTRY  cTOSSER::save_rest(void)
{
DOSFILE 			new_pkt;
systimetype 		timerec;
sysdatetype 		daterec;
int 				len;
char				name[100],buffer[2048],fname[20];

  checkstack();
  memset(buffer,0,sizeof(buffer));
  getsysdate(&daterec);
  getsystime(&timerec);

  sprintf(fname,"%02d%02d%02d%02X.PKT",daterec.Day,timerec.Hours,timerec.Minutes,timerec.Seconds);
  stripright(directory_info.work_directory,' ');
  strcpy(name,directory_info.work_directory);
  strcat(name,fname);

  if(dosfopen(name,OPEN_CREATE|OPEN_RDWR|OPEN_DENYWRIT,&new_pkt)==-1)
	{
	  dosfclose(&new_pkt);
	  return;
	}

  if(pkt_type==2)
	{
	  if(dosfwrite(&fido_header_type2,sizeof(FIDO_PACKET_HEADER_TYPE2),&new_pkt)==-1)
		{
		  dosfclose(&new_pkt);
		  return;
		}
	}
  else if(pkt_type==1)
	{
	  if(dosfwrite(&fido_header,sizeof(FIDO_PACKET_HEADER),&new_pkt)==-1)
		{
		  dosfclose(&new_pkt);
		  return;
		}
	}

  while(1)
	{
	  len=dosfread(buffer,sizeof(buffer),&new_pkt);
	  dosfwrite(buffer,sizeof(buffer),&new_pkt);
	  if(len<sizeof(buffer))
		break;
	}
  dosfclose(&new_pkt);
}

/*****************************************************************************/
/* Toss all PKT files.																																																					 */

bool   LIBENTRY  cTOSSER::importMessages(void)
{
struct find_t	  file_block;
char			  filename[80];
char	 *		  ptr = NULL;
int 			  screen;


  checkstack();
  setcursor(CUR_BLANK);
  // Limit Q's scope
  {
    #if __BORLANDC__ >= 0x500
      fido_scanqueue();
    #else
      cNEWQ Q;
      Q.scanQueue();
    #endif
  }
  banner_disp=TRUE;
  memset(filename,0,sizeof(filename));
  assert(stackavail() > 2000);

  screen=memsavescreen();
  openlog();
  if(read_fido_config(NODE_RECS|ADDR_RECS)==FALSE)
  {
	memfreescreen(screen);
	return(FALSE);
  }

  cls();
  initUMWF();
  print_banner();

  sprintf(messagebuff,"Searching for and copying files for import...",DTA.ff_name);
  clearLine(START_COL+12,MESSAGES);
  fastprint(START_COL,STATUS_LINE,messagebuff,0x0b);

  look_for_files();

  directory_info.work_directory[sizeof(directory_info.work_directory)-1]=NULL;
  stripright(directory_info.work_directory,' ');
  sprintf(filename,"%s*.PKT",directory_info.work_directory);
  assert(strlen(filename) < sizeof(filename));

  bool logged = FALSE;
  int i = 0;

  #define NUM  100
  FILEDATETIME	importFiles[NUM];
  int numFiles = 0;

  memset(importFiles,0,sizeof(FILEDATETIME)*NUM);
  generateImportList(filename,importFiles,numFiles);

  for(i = 0;i<numFiles;i++)
	{
	  if(!logged)
	  {
		sprintf(messagebuff,"Beginning FIDO mail import.");
		writeFidolog(messagebuff,BLOCK_START);
		logged = TRUE;
	  }
	  if(banner_disp==TRUE)
		{
		  print_banner();
		  banner_disp=FALSE;
		}

		  if(importFiles[i].fName[0] != '\x0') sprintf(filename,"%s%s",directory_info.work_directory,importFiles[i].fName);
		  else continue;
		  assert(strlen(filename) < sizeof(filename));

		  if(fileexist(filename) == 255)
		  {
			sprintf(messagebuff,"%s does not exist.",file_block.name);
			writeFidolog(messagebuff,BLOCK);
			continue;
		  }

		  sprintf(messagebuff,"Importing Packet :%s",DTA.ff_name);
		  clearLine(START_COL+12,MESSAGES);
		  fastprint(12,MESSAGES-2,messagebuff,0x0b);
		  if(DTA.ff_fsize == 0)
		  {
			  sprintf(messagebuff,"%s is still being received.",file_block.name);
			  writeFidolog(messagebuff,BLOCK);
			  if(file_block.name[0] != '\x0')
			  {
				unlink(filename);
			  }
			  continue;
		  }


		  if(toss_packet(filename)==FALSE)
			{
			  dosfclose(&pkt_file);

			  ptr=strrchr(filename,'\\');

			  if(ptr!=NULL)
				ptr++;
			  else
				ptr=filename;

			  if(fileexist(filename) != 255)
			  {
				sprintf(messagebuff,"Could not import packet %s. Moved to BAD dir.",ptr);
				writeFidolog(messagebuff,BLOCK);
				char badDir[MAXFLEN];
				maxstrcpy(badDir,directory_info.bad_packets,sizeof(badDir)-13);
				strcat(badDir,ptr);
				pcbmovefile(filename,badDir);
			  }
			  continue;

			}
		  else
			{
			  clsbox(START_COL,MESSAGES,78,MESSAGES,0x00);
			  fastprint(START_COL,MESSAGES,"Done.",0x0e);

			  sprintf(messagebuff,"Packet %s imported.",findstartofname(filename));

			  writeFidolog(messagebuff,BLOCK);
			  dosfclose(&pkt_file);
			  unlink(filename);
			}
			if(kbdhit(NOBUFFER) == ESC_KEY)
			{
				i++;
				break;
			}
	}
  free_fido_memory();
  doUMWF();
  resetUMWF();
  resetuser();
  {
    bool keyboard = FALSE;
    fgetbyte(SIXSECONDS,keyboard);
  }
  memrestorescreen(screen,FREESCREEN);
  if(i>0)
  {
	sprintf(messagebuff,"%ld total messages imported.",grandtotalimported);
	writeFidolog(messagebuff,BLOCK);
	writeFidolog("Ending message import.",BLOCK_END);
  }
  setcursor(CUR_BLANK);
  closecallerlog();
  return(TRUE);
}

int LIBENTRY cTOSSER::handle_routing(char * filename)
{
	char znnBuf[25];
	char srcDir[55],destDir[55];
	char *chPtr1;
	QUEUE_RECORD q_rec;
	int i;

	checkstack();
	memset(&q_rec,0,sizeof(QUEUE_RECORD));
	memset(znnBuf,49,sizeof(znnBuf));
	chPtr1 = filename;
	//strip out any path info
	if( (chPtr1 = strrchr(filename,'\\')) != NULL) ++chPtr1;

	cAKAS	 akas;
	NADDRESS rec;
	unsigned int numrecs=akas.totRecs();
	switch(pkt_type)
	{
	   case 1:
		  // See if this packet is for me. Check all akas.
		  // If it's for me then return TRUE and continue processing
		  sprintf(znnBuf,"*:%u/%u ",fido_header.Dest_Net, fido_header.Dest_Node);
		  for(i=1;i<=numrecs;i++)
		  {
			rec = akas.getRec(i);
			if(rec.net == fido_header.Dest_Net && rec.node ==fido_header.Dest_Node )
			  return TRUE;
		  }

		  break;
	   case 2:
		  sprintf(znnBuf,"%u:%u/%u",fido_header_type2.DestZone,fido_header_type2.Dest_Net, fido_header_type2.Dest_Node);

		  for(i=1;i<=numrecs;i++)
		  {
			rec = akas.getRec(i);
			if(rec.zone == fido_header_type2.Dest_Zone && rec.net == fido_header_type2.Dest_Net && rec.node ==fido_header_type2.Dest_Node )
			  return TRUE;
		  }

		  break;
	   default:
		  return TRUE;
	}

	// If it's not for me see if I can route it
	char destAddress[50];
	maxstrcpy(destAddress,znnBuf,sizeof(destAddress));
	long flags = get_event_info(znnBuf);
	if(flags & ALLOW_ROUTE)
	{
		// If I can then add it to the master queue and retrun FALSE
		// copy file into outbound dir
		sprintf(srcDir,"%s",filename);
		sprintf(destDir,"%s%s",directory_info.outgoing_packets,findstartofname(filename));
		dosfclose(&pkt_file);
		pcbmovefile(srcDir, destDir);

		// Fill in structure.
		maxstrcpy(q_rec.filename,destDir,sizeof(q_rec.filename));
		maxstrcpy(q_rec.nodestr,znnBuf,sizeof(q_rec.nodestr));
		re_route(q_rec.nodestr);
		q_rec.flag = (Q_KILLSENT | Q_NORMAL);

		// Limit Q's scope
		{
          #if __BORLANDC__ >= 0x500
            fido_addqueue(q_rec);
          #else
            cNEWQ Q;
            Q.addEntry(q_rec);
          #endif
		}
		sprintf(messagebuff,"Routing file:%s for %-1.25s to %-1.25s.",findstartofname(q_rec.filename),destAddress,q_rec.nodestr);
		writeFidolog(messagebuff,BLOCK);
		return FALSE;
	}
	// If I can't then write to log and return FALSE
	else
	{
		sprintf(messagebuff,"Cannot route file %-1.14s for %-1.25s to address %-1.25s.",chPtr1,destAddress,znnBuf);
		writeFidolog(messagebuff,BLOCK);
		// Mark as hold.
		sprintf(srcDir,"%s",filename);
		sprintf(destDir,"%s%s",directory_info.outgoing_packets,findstartofname(filename));
		dosfclose(&pkt_file);
		pcbmovefile(srcDir, destDir);
		maxstrcpy(q_rec.filename, destDir,sizeof(q_rec.filename));
		strcpy(q_rec.nodestr,znnBuf);
		re_route(q_rec.nodestr);
		q_rec.flag = Q_KILLSENT | Q_HOLD;

		// Limit Q's scope
		{
          #if __BORLANDC__ >= 0x500
            fido_addqueue(q_rec);
          #else
            cNEWQ Q;
            Q.addEntry(q_rec);
          #endif
		}
		return FALSE;
	}
}

bool LIBENTRY cTOSSER::exportPassthru(const char * userName)
{
cPASSTHROUGH P;
cPTINFO 	 I;
FUSERS		 user;
bool		 sentmsg = FALSE;

	if(P.needToPass() == FALSE) return FALSE;
	fido_nodestr_to_int((char *)userName,user.zone,user.net,user.node,user.point);


	sprintf(logBuf,"Processing passthough areas for node %s                  ",userName);
	clearLine(0,STATUS_LINE);
	fastprint(0,STATUS_LINE-1,logBuf,0x0f);

	// Prepare packet file
	if(open_pkt_file()==FALSE) return FALSE;
	write_pkt_header((char *)userName);

	//while there are more messages for this user
	while(P.getNextMsgForUser(user,pkt_file,I,sentmsg) != 0);
	dosfwrite(zeros,1,&pkt_file);
	dosfclose(&pkt_file);
	return sentmsg;
}

int LIBENTRY cTOSSER::getNewAddr(int oldaka,const char * command)
{
char  addr[25];
char *nullit=NULL;
uint zone=0,net=0,node=0,point=0;

  maxstrcpy(addr,(char *)command+1,sizeof(addr));
  nullit = strchr(addr,']');
  if(!nullit) return oldaka;
  *nullit = '\x0';
  fido_nodestr_to_int(addr,zone,net,node,point);
  for(int i=0;i<num_akas;i++)
  {
	if(address[i].zone == zone && address[i].net == net && address[i].node == node && address[i].point == point)
	  return i;
  }
  return oldaka;
}


bool LIBENTRY cTOSSER::allowPrvMsgs(const char * areaname)
{
cAREAS		 areas;
NAREA_STRUCT area;
unsigned int areanum=0;

  areanum = areas.findRec(areaname);
  area	  = areas.getRec(areanum);
  return	area.AllowPrivate == 'Y';
}



#endif
