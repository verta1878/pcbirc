/* i4addtag.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

#ifndef S4OFF_WRITE

#ifdef S4NDX
int S4FUNCTION i4get_last_key( TAG4 *t4, char *key_data, long for_block )
{
   int rc = 0 ;
   B4BLOCK *temp_block ;

   #ifdef S4DEBUG
      if ( for_block <= 0 || t4 == 0 || key_data == 0 )
         e4severe( e4parm, E4_I4GET_LAST_KEY ) ;
   #endif

   /* must get block for_block, then find the last key */
   temp_block = b4alloc( t4, for_block ) ;
   if ( temp_block == 0 )
      return -1 ;

   #ifdef S4NDX
      if ( file4read_all( &t4->file, I4MULTIPLY * for_block, &temp_block->n_keys, B4BLOCK_SIZE ) < 0 )
   #else
      if ( file4read_all( &t4->index->file, I4MULTIPLY * for_block, &temp_block->n_keys, t4->index->header.block_rw ) < 0 )
   #endif
      return -1 ;

   b4go_eof( temp_block ) ;
   if ( b4leaf( temp_block ) )
      rc = 2 ;
   else
      rc = b4get_last_key( temp_block, key_data ) ;

   b4free( temp_block ) ;

   return rc ;
}
#endif

#ifndef N4OTHER

#ifdef S4FOX
/* (temporary) fix for FoxPro multi-user compatibility 
   swaps parent and right blocks */
#ifndef S4SINGLE
static long t4swap( B4BLOCK *parent, B4BLOCK *left )
{
   long temp_fb ;

   temp_fb = left->file_block ;
   left->file_block = parent->file_block ;
   parent->file_block = temp_fb ;

   /* now update neighbours */
   if ( left->header.right_node != -1 )
      file4write( &parent->tag->index->file, left->header.right_node + 2*sizeof(short),
                  &left->file_block, sizeof( left->header.left_node ) ) ;

   if ( left->header.left_node != -1 )
      file4write( &parent->tag->index->file, left->header.left_node + 2*sizeof(short),
                  &left->file_block, sizeof( left->header.right_node ) ) ;

   if ( parent->header.right_node != -1 )
      file4write( &parent->tag->index->file, parent->header.right_node + 2*sizeof(short),
                  &parent->file_block, sizeof( parent->header.left_node ) ) ;

   if ( parent->header.left_node != -1 )
      file4write( &parent->tag->index->file, parent->header.left_node + 2*sizeof(short),
                  &parent->file_block, sizeof( parent->header.right_node ) ) ;

   return left->file_block ;
}
#endif
#endif

int S4FUNCTION t4add( TAG4 *t4, unsigned char *key_info, long rec )
{
   CODE4 *c4 ;
   INDEX4 *i4 ;
   B4BLOCK *old_block, *root_block, *new_block ;
   int rc ;
   long  old_file_block, extend_block ;
   #ifdef S4FOX
      int  key_on ;
      long rec1 = 0L ;
      long rec2 = 0L ;
      unsigned char *temp_key ;
      int do_insert, update_reqd ;
   #else
      int is_branch ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 || key_info == 0 || rec < 1 )
         e4severe( e4parm, E4_T4ADD ) ;
   #endif

   c4 = t4->code_base ;
   i4 = t4->index ;

   if ( c4->error_code < 0 )
      return -1 ;

   #ifdef S4FOX
      rc = t4go( t4, (char *)key_info, rec ) ;
   #else
      rc = t4seek( t4, (char *)key_info, t4->header.key_len ) ;
   #endif
   if ( rc < 0 )
      return -1 ;

   #ifdef S4FOX
      if ( rc == 0 && t4->unique_error == e4unique )
      {
         e4( c4, e4unique, i4->file.name ) ;
         return e4unique ;
      }
   #endif

   #ifdef S4FOX
      if ( (t4->header.type_code & 0x01) && rc == r4found )
   #else
      if ( t4->header.unique && rc == 0 )
   #endif
      {
         switch ( t4->unique_error )
         {
            case e4unique:
               return e4( c4, e4unique, i4->file.name ) ;
            case r4unique:
               return r4unique ;
            case r4unique_continue:
               return r4unique_continue ;
            default:
               break ;
         }
      }

   if ( t4->filter && !t4->has_keys )
   {
      file4write(&t4->index->file, t4->header_offset+sizeof(t4->header)+222, (char *) "\0", (int) 1 ) ;
      t4->has_keys = (char)1 ;
      #ifdef S4MDX
         t4->had_keys = (char)0 ;
      #endif
   }

   old_block = t4block(t4) ;
   old_file_block = 0 ;

   #ifdef S4FOX
      do_insert = 1 ;
      update_reqd = 0 ;

      for( ;; )
      {
         if ( do_insert == 1 )
         {
            i4->tag_index->header.version = i4->version_old+1 ;
            if ( old_block == 0 )
            {
               /* Must create a new root block */
               extend_block = i4extend(i4) ;
               if ( extend_block < 0 )
                  return (int)extend_block ;

               root_block = b4alloc( t4, extend_block) ;
               if ( root_block == 0 )
                  return -1 ;

               root_block->header.left_node = -1 ;
               root_block->header.right_node = -1 ;
               root_block->header.node_attribute = 1 ;

               l4add( &t4->blocks, root_block ) ;

               #ifndef S4SINGLE
                  if ( t4->index->file.is_exclusive == 0 )
                  {
                     old_file_block = t4swap( root_block, (B4BLOCK *)l4last( &t4->saved ) ) ;
                     if ( old_file_block < 0 )
                        return -1 ;
                  }

               #endif

               b4top( root_block ) ;
               b4insert( root_block, temp_key, rec, rec2, 1 ) ;
               b4insert( root_block, key_info, old_file_block, rec1, 1 ) ;
               rec1 = 0L ;
               rec2 = 0L ;
               t4->header.root = root_block->file_block ;
               t4->root_write  = 1 ;
               return 0 ;
            }

            if ( (rc = b4insert( old_block, key_info, rec, rec1, 0 )) != 1 )
            {
               if ( rc == 0 )
               {
                  if ( b4leaf( old_block ) )
                  {
                     temp_key = key_info ;
                     rec2 = rec ;
                  }
                  if ( old_block->key_on == old_block->header.n_keys - 1 )
                     update_reqd = 1 ;
                  do_insert = 0 ;
                  continue ;
               }
               else
                  return rc ;
            }
            else
            {
               l4pop( &t4->blocks ) ;
               key_on = old_block->key_on ;

               /* The new block's end key gets added to the block just up */
               new_block= t4split( t4, old_block ) ;
               if ( new_block == 0 )
                  return -1 ;

               l4add( &t4->saved, old_block ) ;

               if ( key_on < old_block->header.n_keys )
               {
                  b4go( old_block, key_on ) ;
                  b4insert( old_block, key_info, rec, rec1, 0 ) ;
               }
               else
               {
                  b4go( new_block, key_on - old_block->header.n_keys ) ;
                  b4insert( new_block, key_info, rec, rec1, 0 ) ;
                  if ( new_block->key_on == new_block->header.n_keys - 1 )
                     update_reqd = 1 ;
               }

               #ifdef S4INDEX_VERIFY
                  if ( b4verify( old_block ) == -1 )
                     e4describe( t4->code_base, e4index, t4->alias, "t4split()-old after split", "" ) ;
                  if ( b4verify( new_block ) == -1 )
                     e4describe( t4->code_base, e4index, t4->alias, "t4split()-new after split", "" ) ;
               #endif

               /* Now add to the block just up */
               b4go_eof( old_block ) ;
               old_block->key_on-- ;

               key_info = b4key_key( old_block, old_block->key_on ) ;
               old_file_block = old_block->file_block ;
               rec1 = b4recno( old_block, old_block->key_on ) ;

               rec = new_block->file_block ;
               if ( b4flush(new_block) < 0 )
                  return -1 ;

               b4go_eof( new_block ) ;
               new_block->key_on-- ;

               temp_key = (unsigned char *)c4->saved_key ;
               memcpy( temp_key, b4key_key( new_block, new_block->key_on ), t4->header.key_len ) ;
               rec2 = b4recno( new_block, new_block->key_on ) ;
               if( new_block->key_on == new_block->header.n_keys - 1 )
                  update_reqd = 1 ;
               b4free( new_block ) ;
            }
         }
         else
            l4add( &t4->saved, l4pop( &t4->blocks ) ) ;

         old_block = (B4BLOCK *)t4->blocks.last_node ;

         if ( old_block == 0 )
         {
            if ( do_insert == 0 )
               return 0 ;
         }
         else
            if ( update_reqd )  /* may have to update a parent block */
            {
               b4br_replace( old_block, (char *)temp_key, rec2 ) ;
               if ( old_block->key_on != old_block->header.n_keys - 1 )  /* done reqd updates */
                  update_reqd = 0 ;
            }
      }
   #else              /* if not S4FOX  */
      i4->changed = 1 ;
      t4->changed = 1 ;
      t4->header.version++ ;

      for(;;)
      {
         if ( old_block == 0 )
         {
            /* Must create a new root block */
            extend_block = i4extend(i4) ;
            if ( extend_block < 0 )
               return (int) extend_block ;

            root_block = b4alloc( t4, extend_block) ;
            if ( root_block == 0 )
               return -1 ;

            l4add( &t4->blocks, root_block ) ;

            b4insert( root_block, key_info, old_file_block ) ;
            b4insert( root_block, key_info, rec ) ;
            root_block->n_keys-- ;
            t4->header.root = root_block->file_block ;
            t4->root_write  = 1 ;
            return 0 ;
         }

         if ( old_block->n_keys < old_block->tag->header.keys_max )
         {
            b4insert( old_block, key_info, rec ) ;
            return 0 ;
         }

         l4pop( &t4->blocks ) ;

         is_branch  = b4leaf( old_block )  ?  0 : 1 ;

         /* NNNNOOOO  N - New, O - Old */
         /* The new block's end key gets added to the block just up */
         new_block= t4split( t4, old_block ) ;
         if ( new_block == 0 )
            return -1 ;

         l4add( &t4->saved, new_block ) ;

         new_block->n_keys -= is_branch ;
         if ( new_block->key_on < (new_block->n_keys+is_branch) )
            b4insert( new_block, key_info, rec ) ;
         else
            b4insert( old_block, key_info, rec ) ;

         #ifdef S4INDEX_VERIFY
            if ( b4verify( old_block ) == -1 )
               e4describe( t4->code_base, e4index, t4->alias, "t4split()-old after split", "" ) ;
            if ( b4verify( new_block ) == -1 )
               e4describe( t4->code_base, e4index, t4->alias, "t4split()-new after split", "" ) ;
         #endif

         /* Now add to the block just up */
         new_block->key_on = b4lastpos(new_block) ;

         key_info = b4key_key( new_block, new_block->key_on ) ;
         rec      = new_block->file_block ;

         old_file_block = old_block->file_block ;
         if ( b4flush(old_block) < 0 )
            return -1 ;

         b4free( old_block ) ;
         old_block = (B4BLOCK *) t4->blocks.last_node ;
      }
   #endif
}

int S4FUNCTION t4add_calc( TAG4 *t4, long rec )
{
   int len ;
   char *ptr ;

   #ifdef S4DEBUG
      if ( t4 == 0 || rec < 1 )
         e4severe( e4parm, E4_T4ADD_CALC ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   if ( t4->filter )
      if ( !expr4true( t4->filter ) )
         return 0;

   len = t4expr_key( t4, &ptr ) ;
   if ( len < 0 )
      return -1 ;

   #ifdef S4DEBUG
      if ( len != t4->header.key_len )
         e4severe( e4result, E4_T4ADD_CALC ) ;
   #endif

   return t4add( t4, (unsigned char *)ptr, rec ) ;
}

#endif  /*  ifndef N4OTHER  */


#ifdef N4OTHER

#ifdef S4NDX
int S4FUNCTION t4add( TAG4 *t4, unsigned char *key_info, long rec )
{
   CODE4 *c4 ;
   INDEX4 *i4 ;
   B4BLOCK *old_block, *root_block, *new_block, *wch_block ;
   int rc, is_branch ;
   long  old_file_block, extend_block ;
   char key_data[I4MAX_KEY_SIZE+1] ;    /* temporary storage for the key data (max size 100) */
   char key2_data[I4MAX_KEY_SIZE+1] ;

   #ifdef S4DEBUG
      if ( t4 == 0 || key_info == 0 || rec < 1 )
         e4severe( e4parm, E4_T4ADD ) ;
   #endif

   c4 = t4->code_base ;
   i4 = t4->index ;

   if ( c4->error_code < 0 )
      return -1 ;

   rc = t4go( t4, (char *)key_info, rec ) ;
   if ( rc < 0 )
      return -1 ;

   if ( rc == 0 && t4->unique_error == e4unique )
   {
      e4( c4, e4unique, i4->file.name ) ;
      return e4unique ;
   }

   if ( rc == r4found || rc == 0 )
   {
      switch ( t4->unique_error )
      {
         case e4unique:
            return e4( c4, e4unique, i4->file.name ) ;
         case r4unique:
            return r4unique ;
         case r4unique_continue:
            return r4unique_continue ;
         default:
            break ;
      }
   }

   old_block = t4block( t4 ) ;
   old_file_block = 0 ;

   t4->header.version = t4->header.old_version + 1 ;

   for(;;)
   {
      if ( old_block == 0 )
      {
         /* Must create a new root block */
         extend_block = t4extend( t4 ) ;
         if ( extend_block < 0 )
            return (int)extend_block ;

         root_block = b4alloc( t4, extend_block ) ;
         if ( root_block == 0 )
            return -1 ;

         l4add( &t4->blocks, root_block ) ;

         i4get_last_key( t4, key_data, old_file_block ) ;

         b4insert( root_block, key_data, 0L, old_file_block ) ;
         b4append( root_block, rec ) ;
         t4->header.root = root_block->file_block ;
         t4->root_write  = 1 ;
         return 0 ;
      }

      if ( b4room( old_block ) )
      {
         if ( b4leaf( old_block ) )
            b4insert( old_block, key_info, rec, 0L ) ;
         else
         {
            b4get_last_key( old_block, key_data ) ;
            if ( old_block->key_on >= old_block->n_keys  )  /*insert at end done different */
            {
               b4insert( old_block, key_data, 0L, old_file_block ) ;
               b4append( old_block, rec ) ;
            }
            else
            {
               b4remove( old_block ) ;
               b4insert( old_block, key_info, 0L, rec ) ;
               b4insert( old_block, key_data, 0L, old_file_block ) ;
            }
         }
         return 0 ;
      }

      l4pop( &t4->blocks ) ;
      is_branch  = b4leaf( old_block )  ?  0 : 1 ;

      /* NNNNOOOO  N - New, O - Old */
      /* The new block's end key gets added to the block just up */
      new_block= t4split( t4, old_block ) ;
      if ( new_block == 0 )
         return -1 ;

      l4add( &t4->saved, new_block ) ;

      /* which block should do the insertion ? */
      if ( old_block->key_on < (old_block->n_keys + is_branch) )
         wch_block = old_block ;
      else wch_block = new_block ;

      if ( b4leaf( wch_block ) )
         b4insert( wch_block, key_info, rec, 0L ) ;
      else
      {
         b4get_last_key( wch_block, key_data ) ;
         if ( wch_block->key_on >= wch_block->n_keys )  /* insert at end done different */
         {
            b4insert( wch_block, key_data, 0L, old_file_block ) ;
            b4append( wch_block, rec ) ;
         }
         else
         {
            b4remove( wch_block ) ;
            b4insert( wch_block, key_info, 0L, rec ) ;
            b4insert( wch_block, key_data, 0L, old_file_block ) ;
         }
      }

      #ifdef S4INDEX_VERIFY
         if ( b4verify( old_block ) == -1 )
            e4describe( t4->code_base, e4index, t4->alias, "t4split()-old after split", "" ) ;
         if ( b4verify( new_block ) == -1 )
            e4describe( t4->code_base, e4index, t4->alias, "t4split()-new after split", "" ) ;
      #endif

      /* Now add to the block just up */
      new_block->key_on = b4lastpos(new_block) ;

      key_info = b4key_key( new_block, new_block->key_on ) ;
      rec = new_block->file_block ;

      if( !b4leaf( new_block ) )
         if ( b4get_last_key( new_block, key2_data ) != 4)
            key_info = (unsigned char *)key2_data ;

      old_block->key_on = b4lastpos( old_block ) ;
      memcpy( key_data, b4key_key( old_block, old_block->key_on ), t4->header.key_len ) ;

      old_file_block = old_block->file_block ;
      if ( b4flush( old_block ) < 0 )
         return -1 ;
      b4free( old_block ) ;
      old_block = (B4BLOCK *) t4->blocks.last_node ;
   }
}
#else
#ifdef S4CLIPPER
int S4FUNCTION t4add( TAG4 *t4, unsigned char *key_info, long rec )
{
   CODE4 *c4 ;
   INDEX4 *i4 ;
   B4BLOCK *old_block, *root_block, *new_block, *wch_block ;
   int rc, is_branch ;
   long  old_file_block, extend_block, new_file_block ;
   B4KEY_DATA *at_new ;
   unsigned char old_key_ptr[ I4MAX_KEY_SIZE ] ;
   unsigned char key_data[I4MAX_KEY_SIZE+1] ;    /* temporary storage for the key data (max size 100) */

   #ifdef S4DEBUG
      if ( t4 == 0 || key_info == 0 || rec < 1 )
         e4severe( e4parm, E4_T4ADD ) ;
   #endif

   c4 = t4->code_base ;
   i4 = t4->index ;
   if ( c4->error_code < 0 )
      return -1 ;

   rc = t4seek( t4, (char *)key_info, t4->header.key_len ) ;
   if ( rc < 0 )
      return -1 ;

   if ( rc == 0 )
   {
      switch ( t4->unique_error )
      {
         case e4unique:
            return e4( c4, e4unique, i4->file.name ) ;
         case r4unique:
            return r4unique ;
         case r4unique_continue:
            return r4unique_continue ;
         default:
            break ;
      }
   }

   old_block = t4block(t4) ;
   old_file_block = 0 ;
   new_file_block = 0 ;

   t4->header.version = (short)(t4->header.old_version + 1L) ;

   while( !b4leaf( old_block ) )
   {
      rc = t4down( t4 ) ;
      if ( rc < 0 || rc == 2 )
         return -1 ;
      old_block = t4block( t4 ) ;
      if ( b4leaf( old_block ) )
         old_block->key_on = b4lastpos( old_block ) + 1 ;
      else
         old_block->key_on = b4lastpos( old_block ) ;
   }

   for(;;)
   {
      if ( old_block == 0 )
      {
         /* Must create a new root block */
         extend_block = t4extend( t4 ) ;
         if ( extend_block < 0 )
            return (int) extend_block ;

         root_block = b4alloc( t4, extend_block) ;
         if ( root_block == 0 )
            return -1 ;

         l4add( &t4->blocks, root_block ) ;

         b4insert( root_block, key_info, rec, old_file_block ) ;
         t4->header.root = root_block->file_block * 512 ;  /* line order here so that the index validation works */
         b4append( root_block, new_file_block ) ;

         t4->root_write  = 1 ;
         return 0 ;
      }

      if ( b4room( old_block ) )
      {
         if ( b4leaf( old_block ) )
            b4insert( old_block, key_info, rec, 0L ) ;
         else   /* update the current pointer, add the new branch */
         {
            #ifdef S4DEBUG
               if ( old_block->n_keys == 0 )
                  e4severe( e4info, E4_T4ADD ) ;
               /*
                  {
                     b4insert( old_block, key_info, rec, old_file_block ) ;
                     at_new = b4key( old_block, 1 ) ;
                     at_new->pointer = new_file_block * 512 ;
                  }
                  else
                  {
                     ...
                  }
               */
            #endif
               at_new = b4key( old_block, old_block->key_on ) ;
               at_new->pointer = new_file_block * 512 ;
               b4insert( old_block, key_info, rec, old_file_block ) ;
         }
         return 0 ;
      }

      l4pop( &t4->blocks ) ;
      is_branch  = b4leaf( old_block )  ?  0 : 1 ;

      /* NNNNOOOO  N - New, O - Old */
      /* The new block's end key gets added to the block just up */
      if ( old_block->key_on < (t4->header.keys_half + is_branch) )
      {
         new_block= t4split( t4, old_block, 0 ) ;
         if ( new_block == 0 )
            return -1 ;
         wch_block = old_block ;
      }
      else
      {
         new_block= t4split( t4, old_block, 1 ) ;
         if ( new_block == 0 )
            return -1 ;
         wch_block = new_block ;
      }

      if ( b4leaf( wch_block ) )
      {
         b4insert( wch_block, key_info, rec, 0L ) ;
         if ( new_block->n_keys <= t4->header.keys_half )   /* add a key from the old block!, must have info in new_block because old_block gets deleted below */
         {
            #ifdef S4DEBUG
               if ( old_block->n_keys <= t4->header.keys_half )  /* impossible */
                  e4severe( e4info, E4_INFO_CIF ) ;
            #endif
            old_block->key_on = old_block->n_keys - 1 ;
            memcpy( key_data, b4key_key( old_block, old_block->key_on ), t4->header.key_len ) ;
            key_info = key_data ;
            rec = b4key( old_block, old_block->key_on )->num ;
            b4remove( old_block ) ;
            new_block->key_on = 0 ;
            b4insert( new_block, key_info, rec, 0 ) ;
         }
         new_block->key_on = 0 ;
         memcpy( key_data, b4key_key( new_block, new_block->key_on ), t4->header.key_len ) ;
         key_info = key_data ;
         rec = b4key( new_block, new_block->key_on )->num ;
         b4remove( new_block ) ;
      }
      else
      {
         /* now get the key to place upwards */
         if ( wch_block->key_on > wch_block->n_keys && wch_block == old_block )
         {
            at_new = b4key( new_block, 0 ) ;
            at_new->pointer = new_file_block * 512 ;
         }
         else
         {
            at_new = b4key( wch_block, wch_block->key_on ) ;
            at_new->pointer = new_file_block * 512 ;
         }
         b4insert( wch_block, key_info, rec, old_file_block ) ;
         memcpy( old_key_ptr, b4key_key( old_block, b4lastpos( old_block )), t4->header.key_len ) ;
         key_info = old_key_ptr ;
         rec = b4key( old_block, b4lastpos( old_block ) )->num ;
      }

      #ifdef S4INDEX_VERIFY
         if ( b4verify( old_block ) == -1 )
            e4describe( t4->code_base, e4index, t4->alias, "t4split()-old after split", "" ) ;
         if ( b4verify( new_block ) == -1 )
            e4describe( t4->code_base, e4index, t4->alias, "t4split()-new after split", "" ) ;
      #endif

      l4add( &t4->saved, new_block ) ;
      new_file_block = new_block->file_block ;
      old_file_block = old_block->file_block ;
      if ( b4flush( old_block ) < 0 )
         return -1 ;
      b4free( old_block ) ;
      old_block = (B4BLOCK *) t4->blocks.last_node ;
   }
}
#endif  /* ifdef S4CLIPPER   */
#endif  /* ifdef S4NDX    */

int S4FUNCTION t4add_calc( TAG4 *t4, long rec )
{
   char *ptr ;

   #ifdef S4DEBUG
      if ( t4 == 0 || rec < 1 )
         e4severe( e4parm, E4_T4ADD_CALC ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifdef S4CLIPPER
      if ( t4->filter )
         if ( !expr4true( t4->filter ) )
            return 0;
   #endif

   if ( t4expr_key( t4, &ptr ) < 0 )
      return -1 ;

   return t4add( t4, (unsigned char *)ptr, rec ) ;
}

#endif  /* ifdef N4OTHER */
#endif  /* S4OFF_WRITE */
#endif  /* S4INDEX_OFF */
