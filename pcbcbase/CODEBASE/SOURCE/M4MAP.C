/* m4map.c   (c)Copyright Sequiter Software Inc., 1993.  All rights reserved. */

#include "d4all.h"

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

static void bitmaps4free( BITMAP4 * ) ;

/* create a single bitmap structure */
BITMAP4 * S4FUNCTION bitmap4create( L4LOGICAL *log, RELATE4 *relate, char and_or, char branch )
{
   BITMAP4 *map ;

   map = (BITMAP4 *)mem4alloc( log->code_base->bitmap_memory ) ;
   if ( map == 0 )  /* must handle by freeing... */
      return 0 ;
   memset( (void *)map, 0, sizeof( BITMAP4 ) ) ;

   map->log = log ;
   map->relate = relate ;
   map->and_or = and_or ;
   map->branch = branch ;
   return map ;
}

/* free up a single bitmap structure */
void S4FUNCTION bitmap4destroy( BITMAP4 *map )
{
   CONST4 *c_on, *c_next ;

   #ifdef S4DEBUG
      if ( map == 0 )
         e4severe( e4parm, E4_BM4DESTROY ) ;
   #endif

   c_on = (CONST4 *)l4first( &map->ne ) ;
   while( c_on != 0 )
   {
      c_next = (CONST4 *)l4next( &map->ne, c_on ) ;
      const4delete_ne( &map->ne, c_on ) ;
      c_on = c_next ;
   }
   mem4free( map->log->code_base->bitmap_memory, map ) ;
}

/* can a query or subquery be bitmap optimized?  Also builds the bitmap representation */
static BITMAP4 *bitmap4can( L4LOGICAL *log, int *pos, RELATE4 *relate )
{
   E4INFO *info_ptr, *info_two ;
   int i, tag_pos, tag_pos2, const_pos ;
   BITMAP4 *map, *child_map ;
   CONST4 *temp, hold ;
   char c_temp ;

   info_ptr = log->expr->info + *pos ;

   if ( info_ptr->function_i == E4AND || info_ptr->function_i == E4OR )
   {
      (*pos)-- ;
      if ( info_ptr->function_i == E4AND && relate == 0 )
         relate = (RELATE4 *)log->info_report[*pos].data_list->pointers[0] ;

      if ( info_ptr->function_i == E4AND )
         c_temp = 1 ;
      else
         c_temp = 2 ;

      map = bitmap4create( log, relate, c_temp, 1 ) ;
      if ( map == 0 )
         return 0 ;

      for( i = 0 ; i < info_ptr->num_parms ; i++ )
      {
         child_map = bitmap4can( log, pos, relate ) ;
         if ( child_map == 0 && log->code_base->error_code == e4memory )
         {
            log->code_base->error_code = 0 ;
            bitmaps4free( map ) ;
            return 0 ;
         }

         if ( child_map != 0 )
         {
            l4add( &map->children, child_map ) ;
            if ( child_map->and_or == 0 )
               child_map->and_or = map->and_or ;
         }
         else
            if ( c_temp == 2 )   /* if an or case, then cannot create if any sub-expression is not bitmap optimizeable */
               for ( ;; )
               {
                  child_map = (BITMAP4 *)l4first( &map->children ) ;
                  if ( child_map == 0 )
                  {
                     bitmap4destroy( map ) ;
                     return 0 ;
                  }
                  l4remove( &map->children, child_map ) ;
                  bitmap4destroy( child_map ) ;
               }
      }
      if ( map->children.n_link == 0 )   /* not bitmap optimizeable */
      {
         bitmap4destroy( map ) ;
         return 0 ;
      }
   }
   else
   {
      if ( info_ptr->function_i >= E4COMPARE_START && info_ptr->function_i <= E4COMPARE_END )
      {
         /* One must be a constant and the other a tag. */
         info_ptr-- ;
         tag_pos = *pos - 1 ;
         tag_pos2 = tag_pos - info_ptr->num_entries ;
         info_two = info_ptr - info_ptr->num_entries ;
         (*pos) -= 1 + info_ptr->num_entries + info_two->num_entries ;

         if ( e4is_constant( info_ptr ) )
         {
            if ( !e4is_tag( log->info_report + tag_pos2, log->expr, info_two, relate->data ) )
               return 0 ;
            const_pos = tag_pos ;
            tag_pos = tag_pos2 ;
         }
         else
         {
            if ( e4is_constant( info_two ) == 0 || !e4is_tag( log->info_report + tag_pos, log->expr, info_ptr, relate->data ) )
               return 0 ;
            const_pos = tag_pos2 ;
            info_ptr = info_two ;
         }

         map = bitmap4create( log, relate, 0, 0 ) ;
         if ( map == 0 )
            return 0 ;
         map->tag = ( log->info_report + tag_pos )->tag ;

         info_ptr++ ;

         memset( (void *)&hold, 0, sizeof( CONST4 ) ) ;
         if ( const4get( &hold, map, log, const_pos ) < 0 )
         {
            bitmap4destroy( map ) ;
            return 0 ;
         }

         if ( info_ptr->function_i >= E4COMPARE_START && info_ptr->function_i <= E4COMPARE_END )
         {
            if ( info_ptr->function_i >= E4NOT_EQUAL && info_ptr->function_i < E4GREATER_EQ ) /* != */
            {
               temp = (CONST4 *)u4alloc( sizeof( CONST4 ) ) ;
               if ( temp == 0 )
               {
                  log->code_base->error_code = 0 ;
                  bitmaps4free( map ) ;
                  return 0 ;
               }
               memcpy( (void *)temp, (void *)&hold, sizeof( CONST4 ) ) ;
               l4add( &map->ne, temp ) ;
            }
            if (info_ptr->function_i >= E4EQUAL && info_ptr->function_i < E4NOT_EQUAL) /* == */
               memcpy( (void *)&map->eq, (void *)&hold, sizeof( CONST4 ) ) ;
            if ( info_ptr->function_i >= E4GREATER && info_ptr->function_i < E4LESS ) /* > */
            {
               if ( map->type == r4str && hold.len < map->tag->header.key_len )   /* same as >= since a partial > */
                  memcpy( (void *)&map->ge, (void *)&hold, sizeof( CONST4 ) ) ;
               else
                  memcpy( (void *)&map->gt, (void *)&hold, sizeof( CONST4 ) ) ;
            }
            if ( info_ptr->function_i >= E4GREATER_EQ && info_ptr->function_i < E4LESS_EQ ) /* >= */
               memcpy( (void *)&map->ge, (void *)&hold, sizeof( CONST4 ) ) ;
            if ( info_ptr->function_i >= E4LESS && info_ptr->function_i < E4DEL )  /* < */
               memcpy( (void *)&map->lt, (void *)&hold, sizeof( CONST4 ) ) ;
            if ( info_ptr->function_i >= E4LESS_EQ && info_ptr->function_i < E4GREATER ) /* <= */
               memcpy( (void *)&map->le, (void *)&hold, sizeof( CONST4 ) ) ;
         }
      }
      else
         return 0 ;
   }

   return map ;
}

/* free the bitmap tree */
static void bitmaps4free( BITMAP4 *map )
{
   BITMAP4 *map_on, *map_next ;

   if ( map->branch == 1 )
   {
      map_on = (BITMAP4 *)l4first( &map->children ) ;
      while( map_on != 0 )
      {
         map_next = (BITMAP4 *)l4next( &map->children, map_on ) ;
         l4remove( &map->children, map_on ) ;
         bitmaps4free( map_on ) ;
         map_on = map_next ;
      }
   }

   bitmap4destroy( map ) ;
}

/* free all the bitmap info */
static void bitmap4free( L4LOGICAL *log, BITMAP4 *map )
{
   bitmaps4free( map ) ;
   u4free( log->buf ) ;
   log->buf = 0 ;
   log->buf_len = 0 ;
   log->buf_pos = 0 ;
}

/* initialize the bitmap structures, determine if bitmapping is possible */
static BITMAP4 *bitmap4init( L4LOGICAL *log, int pos )
{
   E4INFO *info_ptr ;
   int pass_pos ;
   BITMAP4 *map ;

   info_ptr = log->expr->info + pos ;

   /* for testing purposes only, allow bitmap disabling */
   if ( log->code_base->bitmap_disable == 1 || d4reccount( log->relation->relate.data ) >= ( (long)UINT_MAX - 2L) * 8L )
   {
      log->relation->bitmaps_freed = 1 ;
      return 0 ;
   }

   if ( log->code_base->bitmap_memory == 0 )
   {
      log->code_base->bitmap_memory = mem4create( log->code_base, 10, sizeof( BITMAP4 ), 5, 0 ) ;
      if ( log->code_base->bitmap_memory == 0 )  /* no memory available for bitmap optimization */
         return 0 ;
   }

   pass_pos = pos ;
   if ( info_ptr->function_i == E4AND )
      map = bitmap4can( log, &pass_pos, &log->relation->relate ) ;
   else
      map = bitmap4can( log, &pass_pos, &log->relation->relate ) ;

   if ( map == 0 && log->code_base->error_code == e4memory )
      log->code_base->error_code = 0 ;

   return map ;
}

/* this function removes all bitmaps from the parent and marks the parent as zero */
static BITMAP4 *bitmap4collapse( BITMAP4 *parent )
{
   BITMAP4 *child_on, *child_next ;
   child_on = (BITMAP4 *)l4first( &parent->children ) ;
   if ( parent->tag == 0 && child_on->tag != 0 )
      parent->tag = child_on->tag ;
   while( child_on != 0 )
   {
      #ifdef S4DEBUG
         if ( child_on->tag == 0 && child_on->children.n_link == 0 )
            e4severe( e4info, E4_BM4COLLAPSE ) ;
      #endif
      child_next = (BITMAP4 *)l4next( &parent->children, child_on ) ;
      l4remove( &parent->children, child_on ) ;
      bitmap4destroy( child_on ) ;
      child_on = child_next ;
   }
   parent->no_match = 1 ;
   return 0 ;
}


/* 0 = successful merger, 1 means collaps because now no records belong to the set */
static int bitmap4combine_and_le( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_next ;
   char eq_found ;

   if ( map1->eq.len )
   {
      if ( const4less( &map2->le, &map1->eq, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->gt.len )
      if ( const4less_eq( &map2->le, &map1->gt, map1 ) )
         return 1 ;

   if ( map1->ge.len )
   {
      if ( const4less( &map2->le, &map1->ge, map1 ) )
         return 1 ;
      if ( const4eq( &map2->le, &map1->ge, map1 ) )
      {
         if ( map2->eq.len )
            if ( !const4eq( &map2->eq, &map2->le, map1 ) )
               return 1 ;
         memcpy( (void *)&map2->eq, (void *)&map2->le, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      eq_found = 0 ;
      while ( c_on != 0 )
      {
         c_next = (CONST4 *)l4next( &map1->ne, c_on ) ;
         if ( const4less( &map2->le, c_on, map1 ) )
            const4delete_ne( &map1->ne, c_on ) ;
         else
            if ( eq_found == 0 )
               if ( const4eq( &map2->le, c_on, map1 ) )
                  eq_found = 1 ;
         c_on = c_next ;
      }
      if ( eq_found )
      {
         memcpy( (void *)&map2->lt, (void *)&map2->le, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->lt.len )
   {
      if ( const4less( &map2->le, &map1->lt, map1 ) )
      {
         memcpy( (void *)&map1->le, (void *)&map2->le, sizeof( CONST4 ) ) ;
         memset( (void *)&map1->lt, 0, sizeof( CONST4 ) ) ;
      }
   }
   else
   {
      if ( map1->le.len )
      {
         if ( const4less( &map2->le, &map1->le, map1 ) )
            memcpy( (void *)&map1->le, (void *)&map2->le, sizeof( CONST4 ) ) ;
      }
      else
         memcpy( (void *)&map1->le, (void *)&map2->le, sizeof( CONST4 ) ) ;
   }

   memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

/* 0 = successful merger, 1 means collaps because now no records belong to the set */
static int bitmap4combine_and_ge( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_next ;
   char eq_found ;

   if ( map1->eq.len )
   {
      if ( const4less( &map1->eq, &map2->ge, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->lt.len )
   {
      if ( const4less_eq( &map1->lt, &map2->ge, map1 ) )
         return 1 ;
   }
   else
      if ( map1->le.len )
      {
         if ( const4less( &map1->le, &map2->ge, map1 ) )
            return 1 ;
         if ( const4eq( &map1->le, &map2->ge, map1 ) )
         {
            if ( map2->eq.len )
               if ( !const4eq( &map2->eq, &map2->ge, map1 ) )
                  return 1 ;
            memcpy( (void *)&map2->eq, (void *)&map2->ge, sizeof( CONST4 ) ) ;
            memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }
      }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      eq_found = 0 ;
      while ( c_on != 0 )
      {
         c_next = (CONST4 *)l4next( &map1->ne, c_on ) ;
         if ( const4less( c_on, &map2->ge, map1 ) )
            const4delete_ne( &map1->ne, c_on ) ;
         else
            if ( eq_found == 0 )
               if ( const4eq( c_on, &map2->ge, map1 ) )
                  eq_found = 1 ;
         c_on = c_next ;
      }
      if ( eq_found )
      {
         memcpy( (void *)&map2->gt, (void *)&map2->ge, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->gt.len )
   {
      if ( const4less( &map1->gt, &map2->ge, map1 ) )
      {
         memcpy( (void *)&map1->ge, (void *)&map2->ge, sizeof( CONST4 ) ) ;
         memset( (void *)&map1->gt, 0, sizeof( CONST4 ) ) ;
      }
   }
   else
   {
      if ( map1->ge.len )
      {
         if ( const4less( &map1->ge, &map2->ge, map1 ) )
            memcpy( (void *)&map1->ge, (void *)&map2->ge, sizeof( CONST4 ) ) ;
      }
      else
         memcpy( (void *)&map1->ge, (void *)&map2->ge, sizeof( CONST4 ) ) ;
   }
   memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

/* 0 = successful merger, 1 means collaps because now no records belong to the set */
static int bitmap4combine_and_lt( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_next ;

   if ( map1->eq.len )
   {
      if ( const4less_eq( &map2->lt, &map1->eq, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      while ( c_on != 0 )
      {
         c_next = (CONST4 *)l4next( &map1->ne, c_on ) ;
         if ( const4less_eq( &map2->lt, c_on, map1 ) )
            const4delete_ne( &map1->ne, c_on ) ;
         c_on = c_next ;
      }
   }

   if ( map1->gt.len )
   {
      if ( const4less_eq( &map2->lt, &map1->gt, map1 ) )
         return 1 ;
   }
   else
      if ( map1->ge.len )
         if ( const4less_eq( &map2->lt, &map1->ge, map1 ) )
            return 1 ;

   if ( map1->lt.len )
   {
      if ( const4less_eq( &map2->lt, &map1->lt, map1 ) )
         memcpy( (void *)&map1->lt, (void *)&map2->lt, sizeof( CONST4 ) ) ;
   }
   else
   {  
      if ( map1->le.len )
      {
         if ( const4less_eq( &map2->lt, &map1->le, map1 ) )
         {
            memcpy( (void *)&map1->lt, (void *)&map2->lt, sizeof( CONST4 ) ) ;
            memset( (void *)&map1->le, 0, sizeof( CONST4 ) ) ;
         }
      }
      else
         memcpy( (void *)&map1->lt, (void *)&map2->lt, sizeof( CONST4 ) ) ;
   }
   memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

/* 0 = successful merger, 1 means collaps because now no records belong to the set */
static int bitmap4combine_and_gt( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_next ;

   if ( map1->eq.len )
   {
      if ( const4less_eq( &map1->eq, &map2->gt, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      while ( c_on != 0 )
      {
         c_next = (CONST4 *)l4next( &map1->ne, c_on ) ;
         if ( const4less_eq( c_on, &map2->gt, map1 ) )
            const4delete_ne( &map1->ne, c_on ) ;
         c_on = c_next ;
      }
   }

   if ( map1->lt.len )
   {
      if ( const4less_eq( &map1->lt, &map2->gt, map1 ) )
         return 1 ;
   }
   else
      if ( map1->le.len )
         if ( const4less_eq( &map1->le, &map2->gt, map1 ) )
            return 1 ;

   if ( map1->gt.len )
   {
      if ( const4less( &map1->gt, &map2->gt, map1 ) )
         memcpy( (void *)&map1->gt, (void *)&map2->gt, sizeof( CONST4 ) ) ;
   }
   else
   {  
      if ( map1->ge.len )
      {
         if ( const4less_eq( &map1->ge, &map2->gt, map1 ) )
         {
            memcpy( (void *)&map1->gt, (void *)&map2->gt, sizeof( CONST4 ) ) ;
            memset( (void *)&map1->ge, 0, sizeof( CONST4 ) ) ;
         }
      }
      else
         memcpy( (void *)&map1->gt, (void *)&map2->gt, sizeof( CONST4 ) ) ;
   }
   memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

/* 0 = successful merger, 1 means collaps because now no records belong to the set */
static int bitmap4combine_and_ne( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_next, *c_on2, *c_next2 ;
   int eq_found ;

   c_on = (CONST4 *)l4first( &map2->ne ) ;
   while ( c_on != 0 )
   {
      c_next = (CONST4 *)l4next( &map2->ne, c_on ) ;

      if ( map1->eq.len )
      {
         if ( const4eq( c_on, &map1->eq, map1 ) )
            return 1 ;
         else
         {
            const4delete_ne( &map2->ne, c_on ) ;
            c_on = c_next ;
            continue ;
         }
      }

      if ( map1->gt.len )
      {
         if ( const4less_eq( c_on, &map1->gt, map1 ) )
         {
            const4delete_ne( &map2->ne, c_on ) ;
            c_on = c_next ;
            continue ;
         }
      }
      else
         if ( map1->ge.len )
         {
            if ( const4eq( c_on, &map1->ge, map1 ) )
            {
               memcpy( (void *)&map1->gt, (void *)&map1->ge, sizeof( CONST4 ) ) ;
               memset( (void *)&map1->ge, 0, sizeof( CONST4 ) ) ;
               const4delete_ne( &map2->ne, c_on ) ;
               c_on = c_next ;
               continue ;
            }
            if ( const4less_eq( c_on, &map1->ge, map1 ) )
            {
               const4delete_ne( &map2->ne, c_on ) ;
               c_on = c_next ;
               continue ;
            }
         }

      if ( map1->lt.len )
      {
         if ( const4less_eq( &map1->lt, c_on, map1 ) )
         {
            const4delete_ne( &map2->ne, c_on ) ;
            c_on = c_next ;
            continue ;
         }
      }
      else
         if ( map1->le.len )
         {
            if ( const4eq( c_on, &map1->le, map1 ) )
            {
               memcpy( (void *)&map1->lt, (void *)&map1->le, sizeof( CONST4 ) ) ;
               memset( (void *)&map1->le, 0, sizeof( CONST4 ) ) ;
               const4delete_ne( &map2->ne, c_on ) ;
               c_on = c_next ;
               continue ;
            }
            if ( const4less_eq( &map1->le, c_on, map1 ) )
            {
               const4delete_ne( &map2->ne, c_on ) ;
               c_on = c_next ;
               continue ;
            }
         }

      if ( map1->ne.n_link != 0 )  /* special case */
      {
         c_on2 = (CONST4 *)l4first( &map1->ne ) ;
         eq_found = 0 ;
         while ( c_on2 != 0 )
         {
            c_next2 = (CONST4 *)l4next( &map1->ne, c_on2 ) ;
            if ( const4eq( c_on, c_on2, map1 ) )
            {
               const4delete_ne( &map2->ne, c_on ) ;
               c_on = c_next ;
               eq_found = 1 ;
               break ;
            }
            c_on2 = c_next2 ;
         }
         if ( eq_found != 1 )
         {
            l4remove( &map2->ne, c_on ) ;
            l4add( &map1->ne, c_on ) ;
         }
         c_on = c_next ;
         continue ;
      }

      /* must add the ne */
      l4remove( &map2->ne, c_on ) ;
      l4add( &map1->ne, c_on ) ;
      c_on = c_next ;
   }

   return 0 ;
}

/* 0 = successful merger, 1 means collaps because now no records belong to the set */
static int bitmap4combine_and_eq( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_next ;

   if ( map1->eq.len )
   {
      if ( const4eq( &map2->eq, &map1->eq, map1 ) )
      {
         memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
      else
         return 1 ;
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      while ( c_on != 0 )
      {
         c_next = (CONST4 *)l4next( &map1->ne, c_on ) ;
         if ( const4eq( &map2->eq, c_on, map1 ) )
            return 1 ;
         else
            const4delete_ne( &map1->ne, c_on ) ;
         c_on = c_next ;
      }
   }

   if ( map1->le.len )
   {
      if ( const4less( &map1->le, &map2->eq, map1 ) )
         return 1 ;
      memset( (void *)&map1->le, 0, sizeof( CONST4 ) ) ;
   }
   else
      if ( map1->lt.len )
      {
         if ( const4less_eq( &map1->lt, &map2->eq, map1 ) )
            return 1 ;
         memset( (void *)&map1->lt, 0, sizeof( CONST4 ) ) ;
      }

   if ( map1->ge.len )
   {
      if ( const4less( &map2->eq, &map1->ge, map1 ) )
         return 1 ;
      memset( (void *)&map1->ge, 0, sizeof( CONST4 ) ) ;
   }
   else
      if ( map1->gt.len )
      {
         if ( const4less_eq( &map2->eq, &map1->gt, map1 ) )
            return 1 ;
         memset( (void *)&map1->gt, 0, sizeof( CONST4 ) ) ;
      }

   memcpy( (void *)&map1->eq, (void *)&map2->eq, sizeof( CONST4 ) ) ;
   memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

static int bitmap4combine_or_le( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on ;

   if ( map1->eq.len )
      if ( const4less_eq( &map1->eq, &map2->le, map1 ) )
         memset( (void *)&map1->eq, 0, sizeof( CONST4 ) ) ;

   if ( map1->gt.len )
   {
      if ( const4less_eq( &map1->gt, &map2->le, map1 ) )
         return 1 ;
   }
   else
      if ( map1->ge.len )
         if ( const4less_eq( &map1->ge, &map2->le, map1 ) )
            return 1 ;

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      #ifdef S4DEBUG
         if ( map1->ne.n_link != 1 )  /* if 2 links, all must belong... */
            e4severe( e4info, E4_BM4COMBINE ) ;
      #endif
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      if ( const4less_eq( c_on, &map2->le, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->lt.len )
   {
      if ( const4less_eq( &map1->lt, &map2->le, map1 ) )
         memset( (void *)&map1->lt, 0, sizeof( CONST4 ) ) ;
      else
      {
         memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->le.len )
         if ( !const4less_eq( &map1->le, &map2->le, map1 ) )
         {
            memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }

   memcpy( (void *)&map1->le, (void *)&map2->le, sizeof( CONST4 ) ) ;
   memset( (void *)&map2->le, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

static int bitmap4combine_or_ge( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on ;

   if ( map1->eq.len )
      if ( const4less_eq( &map2->ge, &map1->eq, map1 ) )
         memset( (void *)&map1->eq, 0, sizeof( CONST4 ) ) ;

   if ( map1->lt.len )
   {
      if ( const4less_eq( &map2->ge, &map1->lt, map1 ) )
         return 1 ;
   }
   else
      if ( map1->le.len )
         if ( const4less_eq( &map2->ge, &map1->le, map1 ) )
            return 1 ;

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      #ifdef S4DEBUG
         if ( map1->ne.n_link != 1 )  /* if 2 links, all must belong... */
            e4severe( e4info, E4_BM4COMBINE ) ;
      #endif
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      if ( const4less_eq( &map2->ge, c_on, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->gt.len )
   {
      if ( const4less_eq( &map2->ge, &map1->gt, map1 ) )
         memset( (void *)&map1->gt, 0, sizeof( CONST4 ) ) ;
      else
      {
         memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->ge.len )
         if ( !const4less_eq( &map2->ge, &map1->ge, map1 ) )
         {
            memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }

   memcpy( (void *)&map1->ge, (void *)&map2->ge, sizeof( CONST4 ) ) ;
   memset( (void *)&map2->ge, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

static int bitmap4combine_or_lt( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on ;

   if ( map1->eq.len )
   {
      if ( const4eq( &map1->eq, &map2->lt, map1 ) )
      {
         #ifdef S4DEBUG
            if ( map2->le.len != 0 )
               e4severe( e4info, E4_BM4COMBINE ) ;
         #endif
         memcpy( (void *)&map2->le, (void *)&map2->lt, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
         memset( (void *)&map1->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
      if ( const4less( &map1->eq, &map2->lt, map1 ) )
         memset( (void *)&map1->eq, 0 , sizeof( CONST4 ) ) ;
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      #ifdef S4DEBUG
         if ( map1->ne.n_link != 1 )  /* if 2 links, all must belong... */
            e4severe( e4info, E4_BM4COMBINE ) ;
      #endif
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      if ( const4less( c_on, &map2->lt, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->gt.len )
   {
      if ( const4less( &map1->gt, &map2->lt, map1 ) )
         return 1 ;
      if ( const4eq( &map1->gt, &map2->lt, map1 ) )
      {
         const4add_ne( map1, &map2->lt ) ;
         memset( (void *)&map1->gt, 0, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->ge.len )
         if ( const4less_eq( &map1->ge, &map2->lt, map1 ) )
            return 1 ;

   if ( map1->lt.len )
   {
      if ( !const4less( &map1->lt, &map2->lt, map1 ) )
      {
         memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->le.len )
      {
         if ( const4less_eq( &map2->lt, &map1->le, map1 ) )
         {
            memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }
         else
            memset( (void *)&map1->le, 0, sizeof( CONST4 ) ) ;
      }

   memcpy( (void *)&map1->lt, (void *)&map2->lt, sizeof( CONST4 ) ) ;
   memset( (void *)&map2->lt, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

static int bitmap4combine_or_gt( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on ;

   if ( map1->eq.len )
   {
      if ( const4eq( &map1->eq, &map2->gt, map1 ) )
      {
         #ifdef S4DEBUG
            if ( map2->ge.len != 0 )
               e4severe( e4info, E4_BM4COMBINE ) ;
         #endif
         memcpy( (void *)&map2->ge, (void *)&map2->gt, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
         memset( (void *)&map1->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
      if ( const4less( &map2->gt, &map1->eq, map1 ) )
         memset( (void *)&map1->eq, 0 , sizeof( CONST4 ) ) ;
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      #ifdef S4DEBUG
         if ( map1->ne.n_link != 1 )  /* if 2 links, all must belong... */
            e4severe( e4info, E4_BM4COMBINE ) ;
      #endif
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      if ( const4less( &map2->gt, c_on, map1 ) )
         return 1 ;
      else
      {
         memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->lt.len )
   {
      if ( const4less( &map2->gt, &map1->lt, map1 ) )
         return 1 ;
      if ( const4eq( &map2->gt, &map1->lt, map1 ) )
      {
         const4add_ne( map1, &map2->gt ) ;
         memset( (void *)&map1->lt, 0, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->le.len )
         if ( const4less_eq( &map2->gt, &map1->le, map1 ) )
            return 1 ;

   if ( map1->gt.len )
   {
      if ( const4less( &map1->gt, &map2->gt, map1 ) )
      {
         memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->ge.len )
      {
         if ( const4less_eq( &map1->ge, &map2->gt, map1 ) )
         {
            memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }
         else
            memset( (void *)&map1->ge, 0, sizeof( CONST4 ) ) ;
      }

   memcpy( (void *)&map1->gt, (void *)&map2->gt, sizeof( CONST4 ) ) ;
   memset( (void *)&map2->gt, 0, sizeof( CONST4 ) ) ;
   return 0 ;
}

static int bitmap4combine_or_eq( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on ;

   if ( map1->eq.len )
   {
      if ( const4eq( &map1->eq, &map2->eq, map1 ) )
      {
         memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      #ifdef S4DEBUG
         if ( map1->ne.n_link != 1 )  /* if 2 links, all must belong... */
            e4severe( e4info, E4_BM4COMBINE ) ;
      #endif
      c_on = (CONST4 *)l4first( &map1->ne ) ;
      if ( const4eq( &map2->eq, c_on, map1 ) )
         return 1 ;
      memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
      return 0 ;
   }

   if ( map1->lt.len )
   {
      if ( const4eq( &map1->lt, &map2->eq, map1 ) )
      {
         memcpy( (void *)&map1->le, (void *)&map1->lt, sizeof( CONST4 ) ) ;
         memset( (void *)&map1->lt, 0, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
      if ( const4less( &map2->eq, &map1->lt, map1 ) )
      {
         memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->le.len )
         if ( const4less_eq( &map2->eq, &map1->le, map1 ) )
         {
            memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }

   if ( map1->gt.len )
   {
      if ( const4eq( &map1->gt, &map2->eq, map1 ) )
      {
         memcpy( (void *)&map1->ge, (void *)&map1->gt, sizeof( CONST4 ) ) ;
         memset( (void *)&map1->gt, 0, sizeof( CONST4 ) ) ;
         memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
      if ( const4less( &map1->gt, &map2->eq, map1 ) )
      {
         memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
         return 0 ;
      }
   }
   else
      if ( map1->ge.len )
         if ( const4less_eq( &map1->ge, &map2->eq, map1 ) )
         {
            memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
            return 0 ;
         }

   if ( map1->eq.len == 0 )
   {
      memcpy( (void *)&map1->eq, (void *)&map2->eq, sizeof( CONST4 ) ) ;
      memset( (void *)&map2->eq, 0, sizeof( CONST4 ) ) ;
   }
   return 0 ;
}

static int bitmap4combine_or_ne( BITMAP4 *map1, BITMAP4 *map2 )
{
   CONST4 *c_on, *c_on2 ;

   #ifdef S4DEBUG
      if ( map2->ne.n_link != 1 )  /* if 2 links, all must belong... */
         e4severe( e4info, E4_BM4COMBINE ) ;
   #endif

   c_on = (CONST4 *)l4first( &map2->ne ) ;

   if ( map1->eq.len )
   {
      if ( const4eq( &map1->eq, c_on, map1 ) )
         return 1 ;
      memset( (void *)&map1->eq, 0, sizeof( CONST4 ) ) ;
   }

   if ( map1->ne.n_link != 0 )  /* special case */
   {
      #ifdef S4DEBUG
         if ( map1->ne.n_link != 1 )  /* if 2 links, all must belong... */
            e4severe( e4info, E4_BM4COMBINE ) ;
      #endif
      c_on2 = (CONST4 *)l4first( &map1->ne ) ;
      if ( const4eq( c_on2, c_on, map1 ) )
      {
         const4delete_ne( &map2->ne, c_on ) ;
         return 0 ;
      }
      else
         return 1 ;
   }

   if ( map1->lt.len )
   {
      if ( const4less( c_on, &map1->lt, map1 ) )
         return 1 ;
      else
         memset( (void *)&map1->lt, 0, sizeof( CONST4 ) ) ;
   }
   else
      if ( map1->le.len )
      {
         if ( const4less_eq( c_on, &map1->le, map1 ) )
            return 1 ;
         else
            memset( (void *)&map1->le, 0, sizeof( CONST4 ) ) ;
      }

   if ( map1->gt.len )
   {
      if ( const4less( &map1->gt, c_on, map1 ) )
         return 1 ;
      else
         memset( (void *)&map1->gt, 0, sizeof( CONST4 ) ) ;
   }
   else
      if ( map1->ge.len )
      {
         if ( const4less_eq( &map1->ge, c_on, map1 ) )
            return 1 ;
         else
            memset( (void *)&map1->ge, 0, sizeof( CONST4 ) ) ;
      }

   l4remove( &map2->ne, c_on ) ;
   l4add( &map1->ne, c_on ) ;
   return 0 ;
}

/* merge two leaf maps together */
BITMAP4 * S4FUNCTION bitmap4combine_leafs( BITMAP4 *parent, BITMAP4 *map1, BITMAP4 *map2 )
{
   BITMAP4 *child_on ;
   #ifdef S4DEBUG
      if ( parent == 0 || map1 == 0 || map2 == 0 )
         e4severe( e4parm, 0 ) ;
      if ( map1->type != map2->type )
         e4severe( e4info, E4_BM4_IM ) ;
   #endif

   if ( parent->and_or == 1 )  /* and */
   {
      if ( map2->le.len )
         if ( bitmap4combine_and_le( map1, map2 ) == 1 )
            return bitmap4collapse( parent ) ;
            
      if ( map2->ge.len )
         if ( bitmap4combine_and_ge( map1, map2 ) == 1 )
            return bitmap4collapse( parent ) ;

      if ( map2->lt.len )
         if ( bitmap4combine_and_lt( map1, map2 ) == 1 )
            return bitmap4collapse( parent ) ;

      if ( map2->gt.len )
         if ( bitmap4combine_and_gt( map1, map2 ) == 1 )
            return bitmap4collapse( parent ) ;

      if ( map2->ne.n_link != 0 )  /* special case */
         if ( bitmap4combine_and_ne( map1, map2 ) == 1 )
            return bitmap4collapse( parent ) ;

      if ( map2->eq.len )
         if ( bitmap4combine_and_eq( map1, map2 ) == 1 )
            return bitmap4collapse( parent ) ;

      #ifdef S4DEBUG
         if ( map2->le.len || map2->ge.len || map2->lt.len || map2->gt.len || map2->eq.len || map2->ne.n_link )
            e4severe( e4info, E4_BM4COMBINE_LF ) ;
         if ( map1->eq.len )
            if ( map1->le.len || map1->ge.len || map1->lt.len || map1->gt.len || map1->ne.n_link )
               e4severe( e4info, E4_BM4COMBINE_LF ) ;
         if ( map1->ne.n_link )
            if ( map1->eq.len )
               e4severe( e4info, E4_BM4COMBINE_LF ) ;
         if ( map1->ge.len )
            if ( map1->gt.len || map1->eq.len )
               e4severe( e4info, E4_BM4COMBINE_LF ) ;
         if ( map1->gt.len )
            if ( map1->ge.len || map1->eq.len )
               e4severe( e4info, E4_BM4COMBINE_LF ) ;
         if ( map1->le.len )
            if ( map1->lt.len || map1->eq.len )
               e4severe( e4info, E4_BM4COMBINE_LF ) ;
         if ( map1->lt.len )
            if ( map1->le.len || map1->eq.len )
               e4severe( e4info, E4_BM4COMBINE_LF ) ;
      #endif

      l4remove( &parent->children, map2 ) ;
      bitmap4destroy( map2 ) ;
      return map1 ;
   }
   else if ( parent->and_or == 2 )  /* or */
   {
      child_on = map2 ;
      while( child_on != 0 )
      {
         if ( map2->lt.len )
            if ( bitmap4combine_or_lt( map1, map2 ) == 1 )
               return bitmap4collapse( parent ) ;

         if ( map2->gt.len )
            if ( bitmap4combine_or_gt( map1, map2 ) == 1 )
               return bitmap4collapse( parent ) ;

         if ( map2->le.len )
            if ( bitmap4combine_or_le( map1, map2 ) == 1 )
               return bitmap4collapse( parent ) ;
               
         if ( map2->ge.len )
            if ( bitmap4combine_or_ge( map1, map2 ) == 1 )
               return bitmap4collapse( parent ) ;

         if ( map2->ne.n_link != 0 )  /* special case */
            if ( bitmap4combine_or_ne( map1, map2 ) == 1 )
               return bitmap4collapse( parent ) ;

         if ( map2->eq.len )
            if ( bitmap4combine_or_eq( map1, map2 ) == 1 )
               return bitmap4collapse( parent ) ;

         child_on = (BITMAP4 *)l4next( &parent->children, child_on ) ;
      }

      if ( map2->le.len || map2->ge.len || map2->lt.len || map2->gt.len || map2->eq.len || map2->ne.n_link )
         return map2 ;
      else
      {
         l4remove( &parent->children, map2 ) ;
         bitmap4destroy( map2 ) ;
         return map1 ;
      }
   }
   return 0 ;
}

static void bitmap4copy( BITMAP4 *to, BITMAP4 *from )
{
   CONST4 *c_on, *temp ;

   #ifdef S4DEBUG
      if ( from->branch )
         e4severe( e4info, E4_BM4COPY ) ;
   #endif
   memset( (void *)to, 0, sizeof( BITMAP4 ) ) ;
   to->and_or = from->and_or ;
   to->log = from->log ;
   to->relate = from->relate ;
   to->type = from->type ;
   to->tag = from->tag ;
   const4duplicate( &to->lt, &from->lt, from->log ) ;
   const4duplicate( &to->le, &from->le, from->log ) ;
   const4duplicate( &to->gt, &from->gt, from->log ) ;
   const4duplicate( &to->ge, &from->ge, from->log ) ;
   const4duplicate( &to->eq, &from->eq, from->log ) ;
   c_on = (CONST4 *)l4first( &from->ne ) ;
   while ( c_on != 0 )
   {
      temp = (CONST4 *)u4alloc( sizeof( CONST4 ) ) ;
      if ( temp == 0 )
         return ;
      const4duplicate( temp, c_on, from->log ) ;
      l4add( &to->ne, temp ) ;
   }
}

/* This function creates a branch out of the input constant, and combines it with the input map */
void S4FUNCTION bitmap4constant_combine( BITMAP4 *parent, BITMAP4 *old_and_map, CONST4 *con, int con_type )
{
   BITMAP4 *temp_leaf, *and_map, *new_branch ;
   CONST4 *temp ;

   if ( con->len == 0 || parent->log->code_base->error_code == e4memory )
      return ;

   new_branch = bitmap4create( parent->log, parent->relate, 1, 1 ) ;
   if ( new_branch == 0 )
      return ;

   and_map = bitmap4create( parent->log, parent->relate, 0, 0 ) ;
   if ( and_map == 0 )
      return ;
   bitmap4copy( and_map, old_and_map ) ;
   l4add( &new_branch->children, and_map ) ;

   temp_leaf = bitmap4create( parent->log, parent->relate, 1, 0 ) ;
   if ( temp_leaf == 0 )
      return ;
   temp_leaf->type = and_map->type ;
   temp_leaf->tag = and_map->tag ;
   l4add( &new_branch->children, temp_leaf ) ;

   switch( con_type )
   {
      case 1:
         memcpy( (void *)&temp_leaf->lt, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 2:
         memcpy( (void *)&temp_leaf->le, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 3:
         memcpy( (void *)&temp_leaf->gt, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 4:
         memcpy( (void *)&temp_leaf->ge, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 5:
         memcpy( (void *)&temp_leaf->eq, (void *)con, sizeof( CONST4 ) ) ;
         break ;
      case 6:
         temp = (CONST4 *)u4alloc( sizeof( CONST4 ) ) ;
         if ( temp == 0 )
            return ;
         memcpy( (void *)temp, (void *)con, sizeof( CONST4 ) ) ;
         l4add( &temp_leaf->ne, temp ) ;
         break ;
      default:
         #ifdef S4DEBUG
            e4severe( e4info, E4_BM4CON_COMBINE ) ;
         #endif
         return ;
   }
   memset( (void *)con, 0 ,sizeof( CONST4 ) ) ;
   new_branch = bitmap4redistribute( 0, new_branch, 0 ) ;

   if ( new_branch->children.n_link == 0 )
      bitmap4destroy( new_branch ) ;
   else
      l4add( &parent->children, new_branch ) ;
}

/* location = 0 if seek_before, 1 if seek 1st, 2 if seek last, 3 if seek_after,  */
/* add 10 if it is to be an approximate seek */
/* returns record number */
static long bitmap4seek( BITMAP4 *map, CONST4 *con, char location, long check, int do_check )
{
   int len, rc ;
   TAG4 *tag ;
   char *result ;
   #ifdef S4CLIPPER
      int old_dec ;
      char hold_result[20] ;  /* enough space for a numerical key */
   #endif
   #ifndef S4HAS_DESCENDING
      char did_skip ;
      int seek_rc ;
   #endif

   tag = map->tag ;
   result = (char *)const4return( map->log, con ) ;

   if ( map->type != r4str )   /* must convert to a proper key */
   {
      #ifdef S4CLIPPER
         {
            old_dec = tag->code_base->decimals ;
            tag->code_base->decimals = tag->header.key_dec ;
            memcpy( hold_result, result, con->len ) ;
            result = hold_result ;
      #endif
      #ifdef S4DEBUG
         if ( expr4len( tag->expr ) == -1 )
            e4severe( e4info, E4_BM4_IEL ) ;
      #endif
      len = expr4key_convert( tag->expr, (char **)&result, con->len, map->type ) ;
      #ifdef S4CLIPPER
            tag->code_base->decimals = old_dec ;
         }
      #endif
   }
   else
      len = con->len ;

   #ifdef S4HAS_DESCENDING
      if ( location > 1 )
         t4descending( tag, 1 ) ;
      else
         t4descending( tag, 0 ) ;
   #endif

   #ifdef S4HAS_DESCENDING
      t4seek( tag, result, len ) ;
   #else
      seek_rc = t4seek( tag, result, len ) ;
   #endif

   #ifdef S4HAS_DESCENDING
      t4descending( tag, 0 ) ;
   #endif

   if ( !t4eof( tag ) )
      if ( do_check )
	 if ( check == t4recno( tag ) )
	    return -1 ;

   switch ( location )
   {
      case 0:
         if ( t4skip( tag, -1L ) != -1L )   /* at top already */
            return -1 ;
         break ;
      case 1:
         break ;
      case 2:
         if( u4memcmp( t4key_data(tag)->value, result, len ) != 0 )  /* last one is too far, go back one for a closure */
         {
            if ( !t4eof( tag ) )
               if ( check == t4recno( tag ) )   /* case where none belong, so break now */
                  return -1 ;
            #ifndef S4HAS_DESCENDING
               if ( t4skip( tag, -1L ) != -1L )
                  return -1 ;
            #endif
         }
         #ifdef S4HAS_DESCENDING
            break ;
         #endif
      case 3:
         #ifdef S4HAS_DESCENDING
            rc = (int)t4skip( tag, 1L ) ;
            if ( rc == 0L )
               return -1 ;
         #else
            did_skip = 0 ;

            for(; u4memcmp( t4key_data(tag)->value, result, len ) == 0; )
            {
               rc = (int)t4skip( tag, 1L ) ;
               if ( rc < 0 )
                  return -1 ;
               if ( rc != 1 )
               {
                  if ( location == 2 )   /* on last record, but it still belongs, so don't skip back */
                     did_skip = 0 ;
                  if ( location == 3 )   /* on last record not far enough, so none match */
                     return -1 ;
                  break ;
               }
               did_skip = 1 ;
            }

            if ( location == 3 )
            {
               if ( did_skip == 0 && seek_rc != 2 )
                  if ( t4skip( tag, 1L ) != 1L )
                     return -1 ;
            }
            else
               if ( did_skip == 1 )
                  if ( t4skip( tag, -1L ) != -1L )
                     return -1 ;
         #endif
         break ;
      default:
         #ifdef S4DEBUG
            e4severe( e4info, E4_BM4SEEK ) ;
         #endif
         return 0 ;
   }

   return t4recno( tag ) ;
}

/* flags a range of bits for the given F4FLAG and map */
static long bitmap4flag_range( F4FLAG *flags, BITMAP4 *map, CONST4 *start_con, CONST4 *end_con, long do_flip, char start_val, char end_val, long check )
{
   long start, end ;
   double pos1, pos2 ;
   TAG4 *tag ;
   long rc ;

   tag = map->tag ;
   start = end = 0L ;

   if ( start_con != 0 )
   {
      start = bitmap4seek( map, start_con, start_val, 0, 0 ) ;
      if ( start == -1L || t4eof( map->tag ) )
         return -2L ;
      if ( start > 0L )
         pos1 = t4position( tag ) ;
   }

   if ( end_con != 0 )
   {
      end = bitmap4seek( map, end_con, end_val, start, (int)check ) ;
      if ( end == -1L )
         return -2L ;
      if ( end > 0L )
         pos2 = t4position( tag ) ;
   }

   if ( start > 0L )
   {
      if ( end > 0L )
      {
         if ( pos1 > pos2 )   /* wrap around case, no matches */
            return do_flip ;
         if ( do_flip == 0 && ( pos2 - pos1 ) > 0.5 )   /* go around */
         {
            rc = do_flip = -1L ;
            for(;;)
            {
               if ( t4skip( tag, 1L ) != 1L )
                  break ;
               f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
            }
            start = bitmap4seek( map, start_con, start_val, 0, 0 ) ;
            for(;;)
            {
               if ( t4skip( tag, -1L ) != -1L )
                  break ;
               f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
            }
         }
         else
         {
            if ( do_flip == 0 )
               do_flip = 1 ;
            rc = do_flip ;
            for( ; rc == do_flip ; )
            {
               if ( do_flip == -1 )
                  f4flag_reset( flags, (unsigned long)t4recno( tag ) ) ;
               else
                  f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
               if ( t4recno( tag ) == start )
                  break ;
               rc = t4skip( tag, -1L ) == -1L ? do_flip : -do_flip ;
            }
         }
      }
      else
      {
         if ( do_flip == 0 )
         {
            if ( pos1 < 0.5 )
            {
               do_flip = -1L ;
               rc = t4skip( tag, do_flip ) ;
            }
            else
               rc = do_flip = 1L ;
         }
         else
            rc = do_flip ;
         for( ; rc == do_flip ; )
         {
            f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
            rc = t4skip( tag, do_flip ) ;
         }
      }
   }
   else
   {
      if ( end > 0L )
      {
         if ( do_flip == 0 )
         {
            if ( pos2 < .5 )
               rc = do_flip = -1L ;
            else
            {
               do_flip = 1L ;
               rc = t4skip( tag, do_flip ) ;
            }
         }
         else
            rc = do_flip ;
         for( ; rc == do_flip ; )
         {
            f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
            rc = t4skip( tag, do_flip ) ;
         }
         do_flip = ( do_flip == -1L ) ? 1L : -1L ;
      }
      else
         return -1L ;
   }

   return do_flip ;
}

/* generate the flags for the bitmap tree */
static int bitmap4flag_generate( BITMAP4 *map, int mode, F4FLAG *flags )
{
   CODE4 *code_base ;
   CONST4 *c_on ;
   long start, end, do_flip, prev_flip, rc ;
   TAG4 *tag ;
   double pos1, pos2 ;
   CONST4 *start_con, *end_con ;
   char start_val, end_val ;
   int is_flipped ;
   #ifdef S4HAS_DESCENDING
      int old_desc ;
   #endif

   code_base = map->log->code_base ;
   tag = map->tag ;

   #ifdef S4DEBUG
      if ( tag == 0 )
         e4severe( e4parm, E4_BM4FLAG_GEN ) ;
   #endif

   #ifdef S4HAS_DESCENDING
      old_desc = tag->header.descending ;
   #endif

   if ( flags->flags == 0 )
   {
      if ( f4flag_init( flags, code_base, (unsigned long)d4reccount( map->relate->data ) + 1L ) < 0 )
      {                                     
         #ifndef S4OPTIMIZE_OFF
            if ( code_base->error_code == e4memory )  /* failed due to out of memory */
               code_base->error_code = 0 ;
            if ( code_base->has_opt && code_base->opt.num_buffers != 0 )
            {
               d4opt_suspend( code_base ) ;
               if  ( code_base->opt.num_buffers > 8 )
                  code_base->opt.num_buffers-- ;
            }
            if ( f4flag_init( flags, code_base, (unsigned long)d4reccount( map->relate->data ) + 1L ) < 0 )
         #endif
            {
               #ifdef S4HAS_DESCENDING
                  t4descending( tag, old_desc ) ;
               #endif
               return -1 ;
            }
      }
      is_flipped = 0 ;
      do_flip = 0L ;
   }
   else
   {
      do_flip = ( flags->is_flip == 1L ) ? -1L : 1L ;
      is_flipped = 1 ;
   }

   start_val = end_val = 0 ;
   start_con = end_con = 0 ;

   if ( map->no_match == 1 )
   {
      if ( mode == 2 )
      {
         if ( is_flipped ) /* previous setting, so make all succeed */
         {
            f4flag_set_all( flags ) ;
            flags->is_flip = 0 ;
         }
         else   /* none set now, so make all succeed by simple flip */
            flags->is_flip = 1 ;
      }
      #ifdef S4HAS_DESCENDING
         t4descending( tag, old_desc ) ;
      #endif
      return 0 ;
   }

   if ( mode == 1 )  /* and */
   {
      if ( map->eq.len )
      {
         #ifdef S4DEBUG
            if ( map->gt.len || map->ge.len || map->le.len || map->lt.len || l4first( &map->ne ) != 0 )
               e4severe( e4info, E4_BM4FLAG_GEN ) ;  /* corrupt map detected */
         #endif
         start_con = end_con = &map->eq ;
         start_val = 1 ;
         end_val = 2 ;
         do_flip = bitmap4flag_range( flags, map, start_con, end_con, do_flip, start_val, end_val, 0L ) ;
      }
      else
      {
         if ( map->gt.len )
         {
            start_con = &map->gt ;
            start_val = 3 ;
         }
         else
            if ( map->ge.len )
            {
               start_con = &map->ge ;
               start_val = 1 ;
            }

         if ( map->lt.len )
         {
            end_con = &map->lt ;
            end_val = 0 ;
         }
         else
            if ( map->le.len )
            {
               end_con = &map->le ;
               end_val = 2 ;
            }

         do_flip = bitmap4flag_range( flags, map, start_con, end_con, do_flip, start_val, end_val, 1L ) ;
         if ( do_flip == -2L )  /* no matches */
         {
            f4flag_set_all( flags ) ;
            flags->is_flip = 1 ;
            #ifdef S4HAS_DESCENDING
               t4descending( tag, old_desc ) ;
            #endif
            return 0 ;
         }

         c_on = (CONST4 *)l4first( &map->ne ) ;
         if( c_on != 0 )
         {
            if ( do_flip != 0L )
               prev_flip = do_flip = ( do_flip == 1L ) ? -1L : 1L ;
            else
               prev_flip = 0L ;
            while ( c_on != 0 )
            {
               start_con = end_con = c_on ;
               start_val = 1 ;
               end_val = 2 ;
               do_flip = bitmap4flag_range( flags, map, start_con, end_con, do_flip, start_val, end_val, 0L ) ;
               if ( do_flip == -2L )
               {
                  if ( prev_flip == 0L )  /* all records not equal, so mark as such */
                  {
                     f4flag_set_all( flags ) ;
                     do_flip = -1L ;
                  }
                  else   /* otherwise no change since no un-equal records */
                     do_flip = prev_flip ;
               }
               c_on = (CONST4 *)l4next( &map->ne, c_on ) ;
               prev_flip = do_flip ;
            }
            do_flip = ( do_flip == 1L ) ? -1L : 1L ;
         }
      }
   }
   else  /* or */
   {
      start = end = do_flip = 0L ;
      c_on = (CONST4 *)l4first( &map->ne ) ;
      if ( c_on != 0 )
      {
         start_con = end_con = c_on ;
         start_val = 1 ;
         end_val = 2 ;
         do_flip = bitmap4flag_range( flags, map, start_con, end_con, do_flip, start_val, end_val, 0L ) ;
         if ( do_flip != -1 )
            f4flag_flip_returns( flags ) ;
         #ifdef S4DEBUG
            c_on = (CONST4 *)l4next( &map->ne, c_on ) ;
            if ( c_on != 0 )
               e4severe( e4info, E4_BM4FLAG_GEN ) ;  /* corrupt map detected */
         #endif
         #ifdef S4HAS_DESCENDING
            t4descending( tag, old_desc ) ;
         #endif
         return 0 ;
      }
      else
      {
         if ( map->gt.len )
            start = bitmap4seek( map, &map->gt, 3, 0, 0 ) ;
         else
            if ( map->ge.len )
               start = bitmap4seek( map, &map->ge, 1, 0, 0 ) ;

         if ( start != 0L )
            pos1 = (double)t4position( tag ) ;

         if ( map->lt.len )
            end = bitmap4seek( map, &map->lt, 0, start, 1 ) ;
         else
            if ( map->le.len )
               end = bitmap4seek( map, &map->le, 2, start, 1 ) ;

         if ( end == -1L )  /* no matches */
         {
            #ifdef S4HAS_DESCENDING
               t4descending( tag, old_desc ) ;
            #endif
            return 0 ;
         }

         if ( start > 0L )
         {
            if ( end > 0L )
            {
               pos2 = (double)t4position( tag ) ;
               if ( do_flip == 0 && ( pos2 - pos1 ) < 0.5 )   /* flip and do between */
               {
                  do_flip = 1L ;
                  if ( t4recno( tag ) != start )
                  {
                     rc = t4skip( tag, do_flip ) ;
                     for( ; rc == do_flip ; )
                     {
                        if ( code_base->error_code < 0 )
                        {
                           #ifdef S4HAS_DESCENDING
                              t4descending( tag, old_desc ) ;
                           #endif
                           return -1 ;
                        }
                        if ( t4recno( tag ) == start )
                           break ;
                        f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
                        rc = t4skip( tag, do_flip ) ;
                     }
                  }
                  do_flip = -1L ;
               }
               else  /* do both ways */
               {
                  for(;;)
                  {
                     f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
                     if ( code_base->error_code < 0 )
                     {
                        #ifdef S4HAS_DESCENDING
                           t4descending( tag, old_desc ) ;
                        #endif
                        return -1 ;
                     }
                     if ( t4skip( tag, -1L ) != -1L )
                        break ;
                  }
                  if ( map->gt.len )
                     start = bitmap4seek( map, &map->gt, 1, 0, 0 ) ;
                  else
                     if ( map->ge.len )
                        start = bitmap4seek( map, &map->ge, 2, 0, 0 ) ;
                  for(;;)
                  {
                     f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
                     if ( code_base->error_code < 0 )
                     {
                        #ifdef S4HAS_DESCENDING
                           t4descending( tag, old_desc ) ;
                        #endif
                        return -1 ;
                     }
                     if ( t4skip( tag, 1L ) != 1L )
                        break ;
                  }
               }
            }
            else
            {
               if ( pos1 < 0.5 )
               {
                  do_flip = -1 ;
                  rc = t4skip( tag, do_flip ) ;
               }
               else
                  do_flip = rc = 1 ;
               for( ; rc == do_flip ; )
               {
                  if ( code_base->error_code < 0 )
                  {
                     #ifdef S4HAS_DESCENDING
                        t4descending( tag, old_desc ) ;
                     #endif
                     return -1 ;
                  }
                  f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
                  rc = t4skip( tag, do_flip ) ;
               }
            }
         }
         else
         {
            if ( end > 0L )
            {
               if ( (double)t4position( tag ) > 0.5 )
               {
                  do_flip = 1 ;
                  rc = t4skip( tag, do_flip ) ;
               }
               else
                  do_flip = rc = -1 ;
               for( ; rc == do_flip ; )
               {
               
                  if ( code_base->error_code < 0 )
                  {
                     #ifdef S4HAS_DESCENDING
                        t4descending( tag, old_desc ) ;
                     #endif
                     return -1 ;
                  }
                  f4flag_set( flags, (unsigned long)t4recno( tag ) ) ;
                  rc = t4skip( tag, do_flip ) ;
               }
               do_flip = ( do_flip == -1L ) ? 1L : -1L ;
            }
         }

         if ( map->eq.len )
         {
            start_con = end_con = &map->eq ;
            start_val = 1 ;
            end_val = 2 ;
            prev_flip = do_flip ;
            do_flip = bitmap4flag_range( flags, map, start_con, end_con, do_flip, start_val, end_val, 0L ) ;
            if ( do_flip == -2L )
               do_flip = prev_flip ;
         }
      }
   }

   if ( do_flip == -1L && !is_flipped )
      f4flag_flip_returns( flags ) ;
   #ifdef S4HAS_DESCENDING
      t4descending( tag, old_desc ) ;
   #endif
   return 0 ;
}

/* generate the bitmaps */
static F4FLAG *bitmap4generate( BITMAP4 *map )
{
   BITMAP4 *map_on ;
   F4FLAG *flag1, *flag2 ;

   if ( map->branch == 0 )
   {
      flag1 = (F4FLAG *)u4alloc( sizeof( F4FLAG ) ) ;
      if ( flag1 == 0 )  /* insufficient memory */
         return 0 ;
      memset( (void *)flag1, 0, sizeof( F4FLAG ) ) ;
      if ( bitmap4flag_generate( map, map->and_or, flag1 ) < 0 )
      {
         u4free( flag1->flags ) ;
         u4free( flag1 ) ;
         return 0 ;
      }
      return flag1 ;
   }

   flag1 = flag2 = 0 ;

   map_on = (BITMAP4 *)l4first( &map->children ) ;
   while( map_on != 0 )
   {
      flag2 = bitmap4generate( map_on ) ;
      if ( flag1 == 0 )
         flag1 = flag2 ;
      else
      {
         #ifdef S4DEBUG
            if ( map->and_or != 1 && map->and_or != 2 )
               e4severe( e4info, E4_BM4FLAG_GEN ) ;
         #endif
         if ( map->and_or == 1 )
            f4flag_and( flag1, flag2 ) ;
         else
            f4flag_or( flag1, flag2 ) ;
         u4free( flag2->flags ) ;
         u4free( flag2 ) ;
      }
      map_on = (BITMAP4 *)l4next( &map->children, map_on ) ;
   }

   return flag1 ;
}

/* evaluate bitmaps out of the log */
int S4FUNCTION bitmap4evaluate( L4LOGICAL *log, int pos )
{
   BITMAP4 *map ;
   F4FLAG *flags ;

   map = bitmap4init( log, pos ) ;
   if ( map == 0 )
      return 0 ;

   map = bitmap4redistribute( 0, map, 1 ) ;
   if ( map == 0 )
      return 0 ;

   map = bitmap4redistribute_branch( 0, map ) ;
   if ( map == 0 )
   {
      if ( log->code_base->error_code == e4memory )
         log->code_base->error_code = 0 ;
      return 0 ;
   }

   flags = bitmap4generate( map ) ;
   if ( flags != 0 )
   {
      memcpy( (void *)&map->relate->set, (void *)flags, sizeof( F4FLAG ) ) ;
      u4free( flags ) ;
   }
   bitmap4free( log, map ) ;
   return 0 ;
}

#endif   /* S4INDEX_OFF */
