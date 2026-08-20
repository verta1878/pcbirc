/* f4info.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

FIELD4INFO *S4FUNCTION d4field_info( DATA4 *d4 )
{
   FIELD4INFO *field_info ;
   FIELD4 *field ;
   int i ;

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4FIELD_INFO ) ;
   #endif

   if ( d4->code_base->error_code < 0 )
      return 0 ;

   field_info = (FIELD4INFO *)u4alloc_free( d4->code_base, ( d4num_fields( d4 ) + 1 ) * (long)sizeof( FIELD4INFO ) ) ;
   if ( field_info == 0 )
      return 0 ;

   for ( i = 0 ; i < d4num_fields( d4 ) ; i++ )
   {
      field = d4field_j( d4, i + 1 ) ;
      field_info[i].name = field->name ;
      field_info[i].len = field->len ;
      field_info[i].dec = field->dec ;
      field_info[i].type = (char) field->type ;
   }

   return field_info ;
}
