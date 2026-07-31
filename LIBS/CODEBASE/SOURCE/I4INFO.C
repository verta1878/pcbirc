/* i4info.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

char *S4FUNCTION t4alias( TAG4 *tag )
{
   return tag->alias ;
}

int S4FUNCTION t4is_descending( TAG4 *tag )
{
   #ifdef S4NDX
      return 0 ;
   #endif
   #ifdef S4FOX
      if ( tag->header.descending )
         return r4descending ;
      else
         return 0 ;
   #endif
   #ifdef S4CLIPPER
      if ( tag->header.descending )
         return r4descending ;
      else
         return 0 ;
   #endif
   #ifdef S4MDX
      if ( tag->header.type_code & 8 )
         return r4descending ;
      else
         return 0 ;
   #endif
}

int S4FUNCTION t4unique( TAG4 *tag )
{
   #ifdef S4FOX
      if ( tag->header.type_code & 0x01 )
         return tag->unique_error ;
   #else
      if ( tag->header.unique )
         return tag->unique_error ;
   #endif
   return 0 ;
}

TAG4INFO *S4FUNCTION i4tag_info( INDEX4 *index )
{
   TAG4INFO *tag_info ;
   TAG4 *tag_on ;
   int num_tags, i ;

   #ifdef S4DEBUG
      if ( index == 0 )
         e4severe( e4parm, E4_I4TAG_INFO ) ;
   #endif

   if ( index->code_base->error_code < 0 )
      return 0 ;

   num_tags = 0 ;
   for( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &index->tags, tag_on ) ;
      if ( tag_on == 0 )
         break ;
      num_tags++ ;
   }

   if ( index->code_base->error_code < 0 )
      return 0 ;
   tag_info = (TAG4INFO *)u4alloc_free( index->code_base, ( (long)num_tags + 1L ) * sizeof( TAG4INFO ) ) ;
   if ( tag_info == 0 )
      return 0 ;

   for( tag_on = 0, i = 0 ;; i++ )
   {
      tag_on = (TAG4 *)l4next( &index->tags, tag_on ) ;
      if ( tag_on == 0 )
         return ( tag_info ) ;
      tag_info[i].name = t4alias( tag_on ) ;
      tag_info[i].expression = expr4source( tag_on->expr ) ;
      tag_info[i].filter = expr4source( tag_on->filter ) ;
      tag_info[i].unique = t4unique( tag_on ) ;
      tag_info[i].descending = t4is_descending( tag_on ) ;
   }
}

#endif
