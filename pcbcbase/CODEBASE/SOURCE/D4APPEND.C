/* d4append.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifndef S4OFF_WRITE
int S4FUNCTION d4append( DATA4 *data )
{
   int rc, save_error, i ;
   long  new_id ;
   #ifndef S4INDEX_OFF
      TAG4 *tag_on ;
      #ifndef S4SINGLE
         int index_locked ;
      #endif
   #endif
   #ifndef S4MEMO_OFF
      F4MEMO *mfield ;
   #endif  /* not S4MEMO_OFF */

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4APPEND ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   #ifdef S4DEBUG
      if ( data->rec_num )
      {
         e4describe( data->code_base, e4result, E4_D4APPEND, E4_RESULT_D4A, 0 ) ;
         return -1 ;
      }
   #endif  /* S4DEBUG */

   /* 0. Lock record count bytes
      1. Update index file
      2. Update memo File
      3. Update data file */

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         data->code_base->mode |= 0x10 ;
      #endif  /* S4DETECT_OFF */
   #endif

   #ifndef S4SINGLE
      rc = d4lock_append( data ) ;
      if ( rc )
         return rc ;
      data->num_recs = -1 ;
      #ifndef S4OPTIMIZE_OFF
         data->code_base->opt.force_current = 1 ;  /* force the reccount to be current */
      #endif
      rc = d4lock( data, d4reccount( data ) + 1 ) ;
      #ifndef S4OPTIMIZE_OFF
         data->code_base->opt.force_current = 0 ;
      #endif
      if ( rc )
      {
         d4unlock_append( data ) ;
         return rc ;
      }
   #endif  /* S4SINGLE */

   data->bof_flag = data->eof_flag = 0 ;
   data->record_changed = 0 ;

   data->rec_num = d4reccount( data ) + 1 ;

   #ifndef S4INDEX_OFF
      #ifndef S4SINGLE
         index_locked = d4lock_test_index( data ) ;
         if ( !index_locked )
         {
            rc = d4lock_index( data ) ;
            if ( rc )
            {
               d4unlock_append( data ) ;
               return rc ;
            }
         }
      #endif  /* not S4SINGLE */

      for( tag_on = 0 ;; )
      {
         tag_on = d4tag_next( data, tag_on ) ;
         if ( !tag_on )
            break ;

         rc = t4add_calc( tag_on, data->rec_num ) ;
         if ( rc < 0 || rc == r4unique )
         {
            save_error = e4set( data->code_base, 0 ) ;

            /* Remove the keys which were just added */
            for(;;)
            {
               tag_on = d4tag_prev( data, tag_on ) ;
               if ( !tag_on )
                  break ;
               t4remove_calc( tag_on, data->rec_num ) ;
            }

            e4set( data->code_base, save_error ) ;
            data->rec_num = 0 ;
            #ifndef S4SINGLE
               d4unlock_append( data ) ;
               if ( !index_locked )
                  d4unlock_index( data ) ;
            #endif
            return rc ;
         }
      }

      #ifndef S4SINGLE
         if ( !index_locked )
            d4unlock_index( data ) ;
      #endif  /* not S4SINGLE */
   #endif  /* not S4INDEX_OFF */

   #ifndef S4MEMO_OFF
      for ( i = 0 ; i < data->n_fields_memo ; i++ )
      {
         mfield = data->fields_memo+i ;
         mfield->is_changed = 0 ;
         if ( mfield->len > 0 )
         {
            new_id = 0L ;
            if ( memo4file_write( &data->memo_file, &new_id, mfield->contents, mfield->len ) )  /* positive value means fail, so don't update field, but append record */
               break ;
            f4assign_long( mfield->field, new_id ) ;
         }
         else
            f4assign( mfield->field, " " ) ;
      }
   #endif  /* S4MEMO_OFF */

   rc = d4append_data( data ) ;
   if ( !rc )
      rc = d4update_header( data, 1, 1 ) ;

   #ifndef S4SINGLE
      d4unlock_append( data ) ;
   #endif  /* S4SINGLE */

   return rc ;
}

int S4FUNCTION d4append_blank( DATA4 *data )
{
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4APPEND_BL ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND_BL ) ;
   #endif

   #ifdef S4DEMO
      if ( d4reccount( data ) >= 200L)
      {
         e4( data->code_base, e4demo, 0 ) ;
         d4close( data ) ;
         return 0 ;
      }
   #endif  /* S4DEMO */

   rc = d4append_start( data, 0 ) ;  /* updates the record, returns -1 if code_base->error_code < 0 */
   if ( rc )
      return rc ;

   memset( data->record, ' ', data->record_width ) ;
   return d4append( data ) ;
}

int S4FUNCTION d4append_data( DATA4 *data )
{
   long count, pos ;
   int  rc ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND_DATA ) ;
   #endif

   #ifdef S4DEMO
      if ( d4reccount( data ) >= 200L)
      {
         e4( data->code_base, e4demo, 0 ) ;
         d4close( data ) ;
         return 0 ;
      }
   #endif  /* S4DEMO */

   count = d4reccount( data ) ;  /* returns -1 if code_base->error_code < 0 */
   if ( count < 0L ) 
      return -1 ;
   data->file_changed = 1 ;
   pos = d4record_position( data, count + 1L ) ;
   data->record[data->record_width] = 0x1A ;

   rc = file4write( &data->file, pos, data->record, ( data->record_width + 1 ) ) ;
   if ( rc >= 0 )
   {
      data->rec_num = count + 1L ;
      data->record_changed = 0 ;
      #ifndef S4SINGLE
         if ( d4lock_test_append( data ) )
      #endif  /* S4SINGLE */
      data->num_recs = count + 1L ;
   }

   data->record[data->record_width] = 0 ;

   return rc ;
}

int S4FUNCTION d4append_start( DATA4 *data, int use_memo_entries )
{
   int rc, i ;
   char *save_ptr ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4APPEND_STRT ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND_STRT ) ;
   #endif

   #ifdef S4DEMO
      if ( d4reccount( data ) >= 200L)
      {
         e4( data->code_base, e4demo, 0 ) ;
         d4close( data ) ;
         return 0 ;
      }
   #endif  /* S4DEMO */

   rc = d4update_record( data, 1 ) ;   /* returns -1 if code_base->error_code < 0 */
   if ( rc )
      return rc ;

   if ( data->rec_num <= 0 )
      use_memo_entries = 0 ;

   #ifndef S4MEMO_OFF
      if ( use_memo_entries && data->n_fields_memo > 0 )
      {
         #ifdef S4DEBUG
            if ( !file4open_test( &data->memo_file.file ) )
               e4severe( e4data, E4_DATA_MEM ) ;
         #endif  /* S4DEBUG */

         /* Read in the current memo entries of the current record */
         #ifndef S4SINGLE
            rc = d4lock( data, data->rec_num ) ;
            if ( rc )
               return rc ;
         #endif  /* S4SINGLE */

         save_ptr = data->record ;
         data->record = data->record_old ;

         d4go_data( data, data->rec_num ) ;

         for ( i = 0 ; i < data->n_fields_memo ; i++ )
            f4memo_read_low( data->fields_memo[i].field ) ;

         data->record = save_ptr ;

         if ( data->code_base->error_code < 0 )
            return -1 ;
      }

      for ( i = 0 ; i < data->n_fields_memo ; i++ )
         f4assign_long( data->fields_memo[i].field, 0 ) ;
   #endif  /* not S4MEMO_OFF */

   #ifndef S4SINGLE
      rc = d4unlock_data( data ) ;
      if ( rc )
         return rc ;
   #endif

   data->rec_num = 0 ;
   return 0 ;
}

#endif  /* S4OFF_WRITE */
