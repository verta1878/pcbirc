/* f4flush.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4WINDOWS
   #include <dos.h>
#endif

/* returns 1 if no handles available */
int S4FUNCTION file4low_flush( FILE4 *file, int do_free )
{
   int rc, handle ;
   #ifndef S4OPTIMIZE_OFF
      long real_len ;
   #endif

   #ifdef S4DEBUG
      if ( file == 0 || do_free < 0 || do_free > 1 )
         e4severe( e4parm, E4_F4LOW_FLUSH ) ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      if ( file->do_buffer == 1 )
      {
         rc = opt4file_flush_list( file, opt4file_find_list( file ), do_free ) ;
         if ( rc != -1 && file->code_base->opt.write_file == file )  /* flush out the final file changes */
            rc = opt4flush_write_buffer( &file->code_base->opt ) ;
         if ( file->len != -1 )
         {
            file->do_buffer = 0 ;
            real_len = file->len ;
            if ( file4len( file ) != real_len )   /* make sure the length is updated */
               file4len_set( file, real_len ) ;
            file->do_buffer = 1 ;
         }
         if ( rc )
            return rc ;
      }
   #endif

   handle = dup( file->hand ) ;
   if ( handle < 0 )  /* means that there are no available handles */
      return 1 ;
   if ( close( handle ) < 0 )
      return -1 ;

   return 0 ;
}

int S4FUNCTION file4flush( FILE4 *file )
{
   int rc ;

   #ifdef S4DEBUG
      if ( file == 0 )
         e4severe( e4parm, E4_F4FLUSH ) ;
   #endif
   rc = file4low_flush( file, 0 ) ;

   if ( rc == 1 )
      return e4( file->code_base, e4opt_flush, 0 ) ;

   return rc ;
}

int S4FUNCTION file4refresh( FILE4 *file )
{
   #ifndef S4OPTIMIZE_OFF
      #ifndef S4SINGLE
         int rc ;
         #ifdef S4DEBUG
            if ( file == 0 )
               e4severe( e4parm, E4_F4REFRESH ) ;
         #endif

         if ( file->do_buffer == 0 || file->is_exclusive == 1 || file->is_read_only == 1 )
            return 0 ;

         rc = file4low_flush( file, 1 ) ;
         if ( rc < 0 )
            return rc ;
         file->len = -1 ;
      #endif
   #endif
   return 0 ;
}


