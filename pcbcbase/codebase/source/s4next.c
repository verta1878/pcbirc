/* s4next.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int s4next_spool_entry( SORT4 *s4 )
{
   long last_disk_pos, disk_data_left, new_rec, spool_rec ;
   unsigned max_read, len_read ;
   int low, high, pos, rc ;
   char *new_data ;
   S4SPOOL save_spool ;

   s4->spool_pointer->pos += s4->tot_len ;
   if ( s4->spool_pointer->pos >= s4->spool_pointer->len )
   {
      s4->spool_pointer->pos = 0 ;
      if ( s4->spool_pointer->disk >= 0L )
      {
         last_disk_pos  = (s4->spool_pointer->spool_i+1) * s4->spool_disk_len  ;
         disk_data_left = last_disk_pos - s4->spool_pointer->disk ;
         max_read = s4->spool_mem_len ;
         if ( (long) s4->spool_mem_len > disk_data_left )
            max_read = (unsigned) disk_data_left ;
         len_read = file4read( &s4->file, s4->spool_pointer->disk,
               s4->spool_pointer->ptr, max_read) ;

         if ( s4->code_base->error_code < 0 )
         {
            sort4free( s4 ) ;
            return -1 ;
         }

         s4->spool_pointer->len = len_read ;
         s4->spool_pointer->disk += len_read ;
         if ( len_read != max_read || len_read == 0 )
         {
            if ( len_read % s4->tot_len )
            {
               sort4free( s4 ) ;
               return e4( s4->code_base, e4read, s4->file.name ) ;
            }
            s4->spool_pointer->disk = -1 ;
            if ( len_read == 0 )
            {
               s4delete_spool_entry( s4 ) ;
               return 0 ;
            }
            else
               s4->spool_pointer->len = len_read ;
         }
         else  /* Check if we are out of disk entries for the spool */
            if ( s4->spool_pointer->disk >= last_disk_pos )
               s4->spool_pointer->disk = -1L ;
      }
      else
      {
         s4delete_spool_entry(s4) ;
         return 0 ;
      }
   }

   /* Position the new entry to the sorted location using a binary search */
   /* New entry is placed before 'result':  int pos >= 1  when complete */
   low = 1 ;
   high = s4->spools_n ;
   new_data = s4->spool_pointer->ptr + s4->spool_pointer->pos ;
   memcpy( (void *)&new_rec, new_data + s4->sort_len, sizeof(new_rec) ) ;

   for(;;)
   {
      pos = ( low + high ) / 2 ;
      if ( pos == low && pos == high )  /* then found */
      {
         memcpy( (void *)&save_spool, (void *)s4->spool_pointer, sizeof(S4SPOOL) ) ;
         memmove( s4->spool_pointer, s4->spool_pointer+1, sizeof(S4SPOOL)*(pos-1) ) ;
         memcpy( (void *)(s4->spool_pointer+pos-1), (void *)&save_spool, sizeof(S4SPOOL) ) ;
         return 0 ;
      }
      rc = (*s4->cmp)(new_data, s4->spool_pointer[pos].ptr + s4->spool_pointer[pos].pos, s4->sort_len) ;
      if ( rc == 0 )
      {
         memcpy( (void *)&spool_rec, s4->spool_pointer[pos].ptr + s4->spool_pointer[pos].pos + s4->sort_len, sizeof(spool_rec) ) ;
         if ( new_rec > spool_rec )
            rc = 1 ;
      }

      if ( rc > 0 )
         low = pos+1 ;
      else
         high = pos ;
      #ifdef S4DEBUG
         if ( high < low )
            e4severe( e4result, E4_S4NEXT_SP_ENT ) ;
      #endif
   }
}

