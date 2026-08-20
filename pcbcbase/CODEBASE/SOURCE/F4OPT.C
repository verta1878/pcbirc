/* f4opt.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION file4optimize( FILE4 *file, int opt_flag, int file_type )
{
   #ifdef S4OPTIMIZE_OFF
      return 0 ;
   #else
      OPT4 *opt ;
      int rc ;

      #ifdef S4DEBUG
         if ( file == 0 || file_type < 0 || file_type > 3 || opt_flag < -1 || opt_flag > 1 )
            e4severe( e4parm, E4_F4OPTIMIZE ) ;
      #endif

      rc = 0 ;

      opt = &file->code_base->opt ;

      if ( opt_flag == -1 )
      #ifdef S4SINGLE
         opt_flag = 1 ;
      #else
         opt_flag = ( file->is_exclusive || file->is_read_only ) ? 1 : 0 ;
      #endif

      if ( opt_flag == 1 )
      {
         if ( file->do_buffer != 0 && file->type != OPT4NONE )  /* already optimized */
            return 0 ;
         if ( opt->num_buffers > 0 )
         {
            file->len = -1 ;
            file->hash_init = opt->hash_trail * opt->block_size ;
            #ifdef S4DEBUG
               if ( file4len( file ) < 0 || opt->block_size == 0 )
                  e4severe( e4info, E4_F4OPTIMIZE ) ;
            #endif
            opt->hash_trail = (opt->hash_trail + file4len( file ) / opt->block_size) % opt->num_blocks ;
            file->do_buffer = 1 ;
         }
         else
            file->hash_init = - 1 ;

         if ( file->type == OPT4NONE )   /* add to list... */
            l4add( &opt->opt_files, file ) ;
         file->type = (char) file_type ;
         rc = file4optimize_write( file, file->code_base->optimize_write ) ;
      }
      else  /* 0 */
      {
         if ( file->type == OPT4NONE )   /* not optimized */
            return 0 ;
         rc = file4optimize_write( file, 0 ) ;
         if ( rc == 0 )
         {
            if ( file4low_flush( file, 1 ) < 0 )
               return e4( file->code_base, e4opt_flush, 0 ) ;
            l4remove( &opt->opt_files, file ) ;
            file->type = OPT4NONE ;
            file->do_buffer = 0 ;
         }
      }

      return rc ;
   #endif
}

/* opt_flag has the same definitions as C4CODE.optimize_write */
int S4FUNCTION file4optimize_write( FILE4 *file, int opt_flag )
{
   #ifdef S4OPTIMIZE_OFF
      return 0 ;
   #else
      int rc ;

      rc = 0 ;

      #ifdef S4DEBUG
         if( file == 0 || opt_flag < -1 || opt_flag > 1 )
            e4severe( e4parm, E4_F4OPTIMIZE_WR ) ;
      #endif

      if ( opt_flag == file->write_buffer || file->do_buffer == 0 )
         return rc ;

      switch ( opt_flag )
      {
         case 0:
            rc = file4flush( file ) ;
            break ;
         case  -1 :
            #ifdef S4SINGLE
               file->buffer_writes = 1 ;
            #else
               if ( file->is_exclusive == 0 )
               {
                  rc = file4flush( file ) ;
                  file->buffer_writes = 0 ;
               }
               else
                  file->buffer_writes = 1 ;
            #endif
            break ;
         case 1:
            #ifndef S4SINGLE
               if ( file->is_exclusive == 1 )
            #endif
            file->buffer_writes = 1 ;
            break ;
         default:
            return 0 ;
      }
      file->write_buffer = (char)opt_flag ;

      return rc ;
   #endif
}

/* tries to actually turn on/off the write bufferring when locking/unlocking a file */
void S4FUNCTION file4set_write_opt( FILE4 *f4, int set_opt )
{
   #ifndef S4OPTIMIZE_OFF
      if ( set_opt == f4->buffer_writes )
         return ;
      if ( set_opt == 1 && f4->write_buffer == 1 )
         f4->buffer_writes = 1 ;
      if ( set_opt == 0 && f4->write_buffer == 1 )
         f4->buffer_writes = 0 ;
   #endif
}
