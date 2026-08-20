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






			 Miscellanious suppprt functions for the tosser
						  Author: Stan Paulsen
					Copyright Clark Development 1995


*****************************************************************************/

#ifdef FIDO
// Borland headers
#include <mem.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
//#include <malloc.h>
#include <dos.h>
#include <time.h>

// PCBoard headers

#include "system.h"
#include "misc.h"
#include "bug.h"
#include "pcb.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "messages.h"
#include "screen.h"
#include "users.h"
#include "pcbmsgs.hpp"
#include "msgstub.hpp"
#include "validate.h"

#ifdef __OS2__
 #define  INCL_DOSPROCESS
 #include <os2.h>
#endif

// Fido Headers
#include "tossmisc.h"
#include "pcbtoss.hpp"
//#include "structs.h"
//#include "defines.h"
#include "prototyp.h"
#include "fidoque.hpp"
#include "passthru.hpp"
#include "byte.h"


#define min(a,b)	(((a) < (b)) ? (a) : (b))

static	   bool 		usersinitialized;
static _FAR_ FIDOUSER	* users = NULL;
extern	   struct		ffblk DTA;
//extern	 THIS_ADDRESS *address;
extern	   NADDRESS *address;
extern	   unsigned int num_akas;
extern _FAR_ DIRECTORIES  directory_info;


	   void 		 LIBENTRY	writeChangesToFile(cDOSFILE & file);
	   void 		 LIBENTRY	writeAreasToFile(cDOSFILE & file,bool allareas);
static bool _NEAR_	 LIBENTRY	findMyHub(FUSERS & user,FUSERS & hub, char * afixPwrd);
static void _NEAR_	 LIBENTRY	compfidomsgbuf ( char * buf );

#ifdef TOSSCLASS
// Highest level function for the exporter

bool LIBENTRY copy_pcb_messages(void)
{
  bool retval = FALSE;
  cTOSSER * tosser=NULL;


	tosser = new(cTOSSER);

	if(!tosser)
	{
	  writeFidolog("Not enough memory available for export!",BLOCK_START);
	  return FALSE;
	}

	setcursor(CUR_BLANK);

	#ifdef __OS2__
	  adjustpriority(PcbData.PriorityFidoOut);
	#endif

	retval = tosser->exportMessages();

	#ifdef __OS2__
	  adjustpriority(PcbData.PriorityNormal);
	#endif

	setcursor(CUR_NORMAL);
	if(tosser) delete(tosser);
	return retval;
}

bool LIBENTRY  toss_messages(void)
{
bool retval = FALSE;
cTOSSER * tosser=NULL;

	tosser = new(cTOSSER);
	if(!tosser)
	{
	   writeFidolog("Not enough memory available for export!",BLOCK_START);
	   return FALSE;
	}
	setcursor(CUR_BLANK);

	#ifdef __OS2__
	  adjustpriority(PcbData.PriorityFidoIn);
	#endif
	retval = tosser->importMessages();
	#ifdef __OS2__
	  adjustpriority(PcbData.PriorityNormal);
	#endif
	setcursor(CUR_NORMAL);
	if(tosser) delete(tosser);
	return retval;

}

#endif

// Writes FIDo logging information to the PCBoard caller log

void LIBENTRY  writeFidolog(const char * entry, int type)
{

	char   date_time_buf[100];
	char   tmpDate[30];
	char   tmpTime[30];

	checkstack();

	// $$$ adding time-date stamp to all fido log entries

	// If it is the start of a block the put full date in, otherwise just
	// put dashes
	if(type == BLOCK_START || type == BLOCK_END)
	  datestr(tmpDate);
	else
	  strcpy(tmpDate,"");

	timestr1(tmpTime);
	sprintf(date_time_buf,"%-1.9s %-8.8s %-1.65s",tmpDate,tmpTime,entry);
	assert(strlen(date_time_buf) < sizeof(date_time_buf));

	writelog(date_time_buf,LEFTJUSTIFY);

	if(type == BLOCK_END)
	  writelog(LogDivider,LEFTJUSTIFY);
}


void LIBENTRY getSysopNam(char * nameBuf)
{
	if(tempuseralloc(FALSE) == -1) return;
	if(gettemprecord(1) 	== -1) return;
	strcpy(nameBuf,TempData->Name);
	tempuserdealloc();

}


void LIBENTRY  checkDateFormat(char * date)
{
	char * tPtr = NULL;
	bool   badFormat = FALSE;
	char   logBuf[65];

	tPtr = date;

	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(*tPtr++ != '-')    badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(*tPtr++ != '-')    badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(!isdigit(*tPtr))   badFormat = TRUE;

	if(badFormat )
	{
	   if(PcbData.FidoLogLevel >= 3)
	   {
		 sprintf(logBuf,"Garbled date field (%-1.10s). Inserting todays date.",date);
		 writeFidolog(logBuf,BLOCK);
	   }
	   maxstrcpy(date,juliantodate(getjuliandate()),8);
	}
}
void LIBENTRY  checkTimeFormat(char * time)
{

	char * tPtr = NULL;
	bool   badFormat = FALSE;
	char   logBuf[65];

	if(!time) return;
	tPtr = time;

	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(*tPtr++ != ':')    badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;
	if(!isdigit(*tPtr++)) badFormat = TRUE;

	if(badFormat )
	{
	  if(PcbData.FidoLogLevel >= 3)
	  {
	   sprintf(logBuf,"Garbled time field (%-1.10s). Inserting new time.",time);
	   writeFidolog(logBuf,BLOCK);
	  }
	  timestr2(time);
	}
}

/*****************************************************************************/
/* Copy a file. 															 */

void LIBENTRY  TK_CopyFile(char _FAR_ *source,char _FAR_ *destination)
{
  checkstack();
  if(!source || !destination) return;
  pcbcopyfile(source,destination,FALSE,FALSE,NULL,FALSE);
}

/****************************************************************************/
/* Load in the user record in recno.										*/

void LIBENTRY  get_fido_rec(long recno)
{

  checkstack();
  //if(Status.UserRecNo == recno) return;
  Status.UserRecNo = recno;
  getuserrecord(FALSE,FALSE);
}

/****************************************************************************/
/* Change PCBoard date format to Fido Date/Time format. 																				*/

void LIBENTRY  translate_date(char *fido_date,char *pcb_date,char *time,int fidolen)
{
char static months[12][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
char *tptr=NULL,buffer[50],pcb_datebuf[9],timebuf[6];
int  len;

  checkstack();
  memset(buffer,0,sizeof(buffer));
  memset(pcb_datebuf,0,sizeof(pcb_datebuf));
  memset(timebuf,0,sizeof(timebuf));
  memset(fido_date,0x20,fidolen);


  if(!fido_date || !pcb_date || !time) return;

  memcpy(pcb_datebuf,pcb_date,8);
  memcpy(timebuf,time,5);

  if((tptr=strtok(pcb_datebuf,"-"))!=NULL)
	if((tptr=strtok(NULL,"-"))!=NULL)     maxstrcpy(buffer,tptr,sizeof(buffer));



  memcpy(pcb_datebuf,pcb_date,8);
  if((tptr=strtok(pcb_datebuf,"-"))!=NULL)
  {
	  strcat(buffer," ");
	  strcat(buffer,months[(atoi(tptr)-1)%12]);
  }

  if((tptr=strtok(NULL,"-"))!=NULL)
  {
	if((tptr=strtok(NULL," "))!=NULL)
	{
		strcat(buffer," ");
		if(strlen(tptr) + strlen(buffer) < sizeof(buffer)-9) strcat(buffer,tptr);
	}
  }
  len=strlen(buffer);
  buffer[len]=0x20;
  buffer[len+1]=0x20;
  buffer[len+2]=NULL;
  strcat(timebuf,":00");
  strcat(buffer,timebuf);

  maxstrcpy(fido_date,buffer,fidolen);
  fido_date[strlen(fido_date)]=0x20;
  fido_date[fidolen-1]=NULL;
}

/*****************************************************************************/
/* Match two fido addresses. Wildcard match supported.						 */

bool LIBENTRY  match_address(char *oldaddr,char *newaddr)
{
char	str[20];
int 	i,j,oldzone,oldnet,oldnode,oldpoint,newzone,newnet,newnode,newpoint;
bool	zonematch,netmatch,nodematch,pointmatch,point;

  checkstack();
  zonematch=netmatch=nodematch=pointmatch=point=FALSE;
  oldzone=oldnet=oldnode=oldpoint=newzone=newnet=newnode=newpoint=0;

  for(i=0,j=0;j<sizeof(str);i++,j++)
	{
	  switch(oldaddr[i])
		{
		  case	':' :
					  str[j]=NULL;
					  if(str[0]=='*')
						zonematch=TRUE;
					  else
						oldzone=atoi(str);
					  j=-1;
					  break;

		  case	'/' :
					  str[j]=NULL;
					  if(str[0]=='*')
						netmatch=TRUE;
					  else
						oldnet=atoi(str);
					  j=-1;
					  break;

		  case	'.' :
					  point=TRUE;
					  str[j]=NULL;
					  if(str[0]=='*')
						nodematch=TRUE;
					  else
						oldnode=atoi(str);
					  j=-1;
					  break;


		  case	NULL:
					  if(point==FALSE)
						{
						  str[j]=NULL;
						  if(str[0]=='*')
							nodematch=TRUE;
						  else
							oldnode=atoi(str);
						}
					  else
						{
						  str[j]=NULL;
						  if(str[0]=='*')
							pointmatch=TRUE;
						  else
							oldpoint=atoi(str);
						}
					  j+=sizeof(str);		// Get out of the loop.
					  break;

		  default	:
					  str[j]=oldaddr[i];
					  break;
		}
	}
  point=FALSE;
  for(i=0,j=0;j<sizeof(str);i++,j++)
	{
	  switch(newaddr[i])
		{
		  case	':' :
					  str[j]=NULL;
					  newzone=atoi(str);
					  j=-1;
					  break;

		  case	'/' :
					  str[j]=NULL;
					  newnet=atoi(str);
					  j=-1;
					  break;

		  case	'.' :
					  point=TRUE;
					  str[j]=NULL;
					  if(str[0]=='*')
						nodematch=TRUE;
					  else
						newnode=atoi(str);
					  j=-1;
					  break;


		  case	NULL:
					  if(point==FALSE)
						{
						  str[j]=NULL;
						  if(str[0]=='*')
							nodematch=TRUE;
						  else
							newnode=atoi(str);
						}
					  else
						{
						  str[j]=NULL;
						  if(str[0]=='*')
							pointmatch=TRUE;
						  else
							newpoint=atoi(str);
						}
					  j+=sizeof(str);		// Get out of the loop.
					  break;

		  default	:
					  str[j]=newaddr[i];
					  break;
		}
	}

  if((oldzone==newzone || zonematch==TRUE) && (oldnet==newnet || netmatch==TRUE) && (oldnode==newnode || nodematch==TRUE) && (oldpoint==newpoint || pointmatch==TRUE))
	return(TRUE);

  return(FALSE);
}

/****************************************************************************/
/* Print out screen layout													*/

void LIBENTRY  print_banner(void)
{
char		buffer[80];

  checkstack();
  memset(buffer,0,sizeof(buffer));
  assert(stackavail() > 2000);

  //cls();
  sprintf(buffer,"PCBoard<->Fidonet Importer/Exporter - Version %s.%s",VERSION_MAJOR,VERSION_MINOR);
  fastcenter(0,buffer,0x0a);
  fastcenter(1,"Copyright 1995-1996  Clark Development Company, Inc.",0x0e);
  fastprint(0,STATUS_LINE,"Status    :",0x0f);
  fastprint(0,MESSAGES,   "Message   :",0x0f);
  fastprint(0,LAST_ERROR, "Last Error:",0x0f);

  if(EnableMSG && !PcbData.FidoCreateMsg)
   fastprint(0,LAST_ERROR+1, "*.MSG Enabled",0x0f);
  gotoxy(0,10);
}

bool LIBENTRY  isEmptyMessage(const char * message)
{
	char * walkPtr = NULL;

	walkPtr = (char *)message;

	while (*walkPtr != '\x0')
	{
	  if (*walkPtr == '\x01')
	  {
		while ((*walkPtr != '\r') && (*walkPtr != '\0') && (*walkPtr != LineSeparator))
		  ++walkPtr;
		if (*walkPtr == '\0')
		  break;
	  }
	  if (*walkPtr != ' ' && *walkPtr != '\r' && (*walkPtr != LineSeparator)) return FALSE;
	  walkPtr++;
	}
	return TRUE;
}

void LIBENTRY  createAttach(const unsigned int confNum,const char * fileName, bool move)
{

   char 		oldFile[MAXFLEN];
   char 		newFile[MAXFLEN];
   char 		attFile[50];
   pcbconftype	conference;

   memset(newFile,0,sizeof(newFile));
   memset(oldFile,0,sizeof(oldFile));

   if(fileexist((char *)fileName) == 255) return;

   getconfrecord(confNum,&conference);

   maxstrcpy(oldFile,(char *)fileName,sizeof(oldFile));
   sprintf(newFile,"%s%s",conference.AttachLoc,findstartofname((char *)fileName));
   if(move)
	 pcbmovefile(oldFile,newFile);
   else
	 TK_CopyFile(oldFile,newFile);
   makefilenameunique(oldFile,newFile);
   sprintf(attFile,"%s (%ld) %s",findstartofname(newFile),DTA.ff_fsize,findstartofname(oldFile));
   buildextheader(EXTHDR_ATTACH,attFile,HDR_FILE);
}


void LIBENTRY generateImportList(char * fileSpec,FILEDATETIME * list,int & numFiles)
{
	ffblk	fblk;
	int 	done = 1;
	int 	dosfindmax=1;
    int     i;
	#ifdef __OS2__
	int DirHandle = SYSTEMDIRHANDLE;
	#endif

	checkstack();
	memset(list,0,sizeof(list));

	dosfindmax = 1;
	done = dosfindfirst(fileSpec,&fblk,0,&dosfindmax PDIRHANDLE);
	if(done)
	{
	  numFiles = 0;
	  return;
	}
    for(i = 0; i < NUMPKTS; i++)
	{
		if(!done)
		{
			strcpy(list[i].fName,fblk.ff_name);
			list[i].fTime = fblk.ff_ftime;
			list[i].fDate = fblk.ff_fdate;
		}
		else
			break;
		done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
	}
	numFiles = i;
	qsort((void *)list,i,sizeof(list[0]),importListCmp);
}

int importListCmp(const void * a, const void *b)
{
	FILEDATETIME *aa,*bb;

	checkstack();
	aa = (FILEDATETIME *) a;
	bb = (FILEDATETIME *) b;

	int dCmp = aa->fDate - bb->fDate;
	return ((dCmp == 0) ? (aa->fTime - bb->fTime) : dCmp);
}

static char lookup [] =
{
	   'C',    'u',    'e',    'a',    'a',    'a',    'a',    'c',
	   'e',    'e',    'e',    'i',    'i',    'i',    'A',    'A',
	   'E',    'a',    'A',    'o',    'o',    'o',    'u',    'u',
	   'y',    'O',    'U',    'c',    '#',    'Y',    'P',    'f',
	   'a',    'i',    'o',    'u',    'n',    'N',    'a',    'o',
	   '?',    '-',    '-',    '2',    '4',    '!',    '<',    '>',
	   '#',    '#',    '#',    '|',    '+',    '+',    '+',    '+',
	   '+',    '+',    '|',    '+',    '+',    '+',    '+',    '+',
	   '+',    '+',    '+',    '+',    '-',    '+',    '+',    '+',
	   '+',    '+',    '+',    '+',    '+',    '-',    '+',    '+',
	   '+',    '+',    '+',    '+',    '+',    '+',    '+',    '+',
	   '+',    '+',    '+',    '#',    '_',    '(',    ')',    '~',
	   'a',    'B',    'T',    'r',    'E',    'o',    'u',    't',
	   '0',    '0',    'O',    'o',    '%',    '0',    'e',    'U',
	   '=',    '+',    '>',    '<',    'f',    'f',    '/',    '=',
	   'O',    'o',    '.',    'V',    'n',    '2',    '*',    ' '
};

char * LIBENTRY xlateFidoText(char * s, char HighAsciiSetting)
{



	char * p = s;

	while (*s)
	{
		if (*s > 127)
			switch (HighAsciiSetting)
			{
				case 'S':
					strcpy(s,s+1);
					continue;

				case 'R':
					*s = '.';
					break;

				case 'C':
					*s = lookup[*s-127];
					break;
			}

		++s;
	}

	return p;
}



bool LIBENTRY  re_route(char * address)
{

	cDOSFILE  sts;
	char	  filename[20];
	char	  addr1[50];
	char	  addr2[50];
	char	  newAddr[50];
	char	  line[90];
	char *	  tptr1 = NULL;
	char *	  tptr2 = NULL;
	bool	  needToRoute = FALSE;


	maxstrcpy(filename,"PCBFIDO.STS",sizeof(filename));
	if(fileexist(filename) == 255 || sts.open(filename,OPEN_READ | OPEN_DENYNONE) != 0)
	  return FALSE;

	// Read in all current event information out of pcbfido.sts
	while( (sts.getln(line,sizeof(line))) != -1)
	{
	  // If this is a route-to verb then see if we need to route this address
	  strupr(line);
	  if(strstr(line,"ROUTE-TO") && !strstr(line,"ALLOW"))
	  {
		tptr1 = strchr(line,',');
		if(tptr1)
		{
		   // Isolate the first address in the Route-To line
		   ++tptr1;
		   while(*tptr1 == ' ') tptr1++;
		   tptr2 = tptr1;
		   while(*tptr2 != ' ' && *tptr2 != ',' && *tptr2 != '\x0') tptr2++;
		   maxstrcpy(addr1,tptr1,size_t(tptr2-tptr1)+1);

		   // Does the parameter address match the first Route-To address?
		   if(match_address(addr1,address) && !isExcluded(address,line))
		   {
			  while(*tptr2 == ' ') tptr2++;
			  maxstrcpy(addr2,tptr2,sizeof(addr2));
			  stripright(addr2,' ');
			  strcpy(newAddr,addr2);
			  needToRoute = TRUE;
		   }
		}
	  }
	}
	if(needToRoute)
	{
	   maxstrcpy(address,newAddr,25);
	   return TRUE;
	}
	return FALSE;

}

bool LIBENTRY isExcluded(char * addr, char * list)
{
  char * walkPtr = NULL;
  char * endPtr  = NULL;

  char compaddr[25];

	 walkPtr = addr;
	 while(*walkPtr == ' ' && *walkPtr != '\x0') walkPtr++;
	 while(*walkPtr != ' ' && *walkPtr != '\x0') walkPtr++;
	 *walkPtr = '\x0';
	 walkPtr = strstr(list,"EXCLUDE");
	 if(walkPtr) endPtr = walkPtr;
	 else		 return FALSE;
	 *walkPtr++ = '\x0';

	// walk the pointer to the first digit in the address
	while(!isdigit(*walkPtr) && *walkPtr != '\x0') walkPtr++;
	if(*walkPtr == '\x0') return FALSE;
	endPtr = walkPtr;
	// Walk the end pointer to the end of the address;
	while(*endPtr != ' ' && *endPtr != '\x0') endPtr++;

	maxstrcpy(compaddr,walkPtr,min(size_t(endPtr-walkPtr+1),sizeof(compaddr)));

	if(match_address((char *)compaddr,(char *)addr)) return TRUE;

	return FALSE;
}







char * LIBENTRY makeUniqueFZipName ( char * name )
{
int dosfindmax=1;
#ifdef __OS2__
int DirHandle = SYSTEMDIRHANDLE;
#endif
	// Initialize day name abbreviations
	char * dayAbbr [] = { "SU", "MO", "TU", "WE", "TH", "FR", "SA" };

	// Get the day of week number (0 = SU)
	int dow = getjuliandate() % 7;

	// A temporary pointer for our use
	char * p;

	// Extract path info from name
	char path [ 128 + 1 ];
	strcpy(path,name);
	p = strrchr(path,'\\');
	if (p)
		p[1] = 0;
	else if (path[1] == ':')
		path[2] = 0;
	else
		path[0] = 0;

	// Mask for files to search
	strcat(name,".*");

	// Make temporary mask for searching
	char temp [ 128 + 1 ];
	strcpy(temp,path);
	strcat(temp,"*.*");

	// Loop through files
	ffblk curFFB;
	dosfindmax = 1;
	int done = dosfindfirst(temp,&curFFB,0,&dosfindmax PDIRHANDLE);
	while (!done)
	{
		// Position on extension
		p = curFFB.ff_name;
		p += strlen(p)-3;

		// If the days don't match and file size is 0, delete the file
		if (((p[0] != dayAbbr[dow][0]) || (p[1] != dayAbbr[dow][1])) &&
			(curFFB.ff_fsize == 0))
		{
			char buf [ 128 + 1 ];
			strcpy(buf,path);
			strcat(buf,curFFB.ff_name);
			unlink(buf);
		}

		// Next file
		done = dosfindnext(&curFFB,&dosfindmax PDIRHANDLE2);
	}

	// Position on wildcard
	p = name;
	p += strlen(p)-1;

	// Copy zeroth extension
	strcpy(p,dayAbbr[dow]);
	p[2] = '0';
	p[3] = 0;

	// Loop until we find a unique extension
	while ((fileexist(name) != 255) && (DTA.ff_fsize == 0 || DTA.ff_fsize >= 50000L))
	{
		if (++p[2] == '9'+1) p[2] = 'A';
	}

	// Return unique name
	return name;
}

/****************************************************************************/
/* This function breaks up the Fido combination date/time string into their */
/* individual compenents, and converts the Fido way of displaying the date	*/
/* in a DD MMM YY (01 Jan 93) format to PCBoard's MM-DD-YY (01-01-93) way   */
/* of displaying it 														*/

void LIBENTRY  translate_date_time(char *inputstr,char *destdate,char *desttime)
{
char static Months[24][4]={"Jan","01-",
						   "Feb","02-",
						   "Mar","03-",
						   "Apr","04-",
						   "May","05-",
						   "Jun","06-",
						   "Jul","07-",
						   "Aug","08-",
						   "Sep","09-",
						   "Oct","10-",
						   "Nov","11-",
						   "Dec","12-"};
char work[5],day[5],workstr[25];
char *tptr=NULL,*ptr=NULL;
int  i=0;

  checkstack();
  memset(work,0,sizeof(work));
  memset(day,0,sizeof(day));
  memset(workstr,0,sizeof(workstr));

  assert(stackavail() > 2000);

  maxstrcpy(workstr,inputstr,sizeof(workstr));
  maxstrcpy(day,"00",sizeof(day));

  if((tptr=strtok(workstr," "))!=NULL)
	maxstrcpy(day,tptr,sizeof(day));

  if((tptr=strtok(NULL," "))!=NULL)
	maxstrcpy(work,tptr,sizeof(work));

  for(i=0;strstr(Months[i],work)==NULL&&i<24;i+=2);

  if(i<24)
	strcpy(destdate,Months[i+1]);
  else
	strcpy(destdate,"00-");

  strcat(destdate,day);
  strcat(destdate,"-");

  if((tptr= strtok(NULL," "))!=NULL)
  {
	strcat(destdate,tptr);
	ptr=tptr+strlen(tptr);
	while(*ptr==0x20 || *ptr==NULL)
	  ptr++;
  }
  if(ptr!=NULL)
	strncpy(desttime,ptr,5);
}

/****************************************************************************/
/* Search the array created by init_users() to find Fido users according to */
/* search paramaters. Returns the user record number it finds.				*/

long  LIBENTRY find_fido_rec(char * addr,bool cont,bool reset,long *new_offset)
{
static long offset=0;
char		searchfld[40],tsf[40],curuser[40];
int 		searchlen;
long		tmp;

  checkstack();
  init_users();
  memset(searchfld,' ',sizeof(searchfld));

  if(reset==TRUE)  /* Just reset the offset pointer and return. */
  {
	 offset=0;
	 return(0L);
  }

  /* if this is not a continuation then start at the beginning. */
  if(cont==FALSE)	   offset=0;

  if(new_offset!=NULL) offset=*new_offset;

  if(addr!=NULL)
  {
	 searchlen = sizeof(users[0].name);
	 char * ptr = strrchr(addr,'@');
	 if(ptr) *ptr = '\x0';

	 sprintf(tsf,"~FIDO~%s",addr);
	 memcpy(searchfld,tsf,strlen(tsf));

	 if(ptr)   *ptr = '@';

  }
  else
  {
	searchlen = 6;
	memcpy(searchfld,"~FIDO~",6);
  }

  while(users[(int)offset].name[0]!=0)
  {

	memset(curuser,' ',sizeof(curuser));
	memcpy(curuser,users[(int)offset].name,strlen(users[(int)offset].name));

	if(memcmp(curuser,searchfld,searchlen) == 0 )
	{
	   tmp=offset;
	   offset++;
	   if(new_offset!=NULL) *new_offset=offset;

	   return(users[(int)tmp].num);
	}
	offset++;
  }
  return(-1L);
}

/****************************************************************************/
/* Init the user structure. Contains all the Fido user record index 		*/
/* enteries for quick lookup of the Fido user records.						*/

void LIBENTRY  init_users(void)
{
char		path[100];
IndexType	index;
IndexType2	index2;
char		searchfld[26];
DOSFILE 	ndxfile;
int 		searchlen;
int 		num=0;

  checkstack();
  if(usersinitialized) return;
  users = new(FIDOUSER[MAX_FIDO_USERS]);
  usersinitialized = TRUE;

  memset(users,0,sizeof(FIDOUSER)*MAX_FIDO_USERS);

  maxstrcpy(path,PcbData.NdxLoc,sizeof(path)-9);
  strcat(path,"PCBNDX.Z");

  if(dosfopen(path,OPEN_READ|OPEN_DENYNONE,&ndxfile)==-1)
	{
	  writeFidolog("Could not open FIDO user index.",BLOCK);
	  return;
	}

  strcpy(searchfld,"~FIDO~");
  searchlen=strlen(searchfld);

  while(1)
	{
	  if(BigNdx)
		{

		  if(dosfread(&index2,sizeof(index2),&ndxfile)!=sizeof(index2))
			break;
		  index2.UserName[sizeof(index2.UserName)-1]=NULL;
		  stripright(index2.UserName,' ');

		  if(memcmp(index2.UserName,searchfld,searchlen)==0)
			{
			  maxstrcpy(users[num].name,index2.UserName,sizeof(users[num].name));
			  users[num].num=index2.UserRec;
			  num++;
			  if(num == MAX_FIDO_USERS) break;
			}
		}
	  else
		{
		  if(dosfread(&index,sizeof(index),&ndxfile)!=sizeof(index))
			break;
		  index.UserName[sizeof(index.UserName)-1]=NULL;
		  stripright(index.UserName,' ');

		  if(memcmp(index.UserName,searchfld,searchlen)==0)
			{
			  maxstrcpy(users[num].name,index.UserName,sizeof(users[num].name));
			  users[num].num=index.UserRec;
			  num++;
			  if(num == MAX_FIDO_USERS) break;
			}
		}
	}
  dosfclose(&ndxfile);
}

void LIBENTRY de_init_users(void)
{
	delete(users);
	users = NULL;
	usersinitialized = FALSE;
}

void LIBENTRY moveFileToDir(char * file, char * dir,char * description)
{
  char	 fname[14];
  char	 oldFile[MAXFLEN];
  char	 newFile[MAXFLEN];
  char	 logBuf[65];

	// Isolate file name from path
	maxstrcpy(fname,findstartofname(file),sizeof(fname));

	// Initialize old and new filenames
	maxstrcpy(oldFile,file,sizeof(oldFile));
	sprintf(newFile,"%-1.*s%-1.13s",sizeof(newFile)-13,dir,fname);

	// Move the file and log it
	pcbmovefile(oldFile,newFile);

	sprintf(logBuf,"%-1.13s  moved to %-1.*s directory.",fname,sizeof(logBuf)-47,description);
	writeFidolog(logBuf,BLOCK);

}


void LIBENTRY freeAndNull(char ** block)
{
	if(*block != NULL)
	{
		free(*block);
		*block = NULL;
	}
}

bool LIBENTRY checkPacketPassword(char * fidoUserAddr, char * pktPassword)
{
  long recno = 0;


	if((recno=find_fido_rec(fidoUserAddr,FALSE,FALSE,NULL))!=-1)
	{
	  get_fido_rec(recno);
	  if(UsersData.SysopComment[0]!=NULL && UsersData.SysopComment[0]!=' ')
	  {
		UsersData.SysopComment[sizeof(UsersData.SysopComment)-1]=NULL;
		stripright(UsersData.SysopComment,' ');
		if(strnicmp(UsersData.SysopComment,pktPassword,strlen(UsersData.SysopComment))!=0)
		  return FALSE;
		else
		  return TRUE;
	  }
	}
	return TRUE;
}


void LIBENTRY updateFidoUserRecord(void)
{

	convertdatatoread(&UsersData,&UsersRead);
	putuserrecord(IGNOREDIRTY);
}

int LIBENTRY determineFidoPktType(FIDO_PACKET_HEADER_TYPE2 & type2Hdr)
{
 int type = 2;

  if(type2Hdr.CW!=SWAPBYTE(type2Hdr.CWValid) || (type2Hdr.CW==0 && type2Hdr.CWValid==0))
	 type = 1;
  else
	 type = 2;
  return type;
}

bool LIBENTRY readType1PktHdr(FIDO_PACKET_HEADER & type1Hdr,DOSFILE & pkt)
{

	dosrewind(&pkt);
	if(dosfread(&type1Hdr,sizeof(type1Hdr),&pkt)==-1)
	{
	  dosfclose(&pkt);
	  return(FALSE);
	}
	return TRUE;
}


bool LIBENTRY readFidoMessageHeader(FIDO_MESSAGE_HDR & msghdr,DOSFILE & pkt_file)
{

   if(dosfread(&msghdr,FIDO_FIXED_SIZE,&pkt_file)!=FIDO_FIXED_SIZE)
		return FALSE;

	  if(msghdr.Packet_Type!=2)
		return FALSE;;

	  // If To field is in datetime field then adjust accordingly (This supports bugs in other mailers)
	  if(msghdr.DateTime[18] == '\x0')
	  {
		msghdr.To_User[0] = msghdr.DateTime[19];
		read_str(&msghdr.To_User[1],sizeof(msghdr.To_User)-1,pkt_file);
	  }
	  else
		read_str(msghdr.To_User,sizeof(msghdr.To_User),pkt_file);

	  read_str(msghdr.From_User,sizeof(msghdr.From_User),pkt_file);
	  read_str(msghdr.Subject,sizeof(msghdr.Subject),pkt_file);

	  return TRUE;
}


/*****************************************************************************/
/* Read a String from the file. Stops on a NULL or when Limit is reached.		 */

void LIBENTRY read_str(char *buffer,int limit,DOSFILE &file)
{
int 	i=0,j=0,len=0,dif=0,length=0;
char	buf[512];

  checkstack();
  j=0;
  memset(buf,0,sizeof(buf));
  assert(stackavail() > 2000);

  while(1)
	{
	  length=dosfread(buf,sizeof(buf),&file);

	  for(i=0;i<sizeof(buf);i++,j++)
		{
		  assert(i >= 0 && i < sizeof(buf));
		  assert(j >= 0 && j < sizeof(buf));
		  if(j==limit)
			{
			  buffer[j]=NULL;
			  return;
			}

		  if(buf[i]==NULL)
			{
			  buffer[j]=buf[i];
			  len=strlen(buf);
			  dif=length-len;
			  dif--;
			  dosfseek(&file,-dif,SEEK_CUR);
			  return;
			}
		  else
			buffer[j]=buf[i];
		}
	}
}

bool LIBENTRY myAKA(unsigned zone, unsigned net, unsigned node, unsigned point)
{

   for(int i=0;i<num_akas;i++)
   {
	  if(address[i].zone==zone &&
		 address[i].net==net   &&
		 address[i].node==node &&
		 address[i].point==point) return TRUE;
   }
   return FALSE;
}

char * LIBENTRY getPath(char * pathname)
{
	char * ptr = NULL;

	ptr = findstartofname(pathname);

	if(ptr) *ptr = '\x0';

	return pathname;
}

char * genMSGID(char * addr, unsigned conf)
{
static	 char msgid[70];
unsigned long ser=0;
char	 addrbuf[25];

  maxstrcpy(addrbuf,addr,sizeof(addrbuf));
  ser = getjuliandate() + exacttime() + conf + message.msgHdr.number+clock();
  stripright(addrbuf,' ');
  sprintf(msgid,"\x01""MSGID: %s %08lx\r",addrbuf,ser);
  return msgid;
}

int LIBENTRY matchAKA(unsigned int zone, unsigned int net, NADDRESS * addr, int numaddr)
{
int matched_aka = 0;
  for(int i=0;i<numaddr;i++)
  {
	if(addr[i].zone == zone && matched_aka == 0)
	{
	   matched_aka = i;
	   if(addr[i].primary == TRUE) break;
	}
	if(addr[i].zone == zone && addr[i].net == net)
	{
	   matched_aka = i;
	   if(addr[i].primary == TRUE) break;
	}
  }
  return matched_aka;
}


void LIBENTRY genResponseMsg(FUSERS & user,int type, const char * text)
{

FIDO_PACKET_HEADER_TYPE2  pkthdr;
FIDO_MESSAGE_HDR		  msghdr;
systimetype 			  timerec;
sysdatetype 			  daterec;
char					  pktfilename[MAXFLEN],resfilename[MAXFLEN],msgbuf[100];
char					  zero,*cr="\rReason: ",logBuf[80];
char					  addr1[25],addr2[25];
char					  toINTL[25],fromINTL[25],TOPT[10],FMPT[10];
char					  afixresfile[15];
char					  crt='\x0D';
int 					  bytesread=0;
unsigned int			  matched_aka=0;
cDOSFILE				  pktfile,resfile;
//cNEWQ 					Q;
QUEUE_RECORD			  qrec;
bool					  afixAttach = FALSE;
bool					  nofile = FALSE;
FUSERS					  hub;
static	char			  secfix=0;
NODE_REC				  nrec;

  if(!PcbData.FidoMakeResponse) return;
  memset(&pkthdr,' ',sizeof(pkthdr));
  memset(&msghdr,' ',sizeof(msghdr));
  memset(&qrec,  0,sizeof(qrec));
  memset(&hub,0,sizeof(hub));
  memset(TOPT,0,sizeof(TOPT));
  memset(FMPT,0,sizeof(FMPT));
  zero = '\x0';

  maxstrcpy(resfilename,directory_info.messages,sizeof(resfilename));
  maxstrcpy(afixresfile,"afreXXXXXX",sizeof(afixresfile));

  // Generate a unique file name for the response attach
  char	   attchfilename[200];
  maxstrcpy(attchfilename,directory_info.outgoing_packets,sizeof(attchfilename));
  strcat(attchfilename,afixresfile);
    extern char *mktemp(char *);
  mktemp(attchfilename);
  char * t = strrchr(attchfilename,'\\');
  if(t != NULL) maxstrcpy(afixresfile,t,sizeof(afixresfile)); // Now copy just the unique file name back into it's buffer complete with mangled name

  matched_aka = matchAKA(user.zone,user.net,address,num_akas);
  if(user.point == 0) sprintf(addr1,"%u:%u/%u",user.zone,user.net,user.node);
  else				  sprintf(addr1,"%u:%u/%u.%u",user.zone,user.net,user.node,user.point);

  // Generate packet file name
  getsysdate(&daterec);
  getsystime(&timerec);
  timerec.Seconds +=secfix++;
  sprintf(pktfilename,"%s%02d%02d%02d%02X.PKT",directory_info.outgoing_packets,daterec.Day,timerec.Hours,timerec.Minutes,timerec.Seconds);

  sprintf(toINTL,"\x01""INTL %u:%u/%u",user.zone,user.net,user.node);
  if(user.point != 0) sprintf(TOPT,"\x01""TOPT %u",user.point);

  sprintf(fromINTL," %u:%u/%u",address[matched_aka].zone,address[matched_aka].net,address[matched_aka].node);
  if(address[matched_aka].point != 0) sprintf(FMPT,"\0x01""FMPT %u",address[matched_aka].point);


  // generate fido packet header
  pkthdr.Dest_Node=user.node;
  pkthdr.Year=daterec.Year;
  pkthdr.Month=daterec.Month;
  pkthdr.Day=daterec.Day;
  pkthdr.Hour=timerec.Hours;
  pkthdr.Minute=timerec.Minutes;
  pkthdr.Second=timerec.Seconds;
  pkthdr.Baud=0;
  pkthdr.Packet_Type=0x0002;
  pkthdr.Orig_Node=address[matched_aka].node;
  pkthdr.Dest_Node=user.node;
  pkthdr.Dest_Net=user.net;
  pkthdr.DestZone=user.zone;
  pkthdr.Dest_Zone=user.zone;
  pkthdr.DestPoint=user.point;
  pkthdr.Orig_Net=address[matched_aka].net;
  pkthdr.OrigPoint=address[matched_aka].point;
  pkthdr.Orig_Zone=address[matched_aka].zone;
  pkthdr.ProtCode=PROTCODE;
  pkthdr.SerialNo=1;
  pkthdr.OrigZone=address[matched_aka].zone;
  pkthdr.Orig_Zone=address[matched_aka].zone;
  pkthdr.OrigPoint=address[matched_aka].point;
  pkthdr.ProdData=0;
  pkthdr.CW=0x0001;
  pkthdr.CWValid=0x0100;

   // generate fido msg header
	msghdr.Packet_Type=2;
	msghdr.Orig_Node = address[matched_aka].node;
	msghdr.Dest_Node = user.node;
	msghdr.Orig_Net  = address[matched_aka].net;
	msghdr.Dest_Net  = user.net;
	msghdr.Attribute |= 0 | MSG_PRIVATE;
	msghdr.Msg_Cost  = 0;

	char time[10];

	timestr2(time);
	translate_date(msghdr.DateTime,juliantodate(getjuliandate()),time,sizeof(msghdr.DateTime));

	if(user.point == 0 && get_node(user.zone,user.net,user.node,&nrec) == TRUE)
	   memcpy(msghdr.To_User,nrec.Sysop_name,strlen(nrec.Sysop_name));
	else
	   memcpy(msghdr.To_User,"SYSOP",5);

	msghdr.To_User[sizeof(msghdr.To_User)-1]='\x0';
	memcpy(msghdr.From_User,"PCBoard Fido",12);
	msghdr.From_User[sizeof(msghdr.From_User)-1]='\x0';

	switch(type)
	{
	  case FREQFAIL:

		memcpy(msghdr.Subject,"FREQ Failure",12);
		strcat(resfilename,"FRQPFAIL.MSG");
		sprintf(logBuf,"File request failure notice sent to %s.",addr1);

		break;

	  case FREQINFO:

		memcpy(msghdr.Subject,"FREQ Information",16);
		strcat(resfilename,"FREQINFO.MSG");
		sprintf(logBuf,"File request information sent to %s.",addr1);
		break;

	  case AFIXFAIL:

		memcpy(msghdr.Subject,"Areafix Failure",15);
		strcat(resfilename,"AFIXFAIL.MSG");
		sprintf(logBuf,"Areafix failure notice sent to %s.",addr1);
		break;

	  case AFIXLIST:

		memcpy(msghdr.Subject,afixresfile,strlen(afixresfile));
		strcat(resfilename,"AFIXLST.HDR");
		sprintf(logBuf,"Areafix list sent to %s.",addr1);
		msghdr.Attribute=MSG_FILEATT;
		afixAttach = TRUE;
		break;

	  case AFIXHELP:

		memcpy(msghdr.Subject,"Areafix:Help.",13);
		strcat(resfilename,"AFIXHELP.MSG");
		sprintf(logBuf,"Areafix help file sent to %s.",addr1);
		break;

	  case AFIXAVAL:

		memcpy(msghdr.Subject,afixresfile,strlen(afixresfile));
		strcat(resfilename,"AFIXAVL.HDR");
		sprintf(logBuf,"Areafix available area list sent to %s.",addr1);
		msghdr.Attribute=MSG_FILEATT;
		afixAttach = TRUE;
		break;

	  case AFIXRESPONSE:

		memcpy(msghdr.Subject,afixresfile,strlen(afixresfile));
		strcat(resfilename,"AFIXRSPN.HDR");
		sprintf(logBuf,"Areafix response sent to %s.",addr1);
		msghdr.Attribute=MSG_FILEATT;
		afixAttach = TRUE;
		break;

	  case AFIXFORWARD:

		findMyHub(user,hub,msghdr.Subject);
		if(hub.zone == 0)
		{
		   writeFidolog("Areafix cannot be forwarded. No hub found to foward to.",BLOCK);
		   return;
		}
		msghdr.Dest_Net   = hub.net;
		msghdr.Dest_Node  = hub.node;
		memset(msghdr.To_User,' ',sizeof(msghdr.To_User));
		msghdr.To_User[sizeof(msghdr.To_User)-1] = '\x0';
		strcpy(msghdr.To_User,"AREAFIX");

		pkthdr.Dest_Node=hub.node;
		pkthdr.Dest_Net=hub.net;
		pkthdr.DestZone=hub.zone;
		pkthdr.Dest_Zone=hub.zone;
		pkthdr.DestPoint=hub.point;

		sprintf(toINTL,"\x01""INTL %u:%u/%u",hub.zone,hub.net,hub.node);

		if(hub.point == 0) sprintf(addr2,"%u:%u/%u",hub.zone,hub.net,hub.node);
		else
		{
		  sprintf(addr2,"%u:%u/%u.%u",hub.zone,hub.net,hub.node,hub.point);
		  sprintf(TOPT,"\x01""TOPT %u",user.point);
		}

		strcat(resfilename,"AFIXFWRD.HDR");
		sprintf(logBuf,"Areafix request from %s forwarded to %s",addr1,addr2);
		break;

	  default: return;
	}

	msghdr.Subject[sizeof(msghdr.Subject)-1]='\x0';
	writeFidolog(logBuf,BLOCK);


	if(fileexist(resfilename) == 255)
	{
	  sprintf(logBuf,"Response file %s is missing.",findstartofname(resfilename));
	  writeFidolog(logBuf,BLOCK);
	  if(strstr(resfilename,".MSG")) return;
	  nofile = TRUE;
	}


	if(fileexist(pktfilename) != 255 ||  pktfile.open(pktfilename,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE) != 0)
	{
	   writeFidolog("Could not open response packet. Aborting response.",BLOCK);
	   return;
	}

	if(!nofile)
	{
	 if(resfile.open(resfilename,OPEN_RDWR | OPEN_DENYNONE) != 0)
	 {
	   writeFidolog("Could not open response message template. Aborting.",BLOCK);
	   return;
	 }
	}


	stripright(msghdr.To_User,' ');
	stripright(msghdr.From_User,' ');
	stripright(msghdr.Subject,' ');

	// write headerss to packet
	pktfile.write(&pkthdr,sizeof(pkthdr));

	pktfile.write(&msghdr,FIDO_FIXED_SIZE);

	pktfile.puts(msghdr.To_User);
	pktfile.write(&zero,1);
	pktfile.puts(msghdr.From_User);
	pktfile.write(&zero,1);
	pktfile.puts(msghdr.Subject);
	pktfile.write(&zero,1);

	// Write INTL address's to packet
	pktfile.write(toINTL,strlen(toINTL));
	pktfile.write(fromINTL,strlen(fromINTL));
	pktfile.write(&crt,1);

	if(TOPT[0] != '\x0')
	{
	   pktfile.write(TOPT,strlen(TOPT));
	   pktfile.write(&crt,1);
	}
	if(FMPT[0] != '\x0')
	{
	  pktfile.write(FMPT,strlen(FMPT));
	  pktfile.write(&crt,1);
	}


	// write resfilename contents to packet
	while(!nofile && (bytesread = resfile.read(msgbuf,sizeof(msgbuf)) ) != 0)
	  pktfile.write(msgbuf,bytesread);

	if(!nofile) resfile.close();

	// Build the attach file name for larger response types



	switch(type)
	{
	  case AFIXFAIL:
		writeFailsToPkt(pktfile);
		break;
	  case AFIXLIST:
	  {
		// this can be a large amoutn of data, so write to a file and file attach it.
		cDOSFILE attch;
		attch.open(attchfilename,OPEN_RDWR | OPEN_DENYNONE | OPEN_CREATE);
		writeAreasToFile(attch,FALSE);
		attch.close();
		break;
	  }
	  case AFIXAVAL:
	  {
		cDOSFILE attch;
		attch.open(attchfilename,OPEN_RDWR | OPEN_DENYNONE | OPEN_CREATE);
		writeAreasToFile(attch,TRUE);
		attch.close();
		break;
	  }
	  case AFIXRESPONSE:
	  {
		if(PcbData.FidoLogLevel >= 3)
		{
		  sprintf(logBuf,"Areafix requests from %s.",addr1);
		  writeFidolog(logBuf,BLOCK);
		}
		cDOSFILE attch;
		attch.open(attchfilename,OPEN_RDWR | OPEN_DENYNONE | OPEN_CREATE);
		writeChangesToFile(attch); // This call writes all afix info to pkt file
		attch.close();
		break;
	  }
	  case AFIXFORWARD:
		if(PcbData.FidoLogLevel >= 3)
		{
		  sprintf(logBuf,"Areafix requests from %s to be forwarded.",addr1);
		  writeFidolog(logBuf,BLOCK);
		}
		writeForwardsToFile(pktfile);
		memcpy(&user,&hub,sizeof(user));
		break;
	  default: break;

	}
	if(text != NULL)
	{
	  pktfile.write(cr,strlen(cr));
	  pktfile.write(text,strlen(text));
	}
	pktfile.write(&zero,sizeof(zero));

	pktfile.close();


	// add packet to queue
	maxstrcpy(qrec.filename,pktfilename,sizeof(qrec.filename));
	if(user.point == 0) sprintf(qrec.nodestr,"%u:%u/%u",user.zone,user.net,user.node);
	else				sprintf(qrec.nodestr,"%u:%u/%u.%u",user.zone,user.net,user.node,user.point);
	qrec.flag |= Q_KILLSENT | Q_NORMAL;
	//qrec.flag |= Q_KILLSENT | Q_CRASH | Q_OUTBOUND;

	// Limit Q's scope
	{
	cNEWQ Q;
	  Q.addEntry(qrec);
	  if(afixAttach == TRUE)
	  {
		maxstrcpy(qrec.filename,attchfilename,sizeof(qrec.filename));
		qrec.flag |= Q_FILESEND;
		Q.addEntry(qrec);
	  }
	}
	return;

}

void LIBENTRY writeAreasToFile(cDOSFILE & file,bool allareas)
{
NAREA_STRUCT  area;
unsigned int  i=1;
char		  cr[1];
char	  *   areaname=NULL;
FUSERS		  user;
uint		  numrecs;

  cr[0] = '\r';
  // Process all PCBoard FIDO conferences

  // Limit areas scope
  {
  cAREAS areas;
  numrecs = areas.totRecs();

  for(i=1;i<numrecs;i++)
  {
	area = areas.getRec(i);

	// If we're not writing all areas, then check registration flags
	// else write it out anyway
	if(!allareas && isset(&ConfReg[REG],area.ConfNum) && isset(&ConfReg[USR],area.ConfNum))
	{
	   file.write(area.AreaTag,strlen(area.AreaTag));
	   file.write(cr,1);
	}
	else if(allareas && area.ConfNum != 0 && isset(&ConfReg[REG],area.ConfNum))
	{
		 file.write(area.AreaTag,strlen(area.AreaTag));
		 file.write(cr,1);
	}
  }
  }

  i=1;
  memset(&user,0,sizeof(user));

  char * tptr = strrchr(UsersData.Name,'~');
  if(tptr) fido_nodestr_to_int(++tptr,user.zone,user.net,user.node,user.point);

  // limit PT scope
  {
  cPTINFO		PT;

  numrecs = PT.numAreas();
  for(i=1;i<numrecs;i++)
  {
	if( ((areaname = PT.seekToAreaNum(i++)) != NULL) && PT.userInArea(area.AreaTag,user))
	{
	  file.write(areaname,strlen(areaname));
	  file.write(cr,1);
	}
  }
  }
}

void LIBENTRY writeChangesToFile(cDOSFILE & file)
{
cDOSFILE	 dfile;
char		 dfilename[MAXFLEN];
char		 buf[1024];
int 		 bytesread=0;

  sprintf(dfilename,"%sAFIX.ADD",directory_info.messages);

  if(fileexist(dfilename) != 255)
  {
	if(dfile.open(dfilename, OPEN_RDWR | OPEN_DENYNONE)!= 0) return;
  }
  else
	if(dfile.open(dfilename, OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE)!= 0) return;


  while( (bytesread = dfile.read(buf,sizeof(buf))) > 0)
  {
	if(PcbData.FidoLogLevel >= 3) writeFidolog(buf,BLOCK);
	file.write(buf,bytesread);
  }
  dfile.close();
}

static bool _NEAR_ LIBENTRY findMyHub(FUSERS & user,FUSERS & hub, char * afixpwrd)
{
char		  paddr[25],addr[35];
char		* nullit=NULL;
bool		  done=FALSE;
long		  offset=0;
long		  userrecno=0;
long		  userno=0;

  memset(&hub,0,sizeof(hub));
  sprintf(paddr,"~FIDO~%u:",user.zone);

  find_fido_rec(NULL,FALSE,TRUE,&offset);
  if(tempuseralloc(FALSE) == -1)
  {
	 writeFidolog("Could not allocate memory for temp user.",BLOCK);
	 return FALSE;
  }


  while(!done)
  {
	for(;userrecno < MAX_FIDO_USERS;userrecno++)
	{
	  if(users[(int)userrecno].name[0] == '\x0')
	  {
		done = TRUE;
		userno = 0;
		break;
	  }
	  if(memcmp(users[(int)userrecno].name,paddr,strlen(paddr)) == 0)
	  {
		 userno = users[(int)userrecno].num;
		 userrecno++;
		 break;
	  }
	}

	if(userno <= 0 || gettemprecord(userno) == -1) return FALSE;

	if(TempData->ExpertMode == TRUE)
	{
	  maxstrcpy(addr,TempData->Name,sizeof(addr));
	  nullit = strrchr(addr,'~');
	  if(nullit) fido_nodestr_to_int(++nullit,hub.zone,hub.net,hub.node,hub.point);
	  else	 fido_nodestr_to_int(addr,hub.zone,hub.net,hub.node,hub.point);
	  if(hub.zone == user.zone)
	  {
		done = TRUE;
		strcpy(afixpwrd,TempData->UserComment);
	  }
	}
  }
  tempuserdealloc();
  return TRUE;
}

void LIBENTRY forwardRequest(const char * area,bool add)
{
  cDOSFILE fwrd,backwrd;
  char	   fwrdname[MAXFLEN],backwrdname[MAXFLEN];
  char	   backmsg[100],fwrdmsg[100];
  char	   logBuf[65];
  char	   ch;

  if(add) ch = '+';
  else	  ch = '-';

  // Build message to send to uplink && downlink
  sprintf(backmsg,"Area not found: %s.\r",area);
  if(PcbData.FidoEnableAreaFix) strcat(backmsg,"Forwarding request\r");
  sprintf(fwrdmsg,"%c%s \r",ch,area);

  // Build file name to store the message in
  sprintf(backwrdname,"%sAFIX.ADD",directory_info.messages);
  sprintf(fwrdname,"%sFWRD.NFO",directory_info.messages);

  // Open files
  if(fwrd.open(fwrdname,OPEN_RDWR | OPEN_DENYNONE) != 0)	   return;
  if(backwrd.open(backwrdname,OPEN_RDWR | OPEN_DENYNONE) != 0) return;

  // Seek to end to append
  fwrd.seek(0,SEEK_END);
  backwrd.seek(0,SEEK_END);

  // Write messages
  backwrd.write(backmsg,strlen(backmsg));
  fwrd.write(fwrdmsg,strlen(fwrdmsg));
  fwrd.close();
  backwrd.close();

  sprintf(logBuf,"Forwarding request for area %s.",area);
  writeFidolog(logBuf,BLOCK);
}

void LIBENTRY writeForwardsToFile(cDOSFILE & file)
{
char	  fname[MAXFLEN];
char	  buf[1024];
int 	  bytesread=0;
cDOSFILE  fwrdfile;

  sprintf(fname,"%sFWRD.NFO",directory_info.messages);
  if(fileexist(fname) == 255) return;

  if(fwrdfile.open(fname, OPEN_RDWR | OPEN_DENYNONE) != 0) return;

  while( (bytesread = fwrdfile.read(buf,sizeof(buf)) ) != 0)
  {
	 if(PcbData.FidoLogLevel >= 3) writeFidolog(buf,BLOCK);
	 file.write(buf,bytesread);
  }

  fwrdfile.close();
  unlink(fname);
}

void LIBENTRY prepAfix(cDOSFILE & file)
{
char fname[MAXFLEN];

  sprintf(fname,"%sFWRD.NFO",directory_info.messages);
  if(fileexist(fname) != 255) unlink(fname);

  sprintf(fname,"%sFAILS.DAT",directory_info.messages);
  if(fileexist(fname) != 255) unlink(fname);

  sprintf(fname,"%sAFIX.ADD",directory_info.messages);
  if(fileexist(fname) != 255) unlink(fname);


  file.open(fname,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE);

}

void LIBENTRY cleanUpAfix(FUSERS & user,cDOSFILE & file)
{
char fname[MAXFLEN];

  if(file.isOpen()) file.close();
  sprintf(fname,"%sAFIX.ADD",directory_info.messages);
  if(fileexist(fname) != 255 && DTA.ff_fsize != 0)
  {
	 genResponseMsg(user,AFIXRESPONSE,NULL);
	 unlink(fname);
  }

  sprintf(fname,"%sFAILS.DAT",directory_info.messages);
  if(fileexist(fname) != 255)  unlink(fname);


  sprintf(fname,"%sFWRD.NFO",directory_info.messages);
  if(fileexist(fname) != 255)
  {
	 genResponseMsg(user,AFIXFORWARD,NULL);
	 unlink(fname);
  }
}

void LIBENTRY doRescan(const char * command)
{
char	 *	 walk = strstr((char *)command,"RESCAN");
char		 area[AREA_SIZE];
unsigned int i=0,areanum=0;
long		 rescannum=0;
cAREAS		 areas;
NAREA_STRUCT arearec;



  memset(area,0,AREA_SIZE);

  if(walk)
	while(*walk != ' ' && *walk != '\x0' && *walk != '\r') walk++;
  else return;

  while(*walk == ' ' && *walk != '\x0' && *walk != '\r') walk++;

  while(*walk != ' ' && *walk != '\x0' && *walk != '\r' && i<sizeof(area))
	area[i++] = *walk++;

  while(*walk == ' ' && *walk != '\x0' && *walk != '\r') walk++;

  rescannum = atol(walk);

  areanum = areas.findRec(area);
  if(areanum == 0) return;
  arearec = areas.getRec(areanum);
  if( (MsgReadPtr[arearec.ConfNum] + rescannum) < 0) MsgReadPtr[arearec.ConfNum] = 0;
  else MsgReadPtr[arearec.ConfNum] = MsgReadPtr[arearec.ConfNum] + rescannum;

}


int LIBENTRY makeMyAkaList(char * buf,int buflen,int whichlist)
{

char akabuf[15];
  int	   j=0;
  uint	   z=0,n=0,node,p;
  char	   *   bname=NULL;

  // get current user name that we are exporting to
  bname = strrchr(UsersData.Name,'~');

  // Get the zone and net we are exporting to
  if(bname) fido_nodestr_to_int(++bname,z,n,node,p);

  cAKAS 		akas;
  NADDRESS		rec;
  unsigned int	i,totrecs=akas.totRecs();
  switch(whichlist)
  {
	case SEENBYS:
		  // get all my akas that colesly match the address we are exporting to.
		  for(i=1;i<=totrecs;i++)
		  {
			rec = akas.getRec(i);
			if(rec.inseenby && rec.zone == z)
			{
			   sprintf(akabuf," %u/%u",rec.net,rec.node);
			   strcat(buf,akabuf);
			   j++;
			}
			if(strlen(buf) > buflen - sizeof(akabuf)) return j;
		  }
		  if(j==0)
		  {
			 memset(akabuf,0,sizeof(akabuf));
			 rec = akas.getRec(1);

			 if(rec.zone == 0 && rec.net == 0)
			   akabuf[0] = ' ';
			 else
			   sprintf(akabuf," %u/%u ",rec.net,rec.node);

			 strcat(buf,akabuf);
			 return 1;
		  }
		  strcat(buf," ");
		  return j;

	case PRESENT:
		  for(i=1;i<=totrecs;i++)
		  {
			rec = akas.getRec(i);
			if(rec.present)
			{
			   if(rec.point == 0) sprintf(akabuf,"%u:%u/%u ",rec.zone,rec.net,rec.node);
			   else 			  sprintf(akabuf,"%u:%u/%u.%u ",rec.zone,rec.net,rec.node,rec.point);

			   strcat(buf,akabuf);
			   j++;
			}

			if(strlen(buf) > buflen - sizeof(akabuf))
			{
			   stripright(buf,' ');
			   return j;
			}
		  }
		  stripright(buf,' ');
		  return j;
  }//switch
  return 0;
}

void  LIBENTRY findMyPathAddr(char * to, NADDRESS & myrec)
{
cAKAS		 akas;
NADDRESS	 rec;
unsigned int totrecs=akas.totRecs();
unsigned int i;
unsigned int hiszone=0;
bool		 done = FALSE;

  // Find out what zone we are looking for
  hiszone = atoi(to);

  for(i=1;i<=totrecs;i++)
  {
	rec = akas.getRec(i);

	// If this zone matches, copy in as a default
	if(rec.zone == hiszone)
	{
	  done = TRUE;
	  memcpy(&myrec,&rec,sizeof(myrec));

	  // If this is a primary address, then we're done
	  if(myrec.primary) break;
	}
  }

  // If none were found that match the zone, use the first address entry as a
  // default
  if(!done)  myrec = akas.getRec(1);
}

char * LIBENTRY eatSpaces(char * ptr)
{
  while(*ptr == ' ' && *ptr != '\x0') ++ptr;
  return ptr;
}

char * LIBENTRY eatNonSpaces(char * ptr)
{
  while(*ptr != ' ' && *ptr != '\x0') ++ptr;
  return ptr;
}

int LIBENTRY fgetbyte(unsigned int waittime, bool & kbd)
{
int thebyte = -1;

  settimer(10,waittime);
  do
  {
	#ifdef __OS2__

	  releasekbdblock(waittime);
	  thebyte = waitforkey();

	#else

	  giveup();
	  thebyte = cinkey();

	#endif

	}while(thebyte <= 0  && !timerexpired(10) );

  kbd = Status.Kbd;
  return thebyte;


}

void LIBENTRY clearLine(int x, int y)
{
//	gotoxy(x,y);
//	clreol();
   clsbox(x,y,79,y,0x00);
}

void LIBENTRY getPointAddr(const char * msg,uint & frmpt,uint & topnt)
{

  char * ptr = NULL;

  if( (ptr = strstr((char*)msg,"\x01""FMPT ")) != NULL)
  {
	ptr  += 6;
	frmpt = atoi(ptr);
  }

  if( (ptr = strstr((char*)msg,"\x01""TOPT ")) != NULL)
  {
	ptr  += 6;
	topnt = atoi(ptr);
  }
}

bool LIBENTRY getPrimAkaForNet(FUSERS & akauser, FUSERS & destuser)
{
NADDRESS rec,trec;
bool	 partialMatch = FALSE;
cAKAS	 akas;
uint	 i,numakas = akas.totRecs();

  memset(&akauser,0,sizeof(akauser));
  memset(&rec,0,sizeof(rec));
  memset(&trec,0,sizeof(trec));

  for(i=1;i<=numakas;i++)
  {
	rec = akas.getRec(i);
	if(rec.zone == destuser.zone)
	{
	  if(rec.net == destuser.net && rec.primary)
	  {
		memcpy(&akauser,&rec,sizeof(akauser));
		return TRUE;
	  }
	  else if(rec.primary)
	  {
		memcpy(&trec,&rec,sizeof(trec));
		partialMatch = TRUE;
	  }
	  else if(!partialMatch)
	  {
		memcpy(&trec,&rec,sizeof(trec));
		partialMatch = TRUE;
	  }
	}
  }

  if(partialMatch)
  {
	memcpy(&akauser,&trec,sizeof(akauser));
	return TRUE;
  }
  return FALSE;

}

void LIBENTRY writeFailsToFile(const char * msg)
{
char		failfile[MAXFLEN];
cDOSFILE	file;
char		cr = '\r';

  sprintf(failfile,"%sFAILS.DAT",directory_info.messages);

  if(fileexist(failfile) == 255 	 && file.open(failfile,OPEN_WRIT|OPEN_CREATE|OPEN_DENYNONE) == -1) return;
  else if(fileexist(failfile) != 255 && file.open(failfile,OPEN_WRIT|OPEN_APPEND | OPEN_DENYNONE) == -1) return;

  file.puts(msg);
  file.write(&cr,1);
  file.close();
}

void LIBENTRY writeFailsToPkt(cDOSFILE & pkt)
{
cDOSFILE f;
char	 fname[MAXFLEN];
char	 line[80];
char	 cr = '\r';

  sprintf(fname,"%sFAILS.DAT",directory_info.messages);
  if(fileexist(fname) == 255) return;

  if(f.open(fname,OPEN_READ|OPEN_DENYNONE) == -1) return;

  while(f.getln(line,sizeof(line)) != -1)
  {
	pkt.write(line,strlen(line));
	pkt.write(&cr,1);
  }
  f.close();
  f.unlink();
}


void LIBENTRY prepBodyForPCB(char * body)
{

char *p,*ptr,*end,*tptr;
char oldchar;
int  i=0,len=0;
long tmplen=0;
//bool sep = FALSE;


  stripright(body,' ');

  compfidomsgbuf(body);

  len=strlen(body);

  // Convert carraige returns (hard and soft) to PCBoard line seperators
  for(i=0;i<=len;i++)
  {
	// Replace CR and soft CR's with lineseparator
   // if(body[i]==13 || body[i]==0x8D)
	if(body[i]== LineSeparator)
	{
	  //body[i]=LineSeparator;
	  //sep = TRUE;

	  // Insert CTRL-A lines before all seenby lines

	  // if it's CRLFSEEN-BY: simply replace LF with 01
	  if(body[i+1]==10 && memcmp(body+i+2,"SEEN-BY:",8) == 0) body[i+1] = 1;
	  else if(memcmp(body+i+1,"SEEN-BY:",8) == 0)
	  {
		p = body+i+1;
		memmove(p+1,p,(len-(int)(p-body))+1);
		len++;
		*p=0x01;
	  }

	}
/*
	// strip line feed
	else if(body[i]==10)
	{
	  memcpy(body+i,body+i+1,len-i);
	  len--;
	  i--;
	}
*/
  }// for

  ptr=body;
  tptr=body;
  //end=body+strlen(body);
  end=body+len;

/*
  // If no line separators were added, then add one at the end.
  if(!sep)
  {
	end++;
	*(end)=LineSeparator;
	*(end+1)=NULL;
  }
*/
  while(1)
  {
	if((p=strchr(tptr,LineSeparator))==NULL)
	{
	  if((p=strchr(tptr,NULL))==NULL) break;
	}

	tptr=p;
	tmplen=tptr-ptr;
	if(tmplen>79)
	{
	  oldchar=*tptr;
	  *tptr=NULL;
	  if(strchr(ptr,0x01)!=NULL || strchr(ptr,0x1B)!=NULL || strstr(ptr,"@X")!=NULL)
	  {
		 *tptr=oldchar;
		 tptr++;
		 ptr=tptr;
		 continue;
	  }
	  else
		 *tptr=oldchar;

	  for(p=ptr+79;p>=ptr;p--)
	  {
		  if(*p==0x20)
		  {
			 *p=LineSeparator;
			 p++;
			 ptr=p;
			 tptr=p;
			 break;
		  }
	  }
	  if(p<ptr)
	  {
		 p=ptr+79;
		 memmove(p+1,p,strlen(p)+1);
		 *p=LineSeparator;
		 p++;
		 ptr=p;
		 tptr=p;
	  }
	}
	else if(tptr>=end)	break;
	else
	{
	  tptr++;
	  ptr=tptr;
	}
  }//While

}

static void _NEAR_ LIBENTRY compfidomsgbuf ( char * buf )
{
	char * p1 = buf;
	char * p2 = buf;

	while (*p2)
	{
		switch (*p2)
		{
			case 0x0D: // Hard CR
			case 0x8D: // Soft CR
				*(p1++) = LineSeparator;
				break;

			case 0x0A: // LF
				break;

			default:   // Any other character
				*(p1++) = *p2;
				break;
		}
		++p2;
	}
	if(*p1 != LineSeparator) *(p1++) = LineSeparator;
	*(p1++) = 0;
}


void LIBENTRY addrToNum(const char * addr, uint & zone,
						uint & net, uint & node,uint & point)
{
char	 tBuf[25];
char * bPtr = NULL;

	// Initializations
	zone = net = node = point = 0;
	maxstrcpy(tBuf,(char *)addr,sizeof(tBuf));

	// grab ZONE
	bPtr = strrchr(tBuf,'~');
	if(bPtr == NULL) return;
	zone = atoi(++bPtr);

	// grab NET
	bPtr = strchr(tBuf,':');
	if(bPtr == NULL) return;
	net = atoi(++bPtr);

	// Grab ZONE
	bPtr = strchr(tBuf,'/');
	if(bPtr == NULL) return;
	node = atoi(++bPtr);

	// grab POINT
	bPtr = strchr(tBuf,'.');
	if(bPtr == NULL) return;
	point = atoi(++bPtr);
}


#endif
