/* d4flush.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
int S4FUNCTION d4changed( DATA4 *data, int flag )
{
   int previous ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4CHANGED ) ;
   #endif

   previous = data->record_changed ;

   if ( flag >= 0 )
      data->record_changed = ( flag > 0 ) ;
   return previous ;
}
#endif

int S4FUNCTION d4flush( DATA4 *data )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
      int rc ;
      #ifndef S4INDEX_OFF
         #ifndef N4OTHER
            INDEX4 *index_on ;
         #else
            TAG4 *tag_on ;
         #endif
      #endif

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4FLUSH ) ;
      #endif

      rc = d4flush_data( data ) ;

      #ifndef S4INDEX_OFF
         #ifndef N4OTHER
            index_on = (INDEX4 *)l4first( &data->indexes ) ;
            if ( index_on )
               do
               {
                  if ( i4flush( index_on ) )
                     rc = -1 ;
                  index_on = (INDEX4 *)l4next( &data->indexes, index_on ) ;
               } while ( index_on != 0 ) ;
         #else
            for( tag_on = 0 ;; )
            {
               tag_on = d4tag_next( data, tag_on ) ;
               if ( !tag_on )
                  break ;
               if ( t4flush( tag_on ) )
                  rc = - 1 ;
            }
         #endif
      #endif

      return rc ;
   #endif
}

int S4FUNCTION d4flush_data( DATA4 *data )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
      int rc ;

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, "d4flush_data()" ) ;
      #endif

      rc = d4update_record( data, 0 ) ;   /* returns -1 if code_base->error_code < 0 */

      #ifndef S4OPTIMIZE_OFF
         file4flush( &data->file ) ;
         #ifndef S4MEMO_OFF
            if ( data->n_fields_memo > 0 && data->memo_file.file.hand != -1 )
               file4flush( &data->memo_file.file ) ;
         #endif
      #endif
      return rc ;
   #endif
}

int S4FUNCTION d4flush_files( CODE4 *c4 )
{
   #ifdef S4OFF_WRITE
      return 0 ;
   #else
      DATA4 *data_on ;
      int rc, rc_return ;

      #ifdef S4DEBUG
         if ( c4 == 0 )
            e4severe( e4parm, "d4flush_files()" ) ;
      #endif

      rc = rc_return = 0 ;

      data_on = (DATA4 *)l4first( &c4->data_list ) ;
      while ( data_on ) 
      {
         rc = d4flush( data_on ) ;   /* returns -1 if code_base->error_code < 0 */
         if ( rc )
            rc_return = rc ;
         data_on = (DATA4 *)l4next( &c4->data_list, data_on) ;
      }

      return rc_return ;
   #endif
}

#ifndef S4OFF_WRITE
int S4FUNCTION d4update( DATA4 *data )
{
   int rc ;
   #ifndef S4INDEX_OFF
      #ifdef S4FOX
         INDEX4 *index_on ;
      #else
         TAG4 *tag_on ;
      #endif
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, "d4update():" ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, "d4update()" ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   rc = d4update_record( data, 0 ) ;
   #ifndef S4INDEX_OFF
      if ( !rc )
      {
         #ifdef S4FOX
            index_on = (INDEX4 *)l4first( &data->indexes ) ;
            if ( index_on )
            do
            {
               if ( i4update( index_on ) < 0 )
                  rc = -1 ;
               index_on = (INDEX4 *)l4next( &data->indexes, index_on ) ;
            } while ( index_on != 0 ) ;
         #else
            for( tag_on = 0 ;; )
            {
               tag_on = d4tag_next( data, tag_on ) ;
               if ( !tag_on )
                  break ;
               if ( t4update(tag_on) < 0 )
                  rc = -1 ;
            }
         #endif
      }
   #endif

   return rc ;
}

int S4FUNCTION d4update_record( DATA4 *data, int do_unlock )
{
   int i, rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, "d4update_record():" ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, "d4update_record()" ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   if ( data->rec_num <= 0 || d4eof( data ) )
   {
      #ifndef S4MEMO_OFF
         for ( i = 0; i < data->n_fields_memo; i++ )
            f4memo_reset( data->fields_memo[i].field ) ;
      #endif
      data->record_changed = 0 ;
      return 0 ;
   }

   if ( data->record_changed )
   {
      rc = d4write( data, data->rec_num ) ;
      if ( rc )
         return rc ;
   }

   #ifndef S4MEMO_OFF
      for ( i = 0; i < data->n_fields_memo; i++ )
         f4memo_reset( data->fields_memo[i].field ) ;
   #endif

   data->rec_num_old = -1 ;
   if ( do_unlock )
      d4unlock_records( data ) ;
   return 0 ;
}
#endif  /* S4OFF_WRITE */

