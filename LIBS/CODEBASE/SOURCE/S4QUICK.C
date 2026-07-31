/* s4quick.c (c)Copyright Sequiter Software Inc., 1990-1993. All rights reserved.

   Iterative Quick Sort Algorithm

   This algorithm is superior to the more traditional recursive
   Quick Sort as the worst case stack size is proportional to 'log(n)'.
   In this algorithm the stack is explicitly maintained.

   In the recursive algorithm, the worst case depth of recursion is
   proportional to 'n'.  For example, if there were 1000 items to
   sort, the Quick Sort could, in the worst case, call itself
   1000 times.

   This routine assumes that there is a record number after the sort
   data for final comparison resolutions in case that two keys are the same.
*/

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

static void flip( int i, int j ) ;
static int greater( int i, int j ) ;
static void sort( void ) ;

static void **pointers ;
static int n_pointers ;
static int sort_len ;
S4CMP_FUNCTION *cmp ;

static void flip( int i, int j )
{
   void *flip_data;
   flip_data  = pointers[i] ;
   pointers[i] = pointers[j] ;
   pointers[j] = flip_data ;
}

static int greater( int i, int j )
{
   long l1, l2 ;
   int rc ;

   rc = (*cmp)( pointers[i], pointers[j], sort_len ) ;
   if ( rc > 0 )
      return 1 ;
   if ( rc < 0 )
      return 0 ;

   memcpy( (void *)&l1, ((char *)pointers[i])+sort_len, sizeof(long) ) ;
   memcpy( (void *)&l2, ((char *)pointers[j])+sort_len, sizeof(long) ) ;
   return l1 > l2 ;
}

void s4quick( void **p, int p_n, S4CMP_FUNCTION *cmp_routine, int width )
{
   cmp = cmp_routine ;
   pointers = p ;
   n_pointers = p_n ;
   sort_len = width ;
   sort() ;
}

static void sort()
{
   /* A stack size of 32 is enough to sort four billion items. */
   int stack_start[32], stack_end[32], f, l, num, j, i, middle, stack_on ;

   stack_on = 0 ;
   stack_start[0] = 0 ;
   stack_end[0] = n_pointers - 1 ;

   while( stack_on >= 0 )
   {
      f = stack_start[stack_on] ;
      l = stack_end[stack_on] ;
      stack_on-- ;

      /* Sort items 'f' to 'l' */
      while ( f < l )
      {
         /* Pick the middle item based on a sample of 3 items. */
         num = l - f ;
         if ( num < 2 )
         {
            if ( num == 1 )  /* Two Items */
               if ( greater( f, l ) )
                  flip( f, l ) ;
            break ;
         }

         /* Choose 'ptr_ptr[f]' to be a median of three values */
         middle = ( l + f ) / 2 ;

         if ( greater( middle, l ) )
            flip( middle, l ) ;

         if ( greater( middle, f ) )
            flip( f, middle ) ;
         else
            if ( greater( f, l ) )
               flip( f , l ) ;

         if ( num == 2 )   /* Special Optimization on Three Items */
         {
            flip( f, middle ) ;
            break ;
         }

         i = f + 1 ;
         while( greater( f, i ) )
         {
            i++ ;
            #ifdef S4DEBUG
               if ( i >= n_pointers )
                  e4severe( e4result, E4_RESULT_INQ ) ;
            #endif
         }

         j = l ;
         while( greater( j, f ) )
            j-- ;

         while ( i < j )
         {
            flip( i, j ) ;
            i++ ;

            while( greater( f, i ) )
            {
               i++ ;
               #ifdef S4DEBUG
                  if ( i >= n_pointers )
                     e4severe( e4result, E4_RESULT_INQ ) ;
               #endif
            }
            j-- ;

            while( greater( j, f ) )
               j-- ;
         }

         flip( f, j ) ;

         /* Both Sides are non-trivial */
         if ( j - f > l - j )
         {
            /* Left sort is larger, put it on the stack */
            stack_start[++stack_on] = f ;
            stack_end[stack_on]  = j - 1 ;
            f = j+ 1 ;
         }
         else
         {
            /* Right sort is larger, put it on the stack */
            stack_start[++stack_on] = j + 1 ;
            stack_end[stack_on] = l ;
            l = j- 1 ;
         }
      }
   }
}
