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
#include <dos.h>
#include <io.h>
#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <screen.h>

#include "misc.h"
#include "pcb.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "messages.h"
#include "umwf.hpp"

#include "pcbmsgs.hpp"
#include "fidomsg.hpp"
#include "fidoque.hpp"




#ifdef DEBUG
#include <memcheck.h>
#endif


static void replaceMo(char * mo);
void   LIBENTRY capMsgInfo(unsigned short ConfNum, long MsgNum, long MsgOffset);
bool LIBENTRY killFileSent(const char * msg);

long                      lastMsgNum,lastMsgOff;
unsigned                  lastConfNum;
char                      zero = 0;

extern _FAR_   DIRECTORIES       directory_info;
extern             long                  filesize(char * filename);
extern struct  ffblk             DTA;


bool cFIDOMSG::preventFile = FALSE;

bool cFIDOMSG::createFile(const char * fromAddr,const char * toAddr,const char * Subject)
{

 msgheadertype hdr;
 long              num = 1;
 char              MSGfilename[MAXFLEN];
 cDOSFILE          MSGFile;
 char              nil = 0;
 char              cr  = 13;
 char     *    tptr = NULL;
 char     *    fptr = NULL;
 char              lineBuf[100];
 char              kludgeBuf[20];
 char              pointBufFrom[100];
 char              pointBufTo[100];
 char              from[50];
 char              to[50];

        // Fill in essential information
        memset(&hdr,0,sizeof(hdr));
        memset(&MSGHdr,0,sizeof(MSGHdr));
        if(strchr(toAddr,'@') == NULL)   sprintf(hdr.ToField,"Sysop@%-1.20s",(char *)toAddr);
        else                                     maxstrcpy(hdr.ToField,(char*)toAddr,sizeof(hdr.ToField));

        if(strchr(fromAddr,'@') == NULL)   sprintf(hdr.FromField,"PCBFido@%-1.20s",(char *)fromAddr);
        else                                       maxstrcpy(hdr.FromField,(char *)fromAddr,sizeof(hdr.FromField));

        maxstrcpy(hdr.SubjField,(char *)Subject,sizeof(hdr.SubjField));
        memcpy(hdr.Date,juliantodate(getjuliandate()),sizeof(hdr.Date));

        char ttime[6];
        memcpy(hdr.Time,timestr2(ttime),5);



        // IF the mail packet is already in the queue then we don't need to
        // Create another MSG for it.
        // Limit Q's scope
        //{
        //cNEWQ Q;
        //if(Q.getRecordNumber((char *)Subject,1) != -1) return TRUE;
        //}

        // Have fillMSGhdr fill in the header structure
        if(!fillMSGHdr(hdr)) return FALSE;


        maxstrcpy(MSGHdr.Subject,(char *)Subject,sizeof(MSGHdr.Subject));
        stripright(MSGHdr.Subject,' ');

        // Get rid of any address info
        if( (tptr = strrchr(MSGHdr.To_User,'@')) != NULL)
        {
          fido_nodestr_to_int(tptr+1,MSGHdr.Dest_Zone,MSGHdr.Dest_Net,MSGHdr.Dest_Node,MSGHdr.Dest_Point);
          maxstrcpy(to,tptr+1,sizeof(to));
          memset(tptr,0,size_t((MSGHdr.To_User+sizeof(MSGHdr.To_User))-tptr));
        }
        if( (tptr != NULL) && (fptr = strrchr(MSGHdr.From_User,'@')) != NULL)
        {
          *tptr = '\x0';
          maxstrcpy(from,fptr+1,sizeof(from));
          memset(fptr,0,size_t((MSGHdr.From_User+sizeof(MSGHdr.From_User))-fptr));
        }


        // Set essential attribute flags
        MSGHdr.Attribute = MSG_FILEATT | MSG_LOCAL | MSG_KILLSENT;
        num = findHiMSG(directory_info.outgoing_msg);
        sprintf(MSGfilename,"%s%ld.MSG",directory_info.outgoing_msg,num);


        sprintf(lineBuf,"\x01INTL %s %s \r",to,from);
        memset(pointBufFrom,0,sizeof(pointBufFrom));
        if(MSGHdr.Orig_Point != 0)
          sprintf(pointBufFrom,"\x01""FMPT %u \r",MSGHdr.Orig_Point);

        memset(pointBufTo,0,sizeof(pointBufTo));
        if(MSGHdr.Dest_Point != 0)
          sprintf(pointBufTo,"\x01TOPT %u \r",MSGHdr.Dest_Point);

        maxstrcpy(kludgeBuf,"\x01""FLAGS KFS ",sizeof(kludgeBuf));

        //Create MSG file with file attach
        if(MSGFile.open(MSGfilename,OPEN_RDWR |+ OPEN_CREATE |OPEN_DENYNONE)==0)
        {

          MSGFile.write(&MSGHdr,sizeof(MSGHdr));
          MSGFile.write(lineBuf,strlen(lineBuf));
          if(pointBufFrom[0] != '\x0') MSGFile.write(pointBufFrom,strlen(pointBufFrom));
          if(pointBufTo[0] != '\x0') MSGFile.write(pointBufTo,strlen(pointBufTo));
          MSGFile.write(kludgeBuf,strlen(kludgeBuf));
          MSGFile.write(&cr,1);
          MSGFile.write(&nil,1);
          MSGFile.close();
          return TRUE;
        }
        return FALSE;
}

long cFIDOMSG::findHiMSG(char * dir)
{
  char  file[100];
  long  hiNum = 0;
  ffblk fblk;
  int   dosfindmax=1;
  #ifdef __OS2__
  int DirHandle = SYSTEMDIRHANDLE;
  #endif

         checkstack();
         sprintf(file,"%-1.90s*.msg",dir);

         dosfindmax = 1;
         int done = dosfindfirst(file,&fblk,0,&dosfindmax PDIRHANDLE);

         while(!done)
         {
                if(atoi(fblk.ff_name) > hiNum) hiNum = atoi(fblk.ff_name);
                done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
         }
         return hiNum+1;
}



void cFIDOMSG::handleAttach(const char * messBuf)
{
 char oldFile[100];
 char newFile[100];
// char attFile[100];
 char fileName[100];

 char SubjBuf[80];
 char * ptr = NULL;
 char * ptr1 = NULL;
 bool   first = TRUE;
 QUEUE_RECORD rec;

         checkstack();
         maxstrcpy(SubjBuf,MSGHdr.Subject,sizeof(SubjBuf));
         ptr = SubjBuf;
         while(*ptr == ' ') ptr++;
         ptr1 = strtok(ptr," ");

         while(ptr1)
         {

          char * File = strrchr(ptr1,'\\');
          if(File) ++File;
          else File = ptr1;
          if(first)
          {
                 getconfrecord(this_area.PCB_Conference,&confrec);
                 sprintf(newFile,"%s%s",confrec.AttachLoc,File);
          }
          else sprintf(newFile,"%s%s",directory_info.outgoing_packets,File);

          // try to find the file.
          while(TRUE)
          {
                  // If the file  exist's as is, copy it to file buffer
                  if(ptr1 && fileexist(ptr1) != 255)
                  {
                         maxstrcpy(fileName,ptr1,sizeof(fileName));
                         break;
                  }

                  // If the file is in the current directory, copy that to file buffer
                  if(fileexist(File) != 255)
                  {
                          maxstrcpy(fileName,File,sizeof(fileName));
                          break;
                  }

                  // Now try the outbound directory
                  sprintf(oldFile,"%s%s",directory_info.outgoing_packets,File);
                  if(fileexist(oldFile) != 255)
                  {
                         maxstrcpy(fileName,oldFile,sizeof(fileName));
                         break;
                  }

                  // Now try the MSG directory
                  sprintf(oldFile,"%s%s",directory_info.outgoing_msg,fileName);
                  if(fileexist(oldFile) != 255)
                  {
                        maxstrcpy(fileName,oldFile,sizeof(fileName));
                        break;
                  }

                  // COuldn't find the file
                  sprintf(oldFile,"Could not find file %s for file attach.",File);
                  writeFidolog(oldFile,BLOCK);
                  break;

          }

          maxstrcpy(oldFile,fileName,sizeof(oldFile));
          if(fileexist(oldFile) != 255)
          {
                 memset(&rec,0,sizeof(rec));
                 maxstrcpy(rec.filename,ptr1,sizeof(rec.filename));

                 char destAddr[40],srcAddr[40],*addrPtr;
                 if(getINTLinfo(messBuf,destAddr,sizeof(destAddr),srcAddr,sizeof(srcAddr)) == FALSE)
                 {

                   addrPtr = strrchr(MSGHdr.To_User,'@');
                   if(addrPtr)
                         maxstrcpy(rec.nodestr,++addrPtr,sizeof(rec.nodestr));
                   else
                         sprintf(rec.nodestr,"%u:%u/%u",(MSGHdr.Dest_Zone > 7000U ? PcbData.FidoDefaultZone : MSGHdr.Dest_Zone),MSGHdr.Dest_Net,MSGHdr.Dest_Node);

                 }
                 else
                   maxstrcpy(rec.nodestr,destAddr,sizeof(rec.nodestr));


                 if(MSGHdr.Attribute & MSG_HOLD)
                 {
                   rec.readOnly = TRUE;
                   rec.flag = Q_HOLD;
                 }
                 else if(MSGHdr.Attribute & MSG_CRASH)
                 {
                   rec.readOnly = TRUE;
                   rec.flag = Q_CRASH | Q_OUTBOUND;
                 }

                 else rec.flag = Q_NORMAL;
                 if(strstr(MSGHdr.To_User,"+K")) rec.flag |= Q_KILLSENT;
                 rec.flag |= Q_FILESEND;

                 char * wp = rec.nodestr;

                 while(*wp != ' ' && *wp != '\x0') ++wp;
                 if(*wp == ' ') *wp = '\x0';

                 // Limit Q's scope
                 {
                 cNEWQ Q;
                   Q.addEntry(rec);
                 }
                first = FALSE;
           }
           ptr1 = strtok(NULL," ");
          }
}

bool cFIDOMSG::checkAttaches(const char * files)
{
  char allFiles[100];
  char * currentFile;
  int    i=0;
  QUEUE_RECORD rec;

         maxstrcpy(allFiles,(char *)files,sizeof(allFiles));

         currentFile = strtok(allFiles," ");
         if(currentFile == NULL) return FALSE;

         do
         {
         cNEWQ Q;
                while( (i = Q.getNextRecord(rec,i+1)) != -1)
                  if(strcmp(rec.filename,currentFile)) return TRUE;
         }while( (currentFile = strtok(NULL," ")) != NULL);
         return FALSE;
}

void cFIDOMSG::start_at_1MSG(void)
{
        ffblk fblk;
        char  dir1[80];
        char  dir2[80];
        char  logBuf[60];
        DOSFILE msgFile;
        bool  logged = FALSE;
        int   dosfindmax=1;
        #ifdef __OS2__
        //int DirHandle = SYSTEMDIRHANDLE;
        int DirHandle = MAKEDIRHANDLE;
        #endif

          checkstack();


          // If MSG is disabled return
          if(directory_info.outgoing_msg[0] == '\x0' || directory_info.outgoing_msg[0] == ' ')
                return;

          initUMWF();
          sprintf(dir1,"%s*.MSG",directory_info.outgoing_msg);

          // Delete all MSG files
          dosfindmax = 1;
          int done = dosfindfirst(dir1,&fblk,0,&dosfindmax PDIRHANDLE);
          while(!done)
          {
                sprintf(logBuf,"Looking at %s                      ",fblk.ff_name);
                fastprint(START_COL,STATUS_LINE,logBuf,0x0b);
                sprintf(dir2,"%s%s",directory_info.outgoing_msg,fblk.ff_name);
                if(dosfopen(dir2,OPEN_RDWR | OPEN_DENYNONE,&msgFile) == -1)
                {
                  done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
                  continue;
                }
                if(dosfread(&MSGHdr,sizeof(MSGHdr),&msgFile)==-1)
                {
                  dosfclose(&msgFile);
                  done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
                  continue;
                }

                bool delit = FALSE;
                if(!PcbData.FidoCreateMsg && MSGHdr.Attribute & MSG_SENT)
                {
                        if( (MSGHdr.Attribute & MSG_KILLSENT) &&
                                (MSGHdr.Attribute & MSG_FILEATT)   &&
                                (checkAttaches(MSGHdr.Subject)) == FALSE )
                                        delit = TRUE;

                        if( (MSGHdr.Attribute & MSG_KILLSENT) &&
                           !(MSGHdr.Attribute & MSG_FILEATT))
                                  delit = TRUE;
                }
                if(! (MSGHdr.Attribute & MSG_SENT))
                {
                        if(!logged) writeFidolog("Beginning *.MSG processing.",BLOCK_START);
                        logged = TRUE;
                        sprintf(logBuf,"Processing  %s.                                  " ,fblk.ff_name);
                        MSGHdr.Attribute |= MSG_SENT;
                        dosfseek(&msgFile,0,SEEK_SET);
                        dosfwrite(&MSGHdr,sizeof(MSGHdr),&msgFile);
                        dosfclose(&msgFile);
                        FiletoMsg(fblk.ff_name);
                        writeFidolog(logBuf,BLOCK);
                        fastprint(START_COL,MESSAGES,logBuf,0x0b);
                }

                dosfclose(&msgFile);
                if(delit)
                {
                        unlink(dir2);
                        done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
                        continue;
                }
                done = dosfindnext(&fblk,&dosfindmax PDIRHANDLE2);
          }
          #ifdef __OS2__
                dosclosedirhandle(DirHandle);
          #endif
          if(logged) writeFidolog("Ending *.MSG processing.",BLOCK_END);

          doUMWF();
          resetUMWF();
}


bool cFIDOMSG::fillMSGHdr(msgheadertype & hdr)
{
 char   *                        ptr = NULL;
 char                            tmp[80];
 char                extto[75],extfrom[75],extsubj[75];
 int                 i=0,matched_aka=0;
 extern unsigned int num_akas;
 extern NADDRESS *       address;

   checkstack();
   memset(&MSGHdr,0,sizeof(MSGHdr));
   memset(extto,0,sizeof(extto));
   memset(extfrom,0,sizeof(extfrom));
   memset(extsubj,0,sizeof(extfrom));

   if(message.getExtHdrs()!=0)
   {
         message.getExtHdrs(ExtHdrFunctions[EXTHDR_FROM],extfrom);
         if(extfrom[0] == '\x0') memcpy(extfrom,hdr.FromField,sizeof(hdr.FromField));

         message.getExtHdrs(ExtHdrFunctions[EXTHDR_TO],extto);
         if(extto[0] == '\x0') memcpy(extto,hdr.ToField,sizeof(hdr.ToField));

         message.getExtHdrs(ExtHdrFunctions[EXTHDR_SUBJECT],extsubj);
         if(extsubj[0] == '\x0') memcpy(extsubj,hdr.SubjField,sizeof(hdr.SubjField));

         if( (ptr = strrchr(extto,'@')) == NULL) return FALSE;
   }
   else
   {
         memcpy(extfrom,hdr.FromField,sizeof(hdr.FromField));
         memcpy(extto,hdr.ToField,sizeof(hdr.ToField));
         memcpy(extsubj,hdr.SubjField,sizeof(hdr.SubjField));

         if( (ptr = strrchr(extto,'@')) == NULL) return FALSE;
   }

   stripright(extfrom,' ');
   stripright(extto,' ');
   stripright(extsubj,' ');

   maxstrcpy(MSGHdr.From_User,extfrom,sizeof(MSGHdr.From_User));
   maxstrcpy(MSGHdr.To_User,extto,sizeof(MSGHdr.To_User));
   maxstrcpy(MSGHdr.Subject,extsubj,sizeof(MSGHdr.Subject));

   // Prepare header DATE information
   translate_date(MSGHdr.Date_Time,hdr.Date,hdr.Time,20);


   fido_nodestr_to_int(++ptr,MSGHdr.Dest_Zone,MSGHdr.Dest_Net,MSGHdr.Dest_Node,MSGHdr.Dest_Point);


   // if there is no @ sysmbol in any of the FROM headers, then the message
   // originated on this board
   if( (ptr = strrchr(MSGHdr.From_User,'@')) == NULL)
   {
       for(i=0,matched_aka=0;i<num_akas;i++)
           {
                 if(address[i].zone==MSGHdr.Dest_Zone) matched_aka=i;

                 if(address[i].zone==MSGHdr.Dest_Zone && address[i].net==MSGHdr.Dest_Net)
                 {
                        matched_aka=i;
                        break;
                 }
           }

           MSGHdr.Orig_Zone  = address[matched_aka].zone;
           MSGHdr.Orig_Net       = address[matched_aka].net;
           MSGHdr.Orig_Node  = address[matched_aka].node;
           MSGHdr.Orig_Point = address[matched_aka].point;

           sprintf(tmp,"%s@%d:%d/%d",MSGHdr.From_User,(MSGHdr.Orig_Zone == 0) ? 1 : MSGHdr.Orig_Zone,
                                                                                                 MSGHdr.Orig_Net,MSGHdr.Orig_Node);
           maxstrcpy(MSGHdr.From_User,tmp,sizeof(MSGHdr.From_User));
   }
   else
         fido_nodestr_to_int(++ptr,MSGHdr.Orig_Zone,MSGHdr.Orig_Net,MSGHdr.Orig_Node,MSGHdr.Orig_Point);


        if(MSGHdr.Orig_Point != 0)
        {
          sprintf(tmp,"%s.%d",MSGHdr.From_User,MSGHdr.Orig_Point);
          maxstrcpy(MSGHdr.From_User,tmp,sizeof(MSGHdr.From_User));
        }

                /* Init unused values to zero. */

        MSGHdr.Times_Read=0;
        MSGHdr.Next_Reply=0;
        MSGHdr.Reply_To=0;
        MSGHdr.Cost=0;

   return TRUE;
}

void cFIDOMSG::getHighMsgNo(long & himsgnum, long & lmx)
{

  checkstack();

  if(directory_info.outgoing_msg[0] == '\x0' || directory_info.outgoing_msg[0] == ' ')
        return;

  // Get conference name associated with the NETMAIL tag
  if(getFidoArea("NETMAIL")==FALSE)
        return;

  // Get the conference record for the NETMAIL area
  getconfrecord(this_area.PCB_Conference,&confrec);

  // Open the NETMAIL message base
  message.close();
  if(message.open(confrec.MsgFile) != 0) return;

  himsgnum = message.highMsgNum();
  lmx      = confrec.ExportPtr;

}


// Convert a *.MSG file to a PCBoard netmail message
bool cFIDOMSG::FiletoMsg(const char * fileName)
{
DOSFILE                         MSGfile;
char                            filenameBuf[80];
char                            TOBuf[60],FromBuf[60];
char                            destAddr[35],srcAddr[35];
char                            mo[4],day[3],yr[3],Date[9],Time[10];
char *                          tptr = NULL;
msgheadertype           Header;
long                            filesize = 0;
msgbasetype             MsgBase;

        checkstack();


        if(directory_info.outgoing_msg[0] == '\x0' || directory_info.outgoing_msg[0] == ' ')
          return FALSE;

        tptr = strrchr((char *)fileName,'.');
        if(tptr) *tptr = '\x0';

        // Build full path/filename and see if the file exists
        sprintf(filenameBuf,"%s%s.MSG",directory_info.outgoing_msg,fileName);

        if(fileexist(filenameBuf) == 255)
                return FALSE;

        filesize = DTA.ff_fsize;

        // Open the *.MSG file
        if(dosfopen(filenameBuf,OPEN_RDWR|OPEN_DENYRDWR,&MSGfile)==-1)
          return FALSE;

        // read *.MSG header information
        memset(&MSGHdr,0,sizeof(MSGHdr));
        dosfread(&MSGHdr,sizeof(MSGHdr),&MSGfile );


        // Allocate a buffer just large enough to hold the message body
        uint msgsize = (unsigned)filesize - sizeof(MSGHdr)+1;

        char * messBuf = (char *)  malloc (msgsize);

        if(!messBuf)
        {
                dosfclose(&MSGfile);
                return FALSE;
        }

        memset(messBuf,0,msgsize);

        // Read the message body
        dosfread(messBuf,msgsize,&MSGfile);


        dosfclose(&MSGfile);

        //stripright(messBuf,' ');


        uint topnt=0,frmpnt=0;
        getPointAddr(messBuf,frmpnt,topnt);
        MSGHdr.Dest_Point = topnt;

        if(killFileSent(messBuf)) strcat(MSGHdr.To_User," +K");
        if(MSGHdr.Attribute&MSG_FILEATT)  handleAttach(messBuf);

        if(*messBuf == '\x0' || messBuf == NULL)
        {
           free(messBuf);
           return FALSE;
        }


        // Strip cr's and do word wrapping
        prepBodyForPCB(messBuf);


        // Prepare header DATE information
        maxstrcpy(day,strtok(MSGHdr.Date_Time," "),sizeof(day));
        maxstrcpy(mo,strtok(NULL," "),sizeof(mo));
        maxstrcpy(yr,strtok(NULL," "),sizeof(yr));
        replaceMo(mo);
        sprintf(Date,"%s-%s-%s",mo,day,yr);

        // Prepare header TIME information
        maxstrcpy(Time,MSGHdr.Date_Time+11,6);

        if(getINTLinfo(messBuf,destAddr,sizeof(destAddr),srcAddr,sizeof(srcAddr)) == FALSE)
        {
          if(frmpnt != 0) sprintf(destAddr,"%d:%d/%d.%d",(MSGHdr.Dest_Zone > 7000U ? PcbData.FidoDefaultZone : MSGHdr.Dest_Zone),
                                                                                                         MSGHdr.Dest_Net,MSGHdr.Dest_Node,
                                                                                                         frmpnt);
          else
          sprintf(destAddr,"%d:%d/%d", (MSGHdr.Dest_Zone > 7000U ? PcbData.FidoDefaultZone : MSGHdr.Dest_Zone),
                                                                   MSGHdr.Dest_Net,MSGHdr.Dest_Node);

          if(topnt != 0) sprintf(srcAddr,"%d:%d/%d.%d", (MSGHdr.Orig_Zone > 7000U ? PcbData.FidoDefaultZone : MSGHdr.Orig_Zone),
                                                                                                        MSGHdr.Orig_Net,MSGHdr.Orig_Node,
                                                                                                        topnt);
          else
          sprintf(srcAddr,"%d:%d/%d",(MSGHdr.Orig_Zone > 7000U ? PcbData.FidoDefaultZone : MSGHdr.Orig_Zone),
                                                                  MSGHdr.Orig_Net,MSGHdr.Orig_Node);
        }

        // Append FIDO address to TO: field
        if(strchr(MSGHdr.To_User,'@') == NULL)
        {
          sprintf(TOBuf,"%s@%s",MSGHdr.To_User,destAddr);
          maxstrcpy(MSGHdr.To_User,TOBuf,sizeof(MSGHdr.To_User));
        }
        if(strchr(MSGHdr.From_User,'@') == NULL)
        {
          sprintf(FromBuf,"%s@%s",MSGHdr.From_User,srcAddr);
          maxstrcpy(MSGHdr.From_User,FromBuf,sizeof(MSGHdr.From_User));
        }

        if(directory_info.outgoing_msg[0] == '\x0' || directory_info.outgoing_msg[0] == ' ')
        {
                free(messBuf);
                return FALSE;
        }

        // Get conference name associated with the NETMAIL tag
        if(getFidoArea("NETMAIL")==FALSE)
        {
          free(messBuf);
          return FALSE;
        }

        // Ge the conference record for the NETMAIL area
        getconfrecord(this_area.PCB_Conference,&confrec);

        if(openmessagebase(this_area.PCB_Conference,&confrec,&MsgBase,RDWRLOCK)==-1)
        {
          free(messBuf);
          return(FALSE);
        }

        if(MSGHdr.Attribute & MSG_CRASH) strcat(TOBuf," +C");
        if(MSGHdr.Attribute & MSG_HOLD)  strcat(TOBuf," +H");

        buildextheader(EXTHDR_FROM,FromBuf,HDR_FROM);
        buildextheader(EXTHDR_TO,TOBuf,HDR_TO);
        buildextheader(EXTHDR_SUBJECT,MSGHdr.Subject,HDR_SUBJ);

    // If the *.MSG file is too big (30k+) Then add it as a file attach
    if (filesize >= 30000)
        createAttach(this_area.PCB_Conference,filenameBuf,FALSE);

        char * nullit = strrchr(FromBuf,'@');
        if(nullit) *nullit = '\x0';

        nullit = strrchr(TOBuf,'@');
        if(nullit) *nullit = '\x0';

        // Convert the *.MSG header info to PCBoard format
        checkDateFormat(Date);
        checkTimeFormat(Time);

        // make sure it is not too big for PCB
        if(strlen(messBuf) >= 0x8000 - 0x100) messBuf[0x8000-0x100] = '\x0';
        makemsgheader(&Header,MSG_RCVR,strlen(messBuf),Date,Time,TOBuf,0,0,"",
                                ' ',FromBuf,MSGHdr.Subject,"",!NoEchoFlag);

        if(Header.Status == '*') MSGHdr.Attribute |= MSG_PRIVATE;
        // Save the message
        if(!isEmptyMessage(messBuf))
          savetomsgbase(this_area.PCB_Conference,&confrec,&MsgBase,&Header,messBuf,0,0,capMsgInfo);
        closemessagebase(&MsgBase);
        freehdrs();
        addUMWF(TOBuf,this_area.PCB_Conference);
        free(messBuf);
        return TRUE;

}


bool cFIDOMSG::MsgtoFile(long msgNum)
{
cDOSFILE          MSGfile;
char              filename[80];
char              lineBuf[81];
char              pointBuf[15];
long              num = 0;

char    *         line   = NULL;
char    *         nullIt = NULL;

        checkstack();
        if(preventFile) return FALSE;
         if(directory_info.outgoing_msg[0] == '\x0' || directory_info.outgoing_msg[0] == ' ')
           return FALSE;


  // Get conference name associated with the NETMAIL tag
  if(getFidoArea("NETMAIL")==FALSE)
        return FALSE;

  // Get the conference record for the NETMAIL area
  getconfrecord(this_area.PCB_Conference,&confrec);

  // Open the NETMAIL message base
  message.close();
  if(message.open(confrec.MsgFile) != 0) return FALSE;


  num = findHiMSG(directory_info.outgoing_msg);

  // Make sure the message exists
  if(message.readIdx(msgNum) > 0)
  {
        // Make sure the offset for this message is greater than 0
        if(message.idxRec.off <= 0)
          return(FALSE);

        // Read header for this message
        if(message.readHdr(message.idxRec.off)!=0)
          return(FALSE);
        message.msgHdr.fname[sizeof(message.msgHdr.fname)-1] = '\x0';
        message.msgHdr.fname[sizeof(message.msgHdr.tname)-1] = '\x0';
        proper(message.msgHdr.fname);
        proper(message.msgHdr.tname);
        // read message body
        if(message.readBody(0)!=0)
        {
          message.freeMsgBuf();
          return(FALSE);
        }
        message.msgBody[message.msgSize] = '\x0';

        if(strstr (message.msgHdr.fname,"SYSOP"))
          getSysopNam(message.msgHdr.fname);

        // Convert PCBoard message header to *.MSG message header
        //memset(&MSGHdr,0,sizeof(MSGHdr));
        msgheadertype hdr;
        memcpy(&hdr,&message.msgHdr,sizeof(hdr));
        if(fillMSGHdr(hdr) == FALSE)
          return FALSE;

        char hdr_buf[100];
        memset(hdr_buf,0,sizeof(hdr_buf));
        message.getExtHdrs(ExtHdrFunctions[EXTHDR_ATTACH],hdr_buf);
        if(hdr_buf[0] != '\x0')
        {
          MSGHdr.Attribute |= MSG_FILEATT;
          char * tptr = strchr(hdr_buf,' ');
          if(tptr) *tptr = '\x0';
          memset(MSGHdr.Subject,0,sizeof(MSGHdr.Subject));
          stripright(hdr_buf,' ');
          maxstrcpy(MSGHdr.Subject,hdr_buf,sizeof(MSGHdr.Subject));
        }
        MSGHdr.Attribute |= MSG_SENT;
        // Remove @ addressing from header.
        nullIt = strchr(MSGHdr.To_User,'@');
        if(nullIt) strset(nullIt,0);
        nullIt = strchr(MSGHdr.From_User,'@');
        if(nullIt) strset(nullIt,0);

        // Set MSG header attributes.
        if(hdr.Status == '*') MSGHdr.Attribute |= MSG_PRIVATE;
        MSGHdr.Attribute |= MSG_LOCAL;

        // Create filename and see if it already exists
        sprintf(filename,"%s%ld.MSG",directory_info.outgoing_msg,num);

        // Open/create the *.MSG file
        if(MSGfile.open(filename,OPEN_CREATE|OPEN_RDWR|OPEN_DENYRDWR)!=0)
          return FALSE;

        // Write the header info to the *.MSG file
        if(MSGfile.write(&MSGHdr,sizeof(MSGHdr))!=sizeof(MSGHdr))
        {
          MSGfile.close();
          return(FALSE);
        }


        // Write kludge lines

          sprintf(lineBuf,"\x01INTL %u:%u/%u %u:%u/%u \r",MSGHdr.Dest_Zone,MSGHdr.Dest_Net,
                          MSGHdr.Dest_Node,MSGHdr.Orig_Zone,MSGHdr.Orig_Net,MSGHdr.Orig_Node);

          MSGfile.puts(lineBuf);
          //dosfputs(lineBuf,&MSGfile);

        if(MSGHdr.Orig_Point != 0)
        {
          sprintf(pointBuf,"\x01""FMPT %u \r",MSGHdr.Orig_Point);
          MSGfile.puts(pointBuf);
          //dosfputs(pointBuf,&MSGfile);
        }

        if(MSGHdr.Dest_Point != 0)
        {
          sprintf(pointBuf,"\x01TOPT %u \r",MSGHdr.Dest_Point);
          MSGfile.puts(pointBuf);
          //dosfputs(pointBuf,&MSGfile);
        }

        // Write the message body to the *.MSG file.
        // **NOTE you must pass the DOSFILE structure with the file aleady OPEN!

        while( ( line = message.getLine() ) != NULL )
        {
                MSGfile.puts(line);
                //dosfputs(line,&MSGfile);
                MSGfile.write("\r",1);
        }
        MSGfile.write(&zero,1);

        MSGfile.close();
  }

        return TRUE;
}


// Reads PCBoard message body and write it to the *.MSG file
bool cFIDOMSG::writeBody(char * messBody,DOSFILE & fblk)
{
  long len = 0;

  checkstack();
  stripright(messBody,' ');
  len = strlen(messBody);

  for(int i=0;i<=len;i++)
  {
        if(messBody[i]==13 || messBody[i]==0x8D)
          messBody[i]=LineSeparator;
        else if(messBody[i]==10)
        {
          maxstrcpy(messBody+i,messBody+i+1,(unsigned)len-i);
          len--;
          i--;
        }
  }

  if(dosfwrite(messBody,(unsigned)len,&fblk)==-1)
        return FALSE;

  return TRUE;
}

bool cFIDOMSG::getFidoArea(char *areaname)
{
cAREAS           areas;
cAKAS            akas;
cORIGINS         origins;
NAREA_STRUCT rec,mtrec;
ORIGIN           org;
NADDRESS         addr;
char             nodestr[25];
unsigned int recnum=0;

        memset(&this_area,0,sizeof(AREA_STRUCT));
        memset(&rec,0,sizeof(rec));
        memset(&mtrec,0,sizeof(mtrec));
        recnum = areas.findRec(areaname);
        rec = areas.getRec(recnum);

        this_area.PCB_Conference = rec.ConfNum;
        maxstrcpy(this_area.Area_Name,rec.AreaTag,sizeof(this_area.Area_Name));
        this_area.HighAscii = rec.HighAscii;

        addr = akas.getRec(rec.AkaIndex);
        if(addr.point == 0) sprintf(nodestr,"%u:%u/%u",addr.zone,addr.net,addr.node);
        else sprintf(nodestr,"%u:%u/%u.%u",addr.zone,addr.net,addr.node,addr.point);
        maxstrcpy(this_area.default_aka,nodestr,sizeof(this_area.default_aka));
        org = origins.getRec(rec.OriginIndex);
        maxstrcpy(this_area.origin,org.origin,sizeof(this_area.origin));

        return(TRUE);
}

/*
void cFIDOMSG::processMessBody(char * buffer)
{

char *p,*ptr,*end,*tptr;
char oldchar;
int  i=0,len=0;
long tmplen=0;
bool sep = FALSE;


  stripright(body,' ');
  len=strlen(body);

  // Convert carraige returns (hard and soft) to PCBoard line seperators
  for(i=0;i<=len;i++)
  {
        // Replace CR and soft CR's with lineseparator
        if(body[i]==13 || body[i]==0x8D)
        {
          body[i]=LineSeparator;
          sep = TRUE;

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
        // strip line feed
        else if(body[i]==10)
        {
          memcpy(body+i,body+i+1,len-i);
          len--;
          i--;
        }
  }// for

  ptr=body;
  tptr=body;
  //end=body+strlen(body);
  end=body+len;

  // If no line separators were added, then add one at the end.
  if(!sep)
  {
        end++;
        *(end)=LineSeparator;
        *(end+1)=NULL;
  }

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
        else if(tptr>=end)      break;
        else
        {
          tptr++;
          ptr=tptr;
        }
  }//While

}

void  cFIDOMSG::updatePackInfo(const char * fname, PACKMSGNFO & pknfo) {
cDOSFILE        nfofile;
int             oflag;

  nfofile.setName(fname);
  nfofile.setExt(".MSG");
  if(fileexist((char*)nfofile.getName()) == 255)  oflag = OPEN_CREATE;
  else                                                                                    oflag = OPEN_APPEND;

  if(nfofile.open(OPEN_WRIT | oflag | OPEN_DENYNONE) != 0) return;
  nfofile.write(&pknfo,sizeof(pknfo));
  nfofile.close();
}

*/
static void replaceMo(char * mo)
{
        char * ptr = NULL;

        checkstack();
        ptr = strupr(mo);

        if(strcmp(ptr,"JAN") == 0) maxstrcpy(mo,"01",sizeof(mo));
        if(strcmp(ptr,"FEB") == 0) maxstrcpy(mo,"02",sizeof(mo));
        if(strcmp(ptr,"MAR") == 0) maxstrcpy(mo,"03",sizeof(mo));
        if(strcmp(ptr,"APR") == 0) maxstrcpy(mo,"04",sizeof(mo));
        if(strcmp(ptr,"MAY") == 0) maxstrcpy(mo,"05",sizeof(mo));
        if(strcmp(ptr,"JUN") == 0) maxstrcpy(mo,"06",sizeof(mo));
        if(strcmp(ptr,"JUL") == 0) maxstrcpy(mo,"07",sizeof(mo));
        if(strcmp(ptr,"AUG") == 0) maxstrcpy(mo,"08",sizeof(mo));
        if(strcmp(ptr,"SEP") == 0) maxstrcpy(mo,"09",sizeof(mo));
        if(strcmp(ptr,"OCT") == 0) maxstrcpy(mo,"10",sizeof(mo));
        if(strcmp(ptr,"NOV") == 0) maxstrcpy(mo,"11",sizeof(mo));
        if(strcmp(ptr,"DEC") == 0) maxstrcpy(mo,"12",sizeof(mo));
}


void LIBENTRY  capMsgInfo(unsigned short ConfNum, long MsgNum, long MsgOffset)
{
        checkstack();
        lastConfNum = ConfNum;
        lastMsgNum      = MsgNum;
        lastMsgOff      = MsgOffset;
}

// Searches MSG body for FLAGS KFS kludge
bool LIBENTRY  killFileSent(const char * msg)
{
char *  kl=NULL,*cr=NULL;
char    buf[100];
char    str[20];

  memset(buf,0,sizeof(buf));
  memset(str,0,sizeof(str));

  sprintf(str,"\x01%s","FLAGS");
  kl = strstr((char *)msg,str);
  if(!kl) return FALSE;
  cr = strchr(kl,'\r');

  if(!cr)  cr = (char*)msg + strlen((char*)msg);

  maxstrcpy(buf,kl,(size_t(cr-kl) > sizeof(buf) ?sizeof(buf) : size_t(cr-kl)+1));
  if(strstr(buf,"KFS") || strstr(buf,"TFS")) return TRUE;
  return FALSE;
}

#endif
