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


#include <string.h>
#include <pcbtools.h>

/* This is a small sample program which is NOT set up as a door.  It   */
/* is a simple utility for copying either binary or text files using   */
/* the stream I/O functions in the PCBoard DOOR Developer's Toolkit.   */
/*                                                                     */
/* For comparison purposes, COPYBIN1 should be faster than COPYTEXT    */
/* and COPYBIN2, which does not use stream I/O, should be the fastest  */
/* since it does its own buffering of the data.                        */
/*                                                                     */
/* For smallest possible size compile using the small memory model and */
/* link the following:                                                 */
/*                                                                     */
/*     copybin1                                                        */
/*     smallerr.obj                                                    */
/*     pcbkit_s.lib                                                    */


/* this function will be used to display information to the caller in place */
/* of the println() function which, since this example is stand alone, is   */
/* not available for use.                                                   */

void pascal say(char *Str) {
  doswrite(0,Str,strlen(Str));
  doswrite(0,"\r\n",2);
}


/* this function has been modified from the examples in the manual to call */
/* the above say() function instead of the println() function.             */


int pascal copybinaryfile(char *SourceName, char *TargetName) {
  char    Buffer[1024];
  DOSFILE InFile;
  DOSFILE OutFile;
  int     BytesRead;

  if (dosfopen(SourceName,OPEN_READ|OPEN_DENYNONE,&InFile) == -1) {
    say("Can't open input file.");
    return(-1);
  }

  if (dosfopen(TargetName,OPEN_WRIT|OPEN_CREATE,&OutFile) == -1) {
    say("Can't create output file.");
    dosfclose(&InFile);
    return(-1);
  }

  dossetbuf(&OutFile,16384);  /* read and write in 16K chunks for speed */
  dossetbuf(&InFile,16384);

  while ((BytesRead = dosfread(Buffer,sizeof(Buffer),&InFile)) > 0) {
    if (dosfwrite(Buffer,BytesRead,&OutFile) == -1) {
      say("Error writing output file.");
      break;
    }
  }

  dosfclose(&InFile);
  dosfclose(&OutFile);
  return(0);
}


/* this is a real simple setup to allow the user to specify what file is */
/* to be copied.  It only allows a single file to be copied.             */

void main(int argc, char **argv) {
  dosinit();

  if (argc != 3) {
    say("usage:  COPYBIN1 sourcefile targetfile\r\n"
        "\r\n"
        "NOTE:  Can be used on both binary and text files\r\n");
    return;
  }

  if (copybinaryfile(argv[1],argv[2]) != -1)
    say("file successfully copied");
}
