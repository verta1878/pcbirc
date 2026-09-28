/* f4field.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
char *S4FUNCTION f4assign_ptr( FIELD4 *field )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_PTR ) ;
   #endif

   field->data->record_changed = 1 ;

   return ( field->data->record + field->offset ) ;
}

void S4FUNCTION f4blank( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4BLANK ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4BLANK ) ;
   #endif

   memset( f4assign_ptr( field ), ' ', field->len ) ;
}
#endif

DATA4 *S4FUNCTION f4data( FIELD4 *field )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4DATA ) ;
   #endif

   return field->data ;
}

int S4FUNCTION f4decimals( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4DECIMALS ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4DECIMALS ) ;
   #endif

   return field->dec ;
}

unsigned S4FUNCTION f4len( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4LEN ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4LEN ) ;
   #endif

   return field->len ;
}

char *S4FUNCTION f4name( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4NAME ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4NAME ) ;
   #endif

   return field->name ;
}

int  S4FUNCTION f4type( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4TYPE ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4TYPE ) ;
   #endif

   if ( field->type == 'F' )
      return (int)r4num ;
   else
      return (int)field->type ;
}

#ifdef S4VBASIC

long S4FUNCTION f4len_v( FIELD4 *f4 )
{
   return (long) f4len( f4 );
}

#ifdef S4VB_DOS

char * f4name_v( FIELD4 *fld )
{
 return v4str( f4name(fld) ) ;
}

#endif /* S4VB_DOS */

#endif /* S4VBASIC */
