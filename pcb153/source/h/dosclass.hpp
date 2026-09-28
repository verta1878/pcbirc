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
/*                                                                            */
/*                                DOSCLASS.HPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*     A pure virtual base class for file access and derived classes for      */
/*                    handle and stream based file access                     */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1995, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

#ifndef ___DOSCLASS_HPP___

#define ___DOSCLASS_HPP___

/******************************************************************************/

/* Included Files */

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#if defined(LIB)
#include    <pcbtools.h>
#undef bool
#undef TRUE
#undef FALSE
#else
#include    <dosfunc.h>
#endif

#include    <types.hpp>

#ifdef __WATCOMC__
  #include <io.h>
#endif

/******************************************************************************/

/* Constants for End Of Line Markers */

extern const char endOfLineStr  [];
extern const char endOfULineStr [];
extern const char endOfNLineStr [];

/******************************************************************************/

/* Pure Virtual Base File Class */

class cFILEBASE
{

    public:

/* Constructor to make sure the name is initialized to empty */
        cFILEBASE ( void ) { _name[0] = NUL; }

/* Virtual destructor to ensure that derived destructors are called */
        virtual ~cFILEBASE ( void ) { }

/* SET / GET / make a TeMPorary name for the file */
              void   setName ( const char * name ) { close();
                                                     if (name)
                                                       strcpy(_name,name); }
        const char * getName ( void )              { return _name; }
        const char * tmpName ( const char * path );

/* NOTE: tmpName hasn't been implemented yet, just stubbed in */
/*       Intended functionality is that a path (C:\PCB\) is passed */
/*       in and a random name (XXXXXXXX.XXX) is appended to the */
/*       specified path. */

/* Functions to fully qualify a path name, make/split the name, */
/* and set/get parts of the name */

        const char * fullName ( void )
            { char _tmpname [ _MAX_PATH ];
              char * p = _fullpath(_tmpname,_name,_MAX_PATH);
              if (p) strcpy(_name,_tmpname), p = _name;
              return p; }

        const char * makeName ( const char * drive, const char * dir,
            const char * name, const char * ext )
            { close();
              _makepath(_name,drive,dir,name,ext);
              return _name; }

        void splitName ( char * drive, char * dir, char * name, char * ext )
            { _splitpath(_name,drive,dir,name,ext); }

        const char * setParts ( const char * drive, const char * dir,
            const char * name, const char * ext );

        void getParts ( char * drive, char * dir, char * name, char * ext )
            { splitName(drive,dir,name,ext); }

        void setDrive ( const char * p ) { setParts(p,0,0,0); }
        void setDir   ( const char * p ) { setParts(0,p,0,0); }
        void setFName ( const char * p ) { setParts(0,0,p,0); }
        void setExt   ( const char * p ) { setParts(0,0,0,p); }

        char * getDrive ( char * p ) { getParts(p,0,0,0); return p; }
        char * getDir   ( char * p ) { getParts(0,p,0,0); return p; }
        char * getFName ( char * p ) { getParts(0,0,p,0); return p; }
        char * getExt   ( char * p ) { getParts(0,0,0,p); return p; }

/* Miscellaneous functions to unlink, rename, move, copy, seek, etc */
        int unlink ( void )              { close();
                                           return ::unlink(getName()); }
        int rename ( const char * name ) { close();
                                           int rv = ::rename(getName(),name);
                                           if (rv == 0) setName(name);
                                           return rv; }
/* move? copy? */

/* Functions to seek to a specified character, implicit SEEK_SET, */
/* seek followed by read/write, etc. */

        int seek ( char c );

        long seek ( long dist ) { return seek(dist,SEEK_SET); }

        int seekRead ( long dist, char meth, void * buf, unsigned size )
            { return ((seek(dist,meth) >= 0) ? readChk(buf,size) : -1); }
        int seekRead ( long dist, void * buf, unsigned size )
            { return seekRead(dist,SEEK_SET,buf,size); }

        int seekWrite ( long dist, char meth, const void * buf, unsigned size )
            { return ((seek(dist,meth) >= 0) ? writeChk(buf,size) : -1); }
        int seekWrite ( long dist, const void * buf, unsigned size )
            { return seekWrite(dist,SEEK_SET,buf,size); }

/* Quick & Dirty access to get/set the absolute file pointer position */
        long pos ( void )   { return seek(0,SEEK_CUR); }
        void pos ( long p ) {        seek(p,SEEK_SET); }

/* Append/Create/Open a file (optionally auto error checking) using */
/* an existing file name */
        int append    ( int flags ) { close();
                                      return _append(_name,flags); }
        int appendChk ( int flags ) { close();
                                      return _appendChk(_name,flags); }
        int create    ( int flags,
                        int attr )  { close();
                                      return _create(_name,flags,attr); }
        int createChk ( int flags,
                        int attr )  { close();
                                      return _createChk(_name,flags,attr); }
        int open      ( int flags ) { close();
                                      return _open(_name,flags); }
        int openChk   ( int flags ) { close();
                                      return _openChk(_name,flags); }

/* Append/Create/Open a file (optionally auto error checking) */
        int append    ( const char * name,
                              int    flags ) { setName(name);
                                               return append(flags); }
        int appendChk ( const char * name,
                              int    flags ) { setName(name);
                                               return appendChk(flags); }
        int create    ( const char * name,
                              int    flags,
                              int    attr )  { setName(name);
                                               return create(flags,attr); }
        int createChk ( const char * name,
                              int    flags,
                              int    attr )  { setName(name);
                                               return createChk(flags,attr); }
        int open      ( const char * name,
                              int    flags ) { setName(name);
                                               return open(flags); }
        int openChk   ( const char * name,
                              int    flags ) { setName(name);
                                               return openChk(flags); }

/* Close a file if it is open */
        int close ( void ) { return (isOpen() ? _close() : 0); }

/* Routine to output a single character */
        int putch ( char c ) { return writeChk(&c,sizeof(c)); }

/* Functions for line oriented input/output */
        virtual int getln   (       char * str, unsigned size );
        virtual int getuln  (       char * str, unsigned size );
        virtual int getnln  (       char * str, unsigned size );

        virtual int puts    ( const char * str )        { return writeChk(str,
                                                            strlen(str)); }
                int putstrs ( const char * str, ... );
                int putln   ( const char * str = NULL ) { return putstrs(str,
                                                            endOfLineStr,
                                                            NULL); }
                int putuln  ( const char * str = NULL ) { return putstrs(str,
                                                            endOfULineStr,
                                                            NULL); }
                int putnln  ( const char * str = NULL ) { int rv = puts(str);
                                                          if (rv == 0)
                                                              rv = putch(NUL);
                                                          return rv; }

/* Functions for formatted input/output */
        int scanf  ( const char * fmt, ... );
        int printf ( const char * fmt, ... );

/* Functions to write a given character a specified number of times */
        int write    ( const char c, unsigned size );
        int writeChk ( const char c, unsigned size );

/* These are all pure virtual and must be defined in derived classes */
        virtual int  commit   (       void )                         = 0;
        virtual int  flush    (       void )                         = 0;
        virtual bool isOpen   (       void )                         = 0;
        virtual int  lock     (       long     off,  long     len )  = 0;
        virtual int  lockChk  (       long     off,  long     len )  = 0;
        virtual int  read     (       void   * buf,  unsigned size ) = 0;
        virtual int  readChk  (       void   * buf,  unsigned size ) = 0;
        virtual int  rewind   (       void )                         = 0;
        virtual long seek     (       long     dist, char     meth ) = 0;
        virtual int  setbuf   (       unsigned size )                = 0;
        virtual long size     (       void )                         = 0;
        virtual int  trunc    (       long     off )                 = 0;
        virtual int  unlock   (       long     off,  long     len )  = 0;
        virtual int  write    ( const void   * buf,  unsigned size ) = 0;
        virtual int  writeChk ( const void   * buf,  unsigned size ) = 0;

/* These are to maintain backward compatibility */
        int appendchk ( const char   * name,
                              int      flags ) { return appendChk(name,
                                                   flags); }
        int createchk ( const char   * name,
                              int      flags,
                              int      attr )  { return createChk(name,flags,
                                                   attr); }
        int lockchk   (       long     off,
                              long     len )   { return lockChk(off,len); }
        int openchk   ( const char   * name,
                              int      flags ) { return openChk(name,flags); }
        int readchk   (       void   * buf,
                              unsigned size )  { return readChk(buf,size); }
        int writechk  ( const void   * buf,
                              unsigned size )  { return writeChk(buf,size); }

    protected:

/* These are all pure virtual and must be defined in derived classes */
/* They are protected because they should not be used directly, but */
/* only by members of this and derived classes */
        virtual int _append    ( const char * name, int flags )           = 0;
        virtual int _appendChk ( const char * name, int flags )           = 0;
        virtual int _close     ( void )                                   = 0;
        virtual int _create    ( const char * name, int flags, int attr ) = 0;
        virtual int _createChk ( const char * name, int flags, int attr ) = 0;
        virtual int _open      ( const char * name, int flags )           = 0;
        virtual int _openChk   ( const char * name, int flags )           = 0;

/* Function to read a line of text using a supplied end of line marker */
        int _getline ( char * str, unsigned size, const char * eol );

    private:

        char _name [ _MAX_PATH ];  /* _MAX_PATH (among others) from stdlib.h */

};

/******************************************************************************/

/* Derived Classes for Handle and Stream Based File Access */

class cFILESTREAM;

class cFILE : public cFILEBASE
{

    friend class cFILESTREAM;

    public:

        cFILE ( const char * name = NULL )
        {
            h = -1;
            aliased = false;
            setName(name);
        }

        virtual ~cFILE ( void ) { close(); }

        int dup ( const cFILE       & file );
        int dup ( const cFILESTREAM & file );

        int alias ( const cFILESTREAM & file );

        virtual int  commit   (       void );
        virtual bool isOpen   (       void );
        virtual int  lock     (       long     off,  long     len );
        virtual int  lockChk  (       long     off,  long     len );
        virtual int  read     (       void   * buf,  unsigned size );
        virtual int  readChk  (       void   * buf,  unsigned size );
        virtual int  rewind   (       void );
        virtual long seek     (       long     dist, char     meth );
        virtual long size     (       void );
        virtual int  trunc    (       long     off );
        virtual int  unlock   (       long     off,  long     len );
        virtual int  write    ( const void   * buf,  unsigned size );
        virtual int  writeChk ( const void   * buf,  unsigned size );

        virtual int flush  ( void ) { return 0; }
        virtual int setbuf ( unsigned size ) { return 0; }

    protected:

        virtual int _append    ( const char * name, int flags );
        virtual int _appendChk ( const char * name, int flags );
        virtual int _close     ( void );
        virtual int _create    ( const char * name, int flags, int attr );
        virtual int _createChk ( const char * name, int flags, int attr );
        virtual int _open      ( const char * name, int flags );
        virtual int _openChk   ( const char * name, int flags );

    private:

        int  h;
        bool aliased;

       #ifdef __OS2__
        os2errtype Os2Error;
       #endif

};

class cFILESTREAM : public cFILEBASE
{

    friend class cFILE;

    public:

        cFILESTREAM ( const char * name = NULL );

        virtual ~cFILESTREAM ( void ) { close(); }

        int dup ( const cFILE       & file );
        int dup ( const cFILESTREAM & file );

        virtual int  commit   (       void );
        virtual int  flush    (       void );
        virtual int  getln    (       char   * str,  unsigned size );
        virtual int  getuln   (       char   * str,  unsigned size );
        virtual bool isOpen   (       void );
        virtual int  lock     (       long     off,  long     len );
        virtual int  lockChk  (       long     off,  long     len );
        virtual int  puts     ( const char   * str );
        virtual int  read     (       void   * buf,  unsigned size );
        virtual int  rewind   (       void );
        virtual long seek     (       long     dist, char     meth );
        virtual int  setbuf   (       unsigned size );
        virtual long size     (       void );
        virtual int  trunc    (       long     off );
        virtual int  unlock   (       long     off,  long     len );
        virtual int  write    ( const void   * buf,  unsigned size );
        virtual int  writeChk ( const void   * buf,  unsigned size );

        virtual int readChk  ( void * buf, unsigned size )
            { return read(buf,size); }

    protected:

        virtual int _append    ( const char * name, int flags );
        virtual int _appendChk ( const char * name, int flags );
        virtual int _close     ( void );
        virtual int _create    ( const char * name, int flags, int attr );
        virtual int _createChk ( const char * name, int flags, int attr );
        virtual int _open      ( const char * name, int flags );
        virtual int _openChk   ( const char * name, int flags );

    private:

        DOSFILE s;

};

/******************************************************************************/

/* Aliases to maintain backward compatibility */

typedef cFILE       cDOS;
typedef cFILESTREAM cDOSFILE;

/******************************************************************************/

#endif

