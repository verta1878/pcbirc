/* c4.c  Conversion Routines  (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifdef S4VBASIC
int c4parm_check( void *ptr, int do_check, char *message )
{
   if (!ptr)
   {
      e4severe_vbasic( e4info, E4_INFO_STR ) ;
      return -1 ;
   }

   switch (do_check)
   {
      case 1:
         if ( ((CODE4 *)ptr)->debug_int != 0x5281)
            do_check = -1 ;
         break ;
      case 2:
         if ( ((DATA4 *)ptr)->debug_int != 0x5281)
            do_check = -1 ;
         break ;
      case 3:
         if ( ((FIELD4 *)ptr)->debug_int != 0x5281)
            do_check = -1 ;
         break ;
   }

   if (do_check < 0 )
   {
      e4severe_vbasic( e4info, E4_INFO_STR ) ;
      return -1 ;
   }
   else return 0 ;
}
#endif  /* S4VBASIC */

/* c4atod    Converts a string to a double */
double S4FUNCTION c4atod( char *str, int len_str )
{
   char buffer[50] ;
   int  len ;

   len = ( len_str >= 50 ) ? 49 : len_str ;
   memcpy( buffer, str, (size_t)len ) ;
   buffer[len] = '\0' ;
   return( strtod( buffer, (char **)0 ) ) ;
}

int S4FUNCTION c4atoi( char *str, int len_str )
{
   char buf[128] ;
   if ( len_str >= (int)sizeof( buf ) )
      len_str = (int)sizeof( buf ) - 1 ;
   memcpy( buf, str, (size_t)len_str ) ;
   buf[len_str] = '\0' ;
   return atoi( buf ) ;
}

long S4FUNCTION c4atol( char *str, int len_str )
{
   char buf[128] ;
   if ( len_str >= (int)sizeof( buf ) )
      len_str = (int)sizeof( buf ) - 1 ;
   memcpy( buf, str, (size_t)len_str) ;
   buf[len_str] = '\0' ;
   return atol( buf ) ;
}

#ifdef S4CLIPPER

/* S4CLIPPER */
char *S4FUNCTION c4descend_num( char *to, char *from, int len ) 
{
   for(; len-- > 0; )
      to[len] = -from[len] ;
   return to ;
}

/* S4CLIPPER */
char *S4FUNCTION c4descend_str( char *to, char *from, int len )
{
   for(; len-- > 0; )
      to[len] = -from[len] ;
   return to ;
}

/* S4CLIPPER */
char *S4FUNCTION c4descend_date( char *to, long l, int to_len ) 
{
   c4ltoa45( 5231808 - l, to, -to_len ) ;
   return to ;
}

/* S4CLIPPER */
int S4FUNCTION c4descend( FIELD4 *f4, char *to, int to_len )
{
   int len ;

   if( f4->type == 'D' )
   {
      c4descend_date( to, f4long(f4), to_len ) ;
      return to_len ;
   }

   len = f4len(f4) ;
   if( len > to_len )
      return -1 ;

   switch( f4->type )
   {
      case 'C':
         c4descend_str( to, f4ptr(f4), to_len ) ;
         break ;
      case 'N':
         c4descend_num( to, f4ptr(f4), len ) ;
         break ;
      default:
         break ;
   }
   return len ;
}

/* S4CLIPPER */
void S4FUNCTION c4dtoa_clipper( double val, char *result, int len, int decimals )
{
   int dig_len, is_neg, zeros_len, dec_pos, dec_len, result_len, i ;
   char *str, *ptr ;

   result_len = len ;
   str = fcvt( val, decimals, &dig_len, &is_neg) ;

   zeros_len = result_len - dig_len - decimals - (decimals > 0 ) ;
   if ( zeros_len > 0 )
      memset( result, '0', zeros_len ) ;

   if ( decimals > 0 )
   {
      dec_pos = result_len - decimals - 1 ;
      result[dec_pos] = '.' ;
   }
   else
      dec_pos = result_len ;

   if ( zeros_len > 0 )
      ptr = result + zeros_len ;
   else
      ptr = result ;

   if ( dig_len >= 0 )
   {
      if ( (dec_pos - dig_len) < 0 )
      {
         memset( ptr, (int) '*', (size_t)result_len) ;
         return ;
      }
      memcpy( ptr, str, (size_t) dig_len ) ;
      if ( zeros_len >= 0 && decimals > 0 )
      {
         ptr += dig_len ;
         *ptr = '.' ;
         memcpy( ++ptr, str+dig_len, (size_t)decimals ) ;
      }
   }
   else
   {
      dec_len = decimals + dig_len ;
      if ( dec_len > 0 )
         memcpy( ptr - dig_len, str, dec_len ) ;
   }

   if ( is_neg )
   {
      for ( i=0; i< result_len; i++ )
         result[i] = (char) 0x5c - result[i] ;
   }
}

/* S4CLIPPER */
/* Numeric Key Database Output is Converted to Numeric Key Index File format */
int   S4FUNCTION c4clip( char *ptr, int len)
{
   int         i, negative ;
   char *p ;

   for ( i= negative= 0, p= ptr; i< len; i++, p++ )
   {
      if ( *p == ' ' )
      {
         *p = '0' ;
      }
      else
      {
         if ( *p == '-' )
         {
            *p = '0' ;
            negative = 1 ;
         }
         else
            break ;
      }
   }

   if ( negative )
   {
      for ( i= 0, p= ptr; i< len; i++, p++ )
         *p = (char) 0x5c - *p ;
   }

   return 0 ;
}
#endif  /* S4CLIPPER */

/* c4dtoa45
   - formats a double to a string
   - if there is an overflow, '*' are returned
*/
void  S4FUNCTION c4dtoa45( double doub_val, char *out_buffer, int len, int dec)
{
   int pre_len, post_len, sign_pos, dec_val, sign_val ;
   char *result ;

   #ifdef S4DEBUG
      if ( len < 0 || len >128 || dec < 0 || dec >= len )
         e4severe( e4info, E4_C4DTOA45 ) ;
   #endif  /* S4DEBUG */

   memset( out_buffer, (int) '0', (size_t) len) ;

   if (dec > 0)
   {
      post_len = dec ;
      if (post_len > 15)     post_len = 15 ;
      if (post_len > len-1)  post_len = len-1 ;
      pre_len = len -post_len -1 ;

      out_buffer[ pre_len] = '.' ;
   }
   else
   {
      pre_len = len ;
      post_len = 0 ;
   }

   result = (char *)fcvt( doub_val, post_len, &dec_val, &sign_val) ;

   if (dec_val > 0)
      sign_pos = pre_len-dec_val -1 ;
   else
   {
      sign_pos = pre_len - 2 ;
      if ( pre_len == 1) sign_pos = 0 ;
   }

   if ( dec_val > pre_len ||  pre_len<0  ||  sign_pos< 0 && sign_val)
   {
      /* overflow */
      memset( out_buffer, (int) '*', (size_t) len) ;
      return ;
   }

   if (dec_val > 0)
   {
      memset( out_buffer, (int)' ', (size_t)(pre_len - dec_val) ) ;
      memmove( out_buffer + pre_len - dec_val, result, (size_t)dec_val) ;
      if ( ( out_buffer[pre_len-1] == '\0' ) && (pre_len > 0) )
         out_buffer[pre_len-1] = '0' ;
   }
   else
   {
      if (pre_len> 0)  memset( out_buffer, (int) ' ', (size_t) (pre_len-1)) ;
   }
   if ( sign_val)  out_buffer[sign_pos] = '-' ;


   out_buffer += pre_len+1 ;
   if (dec_val >= 0)
   {
      result+= dec_val ;
   }
   else
   {
      out_buffer    -= dec_val ;
      post_len += dec_val ;
   }

   if ( post_len > (int) strlen(result) )
      post_len = (int) strlen( result) ;

   /*  - out_buffer   points to where the digits go to
       - result       points to where the digits are to be copied from
       - post_len     is the number to be copied
   */

   if (post_len > 0)   memmove( out_buffer, result, (size_t) post_len) ;
}

/* c4encode

   - From CCYYMMDD to CCYY.MM.DD

   Ex.        c4encode( to, from, "CCYY.MM.DD", "CCYYMMDD" ) ;
*/

void  S4FUNCTION c4encode( char *to, char *from, char *t_to, char *t_from)
{
   int pos ;
   char chr ;
   char *chr_pos ;

   strcpy( to, t_to ) ;

   while ( (chr = *t_from++) != 0)
   {
      if ( ( chr_pos= strchr( t_to, chr ) ) ==0 )
      {
         from++;
         continue ;
      }

      pos = (int)( chr_pos - t_to ) ;
      to[pos++] = *from++ ;

      while (chr == *t_from)
      {
         if (chr == t_to[pos] )
            to[pos++] = *from ;
         t_from++ ;
         from++ ;
      }
   }
}

/*  c4ltoa45

    Converts a RECNUM to a string.  Fill with '0's rather than blanks if
    'num' is less than zero.
*/

void  S4FUNCTION c4ltoa45( long l_val, char *ptr, int num)
{
   int   n, num_pos ;
   long  i_long ;

   i_long = (l_val>0) ? l_val : -l_val ;
   num_pos = n = (num > 0) ? num : -num ;

   while (n-- > 0)
   {
      ptr[n] = (char) ('0'+ i_long%10) ;
      i_long = i_long/10 ;
   }

   if ( i_long > 0 )
   {
     memset( ptr, (int) '*', (size_t) num_pos ) ;
     return ;
   }

   num--;
   for (n=0; n<num; n++)
      if (ptr[n]=='0')
         ptr[n]= ' ';
      else
         break ;

   if (l_val < 0)
   {
      if ( ptr[0] != ' ' )
      {
         memset( ptr, (int) '*', (size_t) num_pos ) ;
         return ;
      }
      for (n=num; n>=0; n--)
         if (ptr[n]==' ')
         {
            ptr[n]= '-' ;
            break ;
         }
   }
}

void S4FUNCTION c4trim_n( char *str, int n_ch )
{
   int len ;

   if ( n_ch <= 0 )
      return ;

   /* Count the Length */
   len = 0 ;
   while ( len< n_ch )
   {
      len++ ;
      if ( *str++ == '\0' )
         break ;
   }

   if ( len < n_ch )
      n_ch = len ;

   *(--str) = '\0' ;

   while( --n_ch > 0 )
   {
      str-- ;
      if ( *str == '\0' ||  *str == ' ' )
         *str = '\0' ;
      else
         break ;
   }
}

#ifndef S4LANGUAGE

void S4FUNCTION c4lower( char *str )
{
#ifdef S4NO_STRLWR
   char *ptr ;
   
   ptr = str ;
   
   while ( *ptr != '\0' )
   {
      if ( *ptr >= 'A' && *ptr <= 'Z' )
       *ptr |= 0x20 ;
      ptr++ ;
   }
#else
   #ifdef S4ANSI
      AnsiLower( str ) ;
   #else
      (void)strlwr( str ) ;
   #endif
#endif
}

void S4FUNCTION c4upper( char *str )
{
#ifdef S4NO_STRUPR
   char *ptr ;
   
   ptr = str ;
   
   while ( *ptr != '\0' )
   {
      if ( *ptr >= 'a'  &&  *ptr <= 'z' )
       *ptr &= 0xDF ;
      ptr++ ;
   }
#else
   #ifdef S4ANSI
      AnsiUpper( str ) ;
   #else
      (void)strupr( str ) ;
   #endif
#endif
}

#else  /* ifdef S4LANGUAGE  */

#ifdef S4GERMAN
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;
       
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
                   case '\204':
                      *ptr = '\216' ;
                      break ;
                   case '\224':
                      *ptr = '\231' ;
                      break ;
                   case '\201':
                      *ptr = '\232' ;
                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
   
   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
               case '\216':
                        *ptr = '\204' ;
                        break ;
               case '\231':
                        *ptr = '\224' ;
                        break ;
               case '\232':
                        *ptr = '\201' ;
                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4GERMAN */

#ifdef S4FRENCH
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;
   
         /* Les accents sont laiss‚ afin d'ˆtre compatible avec toute p‚riph‚rie */ 
         if ( *ptr >= E4C_CED )
         {
            switch( *ptr )
            {
               case E4A_TRE :
               case E4A_GRA :
                     case E4A_CIR :
                     case E4A_CI2 :
               case E4A_EGU :
                        *ptr = 'A' ;        /* A */
                        break ;
               case E4C_CED :      
                  *ptr = 'C' ;        /* C */
                  break ;                      
                     case E4E_EGU :
               case E4E_GRA :
                     case E4E_CIR :
                     case E4E_TRE :
                  *ptr = 'E' ;        /* E */
                  break ;
                     case E4I_TRE :
               case E4I_EGU :
               case E4I_GRA :
                     case E4I_CIR :
                  *ptr = 'I' ;        /* I */
                  break ;
                     case E4U_CIR :
                     case E4U_TRE :
                     case E4U_GRA :
                     case E4U_EGU :
                  *ptr = 'O' ;        /* O */
                  break ;
                     case E4O_CIR :
                     case E4O_GRA :
                     case E4O_TRE :
                     case E4O_EGU :
                  *ptr = 'U' ;        /* U */
                  break ;
                     case E4Y_TRE :
                  *ptr = 'Y' ;        /* Y */
                  break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
      
   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
   
         /* Les accents sont laiss‚ afin d'ˆtre compatible avec toute p‚riph‚rie */
         if ( *ptr >= E4C_CED )
         {
            switch( *ptr )
            {
               case E4CM_CED:
                  *ptr = 'c' ;       /* c */
                  break ;
               case E4AM_TRE:
               case E4AM_CIR:
                  *ptr = 'a' ;       /* a */
                  break ;
               case E4EM_EGU:
                  *ptr = 'e' ;       /* e */
                  break ;
               case E4OM_TRE:
                  *ptr = 'o' ;       /* o */
                  break ;
               case E4UM_TRE:
                  *ptr = 'u' ;       /* u */
                  break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif   /*  S4FRENCH  */

#ifdef S4SWEDISH
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;
       
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
                   case '\201':
                      *ptr = '\232' ;
                      break ;
                   case '\202':
                      *ptr = '\220' ;
                      break ;
                   case '\204':
                      *ptr = '\216' ;
                      break ;
                   case '\206':
                      *ptr = '\217' ;
                      break ;
                   case '\221':
                      *ptr = '\222' ;
                      break ;
                   case '\224':
                      *ptr = '\231' ;
                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
   
   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
               case '\216':
                        *ptr = '\204' ;
                        break ;
               case '\217':
                        *ptr = '\206' ;
                        break ;
               case '\220':
                        *ptr = '\202' ;
                        break ;
               case '\222':
                        *ptr = '\221' ;
                        break ;
               case '\231':
                        *ptr = '\224' ;
                        break ;
               case '\232':
                        *ptr = '\201' ;
                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4SWEDISH */

#ifdef S4FINISH
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;
       
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
                   case '\201':
                      *ptr = '\232' ;
                      break ;
                   case '\202':
                      *ptr = '\220' ;
                      break ;
                   case '\204':
                      *ptr = '\216' ;
                      break ;
                   case '\206':
                      *ptr = '\217' ;
                      break ;
                   case '\221':
                      *ptr = '\222' ;
                      break ;
                   case '\224':
                      *ptr = '\231' ;
                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
   
   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
               case '\216':
                        *ptr = '\204' ;
                        break ;
               case '\217':
                        *ptr = '\206' ;
                        break ;
               case '\220':
                        *ptr = '\202' ;
                        break ;
               case '\222':
                        *ptr = '\221' ;
                        break ;
               case '\231':
                        *ptr = '\224' ;
                        break ;
               case '\232':
                        *ptr = '\201' ;
                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4FINISH */

#ifdef S4NORWEGIAN
   void  S4FUNCTION c4upper( char *str )
   {
   #ifdef S4ANSI
      AnsiUpper(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'a'  &&  *ptr <= 'z' )
            *ptr &= 0xDF ;
       
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
                   case '\201':
                      *ptr = '\232' ;
                      break ;
                   case '\202':
                      *ptr = '\220' ;
                      break ;
                   case '\204':
                      *ptr = '\216' ;
                      break ;
                   case '\206':
                      *ptr = '\217' ;
                      break ;
                   case '\221':
                      *ptr = '\222' ;
                      break ;
                   case '\224':
                      *ptr = '\231' ;
                      break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
   
   void S4FUNCTION c4lower( char *str )
   {
   #ifdef S4ANSI
      AnsiLower(str) ;
   #else
      char *ptr ;
   
      ptr = str ;
   
      while ( *ptr != '\0' )
      {
         if ( *ptr >= 'A' && *ptr <= 'Z' )
            *ptr |= 0x20 ;
         if ( *ptr >= '\201' )
         {
            switch( *ptr )
            {
               case '\216':
                        *ptr = '\204' ;
                        break ;
               case '\217':
                        *ptr = '\206' ;
                        break ;
               case '\220':
                        *ptr = '\202' ;
                        break ;
               case '\222':
                        *ptr = '\221' ;
                        break ;
               case '\231':
                        *ptr = '\224' ;
                        break ;
               case '\232':
                        *ptr = '\201' ;
                        break ;
            }
         }
         ptr++ ;
      }
   #endif
   }
#endif /* S4NORWEGIAN */

#endif   /*  S4LANGUAGE  */


#ifdef S4NO_MEMMOVE
void *memmove(void *dest, void *src, size_t count )
{
   if ( dest < src )
      if ( (char *)dest + count  <= (char *) src )
      {
         memcpy( dest, src, count ) ;
         return( src ) ;
      }
      else
      {
         /* Start at beginning of 'src' */
         int  i ;
         for ( i=0; i< count; i++ )
            ((char *) dest)[i] = ((char *)src)[i] ;
      }

   if ( src < dest )
      if ( (char *) src + count  <= (char *) dest )
      {
         memcpy( dest, src, count ) ;
         return( src ) ;
      }
      else
      {
         /* Start at end of 'src' */
         for(;count!=0;)
         {
            --count ;
            ((char *)dest)[count] = ((char *) src)[count] ;
         }
      }

   return( src ) ;
}
#endif  /* S4NO_MEMMOVE */

#ifdef S4NO_STRNICMP
int strnicmp(char *a, char *b, size_t n )
{
   unsigned char  a_char, b_char ;

   for ( ; *a != '\0'  &&  *b != '\0' && n != 0; a++, b++, n-- )
   {
      a_char = (unsigned char) *a  & 0xDF ;
      b_char = (unsigned char) *b  & 0xDF ;

      if ( a_char < b_char )  return -1 ;
      if ( a_char > b_char )  return  1 ;
   }
   return 0 ;
}
#endif  /* S4NO_STRNICMP */

#ifndef S4OPTIMIZE_OFF
/* used by optimization */
int S4FUNCTION c4calc_type( unsigned long l )
{
   int i ;

   for ( i = 0 ; l > 1 ; i++ )
      l >>= 1 ;
   return i ;
}

#endif
