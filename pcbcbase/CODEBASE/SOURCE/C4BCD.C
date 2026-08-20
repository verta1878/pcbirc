/*  c4bcd.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */
/*                binary digit is:   xxxx xxxx  xxxx xx01                */
/*                                   sig dig    |||| ||||                */
/*                                              |||| ||always 01         */ 
/*                                              |length                  */
/*                                              sign                     */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifdef S4MDX

/* MDX */
int S4CALL c4bcd_cmp( S4CMP_PARM a_ptr, S4CMP_PARM b_ptr, size_t dummy_len )
{
   int a_sign, b_sign, a_len, b_len, a_sig_dig, b_sig_dig, compare_len, rc ;

   if ( ((C4BCD *)a_ptr)->digit_info  & 0x80 )  /* get digit sign */
      a_sign = -1 ;
   else
      a_sign =  1 ;

   if ( ((C4BCD *)b_ptr)->digit_info & 0x80 )
      b_sign = -1 ;
   else
      b_sign =  1 ;

   if ( a_sign != b_sign )
      return a_sign ;

   a_len = ( (((C4BCD *)a_ptr)->digit_info >> 2) & 0x1F ) ; /* get digit length */
   b_len = ( (((C4BCD *)b_ptr)->digit_info >> 2) & 0x1F ) ;

   if ( a_len == 0 )
      a_sig_dig = 0 ;
   else
      a_sig_dig = ((C4BCD *)a_ptr)->sig_dig ;  /* get digit significant digit */

   if ( b_len == 0 )
      b_sig_dig = 0 ;
   else
      b_sig_dig = ((C4BCD *)b_ptr)->sig_dig ;

   if ( a_sig_dig != b_sig_dig )
   {
      if ( a_sig_dig < b_sig_dig )
         return -a_sign ;
      else
         return a_sign ;
   }

   compare_len = (a_len < b_len) ? b_len : a_len ;  /* Use Max */

   compare_len = (compare_len+1)/2 ;

   rc = memcmp( ((C4BCD *)a_ptr)->bcd, ((C4BCD *)b_ptr)->bcd, compare_len ) ;
   if ( a_sign < 0 )  return -rc ;

   return rc ;
}

/* MDX */
void c4bcd_from_a( char *result, char *input_ptr, int input_ptr_len )
{
   char *ptr ;
   unsigned len ;
   int n, last_pos, pos, is_before_dec, zero_count ;

   memset( result, 0, sizeof(C4BCD) ) ;

   last_pos = input_ptr_len - 1 ;
   pos = 0 ;
   for ( ; pos <= last_pos; pos++ )
      if ( input_ptr[pos] != ' ' )  break ;

   if ( pos <= last_pos )
   {
      if ( input_ptr[pos] == '-' )
      {
         ((C4BCD *)result)->digit_info=((int)((C4BCD *)result)->digit_info | 0x80) ; 
         pos++ ;
      }
      else
      {
         if ( input_ptr[pos] == '+' )  pos++ ;
      }
   }

   for ( ; pos <= last_pos; pos++ )
      if ( input_ptr[pos] != ' ' && input_ptr[pos] != '0' )  break ;

   is_before_dec = 1 ;

   ((C4BCD *)result)->sig_dig = 0x34 ;
   if ( pos <= last_pos )
      if ( input_ptr[pos] == '.' )
      {
         is_before_dec = 0 ;
         pos++ ;
         for ( ; pos <= last_pos; pos++ )
         {
            if ( input_ptr[pos] != '0' )  break ;
            ((C4BCD *)result)->sig_dig-- ;
         }
      }

   ptr = (char *) ((C4BCD *)result)->bcd ; ;
   zero_count = 0 ;

   for ( n=0; pos <= last_pos; pos++ )
   {
      if ( input_ptr[pos] >= '0' && input_ptr[pos] <= '9' )
      {
         if ( input_ptr[pos] == '0' )
            zero_count++ ;
         else
            zero_count = 0 ;
         if ( n >= 20 )  break ;
         if ( n & 1 )
            *ptr++ |= input_ptr[pos] - '0' ;
         else
            *ptr += (unsigned char)( (unsigned char)((unsigned char)input_ptr[pos] - (unsigned char)'0') << 4 ) ;
      }
      else
      {
         if ( input_ptr[pos] != '.'  ||  ! is_before_dec )
            break ;

         is_before_dec = 0 ;
         continue ;
      }
      if ( is_before_dec )
         ((C4BCD *)result)->sig_dig++ ;

      n++ ;
   }
                                        /* 'always one' bit filled  */
   ((C4BCD *)result)->digit_info = ( ((C4BCD *)result)->digit_info | 0x1 ) ;

   #ifdef S4DEBUG
      if ( n - zero_count < 0 )
         e4severe( e4info, E4_C4BCD_FROM_A ) ;
   #endif

   len = n - zero_count ;
   if (len > 31)
      len = 31 ;

   ((C4BCD *)result)->digit_info = ( ((C4BCD *)result)->digit_info | (len << 2) ) ;

   if ( len == 0 )
      ((C4BCD *)result)->digit_info = ( ((C4BCD *)result)->digit_info & 0x7F ) ;
}

/* MDX */
void  c4bcd_from_d( char *result, double doub )
{
   char  temp_str[258], *ptr ;
   int   sign, dec, len, pos ;

   ptr = ecvt( doub, E4ACCURACY_DIGITS, &dec, &sign ) ;
   if ( sign )
   {
      pos = 1 ;
      temp_str[0] = '-' ;
   }
   else
      pos = 0 ;

   if ( dec < 0 )
   {
      dec = -dec ;
      len = dec+1+pos ;
      memcpy( temp_str+len, ptr, E4ACCURACY_DIGITS ) ;
      memset( temp_str+pos, '0', len-pos ) ;
      temp_str[pos] = '.' ;

      c4bcd_from_a( result, temp_str, E4ACCURACY_DIGITS + len ) ;
   }
   else
   {
      memcpy( temp_str+pos, ptr, dec ) ;
      pos += dec ;
      if ( dec < E4ACCURACY_DIGITS )
      {
         temp_str[pos++] = '.' ;

         len = E4ACCURACY_DIGITS - dec ;
         memcpy( temp_str+pos, ptr+dec, len ) ;
         pos += len ;
      }
      c4bcd_from_a( result, temp_str, pos ) ;
   }
}

#endif  /* S4MDX */


