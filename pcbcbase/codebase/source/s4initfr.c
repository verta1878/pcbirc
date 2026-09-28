/* s4initfr.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

void S4FUNCTION relate4free_bitmaps( RELATE4 *relate )
{
   RELATE4 *relate_on ;

   for( relate_on = 0 ;; )
   {
      relate_on = (RELATE4 *)l4next( &relate->slaves, relate_on ) ;
      if ( relate_on == 0 )
         break ;
      relate4free_bitmaps( relate_on ) ;
   }

   u4free( relate->set.flags ) ;
   memset( (void *)&relate->set, 0, sizeof ( F4FLAG ) ) ;
}

/* frees up any extra memory that is not neccessarily required for CodeBase */
/* returns 0 if successfully freed info, else 0 */
static int sort4free_extra( RELATE4 *relate, CODE4 *code_base )
{
   #ifndef S4OPTIMIZE_OFF
      if ( code_base->has_opt && code_base->opt.num_buffers )
      {
         d4opt_suspend( code_base ) ;
         code_base->error_code = 0 ;
         return 0 ;
      }
   #endif

   if ( relate != 0 )
      if ( relate->relation->bitmaps_freed == 0 )
      {
         relate4free_bitmaps( relate ) ;
         relate->relation->bitmaps_freed = 1 ;
         return 0 ;
      }

   return 1 ;
}

int S4FUNCTION sort4get_init_free( SORT4 *s4, RELATE4 *relate )
{
   CODE4 *code_base ;
   int rc, old_pool_n, old_pool_entries, old_tot_len, prev ;

   code_base = s4->code_base ;
   if ( code_base->error_code < 0 )
      return -1 ;

   old_pool_n = s4->pool_n ;
   old_pool_entries = s4->pool_entries ;
   old_tot_len = s4->tot_len ;

   prev = 0 ;

   if ( s4->spools_max > 0 )
   {
      rc = sort4spools_init( s4, prev ) ;
      while( rc == e4memory )
      {
         if ( sort4free_extra( relate, code_base ) == 1 )
         {
            sort4free( s4 ) ;
            return e4( s4->code_base, e4memory, E4_MEMORY_S ) ;
         }
         prev = 1 ;
         s4->pool_entries = old_pool_entries ;
         s4->tot_len = old_tot_len ;
         /* reset the pools */
         mem4release( s4->pool_memory ) ;
         s4->pool_memory =  mem4create( s4->code_base, 1, (unsigned) s4->pool_entries*s4->tot_len+sizeof(LINK4), 1, 1 ) ;
         for ( s4->pool_n = 0 ; old_pool_n ; old_pool_n-- )
            if ( mem4alloc( s4->pool_memory ) )
               s4->pool_n++ ;
         rc = sort4spools_init( s4, prev ) ;
      }
   }
   else
   {
      sort4get_mem_init( s4 ) ;
      return 0 ;
   }

   return rc ;
}

int S4FUNCTION sort4init_free( SORT4 *s4, CODE4 *c4, int sort_l, int info_l, RELATE4 *relate )
{
   int rc ;

   sort4init_set( s4, c4, sort_l, info_l ) ;
   rc = sort4init_alloc( s4 ) ;
   while( rc == e4memory )
   {
      if ( sort4free_extra( relate, c4 ) == 1 )
      {
         sort4free( s4 ) ;
         return e4( s4->code_base, e4memory, E4_MEMORY_S ) ;
      }
      rc = sort4init_alloc( s4 ) ;
   }
   return rc ;
}

