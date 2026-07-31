 /* d4unlock.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION d4unlock( DATA4 *d4 )
{
   #ifndef S4SINGLE
      int rc ;
      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK ) ;
      #endif

      #ifndef S4OFF_WRITE
         rc =  d4update( d4 ) ;  /* returns -1 if code_base->error_code < 0 */
         if ( rc < 0 )
            return -1 ;
      #endif

      d4unlock_data( d4 ) ;
      #ifndef N4OTHER
         #ifndef S4MEMO_OFF
            if ( d4->n_fields_memo > 0 && d4->memo_file.file.hand != -1 )
               memo4file_unlock( &d4->memo_file ) ;
         #endif
      #endif
      d4unlock_index( d4 ) ;
      if ( d4->code_base->error_code < 0 )
         return -1 ;
      return rc ;
   #else
      return 0 ;
   #endif
}

/* only unlocks the append byte */
int S4FUNCTION d4unlock_append( DATA4 *d4 )
{
   #ifndef S4SINGLE
      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK_AP ) ;
      #endif
      if ( d4->append_lock )
      {
         if ( file4unlock( &d4->file, L4LOCK_POS, 1L ) < 0 )
            return -1 ;
         d4->append_lock =  0 ;
         d4->num_recs = -1 ;
      }
      if ( d4->code_base->error_code < 0 )
         return -1 ;
   #endif
   return 0 ;
}

int S4FUNCTION d4unlock_data( DATA4 *d4 )
{
   #ifndef S4SINGLE
      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK_DATA ) ;
      #endif
      d4unlock_file( d4 ) ;
      d4unlock_append( d4 ) ;
      d4unlock_records( d4 ) ;
      if ( d4->code_base->error_code < 0 )
         return -1 ;
   #endif
   return 0 ;
}

int S4FUNCTION d4unlock_file( DATA4 *d4 )
{
   #ifndef S4SINGLE
      #ifdef S4VBASIC
         if ( c4parm_check( d4, 2, E4_D4UNLOCK_FILE ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK_FILE ) ;
      #endif

      if ( d4->file_lock )
      {
         #ifdef N4OTHER
            if ( file4unlock( &d4->file, L4LOCK_POS, L4LOCK_POS ) < 0 )
               return -1 ;
         #endif
         #ifdef S4MDX
            if ( file4unlock( &d4->file, L4LOCK_POS_OLD, L4LOCK_POS - L4LOCK_POS_OLD + 1 ) < 0 )
               return -1 ;
         #endif
         #ifdef S4FOX
            /* codebase locks the append byte as well... */
            if ( file4unlock( &d4->file, L4LOCK_POS_OLD, L4LOCK_POS_OLD - 1L ) < 0 )
               return -1 ;
         #endif
         d4->rec_num_old =  -1 ;
         d4->memo_validated =  0 ;
         d4->file_lock =  0 ;
         d4->num_recs = -1 ;
      }
      if ( d4->code_base->error_code < 0 )
         return -1 ;
   #endif
   return 0 ;
}

int S4FUNCTION d4unlock_files( CODE4 *c4 )
{
   #ifndef S4SINGLE
      DATA4 *data_on ;

      #ifdef S4DEBUG
         if ( c4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK_FILES ) ;
      #endif

      for ( data_on = 0 ;; )
      {
         data_on = (DATA4 *)l4next( &c4->data_list, data_on ) ;
         if ( data_on == 0 )
            break ;
         d4unlock( data_on ) ;
      }

      if ( c4->error_code < 0 )
         return -1 ;
   #endif
   return 0 ;
}

int S4FUNCTION d4unlock_index( DATA4 *d4 )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      INDEX4 *index_on ;

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK_INDEX ) ;
      #endif

      for ( index_on = 0 ;; )
      {
         index_on = (INDEX4 *) l4next(&d4->indexes,index_on) ;
         if ( index_on == 0 )
         {
            if ( d4->code_base->error_code < 0 )
               return -1 ;
            return 0 ;
         }
         i4unlock(index_on) ;
      }
   #endif
}

int S4FUNCTION d4unlock_records( DATA4 *d4 )
{
   #ifndef S4SINGLE
      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4UNLOCK_REC ) ;
      #endif

      d4->rec_num_old =  -1 ;
      d4->memo_validated =  0 ;

      while ( d4->num_locked > 0 )
      {
         d4->num_locked-- ;
         #ifdef N4OTHER
            if ( file4unlock( &d4->file, L4LOCK_POS + d4->locks[d4->num_locked], 1L ) < 0 )
               return -1 ;
         #endif
         #ifdef S4MDX
            if ( file4unlock( &d4->file, L4LOCK_POS - d4->locks[d4->num_locked] -1L, 1L ) < 0 )
               return -1 ;
         #endif
         #ifdef S4FOX
            if ( d4->has_mdx )
            {
               if ( file4unlock( &d4->file, L4LOCK_POS - d4->locks[d4->num_locked], 1L ) < 0 )
                  return -1 ;
            }
            else
               if ( file4unlock( &d4->file, L4LOCK_POS_OLD + d4record_position( d4, d4->locks[d4->num_locked] ), 1L ) < 0 )
                  return -1 ;
         #endif
      }
   #endif
   return 0 ;
}
