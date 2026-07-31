/* d4go.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION d4go( DATA4 *data, long rec )
{
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4GO ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4GO ) ;
   #endif

   #ifdef S4OFF_WRITE
      if ( data->code_base->error_code < 0 )
         return -1 ;
   #else
      rc = d4update_record( data, 0 ) ;   /* returns -1 if code_base->error_code < 0 */
      if ( rc )
         return rc ;
   #endif

   #ifndef S4SINGLE
      if ( !d4lock_test( data, rec ) )  /* record not already locked */
      {
         rc = d4unlock_data( data ) ;
         if ( rc )
            return rc ;

         if ( data->code_base->read_lock )
         {
            rc = d4lock( data, rec ) ;
            if ( rc )
               return rc ;
         }
      }

      #ifndef S4OPTIMIZE_OFF
         /* if bufferred old, and only record is locked, read from disk */
         if ( !data->memo_validated && data->file.do_buffer && !data->file_lock )
            if ( d4lock_test( data, rec ) )
               data->code_base->opt.force_current = 1 ;
      #endif
   #endif

   rc = d4go_data( data, rec ) ;

   #ifndef S4SINGLE
      #ifndef S4OPTIMIZE_OFF
         data->code_base->opt.force_current = 0 ;
      #endif
   #endif

   if ( rc )
      return rc ;

   data->bof_flag = data->eof_flag = 0 ;

   #ifndef S4SINGLE
      if ( d4lock_test( data, rec ) )
      {
   #endif
      memcpy( data->record_old, data->record, data->record_width ) ;
      data->rec_num_old = data->rec_num ;
      data->memo_validated = 1 ;
   #ifndef S4SINGLE
      }
      else
         data->memo_validated = 0 ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         data->code_base->mode |= 0x01 ;
      #endif
   #endif

   return rc ;
}

int S4FUNCTION d4go_data( DATA4 *data, long rec )
{
   unsigned len ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4GO_DATA ) ;
   #endif

   if( data->code_base->error_code < 0 )
      return -1 ;

   if ( rec <= 0 )
      len = 0 ;
   else
   {
      len = file4read( &data->file, d4record_position( data, rec ) ,data->record, data->record_width ) ;
      if( data->code_base->error_code < 0 )
         return -1 ;
   }

   if ( len != data->record_width )
   {
      memset( data->record, ' ', data->record_width ) ;  /* clear the partially read record to avoid corruption */
      data->rec_num = -1 ;  /* at an invalid position */
      if ( data->code_base->go_error )
         return e4( data->code_base, e4read, data->file.name ) ;
      return r4entry ;
   }
   data->rec_num = rec ;
   return 0 ;
}

int  S4FUNCTION d4go_eof( DATA4 *data )
{
   int rc ;
   long count ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4GO_EOF ) ;
   #endif

   #ifdef S4OFF_WRITE
      if ( data->code_base->error_code < 0 )
         return -1 ;
   #else
      rc = d4update_record( data, 1 ) ;   /* returns -1 if code_base->error_code < 0 */
      if ( rc )
         return rc ;
   #endif

   count = d4reccount( data ) ;
   if ( count < 0 )
      return -1 ;
   data->rec_num = count + 1L ;
   data->eof_flag = 1 ;
   if ( data->rec_num == 1 )
      data->bof_flag = 1 ;
   memset( data->record, ' ', data->record_width ) ;
   return r4eof ;
}
