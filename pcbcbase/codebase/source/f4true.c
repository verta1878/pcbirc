/* f4true.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved.

   Returns a true or false.
*/

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#include <ctype.h>

int S4FUNCTION f4true( FIELD4 *field )
{
   char char_value ;

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4TRUE ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4TRUE ) ;
   #endif

   char_value = (char) toupper( *f4ptr( field ) ) ;
   return ( char_value == 'Y' || char_value == 'T' ) ? 1 : 0 ;
}
