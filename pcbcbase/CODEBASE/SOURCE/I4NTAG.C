/* i4ntag.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

#include <time.h>

int S4FUNCTION t4type( TAG4 *t4 )
{
 #ifdef S4FOX
    return t4->expr->type ;
 #endif
 #ifdef S4CLIPPER
    return t4->expr->type ;
 #endif
 #ifdef S4NDX
    return t4->header.type ;
 #endif
 #ifdef S4MDX
    return t4->header.type ;
 #endif
}

#ifdef N4OTHER

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

#ifdef S4CLIPPER
int S4FUNCTION t4rl_bottom( TAG4 *t4 )
#else
int S4FUNCTION t4bottom( TAG4 *t4 )
#endif
{
   int rc ;
   B4BLOCK *block_on ;

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
         b4go_eof( t4block( t4 ) ) ;
         do
         {
            rc = t4down( t4 ) ;
            if ( rc < 0 )
               return -1 ;
            b4go_eof( t4block( t4 ) ) ;
         } while ( rc == 0 ) ;
      }

      if ( rc == 2 )   /* failed due to read while locked */
         t4out_of_date( t4 ) ;
   } while ( rc == 2 ) ;

   block_on = t4block(t4) ;
   if ( block_on->key_on > 0 )
      block_on->key_on = block_on->n_keys-1 ;

   #ifdef S4DEBUG
      if ( block_on->key_on < 0 )  e4severe( e4info, E4_T4BOTTOM ) ;
   #endif

   return 0 ;
}

#ifdef S4CLIPPER
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
#endif

#ifdef S4CLIPPER
#ifdef S4HAS_DESCENDING
void S4FUNCTION t4descending( TAG4 *tag, int setting )
{
   tag->header.descending = setting ;
}
#endif
#endif

/* Returns  1 - Cannot move down; 0 - Success; -1 Error */
int S4FUNCTION t4down( TAG4 *t4 )
{
   long block_down ;
   B4BLOCK *block_on, *pop_block, *new_block, *parent ;
   int rc ;
   #ifdef S4BYTE_SWAP
      char *swap_ptr ;
      int i ;
      short short_val ;
      long long_val ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4DOWN ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   block_on = (B4BLOCK *)t4->blocks.last_node ;

   if ( block_on == 0 )    /* Read the root block */
   {
      if ( t4->header.root <= 0L )
      {
         #ifdef S4NDX
            if ( file4read_all( &t4->file, t4->header.header_offset, &t4->header.root, 2*sizeof(long)) < 0 )
               return -1 ;
         #else
            #ifdef S4CLIPPER
               if ( file4read_all( &t4->file, t4->header.header_offset + 2*sizeof(short), &t4->header.root, 2*sizeof(long)) < 0 )
                  return -1 ;
            #endif
         #endif

         #ifdef S4BYTE_SWAP
	    t4->header.root = x4reverse_long( t4->header.root ) ;
	    t4->header.eof  = x4reverse_long( t4->header.eof ) ;
         #endif
      }
      block_down = t4->header.root ;
   }
   else
   {
      if ( b4leaf( block_on ) )
         return 1 ;
      block_down = b4key( block_on, block_on->key_on )->pointer ;
      #ifdef S4DEBUG
         if ( block_down <= 0L )
            e4severe( e4info, E4_INFO_ILF ) ;
      #endif
   }

   /* Get memory for the new block */
   pop_block = (B4BLOCK *)l4pop( &t4->saved ) ;
   new_block = pop_block ;
   if ( new_block == 0 )
      new_block = b4alloc( t4, block_down ) ;
   if ( new_block == 0 )
      return -1 ;
   parent = (B4BLOCK *)l4last( &t4->blocks ) ;
   l4add( &t4->blocks, new_block ) ;

   #ifdef S4NDX
      if ( pop_block == 0  ||  new_block->file_block != block_down )
   #else
      if ( pop_block == 0  ||  new_block->file_block*I4MULTIPLY != block_down )
   #endif
   {
      if ( new_block->changed == 1 )
         if ( b4flush(new_block) < 0 )
            return -1 ;

      rc = i4read_block( &t4->file, block_down, parent, new_block ) ;

      if ( rc < 0 )
         return -1 ;

      if ( rc == 1 )
      {
         l4remove( &t4->blocks, new_block ) ;
         l4add( &t4->saved, new_block ) ;
         return 2 ;
      }

      #ifdef S4BYTE_SWAP
         #ifdef S4NDX
	    swap_ptr = (void *)&new_block->n_keys ;

	    short_val = x4reverse_short( *(short *)swap_ptr ) ;
	    memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;

	    swap_ptr += 2 + sizeof(short) ;

	    for ( i = 0 ; i < (*(short *)swap) ; i++ )
	    {
	       long_val = x4reverse_long( *(long *)swap_ptr ) ;
	       memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
	       long_val = x4reverse_long( *(long *)swap_ptr+sizeof(long) ) ;
	       memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
	       swap_ptr += r4->group_len ;
	    }

	    long_val = x4reverse_long( *(long *)swap_ptr ) ;
	    memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
	 #endif

	 #ifdef S4CLIPPER
	    swap_ptr = (void *)&new_block->n_keys ;

	    short_val = x4reverse_short( *(short *)swap_ptr ) ;
	    memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;

	    swap_ptr += 2 ;
			    /* swap the short pointers to B4KEY_DATA */
	    for ( i = 0 ; i <= t4->header.keys_max ; i++ )
            {
               short_val = x4reverse_short( *(short *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
               swap_ptr += sizeof(short) ;
            }
                            /* swap the B4KEY_DATA's */
            for ( i = 0 ; i < t4->header.keys_max ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
	       swap_ptr += t4->header.group_len ;
            }
         #endif
      #endif

      for ( ;; )
      {
         block_on = (B4BLOCK *)l4pop( &t4->saved ) ;
         if ( block_on == 0 )
            break ;
         if ( b4flush(block_on) < 0 )
            return -1 ;
         b4free( block_on ) ;
      }
   }

   new_block->key_on = 0 ;
   return 0 ;
}

int S4FUNCTION t4eof( TAG4 *t4 )
{
   B4BLOCK *b4 ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4EOF ) ;
   #endif

   b4 = t4block( t4 ) ;
   return ( b4->key_on >= b4->n_keys ) ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION t4flush( TAG4 *t4 )
{
   int rc ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4FLUSH ) ;
   #endif

   rc = t4update( t4 ) ;
   #ifndef S4OPTIMIZE_OFF
      if ( rc )
         rc = file4flush( &t4->file ) ;
   #endif
   return rc ;
}
#endif

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
      #ifndef S4OFF_WRITE
         if ( b4flush( block_on ) < 0 )
            return -1 ;
      #endif
      b4free( block_on ) ;
   }
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
   #ifdef S4NDX
      if ( !b4leaf( block_on ) )
         return -2L ;
   #else
      if ( block_on->key_on >= block_on->n_keys )
         return -1 ;
   #endif

   return b4recno( block_on, block_on->key_on ) ;
}

int S4FUNCTION t4seek( TAG4 *t4, void *ptr, int len_ptr )
{
   int rc ;
   B4BLOCK *block_on ;
   #ifdef S4CLIPPER
      int upper_fnd, upper_aft, inc_pos, d_set ;
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

   if ( len_ptr > t4->header.key_len )
      len_ptr = t4->header.key_len ;

   #ifdef S4CLIPPER
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
   for(;;) /* Repeat until found */
   {
      while ( rc >= 2 )
      {
         if ( rc == 2 )
            t4out_of_date( t4 ) ;
         rc = t4up_to_root( t4 ) ;
         #ifdef S4CLIPPER
            upper_fnd = 0 ;
            upper_aft = 0 ;
         #endif

         if ( rc < 0 )
            return -1 ;
      }
      block_on = (B4BLOCK *)t4->blocks.last_node ;
      #ifdef S4DEBUG
         if ( block_on == 0 )
            e4severe( e4info, E4_T4SEEK ) ;
      #endif

      rc = b4seek( block_on, (char *)ptr, len_ptr ) ;
      #ifdef S4NDX
         if ( b4leaf( block_on ) )
            break ;
      #else
         #ifdef S4CLIPPER
            if ( b4leaf( block_on ) )
            {
               if ( rc == r4after && upper_aft && block_on->key_on >= block_on->n_keys )
                   while( upper_aft-- > 1 )
                      t4up( t4 ) ;
               if ( rc == 0 || !upper_fnd )
                  break ;

               while( upper_fnd-- > 1 )
                   t4up( t4 ) ;
               rc = 0 ;
               break ;
            }
            if ( rc == 0 )
            {
               upper_fnd = 1 ;
               upper_aft = 0 ;
            }
            else
               if ( rc == r4after && !upper_fnd && !( block_on->key_on >= block_on->n_keys ) )
                  upper_aft = 1 ;
         #endif
      #endif

      rc = t4down( t4 ) ;
      if ( rc < 0 )
         return -1 ;

      #ifdef S4CLIPPER
         if ( upper_fnd )
            upper_fnd++ ;
         if ( upper_aft )
            upper_aft++ ;
      #endif
   }
   #ifdef S4CLIPPER
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
/*               b4go( t4block( t4 ), t4block( t4 )->key_on ) ; */
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
   int rc, sign ;
   B4BLOCK *block_on ;

   #ifdef S4NDX
      long num_left = num_skip ;
   #endif
   #ifdef S4CLIPPER
      long j ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4SEEK ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         t4->index->code_base->mode |= 0x04 ;
      #endif
   #endif

   if ( num_skip < 0)
      sign = -1 ;
   else
      sign = 1 ;

   block_on = (B4BLOCK *)t4->blocks.last_node ;
   if ( block_on == 0 )
   {
      rc = t4top( t4 ) ;
      if ( rc < 0 )
         return -num_skip ;
      block_on = (B4BLOCK *)t4->blocks.last_node ;
   }

   #ifdef S4NDX
      #ifdef S4DEBUG
         if ( !b4leaf( block_on ) )
            e4severe( e4info, E4_T4SKIP ) ;
      #endif

      for(;;)
      {
         /* save the current key in case skip fails */
         memcpy( t4->code_base->saved_key, b4key_key( block_on, block_on->key_on ), t4->header.key_len ) ;

         while ( (rc = t4down(t4)) == 0 )
            if ( sign < 0 )
            {
               block_on = t4block(t4) ;
               b4go_eof( block_on ) ;
               if ( b4leaf(block_on) )
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
               file4refresh( &t4->file ) ;
            #endif
            #endif
            rc = t4seek( t4, t4->code_base->saved_key, t4->header.key_len ) ;
            if ( rc < 0 )
               return -num_skip ;
            if ( rc == r4after )   /* means skipped 1 ahead */
               num_left-- ;
         }

         block_on = t4block( t4 ) ;

         if ( rc < 0 )  /* Error */
            return( -num_skip ) ;

         num_left -= b4skip( block_on, num_left ) ;
         if ( num_left == 0 )  /* Success */
            return( num_skip ) ;

         do  /* Skip 1 to the next leaf block  */
         {
            #ifdef S4DEBUG
               if ( t4->blocks.last_node == 0 )
                  e4severe( e4result, E4_T4SKIP ) ;
            #endif
            if ( l4prev( &t4->blocks, t4->blocks.last_node ) == 0 )  /* root block */
            {
               if ( num_skip > 0 )
               {
                  if ( t4bottom( t4 ) < 0 )
                     return -num_skip ;
               }
               else
                  if ( t4top( t4 ) < 0 )
                     return -num_skip ;

               return( num_skip - num_left ) ;
            }
            t4up( t4 ) ;
            block_on = (B4BLOCK *)t4->blocks.last_node ;
         }  while ( b4skip( block_on, (long) sign) != sign) ;
         num_left -= sign ;
      }
   #else
      #ifdef S4CLIPPER
         for( j = num_skip; j != 0; j -= sign )  /* skip 1 * num_skip */
         {
            if ( b4leaf(block_on) )
            {
               if ( b4skip( block_on, (long) sign) != sign )  /* go up */
               {
                  int go_on = 1 ;
                  while ( go_on )
                  {
                     if ( l4prev( &t4->blocks, t4->blocks.last_node ) == 0 )  /* root block */
                     {
                        if ( num_skip > 0 )
                        {
                           if ( t4bottom( t4 ) < 0 )  return -num_skip ;
                        }
                        else
                           if ( t4top( t4 ) < 0 ) return -num_skip ;

                        return ( num_skip - j ) ;
                     }

                     rc = t4up( t4 ) ;
                     block_on = t4block( t4 ) ;
                     if ( rc != 0 ) return -1 ;

                     if ( sign > 0 )  /* forward skipping */
                     {
                        if ( !( block_on->key_on >= block_on->n_keys ) )
                           go_on = 0 ;
                     }
                     else   /* backward skipping */
                     {
                        if ( ! (block_on->key_on == 0) )
                        {
                           b4skip( block_on, -1L ) ;
                           go_on = 0 ;
                        }
                     }
                  }
               }
            }
            else
            {
               if ( sign > 0 )
                  b4skip( block_on, 1L ) ;

               /* save the current key in case skip fails */
               memcpy( t4->code_base->saved_key, b4key_key( block_on, block_on->key_on ), t4->header.key_len ) ;

               while ( (rc = t4down( t4 ) ) == 0 )
               {
                  if ( sign < 0 )
                  {
                     block_on = t4block( t4 ) ;
                     b4go_eof( block_on ) ;
                     if ( b4leaf( block_on ) )
                        block_on->key_on-- ;
                  }
               }
               if ( rc < 0 )
                  return -num_skip ;

               if ( rc == 2 )   /* failed on i/o, seek current spot to make valid */
               {
                  #ifndef S4OPTIMIZE_OFF
                  #ifndef S4SINGLE
                     file4refresh( &t4->file ) ;
                  #endif
                  #endif
                  rc = t4seek( t4, t4->code_base->saved_key, t4->header.key_len ) ;
                  if ( rc < 0 )
                     return -num_skip ;
                  if ( rc == r4after )   /* means skipped 1 ahead */
                     j-- ;
               }

               block_on = t4block( t4 ) ;
            }
         }
         return num_skip ;
      #endif
   #endif
}

#ifndef S4OFF_WRITE
#ifdef S4NDX
B4BLOCK *S4FUNCTION t4split( TAG4 *t4, B4BLOCK *old_block )
{
   long new_file_block ;
   B4BLOCK *new_block ;
   int new_len ;

   if ( t4->code_base->error_code < 0 )
      return 0 ;

   #ifdef S4INDEX_VERIFY
      if ( b4verify( old_block ) == -1 )
         e4describe( old_block->tag->code_base, e4index, old_block->tag->alias, "t4split()-before split", "" ) ;
   #endif

   new_file_block = t4extend( t4 ) ;
   new_block = b4alloc( t4, new_file_block ) ;
   if ( new_block == 0 )
      return 0 ;

   new_block->changed = 1 ;
   old_block->changed = 1 ;

   /* NNNNOOOO  N - New, O - Old */
   new_block->n_keys = ( old_block->n_keys )/2 ;
   old_block->n_keys -= new_block->n_keys ;

   new_len = new_block->n_keys * t4->header.group_len ;

   memmove(  b4key(new_block,0),  b4key(old_block, old_block->n_keys ), new_len+ (sizeof(long)*(!(b4leaf( old_block )) ) ) ) ;
   new_block->key_on = old_block->key_on - old_block->n_keys ;

   old_block->n_keys -= !( b4leaf( old_block ) ) ;

   return new_block ;
}
#else
#ifdef S4CLIPPER
/* NTX only needs to do a copy and adjust the index pointers */
/* if extra_old is true then the extra key is placed in old, otherwise in new */
B4BLOCK *S4FUNCTION t4split( TAG4 *t4, B4BLOCK *old_block, int extra_old )
{
   long  new_file_block ;
   B4BLOCK *new_block ;
   int is_branch ;

   if ( t4->code_base->error_code < 0 )
      return 0 ;

   #ifdef S4INDEX_VERIFY
      if ( b4verify( old_block ) == -1 )
         e4describe( old_block->tag->code_base, e4index, old_block->tag->alias, "t4split()-before split", "" ) ;
   #endif

   new_file_block = t4extend( t4 ) ;
   new_block = b4alloc( t4, new_file_block ) ;
   if ( new_block == 0 )
      return 0 ;

   new_block->changed = 1 ;
   old_block->changed = 1 ;

   memcpy( new_block->data, old_block->data, B4BLOCK_SIZE - ( t4->header.keys_max + 2 ) * sizeof(short) ) ;

   if ( extra_old )
   {
      new_block->n_keys = old_block->n_keys / 2 ;
      old_block->n_keys -= new_block->n_keys ;
   }
   else
   {
      new_block->n_keys = old_block->n_keys ;
      old_block->n_keys = old_block->n_keys / 2 ;
      new_block->n_keys -= old_block->n_keys ;
      if ( old_block->n_keys == new_block->n_keys )
      {
         new_block->n_keys++ ;
         old_block->n_keys-- ;
      }
   }

   is_branch = !b4leaf( old_block ) ;

   memcpy( new_block->pointers, &old_block->pointers[old_block->n_keys + is_branch],
           new_block->n_keys * sizeof(short) ) ;
   memcpy( &new_block->pointers[new_block->n_keys], old_block->pointers,
           (old_block->n_keys + is_branch) * sizeof(short) ) ;

   if ( is_branch == 0 )  /* leaf blocks need one more copy */
      new_block->pointers[t4->header.keys_max] = old_block->pointers[t4->header.keys_max] ;

   new_block->key_on = old_block->key_on - old_block->n_keys - is_branch ;
   new_block->n_keys -= is_branch ;

   return new_block ;
}
#endif
#endif
#endif  /* S4OFF_WRITE */

#ifdef S4CLIPPER
int S4FUNCTION t4rl_top( TAG4 *t4 )
#else
int S4FUNCTION t4top( TAG4 *t4 )
#endif
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

#ifdef S4CLIPPER
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
#endif

int S4FUNCTION t4up( TAG4 *t4 )
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

   if ( t4update_header( t4 ) < 0 )
      return -1 ;

   for( block_on = 0 ;; )
   {
      block_on = (B4BLOCK *)l4next( &t4->saved ,block_on ) ;
      if ( block_on == 0 )
         break ;
      if ( b4flush(block_on) < 0 )
         return -1 ;
   }

   for( block_on = 0 ;; )
   {
      block_on = (B4BLOCK *)l4next( &t4->blocks, block_on ) ;
      if ( block_on == 0 )
         break ;
      if ( b4flush( block_on ) < 0 )
         return -1 ;
   }

   if ( t4->root_write )
   {
      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverse_long( t4->header.root ) ;
         t4->header.eof = x4reverse_long( t4->header.eof ) ;
      #endif
      #ifdef S4NDX
         if ( file4write( &t4->file, t4->header_offset, &t4->header.root, 2 * sizeof(long) ) < 0 )
            return -1 ;
      #else
         #ifdef S4CLIPPER
            if ( file4write( &t4->file, t4->header_offset + 2*sizeof( short ),
                 &t4->header.root, 2*sizeof(long) ) < 0 )  return -1 ;
         #endif
      #endif
      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverse_long( t4->header.root ) ;
         t4->header.eof = x4reverse_long( t4->header.eof ) ;
      #endif
      t4->root_write = 0 ;
   }

   return 0 ;
}
#endif

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
         return t4down(t4) ;
      l4add( &t4->saved, link_on ) ;
   }
}

int S4FUNCTION t4close( TAG4 *t4 )
{
   int final_rc ;
   int save_attempts ;
   DATA4 *d4 ;

   #ifdef S4VBASIC
      if ( c4parm_check( t4, 0, E4_T4CLOSE ) )
         return -1 ;
   #endif

   final_rc = e4set( t4->code_base, 0 ) ;
   save_attempts = t4->code_base->lock_attempts ;
   d4 = t4->index->data ;

   t4->code_base->lock_attempts = -1 ;

   if ( d4 )
      if ( d4update( d4 ) < 0 )
         final_rc = e4set( t4->code_base, 0 ) ;

   #ifndef S4SINGLE
      if ( t4->file_locked )
      {
         file4unlock( &t4->file, L4LOCK_POS, L4LOCK_POS ) ;
         t4->file_locked = 0 ;
      }
   #endif

   t4free_all( t4 ) ;

   if ( file4open_test( &t4->file ) )
      if ( file4close( &t4->file ) < 0 )
         final_rc = e4set( t4->code_base, 0 ) ;

   t4->code_base->lock_attempts = save_attempts ;
   e4set( t4->code_base, final_rc ) ;

   return final_rc ;
}

#ifndef S4OFF_WRITE
long S4FUNCTION t4extend( TAG4 *t4 )
{
   long old_eof ;
   CODE4 *c4 = t4->index->code_base ;

   if ( c4->error_code < 0 )
      return -1 ;

   #ifdef S4DEBUG
      if ( t4->header.version == t4->header.old_version )
         e4severe( e4info, E4_T4EXTEND ) ;
   #endif

   old_eof = t4->header.eof ;

   #ifdef S4NDX
      #ifdef S4DEBUG
        if (old_eof <= 0L )  e4severe( e4info, E4_T4EXTEND ) ;
      #endif

      t4->header.eof += B4BLOCK_SIZE / I4MULTIPLY ;
      t4->root_write = 1 ;
      return old_eof ;
   #else
      #ifdef S4CLIPPER
         if ( old_eof != 0 )   /* case where free-list exists */
            t4->header.eof = 0L ;
         else
         {
            old_eof = file4len( &t4->file ) ;
            #ifdef S4DEBUG
               if ( old_eof <= t4->check_eof )
                  e4severe( e4info, E4_T4EXTEND ) ;
               t4->check_eof = old_eof ;
            #endif
            file4len_set( &t4->file, file4len( &t4->file ) + 1024 ) ; /* and extend the file */
         }
         return old_eof/512 ;
      #endif
   #endif
}
#endif

int S4FUNCTION t4go2( TAG4 *t4, char *ptr, long rec_num )
{
   int  rc ;
   long rec ;

   #ifdef S4DEBUG
      if ( t4 == 0 || ptr == 0 || rec_num < 1 )
         e4severe( e4parm, E4_T4GO ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   rc = t4seek( t4, ptr, t4->header.key_len ) ;
   if ( rc )
      return rc ;

   for(;;)
   {
      rec = t4recno( t4 ) ;
      if (rec == rec_num )
         return 0 ;

      rc = (int)t4skip( t4, 1L ) ;
      if ( rc == -1 )
         return -1 ;
      if ( rc == 0 )
      {
         b4go_eof( t4block( t4 ) ) ;
         return r4found ;
      }

      if ( (*t4->cmp)( t4key_data( t4 )->value, ptr, t4->header.key_len ) )
         return r4found ;
   }
}

TAG4 *S4FUNCTION t4open( DATA4 *d4, INDEX4 *i4ndx, char *file_name )
{
   CODE4  *c4 ;
   INDEX4 *i4 ;
   char   buf[258], buffer[1024] ;
   #ifdef S4NDX
      char expr_buf[221] ;
   #else
      #ifdef S4CLIPPER
         char expr_buf[I4MAX_EXPR_SIZE], *ptr, garbage ;
      #endif
   #endif
   TAG4 *t4 ;
   int rc, len ;
   FILE4SEQ_READ seq_read ;
   #ifdef S4DEBUG
      int old_tag_err ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_T4OPEN ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 || file_name == 0 )
         e4severe( e4parm, E4_T4OPEN ) ;

      u4name_piece( buf, sizeof( buf ), file_name, 0, 0 ) ;
      old_tag_err = d4->code_base->tag_name_error ;
      d4->code_base->tag_name_error = 0 ;
      if ( d4tag( d4, buf ) )
      {
         e4( d4->code_base, e4info, E4_INFO_TAO ) ;
         d4->code_base->tag_name_error = old_tag_err ;
         return 0 ;
      }
      d4->code_base->tag_name_error = old_tag_err ;
      d4->code_base->error_code = 0 ;
   #endif

   if ( d4->code_base->error_code < 0 )
      return 0 ;

   c4 = d4->code_base ;

   if ( i4ndx == 0 )   /* must create an index for the tag */
   {
      if ( c4->index_memory == 0 )
      {
         c4->index_memory = mem4create( c4, c4->mem_start_index, sizeof( INDEX4 ), c4->mem_expand_index, 0 ) ;
         if ( c4->index_memory == 0 )
            return 0 ;
      }

      i4 = (INDEX4 *)mem4alloc( c4->index_memory ) ;
      if ( i4 == 0 )
      {
         e4( c4, e4memory, 0 ) ;
         return 0 ;
      }

      i4->code_base = c4 = d4->code_base ;
      u4name_piece( i4->alias, sizeof( i4->alias ), file_name, 0, 0 ) ;
    }
   else
      i4 = i4ndx ;

   if ( i4->block_memory == 0 )
      i4->block_memory = mem4create( c4, c4->mem_start_block,
                                     (sizeof(B4BLOCK)) + B4BLOCK_SIZE -
                                     (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[2])),
                                     c4->mem_expand_block, 0 ) ;

   if ( i4->block_memory == 0 )
   {
      i4close( i4 ) ;
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      return 0 ;
   }

   if ( c4->tag_memory == 0 )
   {
      c4->tag_memory = mem4create( c4, c4->mem_start_tag, sizeof(TAG4),
                                   c4->mem_expand_tag, 0 ) ;
      if ( c4->tag_memory == 0 )
      {
         if ( i4ndx == 0 )
            mem4free( c4->index_memory, i4 ) ;
         return 0 ;
      }
   }

   t4 = (TAG4 *)mem4alloc( c4->tag_memory ) ;
   if ( t4 == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      return 0 ;
   }

   u4ncpy( buf, file_name, sizeof( buf ) ) ;
   c4upper(buf) ;

   #ifdef S4NDX
      u4name_ext( buf, sizeof(buf), "NDX", 0 ) ;
   #else
      #ifdef S4CLIPPER
         u4name_ext( buf, sizeof(buf), "NTX", 0 ) ;
      #endif
   #endif

   rc = file4open( &t4->file, c4, buf, 1 ) ;
   if ( rc != 0 )
   {
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   file4seq_read_init( &seq_read, &t4->file, 0, buffer, 1024 ) ;
   #ifdef S4NDX
      if ( file4seq_read_all( &seq_read, &t4->header.root, sizeof(I4IND_HEAD_WRITE) ) < 0 )
      {
         if ( i4ndx == 0 )
            mem4free( c4->index_memory, i4 ) ;
         file4close( &t4->file ) ;
         mem4free( c4->tag_memory, t4 ) ;
         return 0 ;
      }

      #ifdef S4BYTE_SWAP
         t4->header.root = x4reverse_long( t4->header.root ) ;
         t4->header.eof = x4reverse_long( t4->header.eof ) ;
         t4->header.key_len = x4reverse_short( t4->header.key_len ) ;
         t4->header.keys_max = x4reverse_short( t4->header.keys_max ) ;
         t4->header.int_or_date = x4reverse_short( t4->header.int_or_date ) ;
         t4->header.group_len = x4reverse_short( t4->header.group_len ) ;
         t4->header.dummy = x4reverse_short( t4->header.dummy ) ;
         t4->header.unique = x4reverse_short( t4->header.unique ) ;
      #endif
      t4->header.type = 0 ;
   #else
      #ifdef S4CLIPPER
         if ( file4seq_read_all( &seq_read, &t4->header.sign, sizeof(I4IND_HEAD_WRITE) ) < 0 )
         {
            if ( i4ndx == 0 )
               mem4free( c4->index_memory, i4 ) ;
            file4close( &t4->file ) ;
            mem4free( c4->tag_memory, t4 ) ;
            return 0 ;
         }
      #endif
      #ifdef S4BYTE_SWAP
         t4->header.sign = x4reverse_short( t4->header.sign ) ;
         t4->header.version = x4reverse_short( t4->header.version ) ;
         t4->header.root = x4reverse_long( t4->header.root ) ;
         t4->header.eof = x4reverse_long( t4->header.eof ) ;
         t4->header.group_len = x4reverse_short( t4->header.group_len ) ;
         t4->header.key_len = x4reverse_short( t4->header.key_len ) ;
         t4->header.key_dec = x4reverse_short( t4->header.key_dec ) ;
         t4->header.keys_max = x4reverse_short( t4->header.keys_max ) ;
         t4->header.keys_half = x4reverse_short( t4->header.keys_half ) ;
      #endif
   #endif
   t4->header.header_offset = 0 ;

   /* Perform some checks */
   if ( t4->header.key_len   > I4MAX_KEY_SIZE    ||
        t4->header.key_len  <= 0  ||
      #ifdef S4CLIPPER
         t4->header.keys_max  != 2* t4->header.keys_half  ||
         t4->header.keys_half <= 0               ||
         t4->header.group_len != t4->header.key_len+ 8  ||
        (t4->header.sign != 0x6        &&  t4->header.sign != 0x106) )
      #else
        t4->header.key_len+8 > t4->header.group_len  ||
        t4->header.keys_max  < 4  ||
        t4->header.keys_max  > 50  ||
        t4->header.eof <= 0L )
      #endif
   {
      #ifdef S4DEBUG_DEV
         e4describe( c4, e4index, buf, "t4open()", "perform check failure" ) ;
      #endif
      e4( c4, e4index, buf ) ;
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      file4close( &t4->file ) ;
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   t4->index = i4 ;
   t4->code_base = c4 ;
   t4->cmp = u4memcmp ;
   t4->header.root = -1 ;
   t4->header.old_version = t4->header.version ;

   u4name_piece( t4->alias, sizeof(t4->alias), file_name, 0, 0 ) ;
   #ifdef S4UNIX
      c4upper( t4->alias ) ;
   #endif

   file4seq_read_all( &seq_read, expr_buf, sizeof( expr_buf ) - 1 ) ;
   c4trim_n( expr_buf, sizeof( expr_buf ) ) ;
   t4->expr = expr4parse( d4, expr_buf ) ;
   if ( t4->expr == 0 )
   {
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      file4close( &t4->file ) ;
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }
   #ifdef S4CLIPPER
      t4->expr->key_len = t4->header.key_len ;
      t4->expr->key_dec = t4->header.key_dec ;
   #endif

   len = expr4key_len( t4->expr ) ;
   if ( len < 0 )
   {
      e4describe( c4, e4info, E4_T4OPEN, E4_INFO_KEY, (char *)0 ) ;
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      file4close( &t4->file ) ;
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   #ifdef S4NDX
      t4->header.type = (char) expr4type(t4->expr) ;
      if ( t4->header.key_len != (short) len )
      {
         #ifdef S4DEBUG_DEV
            e4describe( c4, e4index, i4->file.name, "t4open()", "expression/header length mismatch" ) ;
         #endif
         e4( c4, e4index, i4->file.name ) ;
         if ( i4ndx == 0 )
            mem4free( c4->index_memory, i4 ) ;
         file4close( &t4->file ) ;
         mem4free( c4->tag_memory, t4 ) ;
         return 0 ;
      }
   #else
      #ifdef S4CLIPPER
         file4seq_read_all( &seq_read, &t4->header.unique, sizeof( t4->header.unique ) ) ;
         file4seq_read_all( &seq_read, &garbage, sizeof( garbage ) ) ;
         file4seq_read_all( &seq_read, &t4->header.descending, sizeof( t4->header.descending ) ) ;
         #ifdef S4BYTE_SWAP
            t4->header.unique = x4reverse_short( t4->header.unique ) ;
            t4->header.descending = x4reverse_short( t4->header.descending ) ;
         #endif
         file4seq_read_all( &seq_read, expr_buf, sizeof( expr_buf ) - 1 ) ;
         c4trim_n( expr_buf, sizeof( expr_buf ) ) ;
         if ( expr_buf[0] != 0 )
         {
            t4->filter = expr4parse( d4, expr_buf ) ;
            if ( t4->filter != 0 )
            {
               len = expr4key( t4->filter, &ptr ) ;
               if ( len < 0 )
               {
                  if ( i4ndx == 0 )
                     mem4free( c4->index_memory, i4 ) ;
                  file4close( &t4->file ) ;
                  mem4free( c4->tag_memory, t4 ) ;
                  return 0 ;
               }
               if ( expr4type( t4->filter ) != 'L' )
               {
                  if ( i4ndx == 0 )
                     mem4free( c4->index_memory, i4 ) ;
                  file4close( &t4->file ) ;
                  mem4free( c4->tag_memory, t4 ) ;
                  #ifdef S4DEBUG_DEV
                     e4describe( c4, e4index, E4_INDEX_FIL, i4->file.name, "Filter expression not logical" ) ;
                  #endif
                  e4describe( c4, e4index, E4_INDEX_FIL, i4->file.name, (char *) 0 ) ;
                  return 0 ;
               }
            }
         }

      #endif
   #endif

   if( t4->header.unique )
      t4->unique_error = c4->default_unique_error ;
   l4add( &i4->tags, t4 ) ;   /* add the tag to its index list */

   #ifdef S4NDX
      t4init_seek_conv(t4,t4->header.type) ;
   #else
      #ifdef S4CLIPPER
         t4init_seek_conv( t4, (char) expr4type(t4->expr) ) ;
      #endif
   #endif

   if ( i4->block_memory == 0 )
      i4->block_memory = mem4create( c4, c4->mem_start_block,
                                     (sizeof(B4BLOCK)) + B4BLOCK_SIZE -
                                     (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[2])),
                                     c4->mem_expand_block, 0 ) ;

   if ( i4->block_memory == 0 )
   {
      e4( c4, e4memory, E4_T4OPEN ) ;
      if ( i4ndx == 0 )
         mem4free( c4->index_memory, i4 ) ;
      file4close( &t4->file ) ;
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   if ( i4ndx == 0 )
   {
      i4->data = d4 ;
      l4add( &d4->indexes, i4 ) ;
   }

   #ifndef S4OPTIMIZE_OFF
      file4optimize( &t4->file, c4->optimize, OPT4INDEX ) ;
   #endif

   return t4 ;
}

#ifndef S4OFF_WRITE
#ifdef S4CLIPPER
int S4FUNCTION t4shrink( TAG4 *t4, long block_no )
{
   t4->header.eof = block_no * 512 ;
   return 0 ;
}
#endif

int S4FUNCTION t4update_header( TAG4 *t4 )
{
   #ifdef S4BYTE_SWAP
      I4IND_HEAD_WRITE swap ;
   #endif
   if ( t4->index->code_base->error_code < 0 )
      return -1 ;

   if ( t4->header.old_version != t4->header.version )
   {
      #ifdef S4NDX
         #ifdef S4BYTE_SWAP
            swap.root = x4reverse_long( t4->header.root ) ;
            swap.eof = x4reverse_long( t4->header.eof ) ;
            swap.key_len = x4reverse_short( t4->header.key_len ) ;
            swap.keys_max = x4reverse_short( t4->header.keys_max ) ;
            swap.int_or_date = x4reverse_short( t4->header.int_or_date ) ;
            swap.group_len = x4reverse_short( t4->header.group_len ) ;
            swap.dummy = x4reverse_short( t4->header.dummy ) ;
            swap.unique = x4reverse_short( t4->header.unique ) ;

            if ( file4write(&t4->file, 0L, &swap, sizeof(I4IND_HEAD_WRITE)) < 0) return -1 ;

            t4->header.version = x4reverse_long( t4->header.version ) ;
            if ( file4write(&t4->file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE,
                 &t4->header.version, sizeof(t4->header.version)) < 0) return -1 ;
            t4->header.version = x4reverse_long(t4->header.version ) ;
         #else
            if ( file4write(&t4->file, 0L, &t4->header.root, sizeof(I4IND_HEAD_WRITE) ) < 0 )
               return -1 ;
            if ( file4write(&t4->file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE,
                 &t4->header.version, sizeof(t4->header.version)) < 0) return -1 ;
         #endif
      #else
         #ifdef S4CLIPPER
            #ifdef S4BYTE_SWAP
               swap.sign = x4reverse_short( t4->header.sign ) ;
               swap.version = x4reverse_short( t4->header.version ) ;
               swap.root = x4reverse_long( t4->header.root ) ;
               swap.eof = x4reverse_long( t4->header.eof ) ;
               swap.group_len = x4reverse_short( t4->header.group_len ) ;
               swap.key_len = x4reverse_short( t4->header.key_len ) ;
               swap.key_dec = x4reverse_short( t4->header.key_dec ) ;
               swap.keys_max = x4reverse_short( t4->header.keys_max ) ;
               swap.keys_half = x4reverse_short( t4->header.keys_half ) ;
   
               if ( file4write(&t4->file, 0L, &swap, 2 * sizeof( long ) + 2 * sizeof( short ) ) < 0)
                  return -1 ;
   
               t4->header.unique = x4reverse_short( t4->header.unique ) ;
               if ( file4write(&t4->file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE, &t4->header.unique, sizeof(t4->header.unique)) < 0)
                       return -1 ;
               t4->header.unique = x4reverse_short( t4->header.unique ) ;

               t4->header.descending = x4reverse_short( t4->header.descending ) ;
               if ( file4write(&t4->file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE + sizeof(t4->header.unique) + 1, &t4->header.descending, sizeof(t4->header.descending)) < 0)
                       return -1 ;
               t4->header.descending = x4reverse_short( t4->header.descending ) ;
            #else
               if ( file4write(&t4->file, 0L, &t4->header.sign, 2 * sizeof( long ) + 2 * sizeof( short ) ) < 0)
                  return -1 ;
               if ( file4write(&t4->file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE, &t4->header.unique, sizeof(t4->header.unique)) < 0)
                  return -1 ;
               if ( file4write(&t4->file, sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE + sizeof(t4->header.unique) + 1, &t4->header.descending, sizeof(t4->header.descending)) < 0)
                  return -1 ;
            #endif
         #endif
      #endif
      t4->header.old_version = t4->header.version ;
   }
   return 0;
}
#endif  /* S4OFF_WRITE */

int S4FUNCTION t4do_version_check( TAG4 *t4, int do_seek )
{
   #ifndef S4SINGLE
      int rc, need_seek ;
      B4BLOCK *b4 ;

      #ifdef S4DEBUG
         if ( t4 == 0 )
            e4severe( e4parm, E4_T4DO_VERSION ) ;
      #endif

      if ( t4->code_base->error_code < 0 )
         return -1 ;

      if ( t4->file.is_exclusive || t4->file.is_read_only )
         return 0 ;

      #ifndef S4OPTIMIZE_OFF
         /* make sure read from disk */
         if ( t4->file.do_buffer )
            t4->code_base->opt.force_current = 1 ;
      #endif

      #ifdef S4NDX
         rc = file4read_all( &t4->file,  sizeof(I4IND_HEAD_WRITE) + I4MAX_EXPR_SIZE,
                             &t4->header.version, sizeof(t4->header.version ) ) ;
         #ifndef S4OPTIMIZE_OFF
            if ( t4->file.do_buffer )
               t4->code_base->opt.force_current = 0 ;
         #endif
         if ( rc < 0 )
            return rc ;

         #ifdef S4BYTE_SWAP
            t4->header.version = x4reverse_long( t4->header.version ) ;
         #endif
      #else
         rc = file4read_all( &t4->file, 0L, &t4->header.sign , 2 * sizeof( short ) + 2 * sizeof( long ) ) ;
         #ifndef S4OPTIMIZE_OFF
            if ( t4->file.do_buffer )
               t4->code_base->opt.force_current = 0 ;
         #endif
         if ( rc < 0 )
            return rc ;
         #ifdef S4BYTE_SWAP
            t4->header.sign = x4reverse_short( t4->header.sign ) ;
            t4->header.version = x4reverse_short( t4->header.version ) ;
            t4->header.root = x4reverse_long( t4->header.root ) ;
            t4->header.eof = x4reverse_long( t4->header.eof ) ;
            t4->header.group_len = x4reverse_short( t4->header.group_len ) ;
            t4->header.key_len = x4reverse_short( t4->header.key_len ) ;
            t4->header.key_dec = x4reverse_short( t4->header.key_dec ) ;
            t4->header.keys_max = x4reverse_short( t4->header.keys_max ) ;
            t4->header.keys_half = x4reverse_short( t4->header.keys_half ) ;
         #endif
      #endif

      if ( t4->header.version == t4->header.old_version )
         return 0 ;

      t4->header.old_version = t4->header.version ;

      /* remember the old position */
      need_seek = 0 ;
      if ( do_seek )
      {
         b4 = t4block( t4 ) ;
         if ( b4 != 0 )
            if ( b4leaf( b4 ) && b4->n_keys != 0 )
            {
               memcpy( t4->code_base->saved_key, b4key( b4, b4->key_on ), t4->header.key_len + 2 * sizeof( long ) ) ;
               need_seek = 1 ;
            }
      }

      if ( t4free_all( t4 ) < 0 )  /* Should be a memory operation only */
         e4severe( e4result, E4_T4DO_VERSION ) ;

      if ( need_seek )
         t4go( t4, ((B4KEY_DATA *)t4->code_base->saved_key)->value, ((B4KEY_DATA *)t4->code_base->saved_key)->num ) ;

   #endif
   return 0;
}

int S4FUNCTION t4version_check( TAG4 *t4, int do_seek )
{
   #ifndef S4OPTIMIZE_OFF
      if ( t4->index->file.do_buffer == 0 )
   #endif
   #ifndef S4SINGLE
         if ( t4->file_locked == 0 )
   #endif
            return t4do_version_check( t4, do_seek ) ;
   return 0 ;
}

#endif  /* N4OTHER */
#endif  /* S4INDEX_OFF */


#ifdef S4VB_DOS
#ifdef N4OTHER

TAG4 S4PTR* S4FUNCTION t4open_v(DATA4 *d4, char *name)
   {
      return t4open( d4, 0, c4str(name) ) ;
   }

#endif
#endif
