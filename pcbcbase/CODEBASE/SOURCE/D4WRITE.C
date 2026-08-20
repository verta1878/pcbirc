/* d4write.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TURBOC__ */
#endif  /* S4UNIX */

#ifndef S4OFF_WRITE
int S4FUNCTION d4write( DATA4 *d4, long rec )
{
   int rc, final_rc, i, old ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4WRITE ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( rec < 1 || d4 == 0 )
         e4severe( e4parm, E4_D4WRITE ) ;
   #endif  /* S4DEBUG */

   if ( d4->code_base->error_code < 0 )
      return -1 ;

   /* 0. Validate memo id's */
   /* 1. Update Keys */
   /* 2. Update Memo Information */
   /* 3. Update Data FILE4 */

   old = d4->record_changed ;
   d4->record_changed = 0 ;

   #ifndef S4MEMO_OFF
      if ( d4->n_fields_memo > 0 )
         if ( ( rc = d4validate_memo_ids( d4 ) ) != 0 )
         {
            d4->record_changed = old ;
            return rc ;
         }
   #endif  /* S4MEMO_OFF */
   #ifndef S4INDEX_OFF
      rc = d4write_keys( d4, rec ) ;
   #endif
   d4->record_changed = old ;
   #ifndef S4INDEX_OFF
      if ( rc )
         return rc ;
   #endif

   final_rc = 0 ;

   #ifndef S4MEMO_OFF
      /* First cycle through the fields to be flushed */
      for ( i = 0; i < d4->n_fields_memo; i++ )
      {
         rc = f4memo_update( d4->fields_memo[i].field) ;
         if ( rc < 0 )
            return -1 ;
         if ( rc > 0 )
            final_rc = rc ;
      }
   #endif  /* S4MEMO_OFF */

   if ( d4write_data( d4, rec ) < 0 )
      return -1 ;
   return final_rc ;
}

int S4FUNCTION d4write_data( DATA4 *d4, long rec )
{
   int rc ;

   #ifdef S4DEBUG
      if ( rec < 1 || d4 == 0 )
         e4severe( e4parm, E4_D4WRITE_DATA ) ;
   #endif  /* S4DEBUG */

   if ( d4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4SINGLE
      rc = d4lock( d4, rec ) ;
      if ( rc )
         return rc ;
   #endif  /* S4SINGLE */

   d4->record_changed = 0 ;
   d4->file_changed = 1 ;
   return file4write( &d4->file, d4record_position(d4, rec), d4->record, d4->record_width ) ;
}

int S4FUNCTION d4write_keys( DATA4 *d4, long rec )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      unsigned char  new_key_buf[I4MAX_KEY_SIZE] ;
      char *old_key, *temp_ptr, *save_rec_buffer ;
      int rc, rc2, save_error, key_len, old_key_added, add_new_key, index_locked ;
      TAG4 *tag_on ;

      #ifdef S4DEBUG
         if ( rec < 1 || d4 == 0 )
            e4severe( e4parm, E4_D4WRITE_KEYS ) ;
      #endif  /* S4DEBUG */

      d4->bof_flag = d4->eof_flag = 0 ;

      #ifndef S4OPTIMIZE_OFF
         #ifndef S4DETECT_OFF
            d4->code_base->mode |= 0x20 ;
         #endif  /* S4DETECT */
      #endif

      #ifndef S4SINGLE
         rc = d4lock(d4, rec) ;
         if ( rc )
            return rc ;
      #endif  /* S4SINGLE */

      if ( d4->indexes.n_link > 0 )
      {
         if ( d4read_old( d4, rec ) < 0 )
            return -1 ;
         if ( u4memcmp( d4->record_old, d4->record, d4->record_width) == 0 )
            return 0 ;
      }

      save_rec_buffer = d4->record ;
      rc = 0 ;
      index_locked = d4lock_test_index( d4 ) ? 2 : 0 ;  /* 2 means was user locked */

      for( tag_on = 0 ;; )
      {
         tag_on = d4tag_next( d4, tag_on ) ;
         if ( tag_on == 0 )
            break ;

         old_key_added = add_new_key = 1 ;

         key_len = t4expr_key( tag_on, &temp_ptr ) ;
         if ( key_len < 0 )
         {
            rc = -1 ;
            break ;
         }
         #ifdef S4DEBUG
            if ( key_len != tag_on->header.key_len || key_len > I4MAX_KEY_SIZE)
               e4severe( e4result, (char *) 0 ) ;
         #endif

         memcpy( new_key_buf, temp_ptr, key_len ) ;

         if ( tag_on->filter )
            add_new_key = expr4true( tag_on->filter ) ;

         d4->record = d4->record_old ;

         if ( tag_on->filter )
            old_key_added = expr4true( tag_on->filter ) ;
         key_len = t4expr_key( tag_on, &old_key ) ;

         d4->record = save_rec_buffer ;

         if ( key_len < 0 )
         {
            rc = key_len ;
            break ;
         }
         if ( old_key_added == add_new_key )   
            if ( u4memcmp( new_key_buf, old_key, key_len) == 0 )
               continue ;

         #ifndef S4SINGLE
            if ( index_locked == 0 )
            {
               index_locked = 1 ;
               rc = d4lock_index( d4 ) ;
               if ( rc )
                  break ;
            }
         #endif  /* S4SINGLE */

         if ( old_key_added )
            if( t4remove( tag_on, old_key, rec ) < 0 )
            {
               rc = -1 ;
               break ;
            }

         if ( add_new_key )
         {
            rc = t4add( tag_on, new_key_buf, rec ) ;
            if ( rc == r4unique || rc == e4unique )
            {
               save_error = e4set(d4->code_base,0) ;

               if ( old_key_added )
                  if ( t4add( tag_on, (unsigned char *)old_key, rec ) < 0 )
                  {
                     rc = -1 ;
                     break ;
                  }
      
               /* Remove the keys which were just added */
               for(;;)
               {
                  tag_on = d4tag_prev( d4, tag_on ) ;
                  if ( tag_on == 0 )
                     break ;
      
                  d4->record = save_rec_buffer ;
      
                  add_new_key = 1 ;
                  if ( tag_on->filter )
                     if ( !expr4true( tag_on->filter ) )
                        add_new_key = 0 ;
      
                  rc2 = t4remove_calc( tag_on, rec ) ;
                  if ( rc2 < 0 )
                  {
                     rc = rc2 ;
                     break ;
                  }
      
                  d4->record = d4->record_old ;
      
                  old_key_added = 1 ;
                  if ( tag_on->filter )
                     if ( !expr4true( tag_on->filter ) )
                        old_key_added = 0 ;
      
                  if ( old_key_added )
                  {
                     rc2 = t4add_calc( tag_on, rec ) ;
                     if ( rc2 < 0 )
                     {
                        d4->record = save_rec_buffer ;
                        rc = rc2 ;
                        break ;
                     }
                  }
               }

               d4->record = save_rec_buffer ;
               e4set( d4->code_base, save_error ) ;
               if ( save_error < 0 )
                  rc = save_error ;
               break ;
            }
            if( rc < 0 )
            {
               rc = -1 ;
               break ;
            }
            rc = 0 ;
         }
      }
      if ( index_locked == 1 )
         d4unlock_index( d4 ) ;

      d4->rec_num_old = -1 ;
      return rc ;
   #endif  /* S4OFF_INDEX */
}
#endif  /* S4OFF_WRITE */
