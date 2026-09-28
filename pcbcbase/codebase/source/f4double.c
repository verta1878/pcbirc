/* f4double.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

/* Returns the value of the corresponding field as a double.
   Only defined for 'Numeric' fields and 'Character' fields
   containing numeric data.
*/

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign_double( FIELD4 *field, double d_value )
{
   #ifdef S4VBASIC
      if ( c4parm_check ( field, 3, E4_F4ASSIGN_DBL ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_DBL ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return ;

   if ( field->type == 'D' )
      date4assign( f4assign_ptr( field ), (long)d_value ) ;
   else
      /* Convert the 'double' to ASCII and then copy it into the record buffer. */
      c4dtoa45( d_value, f4assign_ptr( field ), field->len, field->dec ) ;
}
#endif

double S4FUNCTION f4double( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4DOUBLE ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4DOUBLE ) ;
   #endif

   if ( field->type == 'D' )
      return (double)date4long( f4ptr( field ) ) ;

   /* Convert the field data into a 'double' */
   return( c4atod( f4ptr( field ), field->len ) ) ;
}

int S4FUNCTION f4double2( FIELD4 *field, double *result )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4DOUBLE2 ) ;
   #endif

   *result = f4double( field ) ;
   return 0 ;
}
