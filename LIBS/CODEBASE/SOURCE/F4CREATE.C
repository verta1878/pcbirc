/* f4create.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

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

int S4FUNCTION file4create( FILE4 *file, CODE4 *code_base, char *name, int do_alloc )
{
   int len ;
   #ifdef S4WINDOWS
      int exist_handle ;
   #else
      int  extra_flag ;
      int  oflag, pmode ;
   #endif
   #ifdef S4UNIX
      char lwr_name[68] ;
   #endif

   #ifdef S4DEBUG
      if ( file == 0 || code_base == 0 || name == 0 )
         e4severe( e4parm, E4_F4CREATE ) ;
   #endif

   memset( (void *)file, 0, sizeof( FILE4 ) ) ;
   file->code_base = code_base ;
   file->hand = -1 ;

   if ( code_base->error_code < 0 )
      return -1 ;
   code_base->error_code = 0 ;

   #ifdef S4UNIX
       /* Force to lower case for SCO Foxbase Compatibility */
      if ( code_base->safety )
         extra_flag = O_EXCL ;
      else
         extra_flag = 0 ;

      strncpy( lwr_name, name, sizeof( lwr_name ) ) ;
      lwr_name[sizeof( lwr_name ) - 1] = '\000' ;
      c4lower( lwr_name ) ;

      oflag = (int)(O_CREAT | O_TRUNC | O_RDWR) ;

      if ( code_base->exclusive )
         pmode = 0600 ;  /* Allow exclusive read/write permission  */
      else
         pmode = 0666 ;  /* Allow full read/write permission  */

      file->hand = open( lwr_name, extra_flag | oflag, pmode ) ;
   #else
      #ifdef S4WINDOWS
         exist_handle =  -1 ;
         if ( code_base->safety )
            exist_handle = access( name, 0 ) ;  /* if file doesn't exist, create */

         if ( exist_handle < 0 )  /* if file doesn't exist, create */
         {
            /* do initial creation in exclusive mode */
            file->hand = _lcreat( name, 0 ) ;   /* attr == 0 : read/write permission */

            if ( file->hand >= 0 )  /* now that file is created, move to regular r/w mode */
            {
               _lclose( file->hand ) ;
               if ( code_base->exclusive )
                  file->hand = _lopen( name, OF_READWRITE | OF_SHARE_EXCLUSIVE ) ;
               else
                  file->hand = _lopen( name, OF_READWRITE | OF_SHARE_DENY_NONE ) ;
            }
         }
      #else
         oflag = (int)( O_CREAT | O_TRUNC | O_BINARY | O_RDWR ) ;
         pmode = (int)( S_IREAD  | S_IWRITE ) ;
         if ( code_base->safety )
            extra_flag = O_EXCL ;
         else
            extra_flag = 0 ;
         if ( code_base->exclusive )
            file->hand = sopen( name, extra_flag | oflag, SH_DENYWR, pmode ) ;
         else
            file->hand = sopen( name, extra_flag | oflag, SH_DENYNO, pmode ) ;
      #endif
   #endif

   if ( file->hand < 0 )
   {
      if ( code_base->create_error )
         return e4describe( file->code_base, e4create, E4_CREATE_FIL, name, (char *) 0 ) ;
      code_base->error_code = r4no_create ;
      return r4no_create ;
   }

   if ( do_alloc )
   {
      len = strlen(name) + 1 ;
      file->name = (char *)u4alloc_er( code_base, len ) ;
      if ( file->name == 0 )
      {
         file4close( file ) ;
         return e4memory ;
      }
      u4ncpy( file->name, name, len ) ;
      file->do_alloc_free = 1 ;
   }
   else
      file->name = name ;

   file->is_exclusive = (char)code_base->exclusive ;
   file->file_created = 1 ;
   return 0 ;
}
