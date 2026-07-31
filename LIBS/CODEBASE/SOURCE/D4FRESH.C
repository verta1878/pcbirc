/* d4fresh.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION d4refresh( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifndef S4OPTIMIZE_OFF
         INDEX4 *index_on ;
         #ifdef N4OTHER
            TAG4 *tag_on ;
         #endif

         #ifdef S4DEBUG
            if ( data == 0 )
               e4severe( e4parm, E4_D4REFRESH ) ;
         #endif

         file4refresh( &data->file ) ;
         if ( data->n_fields_memo > 0 && data->memo_file.file.hand != -1 )
            file4refresh( &data->memo_file.file ) ;
         for ( index_on = 0 ;; )
         {
            index_on = (INDEX4 *) l4next( &data->indexes, index_on ) ;
            if ( index_on == 0 )
               break ;
            #ifdef N4OTHER
               for( tag_on = 0 ;; )
               {
                  tag_on = (TAG4 *)l4next( &index_on->tags, tag_on) ;
                  if ( tag_on == 0 )
                     break ;
                  file4refresh( &tag_on->file ) ;
               }
            #else
               file4refresh( &index_on->file ) ;
            #endif
         }
         if ( data->code_base->error_code < 0 )
            return -1 ;
      #endif
   #endif
   return 0 ;
}

int S4FUNCTION d4refresh_record( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifndef S4OPTIMIZE_OFF
         #ifndef S4MEMO_OFF
            int i ;
         #endif
         OPT4 *opt ;
         int rc ;

         #ifdef S4DEBUG
            if ( data == 0 )
               e4severe( e4parm, E4_D4REFRESH_REC ) ;
         #endif

         opt = &data->code_base->opt ;

         if ( data->file_lock || data->file.is_exclusive || opt == 0 || data->rec_num <= 0L || data->rec_num > d4reccount( data ) )
            return 0 ;

         if ( data->file.do_buffer )  /* also makes sure 'opt' should exist */
            opt->force_current = 1 ;

         #ifndef S4MEMO_OFF
            if ( data->n_fields_memo > 0 && data->memo_file.file.hand != -1 )
            {
               file4refresh( &data->memo_file.file ) ;
               for ( i = 0; i < data->n_fields_memo; i++ )
                  f4memo_reset( data->fields_memo[i].field ) ;
            }
         #endif

         data->record_changed = 0 ;
         rc = d4go_data( data, data->rec_num  ) ;
         if ( rc )
            return rc ;
         data->bof_flag = data->eof_flag = 0 ;

         if ( data->file.do_buffer )  /* also makes sure 'opt' should exist */
            opt->force_current = 0 ;

         #ifndef S4DETECT_OFF
            data->code_base->mode |= 0x01 ;
         #endif

         if ( d4lock_test( data, data->rec_num ) )
         {
            memcpy( data->record_old, data->record, data->record_width ) ;
            data->rec_num_old = data->rec_num ;
            data->memo_validated = 1 ;
         }
         else
            data->memo_validated = 0 ;
         if ( data->code_base->error_code < 0 )
            return -1 ;
      #endif
   #endif
   return 0 ;
}
