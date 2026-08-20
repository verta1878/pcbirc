/* f4close.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

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
         #include <sys/types.h>
         #include <sys/locking.h>
      #endif
      #ifdef __TURBOC__
   /*      extern int cdecl errno ; */
      #endif
   #endif

   #include <sys/stat.h>
   #include <share.h>
#endif

#include <fcntl.h>
#include <errno.h>

#ifdef S4DO_ERRNO
   extern int errno ;
#endif

int S4FUNCTION file4close( FILE4 *file )
{
   int rc ;
   CODE4 *c4;

   if ( file == 0 ) return 0 ;

   c4 = file->code_base;

   if ( file->hand < 0 )
   {
      if ( c4->error_code < 0 )
         return -1 ;
      return 0 ;
   }

   #ifndef S4OPTIMIZE_OFF
      if ( file->file_created == 0 )
      {
         file4len_set( file, 0 ) ;
         file4optimize( file, 0, 0 ) ;
      }
      else
      {
         file4optimize( file, 0, 0 ) ;
   #endif

      #ifdef S4WINDOWS
         rc = _lclose( file->hand ) ;
      #else
         rc = close( file->hand ) ;
      #endif
      if ( rc < 0 )
      {
         if ( file->name == 0 )
            e4( c4, e4close, E4_CLOSE ) ;
         else
            e4( c4, e4close, file->name ) ;
      }

      if ( file->is_temp )
         u4remove( file->name ) ;

   #ifndef S4OPTIMIZE_OFF
      }
   #endif

   if ( file->do_alloc_free )
      u4free( file->name ) ;

   memset( (void *)file, 0, sizeof( FILE4 ) ) ;
   file->hand = -1 ;

   if ( c4->error_code < 0 )
      return -1 ;

   return 0 ;
}
