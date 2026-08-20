/* s4sort.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved.

Internal sort information is saved as follows:
   Sort Info, Rec Num, Other Info

s4quick assumes there is an extra four bytes where the record number
is put.
*/

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

void s4delete_spool_entry( SORT4 *s4 )
{
   memcpy( (void *)s4->spool_pointer, (void *)(s4->spool_pointer+1), --s4->spools_n * sizeof(S4SPOOL) ) ;
}

int s4flush( SORT4 *s4 )
{
   unsigned i ;

   s4quick( (void **) s4->pointers, s4->pointers_used, s4->cmp, s4->sort_len ) ;

   if ( s4->spools_max == 0 )  /* Temporary file must be created. */
   {
      file4temp( &s4->file, s4->code_base, s4->file_name_buf, 1 ) ;
      file4seq_write_init( &s4->seqwrite, &s4->file, 0L, s4->seqwrite_buffer,
          s4->code_base->mem_size_sort_buffer ) ;
   }

   for ( i = 0; i < s4->pointers_used; i++ )
      if ( file4seq_write( &s4->seqwrite, s4->pointers[i], s4->tot_len) < 0 ) return -1 ;

   s4->pointers_used = 0 ;
   if ( (unsigned long) s4->spools_max * (unsigned long) sizeof(S4SPOOL) >= (unsigned long) UINT_MAX )
   {
      e4( s4->code_base, e4memory, E4_MEMORY_S ) ;
      sort4free(s4) ;
      return e4memory ;
   }
   s4->spools_max++ ;

   return 0 ;
}

int  S4FUNCTION sort4free( SORT4 *s4 )
{
   void *pool_ptr ;

   u4free( s4->spool_pointer ) ;
   u4free( s4->pointers ) ;
   u4free( s4->seqwrite_buffer ) ;

   if ( s4->file.hand >= 0 )
      if ( file4close( &s4->file ) < 0 )
         return -1 ;

   for ( ;; )
   {
      pool_ptr = l4pop( &s4->pool ) ;
      if ( pool_ptr == 0 )
         break ;
      mem4free( s4->pool_memory, pool_ptr ) ;
   }
   mem4release( s4->pool_memory ) ;

   memset( (void *)s4, 0, sizeof(SORT4) ) ;
   s4->file.hand = -1 ;

   return 0 ;
}

int S4FUNCTION sort4get( SORT4 *s4, long *rec_ptr, void **sort_data, void **info_ptr )
{
   char *ptr ;
   int rc ;

   #ifdef S4DEBUG
      if ( s4 == 0 )
         e4severe( e4parm, E4_S4GET ) ;
   #endif

   if ( s4->code_base->error_code < 0 )
      return -1 ;

   rc = sort4get_ptr_ptr( s4, &ptr ) ;
   if ( rc )
      return rc ;

   memcpy( (void *)rec_ptr, ptr + s4->sort_len, sizeof( long ) ) ;
   *sort_data= (void *)ptr ;
   *info_ptr = (void *)( ptr + s4->info_offset ) ;

   return 0 ;
}

int sort4get_ptr_ptr( SORT4 *s4, char **ptr_ptr )
{
   if ( s4->pointers != 0 )  /* then no spooling was done */
   {
      if ( s4->pointers_i < s4->pointers_used )
      {
         *ptr_ptr = (char *)s4->pointers[s4->pointers_i++] ;
         return 0 ;
      }
      if ( sort4free( s4 ) < 0 )
         return -1 ;
      return 1 ;
   }

   if ( s4->spools_n == 0 )
   {
      if ( sort4free( s4 ) < 0 )
         return -1 ;
      return 1 ;  /* None available */
   }

   if ( s4next_spool_entry( s4 ) < 0 )
      return -1 ;

   if ( s4->spools_n == 0 )
   {
      if ( sort4free( s4 ) < 0 )
         return -1 ;
      return 1 ;
   }
   *ptr_ptr = s4->spool_pointer->ptr+s4->spool_pointer->pos ;

   return 0 ;
}

int S4FUNCTION sort4put( SORT4 *s4, long rec, void *sort_data, void *info )
{
   char *ptr ;
   int rc ;
   #ifndef S4PORTABLE
      unsigned ptr_mem_avail, new_entries ;
   #endif

   #ifdef S4DEBUG
      if ( s4 == 0 )
         e4severe( e4parm, E4_S4PUT ) ;
   #endif

   if ( s4->code_base->error_code < 0 )
      return -1 ;

   if ( s4->pointers_used >= s4->pointers_init )
   {
      if ( s4->pointers_used < s4->pointers_max && s4->is_mem_avail )
      {
         ptr = (char *)mem4alloc( s4->pool_memory ) ;
         if ( ptr == 0 )
         {
            #ifndef S4PORTABLE
               ptr_mem_avail = ( s4->pointers_max - s4->pointers_used ) * ( sizeof( char * ) ) - ( sizeof( long ) ) ;
               new_entries = ptr_mem_avail / ( sizeof( char * ) + s4->tot_len ) ;
               s4->pointers_max = s4->pointers_used + new_entries ;
               sort4init_pointers( s4, (char *)( s4->pointers + s4->pointers_max ), new_entries * s4->tot_len ) ;
            #endif

            s4->is_mem_avail = 0  ;
         }
         else
         {
            l4add( &s4->pool, ptr ) ;
            s4->pool_n++ ;
            sort4init_pointers( s4, ptr + sizeof( LINK4 ), s4->code_base->mem_size_sort_pool - sizeof( LINK4 ) ) ;
         }
      }
   }

   if ( s4->pointers_used >= s4->pointers_init )
   {
      rc = s4flush( s4 ) ;
      if ( rc < 0 )
         return rc ;
   }

   #ifdef S4DEBUG
      if ( s4->pointers_used >= s4->pointers_init )
         e4severe( e4result, E4_S4PUT ) ;
   #endif

   ptr = s4->pointers[s4->pointers_used++] ;
   memcpy( ptr, sort_data, s4->sort_len ) ;
   memcpy( ptr+ s4->sort_len, (void *)&rec, sizeof( rec ) ) ;
   memcpy( ptr+ s4->info_offset, info, s4->info_len ) ;

   return 0 ;
}
