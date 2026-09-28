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



#include    <string.h>

#include    <uudecode.hpp>

// const char NUL = '\0';

inline char decodeChar ( unsigned char c )
{
    c -= ' ';
    return ((c > 0100) ? 0100 : (c & 0077));
}

void pascal decodeLine ( char * i, char * o, int & s )
{
    int n = decodeChar(*(i++));

    int ilen = strlen(i);
    if ((n > 45) || (n < 1) ||
        (n > (ilen*3/4+1)) || ((n+3) < (ilen*3/4+1)) ||
        (ilen > 61))
    {
        s = 0;
        return;
    }

    s = n;
    while (n > 0)
    {
        char a = decodeChar(*(i++));
        char b = decodeChar(*(i++));
        char c = decodeChar(*(i++));
        char d = decodeChar(*(i++));
        if ((a == 0100) || (b == 0100) || (c == 0100) || (d == 0100))
        {
            s = 0;
            break;
        }
        if (n-- > 0) *(o++) = ((a<<2)|(b>>4));
        if (n-- > 0) *(o++) = ((b<<4)|(c>>2));
        if (n-- > 0) *(o++) = ((c<<6)|(d));
    }
}
/*
int pascal decodeLineCheck ( char * line )
{
    int n = decodeChar(*(line++));
    return !((n > 45) || (n < 1) || (n > (strlen(line)*3/4+1)));
}

void pascal decodeFile ( char * tpath, char * fname, cDOSFILE & ifile )
{
    char pname [ 128 + 1 ];
    buildstr(pname,tpath,fname,NULL);

    cDOSFILE ofile;
    if (ofile.open(pname,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE) != 0) return;

    char line [ 128 + 1 ];

    while (ifile.getuln(line,sizeof(line)) == 0)
    {
        if (line[0] == NUL) continue;

        if ((line[0] == 'e') &&
            (line[1] == 'n') &&
            (line[2] == 'd') &&
            (line[3] == NUL))
            break;

        char buf [ 45 + 1 ];
        int size;
        decodeLine(line,buf,size);
        if (ofile.write(buf,size) != 0) break;
    }

    ofile.close();
}
*/
static cDOSFILE tmpFile;

static char pname [ 128 + 1 ];

int pascal decodeFileStart ( char * tpath, char * fname )
{
    buildstr(pname,tpath,fname,NULL);

    tmpFile.open(pname,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE);

    return (tmpFile.isOpen() ? 0 : -1);
}

//#include    <constream.h>

//extern constream conerr;

void pascal decodeFileLine  ( char * line )
{
 // static unsigned long i = 0;
 // conerr << "Decoding line " << ++i << " to " << pname << endl;

    if (line[0] == NUL) return;

    char buf [ 45 + 1 ];
    int size;
    decodeLine(line,buf,size);

    tmpFile.write(buf,size);
}

void pascal decodeFileStop  ( void )
{
    tmpFile.close();
}

#ifdef ___TEST___

#include    <iostream.h>
#include    <iomanip.h>

extern "C" {

    void pascal pcbinit ( void );

}

int main ( int argc, char * argv [] )
{
    if (argc < 3)
    {
        cerr << "Usage: UUDECODE SOURCE DESTINATION" << endl;
        return 1;
    }

    pcbinit();

    cDOSFILE in, out;

    if (in.open(argv[1],OPEN_READ|OPEN_DENYNONE) != 0)
    {
        cerr << "Error opening " << argv[1] << '!' << endl;
        return 2;
    }

    if (out.open(argv[2],OPEN_READ|OPEN_DENYNONE) == 0)
    {
        cerr << argv[2] << " already exists!" << endl;
        return 3;
    }

    if (out.open(argv[2],OPEN_CREATE|OPEN_WRIT|OPEN_DENYRDWR) != 0)
    {
        cerr << "Error creating " << argv[2] << '!' << endl;
        return 3;
    }

    cout << "Decoding " << argv[1] << " to " << argv[2] << " ..." << endl;

    char line [ 1024 + 1 ];
    char buf  [ 1024 + 1 ];
    bool decode = FALSE;

    while (in.getln(line,sizeof(line)) == 0)
    {
        if (memcmp(line,"begin ",6) == 0)
        {
            decode = TRUE;
        }
        else if (strcmp(line,"end") == 0)
        {
            break;
        }
        else if (decode)
        {
            int size;
            decodeLine(line,buf,size);
            if (out.write(buf,size) != 0)
            {
                cerr << "Error writing!" << endl;
                break;
            }
        }
    }

    out.close();
    in.close();

    cout << "Decoding Complete!" << endl;

    return 0;
}

#endif

