/* f4ass_f.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign_field( FIELD4 *field_to, FIELD4 *field_from )
{
   #ifdef S4DEBUG
      if ( field_to == 0 || field_from == 0 )
         e4severe( e4parm, E4_F4ASSIGN_FLD ) ;
   #endif

   if ( field_to->data->code_base->error_code < 0 )
      return ;

   switch( field_to->type )
   {
      case 'C':
         f4assign_n( field_to, f4ptr( field_from ), field_from->len ) ;
         break ;
      case 'N':
      case 'F':
         if( field_from->len == field_to->len && field_from->dec == field_to->dec &&
             (field_from->type == 'N' || field_from->type == 'F') )
            memcpy( f4assign_ptr( field_to ), f4ptr( field_from ), field_to->len ) ;
         else
            f4assign_double( field_to, f4double( field_from ) ) ;
         break ;
      case 'D': 
         if( field_from->type == 'D' )
            memcpy( f4assign_ptr( field_to ), f4ptr( field_from ), field_to->len ) ;
         break ;
      case 'L':
         if( field_from->type == 'L' )
            *f4assign_ptr( field_to ) = *f4ptr( field_from ) ;
         break ;
      default:
         #ifdef S4DEBUG
            e4severe( e4info, E4_INFO_IFT ) ;
         #endif
         break ;
   }
}
#endif

