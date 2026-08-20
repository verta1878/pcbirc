/* r4total.c  (c)Copyright Sequiter Software Inc., 1991-1992.  All rights reserved. */

#include "d4all.h"
#include <math.h>

#ifdef __TURBOC__
   #pragma hdrstop
#endif


void S4FUNCTION total4value_update( TOTAL4 *t4, GROUP4 *group_reset )
{
   double tvalue;

   if( t4->reset_level && group_reset)
      if( group_reset->position <= t4->reset_level->position )
      {
         t4->low = 1.7 * pow( 10,308 ) ;   
         t4->high = -1.7 * pow( 10,308 ) ;
         t4->total = 0.0 ;
         t4->count = 0;
      }
      
   tvalue = expr4double( t4->calc_ptr->expr ) ;
   t4->total += tvalue ;
   t4->count += 1 ;
   if ( tvalue < t4->low )
      t4->low = tvalue ;

   if( tvalue > t4->high )
      t4->high = tvalue ;
   
}

void S4FUNCTION total4value_reset(TOTAL4 *t4)
{
   t4->low = 1.7 * pow(10,308 ) ;
   t4->high = -1.7 * pow( 10,308 ) ;
   t4->total = 0.0 ;
   t4->count = 0;
}

void S4FUNCTION total4reset_level( TOTAL4 *t4, GROUP4 *g4 )
{
   t4->reset_level =  g4 ;
}


TOTAL4 * S4FUNCTION total4create_total( REPORT4 *r4, char *name, EXPR4 *expr, int total_type )
{
   TOTAL4 *t4 =  (TOTAL4 *)  mem4create_alloc( r4->cb,
   &r4->cb->total_memory, 5, sizeof(TOTAL4), 5, 0);
   if ( t4 == 0 )
   {
      e4(r4->cb,e4report,E4_REPORT_ATOTAL);
      return 0 ;
   }

   t4->calc_ptr =  expr4calc_create( r4->cb, expr, name) ;
   if( t4->calc_ptr == 0 )
   {
      mem4free( r4->cb->total_memory, t4 ) ;
      return 0 ;
   }
   t4->calc_ptr->total =  t4 ;
   t4->total_type =  (char) total_type ;
   t4->report =  r4 ;
   t4->total = 0.0;
   t4->count = 0;
   t4->low = 1.7 * pow( 10,308 ) ;
   t4->high = -1.7 * pow( 10,308 ) ;
   l4add( &r4->cb->total_list, t4 ) ;
   return t4 ;
}

TOTAL4 * S4FUNCTION total4create(REPORT4 *report, char *name, char *expr_buf, int total_type)
{

   EXPR4 *expr;

   expr = expr4parse(report->relate->data,expr_buf);
   if(expr == 0)
   {
      e4set(report->cb,0);
      return NULL;
   }

   return (total4create_total(report,name,expr,total_type));

}

TOTAL4 * S4FUNCTION total4lookup( REPORT4 *r4, char *name )
{
   TOTAL4 *total_on ;
   CODE4 *c4 = r4->cb;

   total_on = 0;
   total_on = (TOTAL4 *) l4next( &c4->total_list,total_on);
   while(total_on)
   {
      if ( strcmp( name, total_on->calc_ptr->name ) == 0 )
         return total_on ;
      total_on = (TOTAL4 *) l4next( &c4->total_list,total_on);
   }
   return 0 ;
}




