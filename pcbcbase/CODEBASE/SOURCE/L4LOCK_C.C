/* l4lock_c.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4LOCK_CHECK

/* Saves information about all current locks  */
typedef struct
{
   LINK4  link ;
   int     next ;
   int     prev ;
   int     hand ;
   long    start_pos ;
   long    len ;
   long    end_pos ;
} L4LOCK_INFO ;

static LIST4 lock_list ;
static int first =  1 ;
static MEM4 *lock_alloc ;

int S4FUNCTION l4lock_check()
{
   return lock_list.n_link ;
}

/* Returns true or false; true means found */
static L4LOCK_INFO *l4lock_find( int h, long p, long l )
{
   L4LOCK_INFO *l_ptr ;

   if ( first )
   {
      first =  0 ;
      memset( &lock_list, 0, sizeof(lock_list) ) ;
      lock_alloc =  mem4create( 0, 20, sizeof(L4LOCK_INFO), 20, 0 ) ;
   }

   for ( l_ptr = 0; l_ptr = (L4LOCK_INFO *) l4next(&lock_list, l_ptr); )
   {
      if ( l_ptr->hand == h  && l_ptr->start_pos == p && l_ptr->len == l )
         return l_ptr ;
      if ( l_ptr->hand == h )
      {
         /* Check for Overlap. */
         if ( l_ptr->start_pos >= p && l_ptr->start_pos <= p+l-1  ||
              l_ptr->end_pos >= p   && l_ptr->end_pos   <= p+l-1  ||
              p >= l_ptr->start_pos && p <= l_ptr->end_pos        ||
              p+l-1 >= l_ptr->start_pos && p+l-1 <= l_ptr->end_pos )
            e4severe( e4result, E4_RESULT_LCO ) ;
      }
   }
   return 0 ;
}

void S4FUNCTION l4lock_remove( int h, long p, long l )
{
   L4LOCK_INFO *l_ptr ;

   #ifndef S4WINDOWS
      printf( "\nUnlock  Handle %d   Start Pos %ld   Length %ld", h, p, l ) ;
   #endif

   l_ptr =  l4lock_find( h,p,l ) ;
   if ( l_ptr == 0 )
      e4severe( e4result, E4_RESULT_REM ) ;

   l4remove( &lock_list, l_ptr ) ;
}

void S4FUNCTION l4lock_save( int h, long p, long l )
{
   L4LOCK_INFO *l_ptr ;

   #ifndef S4WINDOWS
      printf( "\nLock    Handle %d   Start Pos %ld   Length %ld", h, p, l ) ;
   #endif

   if ( l4lock_find(h,p,l) != 0 )
      e4severe( e4result, E4_RESULT_LCO ) ;

   l_ptr =  (L4LOCK_INFO *) mem4alloc( lock_alloc ) ;
   if ( l_ptr == 0 )
      e4severe( e4memory, E4_MEMORY_SAV ) ;
   l4add( &lock_list, l_ptr ) ;

   l_ptr->hand = h ;
   l_ptr->start_pos = p ;
   l_ptr->len  = l ;
   l_ptr->end_pos =  p+l-1 ;
}

#endif
