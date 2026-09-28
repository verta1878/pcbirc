/* c4const.c  (c)Copyright Sequiter Software Inc., 1993.  All rights reserved. */

#include "d4all.h"

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

/* this function redistributes (splits and combines) and/and, or/or block sequences */
BITMAP4 * S4FUNCTION bitmap4redistribute( BITMAP4 *parent, BITMAP4 *map, char do_shrink )
{
   BITMAP4 *child_map, *child_on, *parent2map ;
   char split ;

   if ( map->branch == 0 )
      return map ;

   /* first combine all the children of this map */
   child_on = child_map = (BITMAP4 *)l4first( &map->children ) ;
   for( ;; )
   {
      if ( child_on == 0 )
         break ;
      child_on = bitmap4redistribute( map, child_on, 0 ) ;
      child_on = (BITMAP4 *)l4next( &map->children, child_on ) ;
   }

   /* now combine all leaf children where possible */

   if ( parent != 0 )
      if ( parent->and_or != map->and_or )  /* case where no combos possible */
         return map ;

   parent2map = 0 ;
   child_on = child_map = (BITMAP4 *)l4first( &map->children ) ;
   for( ; child_map != 0 ; )
   {
      child_on = (BITMAP4 *)l4next( &map->children, child_map ) ;
      if ( child_on == 0 )
         break ;

      split = 0 ;
      if ( child_on->tag != child_map->tag || child_on->and_or != child_map->and_or )
        split = 1 ;
      else
      {
        if ( map != 0 )
           if ( map->and_or != child_on->and_or )
              split = 1 ;
      }

      if ( split == 1 )
      {
         if ( parent2map == 0 )
         {
            parent2map = bitmap4create( map->log, map->relate, map->and_or, 1 ) ;
            if ( parent2map == 0 )  /* must handle by freeing... */
               return 0 ;
            if ( parent == 0 )
            {
               parent = bitmap4create( map->log, map->relate, map->and_or, 1 ) ;
               if ( parent == 0 )  /* must handle by freeing... */
                  return 0 ;
               l4add( &parent->children, map ) ;
            }
            l4add( &parent->children, parent2map ) ;
         }
//         if ( parent != 0 )  /* have a parent */
//            if ( child_on->children.n_link == 0 ) /* our children not branches */
//               if ( map->and_or != parent->and_or )
//               {
                 l4remove( &map->children, child_on ) ;
                 l4add( &parent2map->children, child_on ) ;
//               }
      }
      else
         child_map = bitmap4combine_leafs( map, child_map, child_on ) ;
   }

   if ( parent2map != 0 )
   {
      #ifdef S4DEBUG
         if ( parent == 0 )
            e4severe( e4info, E4_BM4REDIST ) ;
      #endif
      bitmap4redistribute( parent, parent2map, 1 ) ;
   }

   if ( do_shrink )
   {
      if ( map->children.n_link == 1 )   /* just a child, so remove myself */
      {
         child_map = (BITMAP4 *)l4first( &map->children ) ;
         l4remove( &map->children, child_map ) ;
         if ( parent != 0 )
         {
            #ifdef S4DEBUG
               if ( child_map->tag == 0 && child_map->children.n_link == 0 )
                  e4severe( e4info, E4_BM4REDIST ) ;
            #endif
            if ( parent->tag == 0 && child_map->tag != 0 )
               parent->tag = child_map->tag ;
            l4add_after( &parent->children, map, child_map ) ;
            l4remove( &parent->children, map ) ;
         }
         bitmap4destroy( map ) ;
         map = child_map ;
      }
   }

   if ( parent2map != 0 && parent != 0 )
      return parent ;

   return map ;
}

/* this function redistributes the input maps by breaking the one up into constants and creating maps for each */
BITMAP4 * S4FUNCTION bitmap4redistribute_leaf( BITMAP4 *parent, BITMAP4 *map1, BITMAP4 *map2 )
{
   BITMAP4 *new_branch, *or_map, *place, *and_map, *temp ;
   CONST4 *c_on ;

   new_branch = bitmap4create( parent->log, parent->relate, 1, 1 ) ;
   if ( new_branch == 0 )
      return 0 ;

   place = bitmap4create( parent->log, parent->relate, 0, 0 ) ;
   if ( place == 0 )
      return 0 ;
   l4add_after( &parent->children, map1, place ) ;

   l4remove( &parent->children, map1 ) ;
   l4remove( &parent->children, map2 ) ;

   if ( map1->and_or == 1 )
   {
      and_map = map1 ;
      or_map = map2 ;
   }
   else
   {
      and_map = map2 ;
      or_map = map1 ;
   }

   bitmap4constant_combine( new_branch, and_map, &or_map->lt, 1 ) ;
   bitmap4constant_combine( new_branch, and_map, &or_map->le, 2 ) ;
   bitmap4constant_combine( new_branch, and_map, &or_map->gt, 3 ) ;
   bitmap4constant_combine( new_branch, and_map, &or_map->ge, 4 ) ;
   bitmap4constant_combine( new_branch, and_map, &or_map->eq, 5 ) ;
   for( ;; )
   {
      c_on = (CONST4 *)l4first( &or_map->ne ) ;
      if ( c_on == 0 )
         break ;
      bitmap4constant_combine( new_branch, and_map, c_on, 6 ) ;
   }

   if ( parent->log->code_base->error_code == e4memory )
      return 0 ;

   if ( new_branch->children.n_link == 0 )   /* collapsed */
   {
      if ( parent->tag == 0 && and_map->tag != 0 )
         parent->tag = and_map->tag ;
      bitmap4destroy( new_branch ) ;
      new_branch = 0 ;
   }
   else
   {
      while( new_branch->branch == 1 && new_branch->children.n_link == 1 )
      {
         temp = (BITMAP4 *)l4first( &new_branch->children ) ;
         bitmap4destroy( new_branch ) ;
         new_branch = temp ;
      }
      l4add_after( &parent->children, place, new_branch ) ;
   }

   l4remove( &parent->children, place ) ;
   bitmap4destroy( place ) ;
   bitmap4destroy( or_map ) ;
   bitmap4destroy( and_map ) ;

   return new_branch ;
}

/* this function splits and combines and/or, or/and block sequences */
/* all bitmaps must be in standard bitmap4redistribute format prior to call */
BITMAP4 * S4FUNCTION bitmap4redistribute_branch( BITMAP4 *parent, BITMAP4 *map )
{
   BITMAP4 *child_on2, *child_on, *child_next2 ;

   if ( map->branch == 0 )
      return map ;

   child_on = (BITMAP4 *)l4first( &map->children ) ;

   for( ;; )
   {
      if ( child_on == 0 )
         break ;
      if ( child_on->branch )
      {
         child_on = bitmap4redistribute_branch( map, child_on ) ;
         if ( child_on == 0 && parent->log->code_base->error_code == e4memory )
            return 0 ;
      }
      if ( child_on->branch == 0 )
      {
         child_on2 = (BITMAP4 *)l4next( &map->children, child_on ) ;
         while( child_on2 != 0 )
         {
            if ( child_on2->branch )
            {
               child_on2 = bitmap4redistribute_branch( map, child_on2 ) ;
               if ( child_on2 == 0 && parent->log->code_base->error_code == e4memory )
                  return 0 ;
            }
            child_next2 = (BITMAP4 *)l4next( &map->children, child_on2 ) ;
            if ( child_on->branch == 0 && map->and_or == 1 && child_on->tag == child_on2->tag &&  child_on->and_or != child_on2->and_or )
            {
               child_on = bitmap4redistribute_leaf( map, child_on, child_on2 ) ;
               if ( child_on == 0 && parent->log->code_base->error_code == e4memory )
                  return 0 ;
            }
            child_on2 = child_next2 ;
         }
      }
      child_on = (BITMAP4 *)l4next( &map->children, child_on ) ;
   }

   if ( map->branch == 1 )
   {
      if ( map->children.n_link == 0 )   /* mark ourselves as a leaf with no match */
      {
         map->branch = 0 ;
         map->no_match = 1 ;
      }
      else
         if ( map->children.n_link == 1 )   /* just a child, so remove myself */
         {
            child_on = (BITMAP4 *)l4first( &map->children ) ;
            l4remove( &map->children, child_on ) ;
            if ( parent != 0 )
            {
               l4add_after( &parent->children, map, child_on ) ;
               l4remove( &parent->children, map ) ;
            }
            bitmap4destroy( map ) ;
            map = child_on ;
         }
   }

   return map ;
}

/* returns a pointer to the constant value */
void *S4FUNCTION const4return( L4LOGICAL *log, CONST4 *c1 )
{
   return (void *)( log->buf + c1->offset ) ;
}

/* updates the log's constant memory buffer, re-allocating memory if required */
int S4FUNCTION const4mem_alloc( L4LOGICAL *log, unsigned len )
{
   if ( ( log->buf_pos + len ) > log->buf_len )
   {
      #ifdef S4DEBUG
         if ( (long)len + (long)log->buf_len != (long)(len + log->buf_len) )
            e4severe( e4memory, E4_MEMORY_OOR ) ;
      #endif
      if ( u4alloc_again( log->code_base, &log->buf, &log->buf_len, log->buf_pos + len ) != 0 )
         return -1 ;
   }
   log->buf_pos += len ;
   return 0 ;
}

/* duplicate an existing constant */
int S4FUNCTION const4duplicate( CONST4 *to, CONST4 *from, L4LOGICAL *log )
{
   unsigned len ;

   len = from->len ;

   if ( len == 0 )
      memset( (void *)to, 0, sizeof( CONST4 ) ) ;
   else
   {
      if ( const4mem_alloc( log, len ) < 0 )
         return -1 ;
      memcpy( log->buf + log->buf_pos - len, const4return( log, from ), len ) ;
      to->offset = log->buf_len - len ;
      to->len = len ;
   }

   return 0 ;
}

/* get a constant from an expr. info structure */
int S4FUNCTION const4get( CONST4 *con, BITMAP4 *map, L4LOGICAL *log, int pos )
{
   unsigned len ;
   char *result ;

   if ( expr4execute( log->expr, pos, (void **)&result ) < 0 )
      return -1 ;
   len = log->expr->info[pos].len ;

   #ifdef S4DEBUG
      if ( map->type != 0 && map->type != v4functions[log->expr->info[pos].function_i].return_type )
         e4severe( e4info, E4_CONST_EIN ) ;
   #endif

   if ( const4mem_alloc( log, len ) < 0 )
      return -1 ;

   memcpy( log->buf + log->buf_pos - len, result, len ) ;
   map->type = v4functions[log->expr->info[pos].function_i].return_type ;
   con->offset = log->buf_len - len ;
   con->len = len ;

   return 0 ;
}

int S4FUNCTION const4less( CONST4 *p1, CONST4 *p2, BITMAP4 *map )
{
   switch( map->type )
   {
      case r4num_doub:
      case r4date_doub:
         #ifdef S4DEBUG
            if ( p1->len != p2->len )
               e4severe( e4info, E4_CONST4LESS ) ;
         #endif
         if ( *(double *)const4return( map->log, p1 ) < *(double *)const4return( map->log, p2 ) )
            return 1 ;
         break ;
      case r4num:
      case r4str:
         if ( p1->len < p2->len )
         {
            if ( memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), p1->len ) <= 0 )
               return 1 ;
         }
         else
            if ( memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), p2->len ) < 0 )
               return 1 ;
         break ;
      default:
         e4severe( e4info, E4_CONST4LESS ) ;
   }

   return 0 ;
}

int S4FUNCTION const4eq( CONST4 *p1, CONST4 *p2, BITMAP4 *map )
{
   if ( p1->len < p2->len )
   {
      #ifdef S4DEBUG
         if ( map->type == r4num_doub || map->type == r4date_doub )
            e4severe( e4info, E4_CONST4EQ ) ;
      #endif
      return 0 ;
   }

   #ifdef S4DEBUG
      switch( map->type )
      {
         case r4num_doub:
         case r4date_doub:
         case r4num:
         case r4str:
         case r4log:
            break ;
         default:
            e4severe( e4info, E4_CONST4EQ ) ;
      }
   #endif

   if ( memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), p1->len ) == 0 )
      return 1 ;

   return 0 ;
}

int S4FUNCTION const4less_eq( CONST4 *p1, CONST4 *p2, BITMAP4 *map )
{
   switch( map->type )
   {
      case r4num_doub:
      case r4date_doub:
         #ifdef S4DEBUG
            if ( p1->len != p2->len )
               e4severe( e4info, E4_CONST4LESS_EQ ) ;
         #endif
         if ( *(double *)const4return( map->log, p1 ) <= *(double *)const4return( map->log, p2 ) )
            return 1 ;
         break ;
      case r4num:
      case r4str:
         if ( p1->len <= p2->len )
         {
            if ( memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), p1->len ) <= 0 )
               return 1 ;
         }
         else
            if ( memcmp( const4return( map->log, p1 ), const4return( map->log, p2 ), p2->len ) < 0 )
               return 1 ;
         break ;
      default:
         e4severe( e4info, E4_CONST4LESS_EQ ) ;
   }

   return 0 ;
}

void S4FUNCTION const4add_ne( BITMAP4 *map, CONST4 *con )
{
   CONST4 *c_on ;

   c_on = (CONST4 *)l4first( &map->ne ) ;
   while ( c_on != 0 )
   {
      if ( const4eq( con, c_on, map ) )  /* ne already exists, so ignore */
         return ;
      c_on = (CONST4 *)l4next( &map->ne, c_on ) ;
   }
   c_on = (CONST4 *) u4alloc( sizeof( CONST4 ) ) ;
   if ( c_on == 0 )
      return ;
   memcpy( (void *)c_on, (void *)con, sizeof( CONST4 ) ) ;
   l4add( &map->ne, c_on ) ;
   memset( (void *)con, 0, sizeof( CONST4 ) ) ;
}

void S4FUNCTION const4delete_ne( LIST4 *list, CONST4 *con )
{
   l4remove( list, con ) ;
   u4free( con ) ;
}

#endif   /* S4INDEX_OFF */

