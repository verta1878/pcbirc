#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>

#include "decode.h"
#include <MESSAGES.H>

//=============================================================================
static char * const illegalChars = ".;,=+<>|\"[] \\?*";
char Base64FileName[128];

//=============================================================================
void pascal delnonchars( char *Filename ) {
  int   vLen = strlen(Filename);
  char  newName[13];
  char  path[MAXPATH], dir[MAXDIR], name[MAXFILE], ext[MAXEXT];
  char *p, *q;

  // break down the name and ext.  We'll just ignore the path and dir info
  fnsplit ( Filename, path, dir, name, ext );
  q = name;

  // check to make sure the name and ext are valid.
  if (strpbrk(name, illegalChars) != NULL || strpbrk(ext+1,illegalChars) != NULL) {

    // clear out the working buffers
    memset(newName,0,sizeof(newName));

    // make the name and ext valid before returning
    while ((p=strpbrk(q,illegalChars)) != NULL) {
      strncat(newName,q,(int)(p-q));
      q = p+1;
    }
    // add the rest of the valid characters.
    strcat(newName,q);

    q = ext;
    while ((p=strpbrk(q,illegalChars)) != NULL) {
      strncat(newName,q,(int)(p-q));
      q = p+1;
    }
    // add the rest of the valid characters.
    strcat(newName,q);

    // copy the new name into the Filename pointer after clearing out
    // the old information.
    memset(Filename,0,vLen);
    strcpy(Filename,newName);
  } else {
    memset(Filename,0,vLen);
    strcpy(Filename,name);
    strcat(Filename,ext);
  }
}

//=============================================================================
void pascal checkmime( cNETHDR &hinfo ) {
  char    Type[256];
  char  * p;

  if (hinfo.ContentType[0] == 0) return;
  maxstrcpy(Type,hinfo.ContentType,sizeof(Type));
  p = strtok(Type,";");

  while (p != NULL) {
    stripleft(p,32); stripright(p,32);    // strip leading/trailing spaces
    stripleft(p,9);  stripright(p,9);     // strip leading/trailing tabs

    // if there is a BOUNDARY signature, parse out the info
    if (memicmp(p,"bound",5) == 0) {
      if (p[9] == '"') {
        memcpy(hinfo.Boundary,p+10,strlen(p)-11);
      } else {
        memcpy(hinfo.Boundary,p+9,strlen(p)-9);
      }

    // check to see if a filename has been specified for a file attachment.
    } else if (memicmp(p,"name",4) == 0) {
      if (p[5] == '"') {
        memcpy(Base64FileName,p+6,strlen(p)-7);
      } else {
        memcpy(Base64FileName,p+5,strlen(p)-5);
      }
      hinfo.file = Base64FileName;
    }

    // parse out the next section of the ContentType string
    p = strtok(NULL,";");
  }

  // remove all non-valid DOS characters from the filename.
  if (hinfo.file != NULL)
    delnonchars(hinfo.file);

  if (hinfo.Disposition[0] == 0) return;
  maxstrcpy(Type,hinfo.Disposition,sizeof(Type));
  p = strtok(Type,";");

  while (p != NULL) {
    stripleft(p,32); stripright(p,32);    // strip leading/trailing spaces
    stripleft(p,9);  stripright(p,9);     // strip leading/trailing tabs

    // check to see if a filename has been specified for a file attachment.
    if (memicmp(p,"name",4) == 0) {
      if (p[5] == '"') {
        memcpy(Base64FileName,p+6,strlen(p)-7);
      } else {
        memcpy(Base64FileName,p+5,strlen(p)-5);
      }
      hinfo.file = Base64FileName;
    } else if (memicmp(p,"filename",8) == 0) {
      if (p[9] == '"') {
        memcpy(Base64FileName,p+10,strlen(p)-11);
      } else {
        memcpy(Base64FileName,p+9,strlen(p)-9);
      }
      hinfo.file = Base64FileName;
    }

    // parse out the next section of the Disposition string
    p = strtok(NULL,";");
  }

  // remove all non-valid DOS characters from the filename.
  if (hinfo.file != NULL)
    delnonchars(hinfo.file);
}

//=============================================================================
long pascal decodeMultiMsg( cNETHDR &hinfo, cTMPMSG &minfo, cDOSFILE &file, long fRead, long fSize, char *line, unsigned lsize ) {
  unsigned  actual;
  int       llen;
  char      tFile[129];
  cDOSFILE  tmp;

  // check for the ContentType header.  If found, parse it
  checkmime(hinfo);

  // if there is no Boundary signature, just return.  We are unable to
  // correctly decode this mesage.  Let it be imported as is.
  if (hinfo.Boundary[0] == 0) return(fRead);

  // read in the next line of the file.  We are going to keep reading in
  // line by line until we find a 'Boundary' segment.
  while (1) {
    if ((llen = readline(file,line,lsize,actual)) == -1)
      return(fRead);

    fRead += (long) actual;
    if (! llen) continue;   // empty line, keep reading in.
    if (memicmp(line+2,hinfo.Boundary,strlen(hinfo.Boundary)) == 0) break;
  }

  memset(hinfo.ContentType,0,sizeof(hinfo.ContentType));
  memset(hinfo.EncodeType,0,sizeof(hinfo.EncodeType));
  memset(hinfo.Disposition,0,sizeof(hinfo.Disposition));
  while (fRead < fSize && ((llen = readline(file,line,lsize,actual)) != -1)) {

    fRead += (long) actual;
    if (! llen) {
      checkmime(hinfo);
      break;
    }
    hinfo.getInfo(line);
  }

PROCESS:
  if (memicmp(hinfo.EncodeType,"base64",6) == 0) {
    // Base64 decoding here
    if (Base64FileName[0] == 0) strcpy(Base64FileName,DEFAULT_FILENAME);
    hinfo.file = Base64FileName;
    fRead = decode64Msg(Base64FileName,file,fRead,fSize,hinfo.Boundary);
    minfo.flushMsg();
  } else if (memicmp(hinfo.EncodeType,"quoted",6) == 0) {
    // Quoted-Printable decoding here
    if (hinfo.file != NULL) {
      buildstr(tFile,PcbData.TmpLoc,hinfo.file,NULL);
      fRead = decodeQuotedMsg(file,tFile,hinfo.Boundary,fRead,fSize);
    } else {
      fRead = decodeQuotedMsg(file,"DECODE.TMP",hinfo.Boundary,fRead,fSize);
      tmp.open("DECODE.TMP",OPEN_READ|OPEN_DENYRDWR);
      addMsgLine(minfo,tmp,0,tmp.size(),NULL);     //lint  !e534
      tmp.close();
      unlink("DECODE.TMP");
    }
    minfo.flushMsg();
  } else {
    // no special processing required for a MIME message.  Just
    // let it fall through for normal processing.
    fRead = addMsgLine(minfo,file,fRead,fSize,hinfo.Boundary);
  }

  // reset the MIME info processing variables
  memset(hinfo.ContentType,0,sizeof(hinfo.ContentType));
  memset(hinfo.EncodeType,0,sizeof(hinfo.EncodeType));
  memset(hinfo.Disposition,0,sizeof(hinfo.Disposition));
  hinfo.file = NULL;

  while (fRead < fSize && ((llen = readline(file,line,lsize,actual)) != -1)) {
    fRead += (long) actual;
    if (! llen) {
      checkmime(hinfo);
      goto PROCESS;
    }
    hinfo.getInfo(line);
  }

  return(fRead);
}

//=============================================================================
// 15.4 fix (per HISTORY): "Now supports 'multipart/alternative' MIME encoding."
//
// A multipart/alternative message contains multiple representations of the
// same content (e.g., text/plain + text/html).  The receiver should pick
// exactly ONE alternative — the "best" one for the recipient's capability.
// For a text-only BBS the best choice is text/plain; the HTML part is
// discarded.  Before this fix, PCBoard 15.3 treated multipart/alternative
// as multipart/mixed and dumped BOTH parts into the mail conference — the
// HTML tags then appeared as raw text alongside the plain version.
//
// Algorithm:
//   1. Locate the first boundary.
//   2. Scan each part's headers, remember offset of any text/plain part.
//   3. If a text/plain part was found, seek back to it and emit it.
//   4. Otherwise, emit the first part.
//
long pascal decodeMultiAltMsg( cNETHDR &hinfo, cTMPMSG &minfo, cDOSFILE &file, long fRead, long fSize, char *line, unsigned lsize ) {
  unsigned  actual;
  int       llen;
  long      plainOff = -1;         // file offset of text/plain part body
  long      firstOff = -1;         // file offset of first part body (fallback)
  char      partCT[80];            // this part's Content-Type
  cNETHDR   partInfo;

  checkmime(hinfo);
  if (hinfo.Boundary[0] == 0) return(fRead);

  // Skip to first boundary
  while (1) {
    if ((llen = readline(file,line,lsize,actual)) == -1) return(fRead);
    fRead += (long) actual;
    if (llen && memicmp(line+2,hinfo.Boundary,strlen(hinfo.Boundary)) == 0)
      break;
  }

  // Walk each part.  For each, parse headers to find Content-Type, record
  // the byte offset where the body starts, then skip to next boundary.
  while (fRead < fSize) {
    partCT[0] = 0;
    memset(&partInfo,0,sizeof(partInfo));

    // Read this part's headers
    while (fRead < fSize && ((llen = readline(file,line,lsize,actual)) != -1)) {
      fRead += (long) actual;
      if (! llen) break;                             // blank line = end of headers
      if (memicmp(line,"Content-Type:",13) == 0) {
        char *v = line + 13;
        while (*v == ' ' || *v == '\t') v++;
        maxstrcpy(partCT, v, sizeof(partCT));
      }
      partInfo.getInfo(line);
    }

    // Record the body-start offset for this part
    long bodyStart = file.seek(0, SEEK_CUR);
    if (firstOff < 0) firstOff = bodyStart;
    if (memicmp(partCT, "text/plain", 10) == 0 && plainOff < 0)
      plainOff = bodyStart;

    // Skip body until next boundary (or EOF).
    while (fRead < fSize && ((llen = readline(file,line,lsize,actual)) != -1)) {
      fRead += (long) actual;
      if (llen && memicmp(line+2,hinfo.Boundary,strlen(hinfo.Boundary)) == 0) {
        // Check for closing boundary (ends with '--')
        int blen = strlen(hinfo.Boundary);
        if (llen >= blen + 4 && line[2+blen] == '-' && line[2+blen+1] == '-') {
          fRead = fSize;                             // force exit
        }
        break;
      }
    }
  }

  // Emit the chosen alternative
  long emitOff = (plainOff >= 0) ? plainOff : firstOff;
  if (emitOff >= 0) {
    file.seek(emitOff, SEEK_SET);
    addMsgLine(minfo, file, emitOff, fSize, hinfo.Boundary);   //lint  !e534
  }

  memset(hinfo.ContentType,0,sizeof(hinfo.ContentType));
  memset(hinfo.EncodeType,0,sizeof(hinfo.EncodeType));
  memset(hinfo.Disposition,0,sizeof(hinfo.Disposition));
  hinfo.file = NULL;

  return(fRead);
}

//=============================================================================
