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


/******************************************************************************/
/*																			  */
/*								  SCREXEC.CPP								  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*					Data and functions for the cSCRIPT class				  */
/*																			  */
/*============================================================================*/
/*																			  */
/*								   Written by								  */
/*							   Scott Dale Robison							  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*			  Copyright (C) 1994 - Clark Development Company, Inc.			  */
/*																			  */
/******************************************************************************/

#ifdef	___EXEC___

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#include	<dos.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	"lrand.hpp"
#include	"pcbmisc.hpp"
#include	"scrmisc.hpp"
#include	<assert.h>
#ifdef FIDO
#include	"structs.h"
#include	"defines.h"
#include	"fidoque.hpp"
#endif

#include	"bug.h"
#ifndef LIB
  #include	"pcbmsgs.hpp"
  #include	"msgstub.hpp"
#endif
#include	"exec.hpp"

#ifdef DEBUGSCR
  #include	<memcheck.h>
#endif

#ifdef __WATCOMC__
  #include	<direct.h>
  #include	<borland.h>
#else
  #include	<dir.h>
#endif
/******************************************************************************/

/******************************************************************************/

// Types

/******************************************************************************/

// Constants

char mask_money [] = { 7, '$', '+', '-', '.', 0, '0', '9' };
char mask_snum	[] = { 5, '+', '-', 0, '0', '9' };
char mask_date	[] = { 5, '-', '/', 0, '0', '9' };
char mask_time	[] = { 4, ':', 0, '0', '9' };


/******************************************************************************/

// Variables

extern char Scrn_BottomRow;
//extern char *errText[];

int curChan 	 = 0;
int pushFlag	 = FALSE;
#ifdef LIB

int 		DebugLevel;
int 		UsersFile = 0;
int 		NumFiles  = 0;

pcbconftype CurrConf;

char *		LastDefaultAnswer  = NULL;
char *		LastAcceptedAnswer = NULL;

#endif

/******************************************************************************/

// Function Prototypes

		 void LIBENTRY pplsetenvvar ( char * varval );
unsigned long LIBENTRY crc32buf 	( char *buf, unsigned len );
unsigned long LIBENTRY crc32file	( char *file );
		 int  LIBENTRY doCommand	( bool	 tryCmdLst,
									char * cmd,
									int    tokens,
									int  & tries,
									char * tokBuf,
									int    tokBufSize,
									char * unTokCmds );

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

void LIBENTRY pplforward(int x)
{
	if (UseAnsi)
		forward(x);
	else
		ttyforward(x);
}

	/*--------------------------------------------------------------------*/

#if defined(PCB_DEMO) || defined(__OS2__)

int LIBENTRY unsupported(char * token)
{
char logBuf[60];

	if(Status.CurSecLevel < PcbData.SysopSec[SEC_SYSOPLEVEL])
	{
	  sprintf(logBuf,"PPL encountered unsupported command %s",token);
	  writelog(logBuf,LEFTJUSTIFY);
	  return TRUE;
	}
	printcls();
	print("This PPE file utilizes the ");
	print(token);
	println(" which");
	#ifdef __OS2__
	  println("is not supported in the OS/2 version of PCBoard");
	#else
	  println("is not supported in the DEMO version of PCBoard");
	#endif
	newline();
	moreprompt(PRESSENTER);
	printcls();
	return TRUE;
}

#endif

	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::searchInit(char * Scriteria, bool caseSensitive)
{
	if (!searchAvailable) return;

	searchStop();

	strcpy(searchInput,Scriteria);

	searchTokens = tokenscan(searchInput,searchCodedInput,caseSensitive);

	searchInUse = (searchTokens > 0);
}

	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::searchFind(char * text, int len)
{
	searchFound = FALSE;
	if (searchInUse && (text != NULL) && (len > 0))
		searchFound = parsersearch(text,len,searchCodedInput,FALSE,0);
}

	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::searchPrint(char * str)
{
	if (searchInUse && searchFound)
		printfoundtext(str);
	else
		printxlated(str);
}

	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::searchStop(void)
{
	if (!searchInUse) return;

	stopsearch();

	searchInUse = searchFound = FALSE;
}

	/*--------------------------------------------------------------------*/

//void LIBENTRY displayPrompt(char * s, int & nt)
//{
//	  dispString(s,nt);
//	  strcpy(s,lastChar(s,'_') ? "_" : "");
//	  nt = 0;
//}


	/*--------------------------------------------------------------------*/

int  LIBENTRY cSCRIPT::stkinit(cVARVAL * o)
{
   cVARVAL * to;
   int err = FALSE;
   int cnt = 0;
   int startId = o->values.vPROCEDURE.firstId;

   checkstack();
   // Initialize stack to prepare it for a procedure/function call
   // First push local id's for dummy parameters.
   // then push all local variable id's
   // push old dummy parameter values onto stack
   for(int op = 0; op<o->values.vPROCEDURE.params;op++)
   {
	  to = new cVARVAL(*varLst[startId++]->data);

	  if(to)
	  {
		err = prgProcVarStack.push(to);
		if(err)
		  break;
		cnt++;
	  }
   }
	 // push old local var values onto stack
	 startId = o->values.vPROCEDURE.firstId+
			   o->values.vPROCEDURE.params;
	 for(int lp = 0 ; lp<o->values.vPROCEDURE.locvarcnt;lp++,startId++)
	 {
		if(varLst[startId]->info.flags.init == 0)
		{
		   if(varLst[startId]->info.dims > 0)
		   {
			 for(int i=0;i<(varLst[startId]->info.vs+1) *
						   (varLst[startId]->info.ms+1) *
						   (varLst[startId]->info.cs+1);i++)
			 {
			   to = new cVARVAL(varLst[startId]->data[i]);
			   if(to)
			   {
				 err = prgProcVarStack.push(to);
				 if(err)
				   break;
				 cnt++;
			   }
			 }
		   }
		   to = new cVARVAL(*varLst[startId]->data);
		   if(to)
			 err = prgProcVarStack.push(to);
			 if(err)
			   break;
			 cnt++;
		}
	 }

	 // if an error occured during stack init, clean up what *was*
	 // initialized.
	 if(err)
	 {
	   for(int i = 0;i<cnt;i++)
		  delete prgProcVarStack.pop();
	 }
	 return err;
}


int  LIBENTRY cSCRIPT::stkclean(cVARVAL * o)
{

   int op	   = 0;
   int startId = o->values.vPROCEDURE.firstId +
				 o->values.vPROCEDURE.params  +
				 o->values.vPROCEDURE.locvarcnt-1;
   cVARVAL * q	= NULL;
   cVARVAL * q1 = NULL;

   checkstack();
   // pop  old local var values off stack
   // re-assign all local vars.
   // pop local variables off of stack
   for(int pb = 0 ; pb<o->values.vPROCEDURE.locvarcnt;pb++,startId--)
   {
	 if(varLst[startId]->info.flags.init == 0 && !prgProcVarStack.isEmpty())
	 {
	   q = prgProcVarStack.pop();
	   assert(q != NULL);

	   if(varLst[startId]->info.dims > 0)
	   {
		 for(int i=((varLst[startId]->info.vs+1) *
				   (varLst[startId]->info.ms+1)  *
				   (varLst[startId]->info.cs+1))-1;i >= 0;i--)
		 {
		   if(prgProcVarStack.isEmpty())
		   {
			 q1 = prgProcVarStack.pop();
			 varLst[startId]->data[i] = *q1;
			 if(q1 && q1->type != vtFUNCTION && q1->type != vtPROCEDURE)
			 {
			   delete q1;
//				 q1 = NULL;
			 }
		   }
		 }
	   }

	   *varLst[startId]->data = *q;
	   //if(q && q1->type != vtFUNCTION && q1->type != vtPROCEDURE)
	   if(q && q->type != vtFUNCTION && q->type != vtPROCEDURE)
		 delete q;

	 }
   }

   // pop old params values off of stack
   startId = o->values.vPROCEDURE.firstId + o->values.vPROCEDURE.params-1;
   for(op = 0;op<o->values.vPROCEDURE.params;op++,startId--)
   {
	  if(!prgProcVarStack.isEmpty())
	  {
		q = prgProcVarStack.pop();
		assert(q != NULL);
		*varLst[startId]->data = *q;
		if(q && q->type != vtFUNCTION && q->type != vtPROCEDURE)
		 delete q;
	  }
   }
   return FALSE;
}


int  LIBENTRY cSCRIPT::initLocals(cVARVAL * o)
{

   int startId = o->values.vPROCEDURE.firstId+
				 o->values.vPROCEDURE.params;

   checkstack();
   for(int lp = 0 ; lp<o->values.vPROCEDURE.locvarcnt;lp++,startId++)
   {
	  if(varLst[startId]->info.flags.init == 0)
	  {
		 varLst[startId]->redimArray(varLst[startId]->info.dims,varLst[startId]->info.vs,
		 varLst[startId]->info.ms,varLst[startId]->info.cs);
		 varLst[startId]->info.flags.init = 0;
	  }
   }
   return FALSE;

}



	/*--------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::clearProc(cVARVAL * fpptr)
{
	cVARVAL * d = NULL;
	static unsigned int mask = 0;

	checkstack();
	// If this is a procedure then findout which parameters need to
	// be copied back into the global var.
	  int sidno = fpptr->values.vPROCEDURE.firstId+fpptr->values.vPROCEDURE.params-1;
	  if(fpptr->values.vPROCEDURE.params <= 16)
		mask = 1U << (fpptr->values.vPROCEDURE.params-1);
	  else
		mask = 0x8000U;
	  for(int i1 = 0;i1<fpptr->values.vPROCEDURE.params;i1++,sidno--)
	  {
		if((fpptr->values.vPROCEDURE.params - i1) <= 16)
		{
		 if((fpptr->values.vPROCEDURE.passFlags & mask) && !prgProcGlobVarStack.isEmpty())
		 {
			d = prgProcGlobVarStack.pop();
			assert(d != NULL);
		   *d = *varLst[sidno]->data;
		 }
		 mask >>= 1;
		}
	  }
	return stkclean(fpptr);
}

#ifndef LIB
	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::msgtofile(void) {

  long l1, l2;
  cVARVAL * r = NULL;
  #ifdef ERRBRK
	int err = 0;
  #endif

   //EVAL(tmpInteger,r,nullErrBreak);	   // Get conf num
   r  = evalArg();
   nullErrReturn(r);
   l1 = r->values.vINTEGER;

   //EVAL(tmpInteger,r,nullErrBreak);	   // get message number
   r  = evalArg();
   nullErrReturn(r);
   l2 = r->values.vINTEGER;

   //EVAL(tmpString,r,nullErrBreak);	   // Get file name
   r  = evalArg();
   nullErrReturn(r);
   strcpy(str1,nul2mty(*r));
   pcbconftype confrec;

   if(l1> PcbData.NumConf || l1 < 0)
   {
	 if(DebugLevel >= 1)
	 {
	   writelog("Conference does not exist (MSFTOFILE)",SPACERIGHT);
	   writelog(ltoa(l1,buf,10),SPACERIGHT);
	 }
	 return;
   }

   getconfrecord((unsigned)l1,&confrec);

   if(message.open(confrec.MsgFile) !=0) return;

   if(l2 == message.readIdx(l2))
   {
	   if(message.readHdr(message.idxRec.off) != 0)
	   {
		 if(DebugLevel >= 1)
		 {
		   writelog("Error reading message header. (MSGTOFILE)",SPACERIGHT);
		   writelog(ltoa(l2,buf,10),SPACERIGHT);
		 }
		 message.close();
		 return;
	   }
	   if(message.readBody(0) != 0)
	   {
		 if(DebugLevel >= 1)
		 {
		   writelog("Error reading message body. (MSGTOFILE)",SPACERIGHT);
		   writelog(ltoa(l2,buf,10),SPACERIGHT);
		 }
		 message.close();
		 return;
	   }
   }
   else
   {
	 if(DebugLevel >= 1)
	 {
	   writelog("Message number does not exist. (MSGTOFILE)",SPACERIGHT);
	   writelog(ltoa(l2,buf,10),SPACERIGHT);
	 }
	 message.close();
	 return;
   }

   DOSFILE f;
   char * buf = NULL;
   char   tmpBuf[80 + 1];
   char   hdrPlusBuf[80 + 1];
   char   tmpBuf2[10 + 1];
   int	  i=0,extHdrCnt=0;

   if(dosfopen(str1,OPEN_WRIT|OPEN_APPEND|OPEN_DENYRDWR,&f) == 0)
   {
	  dossetbuf(&f,16384);

	  sprintf(hdrPlusBuf,"          Status: %c\r\n",message.msgHdr.status);
	  dosfputs(hdrPlusBuf,&f);

	  ltoa((long)message.msgHdr.number,tmpBuf2,10);
	  sprintf(hdrPlusBuf,"  Message Number: %s\r\n",tmpBuf2);
	  dosfputs(hdrPlusBuf,&f);

	  ltoa((long)message.msgHdr.refer,tmpBuf2,10);
	  sprintf(hdrPlusBuf,"Reference Number: %s\r\n",tmpBuf2);
	  dosfputs(hdrPlusBuf,&f);

	  itoa((int)message.msgHdr.blocks,tmpBuf2,10);
	  sprintf(hdrPlusBuf,"Number of blocks: %s\r\n",tmpBuf2);
	  dosfputs(hdrPlusBuf,&f);

	  memcpy(tmpBuf,message.msgHdr.dmsg,sizeof(message.msgHdr.dmsg));
	  tmpBuf[sizeof(message.msgHdr.dmsg)] = '\x0';
	  sprintf(hdrPlusBuf,"            Date: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  memcpy(tmpBuf,message.msgHdr.tmsg,sizeof(message.msgHdr.tmsg));
	  tmpBuf[sizeof(message.msgHdr.tmsg)] = '\x0';
	  sprintf(hdrPlusBuf,"            Time: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  memcpy(tmpBuf,message.msgHdr.tname,sizeof(message.msgHdr.tname));
	  tmpBuf[sizeof(message.msgHdr.tname)] = '\x0';
	  sprintf(hdrPlusBuf,"              To: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  ltoa((long)message.msgHdr.dread,tmpBuf2,10);
	  sprintf(hdrPlusBuf,"           Reply: %s\r\n",tmpBuf2);

	  memcpy(tmpBuf,message.msgHdr.tread,sizeof(message.msgHdr.tread));
	  tmpBuf[sizeof(message.msgHdr.tread)] = '\x0';
	  sprintf(hdrPlusBuf,"   Time of reply: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  //tmpBuf2[0] = message.msgHdr.reply;
	  //tmpBuf2[1] = '\x0';

	  sprintf(hdrPlusBuf,"           Reply: %c\r\n",message.msgHdr.reply);
	  //memcpy(tmpBuf,message.msgHdr.reply,sizeof(message.msgHdr.reply));
	  //tmpBuf[sizeof(message.msgHdr.reply)] = '\x0';

	  memcpy(tmpBuf,message.msgHdr.fname,sizeof(message.msgHdr.fname));
	  tmpBuf[sizeof(message.msgHdr.fname)] = '\x0';
	  sprintf(hdrPlusBuf,"            From: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  memcpy(tmpBuf,message.msgHdr.subj,sizeof(message.msgHdr.subj));
	  tmpBuf[sizeof(message.msgHdr.fname)] = '\x0';
	  sprintf(hdrPlusBuf,"         Subject: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  memcpy(tmpBuf,message.msgHdr.pwd,sizeof(message.msgHdr.pwd));
	  tmpBuf[sizeof(message.msgHdr.pwd)] = '\x0';
	  sprintf(hdrPlusBuf,"        Password: %s\r\n",tmpBuf);
	  dosfputs(hdrPlusBuf,&f);

	  itoa((int)message.msgHdr.active,tmpBuf2,10);
	  sprintf(hdrPlusBuf,"          Active: %s\r\n",tmpBuf2);
	  dosfputs(hdrPlusBuf,&f);

	  sprintf(hdrPlusBuf,"            Echo:%c\r\n",message.msgHdr.echo);
	  dosfputs(hdrPlusBuf,&f);


	  sMSGEXTHDR * extHdr;
	  extHdrCnt = message.getExtHdrs();
	  if(extHdrCnt > 0)
	  {
		sprintf(hdrPlusBuf,"  Extended headers: %d\r\n",extHdrCnt);
		dosfputs(hdrPlusBuf+2,&f);

		for(i=0;i<extHdrCnt;i++)
		{
		   extHdr = message.getExtHdr(i);
		   memcpy(hdrPlusBuf,extHdr,sizeof(*extHdr));
		   hdrPlusBuf[sizeof(*extHdr)-1] = '\x0';
		   dosfputs(hdrPlusBuf+2,&f);
		   dosfputs("\r\n",&f);
		}
	  }
	  strcpy(hdrPlusBuf,"Message Body:\r\n");
	  dosfputs(hdrPlusBuf,&f);

	  while( (buf = message.getLine()) != NULL)    // Write message body
	  {
		  dosfputs(buf,&f);
		  dosfputs("\r\n",&f);
	  }
	  dosfclose(&f);
   }
   else
	 writelog("Error opening output file (MSGTOFILE)",SPACERIGHT);
   message.close();
}

#endif

	/*--------------------------------------------------------------------*/

#if defined(__OS2__) && defined(__BORLANDC__)
  // There is a bug in BC++ for OS/2 which causes the compiler to get hung up
  // trying to compile this function if Global Register Allocation is enabled.
  // To get around the bug, we disable GRA here and re-enable it at the bottom
  // of this function
  #pragma option -O-e
  #ifdef DBASE
	// if DBASE is enabled, then another bug in BC++ for OS/2 causes the
	// compiler to get hung up if the Optimize Locally option is enabled, so
	// we'll disable it here as well and re-enable it down below.
	#pragma option -O-c
  #endif
#endif

int LIBENTRY cSCRIPT::execute(char * af)
{
	// Declarations
	bool b1, b2;

	char c1;

	int err, exitFlag, aborted, i1, i2, i3;

	sint token;   // DWT:  changed int to sint

	unsigned u1, u2;

	char * p1, * p2, * p3, * p4;

	cVARVAL * r;
	cVARVAL * d;
	cVARVAL * fpptr = NULL;
	static unsigned int mask;

	void LIBENTRY (* func)(void _FAR_ * bs, int bn);

	long l1, l2;
#ifndef LIB
	double d1;
#endif


	char uniqueName[12+1];

	bool confBitsAccessed = FALSE;

	// Initializations
	err 	 = FALSE;
	exitFlag = FALSE;
	aborted  = FALSE;

	tVS   *  prgProcStack = NULL;

	prgProcStack = new tVS(DD_NO,FALSE);

	checkstack();
	if (prgProcStack == NULL)
	{
		scriptErr(0,-1,SCR_ERR_MEM,"EVALUATING EXPRESSION");
		prgProcStack = prgProcStkStk.pop();
		prgProcStkStk.push(prgProcStack);						  //lint !e534 Not using return value.
		if(Status.SysopFlag == ' ')
		{
		  Status.SysopFlag = (Status.SysopFlag == 'R' ? ' ' : 'R');
		  makepcboardsys();
		}
		return NULL;
	}

	prgProcStack->push(NULL);									   //lint !e534 Not using return value.
	prgProcStkStk.push(prgProcStack);							   //lint !e534 Not using return value.

	if(numCalls < 1)
	 scriptPointer = scriptBuffer;
	numCalls++;
	// Create and init temp file
	if (af != NULL)
	{
		sprintf(uniqueName,"SCR%05u.$$$",PcbData.NodeNum);
		openChan(0,uniqueName,OPEN_WRIT|OPEN_CREATE|OPEN_DENYRDWR);

		putChan(0,"*********************************"
				  "*****************************\r\n");

		char tmp[5+1];
		char Date[8+1];
		sprintf(str1,"From: %s, %s (%s) Node %u Sec %u Exp %s\r\n",
			ptrUData->Name, countrydate(datestr(Date)),
			timestr2(tmp), PcbData.NodeNum, Status.CurSecLevel,
			countrydate(juliantodate(ptrUData->RegExpDate)));
		putChan(0,str1);
	}

	//slrand(exacttime()); Moved to constructor

#if !defined(LIB) && defined(COMM)
	initcommwindow(HIDE);
#endif
	do {

		token = *((sint _FAR_ *) scriptPointer);   // DWT:	changed int to sint

		// Prevent entering the switch if breakflag is true.
		if(breakFlag)
		  break;

		switch (token)
		{

			case TOK_SOUNDDELAY:
			{
			sint freq,duration;

				 EVAL(tmpInteger,r,nullErrBreak);
				 freq = sint(tmpInteger.values.vINTEGER);

				 EVAL(tmpInteger,r,nullErrBreak);
				 duration = sint(tmpInteger.values.vINTEGER);

				 mysound(freq,duration);

				 break;
			}
			case TOK_ADJTUBYTES:

				 EVAL(tmpInteger,r,nullErrBreak);
				 ptrUData->TotUpldBytes   += tmpInteger.values.vDREAL;
				 break;

			case TOK_GRAFMODE:

				EVAL(tmpInteger,r,nullErrBreak);
				#ifndef LIB
				switch(tmpInteger.values.vINTEGER)
				{
				  case 1: if(UseAnsi)
						  {
							 Control.GraphicsMode = TRUE;
							 Control.RipMode = FALSE;
						  }
						  break;
				  case 2: UseAnsi = TRUE;
						  Control.GraphicsMode = TRUE;
						  Control.RipMode = FALSE;
						  break;

				  case 3: Control.GraphicsMode = FALSE;
						  Control.RipMode = FALSE;
						  break;

				  case 4: UseAnsi = FALSE;
						  Control.GraphicsMode = FALSE;
						  Control.RipMode = FALSE;
						  break;

				  case 5: if(HasRip) Control.RipMode = TRUE;
				}
				#endif
				 break;
			case TOK_ADDUSER:

				 long newusernum;

				 EVAL(tmpString,r,nullErrBreak);
				 EVAL(tmpBoolean,r,nullErrBreak);

				 // Free any previous GETALTUSER
				 if (altUserAvailable)
					if (TempRead != NULL)
						tempuserdealloc();

				 // v15.4:  force LMR allocation on ADDUSER, regardless of the
				 // last USELMRS setting.  ppladduser() -> adduserrecord() does
				 // fmemset(MsgPtr,0,MsgPtrLen) without a NULL check, so calling
				 // it with TempMsgPtr==NULL (as happens after USELMRS FALSE +
				 // GETALTUSER) crashes PCBoard.  A temp allocation of the LMR
				 // buffer is cheap and only lives for the duration of ADDUSER.
				 if(tempuseralloc(TRUE) == 0)
				 {
				   newusernum = ppladduser(tmpString.values.vSTRING);

				   // Dealloc temp user because getUserVars will re-allocate it
				   tempuserdealloc();
				   if(newusernum == -1 || !tmpBoolean.values.vBOOLEAN)
					   getUserVars(CUR_USER_GOT,err);
				   else
					   getUserVars(newusernum,err);
				 }
				 break;

			case TOK_KILLMSG:
			{
			  uint confnum;
			  long msgnum;

				 EVAL(tmpInteger,r,nullErrBreak);
				 confnum   = (uint)tmpInteger.values.vINTEGER;

				 EVAL(tmpInteger,r,nullErrBreak);
				 msgnum    = tmpInteger.values.vINTEGER;
				 killmessage(unsigned(confnum),msgnum);
				 break;
			}
			case TOK_CHDIR:

				 EVAL(tmpString,r,nullErrBreak);
				 chdir(tmpString.values.vSTRING);

				 break;

			case TOK_MKDIR:

				 EVAL(tmpString,r,nullErrBreak);
				 mkdir(tmpString.values.vSTRING);

				 break;
			case TOK_REDIR:

				 EVAL(tmpString,r,nullErrBreak);
				 rmdir(tmpString.values.vSTRING);

				 break;
#ifdef FIDO
/*
			case TOK_FDOWRAKA:
			case TOK_FDOADDAKA:
			{
			cAKAS	 akas;
			NADDRESS aka;
			long	 recno=0;

				 // Which record number are we going to modify
				 EVAL(tmpInteger,r,nullErrBreak);
				 recno = tmpInteger.values.vINTEGER;

				 // Get zone number
				 EVAL(tmpInteger,r,nullErrBreak);
				 aka.zone = tmpInteger.values.vINTEGER;

				 // Get net number
				 EVAL(tmpInteger,r,nullErrBreak);
				 aka.net = tmpInteger.values.vINTEGER;

				 // Get node number
				 EVAL(tmpInteger,r,nullErrBreak);
				 aka.node = tmpInteger.values.vINTEGER;

				 // Get point number
				 EVAL(tmpInteger,r,nullErrBreak);
				 aka.point = tmpInteger.values.vINTEGER;

				 // Is it a primary AKA
				 EVAL(tmpBoolean,r,nullErrBreak);
				 aka.primary = tmpInteger.values.vBOOLEAN;

				 // Does it go in seenby lines
				 EVAL(tmpBoolean,r,nullErrBreak);
				 aka.inseenby = tmpInteger.values.vBOOLEAN;

				 // Does it go in handshake
				 EVAL(tmpBoolean,r,nullErrBreak);
				 aka.present = tmpInteger.values.vBOOLEAN;

				 aka.UpOrDownLink = FALSE;

				 switch(token)
				 {
				   TOK_FDOWRAKA: akas.modifyRec(recno,aka);
								 break;

				   TOK_FDOADDAKA:akas.addRec(aka);
								 break;
				 }
				 break;
			}
			case TOK_FDOWRORG:

				 break;
			case TOK_FDOADDORG:

				 break;
*/
			// Fall though
			case TOK_FDOQMOD:
			case TOK_FDOQADD:
			{
			  cNEWQ Q;
			  QUEUE_RECORD rec;
			  long		   recno=0;

				 memset(&rec,0,sizeof(rec));
				 if(token == TOK_FDOQMOD)
				 {
					EVAL(tmpInteger,r,nullErrBreak);
					recno = tmpInteger.values.vINTEGER;
				 }
				 // get address to send to
				 EVAL(tmpString,r,nullErrBreak);
				 maxstrcpy(rec.nodestr,tmpString.values.vSTRING,sizeof(rec.nodestr));

				 // get file to send
				 EVAL(tmpString,r,nullErrBreak);
				 maxstrcpy(rec.filename,tmpString.values.vSTRING,sizeof(rec.filename));

				 //get flags
				 EVAL(tmpInteger,r,nullErrBreak);
				 if(tmpInteger.values.vINTEGER == 1)
				   rec.flag |= Q_NORMAL;
				 else if(tmpInteger.values.vINTEGER == 2)
				 {
				   rec.flag |= Q_CRASH | Q_OUTBOUND;
				   rec.readOnly = TRUE;
				 }

				 switch(token)
				 {
				   case TOK_FDOQADD: Q.addEntry(rec);
									 break;
				   case TOK_FDOQMOD: Q.modifyRec(rec,(unsigned int)recno);
									 break;
				 }
				 break;
			}
			case TOK_FDOQDEL:
			{
			  cNEWQ Q;
			  long		   recno=0;

				 EVAL(tmpInteger,r,nullErrBreak);
				 recno = tmpInteger.values.vINTEGER;
				 Q.removeEntry((int)recno);

				 break;
			}
#endif	// ifdef FIDO

			case TOK_CONFINFO:
			{
			  long confnum = 0;
			  long field   = 0;

				EVAL(tmpInteger,r,nullErrBreak);
				confnum = tmpInteger.values.vINTEGER;

				EVAL(tmpInteger,r,nullErrBreak);
				field = tmpInteger.values.vINTEGER;

				r = evalArg();
				nullErrBreak(r);

				putConfInfo((unsigned)confnum,(unsigned)field,r);
			}

				break;


#ifndef LIB
			case TOK_COMMAND:
				// Get the try cmd.lst flag
				EVAL(tmpBoolean,r,nullErrBreak);
				b1 = tmpBoolean.values.vBOOLEAN;

				// Get the command line
				EVAL(tmpString,r,nullErrBreak);
				strcpy(str1,nul2mty(tmpString));

				// Make a copy of it
				strcpy(str2,str1);

				u1 = tokenize(str2);
				if (u1 > 0)
				{
					int tries = 0;
					p1 = getnexttoken();
					doCommand(!b1,p1,u1,tries,str2,sizeof(str2),str1);
				}

				tokenCnt = 0;

				break;

			case TOK_QWKLIMITS:
			  EVAL(tmpInteger,r,nullErrBreak);
			  l1 = tmpInteger.values.vINTEGER;

			  EVAL(tmpInteger,r,nullErrBreak);
			  l2 = tmpInteger.values.vINTEGER;
			  if (!QwkSupport) break;

			  switch(l1)
			  {
				 case 0:
					ptrUData->QwkConfig.MaxMsgs = (uint)l2;
					if (l1 > PcbData.MaxTotalMsgs) ptrUData->QwkConfig.MaxMsgs = (uint) PcbData.MaxTotalMsgs;
					if (l1 < 0) ptrUData->QwkConfig.MaxMsgs = 0;
					break;
				 case 1:
					ptrUData->QwkConfig.MaxMsgsPerConf = (uint)l2;
					if (l1 > PcbData.MaxConfMsgs) ptrUData->QwkConfig.MaxMsgsPerConf = (uint) PcbData.MaxConfMsgs;
					if (l1 < 0) ptrUData->QwkConfig.MaxMsgsPerConf = 0;
					break;
				 case 2:
					ptrUData->QwkConfig.PersonalAttachLimit = l2;
					break;
				 case 3:
					ptrUData->QwkConfig.PublicAttachLimit = l2;
					break;
			  }

			  break;

			case TOK_MSGTOFILE:
			  msgtofile();
			  break;

			  case TOK_ACCOUNT:

				EVAL(tmpInteger,r,nullErrBreak);
				EVAL(tmpDReal,r,nullErrBreak);
				l1 = tmpInteger.values.vINTEGER;

				switch(l1)
				{
					case 0:
						ptrUData->Account.StartingBalance	   = tmpDReal.values.vDREAL
																+ ptrUData->Account.StartingBalance;
						break;
					case 1:
						ptrUData->Account.StartThisSession	   = tmpDReal.values.vDREAL
																+ptrUData->Account.StartThisSession;
						break;
					case 2:
						ptrUData->Account.DebitCall 		   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitCall;
						break;
					case 3:
						ptrUData->Account.DebitTime 		   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitTime;
						break;
					case 4:
						ptrUData->Account.DebitMsgRead		   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitMsgRead;
						break;
					case 5:
						ptrUData->Account.DebitMsgReadCapture  = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitMsgReadCapture;
						break;
					case 6:
						ptrUData->Account.DebitMsgWrite 	   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitMsgWrite;
						break;
					case 7:
						ptrUData->Account.DebitMsgWriteEchoed  = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitMsgWriteEchoed;
						break;
					case 8:
						ptrUData->Account.DebitMsgWritePrivate = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitMsgWritePrivate;
						break;
					case 9:
						ptrUData->Account.DebitDownloadFile    = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitDownloadFile;
						break;
					case 10:
						ptrUData->Account.DebitDownloadBytes   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitDownloadBytes;
						break;
					case 11:
						ptrUData->Account.DebitGroupChat	   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitGroupChat;
						break;
					case 12:
						ptrUData->Account.DebitTPU			   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitTPU;
						break;
					case 13:
						ptrUData->Account.DebitSpecial		   = tmpDReal.values.vDREAL
																+ptrUData->Account.DebitSpecial;
						break;
					case 14:
						ptrUData->Account.CreditUploadFile	   = tmpDReal.values.vDREAL
																+ptrUData->Account.CreditUploadFile;
						break;
					case 15:
						ptrUData->Account.CreditUploadBytes    = tmpDReal.values.vDREAL
																+ptrUData->Account.CreditUploadBytes;
						break;
					case 16:
						ptrUData->Account.CreditSpecial 	   = tmpDReal.values.vDREAL
																+ptrUData->Account.CreditSpecial;
						break;
					case 17:
						ptrUData->Account.DropSecLevel		   = (unsigned char) tmpDReal.values.vDREAL;

						break;
					default:
						break;

				}


				break;
			case TOK_RECORDUSAGE:
				EVAL(tmpInteger,r,falseErrBreak);
				l1 = tmpInteger.values.vINTEGER;

				EVAL(tmpString,r,falseErrBreak);
				strcpy(str1,nul2mty(tmpString));

				EVAL(tmpString,r,falseErrBreak);
				strcpy(str2,nul2mty(tmpString));

				EVAL(tmpDReal,r,falseErrBreak);
				d1 = tmpDReal.values.vDREAL;

				EVAL(tmpInteger,r,falseErrBreak);
				l2 = tmpInteger.values.vINTEGER;

				switch (l1)
				{
					case 2:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitCall);
					  break;
					case 3:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitTime);
					  break;
					case 4:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitMsgRead);
					  break;
					case 5:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitMsgReadCapture);
					  break;
					case 6:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitMsgWrite);
					  break;
					case 7:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitMsgWriteEchoed);
					  break;
					case 8:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitMsgWritePrivate);
					  break;
					case 9:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitDownloadFile);
					  break;
					case 10:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitDownloadBytes);
					  break;
					case 11:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitGroupChat);
					  break;
					case 12:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitTPU);
					  break;
					case 13:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.DebitSpecial);
					  break;
					case 14:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.CreditUploadFile);
					  break;
					case 15:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.CreditUploadBytes);
					  break;
					case 16:
					  recordusage(str1,str2,d1,l2,&ptrUData->Account.CreditSpecial);
					  break;
					default:
					  break;
				}
				break;
#endif // ifdeflib
			case TOK_EVAL:
				EVAL(tmpBoolean,r,nullErrBreak);
				break;

#ifdef DBASE
			case TOK_DADD:
			case TOK_DSETALIAS:
			case TOK_DAPPEND:
			case TOK_DBLANK:
			case TOK_DBOTTOM:
			case TOK_DCLOSE:
			case TOK_DCLOSEALL:
			case TOK_DCREATE:
			case TOK_DDELETE:
			case TOK_DFBLANK:
			case TOK_DFCOPY:
			case TOK_DGET:
			case TOK_DGO:
			case TOK_DLOCK: 		//			   TOK_DLOCKF same token
			case TOK_DLOCKG:
			case TOK_DLOCKR:
			case TOK_DNCLOSE:
			case TOK_DNCLOSEALL:
			case TOK_DNCREATE:
			case TOK_DNEW:
			case TOK_DNOPEN:
			case TOK_DOPEN:
			case TOK_DPACK:
			case TOK_DPUT:
			case TOK_DRECALL:
			case TOK_DSEEK:
			case TOK_DSKIP:
			case TOK_DTAG:
			case TOK_DTOP:
			case TOK_DUNLOCK:

				execdbs(token,err);
				break;
#endif	// ifdef DBASE

			case TOK_STACKABORT:	 // ... TOK_STACKABORT EXPR ...

				EVAL(tmpBoolean,r,nullErrBreak);
				stackErrContFlag = tmpBoolean.values.vBOOLEAN;
				break;

			case TOK_USELMRS:	  // ... TOK_USELMRS EXPR ...

				EVAL(tmpBoolean,r,nullErrBreak);
				altUserLMRFlag = tmpBoolean.values.vBOOLEAN;
				break;

			case TOK_SETENV:	 // ... TOK_SETENV EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				pplsetenvvar(nul2mty(tmpString));
				break;

			case TOK_SETLMR:	 // ... TOK_SETLMR EXPR EXPR ...

				EVAL(tmpInteger,r,nullErrBreak);
				u1 = (unsigned) tmpInteger.values.vINTEGER;
				if(u1 > PcbData.NumConf)
				  u1 = PcbData.NumConf;

				EVAL(tmpInteger,r,nullErrBreak);
				l1 = (long) tmpInteger.values.vINTEGER;
				if(l1 < 1)
				  l1 = 1;

				if(ptrULMR) ptrULMR[u1] = l1;

				break;

			/*----------------------------------------------------------
			 * 15.4 statement additions (see NEWSCR.HPP TOK_MOVEMSG,
			 * TOK_SETBANKVAL, TOK_SHORTDESCSTMT).  Sematics inferred
			 * from name + argcount + the recovered PCBOARDM.EXE 15.4b
			 * MOVEMSG error strings ("Conference does not exist
			 * (MOVEMSG)", "Message number does not exist (MOVEMSG)",
			 * "Unable to read message body (MOVEMSG)", "Unable to
			 * read message header (MOVEMSG)").
			 *---------------------------------------------------------*/

			case TOK_MOVEMSG:	// ... TOK_MOVEMSG EXPR EXPR EXPR ...
				/* MOVEMSG conf, message, movetype  (per WHATSNEW 15.4).
				 * arg1 = conference number (INTEGER)
				 * arg2 = message number (DREAL — msgnums are 32-bit)
				 * arg3 = movetype BOOLEAN (TRUE=move, FALSE=copy)
				 *
				 * Moves (or copies) the given message from the
				 * specified conference base to the end of the
				 * message base.  Errors printed at runtime match
				 * the strings recovered from PCBOARDM.EXE 15.4b:
				 *   "Conference does not exist (MOVEMSG)"
				 *   "Message number does not exist (MOVEMSG)"
				 *   "Unable to read message body (MOVEMSG)"
				 *   "Unable to read message header (MOVEMSG)"
				 *
				 * Delegates to pplmovemsg() in the message-base I/O
				 * layer.  For a PPL script the return code is
				 * silently swallowed (the caller can check via a
				 * subsequent read).
				 */
				EVAL(tmpInteger,r,nullErrBreak);
				u1 = (unsigned) tmpInteger.values.vINTEGER; /* conf     */
				EVAL(tmpInteger,r,nullErrBreak);
				/* msgnum is DREAL — pass as long, truncating any fractional */
				l1 = (long) tmpInteger.values.vDREAL;          /* msg num  */
				EVAL(tmpInteger,r,nullErrBreak);
				/* movetype: nonzero = move (delete source), 0 = copy */
				pplmovemsg(u1, l1, (int)tmpInteger.values.vINTEGER); /*lint !e534 */
				break;

			case TOK_SETBANKVAL:  // ... TOK_SETBANKVAL EXPR EXPR ...
				/* SETBANKVAL idx value — write Time Bank PSA field 0-11. */
				EVAL(tmpInteger,r,nullErrBreak);
				l1 = (long) tmpInteger.values.vINTEGER;   /* idx 0-11 */
				EVAL(tmpInteger,r,nullErrBreak);
				if (BankSupport && l1 >= 0 && l1 < 12) {
				    long *bp = (long *) &Bank;
				    bp[l1] = (long) tmpInteger.values.vDREAL;
				}
				break;

			case TOK_SHORTDESCSTMT:  // ... TOK_SHORTDESCSTMT EXPR ...
				/* arg1 = new value for the current user's
				 *        ShortDesc byte.  Set at runtime; will
				 *        be persisted next time the user's URead
				 *        gets written (log off or explicit save).
				 */
				EVAL(tmpInteger,r,nullErrBreak);
				if (ptrURead != NULL)
				    ptrURead->ShortDesc = (char) tmpInteger.values.vINTEGER;
				break;

			case TOK_END:		 // ... TOK_END ...

				breakFlag = TRUE;
				exitFlag = TRUE;
				break;

			case TOK_CLS:		 // ... TOK_CLS ...
				printcls();
				break;

			case TOK_CLREOL:	 // ... TOK_CLREOL ...
				cleareol();
				break;

			case TOK_MORE:		 // ... TOK_MORE ...
				moreprompt(MOREPROMPT);
				break;

			case TOK_WAIT:		 // ... TOK_WAIT ...
				moreprompt(PRESSENTER);
				break;

			case TOK_COLOR: 	 // ... TOK_COLOR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				printcolor(int(tmpInteger.values.vINTEGER & 0x000000FF));
				break;

			case TOK_GOSUB: 	 // ... TOK_GOSUB INT ...
				err = currProcStack.push(currProc);
				currProc = NULL;
				//err = (err || retAddr.push(scriptPointer+sizeof(sint)+sizeof(sint)+sizeof(sint)));
				err = (err || retAddr.push((scriptPointer-scriptBuffer)+sizeof(sint)+sizeof(sint)+sizeof(sint)));  // DWT: changed int to sint

			case TOK_GOTO:		 //lint !e616 ... TOK_GOTO INT ...					 Falls through

				scriptPointer = (scriptBuffer+(*((uint _FAR_ *) (scriptPointer+sizeof(sint)))))-sizeof(sint);  // DWT: changed all int's to sint's
				checkstatus();

				// If PCBoard fails to detect carrier loss
				// Then PPL needs to.
				if (Asy.LostCarrier && !Asy.IgnoreCDLoss)
					breakFlag = TRUE;
				break;

			case TOK_RETURN:	 // ... TOK_RETURN ...

				if (currProc && currProc->type == vtFUNCTION)			// These two if blocks
				{
				  numCalls--;
				  exitFlag = TRUE;
				  break;
				}

				if(!prgProcStack->isEmpty())
				  fpptr = prgProcStack->pop();

				if (currProc && currProc->type == vtPROCEDURE)
				{
				  if(fpptr != NULL)
					clearProc(fpptr);						//lint !e534

				}
				if (retAddr.isEmpty())
					exitFlag = TRUE;
				else
					//scriptPointer = retAddr.pop()-sizeof(sint)-sizeof(sint);	// DWT:  changed int to sint
					scriptPointer = (char _FAR_ *)((long)scriptBuffer+retAddr.pop())-sizeof(sint)-sizeof(sint);  // DWT:  changed int to sint
				currProc = currProcStack.pop();
				break;

			case TOK_LET:		 // ... TOK_LET VARID EXPR ...
				d = getVarSubPtr();
				nullErrBreak(d);

				EVAL(*d,r,nullErrBreak);

				break;

			case TOK_PRINT: 	 // ... TOK_PRINT EXPR [EXPR ...] ...
			case TOK_PRINTLN:	 // ... TOK_PRINTLN [EXPR ...] ...
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::printVal)) err = TRUE;
				if (token == TOK_PRINTLN && !breakFlag) newline();
				break;

			case TOK_PRFOUND:	 // ... TOK_PRFOUND EXPR [EXPR ...] ...
			case TOK_PRFOUNDLN:  // ... TOK_PRFOUNDLN [EXPR ...] ...
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::prFoundVal)) err = TRUE;
				if (token == TOK_PRFOUNDLN) newline();
				break;

			case TOK_IF:		 // ... TOK_IF EXPR OFF TOK [...] ...
				EVAL(tmpBoolean,r,nullErrBreak);

				if (tmpBoolean.values.vBOOLEAN)
					scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				else
					scriptPointer =
						scriptBuffer + *((uint _FAR_ *) scriptPointer + 1) -   // DWT: changed int to sint
							sizeof(sint);	  // DWT:  changed int to sint

				break;

			case TOK_CONFFLAG:	 // ... TOK_CONFFLAG EXPR EXPR ...
			case TOK_CONFUNFLAG: // ... TOK_CONFUNFLAG EXPR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				l1 = tmpInteger.values.vINTEGER & 0x0000FFFF;

				EVAL(tmpInteger,r,nullErrBreak);
				l2 = tmpInteger.values.vINTEGER & 0x0000003F;

				if( (l1 >= 0) && (l1 <= PcbData.NumConf) )
				{
					func = ((token == TOK_CONFFLAG) ? setbit : unsetbit);

					if (l2 & 0x01)	// Registered
						func(&ptrUReg[CNF_REG], int(l1));

					if (l2 & 0x02)	// Expired
						func(&ptrUReg[CNF_EXP], int(l1));

					if (l2 & 0x04)	// Selected
						func(&ptrUReg[CNF_USR], int(l1));

					if (l2 & 0x08)	// Conference SysOp
						func(&ptrUReg[CNF_CON], int(l1));

					if (l2 & 0x10)	// Mail Waiting
					{
						func(&ptrUReg[CNF_MFL], int(l1));

						if (token == TOK_CONFFLAG)
							ptrUData->PackedFlags.HasMail = TRUE;
					}
					if (l2 & 0x20)	// NET Status
						func(&ptrUReg[CNF_NET], int(l1));

					confBitsAccessed = TRUE;
				}

				break;

			case TOK_DISPFILE:	 // ... TOK_DISPFILE EXPR ...
				EVAL(tmpString,r,nullErrBreak);

				EVAL(tmpInteger,r,nullErrBreak);
				l1 = (tmpInteger.values.vINTEGER & 0x00000007);

				i1 = NOALTERNATE;
				if (l1 & 0x01) i1 |= GRAPHICS;
				if (l1 & 0x02) i1 |= SECURITY;
				if (l1 & 0x04) i1 |= LANGUAGE;

				displayfile(nul2mty(tmpString),(displayfiletype)i1);

				break;

			case TOK_INPUT: 	 // ... TOK_INPUT EXPR VARID ...
				EVAL(tmpString,r,nullErrBreak);

				d = getVarSubPtr();
				nullErrBreak(d);

				strcpy(str1,nul2mty(*d));
				str1[60] = '\0';

//				  displayPrompt(nul2mty(tmpString),tokenCnt);

				inputfieldstr(str1,nul2mty(tmpString),PCB_YELLOW,60,
					FIELDLEN|GUIDE|HIGHASCII,NOHELP,mask_alphanum);

				*d = str1;

				break;

			case TOK_FCLOSEALL:  // ... TOK_FCLOSEALL ...
			{
				if (readLineStream.handle != 0) dosfclose(&readLineStream);
				for (int i = 0; i < MAX_FILE_CHAN; ++i)
					closeChan(i);

				break;
			}

			case TOK_FCREATE:	 // ... TOK_FCREATE EXPR EXPR EXPR EXPR ...
			case TOK_FOPEN: 	 // ... TOK_FOPEN EXPR EXPR EXPR EXPR ...
			case TOK_FAPPEND:	 // ... TOK_FAPPEND EXPR EXPR EXPR EXPR ...
			case TOK_FCLOSE:	 // ... TOK_FCLOSE EXPR ...
			case TOK_FREWIND:	 // ... TOK_FREWIND EXPR ...
			case TOK_FGET:		 // ... TOK_FGET EXPR VARID ...
			case TOK_FPUT:		 // ... TOK_FPUT EXPR EXPR [EXPR ...] ...
			case TOK_FPUTLN:	 // ... TOK_FPUTLN EXPR [EXPR ...] ...
			case TOK_FPUTPAD:	 // ... TOK_FPUTPAD EXPR EXPR EXPR ...
			case TOK_FSEEK: 	 // ... TOK_FSEEK EXPR EXPR ...
			case TOK_FFLUSH:	 // ... TOK_FFLUSH EXPR ...
			case TOK_FREAD: 	 // ... TOK_FREAD EXPR VARID EXPR ...
			case TOK_FWRITE:	 // ... TOK_FWRITE EXPR EXPR EXPR ...
			case TOK_FDEFIN:	 // ... TOK_FDEFIN EXPR ...
			case TOK_FDEFOUT:	 // ... TOK_FDEFOUT EXPR ...
			case TOK_FDGET: 	 // ... TOK_FDGET VARID ...
			case TOK_FDPUT: 	 // ... TOK_FDPUT EXPR [EXPR ...] ...
			case TOK_FDPUTLN:	 // ... TOK_FDPUTLN [EXPR ...] ...
			case TOK_FDPUTPAD:	 // ... TOK_FDPUTPAD EXPR EXPR ...
			case TOK_FDREAD:	 // ... TOK_FDREAD VARID EXPR ...
			case TOK_FDWRITE:	 // ... TOK_FDWRITE EXPR EXPR ...
				execFile(token,err);
				break;


			case TOK_RESETDISP:  // ... TOK_RESETDISP ...
				checkdisplaystatus();
				break;

			case TOK_STARTDISP:  // ... TOK_STARTDISP EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);

				switch (tmpInteger.values.vINTEGER & 0x00000003)
				{
					case 1:  // FORCENONSTOP
						i1 = FORCENONSTOP;
						break;

					case 2:  // FORCECOUNTLINES
						i1 = FORCECOUNTLINES;
						break;

					case 0:  // NOCHANGE
					default: // NOCHANGE
						i1 = NOCHANGE;
						break;
				}

				startdisplay(startdisplaytype(i1));

				break;

			case TOK_GETALTUSER: // ... TOK_GETALTUSER EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				getUserVars(tmpInteger.values.vINTEGER,err);
				break;

			case TOK_FREALTUSER: // ... TOK_FREALTUSER ...
				if (altUserAvailable)
					if (TempRead != NULL)
						tempuserdealloc();
				// deliberately fall through to TOK_GETUSER

			case TOK_GETUSER:	 // ... TOK_GETUSER ...
				getUserVars(CUR_USER_GOT,err);
				break;

			case TOK_PUTUSER:	 // ... TOK_PUTUSER ...
				putUserVars(err);
				break;

			case TOK_DEFCOLOR:	 // ... TOK_DEFCOLOR ...
				printdefcolor();
				break;

			case TOK_DELUSER:	 // ... TOK_DELUSER ...
				ptrUData->DeleteFlag = TRUE;
				break;

			case TOK_ADJTIME:	 // ... TOK_ADJTIME EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				l1 = tmpInteger.values.vINTEGER;

				if (l1 < -32767)
					l1 = -32767;
				else if (l1 > 32767)
					l1 = 32767;

				if (l1 < 0)
					subtime(l1*-60L,CREDTIME);
				else
					addtime(l1*+60L,CREDTIME);

				break;

			case TOK_ADJBYTES:	 // ... TOK_ADJBYTES EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				ptrUData->TotDnldBytes	 += tmpInteger.values.vDREAL;
				ptrUData->DailyDnldBytes += tmpInteger.values.vINTEGER;
#ifndef LIB
				if (((lastUserGot == CUR_USER_GOT) ||
					(lastUserGot == NO_USER_GOT)) &&
					(Status.BytesRemaining > -1))
				{
					Status.BytesRemaining -= tmpInteger.values.vINTEGER;
					if (Status.BytesRemaining < 0) Status.BytesRemaining = 0;
				}
#endif
				break;

			case TOK_ADJDBYTES:  // ... TOK_ADJDBYTES EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				ptrUData->DailyDnldBytes += tmpInteger.values.vINTEGER;
#ifndef LIB
				if (((lastUserGot == CUR_USER_GOT) ||
					(lastUserGot == NO_USER_GOT)) &&
					(Status.BytesRemaining > -1))
				{
					Status.BytesRemaining -= tmpInteger.values.vINTEGER;
					if (Status.BytesRemaining < 0) Status.BytesRemaining = 0;
				}
#endif
				break;

			case TOK_ADJTBYTES:  // ... TOK_ADJTBYTES EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				ptrUData->TotDnldBytes	 += tmpInteger.values.vDREAL;
				break;

			case TOK_ADJTFILES:  // ... TOK_ADJTFILES EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				ptrUData->NumDownloads	 += unsigned(tmpInteger.values.vINTEGER);
				break;

			case TOK_LOG:		 // ... TOK_LOG EXPR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				EVAL(tmpBoolean,r,nullErrBreak);

				xlatetext(str1,nul2mty(tmpString));

				writelog(str1,tmpBoolean.values.vBOOLEAN ?
					LEFTJUSTIFY : SPACERIGHT);

				break;

			case TOK_INPUTSTR:	 // ... TOK_INPUTSTR EXPR VARID EXPR EXPR EXPR EXPR ...
			case TOK_PROMPTSTR:  // ... TOK_PROMPTSTR EXPR VARID EXPR EXPR EXPR ...
				r = evalArg();
				nullErrBreak(r);
				if (token == TOK_INPUTSTR)
				{
					tmpString = *r;
					strcpy(str2,nul2mty(tmpString));
				}
				else
				{
					tmpInteger = *r;
					i1 = int(tmpInteger.values.vINTEGER);
				}

				d = getVarSubPtr();

				if (token == TOK_INPUTSTR)
				{
					EVAL(tmpInteger,r,nullErrBreak);
					i2 = int(tmpInteger.values.vINTEGER & 0x000000FF);
				}

				EVAL(tmpInteger,r,nullErrBreak);
				i3 = int(tmpInteger.values.vINTEGER & 0x000000FF);
				if (i3 < 1)
					i3 = 1;
				else if (i3 > MAX_STR_LEN)
					i3 = MAX_STR_LEN;

				EVAL(tmpString,r,nullErrBreak);
				strcpy(str3+1,nul2mty(tmpString));
				*str3 = (char) strlen(str3+1);

				EVAL(tmpInteger,r,nullErrBreak);
				l1 = long(tmpInteger.values.vINTEGER & 0x0001FFFFL);

				u1 = 0;

				if (l1 & 0x00000001L) u1 |= ECHODOTS;
				if (l1 & 0x00000002L) u1 |= FIELDLEN;
				if (l1 & 0x00000004L) u1 |= GUIDE;
				if (l1 & 0x00000008L) u1 |= UPCASE;
				if (l1 & 0x00000010L) u1 |= STACKED;
				if (l1 & 0x00000020L) u1 |= ERASELINE;
				if (l1 & 0x00000040L) u1 |= NEWLINE;
				if (l1 & 0x00000080L) u1 |= LFBEFORE;
				if (l1 & 0x00000100L) u1 |= LFAFTER;
				if (l1 & 0x00000200L) u1 |= WORDWRAP;
				if (l1 & 0x00000400L) u1 |= NOCLEAR;
// not used 	if (l1 & 0x00000800L) u1 |= BELL;
				if (l1 & 0x00001000L) u1 |= HIGHASCII;
				if (l1 & 0x00002000L) u1 |= AUTO;
				if (l1 & 0x00004000L) u1 |= YESNO;
// not used 	if (l1 & 0x00008000L) u1 |= LOGIT;
// not used 	if (l1 & 0x00010000L) u1 |= LOGITLEFT;

				tmpString = *d;
				strcpy(str1,nul2mty(tmpString));

				if (token == TOK_INPUTSTR)
				{
//					  displayPrompt(str2,tokenCnt);
					inputfieldstr(str1,str2,i2,i3,									   //lint !e644
						(displaytype)u1,NOHELP,str3);								   //lint !e644
				}
				else
					inputfield(str1,i1,i3,								 //lint !e644
						(displaytype)u1,NOHELP,str3);					 //lint !e644

				*d = str1;

				break;

			case TOK_INPUTTEXT:  // ... TOK_INPUTTEXT EXPR VARID EXPR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				strcpy(str2,nul2mty(tmpString));

				d = getVarSubPtr();

				EVAL(tmpInteger,r,nullErrBreak);
				i2 = int(tmpInteger.values.vINTEGER & 0x000000FF);

				EVAL(tmpInteger,r,nullErrBreak);
				i3 = int(tmpInteger.values.vINTEGER & 0x000000FF);
				if (i3 < 1)
					i3 = 1;
				else if (i3 > MAX_STR_LEN)
					i3 = MAX_STR_LEN;

				tmpString = *d;
				strcpy(str1,nul2mty(tmpString));
				str1[i3] = '\0';

//				  displayPrompt(str2,tokenCnt);

				inputfieldstr(str1,str2,i2,i3,
					FIELDLEN|GUIDE|HIGHASCII,NOHELP,mask_alphanum);

				*d = str1;

				break;

			case TOK_INPUTYN:	 // ... TOK_INPUTYN EXPR VARID EXPR ...
			case TOK_INPUTMONEY: // ... TOK_INPUTMONEY EXPR VARID EXPR ...
			case TOK_INPUTINT:	 // ... TOK_INPUTINT EXPR VARID EXPR ...
			case TOK_INPUTCC:	 // ... TOK_INPUTCC EXPR VARID EXPR ...
			case TOK_INPUTDATE:  // ... TOK_INPUTDATE EXPR VARID EXPR ...
			case TOK_INPUTTIME:  // ... TOK_INPUTTIME EXPR VARID EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				strcpy(str2,nul2mty(tmpString));

				d = getVarSubPtr();

				EVAL(tmpInteger,r,nullErrBreak);
				i2 = int(tmpInteger.values.vINTEGER & 0x000000FF);

				tmpString = *d;
				strcpy(str1,nul2mty(tmpString));

				u1 = FIELDLEN|GUIDE|UPCASE;

				switch (token)
				{
					case TOK_INPUTYN:
//#ifndef LIB
						u1 |= YESNO;
//#endif
						i3 = 1;
						p1 = mask_yesno;
						break;

					case TOK_INPUTMONEY:
						i3 = 13;
						p1 = mask_money;
						break;

					case TOK_INPUTINT:
						i3 = 11;
						p1 = mask_snum;
						break;

					case TOK_INPUTCC:
						i3 = 16;
						p1 = mask_num;
						break;

					case TOK_INPUTDATE:
						i3 = 8;
						p1 = mask_date;
						break;

					case TOK_INPUTTIME:
						i3 = 8;
						p1 = mask_time;
						break;
					default:
						p1 = mask_num;
				}

				str1[i3] = '\0';                                                //lint !e644

//				  displayPrompt(str2,tokenCnt);

				inputfieldstr(str1,str2,i2,i3,displaytype(u1),NOHELP,p1);

				*d = str1;

				break;

			case TOK_DELAY: 	 // ... TOK_DELAY EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);

				l1 = tmpInteger.values.vINTEGER;
				//if (l1 > 32767) l1 = 32767;
				if (l1 > 0) tickdelay(int(standardizeTicks(l1)));

				break;

			case TOK_DTRON: 	 // ... TOK_DTRON ...
#ifdef COMM
				turnondtr();
#endif
				break;

			case TOK_DTROFF:	 // ... TOK_DTROFF ...
#ifdef COMM
				turnoffdtr();
#endif
				break;

			case TOK_CDCHKON:	 // ... TOK_CDCHKON ...
#ifndef LIB
#ifdef COMM
				if (Asy.Online == REMOTE)
				{
					settimer(4,ONESECOND);
					while (!timerexpired(4) && !online()) giveup();
					Asy.LostCarrier = !online();
					Asy.IgnoreCDLoss = FALSE;
				}
#endif
#endif
				break;

			case TOK_CDCHKOFF:	 // ... TOK_CDCHKOFF ...
#ifdef COMM
				if (Asy.Online == REMOTE) Asy.IgnoreCDLoss = TRUE;
#endif
				break;

			case TOK_KBDCHKON:	 // ... TOK_KBDCHKON ...
				turnkbdtimeron();
				break;

			case TOK_KBDCHKOFF:  // ... TOK_KBDCHKOFF ...
				Control.WatchKbdClock = FALSE;
				break;

			case TOK_SENDMODEM:  // ... TOK_SENDMODEM EXPR ...
				EVAL(tmpString,r,nullErrBreak);
#ifdef COMM
				slowsendtomodem(nul2mty(tmpString));
#endif
				break;

			case TOK_INC:		 // ... TOK_INC VARID ...
				d = getVarSubPtr();
				nullErrBreak(d);
				++(*d);
				break;

			case TOK_DEC:		 // ... TOK_DEC VARID ...
				d = getVarSubPtr();
				nullErrBreak(d);
				--(*d);
				break;

			case TOK_NEWLINE:	 // ... TOK_NEWLINE ...
				newline();
				break;

			case TOK_NEWLINES:	 // ... TOK_NEWLINES EXPR ...	***
				EVAL(tmpInteger,r,nullErrBreak);

				l1 = tmpInteger.values.vINTEGER;
				if (l1 < 1) l1 = 1;
				for (; l1 > 0; --l1) newline();

				break;

			case TOK_TOKENIZE:	 // ... TOK_TOKENIZE EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				strcpy(tokenBuf,nul2mty(tmpString));
				tokenCnt = tokenizestr(tokenBuf);
				break;

			case TOK_GETTOKEN:	 // ... TOK_GETTOKEN VARID ...
				d = getVarSubPtr();
				nullErrBreak(d);
				if (tokenCnt > 0)
				{
					--tokenCnt;
					*d = getnexttoken();
				}
				else
					*d = "";
				break;

			case TOK_SHELL: 	 // ... TOK_SHELL EXPR VARID EXPR EXPR ...
#ifdef PCB_DEMO
				err = unsupported("SHELL statement");
#else
				EVAL(tmpBoolean,r,nullErrBreak);

				d = getVarSubPtr();
				nullErrBreak(d);

				EVAL(tmpString,r,nullErrBreak);
				strcpy(str1,nul2mty(tmpString));

				EVAL(tmpString,r,nullErrBreak);
				strcpy(str2,nul2mty(tmpString));

				cls();

#ifdef COMM
				closemodem(FALSE);
#endif

				#ifndef __OS2__  // may need to finish this
				  nosound(); // Ensure speaker off at SHELL
				#endif

				//*d = performshell(str1,str2,
				//	  tmpBoolean.values.vBOOLEAN ? SHELLVIACOMMAND :
				//	  SHELLDIRECT);

				*d = performshell(str1,str2,
					tmpBoolean.values.vBOOLEAN ? SHELLVIACOMMAND :
					SHELLDIRECT,
					PcbData.PriorityShells,
					PcbData.MinimizeShells & 1,
					PcbData.MinimizeShells & 2,
					-1);

#ifdef COMM
				openmodem(HIDE);
				if (Asy.Online == REMOTE)
				{
					online();
					if (cdstillup() == 0) Asy.LostCarrier = TRUE;
				}
#endif

				cls();
				redisplaystatusline();
#endif
				break;

			case TOK_DISPTEXT:	 // ... TOK_DISPTEXT EXPR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				i1 = int(tmpInteger.values.vINTEGER & 0x00007FFF);

				EVAL(tmpInteger,r,nullErrBreak);
				l1 = long(tmpInteger.values.vINTEGER & 0x0001FFFFL);

				u1 = 0;

// not used 	if (l1 & 0x00000001L) u1 |= ECHODOTS;
// not used 	if (l1 & 0x00000002L) u1 |= FIELDLEN;
// not used 	if (l1 & 0x00000004L) u1 |= GUIDE;
// not used 	if (l1 & 0x00000008L) u1 |= UPCASE;
// not used 	if (l1 & 0x00000010L) u1 |= STACKED;
// not used 	if (l1 & 0x00000020L) u1 |= ERASELINE;
				if (l1 & 0x00000040L) u1 |= NEWLINE;
				if (l1 & 0x00000080L) u1 |= LFBEFORE;
				if (l1 & 0x00000100L) u1 |= LFAFTER;
// not used 	if (l1 & 0x00000200L) u1 |= WORDWRAP;
// not used 	if (l1 & 0x00000400L) u1 |= NOCLEAR;
				if (l1 & 0x00000800L) u1 |= BELL;
// not used 	if (l1 & 0x00001000L) u1 |= HIGHASCII;
// not used 	if (l1 & 0x00002000L) u1 |= AUTO;
// not used 	if (l1 & 0x00004000L) u1 |= YESNO;
				if (l1 & 0x00008000L) u1 |= LOGIT;
				if (l1 & 0x00010000L) u1 |= LOGITLEFT;

				displaypcbtext(i1,displaytype(u1));

				break;

			case TOK_STOP:		 // ... TOK_STOP ...
				aborted = exitFlag = TRUE;
				break;

			case TOK_BEEP:		 // ... TOK_BEEP ...
				bell();
				break;

			case TOK_PUSH:		 // ... TOK_PUSH EXPR [EXPR ...] ...
#ifdef PCB_DEMO
				err = unsupported("PUSH statement");
#else
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::pushVars)) err = TRUE;
#endif
				break;

			case TOK_POP:		 // ... TOK_POP VARID [VARID ...] ...
#ifdef PCB_DEMO
				err = unsupported("POP statement");
#else
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::popVars)) err = TRUE;
#endif
				break;

			case TOK_KBDSTUFF:	 // ... TOK_KBDSTUFF EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				stuffpplbuffer(nul2mty(tmpString));
				break;

			case TOK_KBDSTRING:  // ... TOK_KBDSTRING EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				stuffbufferstr(nul2mty(tmpString));
				break;

			case TOK_KBDFILE:	 // ... TOK_KBDFILE EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				stuffkbdwithfile(nul2mty(tmpString));
				break;

			case TOK_MDMFLUSH:	 // ... TOK_MDMFLUSH ...
#ifdef COMM
				clearinbuf();
#endif
				break;

			case TOK_KEYFLUSH:	 // ... TOK_KEYFLUSH ...
#ifdef COMM
				clearinbuf();
#endif
				// fall through

			case TOK_KBDFLUSH:	 // ... TOK_KBDFLUSH ...
				clearkbdbuffer();
				clearpplbuffer();
				closeautostuff();
				clearBiosKbd();
				break;

			case TOK_CALL:		 // ... TOK_CALL EXPR ...
#ifdef PCB_DEMO
				err = unsupported("CALL statement");
				break;
#else
			{
				EVAL(tmpString,r,nullErrBreak);

				strcpy(str1,strupr(nul2mty(tmpString)));
				substenvinfile(str1,sizeof(str1));
				swapOut();
				tmpScript = new cSCRIPT(tokenCnt);
				if ((tmpScript != NULL) && !tmpScript->load(str1))
				{
					#ifndef __OS2__  // may need to finish this
					nosound(); // Ensure speaker off at PPL execution
					#endif
					tmpScript->execute(NULL);										   //lint !e534
					tokenCnt = 0;
				}

				if (tmpScript != NULL)
				{
					delete tmpScript;
					tmpScript = NULL;
				}

				if(swapIn())
				  err = TRUE;
				break;
			}
#endif

			case TOK_ALIAS: 	 // ... TOK_ALIAS EXPR ...
				EVAL(tmpBoolean,r,nullErrBreak);
				Status.UseAlias = (tmpBoolean.values.vBOOLEAN ? TRUE : FALSE);
				getdisplaynames(); // Necessary to enable alias usage
				break;

			case TOK_LASTIN:	 // ... TOK_LASTIN EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				ptrUData->LastConference =
					uint(tmpInteger.values.vINTEGER);
				Status.Conference = ptrUData->LastConference;
				getconfrecord(Status.Conference,&CurrConf);
				break;

			case TOK_LANG:		 // ... TOK_LANG EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				setlanguage(int(tmpInteger.values.vINTEGER));				  //lint !e534
				break;

			case TOK_FLAG:		 // ... TOK_FLAG EXPR ...
			{
				EVAL(tmpString,r,nullErrBreak);

				// str1 is the complete path and file name
				// str2 is the complete path minus file name
				// str3 is the file name minus complete path

				// Copy the evaluated string into path name
				strupr(strcpy(str1,nul2mty(tmpString)));

				// Copy path name to path
				strcpy(str2,str1);

				// Go to the first character past last backslash
				p1 = strrchr(str2,'\\');
				p1 = (p1 ? ++p1 : str2);

				// Copy file name to file
				strcpy(str3,p1);

				// Terminate the path
				*p1 = '\0';

				ffblk curFFB;
				int   dosfindmax = 1;
				#ifdef __OS2__
				int DirHandle = MAKEDIRHANDLE;
				#endif
				int done = dosfindfirst(str1,&curFFB,0,&dosfindmax PDIRHANDLE);

				if (!done) allocatefilelist(DOWNLOAD);						   //lint !e534

				int tmpNumFiles=NumFiles;
				while (!done)
				{
					l1 = curFFB.ff_fsize;
					addfiletolist(curFFB.ff_name,str2,l1,TRUE,FALSE);					//lint !e534
					dosfindmax = 1;
					done = dosfindnext(&curFFB,&dosfindmax PDIRHANDLE2);
				}

				#ifdef __OS2__
				  dosclosedirhandle(DirHandle);
				#endif

				checkdlfiles(tmpNumFiles);

				break;
			}

			case TOK_DOWNLOAD:	 // ... TOK_DOWNLOAD ...
				EVAL(tmpString,r,nullErrBreak);
				tokenCnt = 0;
				strcpy(tokenBuf,nul2mty(tmpString));
				send(tokenize(tokenBuf));
				break;

			case TOK_JOIN:		 // ... TOK_JOIN EXPR ...
				if (confBitsAccessed)
				{
					updateuserinformation();
					confBitsAccessed = FALSE;
				}
				EVAL(tmpString,r,nullErrBreak);
				tokenCnt = 0;
				strcpy(tokenBuf,nul2mty(tmpString));
				joinconfcommand(tokenize(tokenBuf));
				break;

			case TOK_QUEST: 	 // ... TOK_QUEST EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				doquestionnaire(int(tmpInteger.values.vINTEGER));			   //lint !e534
				break;

			case TOK_BLT:		 // ... TOK_BLT EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				showblt(int(tmpInteger.values.vINTEGER));					   //lint !e534
				break;

			case TOK_DIR:		 // ... TOK_DIR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				tokenCnt = 0;
				strcpy(tokenBuf,nul2mty(tmpString));
				dirmenu(tokenize(tokenBuf));
				break;

			#ifndef LIB
			case TOK_HANGUP:	 // ... TOK_HANGUP ...
				loguseroff(ALOGOFF);
				exitFlag = TRUE;
				break;

			case TOK_BYE:		 // ... TOK_BYE ...
				loguseroff(NLOGOFF);
				break;
			#endif

			case TOK_GOODBYE:	 // ... TOK_GOODBYE ...
				byecommand();
				break;

			case TOK_BROADCAST:  // ... TOK_BROADCAST EXPR EXPR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				i1 = int(tmpInteger.values.vINTEGER);

				EVAL(tmpInteger,r,nullErrBreak);
				i2 = int(tmpInteger.values.vINTEGER);

				EVAL(tmpString,r,nullErrBreak);

				if ((i1 == 0) || (i2 == 0))
					sprintf(str1,"ALL %s",nul2mty(tmpString));
				else if (i1 == i2)
					sprintf(str1,"%d %s",i1,nul2mty(tmpString));
				else
					sprintf(str1,"%d-%d %s",i1,i2,nul2mty(tmpString));

				tokenCnt = 0;
				broadcast(tokenize(str1));

				break;

			case TOK_WAITFOR:	 // ... TOK_WAITFOR EXPR VARID EXPR ...
				EVAL(tmpString,r,nullErrBreak);

				d = getVarSubPtr();
				nullErrBreak(d);

				tmpBoolean.values.vBOOLEAN = FALSE;
				*d = tmpBoolean;

				EVAL(tmpInteger,r,nullErrBreak);
#ifndef LIB
#ifdef COMM
				p1 = nul2mty(tmpString);
				u1 = strlen(p1);
				if ((u1 > 0) && (Asy.Online != LOCAL))
				{
					*str1 = '\0';

					settimer(4,tmpInteger.values.vINTEGER*ONESECOND);
					while (!timerexpired(4))
					{
						i2 = comminkey();
						if (i2 != -1)
						{
							addchar(str1,(char)i2);
							if (strlen(str1) > u1) strcpy(str1,str1+1);
							if (stricmp(str1,p1) == 0)
							{
								tmpBoolean.values.vBOOLEAN = TRUE;
								*d = tmpBoolean;
								break;
							}
						}
						else
							giveup();
					}
				}
#endif
#endif

				break;

			case TOK_OPTEXT:	 // ... TOK_OPTEXT EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				p1 = nul2mty(tmpString);
				if (strlen(p1) > 79) p1[79] = '\0';
				strcpy(Status.DisplayText,p1);
				break;

			case TOK_DISPSTR:	 // ... TOK_DISPSTR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				dispString(nul2mty(tmpString),tokenCnt);
				tokenCnt = 0;
				break;

			case TOK_RDUNET:	 // ... TOK_RDUNET EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				readusernetrecord(unsigned(tmpInteger.values.vINTEGER),
					&unInfo);
				break;

			case TOK_WRUNET:	 // ... TOK_WRUNET EXPR EXPR EXPR EXPR EXPR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);

				EVAL(tmpString,r,nullErrBreak);
				unInfo.Status = *nul2mty(tmpString);

				EVAL(tmpString,r,nullErrBreak);
				p1 = nul2mty(tmpString);
				if (strlen(p1) > 25) p1[25] = '\0';
				strcpy(unInfo.Name,p1);

				EVAL(tmpString,r,nullErrBreak);
				p1 = nul2mty(tmpString);
				if (strlen(p1) > 24) p1[24] = '\0';
				strcpy(unInfo.City,p1);

				EVAL(tmpString,r,nullErrBreak);
				p1 = nul2mty(tmpString);
				if (strlen(p1) > 48) p1[48] = '\0';
				strcpy(unInfo.Operation,p1);

				EVAL(tmpString,r,nullErrBreak);
				p1 = nul2mty(tmpString);
				if (strlen(p1) > 79) p1[79] = '\0';
				strcpy(unInfo.Message,p1);

				updateusernetrecord(unsigned(tmpInteger.values.vINTEGER),
					&unInfo);

				break;

			case TOK_DOINTR:	 // ... TOK_DOINTR EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR ...
#ifdef __OS2__
				err = unsupported("DOINTR statement");
#else
				EVAL(tmpInteger,r,nullErrBreak);
				i1 = int(tmpInteger.values.vINTEGER & 0x000000FF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.ax = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.bx = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.cx = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.dx = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.si = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.di = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrREGS.w.cflag = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrSEGS.ds = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpInteger,r,nullErrBreak);
				intrSEGS.es = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				int86x(i1,&intrREGS,&intrREGS,&intrSEGS);					   //lint !e534
#endif
				break;

			case TOK_VARADDR:	 // ... TOK_VARADDR VAR VAR ...
#if defined(PCB_DEMO) || defined(__OS2__)
				err = unsupported("VARADDR statement");
#else
				d = getVarSubPtr();
				nullErrBreak(d);

				tmpInteger = (tINTEGER) d->varAddr();

				d = getVarSubPtr();
				nullErrBreak(d);

				*d = tmpInteger;
#endif
				break;

			case TOK_VARSEG:	 // ... TOK_VARSEG VAR VAR ...
#if defined(PCB_DEMO) || defined(__OS2__)
				err = unsupported("VARSEG statement");
#else
				d = getVarSubPtr();
				nullErrBreak(d);

				tmpInteger = FP_SEG(d->varAddr());

				d = getVarSubPtr();
				nullErrBreak(d);

				*d = tmpInteger;
#endif
				break;

			case TOK_VAROFF:	 // ... TOK_VAROFF VAR VAR ...
#if defined(PCB_DEMO) || defined(__OS2__)
				err = unsupported("VAROFF statement");
#else
				d = getVarSubPtr();
				nullErrBreak(d);

				tmpInteger = FP_OFF(d->varAddr());

				d = getVarSubPtr();
				nullErrBreak(d);

				*d = tmpInteger;
#endif
				break;

			case TOK_POKEB: 	 // ... TOK_POKEB EXPR EXPR ...
#if defined(PCB_DEMO) || defined(__OS2__)
				err = unsupported("POKE or POKEB statement");
				break;
#else
			{
				EVAL(tmpInteger,r,nullErrBreak);

				tBYTE _FAR_ * p = (tBYTE _FAR_ *) (tmpInteger.values.vINTEGER);

				EVAL(tmpInteger,r,nullErrBreak);

				*p = tBYTE(tmpInteger.values.vINTEGER & 0x000000FF);

				break;
			}
#endif

			case TOK_POKEW: 	 // ... TOK_POKEW EXPR EXPR ...
#if defined(PCB_DEMO) || defined(__OS2__)
				err = unsupported("POKEW statement");
				break;
#else
			{
				EVAL(tmpInteger,r,nullErrBreak);

				tWORD _FAR_ * p = (tWORD _FAR_ *) (tmpInteger.values.vINTEGER);

				EVAL(tmpInteger,r,nullErrBreak);

				*p = tWORD(tmpInteger.values.vINTEGER & 0x0000FFFF);

				break;
			}
#endif

			case TOK_POKEDW:	 // ... TOK_POKEDW EXPR EXPR ...
#if defined(PCB_DEMO) || defined(__OS2__)
				err = unsupported("POKEDW statement");
				break;
#else
			{
				EVAL(tmpInteger,r,nullErrBreak);

				tINTEGER _FAR_ * p =
					(tINTEGER _FAR_ *) (tmpInteger.values.vINTEGER);

				EVAL(tmpInteger,r,nullErrBreak);

				*p = tmpInteger.values.vINTEGER;

				break;
			}
#endif

			case TOK_ANSIPOS:	 // ... TOK_ANSIPOS EXPR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				i1 = int(tmpInteger.values.vINTEGER);
				if (i1 < 1) i1 = 1;

				EVAL(tmpInteger,r,nullErrBreak);
				i2 = int(tmpInteger.values.vINTEGER);
				if (i2 < 1) i2 = 1;

				if (UseAnsi) movecursor(--i1,--i2);

				break;

			case TOK_BACKUP:	 // ... TOK_BACKUP EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				i1 = int(tmpInteger.values.vINTEGER);
				if (i1 < 0) i1 = 0;
				if (i1 > awherex()) i1 = awherex();
				backup(i1);
				break;

			case TOK_FORWARD:	 // ... TOK_FORWARD EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				i1 = int(tmpInteger.values.vINTEGER);
				if (i1 < 0) i1 = 0;
				if (i1 > 79-awherex()) i1 = 79-awherex();
				pplforward(i1);
				break;

			case TOK_FRESHLINE:  // ... TOK_FRESHLINE ...
				freshline();
				break;

			case TOK_RDUSYS:	 // ... TOK_RDUSYS ...
#if    defined(PCB_DEMO)
				err = unsupported("RDUSYS statement");
#elif !defined(LIB)
				subtime(Status.CreditMinutes*60L,CREDTIME);
				if (fexist("USERS.SYS"))
				{
					Status.BackFromDoor = TRUE;
					getuserrecord(TRUE,FALSE);
					Status.BackFromDoor = FALSE;
					unlink("USERS.SYS");
				}
				if (fexist("PCBOARD.SYS")) readpcboardsys();
#endif
				break;

			case TOK_WRUSYSDOOR: // ... TOK_WRUSYSDOOR EXPR ...
#if    defined(PCB_DEMO)
				err = unsupported("WRUSYSDOOR statement");
#elif !defined(LIB)
				EVAL(tmpString,r,nullErrBreak);
				{ cVARVAL _s1(vtSTRING,"b "); tmpString = tmpString.strip(_s1); }
				{ cVARVAL _s2(vtINTEGER,8); tmpString = tmpString.left(_s2); }
				{ cVARVAL _s3(vtSTRING," "); tmpString = tmpString.trim(_s3); }
				strcpy(Status.DoorName,nul2mty(tmpString));
#endif

			case TOK_WRUSYS:	 //lint !e616  ... TOK_WRUSYS ...		Falls Through
#if    defined(PCB_DEMO)
				err = unsupported("WRUSYS statement");
#elif !defined(LIB)
				makepcboardsys();
				if (Status.UserRecNo != 0)
				{
					convertdatatoread(&UsersData,&UsersRead);
					putuserrecord(CHECKDIRTY);
				}
				makeusersys();
				*Status.DoorName = '\0';
#endif
				break;

			case TOK_NEWPWD:	 // ... TOK_NEWPWD EXPR VAR ...
				EVAL(tmpString,r,nullErrBreak);

				d = getVarSubPtr();
				nullErrBreak(d);

                tmpBoolean = (int) !setnewpassword(ptrUData,
					strupr(nul2mty(tmpString)));
				*d = tmpBoolean;

				d = aVarValPtr(VID_U_PWDEXP);
				nullErrBreak(d);
				*d = ptrUData->PwrdHistory.ExpireDate;

				d = aVarValPtr(VID_U_PWD);
				nullErrBreak(d);
				*d = ptrUData->Password;	// Get the current password

				break;

			case TOK_OPENCAP:	 // ... TOK_OPENCAP EXPR VAR ...
				EVAL(tmpString,r,nullErrBreak);

				d = getVarSubPtr();
				nullErrBreak(d);

#ifndef LIB
				if (Status.Capture) closecapturefile();
#endif

				tmpBoolean =
#ifndef LIB
					opencapturefile(nul2mty(tmpString));
#else
					(signed long int) 0;
#endif
				*d = tmpBoolean;

				redisplaystatusline();

				break;

			case TOK_CLOSECAP:	 // ... TOK_CLOSECAP ...
#ifndef LIB
				closecapturefile();
#endif
				redisplaystatusline();
				break;

			case TOK_MESSAGE:	 // ... TOK_MESSAGE EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				u1 = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

				EVAL(tmpString,r,nullErrBreak);
				p1 = nul2mty(tmpString);
				if (*p1 == '\0') p1 = NULL;
				if (p1 != NULL) p1 = strdup(p1);				   // USed to be bstrdup

				EVAL(tmpString,r,nullErrBreak);
				p2 = nul2mty(tmpString);
				if (*p2 == '\0') p2 = NULL;
				if (p2 != NULL) p2 = strdup(p2);				   // USed to be BSTRDUP

				EVAL(tmpString,r,nullErrBreak);
				p3 = nul2mty(tmpString);
				if (p3 != NULL) p3 = strdup(p3);				   // USed to be bstrdup

				EVAL(tmpString,r,nullErrBreak);
				c1 = *nul2mty(tmpString);

				EVAL(tmpDate,r,nullErrBreak);
				u2 = unsigned(tmpDate.values.vDATE);

				EVAL(tmpBoolean,r,nullErrBreak);
				b1 = tmpBoolean.values.vBOOLEAN;

				EVAL(tmpBoolean,r,nullErrBreak);
				b2 = tmpBoolean.values.vBOOLEAN;

				EVAL(tmpString,r,nullErrBreak);
				p4 = nul2mty(tmpString);
				if (p4 != NULL) p4 = strdup(p4);				   // Used to be bstrdup

				if (TempRead == NULL)
					entermessagefromfile(u1,p1,p2,p3,c1,
						u2,b1,b2,p4);

				if (p1 != NULL) free(p1);						// USed to be BFREE
				if (p2 != NULL) free(p2);						// Used to be BFREE
				if (p3 != NULL) free(p3);						// Used to be BFREE
				if (p4 != NULL) free(p4);						// Used to be BFREE

				break;

			case TOK_SAVESCRN:	 // ... TOK_SAVESCRN ...
				savescrnbuffer();
				break;

			case TOK_RESTSCRN:	 // ... TOK_RESTSCRN ...
				restscrnbuffer();
				break;

			case TOK_SOUND: 	 // ... TOK_SOUND EXPR ...

				EVAL(tmpInteger,r,nullErrBreak);
				u1 = unsigned(tmpInteger.values.vINTEGER & 0x0000FFFF);

#if defined(__OS2__)
				//err = unsupported("SOUND statement");
				if(u1 != 0) mysound(u1,QUARTERSECOND);
#else

				if (u1 == 0)
					nosound();
				else
					sound(u1);
#endif
				break;

			case TOK_CHAT:		 // ... TOK_CHAT ...
#ifdef COMM
				sysopchat(
#ifndef LIB
				1 // didn't start from O command
#endif
				);
#endif
				break;

			case TOK_SPRINT:	 // ... TOK_SPRINT EXPR [EXPR ...] ...
			case TOK_SPRINTLN:	 // ... TOK_SPRINTLN [EXPR ...] ...
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::sprintVal)) err = TRUE;
				if (token == TOK_SPRINTLN) ansi("\r\n");
				break;

			case TOK_MPRINT:	 // ... TOK_MPRINT EXPR [EXPR ...] ...
			case TOK_MPRINTLN:	 // ... TOK_MPRINTLN [EXPR ...] ...
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::mprintVal)) err = TRUE;
#ifndef LIB
#ifdef COMM
				if (token == TOK_MPRINTLN) printcom("\r\n");
#endif
#endif
				break;

			case TOK_DELETE:	 // ... TOK_DELETE EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				unlink(nul2mty(tmpString));
				break;

			case TOK_RENAME:	 // ... TOK_RENAME EXPR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				strcpy(str1,nul2mty(tmpString));
				EVAL(tmpString,r,nullErrBreak);
				rename(str1,nul2mty(tmpString));							   //lint !e534
				break;

			case TOK_APPEND:	 // ... TOK_APPEND EXPR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				strcpy(str1,nul2mty(tmpString));
				EVAL(tmpString,r,nullErrBreak);
				appendfile(str1,nul2mty(tmpString),FALSE);					   //lint !e534
				break;

			case TOK_COPY:		 // ... TOK_COPY EXPR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				strcpy(str1,nul2mty(tmpString));
				EVAL(tmpString,r,nullErrBreak);
				pcbcopyfile(str1,nul2mty(tmpString),FALSE,FALSE,NULL,TRUE);    //lint !e534
				break;

			case TOK_DBGLEVEL:	 // ... TOK_DBGLEVEL EXPR ...
				EVAL(tmpInteger,r,nullErrBreak);
				DebugLevel = int(tmpInteger.values.vINTEGER & 0x00007FFF);
				break;

			case TOK_SHOWON:	 // ... TOK_SHOWON ...

				// tmp* variables initialized at PPE code load
				Display.ShowOnScreen = TRUE;
				Control.RipMode 	 = tmpRipMode;
				Control.GraphicsMode = tmpGraphMode;
				UseAnsi 			 = tmpUseAnsi;

				break;

			case TOK_SHOWOFF:	 // ... TOK_SHOWOFF ...

				// Need to clear all these for captures
				Display.ShowOnScreen = FALSE;
				Control.RipMode 	 = FALSE;
				Control.GraphicsMode = FALSE;
				UseAnsi 			 = FALSE;

				break;

			case TOK_PAGEON:	 // ... TOK_PAGEON ...
#ifndef LIB
				Status.Paged = TRUE;
				++ptrUData->Stats.NumSysopPages;
				redisplaystatusline();
#endif
				break;

			case TOK_PAGEOFF:	 // ... TOK_PAGEOFF ...
#ifndef LIB
				Status.Paged = FALSE;
				redisplaystatusline();
#endif
				break;

			case TOK_REDIM: 	 // ... TOK_DIM COUNT VPTR EXPR EXPR EXPR ...
				scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
				if (doForItems(cSCRIPT::redimArray)) err = TRUE;
				break;

			case TOK_SORT:		 // ... TOK_SORT VPTR VPTR ...
			{
				cVAR * a = getVarPtr();
				cVAR * i = getVarPtr();
				a->sort(i);
				break;
			}

			case TOK_MOUSEREG:	 // ... TOK_MOUSEREG EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR EXPR ...
			{
				int  num, x1, y1, x2, y2, fontX, fontY;
				bool invert, clear;

				// num parameter is not used but must be processed
				// for ppl version to version compatibility
				EVAL(tmpInteger,r,nullErrBreak);
				num    = int(tmpInteger.values.vINTEGER);
				inRange(num,0,1295);

				EVAL(tmpInteger,r,nullErrBreak);
				x1	   = int(tmpInteger.values.vINTEGER);
				inRange(x1,1,80);

				EVAL(tmpInteger,r,nullErrBreak);
				y1	   = int(tmpInteger.values.vINTEGER);
				inRange(y1,1,50);

				EVAL(tmpInteger,r,nullErrBreak);
				x2	   = int(tmpInteger.values.vINTEGER);
				inRange(x2,1,80);

				EVAL(tmpInteger,r,nullErrBreak);
				y2	   = int(tmpInteger.values.vINTEGER);
				inRange(y2,1,50);

				EVAL(tmpInteger,r,nullErrBreak);
				fontX  = int(tmpInteger.values.vINTEGER);
				inRange(fontX,1,16);

				EVAL(tmpInteger,r,nullErrBreak);
				fontY  = int(tmpInteger.values.vINTEGER);
				inRange(fontY,1,14);

				EVAL(tmpBoolean,r,nullErrBreak);
				invert = (tmpBoolean.values.vBOOLEAN ? TRUE : FALSE);

				EVAL(tmpBoolean,r,nullErrBreak);
				clear  = (tmpBoolean.values.vBOOLEAN ? TRUE : FALSE);

				EVAL(tmpString,r,nullErrBreak);
				strcpy(str1,nul2mty(tmpString));

				// remember not to use num parameter per TeleGrafix
				setripmouseregion(0,x1,y1,x2,y2,fontX,fontY,
					invert,clear,str1);

				break;
			}

			case TOK_SCRFILE:	 // ... TOK_SCRFILE VAR VAR ...
				d = getVarSubPtr();
				nullErrBreak(d);
				tmpInteger = *d;

				*d = checkscreenforfilenames(int(tmpInteger.values.vINTEGER),str1);

				d = getVarSubPtr();
				nullErrBreak(d);
				*d = str1;

				break;

			case TOK_SEARCHINIT: // ... TOK_SEARCHINIT EXPR EXPR ...
				EVAL(tmpString,r,nullErrBreak);
				EVAL(tmpBoolean,r,nullErrBreak);
				searchInit(nul2mty(tmpString),tmpBoolean.values.vBOOLEAN);
				break;

			case TOK_SEARCHFIND: // ... TOK_SEARCHFIND EXPR VAR ...
				EVAL(tmpBigStr,r,nullErrBreak);

				searchFind(tmpBigStr.values.vBIGSTR.ptr,
					tmpBigStr.values.vBIGSTR.len);

				d = getVarSubPtr();
				nullErrBreak(d);

                tmpBoolean = (int) searchFound;
				*d = tmpBoolean;

				break;

			case TOK_SEARCHSTOP: // ... TOK_SEARCHSTOP ...
				searchStop();
				break;

			case TOK_TPAGET:	 // ... TOK_TPAGET EXPR VAR ...
			case TOK_TPAPUT:	 // ... TOK_TPAPUT EXPR EXPR ...
			case TOK_TPACGET:	 // ... TOK_TPACGET EXPR VAR EXPR ...
			case TOK_TPACPUT:	 // ... TOK_TPACPUT EXPR EXPR EXPR ...
			case TOK_TPAREAD:	 // ... TOK_TPAREAD EXPR VAR ...
			case TOK_TPAWRITE:	 // ... TOK_TPAWRITE EXPR EXPR ...
			case TOK_TPACREAD:	 // ... TOK_TPACREAD EXPR VAR EXPR ...
			case TOK_TPACWRITE:  // ... TOK_TPACWRITE EXPR EXPR EXPR ...
				execTPA(token,err);
				break;

			case TOK_BITSET:	 // ... TOK_BITSET VAR EXPR ...
				d = getVarSubPtr();
				nullErrBreak(d);

				EVAL(tmpInteger,r,nullErrBreak);

				*d = d->bitSet(tmpInteger);
				break;

			case TOK_BITCLEAR:	 // ... TOK_BITCLEAR VAR EXPR ...
				d = getVarSubPtr();
				nullErrBreak(d);

				EVAL(tmpInteger,r,nullErrBreak);

				*d = d->bitClear(tmpInteger);
				break;

			case TOK_BRAG:		 // ... TOK_BRAG ...  NO LONGER SUPPORTED! NOP

				break;



			case TOK_FEND:
				numCalls--;
				exitFlag = TRUE;
				break;

			case TOK_PCALL: 	// ... TOK_PCALL ...

				 {
				 err = currProcStack.push(currProc);
				 // get proc id
				 fpptr = getVarSubPtr();
				 currProc = fpptr;

				//If actual parameter is passed by var then push actual
				// parameter onto stack.
				mask = 1;
				stkinit(fpptr);
				if(err)
				  break;
				int sidno = fpptr->values.vPROCEDURE.firstId;
				for(i1=0;i1<fpptr->values.vPROCEDURE.params;i1++)
				{

				  // If it's a VAR parameter then read in the variable id
				  // else evaluate the expression
				  if(fpptr->values.vPROCEDURE.passFlags & mask )
				  {
					d = getVarSubPtr();
					*varLst[sidno++]->data = *d;
					scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
					// set pushFlag so evalPostFix will push var id read in from token stream
					//pushFlag = TRUE;
					 err = prgProcGlobVarStack.push(d);
				   }
				   else
					 EVAL(*varLst[sidno++]->data,r,nullErrBreak);
				  pushFlag = FALSE;
				  mask = mask << 1;
				}

				initLocals(fpptr);
				prgProcStack->push(fpptr);
				if(err)
				  break;

				// perform same code as gosub goto
				//err = retAddr.push(scriptPointer+sizeof(sint)+sizeof(sint));
				err = retAddr.push((scriptPointer-scriptBuffer)+sizeof(sint)+sizeof(sint)); // DWT:  changed int to sint

				scriptPointer = (scriptBuffer+(((fpptr->values.vPROCEDURE.offset))))-sizeof(sint);	 // DWT:  changed int to sint
				checkstatus();
				}
				break;


			case TOK_FPCLR: 	 // ... TOK_FPCLR ...

				 if(!prgProcStack->isEmpty())
				 {
				   fpptr = prgProcStack->pop();
				   currProc = currProcStack.pop();
				   if(fpptr && fpptr->type == vtPROCEDURE)	err = clearProc(fpptr);
				 }
				 // Return to original code offset.
				 if (retAddr.isEmpty())
					exitFlag = TRUE;
				 else
					//scriptPointer = retAddr.pop()-sizeof(sint)-sizeof(sint);
					scriptPointer = (char _FAR_ *)((long)scriptBuffer+retAddr.pop()-sizeof(sint)-sizeof(sint)); // DWT:  changed int to sint
				 break;



			default:
				scriptErr(0,-1,SCR_ERR_TOKEN,"%d",token);
				err = TRUE;
				break;
		}


		if(breakFlag)
		{

			exitFlag = TRUE;
			if(numCalls > 1)
			{

			 while(!prgProcStack->isEmpty() )
			 {
			   fpptr = prgProcStack->pop();
			   if(fpptr != NULL)
			   {
				 currProc = fpptr;
				 err = clearProc(fpptr);
			   }
			 }

			}
		}

		scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
	} while (!exitFlag && !err);

	if (confBitsAccessed) updateuserinformation();

	#ifndef __OS2__
	  nosound(); // Ensure speaker off after PPL execution
	#endif

	if (af != NULL)
	{
		closeChan(0);

		if (!err && !aborted)
		{
			if (appendfile(uniqueName,af,TRUE) == -1)
				scriptErr(0,-1,SCR_ERR_UNAPND,"%s",uniqueName);
			else
				ansAppended = TRUE;
		}

		if (unlink(uniqueName) == -1)										  //lint !e645	 uniquename inited in same conditional.
			scriptErr(0,-1,SCR_ERR_UNDEL,"%s",uniqueName);
	}
	if(!prgProcStkStk.isEmpty())
	{
	  tVS * tmp = prgProcStkStk.pop();
	  if(tmp != NULL)
		delete tmp;
	  prgProcStack = prgProcStkStk.pop();
	  err = prgProcStkStk.push(prgProcStack);
	}

	return err;
}

#if defined(__OS2__) && defined(__BORLANDC__)
  #pragma option -Oe	  // BC++ for OS/2 bug - re-enable global register allocation optimization
  #ifdef DBASE
	#pragma option -Oc	  // BC++ for OS/2 bug - re-enable local optimizations
  #endif
#endif

	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::execTPA(int token, int & err)
{
	static char    lastKW[8+1] = "\0";
	static apptype appHdr;

	char	  keyword[8+1];
	unsigned  conf;
	cVARVAL * d;
#ifdef ERRBRK
	cVARVAL * r;
#endif

	checkstack();
	// Evaluate the keyword and store it in str1
	EVAL(tmpString,r,nullErrReturn);
	{ cVARVAL _t1(vtSTRING," "); tmpString = tmpString.strip(_t1); }
	{ cVARVAL _t2(vtINTEGER,8); tmpString = tmpString.left(_t2); }
	{ cVARVAL _t3(vtSTRING," "); tmpString = tmpString.trim(_t3); }
	strcpy(keyword,nul2mty(tmpString));

	// Get either the variable or expression
	switch (token)
	{
		case TOK_TPAGET:
		case TOK_TPACGET:
		case TOK_TPAREAD:
		case TOK_TPACREAD:
			d = getVarSubPtr();
			break;

		default:
			d = evalArg();
			break;
	}

	nullErrReturn(d);

	// Make a copy of d in case it is an evaluated expression
	cVARVAL tmpVar(*d);

	// Get the conference number if necessary
	switch (token)
	{
		case TOK_TPACGET:
		case TOK_TPACPUT:
		case TOK_TPACREAD:
		case TOK_TPACWRITE:
			EVAL(tmpInteger,r,nullErrReturn);
			break;
	}

	if (err) return;

	conf = unsigned(tmpInteger.values.vINTEGER);

	// Get the tpa header if we have a new, different, keyword
	if ((*keyword != '\0') && (strcmp(lastKW,keyword) != 0))
	{
		if (gettpaheader(keyword,&appHdr) == 0)
			strcpy(lastKW,keyword);
		else
			*lastKW = '\0';
	}

	// If lastKW is a 0 length string, error, so return
	if (*lastKW == '\0') return;

	char	 * strBuf;
	unsigned   blen;

	blen = appHdr.SizeOfRec;
	if (blen < appHdr.SizeOfConfRec) blen = appHdr.SizeOfConfRec;

	strBuf = (char *) malloc(blen+1);
	if (!strBuf) return;

	switch (token)
	{
		case TOK_TPAGET:
		case TOK_TPAREAD:
			gettparec(&appHdr,ptrUData->RecNum,strBuf); 					   //lint !e534
			blen = appHdr.SizeOfRec;
			break;

		case TOK_TPACGET:
		case TOK_TPACREAD:
			gettpaconfrec(&appHdr,ptrUData->RecNum,conf,strBuf);			   //lint !e534
			blen = appHdr.SizeOfConfRec;
			break;

		case TOK_TPAPUT:
		case TOK_TPACPUT:
			tmpBigStr = tmpVar;
			break;
	}

	if (blen > MAX_BIGSTR_LEN) blen = MAX_BIGSTR_LEN;

	switch (token)
	{
		case TOK_TPAGET:
		case TOK_TPACGET:
			tmpBigStr.pokeBuf(strBuf,blen);
			*d = tmpBigStr;
			break;

		case TOK_TPAREAD:
		case TOK_TPACREAD:
			d->pokeBuf(strBuf,blen);
			break;

		case TOK_TPAPUT:
			puttparec(&appHdr,ptrUData->RecNum,nul2mty(tmpBigStr)); 		   //lint !e534
			break;

		case TOK_TPACPUT:
			puttpaconfrec(&appHdr,ptrUData->RecNum,conf,nul2mty(tmpBigStr));   //lint !e534
			break;

		case TOK_TPAWRITE:
			puttparec(&appHdr,ptrUData->RecNum,tmpVar.varAddr());			   //lint !e534
			break;

		case TOK_TPACWRITE:
			puttpaconfrec(&appHdr,ptrUData->RecNum,conf,tmpVar.varAddr());	   //lint !e534
			break;
	}

	free(strBuf);
}

	/*--------------------------------------------------------------------*/

#define getUserVar(id,sub,info) 			\
		{									\
			d = aVarValPtr(VID_U_##id,sub); \
			nullErrReturn(d);				\
			*d = info;						\
		}

#ifndef ERRBRK
#pragma argsused
#endif
void LIBENTRY cSCRIPT::getUserVars(long user, int & err)
{
	err = FALSE;
	lastUserGot = NO_USER_GOT;

	checkstack();
	if (user == Status.UserRecNo) user = CUR_USER_GOT;

	ptrUData = &UsersData;
	ptrUReg  =	ConfReg;
	ptrULMR  =	MsgReadPtr;

	if ((user < CUR_USER_GOT) || (user > numrecs(UsersFile,sizeof(URead))))
	{
		return;
	}
	else if (user != CUR_USER_GOT)
	{
		if (!altUserAvailable)								  return;
		//if ((TempRead == NULL) && (tempuseralloc(TRUE) != 0)) return;
		if ((TempRead == NULL) && (tempuseralloc(altUserLMRFlag) != 0)) return;
		if (gettemprecord(user) != 0)						  return;
		convertreadtodata(TempRead,TempData);

		ptrUData = TempData;
		ptrUReg  = TempReg;
		ptrULMR  = (altUserLMRFlag ? TempMsgPtr : NULL);
	}

	cVARVAL * d;
	int 	  i;

	if (ppeVer >= 100) // These were supported in version 1.00 PPE files
	{
		// Initialize the protocol for later use
		char prot[2];
		prot[0] = ptrUData->Protocol;
		prot[1] = '\0';

        getUserVar(EXPERT, 0,(int) ptrUData->ExpertMode);
		getUserVar(FSE,    0,ptrUData->PackedFlags.FSEDefault);
        getUserVar(FSEP,   0,(int) !ptrUData->PackedFlags.DontAskFSE);
		getUserVar(CLS,    0,ptrUData->PackedFlags.MsgClear);
		getUserVar(EXPDATE,0,ptrUData->RegExpDate);
		getUserVar(SEC,    0,ptrUData->SecurityLevel);
		getUserVar(PAGELEN,0,ptrUData->PageLen);
		getUserVar(EXPSEC, 0,ptrUData->ExpSecurityLevel);
		getUserVar(CITY,   0,ptrUData->City);
		getUserVar(BDPHONE,0,ptrUData->BusDataPhone);
		getUserVar(HVPHONE,0,ptrUData->HomeVoicePhone);
		getUserVar(TRANS,  0,prot);
		getUserVar(CMNT1,  0,ptrUData->UserComment);
		getUserVar(CMNT2,  0,ptrUData->SysopComment);
		getUserVar(PWD,    0,ptrUData->Password);
		getUserVar(SCROLL, 0,ptrUData->PackedFlags.ScrollMsgBody);
        getUserVar(LONGHDR,0,(int) !ptrUData->PackedFlags.ShortHeader);
		getUserVar(DEF79,  0,ptrUData->PackedFlags.WideEditor);
		getUserVar(ALIAS,  0,ptrUData->Alias);
		getUserVar(VER,    0,ptrUData->Verify);

		for (i = 0; i < 2; ++i)
			getUserVar(ADDR,i,ptrUData->Address.Street[i]);

		getUserVar(ADDR,   2,ptrUData->Address.City);
		getUserVar(ADDR,   3,ptrUData->Address.State);
		getUserVar(ADDR,   4,ptrUData->Address.Zip);
		getUserVar(ADDR,   5,ptrUData->Address.Country);

		for (i = 0; i < 5; ++i)
			getUserVar(NOTES,i,ptrUData->Notes.Line[i]);
#ifndef LIB
		getUserVar(PWDEXP, 0,ptrUData->PwrdHistory.ExpireDate);
#endif
	}

	lastUserGot = user;
}

	/*--------------------------------------------------------------------*/

#define putUserVar(id,sub,info,expr)		\
		{									\
			d = aVarValPtr(VID_U_##id,sub); \
			nullErrReturn(d);				\
			info = expr;					\
		}

#define putUserStr(id,sub,info,len) 		\
		{									\
			d = aVarValPtr(VID_U_##id,sub); \
			nullErrReturn(d);				\
			strcpy(s,nul2mty(*d));			\
			s[len] = '\0';                  \
			stripright(s,' ');              \
			strcpy(info,s); 				\
		}

#define putUserFunc(id,sub,info,len,func)	\
		{									\
			d = aVarValPtr(VID_U_##id,sub); \
			nullErrReturn(d);				\
			strcpy(s,nul2mty(*d));			\
			s[len] = '\0';                  \
			stripright(s,' ');              \
			func;							\
			*d = info;						\
		}

#ifndef ERRBRK
#pragma argsused
#endif
void LIBENTRY cSCRIPT::putUserVars(int & err)
{
	checkstack();
	err = FALSE;
	switch (lastUserGot)
	{
		case NO_USER_GOT:
			return;

		case CUR_USER_GOT:
			lastUserGot = Status.UserRecNo;
			break;
	}

	cVARVAL * d;
	int 	  i;
	char	  s [ MAX_STR_LEN+1 ];

	if (ppeVer >= 100) // These were supported in version 1.00 PPE files
	{
		putUserVar(EXPERT, 0,ptrUData->ExpertMode,				  d->values.vBOOLEAN);
		putUserVar(FSE,    0,ptrUData->PackedFlags.FSEDefault,	  d->values.vBOOLEAN);
		putUserVar(FSEP,   0,ptrUData->PackedFlags.DontAskFSE,	 !d->values.vBOOLEAN);
		putUserVar(CLS,    0,ptrUData->PackedFlags.MsgClear,	  d->values.vBOOLEAN);
		putUserVar(EXPDATE,0,ptrUData->RegExpDate,				  d->values.vDATE);
		putUserVar(SEC,    0,ptrUData->SecurityLevel,			 tBYTE(d->values.vINTEGER & 0x000000FF));
		putUserVar(PAGELEN,0,ptrUData->PageLen, 				 tBYTE(d->values.vINTEGER & 0x000000FF));
		putUserVar(EXPSEC, 0,ptrUData->ExpSecurityLevel,		 tBYTE(d->values.vINTEGER & 0x000000FF));
		putUserVar(TRANS,  0,ptrUData->Protocol,				 ((*nul2mty(*d) == '\0') ? 'N' : *nul2mty(*d)));
		putUserVar(SCROLL, 0,ptrUData->PackedFlags.ScrollMsgBody, d->values.vBOOLEAN);
		putUserVar(LONGHDR,0,ptrUData->PackedFlags.ShortHeader,  !d->values.vBOOLEAN);
		putUserVar(DEF79,  0,ptrUData->PackedFlags.WideEditor,	  d->values.vBOOLEAN);

		putUserStr(CITY,   0,ptrUData->City,					 24);
		putUserStr(BDPHONE,0,ptrUData->BusDataPhone,			 13);
		putUserStr(HVPHONE,0,ptrUData->HomeVoicePhone,			 13);
		putUserStr(CMNT1,  0,ptrUData->UserComment, 			 30);
		putUserStr(CMNT2,  0,ptrUData->SysopComment,			 30);

			/*----------------------------------------------------*/

		unsigned expDate = ptrUData->PwrdHistory.ExpireDate;

		putUserFunc(PWD,   0,ptrUData->Password,				 12,setnewpassword(ptrUData,strupr(s)));	 //lint !e534

		d = aVarValPtr(VID_U_PWDEXP);
//		  nullErrBreak(d);

		if (expDate == ptrUData->PwrdHistory.ExpireDate)
			ptrUData->PwrdHistory.ExpireDate = d->values.vDATE;
		else
			*d = ptrUData->PwrdHistory.ExpireDate;

			/*----------------------------------------------------*/

		putUserFunc(ALIAS, 0,ptrUData->Alias,					 25,setnewalias(ptrUData,strupr(s),lastUserGot));  //lint !e534

			/*----------------------------------------------------*/

		putUserStr(VER,    0,ptrUData->Verify,					 25);

		for (i = 0; i < 2; ++i)
			putUserStr(ADDR,i,ptrUData->Address.Street[i],50);

		putUserStr(ADDR,2,ptrUData->Address.City,25);
		putUserStr(ADDR,3,ptrUData->Address.State,10);
		putUserStr(ADDR,4,ptrUData->Address.Zip,10);
		putUserStr(ADDR,5,ptrUData->Address.Country,15);

		for (i = 0; i < 5; ++i)
			putUserStr(NOTES,i,ptrUData->Notes.Line[i],60);
			/*----------------------------------------------------*/

		Display.PageLen = ptrUData->PageLen;
	}

	if (lastUserGot == Status.UserRecNo) lastUserGot = CUR_USER_GOT;

	if (lastUserGot == CUR_USER_GOT)
	{
		updateuserinformation();
		makepcboardsys();
	}
	else
	{
		convertdatatoread(TempData,TempRead);
		puttemprecord(lastUserGot,IGNOREDIRTY);
	}
}

	/*--------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::execFile(int token, int & err)
{
	char meth;

	int  channel=0, mode, share, openFlags=0, flen, leftJust;

	long pos;

	cVARVAL * r, * d;

	char name[MAX_STR_LEN+1], strbuf[MAX_BIGSTR_LEN+1+2], fdat[MAX_STR_LEN+1];

	scriptPointer += sizeof(sint);		// DWT:  changed int to sint

	switch (token)
	{
		case TOK_FPUT:					// These four must be allowed to do
		case TOK_FPUTLN:				// their own channel selection in the
		case TOK_FDPUT: 				// fputVal/fdputVal functions because
		case TOK_FDPUTLN:				// of the variable arg list size
			break;

		case TOK_FDGET: 				// Use default input channel
		case TOK_FDREAD:
			channel = defInChan;
			scriptPointer -= sizeof(sint);	  // DWT:  changed int to sint
			break;

		case TOK_FDPUTPAD:				// Use default output channel
		case TOK_FDWRITE:
			channel = defOutChan;
			scriptPointer -= sizeof(sint);	  // DWT:  changed int to sint
			break;

		default:						// Otherwise, evaluate channel
			r = evalPostFix();
			nullErrReturn(r);
			tmpInteger = *r;
			if ((token == TOK_FCLOSE) && (tmpInteger.values.vINTEGER < 0))
				channel = -1;
			else
				channel = int(tmpInteger.values.vINTEGER % MAX_FILE_CHAN);
			break;
	}

	switch (token)
	{
		case TOK_FDEFIN:
			defInChan = channel;											  //lint !e644
			break;

		case TOK_FDEFOUT:
			defOutChan = channel;											  //lint !e644
			break;


		case TOK_FCREATE:
		case TOK_FOPEN:
		case TOK_FAPPEND:

			EVAL(tmpString,r,nullErrBreak);
			strcpy(name,nul2mty(tmpString));

			EVAL(tmpInteger,r,nullErrBreak);
			mode = int(tmpInteger.values.vINTEGER & 0x00000003);

			EVAL(tmpInteger,r,nullErrBreak);
			share = int(tmpInteger.values.vINTEGER & 0x00000003);

			switch (mode)
			{
				case 0: // READ
					openFlags = OPEN_READ;
					break;

				case 1: // WRIT
					openFlags = OPEN_WRIT;
					break;

				case 2: // RDWR
				case 3: // WRRD :)
					openFlags = OPEN_RDWR;
					break;
			}

			switch (token)
			{
				case TOK_FCREATE:
					openFlags |= OPEN_CREATE;
					break;

				case TOK_FAPPEND:
					openFlags  = OPEN_APPEND|OPEN_RDWR;
					break;
			}

			switch (share)
			{
				case 0: // Deny NONE
					openFlags |= OPEN_DENYNONE;
					break;

				case 1: // Deny READ
					openFlags |= OPEN_DENYREAD;
					break;

				case 2: // Deny WRIT
					openFlags |= OPEN_DENYWRIT;
					break;

				case 3: // Deny RDWR
					openFlags |= OPEN_DENYRDWR;
					break;
			}

			openChan(channel,name,openFlags);

			break;


		case TOK_FCLOSE:

			if (channel == -1)
			{
				if (readLineStream.handle != 0) dosfclose(&readLineStream);
			}
			else
			{
				closeChan(channel);
			}

			break;


		case TOK_FREWIND:
			rewindChan(channel);
			break;


		case TOK_FSEEK:

			EVAL(tmpInteger,r,nullErrBreak);
			pos = tmpInteger.values.vINTEGER;

			EVAL(tmpInteger,r,nullErrBreak);
			meth = (char) (tmpInteger.values.vINTEGER) % 3;

			seekChan(channel,pos,meth);

			break;


		case TOK_FFLUSH:
			flushChan(channel);
			break;


		case TOK_FGET:
		case TOK_FDGET:

			{
			*strbuf = '\0';
			getChan(channel,strbuf,MAX_BIGSTR_LEN+1+2);
			strbuf[MAX_BIGSTR_LEN] = '\0';

			d = getVarSubPtr();
			nullErrBreak(d);
			*d = cVARVAL(vtBIGSTR,strbuf);
			//*d = strbuf;
			int i = strlen(strbuf);
			if(d->type == vtBIGSTR)
			  d->values.vBIGSTR.len = (uint) i;

			if(d->type == vtBIGSTR)
			  d->values.vBIGSTR.len = (uint) strlen(strbuf);
			}
			break;


		case TOK_FPUT:
		case TOK_FPUTLN:
			if (doForItems(fputVal)) err = TRUE;
			if (token == TOK_FPUTLN) putChan(curChan,"\r\n");
			break;

		case TOK_FDPUT:
		case TOK_FDPUTLN:
			if (doForItems(fdputVal)) err = TRUE;
			if (token == TOK_FDPUTLN) putChan(curChan,"\r\n");
			break;


		case TOK_FPUTPAD:
		case TOK_FDPUTPAD:

			EVAL(tmpString,r,nullErrBreak);
			strcpy(fdat,nul2mty(tmpString));

			EVAL(tmpInteger,r,nullErrBreak);
			flen = int(tmpInteger.values.vINTEGER);

			leftJust = FALSE;

			if (tmpInteger.values.vINTEGER < 0)
			{
				leftJust = TRUE;
				flen = -flen;
			}

			if (flen > MAX_STR_LEN) flen = MAX_STR_LEN;

			if (flen != 0)
			{
				sprintf(strbuf, leftJust ? "%-*.*s" : "%*.*s",
					flen, flen, fdat);
				putChan(channel,strbuf);
			}

			break;


		case TOK_FREAD:
		case TOK_FDREAD:

			// Get varid for read
			d = getVarSubPtr();
			nullErrBreak(d);

			// Get size of read
			EVAL(tmpInteger,r,nullErrBreak);
			flen = int(tmpInteger.values.vINTEGER);
			if (flen < 0)			   flen = 0;
			if (flen > MAX_BIGSTR_LEN) flen = MAX_BIGSTR_LEN;

			readChan(channel,strbuf,flen);

			d->pokeBuf(strbuf,flen);

			break;


		case TOK_FWRITE:
		case TOK_FDWRITE:

			// Get expr to write
			d = evalArg();
			nullErrBreak(d);

			cVARVAL tmp(*d);

			// Get size of write
			EVAL(tmpInteger,r,nullErrBreak);
			flen = int(tmpInteger.values.vINTEGER);
			if (flen < 0)			   flen = 0;
			if (flen > MAX_BIGSTR_LEN) flen = MAX_BIGSTR_LEN;

			writeChan(channel,(char *) tmp.varAddr(),flen);

			break;
	}
}

	/*--------------------------------------------------------------------*/


	/*------------------------------------------------------------------*/

cVARVAL * LIBENTRY cSCRIPT::aVarValPtr(int varID, int v, int m, int c)
{
	cVAR * p = varLst[varID-1];
	return p->getVal(v,m,c);
}

	/*------------------------------------------------------------------*/

cVARVAL * LIBENTRY cSCRIPT::aVarSubPtr(int varID)
{
	cVAR * p = varLst[varID-1];
	return getSub(p);
}

	/*------------------------------------------------------------------*/

cVAR * LIBENTRY cSCRIPT::aVarPtr(int varID)
{
	return varLst[varID-1];
}

	/*------------------------------------------------------------------*/

cVARVAL * LIBENTRY cSCRIPT::curVarSubPtr(void)
{
	return aVarSubPtr(curVID());
}

	/*------------------------------------------------------------------*/

cVARVAL * LIBENTRY cSCRIPT::getVarSubPtr(void)
{
	return aVarSubPtr(getVID());
}

	/*------------------------------------------------------------------*/

cVAR * LIBENTRY cSCRIPT::getVarPtr(void)
{
	return aVarPtr(getVID());
}

	/*------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::curVID(void)
{
	return *((sint _FAR_ *) scriptPointer); 	 // DWT: changed int to sint
}

	/*------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::getVID(void)
{
	scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
	return curVID();
}

	/*------------------------------------------------------------------*/

cVARVAL * LIBENTRY cSCRIPT::getSub(cVAR * p)
{
	static cVARVAL zero(vtINTEGER,(long signed int) 0);

	int i, v, m, c, cnt;

	cVARVAL * r;

	long l;

	v = 0; m = 0; c = 0;

	checkstack();
	scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
	cnt = *((sint _FAR_ *) scriptPointer);	// DWT:  changed int to sint

	for (i = 0; i < cnt; ++i)
	{
		r = evalArg();
		if (r == NULL) return NULL;
		tmpInteger = *r;

		l = tmpInteger.values.vINTEGER;
//		  if ((l < 0) || (l > 65535U)) break;

//		  c = m;
//		  m = v;
//		  v = int(l);

		switch (i)
		{
			case 0:
				v = int(l);
				break;

			case 1:
				m = int(l);
				break;

			default:
				c = int(l);
				break;
		}
	}

	if (i < cnt) return NULL;

	cVARVAL * sub = p->getVal(v,m,c);

	if (sub == NULL)
	{
		switch (zero.type)
		{
			case vtSTRING:
				if (zero.values.vSTRING != NULL)
					free(zero.values.vSTRING);						// USed to be BFREE
				break;

			case vtBIGSTR:
				if (zero.values.vBIGSTR.ptr != NULL)
					free(zero.values.vBIGSTR.ptr);					// USed to be BFREE
				break;
		}
		zero.type = p->data->type;
//		  memset(&zero.values,0,sizeof(zero.values));
		zero.initialize();
		sub = &zero;
	}

	return sub;
}

	/*------------------------------------------------------------------*/

cVARVAL * LIBENTRY cSCRIPT::evalArg(void)
{

	scriptPointer += sizeof(sint);	  // DWT:  changed int to sint
	return evalPostFix();
}

	/*------------------------------------------------------------------*/

char * LIBENTRY cSCRIPT::expandMask(char * msk)
{
	char strBuf[MAX_STR_LEN+1]; 		   // Buffer for decompressed mask

	char * ptr = strBuf;				   // Initialize temporary pointer to start of strBuf

	int ms = *msk;						// Initialize the mask size

	checkstack();
	for (int i = 0; i < ms; ++i)		// Index into mask
	{
		int mc = msk[1+i];				// The current mask character

		if (mc == 0)					// Are we sitting on a range
		{
			int ml = msk[1+i+1];		// The low character of a range from mask
			int mh = msk[1+i+2];		// The high character of a range from mask

			// Decompress the range of characters
			for (int j = ml; j <= mh; ++j) *(ptr++) = (char) j;

			i += 2; 					// Skip the extra two character in the range
		}
		else
			*(ptr++) = (char) mc;		// Copy the character
	}

	*(ptr++) = '\0';                    // Terminate the decompressed string

	tmpString = strBuf; 				   // Make it a 'permanent' string
	return nul2mty(tmpString);			// Return a pointer to decompressed mask
}

	/*------------------------------------------------------------------*/

char * LIBENTRY cSCRIPT::readLine(char * fn, long ln, char * strBuf)
{
	checkstack();
	if ((ln < 1) || !fexist(fn)) return "";

	if ((readLineStream.handle == 0) || (stricmp(fn,readLineFile) != 0))
	{
		readLineLine = 0;
		if (readLineStream.handle != 0) dosfclose(&readLineStream);
		strcpy(readLineFile,fn);
		strupr(readLineFile);
		if (dosfopen(readLineFile,OPEN_READ|OPEN_DENYNONE,
			&readLineStream) != 0) return "";
	}

	if (readLineStream.handle == 0) return "";

	if (--ln < readLineLine)
	{
		dosrewind(&readLineStream);
		readLineLine = 0;
	}

	long i = readLineLine;

	readLineLine = ln + 1;

	for (; i < ln; ++i)
	{
		switch (dosfgets(strBuf,MAX_STR_LEN+1,&readLineStream))
		{
			case -1: return "";
			case  1: --i; break;
		}
	}

	*strBuf = '\0';
	char tmp[MAX_BIGSTR_LEN+1+2];
	if (dosfgets(strBuf,MAX_BIGSTR_LEN+1+2,&readLineStream) == 1)
		while (dosfgets(tmp,MAX_BIGSTR_LEN+1+2,&readLineStream) == 1) ;

	return strBuf;
}

	/*------------------------------------------------------------------*/

char * LIBENTRY scrtext(char * b, int x, int y, int l, bool f)
{
	char * p = b;

	unsigned char mem[MAX_COLS*2];

	if (l > MAX_COLS) l = MAX_COLS;

	#ifdef __OS2__
	  memcpy(mem,(char *) ((char *)Scrn_Buf + (((long)y-1)*MAX_COLS*2) + (((long)x-1)*2)), l*2);
	#else
	  memcpy(mem, (void _FAR_ *) (Scrn_Addr + (((long)y-1)*MAX_COLS*2) + (((long)x-1)*2)), l*2);
	#endif

	int lc = -1;						// The last color found

	checkstack();
	for (int loopDex = 0; loopDex < l; ++loopDex, ++b)
	{
		if (f && (mem[loopDex*2+1] != lc))
		{
			sprintf(b,"@X%02X",mem[loopDex*2+1]);
			b += 4;
		}

		lc = mem[loopDex*2+1];

		*b = mem[loopDex*2];
		if (*b == '\0') *b = ' ';
	}

	*b = '\0';

	return p;
}

	/*------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::doForItems(int LIBENTRY (* itemfunc)(cSCRIPT & obj, int num))
{
	int err = FALSE;
	sint itemCount = *((sint _FAR_ *) scriptPointer);  // DWT: changed int to sint
	for (int i = 0; (i < itemCount) && !err; ++i) err = itemfunc(*this,i);

	return err;
}

	/*------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::redimArray(cSCRIPT & obj, int num)
{
	static cVAR * vp   = NULL;
	static int	  dims = 0;
	static int	  cd   = 0;
	static int	  md   = 0;
	static int	  vd   = 0;

	int * ip = NULL;

	checkstack();
	switch (num)
	{
		case 0:
			vp = obj.getVarPtr();
			dims = 0;
			cd = 0;
			md = 0;
			vd = 0;
			if (vp == NULL) return TRUE;
			break;

		case 1:
			ip = &vd;
			break;

		case 2:
			ip = &md;
			break;

		case 3:
			ip = &cd;
			break;
	}

	if (num > 0)
	{
		++dims;

		cVARVAL * v = obj.evalArg();
		if (v == NULL) return TRUE;

		obj.tmpInteger = *v;
		*ip = int(obj.tmpInteger.values.vINTEGER);
	}

	if (vp->redimArray(dims,vd,md,cd)) return TRUE;

	return FALSE;
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::printVal(cSCRIPT & obj, int num)
{
	cVARVAL * var = obj.evalArg();
	checkstack();
	if (var == NULL) return TRUE;

	obj.tmpBigStr = *var;
	if(!obj.breakFlag)
	  printxlated(nul2mty(obj.tmpBigStr));

	return FALSE;
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::prFoundVal(cSCRIPT & obj, int num)
{
	cVARVAL * var = obj.evalArg();
	checkstack();
	if (var == NULL) return TRUE;

	obj.tmpBigStr = *var;
	obj.searchPrint(nul2mty(obj.tmpBigStr));

	return FALSE;
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::sprintVal(cSCRIPT & obj, int num)
{
	cVARVAL * var = obj.evalArg();
	checkstack();
	if (var == NULL) return TRUE;

	obj.tmpBigStr = *var;
	ansi(nul2mty(obj.tmpBigStr));

	return FALSE;
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::mprintVal(cSCRIPT & obj, int num)
{
	cVARVAL * var = obj.evalArg();
	checkstack();
	if (var == NULL) return TRUE;

#ifndef LIB
#ifdef COMM
	obj.tmpBigStr = *var;
	printcom(nul2mty(obj.tmpBigStr));
#endif
#endif

	return FALSE;
}

	/*------------------------------------------------------------------*/

int LIBENTRY cSCRIPT::fputVal(cSCRIPT & obj, int num)
{
	cVARVAL * var = obj.evalArg();
	checkstack();
	if (var == NULL) return TRUE;

	if (num == 0)
	{
		obj.tmpInteger = *var;
		curChan = int(obj.tmpInteger.values.vINTEGER & 0x00000007);
		return FALSE;
	}

	obj.tmpBigStr = *var;
	obj.putChan(curChan,nul2mty(obj.tmpBigStr));

	return FALSE;
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::fdputVal(cSCRIPT & obj, int num)
{
	if (num == 0) curChan = obj.defOutChan;

	cVARVAL * var = obj.evalArg();
	if (var == NULL) return TRUE;

	obj.tmpBigStr = *var;
	obj.putChan(curChan,nul2mty(obj.tmpBigStr));

	return FALSE;
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::pushVars(cSCRIPT & obj, int num)
{
	cVARVAL * var = obj.evalArg();
	checkstack();
	if (var == NULL) return TRUE;

	return obj.prgVarStack.push(new cVARVAL(*var));

	//return FALSE; 	   // changed to make lint happy
}

	/*------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY cSCRIPT::popVars(cSCRIPT & obj, int num)
{
	cVARVAL * var, * tmp = NULL;

	checkstack();
	var = obj.getVarSubPtr();
	if (var == NULL) return TRUE;

	if (!obj.prgVarStack.isEmpty())
		tmp = obj.prgVarStack.pop();

	if (tmp != NULL)
	{
		*var = *tmp;
		delete tmp;
	}
	else
	{
		switch (var->type)
		{
			case vtSTRING:							 // If it is a STRING
				if (var->values.vSTRING)			 // And it has a value
					 free(var->values.vSTRING); 	 // Free the value Used to be BFREE
				break;

		   case vtBIGSTR:							// If it is a BIGSTR
				if (var->values.vBIGSTR.ptr)		 // And it has a value
					 free(var->values.vBIGSTR.ptr);  // Free the value USed to be BFREE
				break;
		}
		var->initialize();
	}

	return FALSE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::openChan(int chan, char * name, int openFlags)
{
	fileArr[chan].errStat = FALSE;

	if (fileArr[chan].inUse ||
		(dosfopen(name,openFlags,&fileArr[chan].hand) != 0))
		fileArr[chan].errStat = TRUE;
	else
		fileArr[chan].inUse = TRUE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::closeChan(int chan)
{
	if (!fileArr[chan].inUse || (dosfclose(&fileArr[chan].hand) != 0))
		fileArr[chan].errStat = TRUE;

	fileArr[chan].inUse = FALSE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::rewindChan(int chan)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}

	dosrewind(&fileArr[chan].hand);
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::getChan(int chan, char * str, int size)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}

	if (dosfgets(str,size,&fileArr[chan].hand) != 0)
		fileArr[chan].errStat = TRUE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::putChan(int chan, char * str)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}

	if (dosfputs(str,&fileArr[chan].hand) != 0)
		fileArr[chan].errStat = TRUE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::seekChan(int chan, long pos, char meth)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}
	long off = dosfseek(&fileArr[chan].hand,pos,meth);
	if (off < 0L)
		fileArr[chan].errStat = TRUE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::flushChan(int chan)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}

	if (dosflush(&fileArr[chan].hand) != 0)
		fileArr[chan].errStat = TRUE;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::readChan(int chan, char * strBuf, int & size)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}

	int br = dosfread(strBuf,size,&fileArr[chan].hand);

	if ((br < 0) || (br < size)) fileArr[chan].errStat = TRUE;

	size = br;
}

	/*------------------------------------------------------------------*/

void LIBENTRY cSCRIPT::writeChan(int chan, char * strBuf, int size)
{
	if (!fileArr[chan].inUse)
	{
		fileArr[chan].errStat = TRUE;
		return;
	}

	size = dosfwrite(strBuf,size,&fileArr[chan].hand);

	if (size < 0) fileArr[chan].errStat = TRUE;
}

	/*------------------------------------------------------------------*/

typedef cLIST<char *> tENVLST;

tENVLST envLst;

int LIBENTRY envVarNameMatch(char * const & varval, const void * _var)
{
	char * var = (char *) _var; 				// convert it to char * for ease of use
	int len = strlen(var);						// get the length since we need it multiple times
	return ((memcmp(varval,var,len) == 0) &&	// if the names match and there
		(varval[len] == '='));                  // is an equal sign in right place, match
}

char * LIBENTRY pplgetenvvar(char * var)
{
	char * * ptr = envLst.firstThat(envVarNameMatch,var);	// find var
	char * value = NULL;									// assume no pointer
	if (ptr != NULL) value = *ptr+strlen(var)+1;			// move past "NAME="
	return value;											// return pointer to value
}

void LIBENTRY pplclrenvvar(char * var)
{
	char * * ptr = envLst.firstThat(envVarNameMatch,var);	// find var
	if (ptr != NULL)										// was it found?
	{														// yes, clear it
		delete *ptr;										// free memory
		envLst.detach(*ptr);								//lint !e534 remove from list
	}
}

void LIBENTRY envVarDelete(char * & obj)
{
	delete obj;
}

void LIBENTRY pplclrenvvars(void)
{
	envLst.forEach(envVarDelete);
	envLst.flush();
}

void LIBENTRY pplsetenvvar(char * varval)
{
	char var [ 128 + 1 ];						// a buffer to hold the name
	maxstrcpy(var,varval,sizeof(var));			// copy the whole thing

	char * value = strchr(var,'=');             // find the equal sign separating var and value
	checkstack();
	if (value != NULL)
		*(value++) = '\0';                      // null terminate var and initialize value at the same time
	else
		value = "";                             // no equal sign so no value

	strupr(var);								// make sure the name is upper case

	char * curval = pplgetenvvar(var);			// find current value if defined

	if (curval != NULL) 						// did we find a current value?
	{											// yes, remove it
		char buf [ 128 + 1 + 1 ];				// an extra character just in case
		buildstr(buf,var,"=",NULL);             // build "NAME="
		putenv(buf);							// remove from C environment list
		pplclrenvvar(var);						// remove var from variable list
	}

	if (strlen(value) > 0)						// do we need to set a value?
	{											// yes, add it
		char * ptr = new char[strlen(var)+1+strlen(value)+1];
		if (ptr != NULL)						// did we allocate the memory?
		{										// yes, use it
			buildstr(ptr,var,"=",value,NULL);   // rebuild var and value string
			envLst.add(ptr);					//lint !e534 add it to the list
			putenv(ptr);						// add it to the C environment list
		}
	}
}

/****************************************************************************/

#ifndef LIB

unsigned long LIBENTRY cSCRIPT::calcCRC32 (bool f, char * buf, unsigned len) {
  return(f ? crc32file(buf) : crc32buf(buf,len));
}
#endif
#endif
