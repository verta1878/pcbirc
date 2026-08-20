/* d4close.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifndef S4MEMO_OFF
   extern char f4memo_null_char ;
#endif  /* not S4MEMO_OFF */

int S4FUNCTION d4close( DATA4 *data )
{
   int final_rc, i ;
   INDEX4 *index_on, *index_next ;
   CODE4 *code_base ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4CLOSE ) ;
   #endif

   code_base = data->code_base ;
   final_rc = e4set( code_base, 0 ) ;

   if ( final_rc == e4unique )
      data->record_changed = 0 ;

   if ( file4open_test( &data->file ) )
   {
      #ifndef S4OFF_WRITE
         if ( d4update( data ) < 0 )
            final_rc = e4set( code_base, 0 ) ;
      #endif

      if ( final_rc == e4unique )
         data->record_changed = 0 ;

      #ifndef S4OFF_WRITE
         if ( data->file_changed )
            if ( d4update_header( data, 1, 0 ) < 0 )  /* update time only, no locking reqd. */
               final_rc = e4set( code_base, 0 ) ;
      #endif

      #ifndef S4SINGLE
         if ( d4unlock( data ) < 0 )
            final_rc = e4set( code_base, 0 ) ;
      #endif  /* S4SINGLE */
   }

   #ifndef S4INDEX_OFF
      for( index_next = (INDEX4 *)l4first( &data->indexes );; )
      {
         index_on = index_next ;
         index_next = (INDEX4 *)l4next( &data->indexes, index_on ) ;
         if ( !index_on )
            break ;

         if ( i4close( index_on ) < 0 )
            final_rc = e4set( code_base, 0 ) ;
      }
   #endif

   if ( file4open_test( &data->file ) )
      l4remove( &code_base->data_list, data ) ;

   if ( file4close( &data->file ) < 0 )
      final_rc = e4set( code_base, 0 ) ;

   u4free( data->record ) ;
   u4free( data->record_old ) ;
   u4free( data->fields )  ;

   #ifndef S4MEMO_OFF
      if ( data->fields_memo != 0 )
      {
         for ( i = 0; i < data->n_fields_memo ; i++ )
            if ( data->fields_memo[i].contents != &f4memo_null_char )
               u4free( data->fields_memo[i].contents ) ;
         u4free( data->fields_memo ) ;
      }

      if ( file4open_test( &data->memo_file.file ) )
         if ( file4close( &data->memo_file.file ) < 0 )
            final_rc = e4set( code_base, 0 ) ;
   #endif  /* S4MEMO_OFF */

   #ifndef S4SINGLE
      if ( data->n_locks > 1 )
         u4free( data->locks ) ;
   #endif  /* S4SINGLE */

   mem4free( code_base->data_memory, data ) ;

   e4set( code_base, final_rc ) ;
   return final_rc ;
}
