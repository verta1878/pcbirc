/*s4str.cpp (c)Copyright Sequiter Software Inc., 1990-1991.  All rights reserved.*/

#include "d4all.h"
#include "d4data.hpp"

#include <ctype.h>
#include <limits.h>

extern char v4buffer[257] ;

#ifdef S4DEBUG
/* Ensure the strings do not overlap */
void s4asser_no_overlap( Str4& s1, Str4& s2 )
{
   if ( s1.maximum() == 0  ||  s2.maximum() == 0 )  return ;

   unsigned s1_end_pos =  s1.maximum()-1 ;
   unsigned s2_end_pos =  s2.maximum()-1 ;

   long  s1_ptr =  long( s1.ptr() ) ;
   long  s2_ptr =  long( s2.ptr() ) ;

   if ( s1_ptr == 0  ||  s2_ptr == 0 )
      e4severe( e4parm, E4_PARM_NSD ) ;

   long  s1_end_ptr =  s1_ptr + s1_end_pos ;
   long  s2_end_ptr =  s2_ptr + s2_end_pos ;

   if ( s1_ptr     >= s2_ptr  && s1_ptr     <= s2_end_ptr      ||
        s1_end_ptr >= s2_ptr  && s1_end_ptr <= s2_end_ptr      ||
        s2_ptr     >= s1_ptr  && s2_ptr     <= s1_end_ptr      ||
        s2_end_ptr >= s1_ptr  && s2_end_ptr <= s1_end_ptr )
      e4severe( e4parm, E4_PARM_OSD ) ;
}
#endif


Str4::operator char()
{
   if ( len() < 1 || ptr() == 0 )  return 0 ;
   return  *ptr() ;
}

Str4::operator double()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str4::operator double() called with a null pointer" ) ;
   #endif
   return c4atod( ptr(), len()) ;
}

Str4::operator int()
{
   char *p =  ptr() ;
   #ifdef S4DEBUG
      if( p == 0 ) e4severe( e4parm, "Str4::operator int() called with a null pointer" ) ;
   #endif
   int i =  len() ;
   int   r =  (int) c4atol( p,i ) ;
   return r ;
}

Str4::operator long()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str4::operator long() called with a null pointer" ) ;
   #endif
   return c4atol( ptr(), len()) ;
}

int Str4::operator==( Str4& s )
{
   #ifdef S4DEBUG
      if( ptr() == 0 || s.ptr() == 0 ) e4severe( e4parm, "Str4::operator == called with a null pointer" ) ;
   #endif
   unsigned l =  len() ;
   if ( l != s.len() )  return 0 ;

   if ( memcmp( ptr(), s.ptr(), l) == 0 )
      return 1 ;
   else
      return 0 ;
}

int Str4::operator< ( Str4& s )
{
   #ifdef S4DEBUG
      if( ptr() == 0 || s.ptr() == 0 ) e4severe( e4parm, "Str4::operator < called with a null pointer" ) ;
   #endif
   unsigned cmp_len ;
   int      result ;
   unsigned l1 =  len() ;
   unsigned l2 =  s.len() ;

   cmp_len =  (l1<l2) ? l1 : l2 ;
   result =  memcmp( ptr(), s.ptr(), cmp_len ) ;
   if ( result == 0 )  return  l1 < l2 ;
   return result < 0 ;
}

int Str4::operator> ( Str4& s )
{
   #ifdef S4DEBUG
      if( ptr() == 0 || s.ptr() == 0 ) e4severe( e4parm, "Str4::operator > called with a null pointer" ) ;
   #endif
   unsigned cmp_len ;
   int      result ;
   unsigned l1 =  len() ;
   unsigned l2 =  s.len() ;

   cmp_len =  (l1<l2) ? l1 : l2 ;
   result =  memcmp( ptr(), s.ptr(), cmp_len ) ;
   if ( result == 0 )  return  l1 > l2 ;
   return result > 0 ;
}

int Str4::operator<= ( Str4& s )
{
   #ifdef S4DEBUG
      if( ptr() == 0 || s.ptr() == 0 ) e4severe( e4parm, "Str4::operator <= called with a null pointer" ) ;
   #endif
   return ! ( *this > s ) ;
}

int Str4::operator>= ( Str4& s )
{
   #ifdef S4DEBUG
      if( ptr() == 0 || s.ptr() == 0 ) e4severe( e4parm, "Str4::operator >= called with a null pointer" ) ;
   #endif
   return ! ( *this < s ) ;
}

int Str4::add( Str4& str_from )
{
   return insert( str_from, len() ) ;
}

int  Str4::at( Str4& search_str )
{
   #ifdef S4DEBUG
      if( ptr() == 0 || search_str.ptr() == 0 ) e4severe( e4parm, "Str::search() called with or has a null pointer" ) ;
   #endif
   unsigned len_search =  search_str.len() ;
   if ( len_search == 0 )  return 0 ;

   unsigned str_len  =  len() ;
   if ( str_len < len_search )  return -1 ;

   unsigned last_try =  str_len - len_search ;
   char *str_ptr =  ptr() ;
   char *search_ptr =  search_str.ptr() ;

   for ( unsigned i = 0; i <= last_try; i++ )
      if ( str_ptr[i]  == *search_ptr )
      {
         for ( unsigned j = 1; j < len_search; j++ )
            if ( str_ptr[i+j] != search_ptr[j] )  break ;
         if ( j >= len_search )  return int(i) ;
      }

   return -1 ;
}

int  Str4::insert( Str4& str_from, unsigned pos )
{
   #ifdef S4DEBUG
      s4asser_no_overlap( *this, str_from ) ;
   #endif

   changed() ;

   unsigned  start_to_len =  len() ;
   #ifdef S4DEBUG
      if ( pos > start_to_len )  e4severe( e4info, "Str::insert()" ) ;
   #endif
   unsigned  move_len =  start_to_len-pos ;

   unsigned  from_len =  str_from.len() ;
   long      want_to_len =  long(start_to_len) +from_len ;
   int rc ;
   if ( want_to_len > UINT_MAX )
      rc =  -1 ;
   else
      rc =  set_len( unsigned(want_to_len) ) ;

   unsigned to_len = len() ;

   if ( rc != 0 )
   {
      #ifdef S4DEBUG
	 if ( to_len < start_to_len )  e4severe( e4info, "Str::insert" ) ;
      #endif
      move_len  -=  unsigned( want_to_len - to_len ) ;

      unsigned space_avail =  to_len-pos ;
      if ( space_avail < from_len ) from_len =  space_avail ;
   }

   char *to_ptr =  ptr() ;
   char *to_pos =  to_ptr+pos ;
   memmove( to_pos+from_len, to_pos, move_len ) ;
   memcpy( to_pos, str_from.ptr(), from_len ) ;

   if ( to_len < maximum() )
      to_ptr[to_len] =  0 ;
   return rc ;
}

int Str4::assign( char *from_ptr )
{
   return assign( from_ptr, strlen(from_ptr) ) ;
}

int Str4::assign( char *from_ptr, unsigned from_len )
{
   #ifdef S4DEBUG
      if( from_ptr == 0 )
         e4severe( e4parm, "Str4::assign() called with a null pointer" ) ;
      s4asser_no_overlap( *this, Str4len(from_ptr, from_len) ) ;
   #endif

   changed() ;

   int rc = set_len( from_len ) ;
   unsigned to_len, cur_len ;
   to_len =  cur_len =  len() ;
   if ( to_len > from_len )
   {
      set( ' ' ) ;
      to_len =  from_len ;
   }

   char *p = ptr() ;
   memcpy( p, from_ptr, to_len ) ;
   if ( cur_len < maximum() )
      p[cur_len] =  0 ;
   return rc ;
}

int Str4::assign( Str4& str_from )
{
   return assign( str_from.ptr(), str_from.len() ) ;
}

void Str4::assign_double( double d, int new_len, int dec )
{
   changed() ;

   if ( new_len >= 0 )
      set_len( new_len ) ;

   char *p =  ptr() ;

   #ifdef S4DEBUG
      if( p == 0 ) e4severe( e4parm, "Str4::assign_double() called, but has a null pointer" ) ;
   #endif

   unsigned l = len() ;

   if ( dec < 0 )  dec =  decimals() ;
   c4dtoa45( d, p, l, dec ) ;
   if ( l < maximum() )
      p[l] =  0 ;
}

void Str4::assign_long( long lval, int new_len, int zeros_in_front )
{
   changed() ;

   char *p =  ptr() ;
   #ifdef S4DEBUG
      if( p == 0 ) e4severe( e4parm, "Str4::assign_long() called, but has a null pointer" ) ;
   #endif
   if ( new_len >= 0 )
      set_len(new_len) ;
   unsigned l = len() ;

   if ( zeros_in_front )
      c4ltoa45( lval, p, -l ) ;
   else
      c4ltoa45( lval, p, l ) ;

   if ( l < maximum() )
      p[l] =  0 ;
}

char *Str4::end_ptr()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str::end_ptr() called, but has a null pointer" ) ;
   #endif
   unsigned pos =  len() ;
   if ( pos == 0 ) return 0 ;
   pos-- ;
   return ptr()+pos ;
}

Str4len Str4::left( unsigned want_len )
{
   return substr( 0, want_len ) ;
}

unsigned Str4::len()
{
   if ( ptr() == 0 ) return 0 ;
   return strlen(ptr()) ;
}

void Str4::lower()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str::lower() called, but has a null pointer" ) ;
   #endif
   changed() ;

   unsigned str_len = len() ;
   char    *str_ptr =  ptr() ;
   for ( unsigned i = 0; i < str_len; i++ )
      str_ptr[i] =  tolower( str_ptr[i] ) ;
}

unsigned Str4::ncpy( char *to, unsigned to_len )
{
   unsigned l = len() ;

   if ( to_len <= l )
   {
      if ( to_len-- > 0 )
      {
         memcpy( to, ptr(), to_len ) ;
         to[to_len] =  0 ;
         return to_len ;
      }
      return (unsigned) 0 ;
   }
   else
   {
      memcpy( to, ptr(), l ) ;
      to[l] =  0 ;
      return l ;
   }
}

int  Str4::replace( Str4& str, unsigned pos )
{
   #ifdef S4DEBUG
      s4asser_no_overlap( *this, str ) ;
   #endif

   changed() ;

   unsigned cur_len =  len() ;
   #ifdef S4DEBUG
      if ( pos > cur_len )  e4severe( e4parm, "Str::replace()" ) ;
   #endif
   unsigned copy_len =  str.len() ;
   long to_final_len =  long(copy_len) + pos ;
   int rc = 0 ;
   if ( to_final_len > UINT_MAX )
      rc =  -1 ;
   else
   {
      if ( to_final_len > cur_len )
      {
         rc =  set_len( unsigned(to_final_len) ) ;
         cur_len =  len() ;
      }
   }
   if ( rc < 0 )
      copy_len =  cur_len - pos ;

   char *p = ptr() ;
   memcpy( p+pos, str.ptr(), copy_len ) ;
   if ( cur_len < maximum() )  p[cur_len] =  0 ;
   return rc ;
}

Str4len Str4::right( unsigned want_len )
{
   unsigned cur_len =  len() ;
   if ( want_len > cur_len )  want_len =  cur_len ;
   return substr( cur_len - want_len, want_len ) ;
}


void Str4::set( int chr_value )
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str::null_ensure() called, but has a null pointer" ) ;
   #endif
   changed() ;

   unsigned l =  len() ;
   char *p =  ptr() ;
   memset( p, chr_value, l ) ;
   if ( l < maximum() )  p[l] =  0 ;
}

char *Str4::str()
{
   ncpy( v4buffer, sizeof(v4buffer) ) ;
   return v4buffer ;
}

void Str4::trim()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str::trim() called, but has a null pointer" ) ;
   #endif
   changed() ;

   char *p =  ptr() ;
   for ( unsigned i = len();; )
   {
      if ( i == 0 )  break ;
      i-- ;

      if ( p[i] == ' ' || p[i] == 0 )
         p[i] =  0 ;
      else
      {
         set_len(i+1) ;
         return ;
      }
   }
   set_len(0) ;
}

int Str4::true()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str4::true() called, but has a null pointer" ) ;
   #endif
   char ch =  char(*this) ;
   if ( ch == 'Y' || ch == 'y' || ch == 'T' || ch == 't' )
      return 1 ;
   else
      return 0 ;
}

void Str4::upper()
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str::upper() called, but has a null pointer" ) ;
   #endif
   changed() ;

   char *start_ptr =  ptr() ;
   unsigned end_pos =  len() ;
   for ( unsigned pos = 0; pos < end_pos; pos++ )
      start_ptr[pos] = toupper( start_ptr[pos] ) ;
}

Str4len Str4::substr( unsigned pos, unsigned result_len )
{
   #ifdef S4DEBUG
      if( ptr() == 0 ) e4severe( e4parm, "Str::substr() called, but has a null pointer" ) ;
   #endif
   unsigned my_len =  len() ;

   if ( long(pos) + result_len > my_len )
   {
      if ( pos > my_len )
         result_len =  0 ;
      else
         result_len =  my_len - pos ;
   }

   return Str4len( ptr()+pos, result_len ) ;
}

int Str4max::set_len( unsigned new_len )
{
   if ( new_len <= max_len )
   {
      cur_len =  new_len ;
      if ( cur_len < max_len )
         p[cur_len] =  0 ;
      return 0 ;
   }
   if ( set_max( new_len )  < 0 )
   {
      set_max( max_len ) ;
      return -1 ;
   }
   return set_len( new_len ) ;
}

int  Str4ten::set_len( unsigned new_len )
{
   if ( new_len <= 10 )
   {
      cur_len =  new_len ;
      buf[cur_len] = 0 ;
      return 0 ;
   }
   set_len( 10 ) ;
   return -1 ;
}

int Str4large::set_len( unsigned new_len )
{
   if ( new_len <= 255 )
   {
      cur_len =  new_len ;
      buf[cur_len] = 0 ;
      return 0 ;
   }
   set_len( 255 ) ;
   return -1 ;
}

Str4ten::Str4ten()          
{ 
   cur_len = 0; 
   buf[0] = 0; 
   buf[10] = 0; 
}

Str4ten::Str4ten( char *p )          
{ 
   cur_len = 0; 
   buf[10] = 0; 
   assign(Str4ptr(p));
}

Str4char::Str4char( char ch )          
{ 
   c = ch ;
}
            
Str4large::Str4large()
{ 
   cur_len = 0; 
   buf[0] = 0; 
   buf[255] = 0;
}

Str4large::Str4large( char *p )          
{ 
   cur_len = 0; 
   buf[255] = 0; 
   assign(Str4ptr(p));
}

Date4::Date4()
{ 
   set( ' ' ) ; 
   dt[8] = 0 ;
}

Date4::Date4( long l )
{ 
   date4assign(dt, l) ; 
   dt[8] = 0 ;
}

Date4::Date4( char *p )          
{ 
   u4ncpy( dt, p, sizeof(dt) ) ;
}
