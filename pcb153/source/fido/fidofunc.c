#ifdef FIDO
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
//#include <limits.h>
#include <assert.h>
#include <malloc.h>
#include <time.h>
#include <dos.h>

#include "misc.h"
#include "bug.h"
#include "pcb.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "messages.h"
#include "screen.h"
#include "users.h"
#include "ansi.h"
#include "xmodem.h"

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include "tossmisc.h"
#ifdef TOSSCLASS
#include "pcbtoss.hpp"
#endif
#include "times.h"
//#include "byte.h"

#include "fidoque.hpp"
#include "fidomsg.hpp"
#include <umwf.hpp>

#ifdef DEBUG
#include <memcheck.h>
#endif

extern		 NADDRESS					*address;
extern		 NFREQ_PATH 				*reqlist;
extern		 NFREQ_MAGIC				*magic_list;
extern		 THIS_ADDRESS			*deny_list;
extern _FAR_ FREQ_INFO					freq_info;
extern _FAR_ DIRECTORIES				directory_info;
extern _FAR_ EMSI_DATA					emsi_data;
extern		 unsigned int			num_deny;
extern		 unsigned int			num_akas;
extern		 unsigned int			num_reqs;
extern		 unsigned int			num_magic;

static char  _FAR_		out_poll[30];
static char  _FAR_		filename[100];
static bool 					remotePCBoard = FALSE;
static char  _FAR_		logBuf[80];
static REMOTE_AKA	  * rakas  = NULL;
static uint  _NEAR_   numakas = 0;



CAPABILITIES capFlags;			// This is a global to implement capabilities flags.

/****************************************************************************
 Prototypes
 ****************************************************************************/


static void _NEAR_ LIBENTRY add_freqs(char *freq_list);
static void _NEAR_ LIBENTRY scanHiAscii(char *str, int strip, int *buflen);
static void _NEAR_ LIBENTRY read_freq_record(char *nodestr,FREQ_LIMIT &dlimit);
static void _NEAR_ LIBENTRY write_freq_record(FREQ_LIMIT &dlimit);
static void _NEAR_ LIBENTRY add_polls(char * poll_list);
static void _NEAR_ LIBENTRY cleanup(void);
static void _NEAR_ LIBENTRY handle_freq(int *sendcount,char *nodestr);
static void _NEAR_ LIBENTRY receive_xmodem_batch(void);
static void _NEAR_ LIBENTRY send_xmodem_batch(void);
static void _NEAR_ LIBENTRY fill_emsi(char *packet, char *remote_nodestr, int len, Emsi &emsi);
static void _NEAR_ LIBENTRY moveFREQs(void);
static bool _NEAR_ LIBENTRY send_telink(char *file);
static bool _NEAR_ LIBENTRY receive_telink(void);
static bool _NEAR_ LIBENTRY get_telink_block(TELINK_BLOCK &telink_block);
static bool _NEAR_ LIBENTRY parse_log_line(char *logline, int direction);
static bool _NEAR_ LIBENTRY Receive_Messages(void);
static bool _NEAR_ LIBENTRY Send_Messages(int sendcount);
static bool _NEAR_ LIBENTRY make_batch_list(char *nodestr,int *sendcount,int status);
static void _NEAR_ LIBENTRY add_sends(const char * line);
static void _NEAR_ LIBENTRY updateLastDateOn(char * addr);
static void _NEAR_ LIBENTRY processDLO(const char * addr);
static bool _NEAR_ LIBENTRY isRightAddress(QUEUE_RECORD & rec);


/****************************************************************************/
// scanHiAscii will either strip and encode high ascii
// or it will decode and expand high asdcii depending on the value
// of the strip parameter.
// if strip is TRUE it will strip, of FALSE it will expand
/****************************************************************************/

static void _NEAR_ LIBENTRY scanHiAscii(char *str, int strip, int *buflen)
{
char * ptr1;
char * buf;
char * tempbuf;
char	 tmpBuf[10];
int 	 len;

  buf =  (char *) bmalloc(strlen(str)*3);
	if(!buf)
	{
		sprintf(logBuf,"Could not allocate high ascii buffer. Size = %d",strlen(str)*3);
		writeFidolog(logBuf,BLOCK);
		return;
	}
	buf[0] = '\x0';
	tmpBuf[0] = '\x0';
	ptr1 = str;

  switch(strip)
	{

		// Stip and encode high ascii
		case TRUE:
		// If the character is not high ascii simply copy into temporary buffer
		if(*ptr1 <= '~' && *ptr1 != '\\')
		{
			sprintf(tmpBuf,"%c",*ptr1);
			strcpy(buf,tmpBuf);
		}

		// Encode the high ascii and append to buffer
		else
			{
			sprintf(tmpBuf,"\\%x",*ptr1);
			strcat(buf,tmpBuf);
			}
		ptr1++;

		// Now that the first character has been taken care of, loop though the rest
		// of the string doing the same thing.
		while(*ptr1 != '\x0')
		{
			// If it's high, encode and append
			if(!isascii(*ptr1) || *ptr1 == '\\')
				{
				sprintf(tmpBuf,"\\%02x",(unsigned char)*ptr1);
				strcat(buf,tmpBuf);
				}
			// else just append.
			else
			{
				sprintf(tmpBuf,"%c",*ptr1);
				strcat(buf,tmpBuf);
			}

			// Increment pointer.
			++ptr1;

		}

		break;
		// expand encoded high ascii
		case FALSE:
			 while(*ptr1 != '\x0')
			 {
			 if(*ptr1 == '\\')
			 {
				++ptr1;
				if(*ptr1 == '\\')
				{
					tmpBuf[0] = '\\';
					tmpBuf[1] = '\x0';
					strcat(buf,tmpBuf);
					continue;
				}

				// Save char after hex value
				tmpBuf[0] = ptr1[2];

				// NUL terminate hex value and temp string
				tmpBuf[2] = ptr1[2] = '\0';

				// Convert hex value to char
				tmpBuf[1] = (char) strtol(ptr1,NULL,16);

				// Restore original char
				ptr1[2] = tmpBuf[0];

				// Concatinate it
				strcat(buf,tmpBuf+1);
				++ptr1;
			 }
			 else
			 {
				sprintf(tmpBuf,"%c",*ptr1);
				strcat(buf,tmpBuf);
			 }
			 ++ptr1;
			 }
			 break;
		default:
			 break;
	}

	len=strlen(buf);

	if(len > *buflen)
	{
		*buflen=len;
	if((tempbuf=(char *)brealloc(str,(*buflen+1)))==NULL)
		{
			if(str!=NULL)
			{
		bfree(str);
		bfree(buf);
		str = buf = NULL;
				return;
			}
		}
		else
		str=tempbuf;
	}

	maxstrcpy(str,buf,*buflen);
  bfree(buf);
}

/****************************************************************************/
/* parse event file...														*/

long LIBENTRY get_event_info(char *zonenetnodestr)
{
	DOSFILE sts;
	const  char COMMA		= ',';
			char * fname	 = "PCBFIDO.STS";
			long allowBits = 0;
			char inputBuf[80 + 1];
			char * tmp2 = NULL;
			char tBuf[20];


	assert(stackavail() > 500);
	// Open file PCBFIDO.STS
	if(fileexist(fname) == 255) return 0;
	if (dosfopen(fname,OPEN_READ|OPEN_DENYWRIT,&sts) == -1) return 0;
	// parse file until end
	while(dosfgets(inputBuf,sizeof(inputBuf),&sts) != -1)
	{
		 strupr(inputBuf);
		 // If this node is not in this input string then move on to the next one.
		 if(*zonenetnodestr == ' ' && strstr(inputBuf,"POLL") != NULL) add_polls(inputBuf);
		 if(*zonenetnodestr == ' ' && strstr(inputBuf,"FREQ") != NULL) add_freqs(inputBuf);
		 if(*zonenetnodestr == ' ' && strstr(inputBuf,"SEND") != NULL) add_sends(inputBuf);
		 if(zonenetnodestr[0] == ' ') continue;
		 tmp2 = strchr(inputBuf,COMMA);

		 // Check for EXCLUSION verbs
		 if(isExcluded(zonenetnodestr,inputBuf)) continue;

		 char * tmp3 = NULL;
	 while(tmp2 && *tmp2 != '\x0')
		 {
	  while(*tmp2 == ' ' || *tmp2 == COMMA && *tmp2 != '\x0') tmp2++;
			tmp3 = tmp2;
			while(*tmp3 != ' ' && *tmp3 != '\x0') tmp3++;
			maxstrcpy (tBuf,tmp2,size_t(tmp3 - tmp2 + 1));
			assert(tmp3-tmp2+1 <= sizeof(tBuf));
			tmp2 = tmp3;
	  while(*tmp2 == ' ' && *tmp2 != '\x0') ++tmp2;

	  if(match_address(tBuf,zonenetnodestr)==FALSE )	continue;

			// Now set the corresponding bit in the return value

			if(strstr(inputBuf,"POLL")) allowBits |=  POLL;
			if(strstr(inputBuf,"FREQ")) allowBits |= Q_FILEREQ;
			if(strstr(inputBuf,"HOLD"))  allowBits |=  Q_HOLD ,allowBits &= ~Q_CRASH;
			if(strstr(inputBuf,"CRASH")) allowBits |=  Q_CRASH,allowBits &= ~Q_HOLD;
			if(strstr(inputBuf,"ALLOW-ROUTE-TO")) allowBits |= ALLOW_ROUTE;
		 }
	}
	dosfclose(&sts);
	return allowBits;
}

/****************************************************************************/
/* Add polls																*/

static void _NEAR_ LIBENTRY add_polls(char * poll_list)
{
char		*ptr,*p;
char		tmpBuf[81];
cNEWQ	Q;

	maxstrcpy(tmpBuf,poll_list,sizeof(tmpBuf));
	p=ptr=tmpBuf;

	if((p=strchr(ptr,','))!=NULL)
	{
		p++;
		if((ptr=strtok(p," "))!=NULL)
				Q.addPoll(ptr,FALSE);
		else if(*(++p)!=NULL)
		{
			Q.addPoll(p,FALSE);
			return;
		}

		while((ptr=strtok(NULL," "))!=NULL)
				Q.addPoll(ptr,FALSE);
	}
}

/****************************************************************************/
/* Add polls																*/

static void _NEAR_ LIBENTRY add_freqs(char *freq_list)
{
char		*ptr,*p;
char		addr[35];
char		tmpBuf[81];
cNEWQ	Q;


	maxstrcpy(tmpBuf,freq_list,sizeof(tmpBuf));
	p=ptr=tmpBuf;

	if((p=strchr(ptr,','))!=NULL)
	{
		p++;
		if((ptr=strtok(p," "))!=NULL)
		{
			maxstrcpy(addr,ptr,sizeof(addr));
			if((ptr=strtok(NULL," "))!=NULL)
				Q.addFREQ(addr,ptr);
		}
	}
}

static void _NEAR_ LIBENTRY add_sends(const char * line)
{
QUEUE_RECORD qrec;
cSEND	filesend;
FSEND	frec;
ffblk	fblk;
char *	walk=NULL,*end;
char		fpath[MAXFLEN];
char		addr[25];
int 		done = 0;
long		qflags=0;
int 	dosfindmax;
#ifdef __OS2__
int 	DirHandle;
#endif

	walk = strstr((char *) line,"SEND");
	if(!walk) return;

	// Walk past "SEND,"
	while(*walk != ' ' && *walk != ',' && *walk != '\x0') walk++;

	// Walk past spaces
	while(*walk == ' ' || *walk == ',' && *walk != '\x0') walk++;
	end = walk;

	// Add a SEND record to keep track of what SEND verbs have been executed
	maxstrcpy(frec.send,walk,sizeof(frec.send));
	if(filesend.findRec(frec.send) !=0) return;
	frec.hit = FALSE;
	filesend.addRec(frec);

	// Now walk past filename
	while(*end != ' ' && *end != '\x0') end++;
	if(*end != '\x0')
	{
		*end = '\x0';
		maxstrcpy(fpath,walk,sizeof(fpath));
		walk = end+1;
	}
	else
		return;

	// Now walk past spaces
	while(*walk == ' ' && *walk != '\x0') walk++;
	end = walk;

	// Walk past address
	while(*end != ' ' && *end != '\x0') end++;
	if(*end != '\x0') *end = '\x0';

	// Copy address
	maxstrcpy(addr,walk,sizeof(addr));

  dosfindmax = 1;
  #ifdef __OS2__
	DirHandle = SYSTEMDIRHANDLE;
  #endif
  done = dosfindfirst(fpath,&fblk,0,&dosfindmax PDIRHANDLE);
	char * nullit = findstartofname(fpath);
	if(nullit) *nullit = '\x0';
	else			 return;

  //qflags = get_event_info(addr);
	if(qflags == 0) qflags = Q_NORMAL;

	while(!done)
	{
		memset(&qrec,0,sizeof(qrec));
		sprintf(qrec.filename,"%s%s",fpath,fblk.ff_name);
		if(fileexist(qrec.filename) != 255)
		{
		cNEWQ Q;
			 maxstrcpy(qrec.nodestr,addr,sizeof(qrec.nodestr));
			 qrec.flag = (int)qflags;
			 Q.addEntry(qrec);
			 sprintf(logBuf,"Sending %s to %s",fblk.ff_name,qrec.nodestr);
			 writeFidolog(logBuf,BLOCK);
		}
	done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
	}
}


/****************************************************************************/
/* Convert the address string to intgers.								 */

void LIBENTRY fido_nodestr_to_int(char *addr, uint & zone, uint & net, uint & node, uint & point)
{
char   tBuf[25];
char * bPtr = NULL;

  // Initializations
  zone = net = node = point = 0;
  if(addr == NULL) return;
  maxstrcpy(tBuf,(char *)addr,sizeof(tBuf));

  // grab ZONE
  bPtr = tBuf;
  zone = atoi(bPtr);

  // grab NET
  bPtr = strchr(tBuf,':');
  if(bPtr == NULL) return;
  ++ bPtr;
  net = atoi(bPtr);

  // Grab ZONE
  bPtr = strchr(tBuf,'/');
  if(bPtr == NULL) return;
  ++bPtr;
  node = atoi(bPtr);

  // grab POINT
  bPtr = strchr(tBuf,'.');
  if(bPtr == NULL) return;
  ++bPtr;
  point = atoi(bPtr);
}


/****************************************************************************/
/* Hang up on the remote system.											*/

void LIBENTRY Disconnect(void)
{
	Asy.IgnoreCDLoss = TRUE;
	turnoffdtr();
	tickdelay(ONESECOND);
	turnondtr();
}


/****************************************************************************/
/* Display the other mailer's stats.                                        */

void LIBENTRY display_fido_info(Emsi &emsi)
{
	assert(stackavail() > 2000);
	clsbox(4,10,75,15,0x11);
	box(4,10,75,15,0x1F,DOUBLE);
	fastcenter(11,emsi.addr,ScrnColors->BoxColor);
	fastprint(6,12,emsi.name,ScrnColors->BoxColor);
	fastprint(6,13,emsi.city,ScrnColors->BoxColor);
	fastprint(6,14,emsi.sysop,ScrnColors->BoxColor);
	clsbox(49,12,74,14,0x11);
	fastprint(50,12,emsi.prodname,ScrnColors->BoxColor);
	fastprint(50,13,emsi.prodver,ScrnColors->BoxColor);
	fastprint(50,14,emsi.prodserial,ScrnColors->BoxColor);
}

/****************************************************************************/
/* Send the files Xmodem/TeLink Batch. (Fido FTS-0001). 					*/

static void _NEAR_ LIBENTRY send_xmodem_batch(void)
{
DOSFILE 	outbound;
char		line[100];

	 if(NoFTS1) return;
	if(dosfopen(OUTBOUND_FILE,OPEN_READ|OPEN_DENYRDWR,&outbound)==-1)
	return;
	writeFidolog("Sending FTS-0001. (Xmodem)",BLOCK);
	while(dosfgets(line,sizeof(line)-1,&outbound)!=-1)
	{
		sprintf(logBuf,"Sending %-1.40s",line);
		writeFidolog(logBuf,BLOCK);
		if(send_telink(line)==FALSE)
		break;
	}
	sendbyte(EOT);
	dosfclose(&outbound);
	return;
}

/****************************************************************************/
/* Send a file with TeLink (modified Xmodem). Just take care of the first  */
/* Block and call DWT's Xmodem function.                                    */

static bool _NEAR_ LIBENTRY send_telink(char *file)
{
TELINK_BLOCK	telink_block;
struct ffblk	file_block;
int 		  incoming_char=0,retrycnt=0;
bool		  done,dropthrough;
int 		  dosfindmax;

#ifdef __OS2__
int 	  DirHandle;
#endif


	if(NoFTS1) return FALSE;
	done=FALSE;
	settimer(9,TWOMINUTES);

	memset(&telink_block,0,sizeof(telink_block));
	memset(telink_block.filename,0x20,sizeof(telink_block.filename));

	//telink_block.syn=SYN;
	telink_block.syn=SOH;
	telink_block.block=0;
	telink_block.compblock=0xff;
	maxstrcpy(telink_block.send_prog,"PCBoard",sizeof(telink_block.send_prog));
	telink_block.crc_mode=0x01;

	dosfindmax = 1;
	#ifdef __OS2__
	  DirHandle = SYSTEMDIRHANDLE;
	#endif

  if(dosfindfirst(file,&file_block,FA_NORMAL|FA_ARCH,&dosfindmax PDIRHANDLE)==0)
	{
		telink_block.file_length=file_block.ff_fsize;
		telink_block.date=file_block.ff_fdate;
		telink_block.time=file_block.ff_ftime;
		maxstrcpy(telink_block.filename,file_block.ff_name,sizeof(telink_block.filename));

		//for(p=(char *)&telink_block;p<(char *)(&telink_block+sizeof(telink_block));p++)
		//	telink_block.checksum+=*p;
		#ifdef __OS2__
		   telink_block.crc = xmodemcrc((char *)&telink_block.file_length,130 /*sizeof(telink_block)-3*/);

		#else
			/// Note, this needs to be worked on
			telink_block.crc = xmodem((char *)&telink_block.file_length,130 /*sizeof(telink_block)-3*/);

			telink_block.crc	= _AH;
			telink_block.crc = _AL;
		 #endif
	}
	else
	{
	  sprintf(logBuf,"Could not find %-1.30 on disk!",file);
	  writeFidolog(logBuf,BLOCK);
	  return(FALSE);
	}

  while(!done && cdstillup())
	{
		if(timerexpired(9))
		{

			writeFidolog("Transfer Timed out.",BLOCK);
			return(FALSE);
		}

		clearinbuf();
		sendstr((char *)&telink_block,sizeof(telink_block)); /* send hello packet	 */
		dropthrough=FALSE;
		while(!dropthrough && cdstillup())	/* wait until we get something interesting	*/
		{
		   if(timerexpired(9))
		   {
			 writeFidolog("Transfer Timed out.",BLOCK);
			 return(FALSE);
		   }

		   incoming_char=comminkey();

		   if(incoming_char!=-1)
		   {
				switch(incoming_char)
				{
					case ACK:
							 if(xmodemsendfile(file)==-1)
							 {
								 sprintf(logBuf,"Could not transfer %-1.20",file);
								 writeFidolog(logBuf,BLOCK);
								 return(FALSE);
							 }
							 else
							 {
								 sprintf(logBuf,"Transfered %-1.20",file);
								 writeFidolog(logBuf,BLOCK);
								 return(TRUE);
							 }

					case NAK:
					case 'C':
					case 'c':
							  dropthrough=TRUE;
							  writeFidolog("Telink block NAKed.",BLOCK);
							  break;

					default :	/* keep watching */
							 sprintf(logBuf,"Incoming Character is %c",incoming_char);
							 writeFidolog(logBuf,BLOCK);
							 break;
				}
			}
		}

		if(++retrycnt==10)
		{
			 if(xmodemsendfile(file)==-1)
			 {
				  sprintf(logBuf,"Could not transfer %-1.20",file);
				  writeFidolog(logBuf,BLOCK);
				  return(FALSE);
			 }
			 else
			 {
				  sprintf(logBuf,"Transfered %-1.20",file);
				  writeFidolog(logBuf,BLOCK);
				  return(TRUE);
			 }
		}
	 }
	 if(cdstillup() == FALSE) writeFidolog("Carrier lost.",BLOCK);


	 return(incoming_char==ACK);
}

/****************************************************************************/
/* Wrapper for Xmodem Receive.												*/

void LIBENTRY receive_inbound_xmodem(void)
{
int 	sendcount=0;

  if(NoFTS1) return;
  TransferMessages(&sendcount,NULL,INBOUND,XMODEM);
}

/****************************************************************************/
/* Receive a batch of files with Xmodem/TeLink (FTS-0001) usuing DTW's      */
/* Xmodem functions.														*/

static void _NEAR_ LIBENTRY receive_xmodem_batch(void)
{
  if(NoFTS1) return;
  if(read_fido_config(0) == FALSE) return;
  writeFidolog("Waiting to receive via XModem",BLOCK);
  while(receive_telink()==TRUE && cdstillup() );
  writeFidolog("End of XModem session",BLOCK);
	free_fido_memory();
}

/****************************************************************************/
/* Receive a file with Xmodem/TeLink (FTS-0001) ... Use DWT's Xmodem        */

static bool _NEAR_ LIBENTRY receive_telink(void)
{
int 		  retry_count;
int 		  incomming;
bool		  done;
bool		  dropthru;
char		  fname[100];
TELINK_BLOCK	telink_block;


	if(NoFTS1) return FALSE;
	retry_count=0;
	done=FALSE;

	settimer(9,SIXTY_SECONDS);

	while(done!=TRUE && cdstillup() )
	{
		sendbyte('C');

		if(retry_count==10||timerexpired(9))
		{
			writeFidolog("Xmodem transfer timeout.",BLOCK);
			return(FALSE);
		}

		dropthru=FALSE;
		settimer(10,TEN_SECONDS);
		while(dropthru!=TRUE&&!timerexpired(10) && cdstillup())
		{
			if((incomming=comminkey())!=-1)
			{
				switch(incomming)
				{
					 case  EOT:
								tickdelay(QUARTERSECOND);
								while(comminkey()!=-1 && cdstillup());
								return(FALSE);

									/*case	SYN:*/
					 case  SOH:
							   //if(get_telink_block(telink_block)==TRUE)
								get_telink_block(telink_block);
								sendbyte(ACK);
								stripright(directory_info.incoming_packets,' ');
								stripright(telink_block.filename,' ');
								sprintf(fname,"%s%s",directory_info.incoming_packets,telink_block.filename);
								if(xmodemreceivefile(fname)==0)
								   return(TRUE);
								else
								{
								   sprintf(logBuf,"Error receiving %-1.20s",fname);
								   writeFidolog(logBuf,BLOCK);
								   return(FALSE);
								}


				  case	  SYN:
				  /*
							  stripright(directory_info.incoming_packets,' ');
							  strcpy(fname,"test.txt");
							  char name[20];
							  sprintf(name,"%s%s",directory_info.incoming_packets,fname);
							  if(xmodemreceivefile(name)==0)
							  return(TRUE);
							  else
							  return(FALSE);
				  */
							  return FALSE;

		  default:
							  if(timerexpired(10))
							  dropthru=TRUE;
							  break;
				}
			}
		}
		retry_count++;
	}

	writeFidolog("Xmodem transfer timeout.",BLOCK);
	return(FALSE);
}

/****************************************************************************/
/* Receive the TeLink block from remote. check the Checksum. TRUE on good.	*/

static bool _NEAR_ LIBENTRY get_telink_block(TELINK_BLOCK &telink_block)
{
char	*p;
//char	checksum;
char	 crc[2];
int 	incoming;
int 	numChars = 0;

	memset(&telink_block,0,sizeof(TELINK_BLOCK));
	memset(crc,0,2);
	//checksum=SYN;
	telink_block.syn=SYN;

	settimer(4,TEN_SECONDS);
	p = (char *)(&telink_block)+1;

	//for(p=(char *)&telink_block+1;!timerexpired(4)||(p<=(char *)(&telink_block+sizeof(telink_block)));p++)
	while(!timerexpired(4) && numChars < sizeof(telink_block) && cdstillup())
	{
		if((incoming=comminkey())!=-1)
		{
			*p=incoming;
			//crc+=incoming;
			p++;
			numChars++;
		}
	}
	sprintf(logBuf,"%d bytes received.",numChars);
	writeFidolog(logBuf,BLOCK);

	// note to stan:	telink_block.crc is probably a checksum value and is
	// probably a single byte value even though it is right now stored in
	// an integer.	 it needs to be compared against a calculated checksum
	// value, probably to be calculated using xmodemsum().

	if(crc[1]==telink_block.crc)
		 return(TRUE);
	else
	{
		 writeFidolog("telink crc failed",BLOCK);
		 return(FALSE);
	}
}

/****************************************************************************/
/* Make the list of files to send.											*/

static bool _NEAR_ LIBENTRY make_batch_list(char *nodestr,int *sendcount,int status)
{
QUEUE_RECORD	queue_record;
REMOTE_AKA		*remote_akas = NULL;
DOSFILE 		outbound;
int 			i			 = 0;
int 			j			 = 0;
int 			num_remotes  = 0;
char			*tptr		 = NULL;
char			*ptr			 = NULL;
char			*p			 = NULL;
bool			first		 = FALSE;
bool			done			 = FALSE;
char			tmp;

	//memset(&outbound,  0,sizeof(DOSFILE));
	memset(&queue_record,		0,sizeof(QUEUE_RECORD));
	assert(stackavail() > 2000);
	assert(nodestr != NULL && sendcount != NULL);

  if(fileexist(OUTBOUND_FILE) == 255)
  {
	if(dosfopen(OUTBOUND_FILE,OPEN_WRIT|OPEN_CREATE|OPEN_DENYWRIT,&outbound)==-1)
	   return FALSE;
  }
  else
  {
	if(dosfopen(OUTBOUND_FILE,OPEN_WRIT|OPEN_APPEND|OPEN_DENYWRIT,&outbound)== -1)
	  return FALSE;
  }


	for(i=0,num_remotes=0;i<2048;i++)
	{
		if(nodestr[i]==0x20)
		num_remotes++;
		else if(nodestr[i]==NULL)
		{
			num_remotes++;
			break;
		}
	}

	if((remote_akas=(REMOTE_AKA*)bmalloc(num_remotes*sizeof(REMOTE_AKA)))==NULL)
	{
		dosfclose(&outbound);
		writeFidolog("Could not allocate buffer for AKA's.",BLOCK);
		return(FALSE);
	}

	i=0;
	tptr=ptr=nodestr;
	first=FALSE;
	done=FALSE;

	while(done!=TRUE)
	{
		if((ptr=strchr(tptr,' '))!=NULL)
		{
			first=FALSE;
			tmp=*ptr;
			*ptr=NULL;
			maxstrcpy(remote_akas[i].str,tptr,sizeof(remote_akas[i].str));
			i++;
			*ptr=tmp;
			tptr=++ptr;
		}
		else if(first==TRUE)
		{
			first=FALSE;
			tmp=*ptr;
			*ptr=NULL;
			maxstrcpy(remote_akas[i].str,tptr,sizeof(remote_akas[i].str));
			i++;
			*ptr=tmp;
			tptr=++ptr;
			done=TRUE;
		}
		else
		{
			first=FALSE;
			maxstrcpy(remote_akas[i].str,tptr,sizeof(remote_akas[i].str));
			i++;
			done=TRUE;
		}
	}

	// Outer parens ar to limit the scope of Q.
	{
	cNEWQ  Q;
	if(Q.getCount()==0)
		{
			dosfclose(&outbound);
	  if(remote_akas) bfree(remote_akas);
	  remote_akas = NULL;
			return(FALSE);
		}
	}

	int lastRecord = 0;
	for(i=0;i<num_remotes;i++)
	{
	updateLastDateOn(remote_akas[i].str);
	if(EnableDLO) processDLO(remote_akas[i].str);
		if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3 )
		{
			sprintf(logBuf,"Remote AKA %d = %s",i,remote_akas[i].str);
			writeFidolog(logBuf,BLOCK);
		}
	}

	// Outer parens are to limit the scope of Q;
	{
	cNEWQ Q;
	while((lastRecord = Q.getNextRecord(queue_record,lastRecord+1)) != -1)
	{
		if(queue_record.flag & Q_POLL && match_address(queue_record.nodestr,nodestr)==TRUE)
		{
		 Q.removePoll(queue_record.nodestr);
		 continue;
		}

			if(strstr(queue_record.filename,".REQ") && capFlags.NRQ)
			{
				writeFidolog("File request not sent. Remote system does not allow FREQ at this time",BLOCK);
				continue;
			}

/* If the file listed in the queue_record does not exist, remove its record and
	 build the outbound list again.
*/
		if(!(queue_record.flag&Q_POLL) && !(queue_record.flag & Q_POLLED))
		{
			if(fileexist(queue_record.filename)==255)
			{
				Q.removeEntry(queue_record.filename);
				continue;
			}
		}
		else
		{
			for(j=0;j<num_remotes;j++)
			{
				if(match_address(remote_akas[j].str,queue_record.nodestr)==TRUE)
				{
					maxstrcpy(out_poll,queue_record.nodestr,sizeof(out_poll));
					break;
				}
			}
		}

		for(j=0;j<num_remotes;j++)
		{

			if(status==INBOUND)
			{
				if(!(queue_record.flag&Q_FILEREQ) && !(queue_record.flag&Q_POLL))
				{
					if(match_address(remote_akas[j].str,queue_record.nodestr)==TRUE)
					{
						*(sendcount)+=1;
						dosfputs(queue_record.filename,&outbound);
						dosfwrite("\r\n",2,&outbound);
					}
					else if((p=strchr(queue_record.nodestr,'.'))!=NULL)
					{
						*p=NULL;
						if(match_address(remote_akas[j].str,queue_record.nodestr)==TRUE)
						{
							*(sendcount)+=1;
							dosfputs(queue_record.filename,&outbound);
							dosfwrite("\r\n",2,&outbound);
						}
						*p='.';
					}
				}
			}
			else
			{
				// On an outbound call don't add polls or HOLDs to outbound list
				if(!(queue_record.flag&Q_POLL) && !(queue_record.flag&Q_HOLD))
				{
					if(match_address(queue_record.nodestr,remote_akas[j].str)==TRUE)
					{
						*(sendcount)+=1;
						dosfputs(queue_record.filename,&outbound);
						dosfwrite("\r\n",2,&outbound);
					}
					else if((p=strchr(queue_record.nodestr,'.'))!=NULL)
					{
						*p=NULL;
						if(match_address(queue_record.nodestr,remote_akas[j].str)==TRUE)
						{
							*(sendcount)+=1;
							dosfputs(queue_record.filename,&outbound);
							dosfwrite("\r\n",2,&outbound);
						}
						*p='.';
					}
				}
			}
		}
	}
	}
	dosfclose(&outbound);
	if(remote_akas!=NULL)
	{
	// Make a copy of the remote akas for later processing
	numakas = num_remotes;
	rakas=(REMOTE_AKA*)bmalloc(numakas*sizeof(REMOTE_AKA));
	if(rakas != NULL) memcpy(rakas,remote_akas,(numakas*sizeof(REMOTE_AKA)));
		bfree(remote_akas);
		remote_akas=NULL;
	}
	return(TRUE);
}

/****************************************************************************/
/* Send any messages destined for the remote system.						*/

static bool _NEAR_ LIBENTRY Send_Messages(int sendcount)
{
	int 		 RetVal=0;
	char		 zmpath[100];
	char		 zmargs[100];
  char	   batfile[MAXFLEN];
	char									 spawn_errors[5][35]={"Arg list too long!",
													 "Invalid argument!",
													 "Path or file name not found!",
													 "Exec format error!",
													 "Not enough core!"};

	assert(stackavail() > 1000);
	maxstrcpy(zmpath,ZM_SEND,sizeof(zmpath));
	assert(heapcheck()==2);


	if(sendcount==0)
	sprintf(logBuf,"No messages to send this time.     ");
  else
	sprintf(logBuf,"Beginning mail transfer: Send      ");

  fastprint(23,9,logBuf,0x0C);
  writeFidolog(logBuf,BLOCK);
  clearinbuf();
  closemodem(FALSE);

  // Look for fidosend.*
  if(findbat(batfile,"FIDOSEND") == 0)
  {
	 sprintf(zmargs,"%d %ld @PCBDSZ.LST %ld %ld",Asy.ComPortNumber,Asy.ModemSpeed,Asy.ConnectSpeed,Asy.CarrierSpeed);
	 if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
	 {
	   sprintf(logBuf,"Shelling to %s",findstartofname(batfile));
	   writeFidolog(logBuf,BLOCK);
	 }
	 RetVal = performshell(batfile,zmargs,SHELLVIACOMMAND,PcbData.PriorityProtocols,PcbData.MinimizeProtocols & 1,PcbData.MinimizeProtocols & 2,-1);
  }
  else if(srchpath(zmpath) != -1)
  {
	 if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
	 {
	   sprintf(logBuf,"Shelling to %s",findstartofname(zmpath));
	   writeFidolog(logBuf,BLOCK);
	 }
	 RetVal = performshell(zmpath,"@PCBDSZ.LST /FORCE",SHELLVIACOMMAND,PcbData.PriorityProtocols,PcbData.MinimizeProtocols & 1,PcbData.MinimizeProtocols & 2,-1);
  }
  else
	 writeFidolog("Could not locate zmodem send module",BLOCK);

  reopenport();

  #ifdef __OS2__
	RetVal = 0;
  #endif

  // Log transfer activity
  if(RetVal == 0)
	sprintf(logBuf,"End of mail transfer:Send");
  else
	sprintf(logBuf,"Unsuccessful transfer:Send");

  writeFidolog(logBuf,BLOCK);

  if (Asy.Online == REMOTE)
  {
	online();			  //lint !e534	this just sets the state
	if (cdstillup() == 0) Asy.LostCarrier = TRUE;
  }

  if (RetVal != 0)
  {
	sprintf(logBuf,"Error %04d running zmodem: %-.25s. Aborted",errno,spawn_errors[errno]);
	fastprint(START_COL,STATUS_LINE,logBuf,0x0c);
	writeFidolog(logBuf,BLOCK);
	tickdelay(TWOSECONDS);
	return(FALSE);
  }

	if(fileexist("PCBDSZ.LST") !=255) unlink("PCBDSZ.LST");
	return(TRUE);

}

/****************************************************************************/
/* Receive any messages from the remote system. 							*/

static bool _NEAR_ LIBENTRY Receive_Messages(void)
{
int 			RetVal=0;
char			workdir[80];
char			zmpath[100];
char	  batfile[MAXFLEN];
char	  zmargs[100];
char	 spawn_errors[5][35]={"Arg list too long!",
							  "Invalid argument!",
							  "Path or file name not found!",
							  "Exec format error!",
							  "Not enough core!"};

  checkstack();
	memset(workdir,0,sizeof(workdir));
	memset(zmpath,0,sizeof(zmpath));

	stripright(IN_DIR,' ');
	stripright(IN_DIR,'\\');

  maxstrcpy(workdir,IN_DIR,sizeof(workdir)-20);
	strcat(workdir," /FORCE");
	strcat(workdir," /NODLPATH");

	clearinbuf();
	closemodem(FALSE);
	maxstrcpy(zmpath,ZM_RECV,sizeof(zmpath));

  maxstrcpy(logBuf,"Beginning mail transfer: Receive.",sizeof(logBuf));
	writeFidolog(logBuf,BLOCK);
  fastprint(23,9,logBuf,0x0C);


	// First look for fidorecv.CMD. If it's there execute it
  if(findbat(batfile,"FIDORECV") == 0)
	{
		sprintf(zmargs,"%d %ld %s %ld %ld",Asy.ComPortNumber,Asy.ModemSpeed,IN_DIR,Asy.ConnectSpeed,Asy.CarrierSpeed);
	if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
	{
	  sprintf(logBuf,"Shelling to %-1.15s",findstartofname(batfile));
	  writeFidolog(logBuf,BLOCK);
	}
	RetVal = performshell(batfile,zmargs,SHELLVIACOMMAND,PcbData.PriorityProtocols,PcbData.MinimizeProtocols & 1,PcbData.MinimizeProtocols & 2,-1);
	}
  // If fidorecv.bat/cmd isn't found, look for zmrecv.exe and execute it
	else if(srchpath(zmpath) != -1)
	{
	if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
	{
	  sprintf(logBuf,"Shelling to %-1.15s",findstartofname(zmpath));
	  writeFidolog(logBuf,BLOCK);
	}
	RetVal = performshell(zmpath,workdir,SHELLVIACOMMAND,PcbData.PriorityProtocols,PcbData.MinimizeProtocols & 1,PcbData.MinimizeProtocols & 2,-1);
	}
	else
		writeFidolog("Could not find zmodem receive module.",BLOCK);

  #ifdef __OS2__
  RetVal = 0;
  #endif

	reopenport();

	if(RetVal == 0)
	maxstrcpy(logBuf,"End of mail transfer:Recv.",sizeof(logBuf));
	else
	maxstrcpy(logBuf,"Unsuccessful transfer:Recv.",sizeof(logBuf));

	writeFidolog(logBuf,BLOCK);

	if (Asy.Online == REMOTE)
	{
	online();			  //lint !e534	this just sets the state
	if (cdstillup() == 0) Asy.LostCarrier = TRUE;
	}

	if (RetVal != 0)
	{
	sprintf(logBuf,"Error %04d running zmodem: %-1.25s\n",errno,spawn_errors[errno%5]);
		fastprint(START_COL,STATUS_LINE,logBuf,0x0c);
	writeFidolog(logBuf,BLOCK);
	tickdelay(ONESECOND);
		return(FALSE);
	}
	strcat(IN_DIR,"\\");
  writeFidolog("Mail Run Complete.",BLOCK);
	return(TRUE);
}

/****************************************************************************/
/* Parse logfile line to test for good transfers.						 */

static bool _NEAR_ LIBENTRY parse_log_line(char *logline, int direction)
{

int 				field = 0;
char				*tptr = NULL;


  if(!logline) return FALSE;

	tptr=logline;

  while(*tptr==0x20) tptr++;

  if(*tptr == '\x0') return FALSE;

  if(*tptr=='z' || *tptr=='Z')
	{
	for(field=0;field<11;field++)
		{
	  if(field==0) tptr=strtok(logline," ");
	  else		   tptr=strtok(NULL," ");
		}

	if(tptr)
		{
	  //maxstrcpy(filename,tptr,sizeof(filename));
	  maxstrcpy(filename,tptr,sizeof(filename));
	  maxstrcpy(logBuf,filename,sizeof(logBuf)-12);
	  stripright(logBuf,' ');
	  if(direction == SEND) strcat(logBuf," sent.");
	  else if(direction == RECV) strcat(logBuf," received.");
	  else if(direction == FREQ) strcat(logBuf," requested.");
	  else strcat(logBuf," transfered.");
	  writeFidolog(logBuf,BLOCK);
		}
		else
	  return(FALSE);

		return(TRUE);
	}
	return(FALSE);
}

/****************************************************************************/
/* Cleanup code. Delete sent files, cleanup the queue file. 								*/

static void _NEAR_ LIBENTRY cleanup(void)
{

char				 logline[100];
int 				 i = 0;
QUEUE_RECORD		 queuerec;
DOSFILE 			 logfile;

  memset(logline  ,0,sizeof(logline));
  memset(&queuerec ,'Z',sizeof(QUEUE_RECORD));

  checkstack();

  if(out_poll[0] != '\x0')
  {
	cNEWQ  Q;
		Q.removePoll(out_poll);
	}

  if(fileexist("FIDOSEND.LOG") != 255 && dosfopen("FIDOSEND.LOG",OPEN_READ|OPEN_DENYNONE,&logfile)==0)
  {
	while(read_line(&logfile,logline,sizeof(logline)-1)!=-1)
	{
	  if(PcbData.FidoLogLevel >= 4) writeFidolog(logline,BLOCK);

	  if(parse_log_line(logline,SEND)==TRUE)
	  {
	  cNEWQ  Q;

		if(PcbData.FidoLogLevel >= 4) writeFidolog(logline,BLOCK);
		i = 0;
		while( (i = Q.getNextRecord(queuerec,i+1)) != -1)
		{
		   if((strcmpi(queuerec.filename,filename)==0) && isRightAddress(queuerec) && !(queuerec.flag&Q_POLL) && !(queuerec.flag & Q_POLLED))
		   {
			   Q.removeEntry(i);
			   break;
		   }
		}
	  }
	}
	dosfclose(&logfile);
	unlink("FIDOSEND.LOG");
  }
  else
  {
	  if(PcbData.FidoLogLevel >= 6)
		  writeFidolog("FIDOSEND.LOG does not exist.",BLOCK);
  }

  if(fileexist("FIDORECV.LOG") != 255 && dosfopen("FIDORECV.LOG",OPEN_READ|OPEN_DENYNONE,&logfile) == 0)
  {
	  while(read_line(&logfile,logline,sizeof(logline)-1)!=-1) parse_log_line(logline,RECV);
	  dosfclose(&logfile);
	  unlink("FIDORECV.LOG");
  }
  else
  {
		if(PcbData.FidoLogLevel >= 6) writeFidolog("FIDORECV.LOG is missing. Cannot log received files.",BLOCK);
  }

  memset(filename,0,sizeof(filename));
  maxstrcpy(filename,getenv("DSZLOG"),sizeof(filename));

  if(filename[0] != '\x0' && fileexist(filename) != 255) unlink(filename);

  if(rakas != NULL)
  {
	bfree(rakas);
	rakas = NULL;
  }
  return;
}

/****************************************************************************/
/* Check for FREQ requests. 												*/

static void _NEAR_ LIBENTRY handle_freq(int *sendcount, char *nodestr)
{
char			logline[100];
char			reqline[100];
char			reqpwrd[20];
char			req_file[100];
char	  tmp_name[200];
char			tmp;
char			*ptr;
char			*tptr;
int 			i = 0;
uint	  zone;
uint	  net;
uint	  node;
uint	  point;
bool			found;
bool			first;
bool			done;
DOSFILE 		logfile;
DOSFILE 		reqfile;
DOSFILE 		outbound;
NODE_REC		noderec;
FREQ_LIMIT		slimit;
FREQ_LIMIT		dlimit;
FUSERS				user;
ffblk				fblk;
int 		  dosmaxfind;
ffblk		  file_block;

#ifdef __OS2__
int 		  DirHandle;
#endif


	found=FALSE;

	memset(logline ,0,sizeof(logline));
	memset(reqline ,0,sizeof(reqline));
	memset(req_file,0,sizeof(req_file));
	memset(tmp_name,0,sizeof(tmp_name));
	memset(&slimit,0,sizeof(slimit));
	memset(&dlimit,0,sizeof(dlimit));

	assert(sendcount != NULL);
	assert(stackavail() > 2000);
  fido_nodestr_to_int(nodestr,user.zone,user.net,user.node,user.point);

	sprintf(tmp_name,"%s%s",directory_info.incoming_packets,"*.REQ");

  dosmaxfind = 1;
  #ifdef __OS2__
	DirHandle = SYSTEMDIRHANDLE;
  #endif
  if(dosfindfirst(tmp_name,&fblk,0,&dosmaxfind PDIRHANDLE) != 0)  return;
  genResponseMsg(user,FREQINFO,NULL);

  if(read_fido_config(FREQ_RECS|MAGIC_RECS|DENY_RECS) == FALSE) return;

	if(Asy.CarrierSpeed<freq_info.baud && freq_info.baud!=0)
	{
		genResponseMsg(user,FREQFAIL,"baud rate too low.");
		return;
	}

	for(i=0;i<num_deny;i++)
	{
	if(!deny_list) break;
		deny_list[i].nodestr[sizeof(deny_list[i].nodestr)-1]=NULL;
		stripright(deny_list[i].nodestr,' ');

		done=FALSE;
		found=FALSE;
		first=TRUE;
		tptr=ptr=nodestr;

		while(done!=TRUE)
		{
			if((ptr=strchr(tptr,' '))!=NULL)
			{
				first=FALSE;
				tmp=*ptr;
		*ptr='\x0';
		if(tptr && deny_list && strcmpi(tptr,deny_list[i].nodestr)==0)
				{
					done=TRUE;
					found=TRUE;
				}
				*ptr=tmp;
				tptr=++ptr;
			}
			else if(first==TRUE)
			{
				first=FALSE;
		if(tptr && deny_list && strcmpi(tptr,deny_list[i].nodestr)==0)
				{
					done=TRUE;
					found=TRUE;
				}
				else
				done=TRUE;
			}
			else
			{
		if(tptr && deny_list && strcmpi(tptr,deny_list[i].nodestr)==0)
				{
					done=TRUE;
					found=TRUE;
				}
				else
				done=TRUE;
			}
		}
		if(found==TRUE)
		{
			genResponseMsg(user,FREQFAIL,"Access Denied");
			return;
		}
	}

	switch(freq_info.listed)
	{
		case	'A':  break;

		case	'U':  done=FALSE;
					found=FALSE;
					first=TRUE;
					tptr=ptr=nodestr;

					while(done!=TRUE)
					{
						if((ptr=strchr(tptr,' '))!=NULL)
						{
							first=FALSE;
							tmp=*ptr;
							*ptr=NULL;

							if(find_fido_rec(tptr,FALSE,FALSE,NULL)!=-1)
							{
								done=TRUE;
								found=TRUE;
							}
							*ptr=tmp;
							tptr=++ptr;
						}
						else if(first==TRUE)
						{
							first=FALSE;
							if(find_fido_rec(nodestr,FALSE,FALSE,NULL)!=-1)
							{
								done=TRUE;
								found=TRUE;
							}
							else
							done=TRUE;
						}
						else
						{
							if(find_fido_rec(tptr,FALSE,FALSE,NULL)!=-1)
							{
								done=TRUE;
								found=TRUE;
							}
							else
							done=TRUE;
						}
					}
					if(found!=TRUE)
					{
						 genResponseMsg(user,FREQFAIL,"Access Denied. User record required but not found");
						 return;
					}
					break;

		case	'N':  done=FALSE;
					found=FALSE;
					first=TRUE;
					tptr=ptr=nodestr;

					while(done!=TRUE)
					{
						if((ptr=strchr(tptr,' '))!=NULL)
						{
							first=FALSE;
							tmp=*ptr;
							*ptr=NULL;
							fido_nodestr_to_int(tptr,zone,net,node,point);
							if(get_node(zone,net,node,&noderec)!=FALSE)
							{
								done=TRUE;
								found=TRUE;
							}
							*ptr=tmp;
							tptr=++ptr;
						}
						else if(first==TRUE)
						{
							first=FALSE;
							fido_nodestr_to_int(nodestr,zone,net,node,point);
							if(get_node(zone,net,node,&noderec)!=FALSE)
							{
								done=TRUE;
								found=TRUE;
							}
							else
							done=TRUE;
						}
						else
						{
			  fido_nodestr_to_int(tptr,zone,net,node,point);
							if(get_node(zone,net,node,&noderec)!=FALSE)
							{
								done=TRUE;
								found=TRUE;
							}
							else
							done=TRUE;
						}
					}
					if(found!=TRUE)
					{
						genResponseMsg(user,FREQFAIL,"Requesting node not found in nodelist");
						return;
					}
					break;

		case	'L':  done=FALSE;
					found=FALSE;
					first=TRUE;
					tptr=ptr=nodestr;

					while(done!=TRUE)
					{
						if((ptr=strchr(tptr,' '))!=NULL)
						{
							first=FALSE;
							tmp=*ptr;
							*ptr=NULL;
							if(find_fido_rec(tptr,FALSE,FALSE,NULL)!=-1)
							{
								done=TRUE;
								found=TRUE;
							}
							*ptr=tmp;
							tptr=++ptr;
						}
						else if(first==TRUE)
						{
							first=FALSE;
							if(find_fido_rec(nodestr,FALSE,FALSE,NULL)!=-1)
							{
								done=TRUE;
								found=TRUE;
							}
							else
							done=TRUE;
						}
						else
						{

							if(find_fido_rec(nodestr,FALSE,FALSE,NULL)!=-1)
							{
								done=TRUE;
								found=TRUE;
							}
							else
							done=TRUE;
						}
					}
					if(found!=TRUE)
					{
						done=FALSE;
						found=FALSE;
						first=TRUE;
						tptr=ptr=nodestr;

						while(done!=TRUE)
						{
							if((ptr=strchr(tptr,' '))!=NULL)
							{
								first=FALSE;
								tmp=*ptr;
								*ptr=NULL;
				fido_nodestr_to_int(tptr,zone,net,node,point);
								if(get_node(zone,net,node,&noderec)!=FALSE)
								{
									done=TRUE;
									found=TRUE;
								}
								*ptr=tmp;
								tptr=++ptr;
							}
							else if(first==TRUE)
							{
								first=FALSE;
				fido_nodestr_to_int(nodestr,zone,net,node,point);
								if(get_node(zone,net,node,&noderec)!=FALSE)
								{
									done=TRUE;
									found=TRUE;
								}
								else
								done=TRUE;
							}
							else
							{
				fido_nodestr_to_int(tptr,zone,net,node,point);
								if(get_node(zone,net,node,&noderec)!=FALSE)
								{
									done=TRUE;
									found=TRUE;
								}
								else
								done=TRUE;
							}
						}
						if(found!=TRUE)
						{
							genResponseMsg(user,FREQFAIL,"User not found in nodelist or in user file.");
							return;
						}
					}
					break;
	}

	if(fileexist("FIDORECV.LOG") == 255 || dosfopen("FIDORECV.LOG",OPEN_READ|OPEN_DENYRDWR,&logfile)==-1)
	return;

  if(dosfopen(OUTBOUND_FILE,OPEN_CREATE|OPEN_RDWR|OPEN_DENYWRIT,&outbound)==-1)
	{
		dosfclose(&logfile);
		return;
	}

	read_freq_record(nodestr,dlimit);

	while(read_line(&logfile,logline,sizeof(logline)-1)!=-1)
	{
		if(parse_log_line(logline,RECV)==TRUE)
		{
			if((strstr(filename,".REQ")!=NULL) || (strstr(filename,".req")!=NULL))
			{
				if(fileexist("FIDORECV.LOG") == 255 || dosfopen(filename,OPEN_READ|OPEN_DENYRDWR,&reqfile)==-1)
				{
					dosfclose(&logfile);
					dosfclose(&outbound);
					writeFidolog("Could not read file request",BLOCK);
					genResponseMsg(user,FREQFAIL,"Could not read request.");
					return;
				}

				while(read_line(&reqfile,reqline,sizeof(reqline)-1)!=-1)
				{
					stripright(reqline,' ');
					if(reqline[0] == '\x0') continue;
					// get password, if any, from request
					char * pwrdptr=reqline;
					while(*pwrdptr != ' ' && *pwrdptr != '\x0' && *pwrdptr != '\r') pwrdptr++;
					while(*pwrdptr == ' ' && *pwrdptr != '\x0' && *pwrdptr != '\r') pwrdptr++;
					if(*pwrdptr == '\x0' || *pwrdptr == '\r') memset(reqpwrd,0,sizeof(reqpwrd));
					else
					{
						*(pwrdptr-1) = '\x0';
						maxstrcpy(reqpwrd,pwrdptr,sizeof(reqpwrd));
						stripright(reqpwrd,' ');
					}

					sprintf(logBuf,"%-1.50s requested.",reqline);
					writeFidolog(logBuf,BLOCK);
					found=FALSE;
					for(i=0;i<num_magic;i++)
					{
						if(strcmpi(magic_list[i].MagicName,reqline)==0)
						{
							if(strcmpi(magic_list[i].Password,reqpwrd) != 0)
							{
								 // $$$ Adding the file name to the string
								 char report[100];
								 sprintf(report,"Incorrect Password. File: %-1.15s",filename);
								 genResponseMsg(user,FREQFAIL,report);
								 found = TRUE;
								 break;
							}
							found=TRUE;
							maxstrcpy(req_file,magic_list[i].RealName,sizeof(req_file));

			  dosmaxfind = 1;
			  #ifdef __OS2__
				DirHandle = SYSTEMDIRHANDLE;
			  #endif
			  if(dosfindfirst(req_file,&file_block,0,&dosmaxfind PDIRHANDLE)==0)
							{
				slimit.bytes+=(file_block.ff_fsize/1024);
				dlimit.bytes+=(file_block.ff_fsize/1024);
				slimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
				dlimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
								*(sendcount)+=1;
								if((ptr=strrchr(magic_list[i].RealName,'\\'))!=NULL)
								{
									ptr++;
									tmp=*ptr;
									*ptr=NULL;
									maxstrcpy(tmp_name,magic_list[i].RealName,sizeof(tmp_name));
				  strcat(tmp_name,file_block.ff_name);
									*ptr=tmp;
								}
								if((slimit.bytes<=freq_info.sbytes || freq_info.sbytes==0)
									&& (dlimit.bytes<=freq_info.dbytes || freq_info.dbytes==0)
									&& (slimit.time<=freq_info.stime || freq_info.stime==0)
									&& (dlimit.time<=freq_info.dtime || freq_info.dtime==0))
								{
									dosfputs(tmp_name,&outbound);
									dosfwrite("\r\n",2,&outbound);
								}

				while(dosfindnext(&file_block,&dosmaxfind PDIRHANDLE2)==0)
								{
				  slimit.bytes+=(file_block.ff_fsize/1024);
				  dlimit.bytes+=(file_block.ff_fsize/1024);
				  slimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
				  dlimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
									*(sendcount)+=1;
									if((ptr=strrchr(magic_list[i].RealName,'\\'))!=NULL)
									{
										ptr++;
										tmp=*ptr;
										*ptr=NULL;
										maxstrcpy(tmp_name,magic_list[i].RealName,sizeof(tmp_name));
					strcat(tmp_name,file_block.ff_name);
										*ptr=tmp;
									}
									if((slimit.bytes<=freq_info.sbytes || freq_info.sbytes==0)
										&& (dlimit.bytes<=freq_info.dbytes || freq_info.dbytes==0)
										&& (slimit.time<=freq_info.stime || freq_info.stime==0)
										&& (dlimit.time<=freq_info.dtime || freq_info.dtime==0))
									{
										dosfputs(tmp_name,&outbound);
										dosfwrite("\r\n",2,&outbound);
									}
								}
							}
						}
					}

					if(found==FALSE)
					{
						for(i=0;i<num_reqs;i++)
						{
							assert(sizeof(req_file) > (strlen(reqlist[i].Path) + strlen(reqline)));
							sprintf(req_file,"%s%s",reqlist[i].Path,reqline);

			  dosmaxfind = 1;
			  #ifdef __OS2__
				DirHandle = SYSTEMDIRHANDLE;
			  #endif
			  if(dosfindfirst(req_file,&file_block,0,&dosmaxfind PDIRHANDLE)==0)
							{
				found = TRUE;
								// If password failure. report and abort
								if(strcmpi(reqlist[i].Password,reqpwrd) != 0)
								{

									 // $$$ Adding the file name to the string
									 char report[100];
									 sprintf(report,"Incorrect Password. File: %-1.15s",filename);
									 genResponseMsg(user,FREQFAIL,report);
									 break;
								}
				slimit.bytes+=(file_block.ff_fsize/1024);
				dlimit.bytes+=(file_block.ff_fsize/1024);
				slimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
				dlimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
								*(sendcount)+=1;
				maxstrcpy(tmp_name,reqlist[i].Path,sizeof(tmp_name)-14);
				strcat(tmp_name,file_block.ff_name);
								if((slimit.bytes<=freq_info.sbytes || freq_info.sbytes==0)
								&& (dlimit.bytes<=freq_info.dbytes || freq_info.dbytes==0)
								&& (slimit.time<=freq_info.stime || freq_info.stime==0)
																&& (dlimit.time<=freq_info.dtime || freq_info.dtime==0))
								{
									dosfputs(tmp_name,&outbound);
									dosfwrite("\r\n",2,&outbound);
								}

				while(dosfindnext(&file_block,&dosmaxfind PDIRHANDLE2)==0)
								{
				  found = TRUE;
				  slimit.bytes+=(file_block.ff_fsize/1024);
				  dlimit.bytes+=(file_block.ff_fsize/1024);
				  slimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
				  dlimit.time+=(numsecsforxfer(file_block.ff_fsize)/60);
									*(sendcount)+=1;
									strcpy(tmp_name,reqlist[i].Path);
				  strcat(tmp_name,file_block.ff_name);
									if((slimit.bytes<=freq_info.sbytes || freq_info.sbytes==0)
										&& (dlimit.bytes<=freq_info.dbytes || freq_info.dbytes==0)
										&& (slimit.time<=freq_info.stime || freq_info.stime==0)
										&& (dlimit.time<=freq_info.dtime || freq_info.dtime==0))
									{
										dosfputs(tmp_name,&outbound);
										dosfwrite("\r\n",2,&outbound);
									}
								}
							}
						}
					}
		  if(!found)
		  {

			 // $$$ Adding the file name to the string
			 char report[100];
			 sprintf(report,"File not found. File: %-1.15s",findstartofname(filename));
			 genResponseMsg(user,FREQFAIL,report);
			 break;
		  }
				}
				dosfclose(&reqfile);
				unlink(filename);
			}
		}
	}
	dosfclose(&logfile);
	dosfclose(&outbound);
	free_fido_memory();
	write_freq_record(dlimit);
	return;
}

/****************************************************************************/
/* Read in the record for the node. 										*/

static void _NEAR_ LIBENTRY read_freq_record(char *nodestr,FREQ_LIMIT &dlimit)
{
DOSFILE 		limit_file;
FREQ_LIMIT		ftmp;
char			*ptr;
char			*tptr;
char			tmp;
bool			done;
bool			first;
bool			found;
unsigned		fdate;

	memset(&dlimit,0,sizeof(FREQ_LIMIT));
	memset(&ftmp,0,sizeof(FREQ_LIMIT));

	if(fileexist(F_LIMIT_FILE) == 255 || dosfopen(F_LIMIT_FILE,OPEN_READ|OPEN_DENYWRIT,&limit_file)==-1)
	return;

	if(dosfread(&fdate,sizeof(fdate),&limit_file)==-1)
	{
		dosfclose(&limit_file);
		return;
	}

	if(fdate!=getjuliandate())
	{
		dosfclose(&limit_file);

			if(fileexist(F_LIMIT_FILE) == 255 || dosfopen(F_LIMIT_FILE,OPEN_READ|OPEN_DENYWRIT|OPEN_CREATE,&limit_file)==-1)
		return;

		fdate=getjuliandate();

		if(dosfwrite(&fdate,sizeof(fdate),&limit_file)==-1)
		{
			dosfclose(&limit_file);
			return;
		}
	}

	while(dosfread(&ftmp,sizeof(FREQ_LIMIT),&limit_file)==sizeof(FREQ_LIMIT))
	{
		first=TRUE;
		done=FALSE;
		found=TRUE;
		tptr=ptr=nodestr;

		while(done!=TRUE)
		{
			if((ptr=strchr(tptr,' '))!=NULL)
			{
				first=FALSE;
				tmp=*ptr;
				*ptr=NULL;
				if(strcmp(ftmp.nodestr,tptr)==0)
				{
					memcpy(&dlimit,&ftmp,sizeof(FREQ_LIMIT));
					found=TRUE;
					done=TRUE;
				}
				*ptr=tmp;
				tptr=++ptr;
			}
			else if(first==TRUE)
			{
				first=FALSE;
				if(strcmp(ftmp.nodestr,nodestr)==0)
				{
					memcpy(&dlimit,&ftmp,sizeof(FREQ_LIMIT));
					found=TRUE;
					done=TRUE;
				}
				else
				done=TRUE;
			}
			else
			{
				if(strcmp(ftmp.nodestr,tptr)==0)
				{
					memcpy(&dlimit,&ftmp,sizeof(FREQ_LIMIT));
					found=TRUE;
					done=TRUE;
				}
				else
				done=TRUE;
			}
		}
	}
	if(found!=TRUE)
	{
		if((ptr=strchr(tptr,' '))!=NULL)
		{
			first=FALSE;
			tmp=*ptr;
			*ptr=NULL;
			maxstrcpy(dlimit.nodestr,tptr,sizeof(dlimit.nodestr));
			*ptr=tmp;
			tptr=++ptr;
			dosfseek(&limit_file,0,SEEK_END);
			dosflush(&limit_file);
			dosfwrite(&dlimit,sizeof(dlimit),&limit_file);
		}
		else
		{
			maxstrcpy(dlimit.nodestr,nodestr,sizeof(dlimit.nodestr));
			dosfseek(&limit_file,0,SEEK_END);
			dosflush(&limit_file);
			dosfwrite(&dlimit,sizeof(dlimit),&limit_file);
		}
	}
	dosfclose(&limit_file);
}

/****************************************************************************/
/* Write out the node's Freq restrictions.                                  */

static void _NEAR_ LIBENTRY write_freq_record(FREQ_LIMIT &dlimit)
{
DOSFILE 		limit_file;
FREQ_LIMIT		ftmp;
unsigned		fdate;

	memset(&dlimit,0,sizeof(FREQ_LIMIT));

	if(fileexist(F_LIMIT_FILE) == 255 || dosfopen(F_LIMIT_FILE,OPEN_RDWR|OPEN_DENYRDWR,&limit_file)==-1)
	return;

	if(dosfread(&fdate,sizeof(fdate),&limit_file)==-1)
	{
		dosfclose(&limit_file);
		return;
	}

	if(fdate!=getjuliandate())
	{
		fdate=getjuliandate();
		dosfclose(&limit_file);

		if(dosfopen(F_LIMIT_FILE,OPEN_RDWR|OPEN_DENYRDWR|OPEN_CREATE,&limit_file)==-1)
		return;

		if(dosfwrite(&fdate,sizeof(fdate),&limit_file)==-1)
		{
			dosfclose(&limit_file);
			return;
		}
	}

	while(dosfread(&ftmp,sizeof(FREQ_LIMIT),&limit_file)==sizeof(FREQ_LIMIT))
	{
		if(strcmp(ftmp.nodestr,dlimit.nodestr)==0)
		{
	  dosfseek(&limit_file,0 - sizeof(FREQ_LIMIT),SEEK_CUR);
			dosflush(&limit_file);
			dosfwrite(&dlimit,sizeof(FREQ_LIMIT),&limit_file);
			break;
		}
	}
	dosfclose(&limit_file);
}

/****************************************************************************/
/* Transfer the messages to and from the remote system. 					*/

void LIBENTRY TransferMessages(int *sendcount,char *nodestr,int status,int protocol)
{

 char	nameBuf[26];
 char * namePtr = NULL;
 char	batfile[MAXFLEN];
// #ifdef __OS2__
// int	  DirHandle;
// #endif

  checkstack();
	assert(sendcount != NULL );
	assert( nodestr != NULL);

	// These calles are to avoid the time expired messages from pcboard.
	Control.WatchKbdClock = FALSE;
	Control.WatchSessionClock = FALSE;

	if(nodestr == NULL) namePtr = NULL;
	else		 namePtr = strchr(nodestr,'/');

	if(namePtr != NULL)
	{
		while(*namePtr != ' ' && *namePtr != '\x0' && isascii(*namePtr)) namePtr++;
		maxstrcpy(nameBuf,nodestr,size_t(namePtr-nodestr+1) > sizeof(nameBuf) ? sizeof(nameBuf) : size_t(namePtr - nodestr+1));
	}
	else
		maxstrcpy(nameBuf,"FIDO user",sizeof(nameBuf));

	if(sizeof(nameBuf) - strlen(nameBuf) > 16) strcat(nameBuf," (FIDO Transfer)");
	maxstrcpy(Status.DisplayName,nameBuf,sizeof(Status.DisplayName));
	writeusernetstatus(HANDLEMAIL,"FIDO mail transfer");
	if(status==INBOUND)
	{
	//if(protocol!=XMODEM)
	//make_batch_list(nodestr,sendcount,status);

		if(protocol!=XMODEM)
		{
		   Receive_Messages();
		   TK_CopyFile(getenv("DSZLOG"),"FIDORECV.LOG");
		}
		else
		   receive_xmodem_batch();

		make_batch_list(nodestr,sendcount,status);
		if(protocol!=XMODEM)
		{

		  handle_freq(sendcount,nodestr);
		  Send_Messages(*sendcount);
		}
		else
		   send_xmodem_batch();

		if(protocol != XMODEM) TK_CopyFile(getenv("DSZLOG"),"FIDOSEND.LOG");
		Disconnect();
		cleanup();
	}
	else if(status==OUTBOUND)
	{

		make_batch_list(nodestr,sendcount,status);
		if(protocol!=XMODEM)
		{
		  Send_Messages(*sendcount);
		  writeFidolog("Getting DSZ log data",BLOCK);
		  TK_CopyFile(getenv("DSZLOG"),"FIDOSEND.LOG");
		}
		else
		   send_xmodem_batch();



		 if(protocol!=XMODEM)
		 {
			  Receive_Messages();
			  TK_CopyFile(getenv("DSZLOG"),"FIDORECV.LOG");
		 }
		 else
			  receive_xmodem_batch();

		Disconnect();
		writeFidolog("Cleaning up.",BLOCK);
		cleanup();
	}

  modemoffhook();
  Status.DisplayName[0] = '\x0';
  clearusernet();

	if(nodestr != NULL)
	{
	sprintf(logBuf,"Disconnected from %-1.60s",nodestr);
		writeFidolog(logBuf,BLOCK);
	sprintf(logBuf,"Ending mail run with %-1.60s ",nodestr);
	}
	else
		sprintf(logBuf,"Ending mail run.");

	writeFidolog(logBuf,BLOCK_END);

  moveFREQs();

	if(findbat(batfile,"PREPROC") == 0)
		{
	   writeFidolog("Shelling to post transfer batch/cmd file. (PREPROC)",BLOCK);
	   performshell(batfile," ",SHELLVIACOMMAND,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,-1);
		}


		if(PcbData.FidoImportAfterXfer)
		{
			if(okaytohandlemail())
			{
	  cTOSSER * tosser = NULL;

				free_fido_memory();
				cls();
		tosser = new cTOSSER;
		if(tosser)
		{
		  tosser->importMessages();
		  delete(tosser);
		}
			}
			else
		writeFidolog("Another node tossing. Could not import after transfer.",BLOCK);
		}



  Status.FileXfer = FALSE;

	// Check to see if the batch file postcall.bat exists in the path
  if(findbat(batfile,"POSTCALL") == 0)
	{
		 writeFidolog("Shelling to post transfer batch/cmd file. (POSTCALL)",BLOCK);
	 performshell(batfile," ",SHELLVIACOMMAND,PcbData.PriorityShells,PcbData.MinimizeShells & 1,PcbData.MinimizeShells & 2,-1);
	}

	Asy.IgnoreCDLoss = FALSE;

}

/****************************************************************************/
/* Read in a line of characters from the file pointed to by filebuffer. 	*/
/* It reads until it finds a carriage return, storing the information in	*/
/* the buffer passed in.	Function returns a 0 on valid string, or a -1 if	*/
/* the end of file was reached. 											*/

int LIBENTRY read_line(DOSFILE *filebuffer,char *buffer,int max_len)
{
int 	i=0,j=0,len=0,dif=0,length=0;
char	buf[512];


  checkstack();

  if(!buffer || !filebuffer) return -1;
	j=0;
	memset(buf,0,sizeof(buf));

	while(1)
	{
		length=dosfread(buf,sizeof(buf),filebuffer);

	if(length==0 || length==-1) return(-1);

		for(i=0;i<sizeof(buf);i++,j++)
		{
	  if(j==max_len-1)
			{
		buffer[j]='\x0';
				return(0);
			}

			switch(buf[i])
			{
				case	CR:
						buf[i]=NULL;
						buffer[j]=buf[i];
						len=strlen(buf);
						dif=length-len;
						dif--;
						dosfseek(filebuffer,-dif,SEEK_CUR);
						return(0);

				case	LF: j--;
						continue;

				default:
						buffer[j]=buf[i];
			}
		}
	}
}


/****************************************************************************/
/* Make an EMSI_DAT packet to send. 										*/

void LIBENTRY make_emsi_dat(Emsi *emsi,char *password)
{


  checkstack();

	memset(emsi->addr,0,sizeof(emsi->addr));
	makeMyAkaList(emsi->addr,sizeof(emsi->addr),PRESENT);
	strcpy(emsi->pw,password);
	assert(sizeof(emsi->line) > strlen(LINE_STATUS));
	strcpy(emsi->line,LINE_STATUS);

	maxstrcpy(emsi->codes,COMPAT_CODES,sizeof(emsi->codes));
	maxstrcpy(emsi->prodcode,PRODUCT_CODE,sizeof(emsi->prodcode));
	maxstrcpy(emsi->prodname,PRODUCT_NAME,sizeof(emsi->prodname));
	maxstrcpy(emsi->prodserial,SERIAL_NUMBER,sizeof(emsi->prodserial));
	maxstrcpy(emsi->name,emsi_data.BBS_Name,sizeof(emsi->name));
	maxstrcpy(emsi->city,emsi_data.City,sizeof(emsi->city));
	maxstrcpy(emsi->sysop,emsi_data.Sysop,sizeof(emsi->sysop));
	maxstrcpy(emsi->phone,emsi_data.Phone,sizeof(emsi->phone));
	maxstrcpy(emsi->baud,emsi_data.Baud,sizeof(emsi->baud));
	maxstrcpy(emsi->flags,emsi_data.Flags,sizeof(emsi->flags));

	assert(sizeof(emsi->prodver) > (strlen(VERSION_MAJOR)+strlen(VERSION_MINOR)));
	sprintf(emsi->prodver,"%s.%s",VERSION_MAJOR,VERSION_MINOR);
}

/****************************************************************************/
/* Receive EMSI handshake from remote.										*/

bool LIBENTRY receive_emsi_handshake(char *remote_nodestr, int len, Emsi &emsi)
{
int 			incoming=0;
int 			tries=0;
int 			state=1;
char			*tptr=NULL;
char			*ptr =NULL;
char			*last=NULL;
char			*buffer=NULL;
char			*buffer2=NULL;
char			crcbuf[5];
char			crcbuf2[5];
char			lenbuf[5];
char			tmp;
bool			done= FALSE,stop=FALSE;
bool			foundrec;
bool			pwmatch;
bool			first;
unsigned short	crc 	 = 0;
unsigned long  length = 0;
long			recno;
// char 			 *p;
int 			buflen;
int 			buflen2;


	buflen=2048;
	buflen2=2048;
	foundrec=FALSE;
	pwmatch=FALSE;

  if((buffer=(char *)bmalloc(buflen))==NULL)
	{
	writeFidolog("Out of memory during handshake. Failure will occur.",BLOCK);
	return(FALSE);
	}

  if((buffer2=(char *)bmalloc(buflen2))==NULL)
	{
		writeFidolog("Could not allocate memory during handshake. Failuire will occur.",BLOCK);
	freeAndNull(&buffer);
		return(FALSE);
	}

	memset(buffer ,0,buflen);
	memset(buffer2,0,buflen2);
	memset(crcbuf ,0,sizeof(crcbuf));
	memset(crcbuf2,0,sizeof(crcbuf2));
	memset(lenbuf ,0,sizeof(lenbuf));
	memset(&emsi,0,sizeof(emsi));
	assert(stackavail() > 2000);

	if(cdstillup() == FALSE)
	writeFidolog("Carrier dropped by remote system. Handshake will fail.",BLOCK);
	tptr=buffer;
	last=tptr+buflen-1;
	do
	{
		if(kbdhit(NOBUFFER)==0x1B)
		{

	  if(buffer) bfree(buffer);
	  if(buffer2) bfree(buffer2);
	  buffer = buffer2 = NULL;
			return(FALSE);
		}

		switch(state)
		{
			case 1:
					settimer(9,TWENTY_SECONDS);
					settimer(10,SIXTY_SECONDS);
					state=2;
					break;

			case 2: tries++;
					if(tries>6)
					{
			if(buffer) bfree(buffer);
			if(buffer2) bfree(buffer2);
			buffer = buffer2 = NULL;
						return(FALSE);
					}

					if(tries>1)
					{
						sendstr("**EMSI_NAKEEC3",14); // Tell it to send again.
						sendbyte(CR);
			if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 2 Going to state 3",BLOCK);
						state=3;
						break;
					}
		  if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 2 Going to state 4",BLOCK);
					state=4;
					break;

			case 3: settimer(9,TWENTY_SECONDS);
		  if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 3 Going to state 4",BLOCK);
					state=4;
					break;

			case 4:
					done=FALSE;
					do
					{
			 //if(!kbdhit(NOBUFFER)) giveup();

			 if(kbdhit(NOBUFFER)==0x1B || !cdstillup())
			 {
			   if(buffer) bfree(buffer);
			   if(buffer2) bfree(buffer2);
			   buffer = buffer2 = NULL;
			   if(!cdstillup()) writeFidolog("Carrier lost.",BLOCK);
			   if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 4 Returning FALSE",BLOCK);
			   return(FALSE);
			 }

			 if(timerexpired(9))
			 {
			   if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 4 Going to state 2",BLOCK);
			   settimer(9,TWENTY_SECONDS); // Added without knowing effect
			   state=2;
			   done=TRUE;
			   break;
			 }

			 if(timerexpired(10))
			 {
			   if(buffer) bfree(buffer);
			   if(buffer2) bfree(buffer2);
			   buffer = buffer2 = NULL;
			   writeFidolog("Handshake timeout while receiving.",BLOCK);
			   if(PcbData.FidoLogLevel >= 6)  writeFidolog("In receive_emsi state 4 returning FALSE",BLOCK);
			   return(FALSE);
			 }

			 if(!done&&(incoming=comminkey())!=-1)	/* check for incoming */
			 {
			   if(incoming!=10)
			   *tptr=incoming;

			   if(tptr==last||*tptr==CARRIAGE_RETURN) /* CR or last char? */
			   {
				 *tptr=NULL;
				 if(strstr(buffer,"EMSI_HBT")!=NULL) // Wait for the
				 {					 // Other side.
				   state=3;
				   if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 4 setting state = 3",BLOCK);
				   done=TRUE;
				 }

				 if(strstr(buffer,"EMSI_DAT")!=NULL) // Got packet
				 {
				   state=5;
				   if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive_emsi state 4 setting state = 5",BLOCK);
				   done=TRUE;
				 }
				 tptr=buffer;
			   }
			   else if(incoming!=10) tptr++;
			 }
					} while(done!=TRUE);
					break;

			case 5:
					memset(buffer2,0,buflen2);
					memset(lenbuf,0,sizeof(lenbuf));
					memset(crcbuf,0,sizeof(crcbuf));
					memset(crcbuf2,0,sizeof(crcbuf2));
					tptr=strstr(buffer,"EMSI_DAT");
					if(tptr != NULL) memcpy(lenbuf,(tptr+8),4);
					lenbuf[4]=NULL;
					length=HexDec(lenbuf);
					if(tptr != NULL) memcpy(buffer2,tptr,(unsigned) (length+12));	 // WARNING:  watch for length overrun
					if(tptr != NULL) memcpy(crcbuf,(tptr+ (unsigned) (length+12)),4); // WARNING:	 watch for length overrun
					if(strlen(crcbuf) > sizeof(crcbuf) || strlen(buffer2) > buflen2)
					  writeFidolog("Buffer overwritten, fidofunc line 2200",BLOCK);

					crcbuf[4]=NULL;
					crc=0;
					crc=fido_updcrc(crc,(unsigned char *)buffer2,strlen(buffer2));

					sprintf(crcbuf2,"%04X",crc);
					if(strcmpi(crcbuf2,crcbuf)!=0)
					{
						sendstr("**EMSI_NAKEEC3",14);
						sendbyte(CR);
						state=4;
						sprintf(logBuf,"EMSI CRC Failure - Cal:%s Got:%s",crcbuf2,crcbuf);
						writeFidolog(logBuf,BLOCK);
						break;
					}
					else
					{

						fill_emsi(buffer,remote_nodestr,len,emsi);
//						p=tptr=remote_nodestr;
						recno=-1;
						memset(&UsersData,0,sizeof(UsersData));

						done=FALSE;
						foundrec=FALSE;
						pwmatch=FALSE;
						first=TRUE;
						tptr=ptr=remote_nodestr;

						while(done!=TRUE)
						{
							if((ptr=strchr(tptr,' '))!=NULL)
							{
								first=FALSE;
								tmp=*ptr;
								*ptr=NULL;
								if( (recno = find_fido_rec(tptr,FALSE,FALSE,NULL))!=-1)
								{
									foundrec=TRUE;
									get_fido_rec(recno);
									if(strcmpi(emsi.pw,UsersData.Password)==0)
									{
										pwmatch=TRUE;
										done=TRUE;
									}
									if(UsersData.Password[0] == ' ' || UsersData.Password[0] == '\x0')
									pwmatch = TRUE;
								}
								*ptr=tmp;
								tptr=++ptr;
							}
							else if(first==TRUE)
							{
								first=FALSE;
								if( (recno = find_fido_rec(tptr,FALSE,FALSE,NULL))!=-1)
								{
									foundrec=TRUE;
									get_fido_rec(recno);
									if(strcmpi(emsi.pw,UsersData.Password)==0)
									pwmatch=TRUE;
									if(UsersData.Password[0] == ' ' || UsersData.Password[0] == '\x0')
									pwmatch = TRUE;
									done=TRUE;
								}
								else
								done=TRUE;
							}
							else
							{
								if( (recno = find_fido_rec(tptr,FALSE,FALSE,NULL))!=-1)
								{
									foundrec=TRUE;
									get_fido_rec(recno);
									if(strcmpi(emsi.pw,UsersData.Password)==0)
									pwmatch=TRUE;
									if(UsersData.Password[0] == ' ' || UsersData.Password[0] == '\x0')
									pwmatch = TRUE;
									done=TRUE;
								}
								else
								done=TRUE;
							}
						}
						if(foundrec==TRUE)
						{
							if(pwmatch==TRUE)
							{
								sendstr("**EMSI_ACKA490",14);
								sendbyte(CR);
								sendstr("**EMSI_ACKA490",14);
								sendbyte(CR);
								writeFidolog("Sent ACK to remote.",BLOCK);

								display_fido_info(emsi);
								state=6;
							}
							else
							{
								strupr(emsi.pw);
								sprintf(logBuf,"   Expected %s, got %s",UsersData.Password,emsi.pw);
								writeFidolog(logBuf,BLOCK);
								sprintf(logBuf,"Session password: %s for node: %s is incorrect.",emsi.pw,UsersData.Name+6);
								writeFidolog(logBuf,BLOCK);
				make_emsi_dat(&emsi," ");
				strcpy(emsi.name,"Password Failure!");
				strcpy(emsi.addr,"PCBoard Incoming Session Password Failure!");
				strcpy(emsi.sysop,"Password Failure!");
				strcpy(emsi.city,"Session Password Failure!");
				sendstr("**EMSI_ACKA490",14);
				sendbyte(CR);
				sendstr("**EMSI_ACKA490",14);
				sendbyte(CR);
				send_emsi_handshake(emsi,INBOUND);
				if(buffer) bfree(buffer);
				if(buffer2) bfree(buffer2);
				buffer = buffer2 = NULL;
								return(FALSE);
							}
						}
						else
						{
							sendstr("**EMSI_ACKA490",14);
							sendbyte(CR);
							sendstr("**EMSI_ACKA490",14);
							sendbyte(CR);
							writeFidolog("Sent ACK to remote",BLOCK);

							display_fido_info(emsi);
							state=6;
						}

					}
					break;

					case 6:
						if(PcbData.FidoLogLevel >= 6) writeFidolog("In state 6 returning TRUE.",BLOCK);
						scanHiAscii(buffer,FALSE,&buflen);
						maxstrcpy(UsersData.Password,emsi.pw,sizeof(UsersData.Password));
						make_emsi_dat(&emsi,UsersData.Password);
			if(buffer) bfree(buffer);
			if(buffer2) bfree(buffer2);
			buffer = buffer2 = NULL;
						return TRUE;

			case 7:
			if(buffer) bfree(buffer);
			if(buffer2) bfree(buffer2);
			buffer = buffer2 = NULL;
						return FALSE;

		} /* End Switch */
	}while(stop!=TRUE);

  if(buffer) bfree(buffer);
  if(buffer2) bfree(buffer2);
  buffer = buffer2 = NULL;
	return TRUE;
}

/****************************************************************************/
/* Fill in struct emsi. 													*/

static void _NEAR_ LIBENTRY fill_emsi(char *packet, char *remote_nodestr, int len, Emsi &emsi)
{
char		*tptr = NULL;
char		*ptr	= NULL;
char		tmp;
int 		i	= 0;

	assert(packet != NULL && remote_nodestr != NULL);
	assert(stackavail() > 2000);
	memset(&emsi,0,sizeof(emsi));

	tptr=ptr=packet;

	ptr=strstr(tptr,"{");
	if(ptr == NULL) return;

	tptr=++ptr;
	ptr=strstr(tptr,"}{");
	if(ptr == NULL) return;

	tptr=++ptr;
	ptr=strstr(tptr,"}{");
	if(tptr == NULL) return;

	if(ptr!=NULL && tptr!=NULL)
	{
		tmp=*ptr;
		*ptr=NULL;
		maxstrcpy(remote_nodestr,tptr+1,len);
		*ptr=tmp;

		for(i=0;i<25;i++)
		{
			if(remote_nodestr[i]==0x20 || remote_nodestr[i]==0x00 || remote_nodestr[i] == 0x7D /*|| remote_nodestr[i]=='@'*/)
			{
				assert(i >=0 && i < sizeof(emsi.addr));
				emsi.addr[i]=NULL;
				break;
			}
			else
			{
				assert(i >= 0 && i < sizeof(emsi.addr) && i < len);
				if(remote_nodestr[i] != '@')
				emsi.addr[i]=remote_nodestr[i];
			}
		}
	}

	for(i=1;i<=6;i++)
	{
		tptr=++ptr;
		if((ptr=strstr(tptr,"}{"))!=NULL)
		{
			tmp=*ptr;
			*ptr=NULL;
			switch(i)
			{
				case 1:
						if(ptr-tptr!=1 && ptr-tptr!=2)
						maxstrcpy(emsi.pw,++tptr,sizeof(emsi.pw));
						break;

				case 2:
						maxstrcpy(emsi.line,++tptr,sizeof(emsi.line));
						break;

				case 3:
						maxstrcpy(emsi.codes,++tptr,sizeof(emsi.codes));
						memset(&capFlags,0,sizeof(capFlags));

						if(strstr(emsi.codes,"ZAP")) capFlags.ZAP = TRUE;
						if(strstr(emsi.codes,"ZMO")) capFlags.ZMO = TRUE;
						if(strstr(emsi.codes,"ARC")) capFlags.ARC = TRUE;
						if(strstr(emsi.codes,"XMA")) capFlags.XMA = TRUE;
						if(strstr(emsi.codes,"FNC")) capFlags.FNC = TRUE;
						if(strstr(emsi.codes,"JAN")) capFlags.JAN = TRUE;
						if(strstr(emsi.codes,"KER")) capFlags.KER = TRUE;
						if(strstr(emsi.codes,"NCP")) capFlags.NCP = TRUE;
											if(strstr(emsi.codes,"NRQ")) capFlags.NRQ = TRUE;
						break;

				case 4:
						maxstrcpy(emsi.prodcode,++tptr,sizeof(emsi.prodname));
						break;

				case 5:
						maxstrcpy(emsi.prodname,++tptr,sizeof(emsi.prodcode));
						break;

				case 6:
						maxstrcpy(emsi.prodver,++tptr,sizeof(emsi.prodver));
						break;
			}
			*ptr=tmp;
		}
	}

	tptr=++ptr;
	if((ptr=strstr(tptr,"}"))!=NULL)
	{
		tmp=*ptr;
		*ptr=NULL;
		maxstrcpy(emsi.prodserial,++tptr,sizeof(emsi.prodserial));
		*ptr=tmp;
	}

	if((ptr=strstr(tptr,"IDENT}{["))!=NULL)
	{
		tptr=ptr+6;
		ptr=tptr;

	for(i=1;i<=5;i++)
		{
			tptr=++ptr;
			if((ptr=strstr(tptr,"]["))!=NULL)
			{
				tmp=*ptr;
				*ptr=NULL;
				switch(i)
				{
					case 1:
							maxstrcpy(emsi.name,++tptr,sizeof(emsi.name));
							break;

					case 2:
							maxstrcpy(emsi.city,++tptr,sizeof(emsi.city));
							break;

					case 3:
							maxstrcpy(emsi.sysop,++tptr,sizeof(emsi.sysop));
							break;

					case 4:
							maxstrcpy(emsi.phone,++tptr,sizeof(emsi.phone));
							break;

					case 5:
							maxstrcpy(emsi.baud,++tptr,sizeof(emsi.baud));
							break;
				}
				*ptr=tmp;
			}
		}

		tptr=++ptr;
		if((ptr=strstr(tptr,"]}"))!=NULL)
		{
			tmp=*ptr;
			*ptr=NULL;
			maxstrcpy(emsi.flags,++tptr,sizeof(emsi.flags));
			*ptr=tmp;
		}
	}

	if(strcmp(emsi.prodname,PRODUCT_NAME) == 0)
	remotePCBoard = TRUE;

	//strcpy(Status.DisplayName,"Fido mail run");

	if(emsi.addr[0] != '\x0')
	  sprintf(logBuf,"Connected to %s",emsi.addr);
	else
	  sprintf(logBuf,"Connected.");

	writeFidolog(logBuf,BLOCK);

	if(PcbData.FidoLogLevel == 1 || PcbData.FidoLogLevel >= 3)
	{
		writeFidolog("Received remote system data.",BLOCK);
		logEMSI(emsi);
	}
}

/****************************************************************************/
/* Wait for a byte to come into the modem buffer.						 */

static bool _NEAR_ LIBENTRY gotabyte(int Ticks)
{
	settimer(4,Ticks);
	while(1)
	{
		if(InBytes!=0)
		return(TRUE);
		if(timerexpired(4))
		return(FALSE);
		#ifndef __OS2__
		giveup();
		#endif
	}
}

/****************************************************************************/
/* Send EMSI handshake to modem.											*/

bool LIBENTRY send_emsi_handshake(Emsi &emsi, int direction)
{
int 			incoming;
int 			state;
int 			tries;
int 			buflen;
int 			buflen2;
char			*p;
char			*tptr=NULL;
char			*last=NULL;
char			*buffer=NULL;
char			*buffer2=NULL;
char			crcbuf[5];
bool			done,stop;
long			length;
unsigned short	crc;

	buflen=2048;
	buflen2=2048;

  if((buffer=(char *)bmalloc(buflen))==NULL)
	{
	writeFidolog("Could not allocate memory during handshake. Failure will occur.",BLOCK);
	return(FALSE);
	}

  if((buffer2=(char *)bmalloc(buflen2))==NULL)
	{
		writeFidolog("Could not allcoate memory during handshake, failure will occur.",BLOCK);
	if(buffer) bfree(buffer);
	buffer = NULL;
		return(FALSE);
	}

	assert(stackavail() > 2000);
	tries=0;
	state=1;
	length=0;
	memset(buffer,0,buflen);
	memset(buffer2,0,buflen2);
	memset(crcbuf,0,sizeof(crcbuf));
	done=FALSE;
	stop=FALSE;
	tptr=buffer;
	last=tptr+buflen-1;

	do
	{
	if(kbdhit(NOBUFFER)==0x1B)
	{
	  if(buffer) bfree(buffer);
	  if(buffer2) bfree(buffer2);
	  buffer = buffer2 = NULL;
	  return(FALSE);
	}

  //else  giveup();

	switch(state)
	{
		case 1: tries=0;
				settimer(9,SIXTY_SECONDS);
		if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 1 going to state 2.",BLOCK);
				state=2;
				break;

		case 2: /* Build and send EMSI_DAT packet */

		if(direction == OUTBOUND && !strstr(emsi.codes,"NRQ")) strcat(emsi.codes,",NRQ");
		sprintf(buffer2,"{EMSI}{%s}{%s}{%s}{%s}{%s}{%s}{%s}{%s}{IDENT}{[%s][%s][%s][%s][%s][%s]}",\
				emsi.addr, emsi.pw, emsi.line, emsi.codes,
								emsi.prodcode, emsi.prodname, emsi.prodver,
								emsi.prodserial, emsi.name, emsi.city,
								emsi.sysop, emsi.phone, emsi.baud,
								emsi.flags);

				scanHiAscii(buffer2,TRUE,&buflen2);


				if(buffer2==NULL)
		{
		  if(buffer) bfree(buffer);
		  buffer = NULL;
		  if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 2 Returning FALSE.",BLOCK);
					return(FALSE);

		}

				length=strlen(buffer2);
		sprintf(buffer,"EMSI_DAT%04lX",length);
				strcat(buffer,buffer2);

				crc=0;
				crc=fido_updcrc(crc,(unsigned char *)buffer,strlen(buffer));
				sprintf(crcbuf,"%04X",crc);
				sendstr("**",2);
				writeFidolog("Sending Emsi string",BLOCK);
				sendstr(buffer,strlen(buffer));
				sendstr(crcbuf,strlen(crcbuf));
				sendbyte(CR);

				if(tries>6)
		{
		  if(buffer) bfree(buffer);
		  if(buffer2) bfree(buffer2);
		  buffer = buffer2 = NULL;
					writeFidolog("Max EMSI retries reached.",BLOCK);
					return FALSE;
		}

		if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 2 Setting state = 3.",BLOCK);

				state=3;
				tries++;
				break;

		case 3: settimer(10,TWENTY_SECONDS);

		if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 3 Setting state = 4.",BLOCK);
				state=4;
				break;

		case 4: done=FALSE;
				do
				{
		  if(kbdhit(NOBUFFER)==0x1B)
					{
			if(buffer) bfree(buffer);
			if(buffer2) bfree(buffer2);
			buffer = buffer2 = NULL;
						writeFidolog("EMSI aborted by user.",BLOCK);
			if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive emsi. State 4 Returning FALSE",BLOCK);
						return(FALSE);
					}
		  //else
		  //  giveup();

					if(timerexpired(9))
					{
						state=6;
						done=TRUE;
					}


					if(timerexpired(10))
					{
						state=2;
						done=TRUE;
					}

		  if(!done && (incoming=comminkey())!=-1)  /* check for incoming */
					{
						if(incoming!=10)
						*tptr=incoming;

						if(*tptr=='*' && *(tptr-1)=='*')
						{
							tptr++;
			  //if(!gotabyte(36))
			  if(!gotabyte(TWOSECONDS))
							{
				writeFidolog("Time out during EMSI.",BLOCK);
				if(buffer) bfree(buffer);
				if(buffer2) bfree(buffer2);
				buffer = buffer2 = NULL;
				return(FALSE);
							}
			  for(p="EMSI_ACKA490";(incoming=comminkey())==*p;p++)
							{
								// Why is this block here?
								// incoming should never be 10 because the for loop
								// would terminate first
								if(incoming!=10)
								{
									*tptr=incoming;
									tptr++;
								}

								if(*p=='0')
								{
									done=TRUE;
									stop=TRUE;
									state=5;
				  if(PcbData.FidoLogLevel == 6) writeFidolog("EMSI ACK received! State is 5",BLOCK);
									break;
								}

				if(!gotabyte(TWOSECONDS))
								{
				  writeFidolog("Time out during EMSI.",BLOCK);
				  if(buffer) bfree(buffer);
				  if(buffer2) bfree(buffer2);
				  buffer = buffer2 = NULL;
				  return(FALSE);
								}
							}
						}
						if(state == 5 && done == TRUE) continue;
						if(tptr==last||*tptr==CARRIAGE_RETURN) /* CR or last char? */
						{
							*tptr=NULL;
							if(strstr(buffer,"**EMSI_ACKA490")!=NULL)  // Success??
							{
								done=TRUE;
				if(PcbData.FidoLogLevel == 6) writeFidolog("EMSI ACK received! Going to state 5",BLOCK);
								stop=TRUE;
								state=5;
							}

							if(strstr(buffer,"**EMSI_REQA77E")!=NULL)
							{
								state=4;
				if(PcbData.FidoLogLevel >= 6) writeFidolog("In receive emsi. State 4 setting state to 4",BLOCK);
								done=TRUE;
								writeFidolog("EMSI REQ received.",BLOCK);
							}

							if(done!=TRUE)
							{
								if(strstr(buffer,"**EMSI")!=NULL)
								{
									state=2;
				  if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 4 setting state to 2",BLOCK);
									done=TRUE;
								}
							}
							tptr=buffer; // We don't need this string anymore, get a new one
						}
						else if(incoming!=10)
						tptr++;
					}
				} while(done!=TRUE && cdstillup());
				break;

		case 5:
		if(buffer) bfree(buffer);
		if(buffer2) bfree(buffer2);
		buffer = buffer2 = NULL;
		if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 5 returning TRUE",BLOCK);
				return TRUE;

		case 6:
		if(buffer) bfree(buffer);
		if(buffer2) bfree(buffer2);
		buffer = buffer2 = NULL;
		if(PcbData.FidoLogLevel == 6) writeFidolog("In receive emsi. State 6 returning FALSE",BLOCK);
				return FALSE;

	} // End Switch
	} while(stop!=TRUE);
  if(buffer) bfree(buffer);
  if(buffer2) bfree(buffer2);
  buffer = buffer2 = NULL;

  return TRUE;
}


/*****************************************************************************
* Function: bool getINTLinfo(const char*,char*,int,char*,int)				*
* Purpose : Locate INTL FIDO address information in a message body, (msgBuf) *
*		 and copy the destination address into the paramter dAddr as well *
*		 as the source address into sAddr.								 *
* Returns : Returns TRUE on success and FALSE on failure					 *
******************************************************************************/

//bool getINTLinfo(const char * msgBuf, char * dAddr, int dAddrSz, char * sAddr,
//					 int sAddrSz);
bool LIBENTRY getINTLinfo(const char * msgBuf, char * dAddr, int dAddrSz, char * sAddr,
				 int sAddrSz)
{
char * addrPtr		= NULL;
char * addrEndPtr = NULL;
char	 tmpdAddr[40];
char	 tmpsAddr[40];

	// Look fro the key word INTL in the message body.
	if( (addrPtr = strstr((char *) msgBuf,"\x01INTL")) == NULL) return FALSE;

	// Find the beginning of the address.
		while(!isdigit(*addrPtr) && isascii(*addrPtr) && *addrPtr != '\x0') addrPtr++;

	// end of message body
	if(*addrPtr == '\x0') return FALSE;
	// Set end pointer to beginning of address
	addrEndPtr = addrPtr;

	// Now walk end pointer to the end of the address
		while(isascii(*addrPtr) && isdigit(*addrEndPtr) || *addrEndPtr == ':' || *addrEndPtr == '/' && *addrEndPtr != '\x0') addrEndPtr++;

	// Copy first address into temp buffer
	if(*addrEndPtr != '\x0' && size_t(addrEndPtr - addrPtr) < sizeof(tmpdAddr))
		maxstrcpy(tmpdAddr,addrPtr,size_t( (addrEndPtr-addrPtr) +1));
	else
		return FALSE;

	// Make sure it's a valid address
	if(!strchr(tmpdAddr,':') || !strchr(tmpdAddr,'/')) return FALSE;

	// Move to the beginning of the next address
	addrPtr = addrEndPtr;
		while(isascii(*addrPtr) && !isdigit(*addrPtr) && *addrPtr != '\x0') addrPtr++,addrEndPtr++;

	// Now walk to the end of the second address
	if(*addrPtr != '\x0')
			while(isascii(*addrPtr) && isdigit(*addrEndPtr) || *addrEndPtr == ':' || *addrEndPtr == '/' && *addrEndPtr != '\x0') addrEndPtr++;
	else
		return FALSE;

	// Copy the address into a temp buffer
	if(*addrEndPtr != '\x0' && ((addrEndPtr - addrPtr)+1) < sizeof(tmpsAddr))
		maxstrcpy(tmpsAddr,addrPtr,size_t(addrEndPtr-addrPtr+1));
	else
		return FALSE;

	if(!strchr(tmpsAddr,':') || !strchr(tmpsAddr,'/'))
		return FALSE;
	else
	{
		maxstrcpy(dAddr,tmpdAddr,dAddrSz);
		maxstrcpy(sAddr,tmpsAddr,sAddrSz);
	}

	// Now grab the point information.
		if( (addrPtr = strstr((char *) msgBuf,"\x01""FMPT")) != NULL)
	{
		char pntBuf[20];
				while(!isdigit(*addrPtr) && *addrPtr != '\x0' && isascii(*addrPtr)) ++addrPtr;
		addrEndPtr = addrPtr;
				while(isdigit(*addrEndPtr) && *addrPtr != '\x0' && isascii(*addrPtr)) ++addrEndPtr;
				maxstrcpy(pntBuf,addrPtr,size_t(addrEndPtr - addrPtr)+1);
				if(strlen(pntBuf) + strlen(sAddr) <= sAddrSz)
				{
					strcat(sAddr,".");
					strcat(sAddr,pntBuf);
				}
	}
	if( (addrPtr = strstr((char *)msgBuf,"\x01TOPT")) != NULL)
	{
		char pntBuf[20];
				while(!isdigit(*addrPtr) && *addrPtr != '\x0' && isascii(*addrPtr)) ++addrPtr;
				addrEndPtr = addrPtr;
				while(isdigit(*addrEndPtr) && isascii(*addrPtr)) ++addrEndPtr;
				maxstrcpy(pntBuf,addrPtr,size_t(addrEndPtr - addrPtr)+1);
				if(strlen(pntBuf) + strlen(dAddr) <= dAddrSz)
				{
					strcat(dAddr,".");
					strcat(dAddr,pntBuf);
				}
	}

	return TRUE;
}

static void _NEAR_ LIBENTRY moveFREQs(void)
{
		char from[MAXFLEN];
		char to[MAXFLEN];
		ffblk fblk;
		int  done = 0;
	int  dosmaxfind=1;
	#ifdef __OS2__
	int  DirHandle;
	#endif


		sprintf(from,"%s\\%s",directory_info.incoming_packets,"*.REQ");

	dosmaxfind = 1;
	#ifdef __OS2__
	  DirHandle = SYSTEMDIRHANDLE;
	#endif
	done = dosfindfirst(from,&fblk,FA_NORMAL,&dosmaxfind PDIRHANDLE);

		while(!done)
		{
				sprintf(from,"%s\\%s",directory_info.incoming_packets,fblk.ff_name);
				sprintf(to,"%s\\%s",directory_info.bad_packets,fblk.ff_name);
		pcbmovefile(from,to);
				sprintf(logBuf,"Un-processed FREQ %s moved to bad directory.",fblk.ff_name);
				writeFidolog(logBuf,BLOCK);
		done = dosfindnext(&fblk,&dosmaxfind PDIRHANDLE2);
		}
}


/****************************************************************************/
/* Make a file request. 													*/

void LIBENTRY create_freq(void)
{
DOSFILE 		reqfile;
uint  zone,net,node,point;
char					filename[160],nodebuf[25],password[80],templine[80];
QUEUE_RECORD	queue_record;
//cFIDOQUE				queue;

char			mask_address[7]={6,0,'0','9',':','/','.'};

	assert(stackavail() > 2000);
	memset(filename 	 ,0,sizeof(filename));
	memset(nodebuf		,1,sizeof(nodebuf));
	memset(&queue_record,0,sizeof(QUEUE_RECORD));
	memset(password,0,sizeof(password));

  if(read_fido_config(FREQ_RECS) == FALSE) return;


	clsbox(22,5,57,19,0x11);
	box(22,5,57,19,0x17,DOUBLE);
	fastprint(27,10,"Node number to FREQ from",ScrnColors->v14Color);
	fastprint(27,11,"                        ",0x01);
	agotoxy(27,11);
	inputfieldstr(queue_record.nodestr,"_",0x07,25,UPCASE,0,mask_address);

	if(queue_record.nodestr[0]==0x20 || queue_record.nodestr[0]==NULL)
	{
		free_fido_memory();
		return;
	}

	default_address(queue_record.nodestr,sizeof(queue_record.nodestr));
	fastprint(27,11,queue_record.nodestr,0x07);

	fastprint(27,13,"File Name",ScrnColors->v14Color);
	fastprint(27,14,"                        ",0x01);
	agotoxy(27,14);
  inputfieldstr(filename,"_",0x0d,80,DEFAULTS,0,mask_alphanum);


	fastprint(27,16,"Password (Enter for none)",ScrnColors->v14Color);
	fastprint(27,17,"                        ",0x01);
	agotoxy(27,17);
	inputfieldstr(password,"_",0x0d,80,DEFAULTS,0,mask_alphanum);
	if(filename[0]==0x20 || filename[0]==NULL)
	{
		free_fido_memory();
		return;
	}

	if(password[0] != '\x0')
	{
		strcat(filename," ");
		strcat(filename,password);
	}

	queue_record.flag=Q_FILEREQ|Q_KILLSENT|Q_CRASH;

	maxstrcpy(nodebuf,queue_record.nodestr,sizeof(nodebuf));
  fido_nodestr_to_int(nodebuf,zone,net,node,point);

	directory_info.outgoing_packets[sizeof(directory_info.outgoing_packets)-1]=NULL;
	stripright(directory_info.outgoing_packets,' ');
	sprintf(queue_record.filename,"%s%04X%04X.REQ",directory_info.outgoing_packets,net,node);

	if(fileexist(queue_record.filename)!=255)
	{
		if(dosfopen(queue_record.filename,OPEN_RDWR|OPEN_DENYWRIT,&reqfile)==-1)
		{
			free_fido_memory();
			return;
		}

	memset(templine,0,sizeof(templine));
		// See if file already exists in REQ file.
	while( dosfgets(templine,sizeof(templine),&reqfile) != -1 )
		{
			if(strstr(templine,filename))
			{
				free_fido_memory();
		dosfclose(&reqfile);
				return;
			}
		}
		dosfputs(filename,&reqfile);
		dosfwrite("\r",1,&reqfile);
		dosfclose(&reqfile);
	}
	else
	{
		if(dosfopen(queue_record.filename,OPEN_RDWR|OPEN_CREATE|OPEN_DENYWRIT,&reqfile)==-1)
		{
			free_fido_memory();
			return;
		}

		dosfputs(filename,&reqfile);
		dosfwrite("\r",1,&reqfile);
		dosfclose(&reqfile);
	}

	// Limit Q's scope
	{
	cNEWQ Q;
		queue_record.readOnly = TRUE;
		Q.addEntry(queue_record);
	}
	free_fido_memory();
	return;
}


/****************************************************************************/
/* Poll a node for new mail.												*/

void LIBENTRY poll_node(void)
{
char node_to_poll[26];
int  i;
int  screen;
char			mask_address[7]={6,0,'0','9',':','/','.'};
//cFIDOQUE	queue;

	memset(node_to_poll,0,sizeof(node_to_poll));

  if(read_fido_config(ADDR_RECS) == FALSE) return;
	screen=memsavescreen();

	clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
	box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x17,DOUBLE);
	fastprint(27,10,"Enter Node Address to Poll",ScrnColors->v14Color);
	agotoxy(26,11);
	fastprint(26,11,"                            ",0x0C);
	agotoxy(26,11);
	inputfieldstr(node_to_poll,"_",ScrnColors->BoxColor,sizeof(node_to_poll)-1,UPCASE,0,mask_address);

	if(node_to_poll[0]==NULL || node_to_poll[0]==' ')
	return;

	default_address(node_to_poll,sizeof(node_to_poll));

	for(i=0;i<num_akas;i++)
	{
	char addrbuf[25];

		if(address[0].point == 0) sprintf(addrbuf,"%u:%u/%u",address[0].zone,address[0].net,address[0].node);
	else					  sprintf(addrbuf,"%u:%u/%u.%u",address[0].zone,address[0].net,address[0].node,address[0].point);

		if(strcmp(node_to_poll,addrbuf)==0)
		{
			fastprint(27,13,"You cannot poll yourself,",ScrnColors->v14Color);
			fastprint(27,14,"or an AKA of yourself.",ScrnColors->v14Color);
			fastprint(27,15,"Press any key to continue.",ScrnColors->v14Color);
			while(!kbdhit(NOBUFFER)) giveup();
			free_fido_memory();
			memrestorescreen(screen,FREESCREEN);
			return;
		}
	}

	// Limit Q's scope
	{
	cNEWQ Q;
		Q.addPoll(node_to_poll,TRUE);
	}
	free_fido_memory();
	memrestorescreen(screen,FREESCREEN);
}


/****************************************************************************/
/* Fill in the default Zone and Net if they don't exist.                    */

void LIBENTRY default_address(char *nodestr,int len)
{
char	temp[60];

	if(strchr(nodestr,':')==NULL)
	{
		if(strchr(nodestr,'/')!=NULL)
		{
			sprintf(temp,"%u:%s",PcbData.FidoDefaultZone,nodestr);
			maxstrcpy(nodestr,temp,len);
		}
		else
		{
			sprintf(temp,"%u:%u/%s",PcbData.FidoDefaultZone,PcbData.FidoDefaultNet,nodestr);
			maxstrcpy(nodestr,temp,len);
		}
	}
}

bool LIBENTRY OKtoImportExport(void)
{

	if(!okaytohandlemail())
	{
		clsbox(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,0x11);
		box(MNU_TOPX,MNU_TOPY,MNU_BOTX,MNU_BOTY,ScrnColors->BoxColor,DOUBLE);
		fastprint(24,7,"Another node is processing mail.",ScrnColors->v14Color);
		fastprint(24,10,"Please wait for the other ",ScrnColors->v14Color);
		fastprint(24,11,"node to finish or abort ",ScrnColors->v14Color);
		fastprint(24,12,"processing on that node.",ScrnColors->v14Color);
		settimer(10,SIXTY_SECONDS);
		while(kbdhit(NOBUFFER) == 0 && !timerexpired(10))
			giveup();
		Status.FileXfer = FALSE;
		return FALSE;
	}
	return TRUE;
}

static void _NEAR_ LIBENTRY updateLastDateOn(char * addr)
{
long recnum=0;

  if(!addr) return;

  recnum = find_fido_rec(addr,FALSE,FALSE,NULL);
  if(recnum <= 0) return;
  tempuseralloc(FALSE);
  gettemprecord(recnum);
  convertreadtodata(TempRead,TempData);
  TempData->LastDateOn = getjuliandate();
  timestr2(TempData->LastTimeOn);
  convertdatatoread(TempData,TempRead);
  puttemprecord(recnum,IGNOREDIRTY);
  tempuserdealloc();
}

static void _NEAR_ LIBENTRY processDLO(const char * addr)
{
char		 fname[100];
char		 line[100];
FUSERS		 user;
cDOSFILE	 dfile;
QUEUE_RECORD qrec;
cNEWQ		 Q;


  fido_nodestr_to_int((char*)addr,user.zone,user.net,user.node,user.point);
  if(user.point != 0)
	 sprintf(fname,"%-.50s%-04.04X%-04.04X.PNT\\%-08.08u.DLO",directory_info.outgoing_packets,user.net,user.node,user.point);
  else
	 sprintf(fname,"%-.50s%-04.04X%-04.04X"".DLO",directory_info.outgoing_packets,user.net,user.node);

  if(fileexist(fname) == 255) return;

  if(dfile.open(fname,OPEN_RDWR|OPEN_DENYNONE) != 0) return;

  while(dfile.getln(line,sizeof(line)) != -1)
  {
	memset(&qrec,0,sizeof(qrec));
	maxstrcpy(qrec.nodestr,(char*)addr,sizeof(qrec.nodestr));

	if(line[0] == '^') maxstrcpy(qrec.filename,line+1,sizeof(qrec.filename));
	else			   maxstrcpy(qrec.filename,line,sizeof(qrec.filename));

	qrec.flag = 0;
	qrec.flag = Q_NORMAL;

	strupr(qrec.filename);
	if(strstr(qrec.filename,".TIC") != 0) qrec.flag |= Q_KILLSENT;

	if(PcbData.FidoLogLevel >= 4)
	{
	  sprintf(logBuf,"DLO Process adding %s to outbound queue.",findstartofname(qrec.filename));
	  writeFidolog(logBuf,BLOCK);
	}
	Q.addEntry(qrec);
  }
  dfile.close();
  dfile.unlink();
}

static bool _NEAR_ LIBENTRY isRightAddress(QUEUE_RECORD & rec)
{
char * nullit = NULL;
char   tmpaddr[50];
char   tmpaka[50];

  if(!rakas) return TRUE;

  maxstrcpy(tmpaddr,rec.nodestr,sizeof(tmpaddr));

  // Strip any ".0" from the address
  nullit = strchr(tmpaddr,'.');
  if(nullit && atoi(nullit+1) == 0) *nullit = '\x0';

  for(uint i = 0;i<numakas;i++)
  {
	maxstrcpy(tmpaka,rakas[i].str,sizeof(tmpaka));

	// strip any @domain addressing from aka
	nullit = strchr(tmpaka,'@');
	if(nullit) *nullit = '\x0';

	// Strip any ".0" from the aka address
	nullit = strchr(tmpaka,'.');
	if(nullit && atoi(nullit+1) == 0) *nullit = '\x0';

	if(strcmp(tmpaddr,tmpaka) == 0) return TRUE;
  }

  if(PcbData.FidoLogLevel >= 6)
  {
	sprintf(logBuf,"File left in queue. Compare %s and %s.",tmpaddr,tmpaka);
	writeFidolog(logBuf,BLOCK);
  }
  return FALSE;
}










#endif
