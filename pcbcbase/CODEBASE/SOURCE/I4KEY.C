/* i4key.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

char S4PTR *S4FUNCTION t4key( TAG4 S4PTR *t4 )
{
   B4BLOCK *b4 ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4KEY ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return 0 ;

   b4 = (B4BLOCK *)(t4->blocks.last_node) ;

   if ( b4 == 0 )
      return 0 ;

   #ifdef S4FOX
      if ( b4->key_on >=  b4->header.n_keys )  /* eof */
   #else
      if ( b4->key_on >= b4->n_keys )  /* eof */
   #endif
         return 0 ;

   return (char *)b4key_key( b4, b4->key_on ) ;
}

#endif

#ifdef S4VB_DOS

char * t4key_v( TAG4 *t4 )
{
   int len ;

   #ifdef S4VBASIC
      if ( c4parm_check( t4->code_base, 1, "t4key():" ) ) return 0 ;
   #endif

   StringRelease( basic_desc );
  
   if( ! (len =  expr4len( t4->expr ) ) )
      return basic_desc ;

   StringAssign( t4key( t4 ), len, basic_desc, 0 );

   return basic_desc ;

}

#endif
