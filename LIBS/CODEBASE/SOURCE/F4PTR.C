/* f4ptr.c   (c)Copyright Sequiter Software Inc., 1990-1993, all rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

char *S4FUNCTION f4ptr( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4PTR ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4PTR ) ;
   #endif

   return ( field->data->record + field->offset ) ;
}
