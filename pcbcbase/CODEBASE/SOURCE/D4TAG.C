/* d4tag.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

TAG4 *S4FUNCTION d4tag( DATA4 *d4, char *tag_name )
{
   #ifndef S4INDEX_OFF
      char tag_lookup[11] ;
      TAG4 *tag_on ;

      #ifdef S4VBASIC
         if ( c4parm_check( d4, 2, E4_D4TAG ) )
            return 0 ;
      #endif

      #ifdef S4DEBUG
         if ( d4 == 0 || tag_name == 0 )
            e4severe( e4parm, E4_D4TAG ) ;
      #endif

      u4ncpy( tag_lookup, tag_name, sizeof( tag_lookup ) ) ;
      c4upper( tag_lookup ) ;

      for( tag_on = 0 ;; )
      {
         tag_on = d4tag_next( d4, tag_on ) ;
         if ( tag_on == 0 )
            break ;
         if ( strcmp( tag_on->alias, tag_lookup ) == 0 )
            return tag_on ;
      }

      if ( d4->code_base->tag_name_error )
         e4( d4->code_base, e4tag_name, tag_name ) ;
   #endif
   return 0 ;
}

TAG4 *S4FUNCTION d4tag_default( DATA4 *d4 )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
      INDEX4 *index ;

      #ifdef S4VBASIC
         if ( c4parm_check( d4, 2, E4_D4TAG_DEF ) )
            return 0 ;
      #endif

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4TAG_DEF ) ;
      #endif

      tag = d4tag_selected( d4 ) ;
      if ( tag )
         return tag ;

      index = (INDEX4 *) l4first( &d4->indexes ) ;
      if ( index )
      {
         tag = (TAG4 *) l4first( &index->tags ) ;
         if ( tag )
            return tag ;
      }
   #endif
   return 0 ;
}

TAG4 *S4FUNCTION d4tag_next( DATA4 *d4, TAG4 *tag_on )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      INDEX4 *i4 ;

      #ifdef S4VBASIC
         if ( c4parm_check( d4, 2, E4_D4TAG_NEXT ) )
            return 0 ;
      #endif

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4TAG_NEXT ) ;
      #endif

      if ( tag_on == 0 )
      {
         i4 = (INDEX4 *)l4first( &d4->indexes ) ;
         if ( i4 == 0 )
            return 0 ;
      }
      else
         i4 = tag_on->index ;

      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on )
         return tag_on ;

      i4 = (INDEX4 *)l4next( &d4->indexes, i4 ) ;
      if ( i4 == 0 )
         return 0 ;

      return (TAG4 *)l4first( &i4->tags ) ;
   #endif
}

TAG4 *S4FUNCTION d4tag_prev( DATA4 *d4, TAG4 *tag_on )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      INDEX4 *i4 ;

      #ifdef S4VBASIC
         if ( c4parm_check( d4, 2, E4_D4TAG_PREV ) )
            return 0 ;
      #endif

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4TAG_PREV ) ;
      #endif

      if ( tag_on == 0 )
      {
         i4 = (INDEX4 *)l4last( &d4->indexes ) ;
         if ( i4 == 0 )
            return 0 ;
      }
      else
         i4 = tag_on->index ;

      tag_on = (TAG4 *)l4prev( &i4->tags, tag_on ) ;
      if ( tag_on )
         return tag_on ;

      i4 = (INDEX4 *)l4prev( &d4->indexes, i4 ) ;
      if ( i4 == 0 )
         return 0 ;

      return (TAG4 *)l4last( &i4->tags ) ;
   #endif
}

void S4FUNCTION d4tag_select( DATA4 *d4, TAG4 *t4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4TAG_SELECT ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4TAG_SELECT ) ;
   #endif

   #ifdef S4INDEX_OFF
      if ( t4 != 0 )
         e4( d4->code_base, e4not_index, E4_D4TAG_SELECT ) ;
   #else
      #ifdef S4DEBUG
         if ( t4 )
            if ( t4->index->data != d4 )
               e4severe( e4parm, E4_D4TAG_SELECT ) ;
      #endif

      if ( t4 == 0 )
         d4->indexes.selected = 0 ;
      else
      {
         d4->indexes.selected = (void *) t4->index ;
         t4->index->tags.selected = (void *)t4 ;
      }
   #endif
}

TAG4 *S4FUNCTION d4tag_selected( DATA4 *d4 )
{
   #ifndef S4INDEX_OFF
      INDEX4 *index ;
      TAG4   *tag ;

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4TAG_SELECTED ) ;
      #endif

      index = (INDEX4 *)d4->indexes.selected ;
      if ( index )
      {
         tag = (TAG4 *)index->tags.selected ;
         if ( tag )
            return tag ;
      }
   #endif
   return 0 ;
}

#ifdef S4VB_DOS

TAG4 *d4tag_v( DATA4 *d4, char *name )
{
   return d4tag( d4, c4str(name) ) ;
}

#endif
