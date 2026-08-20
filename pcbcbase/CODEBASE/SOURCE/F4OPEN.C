/* f4open.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4TEMP
   #include "t4test.h"
#endif

#include <time.h>

#ifndef S4UNIX
   #ifndef __IBMC__
      #ifndef __TURBOC__
         #include <sys/locking.h>
         #define S4LOCKING
      #endif
      #ifdef __ZTC__
         extern int  errno ;
      #endif
      #ifdef _MSC_VER
         #include <sys/locking.h>
      #endif
      #ifdef __TURBOC__
   /*      extern int cdecl errno ; */
      #endif
   
      #include <dos.h>
   #endif
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <share.h>
#endif

#include <fcntl.h>
#include <errno.h>

#ifndef S4WINDOWS
#ifndef S4OS2
   #ifdef __TURBOC__
      #define S4USE_CHMOD
   #endif
   #ifdef _MSC_VER
      #define S4USE_CHMOD
   #endif
#endif
#endif

#ifdef S4DO_ERRNO
   extern int errno ;
#endif

int S4FUNCTION file4open( FILE4 *file, CODE4 *code_base, char *name, int do_alloc )
{
   int oflag, len ;
   #ifdef _MSC_VER
      unsigned attribute ;
   #endif
   #ifdef S4UNIX
      int pmode ;
      char lwr_name[68] ;
   #endif

   #ifdef S4DEBUG
      if ( file == 0 || code_base == 0 || name == 0 )
         e4severe( e4parm, E4_F4OPEN ) ;
   #endif
   if ( code_base->error_code < 0 )
      return -1 ;

   memset( (void *)file, 0, sizeof( FILE4 ) ) ;
   file->code_base = code_base ;
   file->hand = -1 ;

   code_base->error_code = 0 ;

   #ifdef S4UNIX
      /* Force to lower case for SCO Foxbase Compatibility */
      strncpy( lwr_name, name, sizeof(lwr_name) ) ;
      lwr_name[sizeof(lwr_name)-1] = '\000' ;
      c4lower( lwr_name ) ;

      if ( code_base->read_only == 1 )
      {
         file->is_read_only = 1 ;
         oflag = (int)O_RDONLY ;
      }
      else
         oflag = (int)O_RDWR ;
      if ( code_base->exclusive )
         pmode = 0600 ;  /* Allow exclusive read/write permission  */
      else
         pmode = 0666 ;  /* Allow full read/write permission  */

      file->hand = open( lwr_name, oflag , pmode ) ;
   #else
      /* check the file's dos read only attrib */
      #ifdef S4USE_CHMOD
         #ifdef __TURBOC__
            if ( _A_RDONLY & _chmod( name, 0 ) )
         #endif
         #ifdef _MSC_VER
            _dos_getfileattr( name, &attribute ) ;
            if (_A_RDONLY & attribute )
         #endif
         {
            if ( errno == ENOENT )  /* file doesn't exist */
            {
               file->hand = -1 ;
               if ( file->code_base->open_error )
                  return e4describe( file->code_base, e4open, E4_CREATE_FIL, name, (char *) 0 ) ;
               else
               {
                  code_base->error_code = r4no_open ;
                  return r4no_open ;
               }
            }
            file->is_read_only = 1 ;
            file->is_exclusive = 1 ;  /* can at least emulate it */
            #ifdef S4WINDOWS
               oflag = (int)OF_READ ;
            #else
               oflag = (int)(O_BINARY | O_RDONLY) ;
            #endif
         }
         else
         {
      #endif
            if ( code_base->read_only == 1 )
            {
               file->is_read_only = 1 ;
               #ifdef S4WINDOWS
                  oflag = (int)OF_READ ;
               #else
                  oflag = (int)(O_BINARY | O_RDONLY) ;
               #endif
            }
            else
            {
               #ifdef S4WINDOWS
                  oflag = (int)OF_READWRITE ;
               #else
                  oflag = (int)(O_BINARY | O_RDWR) ;
               #endif
            }
      #ifdef S4USE_CHMOD
         }
      #endif

      #ifdef S4WINDOWS
         if ( code_base->exclusive )
            oflag |= (int)OF_SHARE_EXCLUSIVE ;
         else
            oflag |= (int)OF_SHARE_DENY_NONE ;
         file->hand = _lopen( name, oflag ) ;
      #else
         if ( code_base->exclusive )
            file->hand = sopen( name, oflag, SH_DENYRW, 0 ) ;
         else
            file->hand = sopen( name, oflag , SH_DENYNO, 0 ) ;
      #endif
   #endif

   if ( file->hand < 0 )
   {
      if ( file->code_base->open_error )
         return e4describe( file->code_base, e4open, E4_CREATE_FIL, name, (char *) 0 ) ;
      else
      {
         code_base->error_code = r4no_open ;
         return r4no_open ;
      }
   }

   if ( do_alloc )
   {
      len = strlen(name) + 1 ;
      file->name = (char *)u4alloc_free( code_base, len ) ;
      if ( file->name == 0 )
      {
         file4close( file ) ;
         return e4( file->code_base, e4memory, 0 ) ;
      }
      file->do_alloc_free = 1 ;
      u4ncpy( file->name, name, (unsigned) len ) ;
   }
   else
      file->name = name ;

   file->is_exclusive = (char) code_base->exclusive ;
   file->file_created = 1 ;
   return 0 ;
}

int S4FUNCTION file4open_test( FILE4 *f4 )
{
   return f4->hand >= 0 ;
}
