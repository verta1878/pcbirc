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


/*******************************************************************************
*
*                                                       PASSTHRU.CPP
*                               Copyright (C) 1995 Clark Development
*
********************************************************************************
*                                                        Author
*                                                       Stan Paulsen
*
* This module contains definitions for classes and functions related to the
* passthrough module for PCBOARD FIDO.
*
********************************************************************************/

#ifdef FIDO
// Borland headers
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

// PCBoard headers
#include <pcboard.h>
#include <users.h>
#include <tossmisc.h>

#include <dosclass.hpp>
#include <misc.h>
#include <structs.h>
#include <prototyp.h>
#include <data.hpp>
#include <passthru.hpp>

#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef FIDOUTIL
static void _NEAR_ LIBENTRY addrToNum(const char * addr, uint & zone, uint & net, uint & node, uint & point);
#endif

/******************************************************************************
*
*                                                        Definition for members of cPASSTHROUGH
*                                This class is used to manage passthrough messages
*
******************************************************************************/

/*******************************************************************************
* Member Function               : cPASSTHROUGH constructor
* Purpose        : construct passthrough object
* Parameters : none
* Returns        : none
********************************************************************************
* Assumptions    :This object will keep a file open for it's lifetime, so it should be short
* Pre  Conditions:un-initialized passthrough object
*
* Post Conditions:initialiazed object with message file open and ready to receive
*                                                               messages
* Comments                       : The message file contains messages in FIDO format as receive
*                                                                from up/downlinks. No packet headers, just message headers and
*                                                                message bodies
*
********************************************************************************/


cPASSTHROUGH::cPASSTHROUGH(void)
{
        cCONFIG                 C;
        DIRECTORIES D;

  err = FALSE;

        C.getDirs(D);

        // Build passthrough message filename
  sprintf(msgfile,"%s%s",D.passthrough,"PTMSG.DAT");

  if(fileexist(msgfile) != 255 &&        msg.open(msgfile,OPEN_RDWR | OPEN_DENYNONE) != 0) err = TRUE;
  else if(fileexist(msgfile) == 255 &&  msg.open(msgfile,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE) != 0) err = TRUE;
}

void LIBENTRY cPASSTHROUGH::removeFile(void)
{
  if(msg.isOpen()) msg.close();
  msg.unlink();
}


/*******************************************************************************
* Member Function               :addMsg
* Purpose        :add a single FIDO message to the passthrough file
* Parameters :null terminated character string containing FIDO type message
* Returns        :success status
********************************************************************************
* Assumptions    : msg is null terminated, file is open
* Pre  Conditions: open message file
*
* Post Conditions: 1 additional message added to message file
*
********************************************************************************/


bool LIBENTRY cPASSTHROUGH::addMsg(const FIDO_MESSAGE_HDR * hdr,const char * fmsg)
{
        char zero = '\x0';
  char           logBuf[65];

        if(err) return FALSE;
        if(PcbData.FidoLogLevel == 6)
        {
          sprintf(logBuf,"Writing message to passthru file %-.30s",findstartofname(msgfile));
          writeFidolog(logBuf,BLOCK);
        }
                msg.seek(0,SEEK_END);
        msg.write(&hdr,sizeof(FIDO_MESSAGE_HDR));
                msg.write(fmsg,strlen(fmsg));
                msg.write(&zero,1);
                return TRUE;
}

/*******************************************************************************
* Member Function               :getNextMsg
* Purpose        :retrieves a FIDO type message from the current location in the message file
* Parameters :msgbuf: an empty buffer to place message into
*                                               buflen: length of msgbuf
* Returns        :success status
********************************************************************************
* Assumptions    : msgbuf is large enough to hold entire message, file is located a beginning of next message in message file
* Pre  Conditions: empty message buffer
*
* Post Conditions: msgbuf filled with next message in message file
*
********************************************************************************/

bool LIBENTRY cPASSTHROUGH::getNextMsg(FIDO_MESSAGE_HDR * hdr,char * msgbuf,unsigned int buflen)
{
unsigned int bytesread=0;

        if(fileexist(msgfile) == 255) return FALSE;
        if( msg.read(hdr,sizeof(FIDO_MESSAGE_HDR)) != sizeof(FIDO_MESSAGE_HDR)) return FALSE;
  if( msg.getnln(msgbuf,buflen) != 0) return FALSE;
  bytesread = strlen(msgbuf);

        // If readbytes is the same as buflen, assume the message is larger than buflen
  // So finish reading the message to keep the file pointer on message boundries
  if(bytesread == buflen)
        {
                char tbuf[1024];
                memset(tbuf,0,sizeof(tbuf));
                while(msg.getnln(tbuf,sizeof(tbuf)) == sizeof(tbuf));
        }

        return TRUE;
}


/*******************************************************************************
* Member Function               :getAreaName
* Purpose        :searches fido message string for AREA: name and places it in parameter areaname
* Parameters :msg: Null terminated string containing FIDO message
*                                               areaname: null terminated string containing the area name to search for
* Returns        :success status
********************************************************************************
* Assumptions    : msg is null terminated
* Pre  Conditions: msg contains a valid FIDO message, areaname is ready to be filled
*
* Post Conditions: areaname filled with area name found in msg
*
********************************************************************************/

bool     LIBENTRY cPASSTHROUGH::getAreaName(const char * msg,char * areaname)
{
        char *nptr=NULL,*eptr=NULL;

        nptr = (char *) strstr(msg,"AREA:");
        if(!nptr)
                return FALSE;
        eptr = strchr(nptr,'\x0D');
        if(!eptr)
                return FALSE;

  //Place nptr past AREA:
  nptr+=5;
        maxstrcpy(areaname,nptr,size_t(eptr-nptr+1));
        return TRUE;
}

/*******************************************************************************
* Member Function               :getNextMsgForUser
* Purpose        :searches message file for the next message that needs to go to the current ~FIDO~ user being processed
* Parameters :msgbuf: An empty buffer to place the next message in
*                                               msgbufsize: number of bytes that can be put into msgbuf
*                                               user: character string with current ~FIDO~ user address
* Returns        :success status
********************************************************************************
* Assumptions    : none
* Pre  Conditions: msgbuf is ready to have a message copied into it
*
* Post Conditions: msgbuf is filled with next message, or no message was found
*
********************************************************************************/

unsigned int LIBENTRY cPASSTHROUGH::getNextMsgForUser(FUSERS & user,DOSFILE & file,cPTINFO & info,bool & sent)
{
char *           msgbuf=NULL;
uint             msgbufsize=0;
unsigned int actualMsgSize=0;
FIDO_MESSAGE_HDR hdr;
char     areaname[AREA_SIZE];

  // determine maximum buffer size
  #ifdef __OS2__
        msgbufsize = 32000U;
  #else
        msgbufsize = (coreleft() > 32000U ? 32000U : size_t(coreleft()));
  #endif

  // allocate buffer
  msgbuf = (char *) malloc(size_t(msgbufsize));

  if(!msgbuf)
  {
        writeFidolog("Could not allocate memory to process passthrus.",BLOCK);
        return 0;
  }

        while(TRUE)
        {
        memset(msgbuf,0,msgbufsize);

                // If there are no more messages then return FALSE
        if(!getNextMsg(&hdr,msgbuf,msgbufsize)) break;

                // If there isn't an area tag then move to the next message
                if(!getAreaName(msgbuf,areaname)) continue;

        actualMsgSize = strlen(msgbuf);

                // If the user is registered in this area then return the message
        if(info.userInArea(areaname,user))
                {
                // Check seenby's to see if this node needs to receive this message
                dosfwrite(&hdr,sizeof(hdr),&file);
                dosfwrite(msgbuf,actualMsgSize,&file);
                sent = TRUE;
                }
        }
  if(msgbuf) free(msgbuf);
  return 0;

}

/*______________________________________________________________________________

                                                                         Definition for members of cPTINFO

This class maintains passthough configuration data such as pasthrough area names
and user registration in passthough areas

 _____________________________________________________________________________*/

/*******************************************************************************
* Member Function               :cPTINFO
* Purpose        :Constructs cPTINFO object
* Parameters :None
* Returns        :None
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Un-initialized object
*
* Post Conditions: Initilaized object with data file open
*
* Comments                       : This object keeps 1 file opne thouhg ouot it's liftime. Keep
*                                                                it's lifetime short.
*
********************************************************************************/

cPTINFO::cPTINFO(void)
{
        cCONFIG                 C;
        DIRECTORIES D;
        err                      = FALSE;
        needtopack = FALSE;

                C.getDirs(D);

                // Build passthrough info filename
        sprintf(datafile,"%-.*s%-.13s",sizeof(D.passthrough)-14,D.passthrough,"PTARS.DAT");

        if(fileexist(datafile) != 255)
          dfile.open(datafile,OPEN_RDWR | OPEN_DENYNONE);
                else
                        create();

        if(!dfile.isOpen())
        {
           err = TRUE;
           return;
        }
                dfile.seek(0,SEEK_SET);
                if(dfile.read(&numusers,sizeof(numusers))!= sizeof(numusers)) reportErr();
                users = new FUSERS[numusers];
                urecs = new REGREC[numusers];

                if(!urecs || !users)
                        reportErr();
                else
                        dfile.read(users,sizeof(FUSERS)*numusers);
}

/*******************************************************************************
* Member Function               :~cPTINFO
* Purpose        :destructs cPTINFO object
* Parameters :None
* Returns        :None
********************************************************************************
* Assumptions    : none
* Pre  Conditions: initialized object
*
* Post Conditions: destroyed object with data file closed and memory freed
*
********************************************************************************/


cPTINFO::~cPTINFO(void)
{
         delete(users);
         delete(urecs);
         if(needtopack) ptPack();
         dfile.close();
}


/*******************************************************************************
* Member Function               :userInArea
* Purpose        :Determine if a ~FIDO~ user is registered in a passthough area
* Parameters :area: passthough area name
* Returns        :TRUE if user is registered, FLASE otherwise
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Un-determined registration status
*
* Post Conditions: Determined registration status
*
* Comments                       : This is the work horse of this object. The whole purpose for
*                                                                maintaining this data to to determine who should receive what
*                                                                passthrough areas.
*
********************************************************************************/

bool LIBENTRY cPTINFO::userInArea(const char * area,unsigned int usernum)
{
REGREC rec;
         if(err) return FALSE;

         if(!seekToArea(area))          return FALSE;
         if(!seekToUser(usernum)) return FALSE;

         dfile.read(&rec,sizeof(rec));
         return (rec.R && rec.S);
}
bool LIBENTRY cPTINFO::userInArea(const char * area,FUSERS &    user)
{
unsigned int i;

  if(err) return FALSE;
        if(!seekToArea(area)) return FALSE;
        readUrecs();
        for(i=0;i<numusers;i++)
                if(memcmp(&users[i],&user,sizeof(users[i])) == 0) return (urecs[i].R && urecs[i].S);

        return FALSE;
}


void LIBENTRY cPTINFO::putUserInArea(unsigned int usernum,const char * area)
{
REGREC rec;
        rec.R = 1;
        rec.S = 1;
        rec.A = 1;

        if(!seekToArea(area))    return;
        if(!seekToUser(usernum)) return;
        modifyRegRec(rec);
}

void LIBENTRY cPTINFO::putUserInArea(FUSERS & user,const char * area)
{
unsigned usernum=0;
        getUserIndex(user,usernum);
        putUserInArea(usernum,area);
}

void LIBENTRY cPTINFO::removeUserFromArea(FUSERS & user, const char * area)
{
REGREC rec;
unsigned int usernum=0;

        rec.R = 0;
        rec.S = 0;
        rec.A = 1;

        if(! seekToArea(area) )                                         return;
  if(! getUserIndex(user,usernum) ) return;

        // Usernum -1 so file pointer will be at beginning of userrec
        if(! seekToUser(usernum-1) )                     return;
        modifyRegRec(rec);

}

bool LIBENTRY cPTINFO::seekToUser(unsigned int usernum)
{
  if(err) return FALSE;
        if(usernum > numusers) return FALSE;
        dfile.seek(usernum*sizeof(REGREC),SEEK_CUR);
        return TRUE;
}

char *  LIBENTRY cPTINFO::seekToAreaNum(unsigned int areanum)
{
static char area[AREA_SIZE];

        if( areanum > numAreas()) return NULL;
        memset(area,0,sizeof(area));
        long headersize = headerSize();
        long areasize   = areaRecSize();
        long areaoffset = headersize + (areasize*(areanum-1));

  if(err) return NULL;
        dfile.seek(areaoffset,SEEK_SET);
        dfile.read(area,sizeof(area));
        return area;

}

/*******************************************************************************
* Member Function               :Create
* Purpose        :If the data file does not exist, this member will create a default file
* Parameters :None
* Returns        :N/A
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Non-existant data file
*
* Post Conditions: A default data file will all FIDo users present
*
********************************************************************************/

void LIBENTRY cPTINFO::create(void)
{
         char           indexfile[MAXFLEN],srchstr[7],user[25];
         unsigned int nusers = 0;
         cDOSFILE       ifile;
         IndexType      index;
         IndexType2 index2;
         bool                   BigNdx = FALSE;
         char                   ndxstr[100];
         FUSERS         fu;

         if(err) return;
         // write out 0 users for place holder
         if(dfile.open(datafile,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE) != 0)
         {
                 err = TRUE;
                 return;
         }
         dfile.seek(0,SEEK_SET);
         dfile.write(&nusers,sizeof(nusers));

         // start searching pcboard user index for ~FIDO~ users
         sprintf(indexfile,"%-.*sPCBNDX.Z",sizeof(indexfile)-14,PcbData.NdxLoc);
         if(fileexist(indexfile) == 255 || ifile.open(indexfile,OPEN_READ | OPEN_DENYNONE) != 0) return;

         strcpy(srchstr,"~FIDO~");

        // Check for /BIGNDX in PCB environment var
        maxstrcpy(ndxstr,getenv("PCB"),sizeof(ndxstr));
        strupr(ndxstr);
        if(strstr(ndxstr,"/BIGNDX")) BigNdx = TRUE;

        while(TRUE)
        {
        if(BigNdx)
        {

                if(ifile.read(&index2,sizeof(index2))!=sizeof(index2))  break;

                index2.UserName[sizeof(index2.UserName)-1]=NULL;
                stripright(index2.UserName,' ');
                maxstrcpy(user,index2.UserName,sizeof(user));
        }
        else
        {
        if( ifile.read(&index,sizeof(index)) != sizeof(index)) break;

                index.UserName[sizeof(index.UserName)-1]=NULL;
                stripright(index.UserName,' ');
                maxstrcpy(user,index.UserName,sizeof(user));
        }

        if(memcmp(srchstr,user,strlen(srchstr))==0)
        {

                addrToNum(user,fu.zone,fu.net,fu.node,fu.point);
                dfile.write(&fu,sizeof(fu));
                nusers++;
        }
        }
        dfile.seek(0,SEEK_SET);
        dfile.write(&nusers,sizeof(nusers));
        ifile.close();
}


/*******************************************************************************
* Member Function               :modifyRegRec
* Purpose        :If the data file does not exist, this member will create a default file
* Parameters :None
* Returns        :N/A
********************************************************************************
* Assumptions    : The file pointer is sitting at the REGREC that need to be modified
* Pre  Conditions: Non-existant data file
*
* Post Conditions: A default data file will all FIDo users present
*
********************************************************************************/

bool LIBENTRY cPTINFO::modifyRegRec(REGREC & newrec)
{
                if(err) return FALSE;

                dfile.write(&newrec,sizeof(newrec));
                return TRUE;
}

#ifdef PCBSETUP
/*******************************************************************************
* Member Function               :removeArea
* Purpose        :Zeros out an area record
* Parameters :area: Nul terminated string containing area tag to search for and remove
* Returns        :N/A
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Data file with or without area record
*
* Post Conditions: Data file without area record
*
********************************************************************************/

void LIBENTRY cPTINFO::removeArea(const char * area)
{
char nularea[AREA_SIZE];
                if(err) return;
                memset(nularea,0,sizeof(nularea));
                if(seekToArea(area))
                {
                        //seektoarea puts filepointer at end of user record.
                        dfile.seek(-AREA_SIZE,SEEK_CUR);
                        dfile.write(nularea,sizeof(nularea));
                        dfile.seek(-AREA_SIZE,SEEK_CUR);
                        needtopack = TRUE;
                }
        ptPack();
}




#endif  //ifdef PCBSETUP


/*******************************************************************************
* Member Function               :removeUser
* Purpose        :Zeros out a user record from the user list as well as all the registration records for that user
* Parameters :user: User address to search and destroy
* Returns        :N/A
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Data file with or without user record
*
* Post Conditions: Data file without user record and registration records
*
********************************************************************************/

void LIBENTRY cPTINFO::removeUser(FUSERS & user)
{
                char                             areaname[AREA_SIZE];
                unsigned int usernum=0;

                for(usernum=0;usernum<numusers;usernum++)
           if(memcmp(&users[usernum],&user,sizeof(users[usernum])) == 0) break;

                if(usernum == numusers) return;

                needtopack = TRUE;
                // remove user record from main user list
                memset(&users[usernum],0,sizeof(users[usernum]));

        writeUsers();
                // seek to start of areas.
                dfile.seek(headerSize(),SEEK_SET);

                // Find user entry for each area and nul it out
                while(dfile.read(areaname,sizeof(areaname)) == sizeof(areaname) )
                {
                         readUrecs();
                         memset(&urecs[usernum],0,sizeof(urecs[usernum])); // Zero out all bits in the structure
                         dfile.seek(-(numusers*sizeof(REGREC)),SEEK_CUR);
                         writeUrecs();
                }
}
/*******************************************************************************
* Member Function               :pack
* Purpose        :removes zeroes out user and registration records from data file
* Parameters :none
* Returns        :N/A
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Data file with or without zeroed out records
*
* Post Conditions: Data file without zeroed out records
*
********************************************************************************/

void LIBENTRY cPTINFO::ptPack(void)
{
        unsigned int i=0,newnum=numusers;
        char                             area[AREA_SIZE];
        cDOSFILE                 tmp;
  char             srcf[MAXFLEN],desf[MAXFLEN];

                if(err) return;

        memset(area,0,sizeof(area));
                dfile.seek(sizeof(numusers),SEEK_SET);

                // Write out user list excluding blank records
        tmp.setName("ptmp.tmp");
        tmp.unlink();
        if(tmp.open(OPEN_RDWR | OPEN_DENYNONE | OPEN_CREATE) != 0) return;
                tmp.write(&newnum,sizeof(newnum));

        readUsers();
                for(i=0;i<numusers;i++)
                {
                if(users[i].zone != 0 && users[i].net != 0) tmp.write(&users[i],sizeof(users[i]));
                                else    newnum--;
                }



                // Start reading area records
        while(dfile.read(area,AREA_SIZE) == AREA_SIZE)
                {
                                // If the record is nulled out then move on
                if(area[0]== '\x0')
                                {
                                        readUrecs();
                                        continue;
                                }
                tmp.write(area,sizeof(area));

                                // Read all ureg records for this area
                                readUrecs();

                                // For each of the urecs
                                for(i=0;i<numusers;i++)
                                {
                                        // If this urec hasn't been nulled out
                                        if(!urecs[i].A) continue;
                                        else tmp.write(&urecs[i],sizeof(REGREC));

                                }
          memset(area,0,sizeof(area));
                }
        dfile.close();

                tmp.seek(0,SEEK_SET);
                tmp.write(&newnum,sizeof(newnum));
        tmp.close();
        dfile.unlink();
        maxstrcpy(srcf,(char*)tmp.getName(),sizeof(srcf));
        maxstrcpy(desf,(char*)dfile.getName(),sizeof(desf));

        #ifdef PCBSETUP
          movefile(srcf,desf);
        #else
          pcbmovefile(srcf,desf);
        #endif

        if(dfile.open(OPEN_RDWR | OPEN_DENYNONE) != 0) err = TRUE;
}


/*******************************************************************************
* Member Function               :seekToArea
* Purpose        :Searches for the area record specified and places the file pointer at the beginning of regoistrasti0n records for that area
* Parameters :areaname: null terminated areaname
* Returns        :TRUE if area was found, FALSE if not found
********************************************************************************
* Assumptions    : none
* Pre  Conditions: Data file with file pointer who knows wheres
*
* Post Conditions: Data file with file pointer at the beginning of registration records for that area
*
********************************************************************************/

bool LIBENTRY cPTINFO::seekToArea(const char * areaname)
{
        char                             tmparea[AREA_SIZE];

                if(err) return FALSE;

                // Seek to the first area record
                dfile.seek(sizeof(numusers)+(numusers*sizeof(FUSERS)),SEEK_SET);

                while(dfile.read(tmparea,sizeof(tmparea)) == sizeof(tmparea))
                {
          if( strcmp(tmparea,areaname) == 0) return TRUE;
                        dfile.seek(numusers*sizeof(REGREC),SEEK_CUR);
                }
                return FALSE;
}

/*******************************************************************************
* Member Function               :readUsers
* Purpose        :Reads in entire user list from data file
* Parameters :none
* Returns        :N/A
********************************************************************************
* Assumptions    : buffer to hold data has been allocated
* Pre  Conditions: buffer filled with who knows what
*
* Post Conditions: buffer filled with FUSERS structures
*
********************************************************************************/

void LIBENTRY cPTINFO::readUsers(void)
{
        if(err) return;
        dfile.seek(sizeof(numusers),SEEK_SET);

        if(users) dfile.read(users,sizeof(FUSERS)*numusers);
        else                    reportErr();
}

/*******************************************************************************
* Member Function               :writeUsers
* Purpose        :Updates datafile with current user list
* Parameters :none
* Returns        :N/A
********************************************************************************
* Assumptions    : buffer has valid user records
* Pre  Conditions: out of synch datafile user list
*
* Post Conditions: cuirrent user list in data file
*
********************************************************************************/

void LIBENTRY cPTINFO::writeUsers(void)
{
        dfile.seek(sizeof(numusers),SEEK_SET);

        if(users) dfile.write(users,sizeof(FUSERS)*numusers);
        else                    reportErr();
}


/*******************************************************************************
* Member Function               :readUrecs
* Purpose        :Reads in all registration records for a given area record
* Parameters :none
* Returns        :N/A
********************************************************************************
* Assumptions    : File pointer is at the correct location. i.i; the beginning of a registration record, Also buffer has been allocated
* Pre  Conditions: urecs filled with who knows what
*
* Post Conditions: urecs filled with registration records for this area
*
********************************************************************************/

void LIBENTRY cPTINFO::readUrecs(void)
{
        if(urecs) dfile.read(urecs,sizeof(REGREC)*numusers);
        else                    reportErr();
}

/*******************************************************************************
* Member Function               :writeUrecs
* Purpose        :Writes out all registration records for a given area record
* Parameters :none
* Returns        :N/A
********************************************************************************
* Assumptions    : File pointer is at the correct location. i.i; the beginning of a registration record, Also buffer has been allocated
* Pre  Conditions: urecs filled with registration records
*
* Post Conditions: data file updated with current registration records
*
********************************************************************************/

void LIBENTRY cPTINFO::writeUrecs(void)
{
        if(urecs) dfile.write(urecs,sizeof(REGREC)*numusers);
        else                    reportErr();
}

/*******************************************************************************
* Member Function               :reportErr
* Purpose        :Writes an error message to caller log
* Parameters :none
* Returns        :N/A
********************************************************************************
* Assumptions    : None
* Pre  Conditions: Un reported error condition
*
* Post Conditions: reported error condition
*
********************************************************************************/

void LIBENTRY cPTINFO::reportErr(void)
{
                err = TRUE;
                //writeFidolog("An error occured while processing pass-throughs.",BLOCK);
}

/*******************************************************************************
* Member Function               :getUserIndex
* Purpose        :retrieves the user number, beginning with 0, from the user list currently in memory
* Parameters :user: Which user to look up
* Returns        :User number (remember it starts with 0)
********************************************************************************
* Assumptions    : None
* Pre  Conditions: Un known user number
*
* Post Conditions: Known user number
*
********************************************************************************/

bool LIBENTRY cPTINFO::getUserIndex(FUSERS & user,unsigned int & usernum)
{
         unsigned int i=0;

                if(err) return FALSE;
                for(i=0;i<numusers;i++)
                {
                         if(memcmp(&users[i],&user,sizeof(users[i])) == 0)
                         {
                                 usernum =      i;
                                 return TRUE;
                         }
                }
                return FALSE;
}
char *  LIBENTRY cPTINFO::getUser(unsigned int usernum)
{
        FUSERS rec;
        static char addr[25];

        if(usernum >= numusers) return NULL;
        else                                                                     memcpy(&rec,&users[usernum],sizeof(rec));

        if(rec.point == 0)sprintf(addr,"%u:%u/%u",rec.zone,rec.net,rec.node);
        else                                                    sprintf(addr,"%u:%u/%u.%u",rec.zone,rec.net,rec.node,rec.point);

        return addr;
}

unsigned int LIBENTRY cPTINFO::numAreas(void)
{
        long filesize = dfile.size();
        unsigned int headersize = headerSize();
        unsigned int areasize = areaRecSize();

        return  (unsigned) ( (filesize - (long) headersize) / areasize);
}

unsigned int LIBENTRY cPTINFO::headerSize(void)
{
        return ( sizeof(numusers) + (numusers*sizeof(FUSERS)) );
}

unsigned int LIBENTRY cPTINFO::areaRecSize(void)
{
        return AREA_SIZE + (numusers*sizeof(REGREC));
}

unsigned int LIBENTRY cPTINFO::countUsersInArea(const char * area)
{
unsigned int cnt=0;

        seekToArea(area);
        readUrecs();
        for(int i=0;i<numusers;i++)
                if(urecs[i].R != 0) cnt++;

        return cnt;
}

void LIBENTRY cPTINFO::addArea(const char * area)
{
 REGREC                         rec;
 char                           arec[AREA_SIZE];
 char     *   tp;
 unsigned int i=0;


        memset(&rec,0,sizeof(rec));
        rec.R=0;
        rec.S=0;
        rec.A=1;

        memset(arec,0,sizeof(arec));
        maxstrcpy(arec,(char *)area,sizeof(arec));
  tp = arec;
  while(*tp != ' ' && *tp != '\x0') ++tp;
  *tp = '\x0';
        if(seekToArea(area) == TRUE) return;
        dfile.seek(0,SEEK_END);
        dfile.write(arec,sizeof(arec));
        for(i=0;i<numusers;i++) dfile.write(&rec,sizeof(rec));
}

#ifdef FIDOUTIL
static void _NEAR_ LIBENTRY addrToNum(const char * addr, uint & zone, uint & net, uint & node, uint & point)
{
char     tBuf[25];
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


void LIBENTRY cPTINFO::updateUsers(void)
{
char       srchstr[30];
char       indexfile[MAXFLEN];
char       ndxstr[MAXFLEN];
char       user[25];
uint       numusers = 0;
cDOSFILE   ifile;
IndexType  index;
IndexType2 index2;
bool       BigNdx = FALSE;
FUSERS     fu;
cPTINFO    PT;


         // start searching pcboard user index for ~FIDO~ users
        sprintf(indexfile,"%-.*sPCBNDX.Z",sizeof(indexfile)-14,PcbData.NdxLoc);
        if(fileexist(indexfile) == 255 || ifile.open(indexfile,OPEN_READ | OPEN_DENYNONE) < 0) return;

        strcpy(srchstr,"~FIDO~");

        // Check for /BIGNDX in PCB environment var
        maxstrcpy(ndxstr,getenv("PCB"),sizeof(ndxstr));
        strupr(ndxstr);
        if(strstr(ndxstr,"/BIGNDX")) BigNdx = TRUE;

        while(TRUE)
        {
        if(BigNdx)
        {

          if(ifile.read(&index2,sizeof(index2))!=sizeof(index2))  break;

          index2.UserName[sizeof(index2.UserName)-1]=NULL;
          stripright(index2.UserName,' ');
          maxstrcpy(user,index2.UserName,sizeof(user));
        }
        else
        {
          if(ifile.read(&index,sizeof(index))!=sizeof(index)) break;
          index.UserName[sizeof(index.UserName)-1]=NULL;
          stripright(index.UserName,' ');
          maxstrcpy(user,index.UserName,sizeof(user));
        }

        // Is this a fido user?
        if(memcmp(srchstr,user,strlen(srchstr))==0)
        {
        uint i=0;
        bool found=FALSE;

          addrToNum(user,fu.zone,fu.net,fu.node,fu.point);
          // is the user already in the PTARS.DAT file?
          numusers = getUserCount();
          for(i=0;i<numusers;i++)
          {
                if(memcmp(&fu,&users[i],sizeof(fu)) == 0) found = TRUE;
          }
          if(!found) addUser(fu);
          found = FALSE;
        }
  }// while
}


void LIBENTRY cPTINFO::delOldUsers(void)
{

char       indexfile[MAXFLEN];
char       ndxstr[MAXFLEN],puser[35];
char       user[35];
cDOSFILE   ifile;
IndexType  index;
IndexType2 index2;
bool       BigNdx = FALSE;
FUSERS     fu;
uint       numusers = getUserCount();
uint       i;
bool       found = FALSE;

         // start searching pcboard user index for ~FIDO~ users
  sprintf(indexfile,"%-.*sPCBNDX.Z",sizeof(indexfile)-14,PcbData.NdxLoc);
  if(fileexist(indexfile) == 255 || ifile.open(indexfile,OPEN_READ | OPEN_DENYNONE) < 0) return;


        // Check for /BIGNDX in PCB environment var
        maxstrcpy(ndxstr,getenv("PCB"),sizeof(ndxstr));
        strupr(ndxstr);
        if(strstr(ndxstr,"/BIGNDX")) BigNdx = TRUE;

  for(i=0;i<numusers;i++)
  {

   // get a user from PTARS
   strcpy(puser,"~FIDO~");
   strcat(puser,getUser(i));
   addrToNum(puser,fu.zone,fu.net,fu.node,fu.point);
   while(TRUE)
   {
        if(BigNdx)
        {

          if(ifile.read(&index2,sizeof(index2))!=sizeof(index2))  break;

          index2.UserName[sizeof(index2.UserName)-1]=NULL;
          stripright(index2.UserName,' ');
          maxstrcpy(user,index2.UserName,sizeof(user));
        }
        else
        {
          if(ifile.read(&index,sizeof(index))!=sizeof(index)) break;
          index.UserName[sizeof(index.UserName)-1]=NULL;
          stripright(index.UserName,' ');
          maxstrcpy(user,index.UserName,sizeof(user));
        }

        // Is this a fido user?
        if(memcmp(puser,user,strlen(puser))==0)
        {
          found = TRUE;
          break;
        }
   }// while
  if(!found) removeUser(fu);
  found = FALSE;
  }
}


void LIBENTRY cPTINFO::addUser(FUSERS & user)
{
cDOSFILE  tmp;
REGREC    dregrec,tregrec;
FUSERS    tuser;
char      area[AREA_SIZE];
uint      bnumusers,numareas,i;

        memset(&dregrec,0,sizeof(dregrec));
        dregrec.A = 1;
        tmp.setName(dfile.getName());
        tmp.setExt(".TMP");
        if(tmp.open(OPEN_WRIT | OPEN_DENYNONE) != 0) return;

        numareas = numAreas();
        dfile.seek(0,SEEK_SET);
        if(dfile.read(&bnumusers,sizeof(bnumusers)) != sizeof(bnumusers))
        {
          tmp.close();
          return;
        }
        bnumusers++;
        tmp.write(&bnumusers,sizeof(bnumusers));
        for(i = 0;i<bnumusers-1;i++)
        {
          dfile.read(&tuser,sizeof(tuser));
          tmp.write(&tuser,sizeof(tuser));
        }
        tmp.write(&user,sizeof(user));

        for(i=0;i<numareas;i++)
        {
          dfile.read(area,sizeof(area));
          tmp.write(area,sizeof(area));
          for(uint j = 0;j<bnumusers-1;j++)
          {
                dfile.read(&tregrec,sizeof(tregrec));
                tmp.write(&tregrec,sizeof(tregrec));
          }
          tmp.write(&dregrec,sizeof(dregrec));
        }

        tmp.close();
        dfile.close();
        #ifdef PCBSETUP
         movefile((char*)tmp.getName(),(char*)dfile.getName());
        #else
         pcbmovefile((char*)tmp.getName(),(char*)dfile.getName());
        #endif
        tmp.unlink();
        if(dfile.open(OPEN_RDWR | OPEN_DENYNONE) != 0) err = TRUE;
        numusers = bnumusers;
}
#endif
#endif
