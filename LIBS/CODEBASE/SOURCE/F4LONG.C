/* f4long.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign_long( FIELD4 *field, long l_value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN_LONG ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_LONG ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return ;

   if ( field->type == 'D' )
      date4assign( f4assign_ptr( field ), l_value ) ;
   else
   {
      if ( field->dec == 0 )
         c4ltoa45( l_value, f4assign_ptr( field ), field->len ) ;
      else
         f4assign_double( field, (double)l_value ) ;
   }
}
#endif

long S4FUNCTION f4long( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4LONG ) )
         return 0L ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4LONG ) ;
   #endif

   if ( field->type == 'D' )
      return date4long( f4ptr( field ) ) ;

   return c4atol( f4ptr( field ), field->len ) ;
}
