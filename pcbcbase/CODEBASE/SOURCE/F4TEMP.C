/* f4temp.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

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

int S4FUNCTION file4temp( FILE4 *file, CODE4 *code_base, char *buf, int auto_remove )
{
   int i, save_flag, rc, old_excl ;
   #ifndef S4OPTIMIZE_OFF
      int old_opt_wr, tf_do_buffer ;
      unsigned long tf_hash_init ;
      char tf_type, tf_buffer_writes, tf_write_buffer ;
   #endif
   long tf_len ;
   LINK4 tf_link ;
   time_t t ;
   char name[20], *name_ptr ;

   #ifdef S4DEBUG
      if ( file == 0 || code_base == 0  )
         e4severe( e4parm, E4_F4TEMP ) ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      if ( auto_remove && code_base->opt.num_buffers != 0 )
      {
         if ( code_base->opt.force_current == 0 )
         {
            memset( (void *)file, 0, sizeof( FILE4 ) ) ;
            file->is_temp = 1 ;
            file->code_base = code_base ;
            file->file_created = 0 ;
            file->is_exclusive = 1 ;
            old_opt_wr = code_base->optimize_write ;
            code_base->optimize_write = 1 ;
            file4optimize( file, 1, OPT4OTHER ) ;
            code_base->optimize_write = old_opt_wr ;
            if ( buf != 0 )
               file->name = buf ;
            return 0 ;
         }
         else
         {
            tf_hash_init = file->hash_init ;
            tf_len = file->len ;
            tf_type = file->type ;
            tf_buffer_writes = file->buffer_writes ;
            tf_do_buffer = file->do_buffer ;
            tf_write_buffer = file->write_buffer ;
            memcpy( (void *)&tf_link, (void *)&file->link, sizeof( LINK4 ) ) ;
         }
      }
   #endif

   name_ptr = buf ;
   if ( name_ptr == 0 )
      name_ptr = name ;

   for ( i = 0 ; i < 100 ; i++ )
   {
      u4delay_sec() ;
      time( &t ) ;
      t %= 10000L ;

      strcpy( name_ptr, "TEMP" ) ;
      c4ltoa45( t, name_ptr + 4, -4 ) ;
      strcpy( name_ptr + 8, ".TMP" ) ;

      save_flag = code_base->create_error ;
      code_base->create_error = 0 ;

      old_excl = code_base->exclusive ;
      code_base->exclusive = 1 ; /* all temporary files are for exclusive access only */
      rc = file4create( file, code_base, name_ptr, (int)(buf == 0) ) ;
      code_base->exclusive = old_excl ;

      code_base->create_error = save_flag ;
      if ( rc < 0 )
         return -1 ;
      if ( rc == 0 )
      {
         if ( auto_remove )
            file->is_temp = 1 ;
         #ifndef S4OPTIMIZE_OFF
            if ( auto_remove && code_base->opt.num_buffers != 0 )
            {
               if ( code_base->opt.force_current == 1 )
               {
                  file->hash_init = tf_hash_init ;
                  file->len = tf_len ;
                  file->type = tf_type ;
                  file->buffer_writes = tf_buffer_writes ;
                  file->do_buffer = tf_do_buffer ;
                  file->write_buffer = tf_write_buffer ;
                  memcpy( (void *)&file->link, (void *)&tf_link, sizeof( LINK4 ) ) ;
               }
               else
               {
                  old_opt_wr = code_base->optimize_write ;
                  code_base->optimize_write = 1 ;
                  file4optimize( file, 1, OPT4OTHER ) ;
                  code_base->optimize_write = old_opt_wr ;
               }
            }
         #endif
         return  0 ;
      }
   }

   return e4( code_base, e4create, E4_CREATE_TEM ) ;
}


