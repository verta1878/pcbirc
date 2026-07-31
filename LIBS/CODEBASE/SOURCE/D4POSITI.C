/* d4positi.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

double S4FUNCTION d4position( DATA4 *data )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag_on ;
   #endif
   long count ;
   int rc, len ;
   char *result ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4POS ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4POS ) ;
   #endif

   if( data->code_base->error_code < 0 )
      return -1 ;

   if ( d4eof( data ) )
      return 1.1 ;

   #ifndef S4INDEX_OFF
      tag_on = d4tag_selected( data ) ;
      if ( tag_on == 0 || data->rec_num <= 0L )
      {
   #endif
      count = d4reccount( data ) ;
      if ( count < 0 )
         return -1.0 ;
      if ( count == 0 || data->rec_num <= 0L )
         return 0.0 ;

      return (double)( data->rec_num - 1 ) / ( count - (count != 1 ) ) ;
   #ifndef S4INDEX_OFF
      }
      else
      {
         /* in fox case, do seek anyway, in order to update parent blocks */
         #ifndef S4FOX
            if ( t4recno( tag_on ) == data->rec_num )
               return t4position( tag_on ) ;
         #endif

         len = t4expr_key( tag_on, &result ) ;
         if ( len < 0 )
            return -1.0 ;
         t4version_check( tag_on, 0 ) ;
         rc = t4seek( tag_on, result, len ) ;
         if ( rc != 0  && rc != r4eof && rc != r4after )
            return -1.0 ;

         return t4position( tag_on ) ;
      }
   #endif
}

int S4FUNCTION d4position2( DATA4 *data, double *result )
{
   *result = d4position( data ) ;
   if ( *result < 0.0 )
      return -1;
   return 0;
}

int S4FUNCTION d4position_set( DATA4 *data, double per )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag_on ;
   #endif
   long new_rec, count ;
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4POS_SET ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4POS_SET ) ;
   #endif

   if( data->code_base->error_code < 0 )
      return -1 ;

   if ( per > 1.0 )
      return d4go_eof( data ) ;
   if ( per <= 0 )
      return d4top( data ) ;

   #ifndef S4INDEX_OFF
      tag_on = d4tag_selected( data ) ;
      if ( tag_on == 0 )
      {
   #endif
      count = d4reccount( data ) ;
      if ( count <= 0L )
         return d4go_eof( data ) ;

      new_rec = (long)( per * ( (double)count - 1 ) + 1.5 ) ;
      if ( new_rec > count )
         new_rec = count ;
   #ifndef S4INDEX_OFF
      }
      else
      {
         rc = t4position_set( tag_on, per ) ;
         if ( rc )
            return rc ;
         if ( rc == r4eof )
            return d4go_eof( data ) ;

         new_rec = t4recno( tag_on ) ;
      }
   #endif

   return d4go( data, new_rec ) ;
}
