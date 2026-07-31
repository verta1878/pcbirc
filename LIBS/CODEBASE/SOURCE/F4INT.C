/* f4int.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign_int( FIELD4 *field, int i_value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN_INT ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_INT ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return ;

   if ( field->dec == 0 )
      c4ltoa45( (long)i_value, f4assign_ptr( field ), field->len ) ;
   else
      f4assign_double( field, (double)i_value ) ;
}
#endif

int S4FUNCTION f4int( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4INT ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4INT ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   /* Convert the field data into an 'int' */
   return c4atoi( f4ptr( field ), field->len ) ;
}
