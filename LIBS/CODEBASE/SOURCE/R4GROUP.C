/* r4group.c  (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif

GROUP4 * S4FUNCTION group4create( REPORT4 *r4 )
{
   GROUP4 *g4group, *group_on ;
   int i ;

   g4group =  (GROUP4 *) u4alloc_er( r4->cb, sizeof(GROUP4) ) ;
   if ( g4group == 0 )  
   {
      e4(r4->cb,e4report,E4_REPORT_AGROUP);
      return 0 ;
   }

   l4add( &r4->groups, g4group ) ;

   objectsort4init( &g4group->header, r4, g4group ) ;
   objectsort4init( &g4group->footer, r4, g4group ) ;
   strcpy( g4group->label, " " ) ;
   g4group->report =  r4 ;


   i = 1 ;
   group_on=0;
   group_on=(GROUP4 *)l4next(&r4->groups,group_on);
   while(group_on)
   {
      group_on->position =  i ;
      group_on=(GROUP4 *)l4next(&r4->groups,group_on);
      i++ ;
   }

   return g4group ;
}


int S4FUNCTION group4expr( GROUP4 *g4, char *expr )
{
   char *ptr;
   EXPR4 *expr_new ;

   for( ptr = expr; *ptr == ' '; ptr++ ) ;
   if( *ptr == 0 )
   {
      expr4free( g4->expr ) ;
      g4->expr = 0 ;
      return 0 ;
   }
   expr_new =  expr4parse( g4->report->relate->data, expr ) ;
   if ( expr_new == 0 )
      return -1 ;

   expr4free( g4->expr ) ;
   g4->expr =  expr_new ;
   return 0 ;
}

GROUP4 * S4FUNCTION group4lookup( REPORT4 *r4, char *ptr )
{
   GROUP4 *group_on;
   group_on = 0;
   group_on = (GROUP4 *) l4next( &r4->groups, group_on);
   while(group_on)
   {
      if( strcmp( ptr, group_on->label ) == 0 )
         return group_on ;
      group_on = (GROUP4 *) l4next( &r4->groups, group_on);
   }
   return 0 ;
}

int S4FUNCTION group4name_set( GROUP4 *group, char *name )
{
   return u4ncpy( group->label, name, sizeof(group->label) ) ;
}


