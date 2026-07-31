/* s4string.cpp (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#ifdef __TURBOC__
   #pragma warn -par
#endif

#include "d4all.h"
#include "d4data.hpp"
#pragma hdrstop

static int  num_per_type[16] = { 8, 6, 4, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

static int  calc_type( unsigned l )
{
   long n = 16 ;
   int  i = 0 ;

   while ( (long) l > n )
   {
      i++ ;
      n <<= 1 ;
   }
   return i ;
}

static unsigned calc_max( int i_type )
{
   unsigned result =  1 << (unsigned) (i_type+4) ;
   if ( result <= 0xFE00 )
      return result ;
   return 0xFE00 ;
}

Str4flex::Str4flex( Code4 *cb ) : Str4max(0,0)
{
   code_base =  cb ;
}

Str4flex::Str4flex( Str4flex& a ) : Str4max(0,0)
{
   code_base =  a.code_base ;
   if ( set_len( a.cur_len ) < 0 )  return ;
   memcpy( p, a.p, a.cur_len ) ;
}

Str4flex::~Str4flex()
{
   free() ;
}

void *Str4flex::operator new( size_t z )
{
   return u4alloc( sizeof( Str4flex ) ) ;
}

void Str4flex::operator delete( void *p )
{
   u4free( p ) ;
}

void Str4flex::free()
{
   if ( p != 0 )
   {
      int i_type = calc_type( max_len ) ;
      mem4free( code_base->string_types[i_type], p ) ;
      p =  0 ;
   }
}

Str4flex::set_max( unsigned n )
{
   if ( code_base->error() )  return -1 ;
   if ( n >= UINT_MAX )  return e4( code_base, e4memory, 0 ) ;

   if ( n == 0 )
   {
      free() ;
      return cur_len =  max_len =  0 ;
   }
   if ( cur_len == 0 )  free() ;

   int i_type =  calc_type( n+1 ) ;
   unsigned new_max =  calc_max(i_type)-1 ;
   if ( new_max < n )  return e4( code_base, e4memory, 0 ) ;

   #ifdef S4DEBUG
      if ( i_type > 16 )
	 e4severe( e4parm, "Str4flex::set_max" ) ;
      if ( calc_type(new_max) != i_type )
	 e4severe( e4parm, "Str4flex::set_max" ) ;
   #endif

   if ( code_base->string_types[i_type] == 0 )
   {
      code_base->string_types[i_type] = mem4create( 0, num_per_type[i_type], new_max+1, num_per_type[i_type], 0 ) ;
      if ( code_base->string_types[i_type] == 0 )  return e4( code_base, e4memory, 0 );
   }

   char *new_p =  (char *) mem4alloc( code_base->string_types[i_type] ) ;
   if ( new_p == 0 )  return e4( code_base, e4memory, 0 ) ;

   if ( cur_len > new_max )  cur_len =  new_max ;
   memcpy( new_p, p, cur_len ) ;

   free() ;
   max_len =  new_max ;
   p =  new_p ;
   p[max_len] =  0 ;

   return 0 ;
}


