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



#include    <stdio.h>

#include    <uuencode.hpp>

// an extern from the UUCP.CPP module
extern long uucpFileSize;

// const char NUL = '\0';

inline char encodeChar ( char c )
{
    char e = char((c&077)+' ');
    if (e == ' ') e = '`';
    return e;
}

static void near pascal encode3bytes ( char * i, char * & o )
{
    *(o++) = encodeChar(                        i[0] >> 2        );
    *(o++) = encodeChar(((i[0] << 4) & 060) | ((i[1] >> 4) & 017));
    *(o++) = encodeChar(((i[1] << 2) & 074) | ((i[2] >> 6) & 003));
    *(o++) = encodeChar(  i[2]       & 077                       );
}

static void near pascal encodeLine ( char * i, int s, char * o )
{
    *(o++) = encodeChar(char(s));
    for (int n = 0; n < s; n += 3) encode3bytes(i+n, o);
    *(o++) = NUL;
}
/*
void pascal encodeFile ( cDOSFILE & ofile, char * fname,
    char * apath, char * sname )
{
    char spathname [ 128 + 1 ];
    buildstr(spathname,apath,sname,NULL);

    cDOSFILE ifile;
    if (ifile.open(spathname,OPEN_READ|OPEN_DENYNONE) != 0) return;

    ofile.putuln();

    char buf [ 80 + 1 ];
    sprintf(buf,"begin 600 %s",fname);
    ofile.putuln(buf);

    int rb = 0;
    while ((rb = ifile.read(buf,45)) > 0)
    {
        char otext [ 1 + 60 + 1 ];
        encodeLine(buf,rb,otext);
        ofile.putuln(otext);
    }

    ofile.putuln();
    ofile.putuln("end");

    ifile.close();
}
*/
static int  _encodePart = 0;
static long _encodeSize = 0L;

void pascal encodeFileStart ( cDOSFILE & ifile )
{
    _encodePart = 0;
    _encodeSize = ifile.size();
}

int pascal encodeFilePart ( cDOSFILE & ofile, cDOSFILE & ifile, char * fname )
{
    ofile.putuln();

    char buf [ 80 + 1 ];

    sprintf(buf,"---BEGIN %s PART %d-------------------------------------"
        "----------",fname,_encodePart+1);
    ofile.putuln(buf);

    if (_encodePart == 0)
    {
        sprintf(buf,"begin 600 %s",fname);
        ofile.putuln(buf);
    }

    long lnum = 0;
    int  rb = 0;

    // 1024 is magic number for max lines (happens to equal 45K [1024*45])
    // With the new addition of the uucpFileSize variable to tell how big
    // to make each part, the 1024 has been change to 'lnum*45<uucpFileSize*1024'
    // this equates to:  line*numOfBytes < segment*1024
    while ( ((lnum++)*45L < (uucpFileSize*1024L))  && ((rb = ifile.read(buf,45)) > 0))
    {
        char otext [ 1 + 60 + 1 ];
        encodeLine(buf,rb,otext);
        ofile.putuln(otext);
    }

    if (rb == 0)
    {
        ofile.putuln("`");
        ofile.putuln("end");
    }

    sprintf(buf,"---END %s PART %d---------------------------------------"
        "----------",fname,_encodePart+1);
    ofile.putuln(buf);

    ++_encodePart;

    // return 0 for more or -1 for no more
    return -(rb == 0);
}

void pascal encodeFileStop ( void )
{
    _encodePart = 0;
    _encodeSize = 0L;
}

