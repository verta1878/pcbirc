/* i4positi.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

/* 'pos' is an percentage, positioning is approximate. */
int S4FUNCTION t4position2( TAG4 *t4, double *result )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION2 ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   *result = t4position( t4 );
   return 0;
}

#ifndef N4OTHER

double S4FUNCTION t4position( TAG4 *t4 )
{
   double pos ;
   B4BLOCK *block_on ;
   int n ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   pos = .5 ;
   for ( block_on = (B4BLOCK *)t4->blocks.last_node ; block_on != 0 ; )
   {
      #ifdef S4FOX
         n = block_on->header.n_keys ;
      #else
         n = block_on->n_keys + 1 ;
         if ( b4leaf( block_on ) )
            n-- ;
      #endif

      if( n == 0 )
         pos = 0.0 ;
      else
         pos = (block_on->key_on+pos)/n ;

      block_on = (B4BLOCK *) block_on->link.p ;
      if ( block_on == (B4BLOCK *) t4->blocks.last_node )
         break ;
   }
   #ifdef S4FOX
      if ( t4->header.descending )   /* backwards in file... */
         return 1.0 - pos ;
      else
   #endif
   return pos ;
}

int S4FUNCTION t4position_set( TAG4 *t4, double pos )
{
   int rc, n, final_pos ;
   B4BLOCK *block_on ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION_SET ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4SINGLE
      i4version_check( t4->index, 0 ) ;
   #endif

   #ifdef S4FOX
      if ( t4->header.descending )   /* backwards in file... */
         pos = 1.0 - pos ;
   #endif

   if ( t4up_to_root( t4 ) < 0 )
      return -1 ;

   for(;;)
   {
      #ifdef S4DEBUG
         if ( pos < 0.0 || pos > 1.0 )  /* Could be caused by rounding error ? */
            e4severe( e4parm, E4_INFO_ILP ) ;
      #endif

      block_on = t4block(t4) ;

      #ifdef S4FOX
         n = block_on->header.n_keys ;
      #else
         n = block_on->n_keys+1 ;
         if ( b4leaf( block_on ) )
            n-- ;
      #endif

      final_pos = (int)( n * pos ) ;
      if ( final_pos == n )
         final_pos-- ;
   
      #ifdef S4FOX
         b4go( block_on, final_pos ) ;
      #else
         block_on->key_on = final_pos ;
      #endif

      pos = pos*n - final_pos ;

      rc = t4down( t4 ) ;
      if ( rc < 0 )
         return -1 ;
      if ( rc == 1 )
         return 0 ;
   }
}

#endif   /*  ifndef N4OTHER  */

#ifdef N4OTHER

double S4FUNCTION t4position( TAG4 *t4 )
{
   double pos ;
   B4BLOCK *block_on ;
   int n ;
   #ifdef S4CLIPPER
      int first = 1 ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   block_on = (B4BLOCK *) t4->blocks.last_node ;

   if ( !b4leaf(block_on) )  
      pos = 1.0 ;
   else
      pos = .5 ;

   for ( ; block_on != 0 ; )
   {
      n = block_on->n_keys + 1 ;
      #ifdef S4CLIPPER
         if ( first )
         {
            if ( b4leaf( block_on ) )
               n-- ;
            first = 0 ;
         }
      #else
         if ( b4leaf( block_on ) )
            n-- ;
      #endif

      if( n == 0 )
         pos = 0.0 ;
      else
         pos = ( block_on->key_on + pos ) / n ;

      block_on = (B4BLOCK *) block_on->link.p ;
      if ( block_on == (B4BLOCK *)t4->blocks.last_node )
         break ;
   }
   #ifdef S4CLIPPER
      if ( t4->header.descending )   /* backwards in file... */
         return 1.0 - pos ;
      else
   #endif
   return pos ;
}

int S4FUNCTION t4position_set( TAG4 *t4, double pos )
{
   int rc, n, final_pos ;
   B4BLOCK *block_on ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION_SET ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifdef S4CLIPPER
      if ( t4->header.descending )   /* backwards in file... */
         pos = 1.0 - pos ;
   #endif

   if ( t4up_to_root( t4 ) < 0 )
      return -1 ;

   for(;;)
   {
      #ifdef S4DEBUG
         if ( pos < 0.0 || pos > 1.0 )  /* Could be caused by rounding error ? */
            e4severe( e4parm, E4_INFO_ILP ) ;
      #endif

      block_on = t4block( t4 ) ;

      n = block_on->n_keys + 1 ;
      if ( b4leaf( block_on ) )
         n-- ;

      final_pos = (int)( n * pos ) ;
      if ( final_pos == n )
         final_pos-- ;
   
      block_on->key_on = final_pos ;
      pos = pos*n - final_pos ;

      rc = t4down( t4 ) ;
      if ( rc < 0 )
         return -1 ;
      if ( rc == 1 )
         return 0 ;
   }
}

#endif  /* N4OTHER */
#endif  /* S4INDEX_OFF */
