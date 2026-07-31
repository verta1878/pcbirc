#include <conio.h>
#include <stdio.h>
#include <string.h>

#include <dosclass.hpp>
#include "decode.h"

static int z;         // current length of line we are processing

//=============================================================================
static int pascal decode_quoted( char *msg, int msgSize, char *lineOut, int x ) {
  int   y;                  // offset into msg
  char  t[3], c;            // temp characters
  char  *Walker = msg;      // set the walking pointer to beginning of 'msg'
  char  *sp;                // to find out where the first space is of a
                            // message line to process

  sp = strchr(msg,32);    // find the first space

  // check to see if we need to append a \r\n on the line.
  if ((((msgSize + (msg-sp)) > 78) && (lineOut[x-1] != '\n')) || msgSize == 0) {
    lineOut[x] = '\r';  x++;
    lineOut[x] = '\n';  x++;
    z = 0;
  }

  // if there was a blank line, just return
  if (msgSize == 0) return (x);

  // start the decoding of the line.  We are only looking for one character
  // before we actually do any processing.  Once we have found the '=' symbol
  // then we will check to see if there is a 'soft line break' or special
  // value following (2 character value).
  memset(t,0,sizeof(t));
  y = 0;
  while (y < msgSize) {
    // will most likely need to add code in to append a \r\n to a message here.

    switch (*Walker) {

       // If the character is our '=' were looking for let's start the
       // special processing.
       case '=':
          Walker++;
          y++;

          // check to see if this is a soft line break
          if (*Walker == '\0') {
            // it is, so let's just return
            return (x);
          } else {
            // not a soft line break so the next two characters in the
            // string are going to be converted to a character and placed
            // into the buffer.
            if (y + 2 > msgSize) return (x);     // error decoding msg

            // here's were we actually place the two character value into the
            // buffer.
            memcpy(t,Walker,2);
            if ((c = (char) strtol(t,(char **)NULL,16)) != 0) {
               lineOut[x] = c;
               Walker += 2;
               y += 2;
               x++;
               z++;
            }

            // check to see if we have reached end of line to process
            if (y >= msgSize) return (x);
          }
       break;

       // add anything to the buffer
       default:
          lineOut[x] = *Walker;
          x++;
          y++;
          z++;
          Walker++;
       break;
    }
  }

  // once we are finished, we need to add a '\r\n' for a EOL terminator.
  lineOut[x] = '\r';  x++;
  lineOut[x] = '\n';  x++;
  z += 2;
  return (x);
}

//=============================================================================
long pascal decodeQuotedMsg( cDOSFILE &Org, char *file, char *Boundary, long fRead, long fSize ) {
  int       x;           // where to save the character (also length of lineOut)
  unsigned  llen, llenout;
  char     *lineIn = NULL;    // line to be decoded
  char     *lineOut = NULL;   // output line.
  cDOSFILE  dFile;            // file to output decoded line to

  llen = 8192;        // size of string to be malloced.
  do {
    llen /= 2;
    lineIn = (char *) malloc(llen);
    if (llen < 128) break;
  } while (lineIn == NULL);

  if (lineIn == NULL)
    return (fRead);

  llenout = 32768U;    // size of string to be written to disk.
  do {
    llenout /= 2;
    lineOut = (char *) malloc(llenout);
    if (llenout < 1024) break;
  } while (lineOut == NULL);

  if (lineOut == NULL) {
    free(lineIn);
    return (fRead);
  }

  if (dFile.create(file,OPEN_CREATE|OPEN_WRIT|OPEN_DENYWRIT,0) == 0) {
    // now process rest of the 'file' (which was passed in).
    x = z = 0;
    unsigned actual;
    while (fRead < fSize) {
      if (readline(Org,lineIn,llen,actual) == -1) break;
      fRead += (long) actual;

      // check to see if we are in a multipart message.  If so, we will need
      // to watch for the BOUNDARY signature and break on there.
      if (Boundary != NULL && Boundary[0] != 0) {
        if (memicmp(Boundary,lineIn+2,strlen(Boundary)) == 0)
          break;
      }

      // there are two lines to check for each decoding.  After the
      // second line, then go ahead and save into the storage.
      x = decode_quoted(lineIn,strlen(lineIn),lineOut,x);

      // check to see if a '\r\n' has been appended to the string.  If so,
      // we will need to write out the output and clear our buffers
      if (lineOut[x-1] == '\n') {
        dFile.write(lineOut,x);
        x = z = 0;
      }
    }

    dFile.close();
  }

  free(lineIn);
  free(lineOut);
  return (fRead);
}

//=============================================================================
