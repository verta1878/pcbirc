/* d4field.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

FIELD4 *S4FUNCTION d4field( DATA4 *data, char *field_name )
{
   int i ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4FIELD ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4FIELD ) ;
   #endif

   i =  d4field_number( data, field_name ) - 1 ;
   if ( i < 0 )
      return 0 ;

   return data->fields + i ;
}

FIELD4 *S4FUNCTION d4field_j( DATA4 *data, int j_field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4FIELDJ ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4FIELDJ ) ;
      if ( data->fields == 0 || j_field > data->n_fields || j_field <= 0 )
         e4severe( e4info, E4_D4FIELDJ ) ;
   #endif
   return data->fields + j_field - 1 ;
}

int S4FUNCTION d4field_number( DATA4 *data, char *field_name )
{
   char buf[256] ;
   int i ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4FIELD_NUM ) ;
   #endif

   if ( field_name )
   {
      u4ncpy( buf, field_name, sizeof( buf ) ) ;
      c4trim_n( buf, sizeof( buf ) ) ;
      c4upper( buf ) ;

      for ( i = 0 ; i < data->n_fields ; i++ )
         if ( !strcmp( buf, data->fields[i].name ) )
            return i + 1 ;
   }

   if ( data->code_base->field_name_error )
      e4( data->code_base, e4field_name, field_name ) ;

   return -1 ;
}

#ifdef S4VB_DOS

FIELD4 *d4field_v( DATA4 *d4, char *fld_name )
{
   return d4field( d4, c4str(fld_name) ) ;
}

int d4fieldNumber( DATA4 *d4, char *fld_name )
{
   return d4field_number( d4, c4str(fld_name) ) ;
}

#endif
