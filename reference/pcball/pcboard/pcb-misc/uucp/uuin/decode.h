#ifndef ___DECODE_H___
#define ___DECODE_H___

#include <exprtmsg.hpp>
#include <pcbmsg.hpp>
#include "msgbase.hpp"


#define  DEFAULT_FILENAME  "FILENAME.UNK"

extern char Base64FileName[128];
extern long starting;

int pascal readline( cDOSFILE &file, char *buf, unsigned maxlen, unsigned &actual );
long pascal decode64Msg( char *tFileName, cDOSFILE &file, long read, long size, char *Boundary );
long pascal decodeQuotedMsg( cDOSFILE &Org, char *file, char *Boundary, long read, long size );
long pascal decodeMultiMsg( cNETHDR &hinfo, cTMPMSG &minfo, cDOSFILE &file, long read, long size, char *line, unsigned lsize );
long pascal addMsgLine( cTMPMSG &minfo, cDOSFILE &msg, long read, long size, char *Boundary );
void pascal delnonchars( char *Filename );
void pascal checkmime( cNETHDR &hinfo );

#endif
