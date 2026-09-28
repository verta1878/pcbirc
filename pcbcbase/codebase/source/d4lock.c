/* d4lock.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#include <time.h>

int S4FUNCTION d4lock( DATA4 *data, long rec )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E4_D4LOCK ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( data == 0 || rec < 1L )
            e4severe( e4parm, E4_D4LOCK ) ;
      #endif

      if ( data->code_base->error_code < 0 )
         return -1 ;

      if ( d4lock_test( data, rec ) > 0 )  /* if record or file already locked */
         return 0 ;

      /* append bytes are left locked if already locked */
      /* if index file is locked and data file not locked, index file is here unlocked */
      #ifndef S4INDEX_OFF
         d4unlock_index( data ) ;
      #endif
      d4unlock_records( data ) ;

      #ifdef N4OTHER
         rc = file4lock( &data->file, L4LOCK_POS + rec, 1L) ;
      #endif
      #ifdef S4MDX
         rc = file4lock( &data->file, L4LOCK_POS - 1U, 1L ) ;
         if ( rc )
            return rc ;
         rc = file4lock( &data->file, L4LOCK_POS - rec - 1U, 1L) ;
         file4unlock( &data->file, L4LOCK_POS - 1U, 1L ) ;
      #endif
      #ifdef S4FOX
         if ( data->has_mdx )
            rc = file4lock( &data->file, L4LOCK_POS - rec, 1L) ;
         else
            rc = file4lock( &data->file, L4LOCK_POS_OLD + d4record_position( data, rec ), 1L) ;
      #endif

      if ( rc )
         return rc ;

      data->num_locked = 1 ;
      *data->locks = rec ;
   #endif
   return 0 ;
}

/* locks database, memo, and index files */
int S4FUNCTION d4lock_all( DATA4 *data )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4LOCK_ALL ) ;
      #endif

      if ( data->code_base->error_code < 0 )
         return -1 ;

      rc = d4lock_file( data ) ;   /* this function will unlock all if required */
      #ifndef N4OTHER
         #ifndef S4MEMO_OFF
            if ( !rc )
               if ( data->n_fields_memo > 0 && data->memo_file.file.hand != -1 )
                  rc = memo4file_lock( &data->memo_file ) ;
         #endif
      #endif
      #ifndef S4INDEX_OFF
         if ( !rc )
            rc = d4lock_index( data ) ;
      #endif

      if ( rc )
         d4unlock( data ) ;

      return rc ;
   #else
      return 0 ;
   #endif
}

int S4FUNCTION d4lock_append( DATA4 *data )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      int rc ;
      #ifndef S4OPTIMIZE_OFF
         OPT4 *opt ;
         OPT4BLOCK *block_on ;
      #endif

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E4_D4LOCK_APP ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4LOCK_APP ) ;
      #endif

      if ( data->code_base->error_code < 0 )
         return -1 ;

      if ( !d4lock_test_append( data ) )
      {
         d4unlock_records( data ) ;
         rc = file4lock( &data->file, L4LOCK_POS, 1L ) ;
         #ifndef S4OPTIMIZE_OFF
            opt = &data->code_base->opt ;
            if ( data->file.is_exclusive == 0 && data->file.do_buffer == 1 && opt->num_buffers != 0 )
            {
               block_on = opt4file_return_block( &data->file, 0L, opt4file_hash( &data->file, 0L ) ) ;
               if ( block_on )
                  if ( block_on->changed )
                  {
                     l4remove( &opt->lists[ opt4file_hash( &data->file, block_on->pos )], block_on ) ;
                     opt4file_lru_top( &data->file, &block_on->lru_link, 0 ) ;
                     l4add( &opt->avail, &block_on->lru_link ) ;
                     opt4block_clear( block_on ) ;
                  }
            }
         #endif
         if ( !rc )
            data->append_lock = 1 ;
      }
      else
         rc = 0 ;
      return rc ;
   #endif
}

int S4FUNCTION d4lock_file( DATA4 *data )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E4_D4LOCK_FILE ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4LOCK_FILE ) ;
      #endif

      if ( data->code_base->error_code < 0 )
         return -1 ;

      if ( d4lock_test_file( data ) )
         return 0 ;

      if ( d4unlock( data ) )  /* first remove any existing locks */
         return -1 ;

      #ifdef N4OTHER
         rc = file4lock( &data->file, L4LOCK_POS, L4LOCK_POS ) ;
      #endif
      #ifdef S4MDX
         rc = file4lock( &data->file, L4LOCK_POS_OLD, L4LOCK_POS - L4LOCK_POS_OLD + 1 ) ;
      #endif
      #ifdef S4FOX
         /* codebase locks the append byte as well... */
         rc = file4lock( &data->file, L4LOCK_POS_OLD, L4LOCK_POS_OLD - 1L ) ;
      #endif

      if ( rc )
         return rc ;
      data->file_lock = 1 ;

      #ifndef S4OPTIMIZE_OFF
         file4refresh( &data->file ) ;   /* make sure all up to date */
      #endif
   #endif

   return 0 ;
}

int S4FUNCTION d4lock_group( DATA4 *data, long *recs, int n_recs )
{
   #ifndef S4SINGLE
      int i, rc, old_attempts, count ;

      #ifdef S4DEBUG
         if ( data == 0 || recs == 0 || n_recs < 0 )
            e4severe( e4parm, E4_D4LOCK_GROUP ) ;
      #endif

      if ( data->code_base->error_code < 0 )
         return -1 ;

      if ( d4lock_test_file( data ) )
         return 0 ;

      if ( d4unlock( data ) < 0 )
         return -1 ;

      rc = 0 ;

      if ( data->n_locks < n_recs )
      {
         if ( data->n_locks > 1 )
            u4free( data->locks ) ;
         data->locks = (long *)u4alloc_er( data->code_base, (long)sizeof(long) * n_recs ) ;
         if ( !data->locks )
         {
            data->n_locks = 1 ;
            data->locks = &data->locked_record ;
            return e4memory ;
         }
         data->n_locks = n_recs ;
      }

      count = old_attempts = data->code_base->lock_attempts ;  /* take care of wait here */
      data->code_base->lock_attempts = 1 ;

      #ifdef S4MDX
         rc = file4lock( &data->file, L4LOCK_POS - 1L, 1L ) ;
         if ( rc == 0 )
      #endif
      for(;;)
      {
         for ( i = 0 ; i < n_recs ; i++ )
         {
            #ifdef N4OTHER
               rc = file4lock( &data->file, L4LOCK_POS+recs[i], 1L ) ;
            #endif
            #ifdef S4MDX
               rc = file4lock( &data->file, L4LOCK_POS - recs[i] - 1L, 1L ) ;
            #endif
            #ifdef S4FOX
               if ( data->has_mdx )
                  rc = file4lock( &data->file, L4LOCK_POS - recs[i], 1L ) ;
               else
                  rc = file4lock( &data->file, L4LOCK_POS_OLD + d4record_position( data, recs[i] ), 1L ) ;
            #endif

            if ( rc < 0 )
               break ;

            data->locks[i] = recs[i] ;
         }

         data->num_locked = i ;
         if ( i == n_recs )  /* all locked */
         {
            data->code_base->lock_attempts = old_attempts ;
            #ifdef S4MDX
               file4unlock( &data->file, L4LOCK_POS - 1L, 1L ) ;
            #endif
            return 0 ;
         }

         d4unlock_records( data ) ;
         if ( rc != r4locked )   /* error */
            break ;

         if ( !count )
            break ;

         if ( count > 0 )
            count-- ;

         #ifdef S4TEMP
            if ( d4display_quit( &display ) )
               e4severe( e4result, E4_RESULT_EXI ) ;
         #endif

         u4delay_sec() ;   /* wait a second */
      }

      #ifdef S4MDX
         file4unlock( &data->file, L4LOCK_POS - 1L, 1L ) ;
      #endif

      data->code_base->lock_attempts = old_attempts ;
      return rc ;
   #else
      return 0 ;
   #endif
}

int S4FUNCTION d4lock_index( DATA4 *data )
{
   #ifndef S4INDEX_OFF
      #ifndef S4SINGLE
         int rc, old_attempts, count ;
         INDEX4 *index_on ;

         #ifdef S4VBASIC
            if ( c4parm_check( data, 2, E4_D4LOCK_INDEX ) )
               return -1 ;
         #endif

         #ifdef S4DEBUG
            if ( data == 0 )
               e4severe( e4parm, E4_D4LOCK_INDEX ) ;
         #endif

         if ( data->code_base->error_code < 0 )
            return -1 ;

         count = old_attempts = data->code_base->lock_attempts ;  /* take care of wait here */
         data->code_base->lock_attempts = 1 ;

         for(;;)
         {
            rc = 0 ;
            for ( index_on = 0 ;; )
            {
               index_on = (INDEX4 *) l4next( &data->indexes, index_on ) ;
               if ( index_on == 0 || rc )
                  break ;
               rc = i4lock( index_on ) ;
            }

            if ( rc != r4locked )
               break ;

            for ( index_on = 0 ;; )
            {
               index_on = (INDEX4 *)l4next( &data->indexes, index_on ) ;
               if ( index_on == 0 )
                  break ;
               if ( i4unlock( index_on ) != 0 )
                  rc = -1 ;
            }

            if ( count == 0 || rc == -1 )
               break ;

            if ( count > 0 )
               count-- ;

            #ifdef S4TEMP
               if ( d4display_quit( &display ) )
                  e4severe( e4result, E4_RESULT_EXI ) ;
            #endif

            u4delay_sec() ;   /* wait a second */
         }

         data->code_base->lock_attempts = old_attempts ;
         if ( data->code_base->error_code < 0 )
            return -1 ;

         return rc ;
      #else
         return 0 ;
      #endif
   #else
      return 0 ;
   #endif
}

int S4FUNCTION d4lock_test( DATA4 *data, long rec )
{
   #ifndef S4SINGLE
      int i ;

      #ifdef S4DEBUG
         if ( !data )
            e4severe( e4parm, E4_D4LOCK_TEST ) ;
      #endif

      if ( data->file_lock )
         return 1 ;

      for ( i = 0; i < data->num_locked; i++ )
         if ( data->locks[i] == rec )
            return 1 ;
      return 0 ;
   #else
      return 1 ;
   #endif
}

int S4FUNCTION d4lock_test_append( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifdef S4DEBUG
         if ( !data )
            e4severe( e4parm, E4_D4LOCK_TEST_APP ) ;
      #endif

      return ( data->file_lock || data->append_lock || data->file.is_exclusive ) ? 1 : 0 ;
   #else
      return 1 ;
   #endif
}

int S4FUNCTION d4lock_test_file( DATA4 *data )
{
   #ifndef S4SINGLE
      #ifdef S4DEBUG
         if ( !data )
            e4severe( e4parm, E4_D4LOCK_TEST_FL ) ;
      #endif

      return ( data->file_lock || data->file.is_exclusive ) ? 1 : 0 ;
   #else
      return 1 ;
   #endif
}

int S4FUNCTION d4lock_test_index( DATA4 *data )
{
   #ifndef S4INDEX_OFF
      #ifndef S4SINGLE
         INDEX4 *index_on ;

         #ifdef S4DEBUG
            if ( !data )
               e4severe( e4parm, E4_D4LOCK_TEST_IN ) ;
         #endif

         for( index_on = 0 ; ; )
         {
            index_on = (INDEX4 *)l4next( &data->indexes, index_on ) ;
            if ( !index_on )
               break ;
            if ( !index_on->file_locked && !data->file.is_exclusive )
               return 0 ;
         }
      #endif
   #endif
   return 1 ;
}
