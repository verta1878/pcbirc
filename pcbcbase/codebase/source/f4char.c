/* f4char.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign_char( FIELD4 *field, int chr )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN_CHAR ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_CHAR ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return ;

   f4blank( field ) ;
   *f4assign_ptr( field ) = (char)chr ;
}
#endif

int S4FUNCTION f4char( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4CHAR ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4CHAR ) ;
   #endif

   /* Return the first character of the record buffer. */
   return *f4ptr( field ) ;
}
