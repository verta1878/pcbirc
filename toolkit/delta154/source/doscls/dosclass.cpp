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


/******************************************************************************/

#include    <io.h>
#include    <stdarg.h>

#include    "clearbuf.hpp"

#include    "dosclass.hpp"

#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

// For Toolkit Compatibility
#if defined(LIB)

  /* the DOS functions don't pass the extended error values because the */
  /* extended error variables are global variables.                     */

  #define DOS2ERROR
  #define POS2ERROR
  #define POS2ERROR2

  extern int      ExtendedError;
  extern char     ExtendedAction;
  extern char     ExtendedClass;
  extern char     ExtendedLocus;
  extern char     Int24Error;
  extern char     Int24Flags;

  int LIBENTRY dostrunc  ( int handle, long offset DOS2ERROR );
  int LIBENTRY dosftrunc ( DOSFILE *file, long offset );
  int LIBENTRY dosfugets ( char *Str, unsigned MaxChars, DOSFILE *file );

#endif

/******************************************************************************/

const char endOfLineStr  [] = "\r\n";
const char endOfULineStr [] = "\n";
const char endOfNLineStr [] = "";

/******************************************************************************/

const char * cFILEBASE::tmpName ( const char * path )
{
    close();
    // build temp name here
    if (!*path) return path; // dummy to suppress warning for now
    return _name;
}

const char * cFILEBASE::setParts ( const char * drive, const char * dir,
    const char * name, const char * ext )
{
    char _drive [ _MAX_DRIVE ];
    char _dir   [ _MAX_DIR   ];
    char _name  [ _MAX_FNAME ];
    char _ext   [ _MAX_EXT   ];
    splitName(_drive,_dir,_name,_ext);
    return makeName(drive?drive:_drive,dir?dir:_dir,
        name?name:_name,ext?ext:_ext);
}

int cFILEBASE::seek ( char c )
{
    // Assume no error
    int rv = 0;

    // Buffer for input and pointer for searching
    char buf [ 128 ];
    char * p;

    // While there are no errors
    do {

        // Attempt to fill up the buffer
        rv = readChk(buf,sizeof(buf));

        if (rv == 0)
        {
            // Break if nothing read in but no error
            break;
        }
        else if (rv > 0)
        {
            // Search for character if something read in without error
            p = (char *) memchr(buf,c,rv);

            // If it was found
            if (p != NULL)
            {
                // Seek back to byte following found character
                rv = ((seek(long(sizeof(buf)-(p-buf)-1)*-1L,SEEK_CUR) < 0) ?
                    -1 : 0);
                // And break out of loop
                break;
            }

            // Assume no error
            rv = 0;
        }

    } while (rv == 0);

    return rv;
}

int cFILEBASE::getln ( char * str, unsigned size )
{
    return _getline(str,size,endOfLineStr);
}

int cFILEBASE::getuln ( char * str, unsigned size )
{
    return _getline(str,size,endOfULineStr);
}

int cFILEBASE::getnln ( char * str, unsigned size )
{
    return _getline(str,size,endOfNLineStr);
}

int cFILEBASE::putstrs ( const char * str, ... )
{
    int rv = 0;

    va_list args;
    va_start(args,str);

    do {

        rv = puts(str?str:"");
        str = va_arg(args,const char *);

    } while ((rv == 0) && (str != NULL));

    va_end(args);

    return rv;
}

int cFILEBASE::scanf ( const char * fmt, ... )
{
    char buf [ 1024 + 1 ];

    int rv = getln(buf,sizeof(buf));

    if (rv == 0)
    {
        va_list args;
        va_start(args,fmt);

        vsscanf(buf,fmt,args);

        va_end(args);
    }

    return rv;
}

int cFILEBASE::printf ( const char * fmt, ... )
{
    char buf [ 1024 + 1 ];

    #ifdef DEBUG
      mc_register(buf,sizeof(buf));
    #endif

    va_list args;
    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    va_end(args);

    int rv = puts(buf);

    #ifdef DEBUG
      mc_unregister(buf);
    #endif

    return rv;
}

#define wrChBuf(f)               \
    int rv = 0;                  \
    char * buf = new char[size]; \
    if (buf)                     \
    {                            \
        memset(buf,c,size);      \
        rv = write(buf,size);    \
        delete [] buf;           \
    }                            \
    else                         \
    {                            \
        rv = -1;                 \
    }                            \
    return rv

int cFILEBASE::write ( const char c, unsigned size )
{
    wrChBuf(write);
}

int cFILEBASE::writeChk ( const char c, unsigned size )
{
    wrChBuf(writeChk);
}

int cFILEBASE::_getline ( char * str, unsigned size, const char * eol )
{
    // Assume no error
    int rv = 0;

    // Computer temporary string buffer size
    int _size = size + strlen(eol) + ((eol[0] == NUL) ? 1 : 0);

    // Allocate a buffer, returning an error if insufficient memory
    char * _str = new char[_size];
    if (_str == NULL) return -1;

    // Read into the buffer
    int br = readChk(_str,_size-1);

    // If we read something in ...
    if (br > 0)
    {
        // NUL terminate the string
        _str[br] = NUL;

        // Length of the end of line marker (assuming none)
        int eollen = 0;

        // Find end of line marker
        char * p = ((eol[0] != NUL) ? strstr(_str,eol) :
            (char *) memchr(_str,eol[0],br));

        // If it was found take care of it
        if (p)
        {
            // NUL terminate at end of line
            *p = NUL;

            // Length of the end of line marker
            eollen = ((eol[0] == NUL) ? 1 : strlen(eol));
        }
        // Otherwise if we read in too much ...
        else if (strlen(_str) > size-1)
        {
            // Adjust to the maximum allowed
            _str[size-1] = NUL;
            eollen = 0;
            rv = 1;
        }

        // Seek back to the beginning of the next line
        if (seek(-(long(br)-strlen(_str)-eollen),SEEK_CUR) < 0)
            rv = -1;

        // Copy the string
        strcpy(str,_str);
    }
    else
    {
        // We didn't read anything, so error
        rv = -1;
    }

    // Free allocated memory buffer
    delete _str;

    // Return any applicable error codes
    return rv;
}

/******************************************************************************/

int cFILE::dup ( const cFILE & file )
{
    close();
    h = dosdup(file.h POS2ERROR);
    return ((h > 0) ? 0 : -1);
}

int cFILE::dup ( const cFILESTREAM & file )
{
    close();
    h = dosdup(file.s.handle POS2ERROR);
    return ((h > 0) ? 0 : -1);
}

int cFILE::alias ( const cFILESTREAM & file )
{
    close();
    h = file.s.handle;
    if (h > 0)
        aliased = true;
    else
        h = -1;
    return ((h > 0) ? 0 : -1);
}

int cFILE::commit ( void )
{
    return doscommit(h);
}

bool cFILE::isOpen ( void )
{
    return (h > 0);
}

int cFILE::lock ( long off, long len )
{
    int r = ::lock(h,off,len);
    return ((r == 0) ? 0 : -1);
}

int cFILE::lockChk ( long off, long len )
{
    int r = doslockcheck(h,off,len);
    return ((r == 0) ? 0 : -1);
}

int cFILE::read ( void * buf, unsigned size )
{
    return dosread(h,buf,size POS2ERROR);
}

int cFILE::readChk ( void * buf, unsigned size )
{
    return readcheck(h,buf,size);
}

int cFILE::rewind ( void )
{
    return ((seek(0,SEEK_SET) == 0) ? 0 : -1);
}

long cFILE::seek ( long dist, char meth )
{
    return doslseek(h,dist,meth);
}

long cFILE::size ( void )
{
    long _pos  = seek(0,SEEK_CUR);
    long _size = seek(0,SEEK_END);
    seek(_pos,SEEK_SET);
    return _size;
}

int cFILE::trunc ( long off )
{
    return dostrunc(h,off POS2ERROR);
}

int cFILE::unlock ( long off, long len )
{
    int r = ::unlock(h,off,len);
    return ((r == 0) ? 0 : -1);
}

int cFILE::write ( const void * buf, unsigned size )
{
    return doswrite(h,(void*)buf,size POS2ERROR);
}

int cFILE::writeChk ( const void * buf, unsigned size )
{
    return writecheck(h,(void*)buf,size);
}

int cFILE::_append ( const char * name, int flags )
{
    h = dosappend((char*)name,flags POS2ERROR);
    return ((h > 0) ? 0 : -1);
}

int cFILE::_appendChk ( const char * name, int flags )
{
    h = dosappendcheck((char*)name,flags);
    return ((h > 0) ? 0 : -1);
}

int cFILE::_close(void)
{
    if (!aliased)
        if (h > 0)
            dosclose(h);
    h = -1;
    aliased = false;
    return 0;
}

int cFILE::_create ( const char * name, int flags, int attr )
{
    h = doscreate((char*)name,flags,attr POS2ERROR);
    return ((h > 0) ? 0 : -1);
}

int cFILE::_createChk ( const char * name, int flags, int attr )
{
    h = doscreatecheck((char*)name,flags,attr);
    return ((h > 0) ? 0 : -1);
}

int cFILE::_open ( const char * name, int flags )
{
    h = dosopen((char*)name,flags POS2ERROR);
    return ((h > 0) ? 0 : -1);
}

int cFILE::_openChk ( const char * name, int flags )
{
    h = dosopencheck((char*)name,flags);
    return ((h > 0) ? 0 : -1);
}

/******************************************************************************/

cFILESTREAM::cFILESTREAM ( const char * name )
{
    clearBuffer(s);
    setName(name);
}

int cFILESTREAM::dup ( const cFILE & file )
{
    close();
    s.handle = file.h;
    return dosfopen("",OPEN_ALREADY,&s);
}

int cFILESTREAM::dup ( const cFILESTREAM & file )
{
    close();
    s.handle = file.s.handle;
    return dosfopen("",OPEN_ALREADY,&s);
}

int cFILESTREAM::commit ( void )
{
    return doscommit(s.handle);
}

int cFILESTREAM::flush ( void )
{
    return dosflush(&s);
}

int cFILESTREAM::getln ( char * str, unsigned size )
{
    return dosfgets(str,size,&s);
}

int cFILESTREAM::getuln ( char * str, unsigned size )
{
   #ifdef ___NO_DOSFUGETS___
    return _getline(str,size,endOfULineStr);
   #else
    return dosfugets(str,size,&s);
   #endif
}

bool cFILESTREAM::isOpen ( void )
{
    return (s.handle > 0);
}

int cFILESTREAM::lock ( long off, long len )
{
    int r = ::lock(s.handle,off,len);
    return ((r == 0) ? 0 : -1);
}

int cFILESTREAM::lockChk ( long off, long len )
{
    int r = doslockcheck(s.handle,off,len);
    return ((r == 0) ? 0 : -1);
}

int cFILESTREAM::puts ( const char * str )
{
    return dosfputs((char*)str,&s);
}

int cFILESTREAM::read ( void * buf, unsigned size )
{
    return dosfread(buf,size,&s);
}

int cFILESTREAM::rewind ( void )
{
    dosrewind(&s);
    return 0;
}

long cFILESTREAM::seek ( long dist, char meth )
{
    return dosfseek(&s,dist,meth);
}

int cFILESTREAM::setbuf ( unsigned size )
{
    dossetbuf(&s,size);
    return 0;
}

long cFILESTREAM::size ( void )
{
    long _pos  = seek(0,SEEK_CUR);
    long _size = seek(0,SEEK_END);
    seek(_pos,SEEK_SET);
    return _size;
}

int cFILESTREAM::trunc ( long off )
{
    return dosftrunc(&s,off);
}

int cFILESTREAM::unlock ( long off, long len )
{
    int r = ::unlock(s.handle,off,len);
    return ((r == 0) ? 0 : -1);
}

int cFILESTREAM::write ( const void * buf, unsigned size )
{
    return ((dosfwrite((void*)buf,size,&s) == 0) ? size : 0);
}

int cFILESTREAM::writeChk ( const void * buf, unsigned size )
{
    return dosfwrite((void*)buf,size,&s);
}

int cFILESTREAM::_append ( const char * name, int flags )
{
    return _open(name,flags|OPEN_APPEND);
}

int cFILESTREAM::_appendChk ( const char * name, int flags )
{
    return _append(name,flags);
}

int cFILESTREAM::_close ( void )
{
    return dosfclose(&s);
}

#pragma argsused
int cFILESTREAM::_create ( const char * name, int flags, int attr )
{
    return _open(name,flags|OPEN_CREATE);
}

int cFILESTREAM::_createChk ( const char * name, int flags, int attr )
{
    return _create(name,flags,attr);
}

int cFILESTREAM::_open ( const char * name, int flags )
{
    return dosfopen((char*)name,flags,&s);
}

int cFILESTREAM::_openChk ( const char * name, int flags )
{
    return _open(name,flags);
}

/******************************************************************************/

