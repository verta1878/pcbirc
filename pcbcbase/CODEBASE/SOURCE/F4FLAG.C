/* f4flag.c  (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION f4flag_init( F4FLAG *f4, CODE4 *c4, unsigned long n_flags )
{
   memset( (void *)f4, 0, sizeof(F4FLAG) ) ;

   if ( c4->error_code < 0 )
      return -1 ;

   f4->code_base = c4 ;
   f4->num_flags = n_flags ;

   if ( n_flags == 0 )
      return 0 ;

   f4->flags = (unsigned char *)u4alloc_free( c4, n_flags / 8 + 2 ) ;
   if ( f4->flags == 0 )
      return -1 ;
/*      return e4( c4, e4memory, 0 ) ; */
   return 0 ;
}

int S4FUNCTION f4flag_reset( F4FLAG *f4, unsigned long flag_num )
{
   unsigned char low_val, set_val ;
   unsigned long high_val ;

   if ( f4->code_base->error_code < 0 )
   {
      u4free( f4->flags ) ;
      f4->flags = 0 ;
      return -1 ;
   }

   if ( flag_num > f4->num_flags || f4->flags == 0 )
      return e4( f4->code_base, e4info, E4_F4LAG_RESET ) ;

   low_val = (unsigned char) (flag_num & 0x7) ;
   high_val = flag_num >> 3 ;
   set_val = 1 << low_val ;
   set_val = ~set_val ;

   f4->flags[high_val] = set_val & f4->flags[high_val] ;

   return 0 ;
}

int S4FUNCTION f4flag_set( F4FLAG *f4, unsigned long flag_num )
{
   unsigned char low_val, set_val ;
   unsigned long high_val ;

   if ( f4->code_base->error_code < 0 )
   {
      u4free( f4->flags ) ;
      f4->flags = 0 ;
      return -1 ;
   }

   if ( flag_num > f4->num_flags || f4->flags == 0 )
      return e4( f4->code_base, e4info, E4_F4LAG_SET ) ;

   low_val = (unsigned char) (flag_num & 0x7) ;
   high_val = flag_num >> 3 ;
   set_val = 1 << low_val ;

   f4->flags[high_val] = set_val | f4->flags[high_val] ;

   return 0 ;
}

int S4FUNCTION f4flag_set_range( F4FLAG *f4, unsigned long flag_num, unsigned long num_flags )
{
   unsigned long i_flag ;

   for ( i_flag = 0; i_flag < num_flags; i_flag++ )
      if ( f4flag_set( f4, flag_num + i_flag ) != 0 )
         return e4info ;
   return 0 ;
}

int S4FUNCTION f4flag_is_set( F4FLAG *f4, unsigned long flag_num )
{
   unsigned char low_val, ret_val ;
   unsigned long high_val ;

   if ( f4->code_base->error_code < 0 )
   {
      u4free( f4->flags ) ;
      f4->flags = 0 ;
      return -1 ;
   }

   if ( flag_num > f4->num_flags  ||  f4->flags == 0 )
      return e4( f4->code_base, e4info, E4_F4LAG_IS_SET ) ;

   low_val = (unsigned char) (flag_num & 0x7) ;
   high_val = flag_num >> 3 ;
   ret_val = (unsigned char)(1 << low_val) & f4->flags[high_val]  ;

   return (int) ret_val ;
}

int S4FUNCTION f4flag_is_all_set( F4FLAG *f4, unsigned long flag_num, unsigned long num_flags )
{
   int rc ;
   unsigned long i_flag ;

   if ( f4->code_base->error_code < 0 )
      num_flags = 1 ;

   for ( i_flag = flag_num; i_flag <= num_flags; i_flag++ )
   {
      rc = f4flag_is_set( f4, i_flag ) ;
      if ( rc < 0 )
         return -1 ;
      if ( rc == 0 )
         return 0 ;
   }
   return 1 ;
}

int S4FUNCTION f4flag_is_any_set( F4FLAG *f4, unsigned long flag_num, unsigned long num_flags )
{
   int rc ;
   unsigned long i_flag ;

   if ( f4->code_base->error_code < 0 )
      num_flags = 1 ;

   for ( i_flag = flag_num; i_flag <= num_flags; i_flag++ )
      if ( (rc = f4flag_is_set( f4, i_flag )) < 0 )
         return rc ;
   return 0 ;
}

void S4FUNCTION f4flag_and( F4FLAG *flag_ptr, F4FLAG *and_ptr )
{
   unsigned num_bytes ;

   if ( and_ptr->num_flags == 0 )
   {
      if ( flag_ptr->num_flags == 0 )
         return ;
      if ( flag_ptr->is_flip != and_ptr->is_flip )
         memset( flag_ptr->flags, 1, (unsigned)((flag_ptr->num_flags) / 8L + 2L ) ) ;
      else
         memset( flag_ptr->flags, 0, (unsigned)((flag_ptr->num_flags) / 8L + 2L ) ) ;
      return ;
   }
   if ( flag_ptr->num_flags != and_ptr->num_flags )
   {
      #ifdef S4DEBUG
         e4severe( e4result, E4_F4LAG_AND ) ;
      #endif
      return ;
   }

   num_bytes = (unsigned)((flag_ptr->num_flags+7)/8) ;
   #ifdef S4DEBUG
      if ( (unsigned long)num_bytes != ( flag_ptr->num_flags + 7L) / 8L )
         e4severe( e4info, E4_F4LAG_AND ) ;
   #endif

   if ( flag_ptr->is_flip != and_ptr->is_flip )
   {
      if ( flag_ptr->is_flip == 1 )
      {
         flag_ptr->is_flip = 0 ;
         do
         {
            flag_ptr->flags[num_bytes] = ~flag_ptr->flags[num_bytes] & and_ptr->flags[num_bytes] ;
         } while (num_bytes-- != 0 ) ;
      }
      else
         do
         {
            flag_ptr->flags[num_bytes] &= ~and_ptr->flags[num_bytes] ;
         } while (num_bytes-- != 0 ) ;
   }
   else
   {
      if ( flag_ptr->is_flip == 1 )
      {
         flag_ptr->is_flip = 0 ;
         do
         {
            flag_ptr->flags[num_bytes] = (unsigned char) (~flag_ptr->flags[num_bytes] & ~and_ptr->flags[num_bytes]) ;
         } while (num_bytes-- != 0 ) ;
      }
      else
         do
         {
            flag_ptr->flags[num_bytes] &= and_ptr->flags[num_bytes] ;
         } while (num_bytes-- != 0 ) ;
   }
}

void S4FUNCTION f4flag_or( F4FLAG *flag_ptr, F4FLAG *or_ptr )
{
   unsigned num_bytes ;

   if ( or_ptr->num_flags == 0 )
         return ;
   if ( flag_ptr->num_flags != or_ptr->num_flags )
   {
      #ifdef S4DEBUG
         e4severe( e4result, E4_F4LAG_OR ) ;
      #endif
      return ;
   }
   num_bytes = (unsigned)(flag_ptr->num_flags / 8L + 1L ) ;
   #ifdef S4DEBUG
      if ( (unsigned long)num_bytes != ( flag_ptr->num_flags / 8L + 1L ) )
         e4severe( e4info, E4_F4LAG_OR ) ;
   #endif
   if ( flag_ptr->is_flip != or_ptr->is_flip )
   {
      if ( flag_ptr->is_flip == 1 )
      {
         flag_ptr->is_flip = 0 ;
         do
         {
           flag_ptr->flags[num_bytes] = ~flag_ptr->flags[num_bytes] | or_ptr->flags[num_bytes] ;
         } while (num_bytes-- != 0 ) ;
      }
      else
         do
         {
            flag_ptr->flags[num_bytes] |= ~or_ptr->flags[num_bytes] ;
         } while (num_bytes-- != 0 ) ;
   }
   else
   {
      if ( flag_ptr->is_flip == 1 )
      {
         flag_ptr->is_flip = 0 ;
         do
         {
            flag_ptr->flags[num_bytes] = (unsigned char)(~flag_ptr->flags[num_bytes] | ~or_ptr->flags[num_bytes]) ;
         } while (num_bytes-- != 0 ) ;
      }
      else
         do
         {
            flag_ptr->flags[num_bytes] |= or_ptr->flags[num_bytes] ;
         } while (num_bytes-- != 0 ) ;
   }
}

void S4FUNCTION f4flag_flip_returns( F4FLAG *flag_ptr )
{
   flag_ptr->is_flip = !flag_ptr->is_flip ;
}

void S4FUNCTION f4flag_set_all( F4FLAG *flag_ptr )
{
   memset( flag_ptr->flags, 0xFF, (unsigned)((flag_ptr->num_flags+7)/8) ) ;
}
