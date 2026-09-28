/* l4link.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#define  LINK_PTR(p)  ((LINK4 *)p)

void S4FUNCTION l4add( LIST4 *list, void *item )
{
   #ifdef S4DEBUG
      if ( list == 0 || item == 0 )
         e4severe( e4parm, E4_L4ADD ) ;
   #endif
   l4add_after( list, list->last_node, item ) ;
}

void S4FUNCTION l4add_after( LIST4 *list, void *anchor, void *item )
{
   #ifdef S4DEBUG
      if ( list == 0 || item == 0 )
         e4severe( e4parm, E4_L4ADD_AFTER ) ;
      if ( l4seek( list, item ) == 1 )
         e4severe( e4info, E4_L4ADD_AFTER ) ;
      if ( list->last_node != 0 )
         if ( l4seek( list, anchor ) == 0 )
            e4severe( e4info, E4_L4ADD_AFTER ) ;
   #endif

   if ( list->last_node == 0 )
   {
      list->last_node = (LINK4 *)item ;
      LINK_PTR(item)->p = (LINK4 *)item ;
      LINK_PTR(item)->n = (LINK4 *)item ;
   }
   else
   {
      LINK_PTR(item)->p = (LINK4 *)anchor ;
      LINK_PTR(item)->n = LINK_PTR(anchor)->n ;
      LINK_PTR(anchor)->n->p= (LINK4 *)item ;
      LINK_PTR(anchor)->n = (LINK4 *)item ;
      if ( anchor == (void *)list->last_node )
         list->last_node = (LINK4 *)item ;
   }

   list->n_link++ ;
   #ifdef S4DEBUG
      l4check( list ) ;
   #endif
}

void S4FUNCTION l4add_before( LIST4 *list, void *anchor, void *item )
{
   #ifdef S4DEBUG
      if ( list == 0 || item == 0 )
         e4severe( e4parm, E4_L4ADD_BEFORE ) ;
      if ( l4seek( list, item ) == 1 )
         e4severe( e4info, E4_L4ADD_BEFORE ) ;
      if ( list->last_node != 0 )
         if ( l4seek( list, anchor ) == 0 )
            e4severe( e4info, E4_L4ADD_BEFORE ) ;
   #endif

   if ( list->last_node == 0 )
   {
      list->last_node = (LINK4 *)item ;
      LINK_PTR(item)->p = (LINK4 *)item ;
      LINK_PTR(item)->n = (LINK4 *)item ;
   }
   else
   {
      LINK_PTR(item)->n = (LINK4 *)anchor ;
      LINK_PTR(item)->p = LINK_PTR(anchor)->p ;
      LINK_PTR(anchor)->p->n= (LINK4 *)item ;
      LINK_PTR(anchor)->p = (LINK4 *)item ;
   }

   list->n_link++ ;
   #ifdef S4DEBUG
      l4check( list ) ;
   #endif
}

#ifdef S4DEBUG
void l4check( LIST4 *list )
{
   /* Check the Linked List */
   LINK4 *on_link ;
   int i ;

   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4CHECK ) ;
   #endif

   on_link = list->last_node ;
   if ( on_link == 0 )
   {
      if ( list->n_link != 0 )
         e4severe( e4info, E4_INFO_CRL ) ;
      return ;
   }

   if ( list->n_link == 0 )
      e4severe( e4info, E4_INFO_CRL ) ;

   for ( i = 1; i <= list->n_link; i++ )
   {
      if ( on_link->n->p != on_link  ||  on_link->p->n != on_link )
         e4severe( e4info, E4_INFO_CRL ) ;

      on_link = on_link->n ;

      if ( i == list->n_link || on_link == list->last_node )
         if ( i != list->n_link || on_link != list->last_node )
            e4severe( e4info, E4_INFO_WRO ) ;
   }
}
#endif

void *S4FUNCTION l4first( LIST4 *list )
{
   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4FIRST ) ;
   #endif

   if ( list->last_node == 0 )
      return 0 ;
   return (void *)list->last_node->n ;
}

void *S4FUNCTION l4last( LIST4 *list )
{
   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4LAST ) ;
   #endif

   return (void *)list->last_node ;
}

void *S4FUNCTION l4next( LIST4 *list, void *link )
{
   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4NEXT ) ;
   #endif

   if ( link == (void *)list->last_node )
      return 0 ;
   if ( link == 0 )
      return l4first( list ) ;

   return (void *)(LINK_PTR(link)->n) ;
}

void *S4FUNCTION l4pop( LIST4 *list )
{
   LINK4 *p ;

   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4POP ) ;
   #endif

   p = list->last_node ;
   l4remove( list, list->last_node ) ;
   return (void *)p ;
}

void *S4FUNCTION l4prev( LIST4 *list, void *link )
{
   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4PREV ) ;
   #endif

   if ( link == 0 )
       return (void *)list->last_node ;
   if ( (void *)list->last_node->n == link )
      return 0 ;

   return (void *)(LINK_PTR(link)->p) ;
}

void S4FUNCTION l4remove( LIST4 *list, void *item )
{
   if ( item == 0 )
      return ;

   #ifdef S4DEBUG
      if ( list == 0 )
         e4severe( e4parm, E4_L4REMOVE ) ;
   #endif

   #ifdef S4DEBUG
      /* Make sure the link being removed is on the linked list ! */
      if ( l4seek( list, item ) == 0 )
         e4severe( e4info, E4_INFO_LIN ) ;
   #endif

   if ( list->selected == item )
   {
      list->selected = (void *)(LINK_PTR(item)->p) ;
      if ( list->selected == item )
         list->selected = 0 ;
   }

   LINK_PTR(item)->p->n = LINK_PTR(item)->n ;
   LINK_PTR(item)->n->p = LINK_PTR(item)->p ;
   if ( item == (void *)list->last_node )
   {
      if ( list->last_node->p == list->last_node )
         list->last_node = 0 ;
      else
         list->last_node = list->last_node->p ;
   }

   list->n_link-- ;

   #ifdef S4DEBUG
      l4check( list ) ;
   #endif
}

/* returns true if the selected link is contained in the list */
int S4FUNCTION l4seek( LIST4 *list, void *item )
{
   LINK4 *link_on ;

   for ( link_on = 0 ;; )
   {
      link_on = (LINK4 *)l4next( list, link_on ) ;
      if ( link_on == 0 )
         break ;
      if ( (void *)link_on == item )
         return 1 ;
   }
   return 0 ;
}
