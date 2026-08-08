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


#include <dos.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pcbtools.h"

#ifdef _MSC_VER
  #include <borland.h>   /* this uses #define to change FA_ARCH to _A_ARCH */
  #include <direct.h>
#else
  #include <dir.h>
#endif

#ifdef DEBUG
#include <memcheck.h>
#endif


int LIBENTRY copyfiletomessage(char           *To,
                               char           *From,
                               char           *Subject,
                               char            MsgStatus,
                               bool            EchoFlag,
                               unsigned short  ConfNum,
                               char           *SourceFile,
                               char           *FileToAttach,
                               void (LIBENTRY *showsaving)(unsigned short,long,long)) {

  int             RetVal;
  unsigned short  BytesRead;
  unsigned short  BytesInBuf;
  unsigned short  BufSize;
  char           *p;
  char           *Body;
  char           *Name;
  long            ToRecNum;
  char            Time[6];
  char            Date[9];
  char            NewPath[66];
  char            AttachName[66];
  char            Desc[80];
  #if defined(__TURBOC__) && __TURBOC__ < 0x300
  struct ffblk    FileInfo;
  #else
  struct find_t   FileInfo;
  #endif
  DOSFILE         File;
  msgheadertype   Header;
  msgbasetype     MsgBase;
  pcbconftype     Conf;

  opencnames();          /* writing a message needs to have the */
  openusersfile();       /* cnames, users and users.inf files open */
  openusersinffile();

  if (dosfopen(SourceFile,OPEN_READ|OPEN_DENYNONE,&File) == -1) {
    RetVal = -1;   /* -1 = unable to open source file */
    goto exit;
  }

  /* BufSize is the size of the file to be read in, rounded up to the    */
  /* nearest full block of 128 characters because the message body needs */
  /* to be an even block of 128 bytes                                    */
  BufSize = (unsigned short) (((dosfseek(&File,0,SEEK_END)+127)/128)*128);
  dosrewind(&File);

  /* allocate the memory for the body of the message */
  if ((Body = (char *) malloc(BufSize)) == NULL) {
    dosfclose(&File);
    RetVal = -2;   /* -2 = unable to allocate memory to hold message */
    goto exit;
  }

  /* Read in the body of the message, putting a line separator in between */
  /* each text line, and keeping track of how many bytes we have read in  */
  /* so that we know how much room is left in the buffer.                 */
  BytesInBuf = 0;
  for (p = Body; dosfgets(p,BufSize-BytesInBuf,&File) != -1; ) {
    BytesRead = (unsigned short) strlen(p);       // find out how much we read in
    p += BytesRead;                               // move buf pointer to the end
    *p++ = LineSeparator;                         // add the line separator
    BytesInBuf += (unsigned short) (BytesRead+1); // count the bytes read+separator

    /* make sure we don't try to read in more bytes than we can hold */
    if (BufSize - BytesInBuf <= 0)
      break;
  }

  /* pad the rest of the message body with spaces to the end */
  if (BufSize - BytesInBuf > 0)
    memset(p,' ',BufSize-BytesInBuf);

  dosfclose(&File);  /* we're done with the text file, close it now */

  getconfrecord(ConfNum,&Conf);  /* get the conference information */

  /* if TO: is longer than 25 characters, then build TO extended header */
  if (strlen(To) > 25) {
    buildextheader(EXTHDR_TO,To,HDR_TO);
    /* if the TO name is longer than a single extended header will hold */
    /* then create the TO2 header with the rest of the name.            */
    if (strlen(To) > EXTDESCLEN)
      buildextheader(EXTHDR_TO2,&To[EXTDESCLEN],HDR_TO);
    /* if we use an extended header, then in general we set the main TO name */
    /* to blanks, unless the long name is something like an internet address */
    /* and the short name is the user-id on the BBS.  We'll set it to blanks */
    To[0] = 0;
  }

  /* if FROM: is longer than 25 characters, then build FROM extended header */
  if (strlen(From) > 25) {
    buildextheader(EXTHDR_FROM,From,HDR_FROM);
    /* if the FROM name is longer than a single extended header will hold */
    /* then create the FROM2 header with the rest of the name.            */
    if (strlen(From) > EXTDESCLEN)
      buildextheader(EXTHDR_FROM2,&From[EXTDESCLEN],HDR_FROM);
    /* if we use an extended header, then in general we set the FROM name    */
    /* to blanks, unless the long name is something like an internet address */
    /* and the short name is the user-id on the BBS.  We'll set it to blanks */
    From[0] = 0;
  }

  /* If SUBJECT is longer than 25 characters, then build SUBJECT extended */
  /* header, but do *not* blank out the short subject header.             */
  if (strlen(Subject) > 25)
    buildextheader(EXTHDR_SUBJECT,Subject,HDR_SUBJ);

  /* if a file attachment was specified, then we need to make sure that the */
  /* file really exists.  If it does, then we need to copy the file to the  */
  /* attachment directory and then make the filename unique so that only    */
  /* one message points to the file.  Then we build the ATTACH extended     */
  /* header with the real filename, file size, and unique name information  */
  if (FileToAttach != NULL && *FileToAttach > 0) {
    #if defined(__TURBOC__) && __TURBOC__ < 0x300
    if (findfirst(FileToAttach,&FileInfo,FA_ARCH) == 0) {
    #else
    if (_dos_findfirst(FileToAttach,FA_ARCH,&FileInfo) == 0) {
    #endif
      /* Point name to the "filename", ignoring drive and path info */
      Name = findstartofname(FileToAttach);
      /* Create a fully qualified attach spec (location+filename) */
      buildstr(AttachName,Conf.AttachLoc,Name,NULL);
      /* copy the file to the attachment location */
      if (copyfile(FileToAttach,AttachName,FALSE) == 0) {
        /* make the attach name unique, get unique name in NewPath */
        makefilenameunique(NewPath,AttachName);
        /* build the attachment extended header */
        #if defined(__TURBOC__) && __TURBOC__ < 0x300
        sprintf(Desc,"%s (%ld) %s",Name,FileInfo.ff_fsize,findstartofname(NewPath));
        #else
        sprintf(Desc,"%s (%ld) %s",Name,FileInfo.size,findstartofname(NewPath));
        #endif
        buildextheader(EXTHDR_ATTACH,Desc,HDR_FILE);
      }
    }
  }

  /* Create the message header, picking up the date and time from DOS via */
  /* the datestr() and timestr2() functions, and pass the rest of the     */
  /* information in according to what was passed this function.           */

  makemsgheader(&Header,
                MsgStatus,
                BufSize,
                datestr(Date),
                timestr2(Time),
                To,
                0,                /* Refer Num */
                0,                /* READ date */
                "",               /* READ time */
                ' ',              /* READ status */
                From,
                Subject,
                "",               /* Password  */
                EchoFlag);

  /* open the message base and lock it so that we can write to it */

  if (openmessagebase(ConfNum,&Conf,&MsgBase,RDWRLOCK) == -1) {
    freehdrs();    /* free the extended headers        */
    free(Body);    /* free the body of the message     */
    RetVal = -3;   /* -3 = unable to open message base */
    goto exit;
  }

  /* write the message into the message base, call showsaving() to show */
  /* that the message is being written.                                 */

  RetVal = savetomsgbase(ConfNum,&Conf,&MsgBase,&Header,Body,0,0,showsaving);

  closemessagebase(&MsgBase);  /* close and unlock the message base */

  freehdrs();    /* free the extended headers    */
  free(Body);    /* free the body of the message */

  /* if RetVal == -1, then we were unable to save the message, but we'll */
  /* use a value of -4 to indicate the error, so change RetVal here.     */

  if (RetVal == -1)  {
    RetVal = -4;   /* -4 = unable to store message */
    goto exit;
  }

  /* See if we can find the user record based on the TO name we were given. */
  /* If so, store the mail waiting flag in the user record.                 */

  if ((ToRecNum = finduser(To)) != -1)
    putmessagewaiting(ConfNum,ToRecNum);

exit:
  closeusersinffile();
  closeusersfile();
  closecnames();

  /* return error status:  0 = no error */
  /* -1 = unable to open source file */
  /* -2 = unable to allocate memory to hold message */
  /* -3 = unable to open message base */
  /* -4 = unable to store message */
  return(RetVal);
}


/* define TEST to create a sample program to test the above functionality */

#ifdef TEST
  /* this is just a function to show LOCALLY that a message is being saved */
  /* to show the message remotely you might use sprintf() and println()    */
  void LIBENTRY showsave(unsigned short ConfNum, long MsgNum, long MsgOffset) {
    printf("Message #%ld saved in Conference #%u\n",MsgNum,ConfNum);
  }

  void main(void) {
    /* as a utility, initdoor() is not needed, use pcbinit() instead */
    pcbinit();

    copyfiletomessage("JOHN DOE",
                      "JANE DOE",
                      "THIS IS A TEST MESSAGE WRITTEN BY THE TOOLKIT",
                      MSG_PBLC,
                      FALSE,
                      0,
                      "C:\\AUTOEXEC.BAT",
                      "C:\\CONFIG.SYS",
                      showsave);
  }
#endif
