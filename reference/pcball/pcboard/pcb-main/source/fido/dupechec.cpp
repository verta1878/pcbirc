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
*                             PCBoard FIDO
*                           Module DUPECHEK.CPP
*
*                         Written by Stan Paulsen
*                            Copyright (C) 1995
*                            Clark Development
*
*****************************************************************************
*
*       Functions to perform Fido Duplicate Message Checking on import
*
*****************************************************************************/
#ifdef FIDO
#include <data.hpp>
#include <dupechec.hpp>
#include <prototyp.h>
#include <pcbmsgs.hpp>
#include <msgstub.hpp>

#ifdef DEBUG
#include <memcheck.h>
#endif

/*******************************************************************************
* Member Function     : cDUPECHECK::checkPath
* Purpose    : Check message path line for existance of node
* Parameters : msg-     Fido style message body
*            : address- Address to find in path line
* Returns    : TRUE if address is found in path line, FALSE if not
********************************************************************************
* Assumptions    :msg is carraige return delimited Fido format. Also carraige return ends path line
* Pre  Conditions:Unknown if address exists in path
*
* Post Conditions:Known if address exists in path line
*
********************************************************************************/

bool LIBENTRY cDUPECHECK::checkPath(const char * msg, uint orgzone)
{
uint  i=0;
const int sz = 200;
char  path[sz];
char  ne[15],no[15];
char *ptr=NULL,*end=NULL;


  // Find the PATH string in the message
  ptr = strstr((char *)msg,"\x01PATH:");  // !!!The place where this is called the line separator will still be an ascii CR!!!
  if(!ptr) return FALSE;

  // Copy the path string into a buffer
  while(*ptr != CR && *ptr != '\x0' && i < sz)
  {
    path[i++] = *ptr;
    ++ptr;
  }
  i=0;
  while(TRUE)
  {
  cAKAS    akas;
  NADDRESS aka;

    i++;
    aka = akas.getRec(i);
    if(aka.zone == 0 && aka.net == 0) break;
    if(aka.zone != orgzone) continue;

    // Break the address up into components and build net & node strings as they would apper in path line
    sprintf(ne,"%u/",aka.net);
    sprintf(no,"/%u ",aka.node);

    // Now isolate the part of the path that applies to this net
    ptr = strstr(path,ne);

    if(!ptr) continue;

    // If the net was found in the path, isolate it, if not return FALSE;
    end = ptr;

    //walk backwards to the space preseeding the next NET entry
    if(end)
    {
      while(*end != ' ' && *end != '\x0') ++end;
      if(*end != '\x0') *(++end) = '\x0';
    }
    if(strstr(ptr,no)) return TRUE;
  }
  return FALSE;
}


/*******************************************************************************
* Member Function     : cDUPECHECK::checkID
* Purpose    : check MSGID database for current conference for existance of MSGID
* Parameters : msg    : Fido style message body
               msgFile: The conference message base file
* Returns    : TRUE if the msgid in the message exists in the database
********************************************************************************
* Assumptions    :msg is a carraige return delimeted fido style message body
* Pre  Conditions:Unknown if MSGID exists in database
*
* Post Conditions:Known if MSGID exists in database.
*
********************************************************************************/

bool LIBENTRY cDUPECHECK::checkID(const char * msg,pcbconftype * conf)
{

char *idptr=NULL,*crptr=NULL;

char thisaddr[35];
char ch;
long goback=PcbData.FidoNumMsgsToTrack;
long lonum = 0;


  // Find the MSGID in the message we are importing. Copy the address portion
  // Into the buffer, then get the serial number
  idptr = strstr((char*)msg,"\x0D\x01MSGID:");
  if(!idptr) return FALSE;
  idptr+=8;

  crptr = idptr;
  while(*crptr != '\r' && *crptr != '\x0') crptr++;
  if(*crptr == '\x0') return FALSE;
  ch = *crptr;
  *crptr = '\x0';
  maxstrcpy(thisaddr,idptr,sizeof(thisaddr));

  *crptr = ch;

  if(conf->Name[0] == '\x0'     ||
     conf->MsgFile[0]  == '\x0' ||
     message.open(conf->MsgFile) != 0 ) return FALSE;


  long hinum=0;
  // Get conference high message number
  hinum=message.highMsgNum();
  lonum = message.lowMsgNum();

  if(hinum == 0)
  {
     message.close();
     return FALSE;
  }

  goback = (PcbData.FidoNumMsgsToTrack > (hinum-lonum) ? lonum : PcbData.FidoNumMsgsToTrack);
  if(PcbData.FidoNumMsgsToTrack == -1) goback = lonum;

  // Now loop through all the message in this message base and look for
  // duplicate id's.

  for(long int i = goback;i<=hinum;i++)
  {
     if(message.readIdx(i) == i)
     {
       if(message.readHdr(message.idxRec.off)!=0)        continue;
       if(message.readBody(0)!=0)                        continue;
       message.msgBody[message.msgSize] = '\x0';

       // If the serial number is not in this message, go to the next one
       if( (idptr = strstr(message.msgBody,thisaddr) ) == NULL ) continue;
       message.close();
       return TRUE;
     }
  }
  message.close();
  return FALSE;
}
#endif
