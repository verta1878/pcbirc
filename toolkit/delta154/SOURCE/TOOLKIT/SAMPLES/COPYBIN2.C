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


#include <alloc.h>
#include <string.h>
#include <pcbtools.h>

/* This is a small sample program which is NOT set up as a door.  It   */
/* is a simple utility for copying either binary or text files using   */
/* lowl level I/O functions in the PCBoard DOOR Developer's Toolkit.   */
/*                                                                     */
/* For comparison purposes, COPYBIN1 should be faster than COPYTEXT    */
/* and COPYBIN2, which does not use stream I/O, should be the fastest  */
/* since it does its own buffering of the data.                        */
/*                                                                     */
/* Example timings on a 2M file copied on a ram disk are as follows:   */
/*                                                                     */
/*       COPYTEXT     4.67  seconds                                    */
/*       COPYBIN1     2.31  seconds                                    */
/*       COPYBIN2     1.81  seconds                                    */
/*                                                                     */
/* This is not to say that the stream I/O functions are slow.  It's    */
/* more a matter of how they are being used.  Since the copy function  */
/* already has its own local buffer for holding the information read   */
/* in the stream I/O buffer is redundant and therefore slower.  The    */
/* dosfgets() and dosfputs() functions are slightly slower still       */
/* because you then need to worry about carriage return/line feed      */
/* separators which, in the binary copy examples, are completely       */
/* ignored.                                                            */
/*                                                                     */
/* Additionally, since the stream I/O functions are not used here the  */
/* final executable is smaller as shown below:                         */
/*                                                                     */
/*       COPYTEXT.EXE   6590                                           */
/*       COPYBIN1.EXE   6446                                           */
/*       COPYBIN2.EXE   4634                                           */
/*                                                                     */
/* For smallest possible size compile using the small memory model and */
/* link the following:                                                 */
/*                                                                     */
/*     copybin2                                                        */
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
/* the above say() function instead of the println() function - also, it   */
/* calls malloc() and free() instead of bmalloc() and bfree().             */

#define BUFSIZE 16384

int pascal copybinaryfile(char *SourceName, char *TargetName) {
  int  InFile;
  int  OutFile;
  char *Buffer;

  if ((Buffer = (char *)malloc(BUFSIZE)) == NULL) {
    say("Can't allocate memory for the buffer.");
    return(-1);
  }

  if ((InFile = dosopen(SourceName,OPEN_READ|OPEN_DENYNONE)) == -1) {
    say("Can't open input file.");
    free(Buffer);
    return(-1);
  }

  if ((OutFile = dosopen(TargetName,OPEN_WRIT)) == -1) {
    say("Can't open output file.");
    dosclose(InFile);
    free(Buffer);
    return(-1);
  }

  while (dosread(InFile,Buffer,BUFSIZE) == BUFSIZE) {
    if (doswrite(OutFile,Buffer,BUFSIZE) == -1) {
      say("Error writing output file.");
      break;
    }
  }

  dosclose(OutFile);
  dosclose(InFile);
  free(Buffer);
  return(0);
}


/* this is a real simple setup to allow the user to specify what file is */
/* to be copied.  It only allows a single file to be copied.             */

void main(int argc, char **argv) {
  dosinit();

  if (argc != 3) {
    say("usage:  COPYBIN2 sourcefile targetfile\r\n"
        "\r\n"
        "NOTE:  Can be used on both binary and text files\r\n");
    return;
  }

  if (copybinaryfile(argv[1],argv[2]) != -1)
    say("file successfully copied");
}
