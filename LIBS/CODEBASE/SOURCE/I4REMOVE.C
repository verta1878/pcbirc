/* i4remove.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
#ifndef S4INDEX_OFF

int S4FUNCTION t4remove( TAG4 *t4, char *ptr, long rec )
{
   int rc ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4REMOVE ) ;
   #endif

   rc = t4go( t4, ptr, rec ) ;   /* returns -1 if code_base->error_code < 0 */
   if ( rc < 0 )
       return rc ;
   if ( rc )
      return r4entry ;

   return t4remove_current( t4 ) ;
}

int S4FUNCTION t4remove_calc( TAG4 *t4, long rec )
{
   char *ptr ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4REM_CALC ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   t4expr_key( t4, &ptr ) ;
   return t4remove( t4, ptr, rec ) ;
}

#ifndef N4OTHER

#ifdef S4FOX
/* remove the (current) branch block */
void S4FUNCTION t4remove_branch( TAG4 *t4, B4BLOCK *block_on )
{
   long l_node, r_node ;
   INDEX4 *i4 ;

   i4 = t4->index ;

   if ( block_on == (B4BLOCK *)l4first( &t4->blocks ) )
   {
      /* Root Block, do not delete */
      #ifdef S4DEBUG
         if ( block_on->header.left_node != -1 || block_on->header.right_node != -1 || block_on->header.n_keys != 1 )
            e4severe( e4info, E4_INFO_CIB ) ;
      #endif
      memset( block_on->data, 0, B4BLOCK_SIZE - sizeof(B4STD_HEADER) - sizeof(B4NODE_HEADER) ) ;
      if ( !b4leaf( block_on ) )   /* if not a leaf, then reset node_hdr too */
      {
         memset( (void *)&block_on->node_hdr, 0, sizeof(B4NODE_HEADER) ) ;
         b4leaf_init( block_on ) ;
      }
      block_on->header.n_keys = 0 ;
      block_on->header.left_node = -1 ;
      block_on->header.right_node = -1 ;
      block_on->key_on = -1 ;
      block_on->built_on = -1 ;
      block_on->header.node_attribute = 3 ;  /* root and leaf */
      block_on->changed = 1 ;
   }
   else /* This block is to be deleted */
   {
      l_node = block_on->header.left_node ;
      r_node = block_on->header.right_node ;
      l4remove( &t4->blocks, block_on ) ;
      if ( i4shrink( i4, block_on->file_block ) < 0 )
         return ;
      block_on->changed = 0 ;

      if ( l_node != -1L )
      {
         if ( file4read_all( &i4->file, I4MULTIPLY*l_node, &block_on->header, B4BLOCK_SIZE) < 0 )
            return ;
         block_on->file_block = l_node ;
         block_on->header.right_node = r_node ;
         block_on->changed = 1 ;
         b4flush( block_on ) ;
      }

      if ( r_node != -1L )
      {
         if ( file4read_all( &i4->file, I4MULTIPLY*r_node, &block_on->header, B4BLOCK_SIZE) < 0 )
            return ;
         block_on->file_block = r_node ;
         block_on->header.left_node = l_node ;
         block_on->changed = 1 ;
         b4flush( block_on ) ;
      }

      b4free( block_on ) ;
   }
}
#endif

/* Remove the current key */
int S4FUNCTION t4remove_current( TAG4 *t4 )
{
   B4BLOCK *block_on ;
   INDEX4 *i4 ;
   #ifdef S4FOX
      void *new_key_info ;
      long rec ;
      int bl_removed, less_than_last ;
   #else
      int remove_done, update_reqd ;
      B4BLOCK *block_iterate ;
   #endif

   i4 = t4->index ;

   #ifdef S4FOX
      i4->tag_index->header.version = i4->version_old+1 ;
      new_key_info = 0 ;

      for ( block_on = (B4BLOCK *)t4->blocks.last_node ; block_on ; )
      {
         bl_removed = 0 ;
         if ( new_key_info == 0 )  /* then delete entry */
         {
            if ( b4lastpos( block_on ) == 0 )
            {
               if ( block_on != (B4BLOCK *)l4first( &t4->blocks ) )
                  bl_removed = 1 ;
               t4remove_branch( t4, block_on ) ;
               block_on = (B4BLOCK *)t4->blocks.last_node ;
            }
            else
            {
               less_than_last = 0 ;
               if ( block_on->key_on < b4lastpos( block_on ) )
                  less_than_last = 1 ;
               b4remove( block_on ) ;
               if ( less_than_last )
                  return 0 ;

               /* On last entry */
               b4go_eof( block_on ) ;
               block_on->key_on-- ;  /* update key_on for goint to last spot */
               new_key_info = b4key_key( block_on, block_on->key_on ) ;
               rec = b4recno( block_on, block_on->key_on ) ;
            }
         }
         else  /* Adjust entry */
         {
            b4br_replace( block_on, (char *)new_key_info, rec ) ;
            if ( block_on->key_on != b4lastpos( block_on ) )  /* not on end key, so exit, else continue */
               return 0 ;
         }

         if ( !bl_removed )
         {
            block_on = (B4BLOCK *)block_on->link.p ;
            if ( block_on == (B4BLOCK *)t4->blocks.last_node )
               break ;
         }
      }
   #endif

   #ifdef S4MDX
      remove_done = 0 ;
      update_reqd = 0 ;
      i4->changed = 1 ;
      t4->changed = 1 ;
      t4->header.version++ ;

      block_iterate = (B4BLOCK *)l4last( &t4->blocks ) ;
      for ( ;; )
      {
         block_on = block_iterate ;
         if ( block_on == 0 )
            break ;
         block_iterate = (B4BLOCK *)l4prev( &t4->blocks, block_iterate ) ;  /* Calculate the previous block while the current block exists. */

         if ( !remove_done )  /* either removing or else updating */
         {
            if ( b4lastpos( block_on ) == 0 )  /* delete block */
            {
               if ( block_on == (B4BLOCK *)l4first( &t4->blocks ) )  /* root block, don't delete */
               {
                  block_on->changed = 1 ;
                  block_on->key_on = 0 ;
                  memset( (void *)&block_on->n_keys, 0, i4->header.block_rw ) ;
                  if (t4->filter )  /* must modify the filter setting for dBASE IV compatibility */
                  {
                     file4write(&t4->index->file, t4->header_offset+sizeof(t4->header)+222, (char *)"\0", (int)1 ) ;
                     t4->has_keys = 0 ;
                     #ifdef S4MDX
                        t4->had_keys = 1 ;
                     #endif
                  }
                  return 0 ;
               }
               else
               {
                  l4remove( &t4->blocks, block_on ) ;
                  if ( i4shrink( i4, block_on->file_block) < 0 )
                     return -1 ;
                  b4free( block_on ) ;
               }
            }
            else  /* just remove entry */
            {
               if ( block_iterate && block_on->key_on == b4lastpos( block_on ) && b4lastpos( block_on ) > 0 )  /* save to become last key of the block for parent update */
               {
                  memcpy( t4->code_base->saved_key, b4key_key( block_on, block_on->n_keys - 1 - b4leaf(block_on) ), t4->header.key_len ) ;
                  update_reqd = 1 ;
               }
               else
                  update_reqd = 0 ;

               b4remove( block_on ) ;
               if ( !b4leaf( block_on ) && b4lastpos( block_on ) == 0 )  /* branch with only one entry, so have the entry take place of this block */
               {
                  if (block_on == (B4BLOCK *)l4first( &t4->blocks ) )   /* the entry becomes the new root */
                  {
                     /* first update the tags root */
                     t4->header.root = b4key( block_on, 0 )->num ;
                     #ifdef S4BYTE_SWAP
                        t4->header.root = x4reverse_long( t4->header.root ) ;
                     #endif
                     file4write( &t4->index->file, t4->header_offset, (void *)&t4->header.root, sizeof(t4->header.root) ) ;
                     #ifdef S4BYTE_SWAP
                        t4->header.root = x4reverse_long( t4->header.root ) ;
                     #endif
                     update_reqd = 0 ;
                  }
                  else  /* remove this branch block */
                  {
                     block_iterate->changed = 1 ;
                     b4key( block_iterate, block_iterate->key_on )->num = b4key( block_on, 0 )->num ;
                  }
                  l4remove( &t4->blocks, block_on ) ;
                  if ( i4shrink( t4->index, block_on->file_block) < 0 )
                      return -1 ;
                  block_on->changed = 0 ;
                  b4free( block_on ) ;
               }
               else
                  block_on->key_on = b4lastpos( block_on ) ;

               if ( !update_reqd )
                  return 0 ;
               remove_done = 1 ;
            }
         }
         else  /* Adjust entry - at most one update will be required in MDX */
         {
            if ( block_on->key_on < b4lastpos( block_on ) )
            {
               block_on->changed = 1 ;
               memcpy( b4key_key( block_on, block_on->key_on), t4->code_base->saved_key, t4->header.key_len ) ;
               return 0 ;
            }
         }
      }
   #endif

   return 0 ;
}

#endif   /* ifndef N4OTHER  */

#ifdef N4OTHER

#ifdef S4NDX
int S4FUNCTION t4remove_current( TAG4 *t4 )
{
   B4BLOCK *block_on, *block_iterate ;
   int remove_done, update_reqd ;
   #ifdef S4USE_OLD
      void *new_key_info ;
      long shrink_br = 0 ;
      B4BLOCK *shrink_block = 0 ;
      new_key_info = 0 ;
   #endif

   t4->header.version = t4->header.old_version+1 ;

   remove_done = 0 ;
   update_reqd = 0 ;
   t4->header.version++ ;

   block_iterate = (B4BLOCK *)l4last( &t4->blocks ) ;
   for ( ;; )
   {
      block_on = block_iterate ;
      if ( block_on == 0 )
         break ;
      block_iterate = (B4BLOCK *)l4prev( &t4->blocks, block_iterate ) ;  /* Calculate the previous block while the current block exists. */

      if ( !remove_done )  /* either removing or else updating */
      {
         if ( b4lastpos( block_on ) == 0 )  /* delete block */
         {
            block_on->changed = 1 ;
            block_on->key_on = 0 ;
            memset( (void *)&block_on->n_keys, 0, B4BLOCK_SIZE ) ;
            if ( block_on == (B4BLOCK *)l4first( &t4->blocks ) )  /* root block, don't delete */
               return 0 ;
            else
            {
               l4remove( &t4->blocks, block_on ) ;
               if ( b4flush( block_on ) < 0 )
                  return -1 ;
               b4free( block_on ) ;
            }
         }
         else  /* just remove entry */
         {
            if ( block_iterate && block_on->key_on == b4lastpos( block_on ) && b4lastpos( block_on ) > 0 )  /* save to become last key of the block for parent update */
            {
               memcpy( t4->code_base->saved_key, b4key_key( block_on, block_on->n_keys - 1 - b4leaf(block_on) ), t4->header.key_len ) ;
               update_reqd = 1 ;
            }
            else
               update_reqd = 0 ;

            b4remove( block_on ) ;
            if ( !b4leaf( block_on ) && b4lastpos( block_on ) == 0 )  /* branch with only one entry, so have the entry take place of this block */
            {
               if (block_on == (B4BLOCK *)l4first( &t4->blocks ) )   /* the entry becomes the new root */
               {
                  /* first update the tags root */
                  t4->header.root = b4key( block_on, 0 )->num ;
                  #ifdef S4BYTE_SWAP
                     t4->header.root = x4reverse_long( t4->header.root ) ;
                  #endif
                  file4write( &t4->index->file, t4->header_offset, (void *)&t4->header.root, sizeof(t4->header.root) ) ;
                  #ifdef S4BYTE_SWAP
                     t4->header.root = x4reverse_long( t4->header.root ) ;
                  #endif
                  update_reqd = 0 ;
               }
               else  /* remove this branch block */
               {
                  block_iterate->changed = 1 ;
                  b4key( block_iterate, block_iterate->key_on )->pointer = b4key( block_on, 0 )->pointer ;
               }
               l4remove( &t4->blocks, block_on ) ;
               block_on->changed = 1 ;
               memset( &block_on->n_keys, 0, B4BLOCK_SIZE ) ;
               if ( b4flush( block_on ) < 0 )
                  return -1 ;
               b4free( block_on ) ;
            }
            else
               block_on->key_on = b4lastpos( block_on ) ;

            if ( !update_reqd )
               return 0 ;
            remove_done = 1 ;
         }
      }
      else  /* Adjust entry - at most one update will be required in MDX */
      {
         if ( block_on->key_on < b4lastpos( block_on ) )
         {
            block_on->changed = 1 ;
            memcpy( b4key_key( block_on, block_on->key_on), t4->code_base->saved_key, t4->header.key_len ) ;
            return 0 ;
         }
      }
   }

   return 0 ;
}
#else
#ifdef S4CLIPPER
int S4FUNCTION t4balance_branch( TAG4 *, B4BLOCK * ) ;

int S4FUNCTION t4remove_ref( TAG4 *t4 )
{
   B4KEY_DATA *my_key_data ;
   B4BLOCK *block_on, *block_up ;
   long reference ;
   int i ;
   int rc ;

   my_key_data = (B4KEY_DATA *)u4alloc_er( t4->code_base, t4->header.group_len ) ;
   if ( my_key_data == 0 )
      return -1 ;
   block_on = (B4BLOCK *)t4->blocks.last_node ;

   /*  no longer required...
   #ifdef S4DEBUG
      if ( block_on->n_keys )
         e4severe( e4info, E4_INFO_DEL ) ;
   #endif
   */

   t4up( t4 ) ;
   block_up = (B4BLOCK *)t4->blocks.last_node ;

   if ( block_up == 0 )  /* root block only, so reference doesn't exist */
   {
      /* reset the block */
      short offset = ( t4->header.keys_max + 2 + ( (t4->header.keys_max/2)*2 != t4->header.keys_max ) ) * sizeof(short) ;
      for ( i = 0 ; i <= t4->header.keys_max ; i++ )
         block_on->pointers[i] = (short)( t4->header.group_len * i ) + offset ;
      return 0 ;
   }
   else  /* delete the block */
   {
      t4shrink( t4, block_on->file_block ) ;
      l4pop( &t4->saved ) ;
      block_on->changed = 0 ;
      b4free( block_on ) ;
   }
   block_on = block_up ;

   if ( block_on->key_on >= block_on->n_keys )
      memcpy( &(my_key_data->num), &( b4key( block_on, block_on->key_on - 1 )->num), sizeof(long) + t4->header.key_len ) ;
   else
      memcpy( &(my_key_data->num), &( b4key( block_on, block_on->key_on )->num), sizeof(long) + t4->header.key_len ) ;

   if ( block_on->n_keys == 1 )
   {
      /* take the branch, and put in place of me, then delete and add me */
      if ( block_on->key_on >= block_on->n_keys )
         reference = (long) b4key( block_on, 0 )->pointer ;
      else
         reference = (long) b4key( block_on, 1 )->pointer ;
      t4shrink( t4, block_on->file_block ) ;
      t4up( t4 ) ;
      l4pop(&t4->saved ) ;
      block_on->changed = 0 ;
      b4free( block_on ) ;
      block_on = (B4BLOCK *)t4->blocks.last_node ;
      if ( block_on == 0 )  /* just removed root, so make reference root! */
         t4->header.root = reference ;
      else
      {
         memcpy( &(b4key( block_on, block_on->key_on )->pointer), &reference, sizeof( reference ) )  ;
         block_on->changed = 1 ;
      }
   }
   else    /* just remove myself, add later */
   {
      b4remove( block_on ) ;
      if ( block_on->n_keys < t4->header.keys_half && block_on->file_block != t4->header.root )  /* if not root may have to balance the tree */
      {
         if ( t4balance_branch( t4, block_on ) < 0 )
            return -1 ;
      }
   }

   /* now add the removed reference back into the index */
   rc = t4add( t4, (unsigned char *)my_key_data->value, my_key_data->num ) ;
   u4free( my_key_data ) ;

   return rc ;
}

static int t4balance_block( TAG4 *t4, B4BLOCK *parent, B4BLOCK *b4, B4BLOCK *b4temp, char is_right, char balance_mode )
{
   B4KEY_DATA *bdata, *bdata2 ;
   int avg, i ;
   long temp_fb ;

   if ( balance_mode == 2 && b4leaf( b4temp ) )
   {
      #ifdef S4DEBUG
         e4severe( e4info, E4_INFO_CIF ) ;
      #endif
      return -1 ;
   }
   else
   {
      if ( b4temp->n_keys + b4->n_keys < t4->header.keys_max )  /* join the two */
      {
         if ( is_right )
         {
            if ( balance_mode == 2 )
            {
               b4->key_on = 0 ;
               parent->key_on-- ;
               bdata = b4key( parent, parent->key_on ) ;
               b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->n_keys )->pointer / 512 ) ;
               for ( i = b4temp->n_keys - 1 ; i >= 0 ; i-- )
               {
                  b4->key_on = 0 ;
                  bdata = b4key( b4temp, i ) ;
                  b4insert( b4, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               }
               b4temp->changed = 0 ;
               t4shrink( t4, b4temp->file_block ) ;
               b4remove( parent ) ;
               if ( parent->n_keys < t4->header.keys_half )
                  t4balance_branch( t4, parent ) ;
            }
            else
            {
               for ( i = 0 ; i < b4->n_keys ; i++ )
               {
                  b4go_eof( b4temp ) ;
                  bdata = b4key( b4, i ) ;
                  b4insert( b4temp, bdata->value, bdata->num, 0L ) ;
               }

               t4->index->code_base->do_index_verify = 0 ;  /* avoid verify errors due to our partial removal */
               do
               {
                  t4down( t4 ) ;
               } while( !b4leaf( t4block( t4 ) ) ) ;
               t4->index->code_base->do_index_verify = 1 ;
                 
               b4flush( b4temp ) ;
               if ( t4remove_ref( t4 ) != 0 )  /* will delete b4 and will remove and re-add the reference if required */
                  return -1 ;
            }
         }
         else
         {
            /* put parent entry, then all but one child, then last child to parent */
            b4go_eof( b4 ) ;
            if ( balance_mode == 2 )
            {
               b4temp->key_on = 0 ;
               bdata = b4key( parent, parent->key_on ) ;
               b4insert( b4temp, bdata->value, bdata->num, b4key( b4, b4->n_keys )->pointer / 512 ) ;
               for ( i = b4->n_keys - 1 ; i >= 0 ; i-- )
               {
                  b4temp->key_on = 0 ;
                  bdata = b4key( b4, i ) ;
                  b4insert( b4temp, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               }
               t4shrink( t4, b4->file_block ) ;
               l4pop( &t4->saved ) ;
               b4->changed = 0 ;
               b4free( b4 ) ;
               b4flush( b4temp ) ;
               b4remove( parent ) ;
               if ( parent->n_keys < t4->header.keys_half )
                  t4balance_branch( t4, parent ) ;
            }
            else
            {
               bdata = b4key( parent, parent->key_on ) ;
               b4insert( b4, bdata->value, bdata->num, 0L ) ;
               for ( i = 0 ; i < b4temp->n_keys - 1 ; i++ )
               {
                  b4go_eof( b4 ) ;
                  bdata = b4key( b4temp, i ) ;
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               }
               bdata = b4key( parent, parent->key_on ) ;
               bdata2 = b4key( b4temp, b4temp->n_keys - 1 ) ;
               memcpy( bdata->value, bdata2->value, t4->header.key_len )  ;
               memcpy( &( bdata->num ), &bdata2->num, sizeof( long ) )  ;

               parent->key_on++ ;
               parent->changed = 1 ;
               
               t4->index->code_base->do_index_verify = 0 ;  /* avoid verify errors due to our partial removal */
               b4flush( b4 ) ;
               do
               {
                  if ( t4down( t4 ) == 1 )
                     break ;
               } while( ((B4BLOCK *)t4->blocks.last_node)->file_block != b4temp->file_block ) ;
               t4->index->code_base->do_index_verify = 1 ;
               if ( t4remove_ref( t4 ) != 0 )  /* will delete b4temp and will remove and re-add the reference if required */
                  return -1 ;
               b4temp->changed = 0 ;
            }
         }
      }
      else  /* will have to redistribute keys */
      {
         avg = ( b4->n_keys + b4temp->n_keys + 1 ) / 2 ;
         if ( avg < t4->header.keys_half )
            avg = t4->header.keys_half ;
         if ( is_right )
         {
            b4->key_on = 0 ;
            parent->key_on-- ;
            bdata = b4key( parent, parent->key_on ) ;

            if ( balance_mode == 2 )
            {
               if ( b4->n_keys == 0 )
               {
                  temp_fb = b4key( b4, 0 )->pointer ;
                  b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->n_keys )->pointer / 512 ) ;
                  b4append( b4, temp_fb / 512 ) ;
               }
               else
                  b4insert( b4, bdata->value, bdata->num, b4key( b4temp, b4temp->n_keys )->pointer / 512 ) ;
            }
            else
               b4insert( b4, bdata->value, bdata->num, 0L ) ;

            while ( b4temp->n_keys > avg && b4->n_keys < avg )
            {
               b4->key_on = 0 ;
               b4temp->key_on = b4temp->n_keys - 1 ;
               bdata = b4key( b4temp, b4temp->key_on ) ;
               if ( balance_mode == 2 )
                  b4insert( b4, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               else
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               b4remove( b4temp ) ;
            }
            b4temp->key_on = b4temp->n_keys - 1 ;
         }
         else
         {
            b4go_eof( b4 ) ;
            if ( balance_mode == 2 )
            {
               bdata = b4key( b4, b4->n_keys ) ;
               bdata2 = b4key( parent, parent->key_on ) ;
               memcpy( bdata->value, bdata2->value, t4->header.key_len )  ;
               memcpy( &(bdata->num), &bdata2->num, sizeof( long ) )  ;
               b4->n_keys++ ;
            }
            else
            {
               bdata = b4key( parent, parent->key_on ) ;
               b4insert( b4, bdata->value, bdata->num, 0L ) ;
            }

            while ( b4->n_keys - (balance_mode == 2) < avg )
            {
               b4go_eof( b4 ) ;
               b4temp->key_on = 0 ;
               bdata = b4key( b4temp, b4temp->key_on ) ;
               if ( balance_mode == 2 )
                  b4insert( b4, bdata->value, bdata->num, bdata->pointer / 512 ) ;
               else
                  b4insert( b4, bdata->value, bdata->num, 0L ) ;
               b4remove( b4temp ) ;
            }

            if ( balance_mode == 2 )
            {
               b4->n_keys-- ;
               b4->key_on-- ;
            }
            b4temp->key_on = 0 ;
         }
         /* and add a key back to the parent */
         bdata = b4key( parent, parent->key_on ) ;
         if ( balance_mode == 2 )
         {
            if ( is_right )
               bdata2 = b4key( b4temp, b4temp->n_keys - 1 ) ;
            else
               bdata2 = b4key( b4, b4->n_keys ) ;
            memcpy( bdata->value, bdata2->value, t4->header.key_len )  ;
            memcpy( &(bdata->num), &bdata2->num, sizeof( long ) )  ;
            if ( is_right )
            {
               b4temp->key_on = b4temp->n_keys ;
               b4remove( b4temp ) ;
            }
         }
         else
         {
            bdata2 = b4key( b4temp, b4temp->key_on ) ;
            memcpy( bdata->value, bdata2->value, t4->header.key_len )  ;
            memcpy( &(bdata->num), &bdata2->num, sizeof( long ) )  ;
            b4remove( b4temp ) ;
         }
         parent->changed = 1 ;
         b4flush( b4temp ) ;
      }
   }
   return 0 ;
}

/* if do_full is true, the whole branch set will be balanced, not just the current leaf block and reqd.  Also, this will balance top down instead of the other way. */
int S4FUNCTION t4balance( TAG4 *t4, B4BLOCK *b4, int do_full )
{
   B4BLOCK *b4temp, *parent ;
   long temp_fb ;
   int rc ;
   char is_right, balance_mode ;  /* balance_mode = 0 for done, 1 for leaf, and 2 for branch */

   if ( !b4leaf( b4 ) )
      return 0 ;

   b4temp = b4alloc( t4, 0L ) ;
   if ( b4temp == 0 )
      return -1 ;

   if ( do_full )
   {
      t4up_to_root( t4 ) ;
      balance_mode = 2 ;  /* branch */
   }
   else
      balance_mode = 1 ;  /* leaf */

   while ( balance_mode != 0 || do_full )
   {
      if ( do_full )
      {
         parent = (B4BLOCK *)t4->blocks.last_node ;
         parent->key_on = parent->n_keys ;
         rc = t4down( t4 ) ;
         if ( rc == 1 )
            break ;
         if ( rc < 0 )
            return -1 ;

         b4 = (B4BLOCK *)t4->blocks.last_node ;
         b4->key_on = b4->n_keys ;

         if ( b4->n_keys >= t4->header.keys_half )
            continue ;

         if ( b4leaf( b4 ) )
            balance_mode = 1 ;
         temp_fb = b4key( parent, parent->key_on - 1 )->pointer ;
         is_right = 1 ;
      }
      else
      {
         if ( balance_mode == 2 )  /* branch */
         {
            b4 = parent ;
            while( t4->blocks.last_node != (LINK4 *)b4 )  /* re-align ourselves */
               t4up( t4 ) ;
         }
         t4up( t4 ) ;
         parent = (B4BLOCK *)t4->blocks.last_node ;

         if ( parent == 0 )  /* root block */
         {
            t4down( t4 ) ;
            parent = (B4BLOCK *)t4->blocks.last_node ;
            if ( !b4leaf( parent ) && parent->n_keys == 0 )  /* remove */
            {
               temp_fb = (long)b4key( parent, 0 )->pointer ;
               t4shrink( t4, parent->file_block ) ;
               t4up( t4 ) ;
               l4pop( &t4->saved ) ;
               parent->changed = 0 ;
               b4free( parent ) ;
               t4->header.root = temp_fb ;
               t4down( t4 ) ;
            }
            break ;
         }

         if ( parent->n_keys == 0 )  /* try to replace parent with ourself */
         {
            #ifdef S4DEBUG
               e4severe( e4info, E4_INFO_CIF ) ;
            #endif
            return -1 ;
         }

         if ( b4->n_keys >= t4->header.keys_half )
         {
            balance_mode = 0 ;
            continue ;
         }
         is_right = (parent->key_on == parent->n_keys ) ;
         if ( is_right )
            temp_fb = b4key( parent, parent->key_on - 1 )->pointer ;
         else
            temp_fb = b4key( parent, parent->key_on + 1 )->pointer ;
      }

      b4temp->file_block = temp_fb ;
      for ( ;; )
      {
         if ( i4read_block( &t4->file, temp_fb, 0, b4temp ) < 0 )
         {
            b4free( b4temp ) ;
            return -1 ;
         }

         if ( balance_mode == 2 || b4leaf( b4temp ) )  /* if branch mode or leaf mode and leaf block found */
            break ;

         if ( is_right )
            temp_fb = b4key( b4temp, b4temp->n_keys )->pointer ;
         else
            temp_fb = b4key( b4temp, 0 )->pointer ;
      }

      if ( t4balance_block( t4, parent, b4, b4temp, is_right, balance_mode ) < 0 )
         return -1 ;

      if ( do_full )
      {
         if ( b4leaf( b4 ) )
            balance_mode = 0 ;
      }
      else
      {
         if ( parent->n_keys < t4->header.keys_half )  /* do parent as well */
            balance_mode = 2 ;
         else
            balance_mode = 0 ;
      }
   }


   b4free( b4temp ) ;
   return 0 ;
}

int S4FUNCTION t4get_replace_entry( TAG4 *t4, B4KEY_DATA *insert_spot, B4BLOCK *block_on )
{
   int rc ;

   if ( b4leaf( block_on ) )
      return 0 ;

   block_on->key_on = block_on->key_on + 1 ;
   while ( !b4leaf( block_on ) )
   {
      rc = t4down( t4 ) ;
      if ( rc < 0 || rc == 2 )
         return -1 ;
      block_on = (B4BLOCK *)t4->blocks.last_node ;
   }
   memcpy( &insert_spot->num, &(b4key( block_on, block_on->key_on )->num), sizeof(long) + t4->header.key_len ) ;
   b4remove( block_on ) ;
   if ( block_on->n_keys < t4->header.keys_half && block_on->file_block != t4->header.root )  /* if not root may have to balance the tree */
      if ( t4balance( t4, block_on, 0 ) < 0 )
         return -1 ;
   return 1 ;
}

int S4FUNCTION t4balance_branch( TAG4 *t4, B4BLOCK *b4 )
{
   B4BLOCK *parent, *b4temp ;
   int is_right ;
   long temp_fb, reference ;

   #ifdef S4DEBUG
      if ( b4leaf( b4 ) )
         e4severe( e4info, E4_T4BALANCE_BR ) ;
   #endif

   if ( b4 == (B4BLOCK *)l4first( &t4->blocks ) )
   {
      if ( b4->n_keys == 0 )   /* empty, so just remove */
      {
         reference = (long)b4key( b4, 0 )->pointer ;
         b4->changed = 0 ;
         t4shrink( t4, b4->file_block ) ;
         t4up( t4 ) ;
         l4pop(&t4->saved ) ;
         t4->header.root = reference ;
      }
      return 0 ;
   }

   #ifdef S4DEBUG
      if (  b4->n_keys < 2 )
         e4severe( e4info, E4_T4BALANCE_BR ) ;
   #endif

   t4up( t4 ) ;
   parent = (B4BLOCK *)t4->blocks.last_node ;

   #ifdef S4DEBUG
      if ( b4key( parent, parent->key_on )->pointer != I4MULTIPLY * b4->file_block )
         e4severe( e4info, E4_T4BALANCE_BR ) ;
   #endif

   if ( parent->key_on == parent->n_keys )
   {
      temp_fb = b4key( parent, parent->key_on - 1 )->pointer ;
      is_right = 1 ;
   }
   else
   {
      temp_fb = b4key( parent, parent->key_on + 1 )->pointer ;
      is_right = 0 ;

   }

   b4temp = b4alloc( t4, 0L ) ;
   if ( b4temp == 0 )
      return -1 ;

   if ( i4read_block( &t4->file, temp_fb, 0, b4temp ) < 0 )
   {
      b4free( b4temp ) ;
      return -1 ;
   }

   if ( t4balance_block( t4, parent, b4, b4temp, is_right, 2 ) < 0 )
      return -1 ;

   b4free( b4temp ) ;
   return 0 ;
}

int S4FUNCTION t4remove_current( TAG4 *t4 )
{
   B4BLOCK *block_on ;

   t4->header.version = (short)( t4->header.old_version + 1L ) ;

   block_on = (B4BLOCK *)t4->blocks.last_node ;

   #ifdef S4DEBUG
      if ( b4lastpos( block_on ) == -b4leaf( block_on ) )
         e4severe( e4info, E4_INFO_UNE ) ;
   #endif

   switch( t4get_replace_entry( t4, b4key( block_on, block_on->key_on ), block_on ) )
   {
      case 0 :   /* leaf delete */
         b4remove( block_on ) ;
         if ( block_on->n_keys == 0 )  /* last entry deleted! -- remove upward reference */
         {
            if ( t4remove_ref( t4 ) != 0 )
               return -1 ;
         }
         else
         {
            if ( block_on->n_keys < t4->header.keys_half && block_on->file_block != t4->header.root )  /* if not root may have to balance the tree */
               return t4balance( t4, block_on, 0 ) ;
         }
         break ;

      case 1 :   /* branch delete */
         block_on->changed = 1 ;  /* mark as changed for all entries */
         if( t4block( t4 )->n_keys == 0 )    /* removed the last key of */
            if ( t4remove_ref( t4 ) != 0 )
               return -1 ;
         break ;

      default:
         e4severe( e4info, E4_INFO_INT ) ;
         break ;
   }
   return 0 ;
}
#endif  /* S4CLIPPER */
#endif  /* S4NDX */
#endif  /* N4OTHER */

#endif  /* S4INDEX_OFF */
#endif  /* S4OFF_WRITE */
