/* f4write.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

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

int S4FUNCTION file4write( FILE4 *file, long pos, void *ptr, unsigned ptr_len )
{
   unsigned urc ;

   if ( file->code_base->error_code < 0 )
      return -1 ;

   if ( file->is_read_only )
      return e4( file->code_base, e4parm, E4_PARM_WRT ) ;

   #ifdef S4DEBUG
      if( file == 0 || pos < 0 || ( ptr == 0 && ptr_len ) )
         e4severe( e4parm, E4_F4WRITE ) ;
      if ( file->hand < 0 )
         e4severe( e4parm, E4_F4WRITE ) ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      if ( file->do_buffer )
      {
         urc = opt4file_write( file, pos, ptr_len, ptr, file->buffer_writes ) ;
         if ( urc != ptr_len )
            return e4( file->code_base, e4write, file->name ) ;
      }

      if ( file->do_buffer == 0 || file->write_buffer == 0 || file->buffer_writes == 0 )
      {

         if ( file->file_created == 0 )
         {
            file->code_base->opt.force_current = 1 ;
            file4temp( file, file->code_base, file->name, 1 ) ;
            file->code_base->opt.force_current = 0 ;
         }
      #endif

      #ifdef S4WINDOWS
         if ( _llseek( file->hand, pos, 0 ) != pos )
      #else
         if ( lseek( file->hand, pos, 0 )  != pos )
      #endif
            return e4( file->code_base, e4write, file->name ) ;

      #ifdef S4WINDOWS
         urc = (unsigned) _lwrite( file->hand, (char *) ptr, ptr_len ) ;
      #else
         urc = (unsigned) write( file->hand, ptr, ptr_len ) ;
      #endif

      if ( urc != ptr_len )
         return e4( file->code_base, e4write, file->name ) ;

      #ifdef S4FLUSH
         file4flush( file ) ;
      #endif

   #ifndef S4OPTIMIZE_OFF
      }
   #endif
   return 0 ;
}

