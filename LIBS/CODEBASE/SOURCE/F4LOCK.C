/* f4lock.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#ifdef __IBMC__
   #define INCL_DOSFILEMGR
   #include <os2.h>
   #ifndef NO_ERROR
      #define NO_ERROR 0
      #define ERROR_LOCK_VIOLATION 33
      #define S4BYTE_DEFINED
   #endif
#endif

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

#ifdef S4UNIX
   #ifdef S4LOCKF
      #include <unistd.h>
   #else
      #include <sys/locking.h>
   #endif
#else
   #ifndef __TURBOC__
      #ifndef __IBMC__
         #include <sys/locking.h>
         #define S4LOCKING
      #endif
   #endif
   #ifdef __ZTC__
      extern int  errno ;
   #endif
   #ifdef _MSC_VER
      #include <sys/types.h>
   #endif
   #ifdef __TURBOC__
/*      extern int cdecl errno ; */
   #endif

   #include <sys/stat.h>
   #include <share.h>
#endif

#include <fcntl.h>
#include <errno.h>

#ifdef S4DO_ERRNO
   extern int errno ;
#endif

#ifdef S4LOCK_HOOK
/* Add alternative code to specify how to handle locking failures.
   This function is commented out to give you the option of defining
   it in your own separate source code file.

   WARNING: The printed documentation does not mention the extra 
   parameter of information containing the number
   of locking tries so far on this attempted file lock.

int file4lock_hook( CODE4 *cb, char *file_name, long offset, long num_bytes, int num_tries )
{
   return 0 ;
}
*/
#endif

int S4FUNCTION file4lock( FILE4 *file, long pos_start, long num_bytes )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      int rc, num_attempts ;

      #ifdef __IBMC__
         struct LockStrc
         {
            long FileOffset ;
            long RangeLength ;
         } L4RANGE ;
      #endif

      #ifdef S4DEBUG
         if ( file == 0 )
            e4severe( e4parm, E4_F4LOCK ) ;
         #ifndef S4MDX
            if ( num_bytes < 0 || pos_start < 0 )
               e4severe( e4parm, E4_F4LOCK ) ;
         #endif
      #endif

      if ( file->is_read_only || file->is_exclusive )  /* don't check error code */
         return 0 ;

      if ( file->code_base->error_code < 0 )
         return -1 ;

      #ifdef S4LOCK_HOOK
         num_attempts = 0 ;
      #else
         num_attempts = file->code_base->lock_attempts ;
         if ( num_attempts == 0 )
            num_attempts = 1 ;
      #endif
      errno = 0 ;

      for( ;; )
      {
         #ifdef S4LOCKING
            #ifdef S4WINDOWS
               _llseek( file->hand, pos_start, 0 ) ;
            #else
               lseek( file->hand, pos_start, 0 ) ;
            #endif

            #ifdef S4LOCKF 
               if ( num_attempts == -1 )    /* sleep until lock */
                  rc = lockf( file->hand, F_LOCK, num_bytes ) ; 
               else
                  rc = lockf( file->hand, F_TLOCK, num_bytes ) ;
            #else
               rc = locking( file->hand, LK_NBLCK, num_bytes ) ;
            #endif
         #else
            #ifdef __IBMC__
               L4RANGE.FileOffset = pos_start ;
               L4RANGE.RangeLength = num_bytes ;

               rc = DosSetFileLocks( (HFILE) file->hand, 0L,(PFILELOCK) &L4RANGE, 100L, 0L ) ;
            #endif

            #ifdef __TURBOC__
               rc = lock( file->hand, pos_start, num_bytes ) ;
            #endif
         #endif
         #ifdef __ZTC__
            if (rc == 0 || errno == 1)
         #else
            #ifdef __IBMC__
               if (rc == NO_ERROR )
            #else
               if (rc == 0 || errno == EINVAL)
            #endif
         #endif
            {
               #ifdef S4LOCK_CHECK
                  l4lock_save( file->hand, pos_start, num_bytes ) ;
               #endif
               return 0 ;
            }

         if ( rc == 0 )
         {
            #ifdef S4LOCK_CHECK
               l4lock_save( file->hand, pos_start, num_bytes ) ;
            #endif
            #ifndef S4OPTIMIZE_OFF
               file4set_write_opt( file, 1 ) ;
            #endif
            return 0 ;
         }

         #ifdef _MSC_VER
            #if _MSC_VER == 600
               #ifdef S4WINDOWS
                  if (errno != -1 )       /* Microsoft 6.00a does not return */
               #else                      /* valid 'errno' under Windows,    */
                  if (errno != EACCES)    /* but performs locking correctly  */
               #endif
            #else
               if (errno != EACCES)
            #endif
         #else
            #ifdef __ZTC__
               if (errno != 33)
            #else
               #ifdef __IBMC__
                  if ( rc != ERROR_LOCK_VIOLATION )
               #else
                  if (errno != EACCES && errno != 0 )
               #endif
            #endif
         #endif
                  return e4( file->code_base, e4lock, file->name ) ;

         #ifdef S4LOCK_HOOK
            rc = file4lock_hook( file->code_base, file->name, pos_start, num_bytes,
                   ++num_attempts ) ;
            if( rc )
               return rc ;
         #else
            if ( num_attempts == 1 )
               return r4locked ;
            if ( num_attempts > 1 )
               num_attempts-- ;
         #endif
         #ifdef S4TEMP
            if ( d4display_quit( &display ) )
               e4severe( e4result, E4_RESULT_EXI ) ;
         #endif

         u4delay_sec() ;   /* wait a second & try lock again */
      }
   #endif
}

int S4FUNCTION file4unlock( FILE4 *file, long pos_start, long num_bytes )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef __IBMC__
         struct LockStrc
         {
            long FileOffset ;
            long RangeLength ;
         } L4RANGE ;
      #endif

      #ifdef S4DEBUG
         if ( file == 0 )
            e4severe( e4parm, E4_F4UNLOCK ) ;
         #ifndef S4MDX
            if ( num_bytes < 0 || pos_start < 0 )
               e4severe( e4parm, E4_F4UNLOCK ) ;
         #endif
      #endif

      if ( file->is_read_only || file->is_exclusive )
         return 0 ;

      #ifndef S4OPTIMIZE_OFF
         file4set_write_opt( file, 0 ) ;
      #endif

      #ifdef S4LOCK_CHECK
         l4lock_remove( file->hand, pos_start, num_bytes ) ;
      #endif

      errno = 0 ;
      #ifdef S4LOCKING
         #ifdef S4WINDOWS
            _llseek( file->hand, pos_start, 0 ) ;
         #else
            lseek( file->hand, pos_start, 0 ) ;
         #endif

         #ifdef S4LOCKF /* lockf() replaces locking() for SUN OS, AT&T */
            rc = lockf( file->hand, F_ULOCK, num_bytes ) ;
         #else
            rc = locking( file->hand, LK_UNLCK, num_bytes ) ;
         #endif
      #else
         #ifdef __IBMC__
            L4RANGE.FileOffset = pos_start ;
            L4RANGE.RangeLength = num_bytes ;

            rc = DosSetFileLocks( (HFILE)file->hand, (PFILELOCK) &L4RANGE, 0L, 100L , 0L ) ;
         #endif
         #ifdef __TURBOC__
            rc = unlock( file->hand, pos_start, num_bytes ) ;
         #endif
      #endif
 
      #ifdef __ZTC__
         if (rc < 0  && errno != 1 )
      #else
         #ifdef __IBMC__
            if (rc != NO_ERROR )
         #else
            if (rc < 0  && errno != EINVAL )
         #endif
      #endif
            return e4( file->code_base, e4unlock, file->name ) ;
   #endif
 
   return 0 ;
}
