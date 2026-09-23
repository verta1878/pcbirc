#include <conio.h>
#include <stdio.h>
#include <string.h>
//#include <io.h>
//#include <fcntl.h>

#include <dosclass.hpp>
#include "decode.h"
#include <MESSAGES.H>

//=============================================================================
#define  FAIL  -1
#define  SKIP  -2

static int _FAR_ fromb64[] = {       // table for decoding base 64
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, SKIP, SKIP, FAIL, FAIL, SKIP, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,

SKIP, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, 0x3e, FAIL, FAIL, FAIL, 0x3f,
0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
0x3c, 0x3d, FAIL, FAIL, FAIL, SKIP, FAIL, FAIL,

FAIL, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
0x17, 0x18, 0x19, FAIL, FAIL, FAIL, FAIL, FAIL,

FAIL, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
0x31, 0x32, 0x33, FAIL, FAIL, FAIL, FAIL, FAIL,

FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL,
FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL, FAIL
};

//=============================================================================
static int pascal decode_base64( char *LineIn, char *LineOut ) {
  char *p;
  int i;
  int l;
  int x;
  int left;
  unsigned long octchar;

  /* Exit on empty input */
  if (LineIn == NULL || LineIn[0] == '\0')
    return (FAIL);

  /* Initialize working pointers */
  p = LineIn;

  l = 0;
  octchar = 0L;
  x = 0;
  left = strlen(LineIn);    // length of text to decode

  // Process entire inbuf.
  while (left != 0) {
      left--;
      i = (int) fromb64[(unsigned char)*p];

      switch(i) {
          case FAIL:
             return (FAIL);
          case SKIP:
             break;
          default:
             octchar = (octchar << 6) | (0x3f & i);
             l++;
             break;
      }

      if (l == 4 || left == 0) {
         switch(l) {
             case 2:
                octchar = octchar >> 4;
                break;
             case 3:
                octchar = octchar >> 2;
                break;
             default:
             break;
         }

         for (l  -= 2; l >= 0; l--) {
            LineOut[x] = (0xff & (octchar >> (l * 8)));
            x++;
         }

         octchar = 0L;
         l = 0;
      }

      p++;
  }

  return (x);
}

long pascal decode64Msg( char *tFileName, cDOSFILE &file, long fRead, long fSize, char *Boundary ) {
  long       tsize;
  int        length;             // length of line after decoding
  int        llen;
  char       File[128];
  char       lineIn[129];        // line to be decoded from base64
  char       lineOut[256];       // output of the decoded input line
  cDOSFILE   dFile;         // file to output decoded line to
  unsigned   actual;

  buildstr(File,PcbData.TmpLoc,tFileName,NULL);
  if (dFile.create(File,OPEN_CREATE|OPEN_WRIT|OPEN_DENYWRIT,0) == 0) {
    while ((llen = readline(file,lineIn,sizeof(lineIn),actual)) != -1) {
      fRead += (long) actual;
      if (! llen) break;
        length = decode_base64(lineIn,lineOut);
        if (length > 0)
          dFile.write(lineOut,length);
      if (lineIn[strlen(lineIn)-1] == '=')
        break;
    }

    tsize = dFile.close();

    // If the file size is zero bytes, just delete it.
    dFile.open(File,OPEN_READ|OPEN_DENYRDWR);
    tsize = dFile.size();
    dFile.close();
    if (tsize==0L) unlink(File);
  }

  // check to see if we are in a multipart message.  If so, we will need
  // to watch for the BOUNDARY signature and break on there.
  if (Boundary != NULL && Boundary[0] != 0) {
    while (fRead < fSize && readline(file,lineIn,sizeof(lineIn),actual) == 0) {
      fRead += (long) actual;
      if (memicmp(Boundary,lineIn+2,strlen(Boundary)) == 0)
        break;
    }
  }

  return(fRead);
}
