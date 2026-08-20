/* i4tag.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

/*#ifndef S4LANGUAGE */
/*#ifndef u4memcmp */
/*int S4CALL u4memcmp( S4CMP_PARM p1, S4CMP_PARM p2, size_t len ) */
/*{ */
/*   return memcmp( p1, p2, len ) ; */
/*} */
/*#endif */
/*#endif */

#ifndef S4INDEX_OFF

#include <time.h>

void S4FUNCTION t4out_of_date( TAG4 *t4 )
{
   #ifdef S4SINGLE
      #ifdef S4DEBUG_DEV
         e4describe( t4->code_base, e4index, t4->alias, "t4out_of_date()", "index file can't be out of date with S4SINGLE, so corrupt" ) ;
      #endif
      e4( t4->code_base, e4index, t4->alias ) ;
   #else
      time_t old_time ;

      /* first make sure we are at a potential read conflict situation (otherwise must be corrupt file) */
      #ifdef N4OTHER
         if ( t4->file.is_exclusive || t4->file.is_read_only ||
              t4->file_locked == 1 )
      #else
         if ( t4->index->file.is_exclusive || t4->index->file.is_read_only ||
              t4->index->file_locked == 1 )
      #endif
      {
         #ifdef S4DEBUG_DEV
            e4describe( t4->code_base, e4index, t4->alias, "t4out_of_date()", "index file can't be out of date since is exclusive, locked, or dos read-only" ) ;
         #endif
         e4( t4->code_base, e4index, t4->alias ) ;
      }

      /* wait a second and refresh */
      time( &old_time) ;
      while ( time( (time_t *)0 ) <= old_time) ;
      #ifdef N4OTHER
         file4refresh( &t4->file ) ;
      #else
         file4refresh( &t4->index->file ) ;
      #endif
   #endif
}

/* uses parent to determine if an inconsistancy has arisen */
/* return -1 = error, 0 = success, 1 = inconsistant */
int S4FUNCTION i4read_block( FILE4 *file, long block_no, B4BLOCK *parent, B4BLOCK *block )
{
   int rc ;
   TAG4 *tag ;
   #ifdef S4CLIPPER
      #ifdef S4INDEX_VERIFY
         int i, j ;
      #endif
   #endif

   #ifndef S4FOX
   #ifndef S4CLIPPER
   /* i.e. if ndx or mdx */
      int par_on, blk_on, eq ;
      #ifdef S4MDX
         if ( file4read_all( file, I4MULTIPLY * block_no, &block->n_keys, block->tag->index->header.block_rw ) < 0 )
            return -1 ;
      #endif
      #ifdef S4NDX
         if ( file4read_all( file, I4MULTIPLY * block_no, &block->n_keys, B4BLOCK_SIZE ) < 0 )
            return -1 ;
         block->file_block = block_no ;
      #endif
   #endif
   #endif

   rc = 0 ;
   tag = block->tag ;

   #ifdef S4FOX
      if ( file4read_all( file, I4MULTIPLY * block_no, &block->header, B4BLOCK_SIZE ) < 0 )
         return -1 ;
   #endif
   #ifdef S4CLIPPER
      if ( file4read_all( file, block_no, &block->n_keys, B4BLOCK_SIZE ) < 0 )
         return -1 ;
      block->file_block = block_no/512 ;
   #endif

   if ( tag->code_base->do_index_verify == 0 )
      return rc ;

   #ifdef N4OTHER
      if ( block->n_keys == 0 )
      #ifdef S4NDX
         if ( tag->header.root != block->file_block )  /* block has no keys but is not the root, must be a problem... */
      #endif
      #ifdef S4CLIPPER
         if ( tag->header.root != block->file_block * I4MULTIPLY )  /* block has no keys but is not the root, must be a problem... */
      #endif
   #else
      #ifdef S4FOX
         if ( block->header.n_keys == 0 )   /* added to free list... therefore bad */
            if ( tag->header.root != block_no )  /* block has no keys but is not the root, must be a problem... */
      #else
         if ( block->n_keys == 0 )   /* added to free list... therefore bad */
            if ( tag->header.root != block_no )  /* block has no keys but is not the root, must be a problem... */
      #endif
   #endif
      return 1 ;

   if ( parent != 0 )  /* check consistancy */
   {
      #ifndef S4FOX
      #ifndef S4CLIPPER
      /* i.e. if ndx or mdx */
         eq = 0 ;
         blk_on = block->n_keys - 1 ;
         if ( parent->key_on >= parent->n_keys )
         {
            par_on = parent->n_keys - 1 ;
            eq = 1 ;
         }
         else
            par_on = parent->key_on ;
         if ( !b4leaf( block ) )
         {
            blk_on = 0 ;
            if ( eq != 1 )
            {
               if ( par_on == 0 )
                  eq = 2 ;
               else
               {
                  eq = 1 ;
                  par_on-- ;
               }
            }
         }

         switch( eq )
         {
            case 0:
               if ( tag->cmp( b4key_key( parent, par_on ), b4key_key( block, blk_on ), tag->header.key_len  ) != 0 )
                  rc = 1 ;
               break ;
            case 1:
               if ( tag->cmp( b4key_key( parent, par_on ), b4key_key( block, blk_on ), tag->header.key_len  ) > 0 )
                  rc = 1 ;
               break ;
            case 2:
               if ( tag->cmp( b4key_key( parent, par_on ), b4key_key( block, blk_on ), tag->header.key_len  ) < 0 )
                  rc = 1 ;
               break ;
            #ifdef S4DEBUG
               default:
                  e4severe( e4info, E4_I4READ_BLOCK ) ;
            #endif
         }
      #endif
      #endif

      #ifdef S4FOX
         if ( rc == 0 )
               if ( b4recno( parent, parent->key_on) != b4recno( block, block->header.n_keys - 1 ) )
                  rc = 1 ;
      #endif
      if ( rc == 0 )
      {
         #ifdef S4INDEX_VERIFY
            if ( b4verify( block ) == -1 )
               e4describe( block->tag->code_base, e4index, block->tag->alias, "i4read_block()-block", "" ) ;
            if ( b4verify( parent ) == -1 )
               e4describe( parent->tag->code_base, e4index, parent->tag->alias, "i4read_block()-parent", "" ) ;
         #endif
      }
      #ifdef S4CLIPPER
         if ( rc == 0 )
         {
            if ( parent->key_on < parent->n_keys )
            {
               if ( tag->cmp( b4key_key( parent, parent->key_on ), b4key_key( block, block->n_keys - 1 ), tag->header.key_len  ) < 0 )
                  rc = 1 ;
            }
            else
            {
               if ( tag->cmp( b4key_key( parent, parent->key_on - 1 ), b4key_key( block, 0 ), tag->header.key_len  ) > 0 )
                  rc = 1 ;
            }
         }
      #endif
   }

   if ( rc == 1 )
      if ( tag->index->file.is_read_only || tag->index->file.is_exclusive || 
      #ifdef N4OTHER
         tag->file_locked == 1
      #else
         tag->index->file_locked == 1
      #endif
      )  /* corrupt */
      {
         #ifdef S4DEBUG_DEV
            e4describe( tag->code_base, e4index, tag->alias, "t4out_of_date()", "Parent-leaf mismatch, index not out of date since exclusive, locked, or read-only" ) ;
         #endif
         e4( tag->code_base, e4index, E4_INFO_CIF ) ;
      }
   return rc ;
}

int S4FUNCTION t4expr_key( TAG4 *tag, char **ptr_ptr )
{
   int len ;

   #ifdef S4CLIPPER
      int old_dec ;
      old_dec = tag->code_base->decimals ;
      tag->code_base->decimals = tag->header.key_dec ;
   #endif

   len = expr4key( tag->expr, ptr_ptr ) ;

   #ifdef S4CLIPPER
      tag->code_base->decimals = old_dec ;
   #endif

   return len ;
}

int S4FUNCTION t4go( TAG4 *t4, char *ptr, long rec_num )
{
   int rc ;

   #ifdef S4HAS_DESCENDING
      int old_desc ;

      old_desc = t4->header.descending ;
      t4->header.descending = 0 ;
   #endif

   rc = t4go2( t4, ptr, rec_num ) ;

   #ifdef S4HAS_DESCENDING
      t4->header.descending = old_desc ;
   #endif

   return rc ;
}

#ifndef N4OTHER

#ifdef S4FOX
#ifdef S4HAS_DESCENDING
void S4FUNCTION t4descending( TAG4 *tag, int setting )
{
   tag->header.descending = setting ;
}
#endif

void t4str_to_fox( char *result, char *input_ptr, int input_ptr_len )
{
   t4dbl_to_fox( result, c4atod( input_ptr, input_ptr_len ) ) ;
}

void t4dtstr_to_fox( char *result, char *input_ptr, int input_ptr_len )
{
   t4dbl_to_fox( result, (double) date4long( input_ptr ) ) ;
}

static void t4no_change_str( char *a, char *b, int l)
{
   memcpy(a,b,l) ;
}

int S4FUNCTION t4rl_bottom( TAG4 *t4 )
{
   int rc ;
   B4BLOCK *block_on ;

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   do
   {
      rc = t4up_to_root( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( rc != 2 )
      {
         b4go( t4block( t4 ), t4block( t4 )->header.n_keys - 1 ) ;
         do
         {
            rc = t4down( t4 ) ;
            if ( rc < 0 )
               return -1 ;
            b4go( t4block( t4 ), t4block( t4 )->header.n_keys - 1 ) ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         t4out_of_date( t4 ) ;
   } while ( rc == 2 ) ;


   block_on = t4block(t4) ;
   if ( block_on->key_on > 0 )
   {
      b4go_eof( block_on ) ;
      block_on->key_on-- ;  /* update key_on after going to last spot */
   }

   return 0 ;
}

int S4FUNCTION t4bottom( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4BOTTOM ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   if ( t4->header.descending )   /* if descending, go bottom means go top */
      return t4rl_top( t4 ) ;
   else
      return t4rl_bottom( t4 ) ;
}

#else       /*  if not S4FOX  */

int S4FUNCTION t4bottom( TAG4 *t4 )
{
   int rc ;
   B4BLOCK *block_on ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4BOTTOM ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   rc = 2 ;

   while ( rc == 2 )
   {
      rc = t4up_to_root( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( rc != 2 )
      {
         b4go_eof( t4block(t4) ) ;
         do
         {
            rc = t4down( t4 ) ;
            if ( rc < 0 )
               return -1 ;
            b4go_eof( t4block(t4) ) ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         t4out_of_date( t4 ) ;
   }

   block_on = t4block(t4) ;
   if ( block_on->key_on > 0 )
      block_on->key_on = block_on->n_keys-1 ;

   return 0 ;
}

#endif  /* ifdef S4FOX */

B4BLOCK *S4FUNCTION t4block( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4BLOCK ) ;
      if ( t4->blocks.last_node == 0 )
         e4severe( e4info, E4_T4BLOCK ) ;
   #endif
   return (B4BLOCK *)t4->blocks.last_node ;
}

/* Returns  2 - cannot go down due to out of date blocks 1 - Cannot move down; 0 - Success; -1 Error */
int S4FUNCTION t4down( TAG4 *t4 )
{
   long block_down ;
   B4BLOCK *block_on, *pop_block, *new_block, *parent ;
   INDEX4 *i4 ;
   int rc ;
   #ifdef S4BYTE_SWAP
      char *swap_ptr ;
      int i ;
      long long_val ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4DOWN ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   i4 = t4->index ;

   block_on = (B4BLOCK *)t4->blocks.last_node ;

   if ( block_on == 0 )    /* Read the root block */
   {
      if ( t4->header.root <= 0L )
      {
         if ( file4read_all( &i4->file, t4->header_offset, &t4->header.root,sizeof(t4->header.root)) < 0 )
            return -1 ;
         #ifdef S4BYTE_SWAP
            t4->header.root = x4reverse_long( t4->header.root ) ;
         #endif
      }
      block_down = t4->header.root ;
   }
   else
   {
      if ( b4leaf( block_on ) )
         return 1 ;
      #ifdef S4FOX
         block_down = x4reverse_long( *(long *)( ((unsigned char *)&block_on->node_hdr)
                      + (block_on->key_on+1)*(2*sizeof(long) + t4->header.key_len) - sizeof(long) ) ) ;
      #else
         block_down = b4key(block_on,block_on->key_on)->num ;
      #endif
      #ifdef S4DEBUG
         if ( block_down <= 0L )
            e4severe( e4info, E4_INFO_ILF ) ;
      #endif
   }

   /* Get memory for the new block */
   pop_block = (B4BLOCK *)l4pop( &t4->saved ) ;
   if ( pop_block == 0 )
   {
      new_block = b4alloc( t4, block_down) ;
      if ( new_block == 0 )
         return -1 ;
   }
   else
      new_block = pop_block ;

   parent = (B4BLOCK *)l4last( &t4->blocks ) ;
   l4add( &t4->blocks, new_block ) ;

   if ( pop_block == 0 || new_block->file_block != block_down )
   {
      if ( b4flush(new_block) < 0 )
         return -1 ;

      rc = i4read_block( &i4->file, block_down, parent, new_block ) ;

      if ( rc < 0 )
         return rc ;

      if ( rc == 1 )
      {
         l4remove( &t4->blocks, new_block ) ;
         l4add( &t4->saved, new_block ) ;
         return 2 ;
      }

      new_block->file_block = block_down ;

      #ifdef S4BYTE_SWAP
         new_block->n_keys = x4reverse_short( new_block->n_keys ) ;

         /* position swap_ptr at beginning of B4KEY's */
         swap_ptr = (char *)&new_block->n_keys ;
         swap_ptr += 6 + sizeof(short) ;

         /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (int) new_block->n_keys ; i++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *)&long_val, sizeof(long) ) ;
            swap_ptr += new_block->tag->header.group_len ;
         }
         /* mark last_pointer */
         if ( !b4leaf( new_block ) )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *)&long_val, sizeof(long) ) ;
         }
      #endif

      #ifdef S4FOX
         new_block->built_on = -1 ;
      #endif

      #ifdef S4BYTE_SWAP
         new_block->n_keys = x4reverse_short( new_block->n_keys ) ;

         /* position swap_ptr at beginning of B4KEY's */
         swap_ptr = (char *)&new_block->n_keys ;
         swap_ptr += 6 + sizeof(short) ;

         /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (int) new_block->n_keys ; i++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *)&long_val, sizeof(long) ) ;
            swap_ptr += new_block->tag->header.group_len ;
         }
         /* mark last_pointer */
         if ( !b4leaf( new_block ) )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *)&long_val, sizeof(long) ) ;
         }
      #endif

      /* flush blocks, don't delete */
      for( block_on = 0 ;; )
      {
         block_on = (B4BLOCK *)l4next(&t4->saved,block_on) ;
         if ( block_on == 0 )
            break ;
         if ( b4flush(block_on) < 0 )
            return -1 ;
         block_on->file_block = -1 ;   /* make it invalid */
      }
   }

   #ifdef S4FOX
      b4top( new_block ) ;
   #else
      new_block->key_on = 0 ;
   #endif

   return 0 ;
}

int S4FUNCTION t4eof( TAG4 *t4 )
{
   B4BLOCK *b4 ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4EOF ) ;
   #endif

   b4 = t4block(t4) ;

   #ifdef S4FOX
      return( (b4->key_on >= b4->header.n_keys) || (b4->header.n_keys == 0) ) ;
   #else
      return( b4->key_on >= b4->n_keys ) ;
   #endif
}

#ifndef S4OFF_WRITE
int S4FUNCTION t4update( TAG4 *t4 )
{
   B4BLOCK *block_on ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4UPDATE ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   for( block_on = 0 ;; )
   {
      block_on = (B4BLOCK *)l4next(&t4->saved,block_on) ;
      if ( block_on == 0 )
         break ;
      if ( b4flush(block_on) < 0 )
         return -1 ;
   }

   for( block_on = 0 ;; )
   {
      block_on = (B4BLOCK *)l4next(&t4->blocks,block_on) ;
      if ( block_on == 0 )
         break ;
      if ( b4flush(block_on) < 0 )
         return -1 ;
   }

   if ( t4->root_write )
   {
      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverse_long( t4->header.root ) ;
      #endif

      if ( file4write( &t4->index->file, t4->header_offset, &t4->header.root, sizeof(t4->header.root)) < 0 )
           return -1 ;

      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverse_long( t4->header.root ) ;
      #endif

      t4->root_write = 0 ;
   }

   return 0 ;
}
#endif  /* S4OFF_WRITE */

int S4FUNCTION t4free_all( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4FREE_ALL ) ;
   #endif

   while ( t4up( t4 ) == 0 ) ;
   return t4free_saved( t4 ) ;
}

int S4FUNCTION t4free_saved( TAG4 *t4 )
{
   B4BLOCK *block_on ;

   if ( t4update( t4 ) < 0 )
      return -1 ;
   for ( ;; )
   {
      block_on = (B4BLOCK *)l4pop( &t4->saved ) ;
      if ( block_on == 0 )
         return 0 ;
      if ( b4flush( block_on) < 0 )
         return -1 ;
      b4free( block_on ) ;
   }
}

int S4FUNCTION t4go2( TAG4 *t4, char *ptr, long rec_num )
{
   #ifdef S4FOX
      B4BLOCK *block_on ;
      int rc, rc2, k_len = t4->header.key_len ;
      unsigned long  rec ;
      char has_skipped ;
   #else
      int rc ;
      long rec, rec_save ;
      B4BLOCK *block_on ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 || ptr == 0 || rec_num < 1 )
         e4severe( e4parm, E4_T4GO ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifdef S4FOX
      rec = x4reverse_long( rec_num ) ;

      if ( t4->code_base->error_code < 0 )
         return -1 ;

      do
      {
         /* Do initial search, moving up only as far as necessary */
         rc2 = t4up_to_root( t4 ) ;
         if ( rc2 < 0 )
            return -1 ;

         if ( rc2 != 2 )
         {
            for(;;) /* Repeat until found */
            {
               block_on = (B4BLOCK *)t4->blocks.last_node ;
               #ifdef S4DEBUG
                  if ( block_on == 0 )
                     e4severe( e4info, E4_T4GO ) ;
               #endif

               if ( b4leaf(block_on) )
               {
                  rc = b4seek( block_on, (char *)ptr, k_len ) ;
                  if ( rc )    /* leaf seek did not end in perfect find */
                     return rc ;

                  /* now do the seek for recno on the leaf block */
                  block_on = (B4BLOCK *)t4->blocks.last_node ;

                  rec = t4recno( t4 ) ;
                  if ( rec == (unsigned long)rec_num )
                     return 0 ;
                  if ( rec > (unsigned long)rec_num )
                     for(;;)
                     {
                        rec = t4recno( t4 ) ;
                        if ( rec == (unsigned long)rec_num )
                           return 0 ;

                        if ( x4dup_cnt( block_on, block_on->key_on ) + x4trail_cnt( block_on, block_on->key_on ) != t4->header.key_len )
                        {
                           block_on->cur_dup_cnt = x4dup_cnt( block_on, block_on->key_on ) ;
                           return r4found ;
                        }

                        if ( rec < (unsigned long)rec_num )
                        {
                           rc = (int)t4skip(t4,1L) ;
                           if ( rc == -1 )
                              return -1 ;
                           return r4found ;
                        }

                        rc = (int)t4skip(t4,-1L) ;
                        if ( rc == 1 )
                           return -1 ;
                        if ( rc == 0 )
                            return r4found ;
                     }
                  else
                  {
                     has_skipped = 0 ;
                     for(;;)
                     {
                        rec = t4recno( t4 ) ;
                        if ( rec == (unsigned long)rec_num )
                           return 0 ;
                        if ( rec > (unsigned long)rec_num )
                        {
                           if ( !has_skipped )
                              block_on->cur_dup_cnt = x4dup_cnt( block_on, block_on->key_on ) ;
                           return r4found ;
                        }

                        has_skipped = 1 ;

                        rc = (int)t4skip(t4,1L) ;
                        if ( rc == -1 )
                           return -1 ;
                        if ( rc == 0 )
                        {
                           b4go_eof( t4block(t4) ) ;
                           return r4found ;
                        }

                        if ( x4dup_cnt( block_on, block_on->key_on ) + x4trail_cnt( block_on, block_on->key_on )
                             != t4->header.key_len )  /* case where key changed */
                           return r4found ;
                     }
                  }
               }
               else
                  rc = b4r_brseek( block_on, (char *)ptr, k_len, rec ) ;

               rc2 = t4down( t4 ) ;
               if ( rc2 < 0 )
                  return -1 ;
               if ( rc2 == 2 )
               {
                  t4out_of_date( t4 ) ;
                  break ;
               }
            }
         }
      } while ( rc2 == 2 ) ;
      return 0 ;
   #endif

   #ifndef S4FOX
      rc = t4seek( t4, ptr, t4->header.key_len ) ;
      if ( rc )
         return rc ;
      rec_save = t4recno( t4 ) ;
      if ( rec_save == rec_num )
         return 0 ;

      /* else find the far end, and then skip back to where now or find */
      t4up_to_root( t4 ) ;
      for( ;; )
      {
         block_on = (B4BLOCK *)t4->blocks.last_node ;
         #ifdef S4DEBUG
            if ( block_on == 0 )
               e4severe( e4info, E4_T4GO ) ;
         #endif
         rc = b4seek( block_on, ptr, t4->header.key_len ) ;
         while( rc == 0 )  /* perfect find, check next */
         {
            if ( b4skip( block_on, 1L ) == 0 )
               break ;
            if ( (*t4->cmp)( b4key_key( block_on, block_on->key_on ), ptr, t4->header.key_len ) != 0 )
               break ;
         }

         if ( b4leaf( block_on ) )
         {
            if ( t4recno( t4 ) == rec_num && rc == 0 )   /* found */
               return 0 ;
            if ( t4recno( t4 ) == rec_save )   /* didn't move */
               return r4found ;
            break ;
         }
         t4down( t4 ) ;
      }

      for(;;)
      {
         rc = (int)b4skip( t4block( t4 ), -1L ) ;
         if ( rc == 0 ) /* try previous tag */
         {
            if ( t4skip( t4, -1L ) == 0 )
               return r4found ;
         }
         rec = t4recno( t4 ) ;
         if ( rec == rec_save )   /* failed to find */
            return r4found ;
         if ( rec == rec_num )
            return 0 ;
      }
   #endif
}

B4KEY_DATA *S4FUNCTION t4key_data( TAG4 *t4 )
{
   B4BLOCK *b4 ;

   b4 = (B4BLOCK *)t4->blocks.last_node ;
   return b4key( b4, b4->key_on ) ;
}

long S4FUNCTION t4recno( TAG4 *t4 )
{
   B4BLOCK *block_on ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4RECNO ) ;
   #endif

   block_on = (B4BLOCK *)t4->blocks.last_node ;
   if ( block_on == 0 )
      return -2L ;

   #ifdef S4FOX
      if ( block_on->key_on >= block_on->header.n_keys )
         return -1 ;
   #else
      if ( !b4leaf( block_on ) )
         return -2L ;
   #endif

   return b4recno( block_on, block_on->key_on ) ;
}

int S4FUNCTION t4seek( TAG4 *t4, void *ptr, int len_ptr )
{
   int rc ;
   B4BLOCK *block_on ;
   #ifdef S4FOX
      int inc_pos, d_set ;
      unsigned char *c_ptr ;

      d_set = 0 ;
      c_ptr = (unsigned char *)ptr ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 || ptr == 0 )
         e4severe( e4parm, E4_T4SEEK ) ;
      if ( len_ptr != t4->header.key_len && t4type( t4 ) != r4str )
         e4severe( e4parm, E4_T4SEEK ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         t4->index->code_base->mode |= 0x08 ;
      #endif
   #endif

   #ifdef S4DEBUG
      if ( len_ptr != t4->header.key_len && t4type( t4 ) != r4str )
         e4severe( e4parm, E4_T4SEEK ) ;
   #endif

   if ( len_ptr > t4->header.key_len )
      len_ptr = t4->header.key_len ;
   #ifdef S4FOX
      if ( t4->header.descending )   /* look for current item less one: */
      {
         for( inc_pos = len_ptr-1 ; d_set == 0 && inc_pos >=0 ; inc_pos-- )
            if ( c_ptr[inc_pos] != 0xFF )
            {
               c_ptr[inc_pos]++ ;
               d_set = 1 ;
            }
      }
   #endif

   rc = 3 ;
   for( ;; ) /* Repeat until found */
   {
      while ( rc >= 2 )
      {
         if ( rc == 2 )
            t4out_of_date( t4 ) ;
         rc = t4up_to_root( t4 ) ;
         if ( rc < 0 )
            return -1 ;
      }
      block_on = (B4BLOCK *)t4->blocks.last_node ;
      #ifdef S4DEBUG
         if ( block_on == 0 )
            e4severe( e4info, E4_T4SEEK ) ;
      #endif

      rc = b4seek( block_on, (char *)ptr, len_ptr ) ;
      if ( b4leaf(block_on) )
         break ;

      rc = t4down( t4 ) ;
      if ( rc < 0 )
         return -1 ;
   }

   #ifdef S4FOX
      if ( t4->header.descending )   /* must go back one! */
      {
         c_ptr[inc_pos+1]-- ; /* reset the search_ptr ; */
         if ( d_set )
         {
            rc = (int)t4skip( t4, -1L ) ;
            if ( rc == 0L )  /* bof = eof condition */
            {
               b4go_eof( block_on ) ;
               rc = r4eof ;
            }
            else
            {
               b4go( t4block( t4 ), t4block( t4 )->key_on ) ;
               if ( (u4memcmp)( b4key_key( t4block( t4 ), t4block( t4 )->key_on ), ptr, len_ptr ) )
                  rc = r4after ;
               else
                  rc = 0 ;  /* successful find */
            }
         }
         else
         {
            if ( rc == 0 )  /* the item was found, so go top, */
               t4top( t4 ) ;
            else  /* otherwise want an eof type condition */
            {
               b4go_eof( block_on ) ;
               rc = r4eof ;
            }
         }
      }
   #endif
   return rc ;
}

long S4FUNCTION t4skip( TAG4 *t4, long num_skip )
{
   long num_left ;
   B4BLOCK *block_on ;
   int rc ;
   #ifdef S4FOX
      int save_dups ;
      long go_to ;
   #else
      int sign ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4SKIP ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         t4->index->code_base->mode |= 0x04 ;
      #endif
   #endif

   num_left = num_skip ;

   block_on = (B4BLOCK *)t4->blocks.last_node ;
   if ( block_on == 0 )
   {
      if ( t4top(t4) < 0 )
         return -num_skip ;
      block_on = (B4BLOCK *)t4->blocks.last_node ;
   }

   #ifdef S4DEBUG
      if ( ! b4leaf(block_on) )
         e4severe( e4info, E4_T4SKIP ) ;
   #endif

   #ifdef S4FOX
      for(;;)
      {
         num_left -= b4skip( block_on, num_left ) ;
         if ( num_left == 0 )  /* Success */
            return( num_skip ) ;

         if ( num_left > 0 )
            go_to = block_on->header.right_node ;
         else
            go_to = block_on->header.left_node ;

         if ( go_to == -1 )
         {
            if ( num_skip > 0 )
            {
               save_dups = t4block( t4 )->cur_dup_cnt ;
               rc = t4bottom(t4) ;
               t4block( t4 )->cur_dup_cnt = save_dups ;
               if ( rc < 0 )
                  return -num_skip ;
            }
            else
               if ( t4top(t4) < 0 )
                 return -num_skip ;
            return (num_skip - num_left) ;
         }

         if ( b4flush( block_on ) < 0 )
            return -num_skip ;

         /* save the current key in case skip fails */
         if ( !t4->index->file.is_read_only && !t4->index->file.is_exclusive )
            memcpy( t4->code_base->saved_key, b4key_key( block_on, block_on->header.n_keys - 1 ), t4->header.key_len ) ;

         rc = i4read_block( &t4->index->file, go_to, 0, block_on ) ;

         if ( rc < 0 )
            return -num_skip ;

         if ( rc == 1 )   /* failed on i/o, seek current spot to make valid */
         {
            #ifndef S4OPTIMIZE_OFF
            #ifndef S4SINGLE
               file4refresh( &t4->index->file ) ;
            #endif
            #endif
            rc = t4seek( t4, t4->code_base->saved_key, t4->header.key_len ) ;
            if ( rc < 0 )
               return -num_skip ;
            if ( rc == r4after )   /* means skipped 1 ahead */
               num_left-- ;
         }

         block_on->file_block = go_to ;
         block_on->built_on = -1 ;
         b4top( block_on ) ;

         if ( num_left < 0 )
            num_left += block_on->header.n_keys ;
         else
            num_left -= 1 ;  /* moved to the next entry */
      }
   #endif

   #ifndef S4FOX
      if ( num_skip < 0)
         sign = -1 ;
      else
         sign = 1 ;

      for(;;)
      {
         /* save the current key in case skip fails */
         memcpy( t4->code_base->saved_key, b4key_key( block_on, block_on->key_on ), t4->header.key_len ) ;

         while ( ( rc = t4down( t4 ) ) == 0 )
            if ( sign < 0 )
            {
               block_on = t4block( t4 ) ;
               b4go_eof( block_on ) ;
               if ( b4leaf( block_on) )
               {
                  block_on->key_on-- ;
                  #ifdef S4DEBUG
                     if ( block_on->key_on < 0 )
                        e4severe( e4info, E4_T4SKIP ) ;
                  #endif
               }
            }

         if ( rc < 0 )
            return -num_skip ;

         if ( rc == 2 )   /* failed on i/o, seek current spot to make valid */
         {
            #ifndef S4OPTIMIZE_OFF
            #ifndef S4SINGLE
               file4refresh( &t4->index->file ) ;
            #endif
            #endif
            rc = t4seek( t4, t4->code_base->saved_key, t4->header.key_len ) ;
            if ( rc < 0 )
               return -num_skip ;
            if ( rc == r4after )   /* means skipped 1 ahead */
               num_left-- ;
         }

         block_on = t4block( t4 ) ;

         if (rc < 0 )  /* Error */
            return( -num_skip ) ;

         num_left -= b4skip( block_on, num_left ) ;
         if ( num_left == 0)      /* Success */
            return( num_skip ) ;

         do  /* Skip 1 to the next leaf block  */
         {
            if ( (B4BLOCK *)block_on->link.p == block_on )
            {
               if ( num_skip > 0 )
               {
                  if ( t4bottom( t4 ) < 0 )
                     return -num_skip ;
               }
               else
                  if ( t4top(t4) < 0 )
                     return -num_skip ;

               return( num_skip - num_left ) ;
            }
            else
               t4up(t4) ;

            block_on = (B4BLOCK *)t4->blocks.last_node ;
         }  while ( b4skip( block_on, (long) sign) != sign) ;

         num_left -= sign ;
      }
   #endif
}

#ifndef S4OFF_WRITE
B4BLOCK *S4FUNCTION t4split( TAG4 *t4, B4BLOCK *old_block )
{
   long new_file_block ;
   B4BLOCK *new_block ;
   #ifndef S4FOX
      int tot_len, new_len ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return 0 ;

   #ifdef S4INDEX_VERIFY
      if ( b4verify( old_block ) == -1 )
         e4describe( old_block->tag->code_base, e4index, old_block->tag->alias, "t4split()-before split", "" ) ;
   #endif

   new_file_block = i4extend( t4->index ) ;

   new_block = b4alloc( t4, new_file_block ) ;
   if ( new_block == 0 )  return 0 ;

   new_block->changed = 1 ;
   old_block->changed = 1 ;

   #ifdef S4FOX
      if ( b4leaf( old_block ) )
         t4leaf_split( t4, old_block, new_block ) ;
      else
         t4branch_split( t4, old_block, new_block ) ;

      new_block->header.right_node = old_block->header.right_node ;
      new_block->header.left_node = old_block->file_block ;
      old_block->header.right_node = new_block->file_block ;

      if ( new_block->header.right_node != -1 )   /* must change left ptr for next block over */
      {
         #ifdef S4BYTE_SWAP
            new_block->file_block = x4reverse_long( new_block->file_block ) ;
         #endif
         file4write( &t4->index->file, new_block->header.right_node + 2*sizeof(short),
                  &new_block->file_block, sizeof( new_block->header.left_node ) ) ;
         #ifdef S4BYTE_SWAP
            new_block->file_block = x4reverse_long( new_block->file_block ) ;
         #endif
      }
   #else
      /* NNNNOOOO  N - New, O - Old */
      new_block->n_keys  = (old_block->n_keys+1)/2 ;
      old_block->n_keys -= new_block->n_keys ;
      new_block->key_on  = old_block->key_on ;

      tot_len = t4->index->header.block_rw - sizeof(old_block->n_keys) - sizeof(old_block->dummy) ;
      new_len = new_block->n_keys * t4->header.group_len ;

      memcpy( (void *)b4key(new_block,0), (void *)b4key(old_block,0), new_len ) ;
      memmove( b4key(old_block,0), b4key(old_block,new_block->n_keys), tot_len - new_len ) ;
      old_block->key_on = old_block->key_on - new_block->n_keys ;
   #endif

   return new_block ;
}

#ifdef S4FOX
void S4FUNCTION t4branch_split( TAG4 *t4, B4BLOCK *old_block, B4BLOCK *new_block )
{
   int new_len, n_new_keys ;
   int g_len = t4->header.key_len + 2*sizeof( long ) ;
   char *o_pos ;

   /* NNNNOOOO  N - New, O - Old */
   n_new_keys = ( old_block->header.n_keys + 1 ) / 2 ;
   if ( old_block->key_on >= n_new_keys )
      n_new_keys-- ;
   new_block->header.n_keys = n_new_keys ;
   old_block->header.n_keys -= n_new_keys ;

   new_len = new_block->header.n_keys * g_len ;

   o_pos = ((char *)&old_block->node_hdr) + g_len * old_block->header.n_keys ;
   memcpy( (void *)&new_block->node_hdr, o_pos, new_len ) ;
   new_block->header.node_attribute = 0 ;
   old_block->header.node_attribute = 0 ;
   new_block->key_on = old_block->key_on - old_block->header.n_keys ;

   /* clear the old data */
   memset( o_pos, 0, new_len ) ;
}

void S4FUNCTION t4leaf_split( TAG4 *t4, B4BLOCK *old_block, B4BLOCK *new_block )
{
   char *obd_pos, *obi_pos ;
   unsigned char buffer[6] ;
   int len, n_keys ;
   int k_len = t4->header.key_len ;
   int i_len = old_block->node_hdr.info_len ;
   int b_len = B4BLOCK_SIZE - (sizeof(old_block->header)) - (sizeof(old_block->node_hdr))
               - old_block->header.n_keys * i_len
               - old_block->node_hdr.free_space ;
   int old_dup = old_block->cur_dup_cnt ;

   b4top( old_block ) ;
      n_keys = old_block->header.n_keys / 2 ;
      for ( len = 0 ; len < old_block->header.n_keys - n_keys ; len++ )
         b4skip( old_block, 1L ) ;

   /* build the key 1st key of the new block from one past new end of old block */
   b4key( old_block, old_block->key_on ) ;

   /* copy the general information */
   memcpy( (void *)&new_block->header, (void *)&old_block->header,
           (sizeof( old_block->header)) + (sizeof(old_block->node_hdr)) ) ;

   /* put 1st key of new block */
   new_block->cur_trail_cnt = b4calc_blanks( old_block->built_key->value, k_len, t4->p_char ) ;
   len = k_len - new_block->cur_trail_cnt ;
   new_block->cur_pos = ((char *)&new_block->header) + B4BLOCK_SIZE - len ;
   memcpy( new_block->cur_pos, old_block->built_key->value, len ) ;

   /* copy remaining key data */
   obd_pos = ((char *)&old_block->header) + B4BLOCK_SIZE - b_len ;
   len = old_block->cur_pos - obd_pos ;
   new_block->cur_pos -= len ;
   memcpy( new_block->cur_pos, obd_pos, len ) ;

   /* copy the info data */
   obi_pos = old_block->data + old_block->key_on * i_len ;
   memcpy( new_block->data, obi_pos, n_keys * i_len ) ;

   /* clear the old data */
   b4skip( old_block, -1L ) ;  /* go to new last entry */
   #ifdef S4DEBUG
      if ( obd_pos < obi_pos )
         e4severe( e4info, E4_INFO_BMC ) ;
   #endif
   memset( obi_pos, 0, old_block->cur_pos - obi_pos ) ;

   /* now reset the place new info data for the first key */
   memset( new_block->data, 0, i_len ) ;
   x4put_info( &new_block->node_hdr, buffer, old_block->built_key->num, new_block->cur_trail_cnt, 0 ) ;
   memcpy( new_block->data, buffer, i_len ) ;

   new_block->header.n_keys = n_keys ;
   old_block->header.n_keys -= n_keys ;
   new_block->header.node_attribute = 2 ;
   old_block->header.node_attribute = 2 ;
   new_block->node_hdr.free_space = new_block->cur_pos - new_block->data
                                    - new_block->header.n_keys * i_len ;
   old_block->node_hdr.free_space = old_block->cur_pos - old_block->data
                                    - old_block->header.n_keys * i_len ;
   old_block->built_on = -1 ;
   new_block->built_on = -1 ;

   b4top( old_block ) ;
   b4top( new_block ) ;

   /* make sure dup_cnt is updated on blocks for insert */
   new_block->cur_dup_cnt = old_dup ;
   old_block->cur_dup_cnt = old_dup ;
}
#endif
#endif  /* S4OFF_WRITE */

#ifdef S4FOX
int S4FUNCTION t4rl_top( TAG4 *t4 )
{
   int  rc ;

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   do
   {
      rc = t4up_to_root( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( rc != 2 )
      {
         do
         {
            b4top( (B4BLOCK *)t4->blocks.last_node ) ;
            if ( (rc = t4down(t4)) < 0 )
               return -1 ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         t4out_of_date( t4 ) ;
   } while ( rc == 2 ) ;
   return 0 ;
}

int S4FUNCTION t4top( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4TOP ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   if ( t4->header.descending )   /* if descending, go top means go bottom */
      return t4rl_bottom( t4 ) ;
   else
      return t4rl_top( t4 ) ;
}
#else
int S4FUNCTION t4top( TAG4 *t4 )
{
   int  rc ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4TOP ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   do
   {
      rc = t4up_to_root( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      if ( rc != 2 )
      {
         ((B4BLOCK *)t4->blocks.last_node)->key_on = 0 ;
         do
         {
            if ( (rc = t4down(t4)) < 0 )
               return -1 ;
            ((B4BLOCK *)t4->blocks.last_node)->key_on = 0 ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         t4out_of_date( t4 ) ;
   } while ( rc == 2 ) ;

   return 0 ;
}
#endif

int  S4FUNCTION t4up( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4UP ) ;
   #endif

   if ( t4->blocks.last_node == 0 )
      return 1 ;
   l4add( &t4->saved, l4pop(&t4->blocks) ) ;
   return 0 ;
}

int  S4FUNCTION t4up_to_root( TAG4 *t4 )
{
   LINK4 *link_on ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4UP_TO_ROOT ) ;
   #endif

   for ( ;; )
   {
      link_on = (LINK4 *)l4pop( &t4->blocks ) ;
      if ( link_on == 0 )
         return t4down( t4 ) ;
      l4add( &t4->saved, link_on ) ;
   }
}

#endif  /* N4OTHER */
#endif  /* S4INDEX_OFF */

#ifdef S4VB_DOS

int t4go_v( TAG4 *t4, char *key, long rec_no )
{
   return t4go( t4, c4str(key), rec_no ) ;
}

int t4seek_v( TAG4 *t4, char *seek_val, int key_len )
{
   return t4seek( t4, v4str(seek_val), key_len) ;
}

#endif
