/* d4skip.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION d4skip( DATA4 *d4, long n )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
   #endif
   int rc, save_flag, old_eof_flag ;
   long start_rec, new_rec, count, n_skipped ;
   char *key_value ;
   CODE4 *c4 ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4SKIP ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4SKIP ) ;
   #endif

   c4 = d4->code_base ;
   if ( c4->error_code < 0 )
      return -1 ;

   if ( d4->rec_num < 1L )
   {
      if ( c4->skip_error == 1 )
         e4( c4, e4info, E4_INFO_SKI ) ;
      return -1L ;
   }

   #ifndef S4INDEX_OFF
      tag = d4tag_selected( d4 ) ;
      if ( tag == 0 )
      {
   #endif
      d4->bof_flag = 0 ;
      start_rec = d4->rec_num ;
      new_rec = start_rec+n ;
      #ifndef S4OPTIMIZE_OFF
         #ifndef S4DETECT_OFF
            c4->mode |= 0x02 ;
         #endif
      #endif
      if ( new_rec > 0L )
      {
         save_flag = c4->go_error ;
         c4->go_error = 0 ;
         rc = d4go( d4, new_rec ) ;
         c4->go_error = save_flag ;
         if ( rc >= 0 && rc != r4entry )
            return rc ;
      }

      count = d4reccount(d4) ;

      if ( count <= 0L || new_rec > count )
      {
         if ( count <= 0L )
         {
            if ( count < 0 )
               return -1 ;
            rc = d4go_eof( d4 ) ;
            if ( rc != r4eof )
               return rc ;
            d4->bof_flag = 1 ;
         }
         if ( n < 0 )
         {
            d4->bof_flag = 1 ;
            return r4bof ;
         }
         else
            return d4go_eof( d4 ) ;
      }

      if ( new_rec < 1L )
      {
         old_eof_flag = d4->eof_flag ;
         rc = d4go( d4, 1L ) ;
         if ( rc )
            return rc ;
         d4->bof_flag = 1 ;
         d4->eof_flag = old_eof_flag ;
         return r4bof ;
      }

      return d4go( d4, new_rec ) ;
   #ifndef S4INDEX_OFF
      }
      else
      {
         if ( d4->eof_flag )
         {
            if ( n >= 0 ) return d4go_eof( d4 ) ;

            rc = d4bottom( d4 ) ;
            if ( rc && rc != r4eof )
               return rc ;
            if ( rc == r4eof )
            {
               rc = d4go_eof( d4 ) ;
               if ( rc != r4eof )
                  return rc ;
               return r4bof ;
            }
            n++ ;
            d4->rec_num = t4recno( tag ) ;
         }

         d4->bof_flag = 0 ;

         #ifndef S4OFF_WRITE
            if ( d4->record_changed )
               if ( d4update_record( d4, 1 ) < 0 )
                  return -1 ;
         #endif

         t4version_check( tag, 1 ) ;

         if ( n == 0 )
            return 0 ;

         if ( t4recno( tag ) != d4->rec_num )
         {
            rc = d4go( d4, d4->rec_num ) ;
            if ( rc )
               return rc ;

            t4expr_key( tag, &key_value ) ;

            rc = t4go( tag, key_value, d4->rec_num ) ;
            if ( rc < 0 )
               return -1 ;

            #ifdef S4HAS_DESCENDING
               if ( tag->header.descending )
               {
                  if ( (rc > 0) && (n < 0) )
                     n-- ;
               }
               else
                  if ( (rc > 0) && (n > 0) )
                     n-- ;
            #else
               if ( (rc > 0) && (n > 0) )
                  n-- ;
            #endif
         }

         #ifdef S4HAS_DESCENDING
            if ( tag->header.descending )
               n_skipped = -t4skip( tag, -n ) ;
            else
               n_skipped = t4skip( tag, n ) ;
            if ( n > 0  &&  n_skipped != n )
               return d4go_eof( d4 ) ;
         #else
            n_skipped = t4skip( tag, n ) ;
            if ( n > 0  &&  n_skipped != n )
               return d4go_eof( d4 ) ;
         #endif

         if ( t4eof( tag ) )
         {
            d4->bof_flag = 1 ;
            return d4go_eof( d4 ) ;
         }

         rc = d4go( d4, t4recno( tag ) ) ;
         if ( rc )
            return rc ;
         if ( n == n_skipped )
            return 0 ;
         #ifdef S4HAS_DESCENDING
            if ( ( n < 0 && !tag->header.descending ) ||  ( n < 0 && tag->header.descending ) )
         #else
            if ( n < 0 )
         #endif
            {
               d4->bof_flag = 1 ;
               return r4bof ;
            }
      }
   #endif
   return 0 ;
}
