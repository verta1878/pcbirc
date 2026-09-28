/* b4block.c  (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TURBOC__ */
#endif  /* S4UNIX */

#ifndef S4INDEX_OFF

#ifdef S4FOX
   #define S4REVERSE_FUNC
#endif
#ifdef S4BYTE_SWAP
   #define S4REVERSE_FUNC
#endif

#ifdef S4INDEX_VERIFY
   int S4FUNCTION b4verify( B4BLOCK *b4 )
   {
      int i, j, type ;
      long val ;

      #ifdef S4CLIPPER
    /* block internal verification... */
    for ( i = 0 ; i < b4->tag->header.keys_max ; i++ )
       for ( j = i + 1 ; j <= b4->tag->header.keys_max ; j++ )
          if ( b4->pointers[i] == b4->pointers[j] )
          {
                  #ifdef S4DEBUG_DEV
                     e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4verify()", "b4 pointer duplication error" ) ;
                  #endif
                  e4( b4->tag->code_base, e4index, E4_INFO_CIF ) ;
                  return -1 ;
               }

         /* validate key count */
         if ( b4->n_keys < b4->tag->header.keys_half && b4->tag->header.root / 512 != b4->file_block )
            return e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4verify()", E4_RESULT_CII ) ;
      #endif

/*      #ifdef S4FOX */
/*          */
/*         if ( b4->node_hdr.free_space != B4BLOCK_SIZE - sizeof( B4STD_HEADER ) */
/*              - sizeof( B4NODE_HEADER ) -  ) */
/*            return e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4verify()", E4_RESULT_CII ) ; */
/*      #endif */

      /* all keys in a given block should be of the same type... - branch = 1 */
      #ifndef S4FOX
      type = b4leaf( b4 ) ? 0 : 1 ;
      #endif

      /* key order verification */
      #ifndef S4FOX
        for ( i = 0 ; i <= b4->n_keys ; i++ )
         {
            #ifdef S4CLIPPER
               val = b4key( b4, i )->pointer ;
               if ( ( b4key( b4, i )->pointer ? 1 : 0 ) != type )
            #endif
            #ifdef S4NDX
               if ( ( b4key( b4, i )->pointer ? 1 : 0 ) != type )
            #endif
            #ifdef S4MDX
               if ( ( b4key( b4, i )->pointer ? 1 : 0 ) != type )
            #endif
            {
               #ifdef S4DEBUG_DEV
                  e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4verify()", "b4 branch/leaf block type inconsistent" ) ;
               #endif
               e4( b4->tag->code_base, e4index, E4_INFO_CIF ) ;
               return -1 ;
            }
         }
      #endif

      #ifdef S4FOX
         for ( i = 0 ; i < b4->header.n_keys - 1 ; i++ )
         {
            if ( (memcmp)( b4key_key( b4, i ), b4key_key( b4, i + 1 ), b4->tag->header.key_len ) > 0 )
      #else
         for ( i = 0 ; i < b4->n_keys - 1 ; i++ )
         {
            if ( (*b4->tag->cmp)( b4key_key( b4, i ), b4key_key( b4, i + 1 ), b4->tag->header.key_len ) > 0 )
      #endif
         {
            #ifdef S4DEBUG_DEV
               e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4verify()", "b4 block ordering invalid" ) ;
            #endif
            e4( b4->tag->code_base, e4index, E4_INFO_CIF ) ;
            return -1 ;
         }
      }

   return 0 ;
   }
#endif

#ifdef S4REVERSE_FUNC
short S4FUNCTION x4reverse_short( short val )
{
   unsigned char out[2], *at_val ;
   at_val = (unsigned char *)&val ;

   out[0] = at_val[1] ;
   out[1] = at_val[0] ;

   return (*(short *)out ) ;
}

long S4FUNCTION x4reverse_long( long val )
{
   unsigned char out[4], *at_val ;
   at_val = (unsigned char *)&val ;

   out[0] = at_val[3] ;
   out[1] = at_val[2] ;
   out[2] = at_val[1] ;
   out[3] = at_val[0] ;

   return *(long *)out ;
}
#endif

B4BLOCK * S4FUNCTION b4alloc( TAG4 *t4, long fb )
{
   B4BLOCK *b4 ;
   #ifdef S4CLIPPER
      short offset ;
      int i ;
   #endif  /* S4CLIPPER */

   b4 = (B4BLOCK *)mem4alloc2( t4->index->block_memory, t4->code_base ) ;
   if ( b4 == 0 )
   {
      e4( t4->code_base, e4memory, E4_B4ALLOC ) ;
      return 0 ;
   }

   b4->tag = t4 ;
   b4->file_block = fb ;

   #ifdef S4CLIPPER
      offset = ( b4->tag->header.keys_max + 2 +
         (( b4->tag->header.keys_max / 2) * 2 != b4->tag->header.keys_max ) )
         * sizeof(short) ;
      for ( i = 0 ; i <= b4->tag->header.keys_max ; i++ )
         b4->pointers[i] = (short)(( b4->tag->header.group_len * i )) + offset ;
      b4->data = (B4KEY_DATA *) ((char *)&b4->n_keys + b4->pointers[0]) ;  /* first entry */
   #endif  /* S4CLIPPER */

   #ifdef S4FOX
      b4->built_key = (B4KEY_DATA *)u4alloc_er( t4->code_base, sizeof(long) + t4->header.key_len + 1 ) ;
      b4->built_on = -1 ;
   #endif  /* S4FOX */

   return b4 ;
}

void S4FUNCTION b4free( B4BLOCK *b4 )
{
   #ifdef S4FOX
      #ifdef S4DEBUG
         if ( b4->changed )
            e4severe( e4info, E4_B4FREE ) ;
      #endif  /* S4DEBUG */
      u4free( b4->built_key ) ;
   #endif /* S4FOX */

   mem4free( b4->tag->index->block_memory, b4 ) ;
}

#ifdef S4NDX

#ifndef S4OFF_WRITE
int S4FUNCTION b4find( TAG4 *tag, long f_block, B4BLOCK *out_block )
{
   B4BLOCK *cur_block ;

   cur_block = (B4BLOCK *)l4first( &tag->blocks ) ;
   if ( cur_block != 0 )
      do
      {
         if ( cur_block->file_block == f_block )
         {
            memcpy( &(out_block->changed), &(cur_block->changed), (B4BLOCK_SIZE + 2*sizeof(int)) ) ;
            out_block->changed = 0 ;
            return( 0 ) ;
         }
         cur_block = (B4BLOCK *)l4next( &tag->blocks, cur_block ) ;
      } while ( cur_block != 0 ) ;

   cur_block = (B4BLOCK *)l4first( &tag->saved ) ;

   if ( cur_block != 0 )
      do
      {
         if ( cur_block->file_block == f_block )
         {
            memcpy( &(out_block->changed), &(cur_block->changed), (B4BLOCK_SIZE + 2*sizeof(int)) ) ;
            out_block->changed = 0 ;
            return 0 ;
         }
         cur_block = (B4BLOCK *)l4next( &tag->saved, cur_block ) ;
      } while ( cur_block != 0 ) ;

   return -1 ;
}

int S4FUNCTION b4get_last_key( B4BLOCK *b4, char *key_info )
{
   int rc ;
   long block_down ;
   B4BLOCK *temp_block ;
   char *key_data ;

   #ifdef S4DEBUG
      if ( b4leaf( b4 ) )
         e4severe( e4info, E4_B4GET_LK ) ;
   #endif  /* S4DEBUG */

   block_down = b4key( b4, b4->key_on )->pointer ;

   temp_block = b4alloc( b4->tag, block_down ) ;
   if ( temp_block == 0 )
      return -1 ;

   do
   {
      #ifdef S4DEBUG
         if ( block_down <= 0L )
            e4severe( e4info, E4_B4GET_LK ) ;
      #endif  /* S4DEBUG */

      if ( b4find( b4->tag, block_down, temp_block ) < 0 )
      {
         if ( file4len( &b4->tag->file ) <= I4MULTIPLY * block_down )
         {
            b4free(temp_block) ;
            return -1 ;
         }
         else
         {
            if ( file4read_all( &b4->tag->file, I4MULTIPLY * block_down, &temp_block->n_keys, B4BLOCK_SIZE ) < 0 )
            {
               b4free(temp_block) ;
               return -1 ;
            }
            temp_block->file_block = block_down ;
         }
      }

      temp_block->key_on = b4lastpos( temp_block ) ;
      block_down = b4key( temp_block, temp_block->key_on )->pointer ;
   } while( !( b4leaf( temp_block ) ) ) ;

   key_data = (char *)b4key_key( temp_block, temp_block->key_on ) ;

   if( strcmp( key_info, key_data ) != 0 )
   {
      strcpy( key_info, key_data ) ;
      rc = 1 ;
   }
   else
      rc = 0 ;

   b4free( temp_block ) ;

   return rc ;
}
#endif  /* S4OFF_WRITE */

#endif  /* S4NDX */

#ifdef S4MDX

/* S4MDX */
#ifndef S4OFF_WRITE
int S4FUNCTION b4flush( B4BLOCK *b4 )
{
   int rc ;
   INDEX4 *i4 ;
   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int i ;
      long long_val ;
      short short_val ;
   #endif  /* S4BYTE_SWAP */

   if ( b4->changed )
   {
      #ifdef S4INDEX_VERIFY
         if ( b4verify( b4 ) == -1 )
            e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4flush()", "" ) ;
      #endif
      i4 = b4->tag->index ;

      #ifdef S4BYTE_SWAP
         swap = (char *) u4alloc_er( b4->tag->code_base, b4->tag->index->header.block_rw ) ;
         if ( swap == 0 )
            return -1 ;

         memcpy( (void *)swap, (void *)&b4->n_keys, b4->tag->index->header.block_rw ) ;
                          /* position swap_ptr at beginning of B4KEY's */
         swap_ptr = swap ;
         swap_ptr += 6 + sizeof(short) ;
                          /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (*(short *)swap) ; i++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
            swap_ptr += b4->tag->header.group_len ;
         }
                          /* mark last_pointer */
         if ( !b4leaf( b4 ) )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
         }
                          /* swap the num_keys value */
         short_val = x4reverse_short( *(short *)swap ) ;
         memcpy( swap, (void *) &short_val, sizeof(short) ) ;

         rc = file4write( &i4->file, (long)b4->file_block*I4MULTIPLY, swap, i4->header.block_rw) ;
         u4free( swap ) ;
      #else
         rc = file4write( &i4->file, (long)b4->file_block*I4MULTIPLY, &b4->n_keys, i4->header.block_rw) ;
      #endif  /* S4BYTE_SWAP */

      if ( rc < 0 )
         return rc ;
      b4->changed = 0 ;
   }
   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4MDX */
void S4FUNCTION b4go_eof( B4BLOCK *b4 )
{
   b4->key_on = b4->n_keys ;
}

/* S4MDX */
#ifndef S4OFF_WRITE
void S4FUNCTION b4insert( B4BLOCK *b4, void *k, long r )
{
   int left_len ;
   B4KEY_DATA *data_ptr, *next_ptr ;

   #ifdef S4DEBUG
      if ( k == 0 || r <= 0L )
         e4severe( e4parm, E4_B4INSERT ) ;
   #endif  /* S4DEBUG */

   data_ptr = b4key( b4, b4->key_on ) ;
   next_ptr = b4key( b4, b4->key_on+1 ) ;
   left_len = b4->tag->index->header.block_rw - ( b4->key_on + 1 ) * b4->tag->header.group_len - sizeof(short) - sizeof(char[6]) ;

   #ifdef S4DEBUG
      if ( b4->key_on < 0  ||  b4->key_on > b4->n_keys  ||  left_len < 0 )
         e4severe( e4info, E4_B4INSERT ) ;
   #endif  /* S4DEBUG */

   memmove( next_ptr, data_ptr, left_len ) ;
   b4->n_keys++ ;
   memcpy( data_ptr->value, k, b4->tag->header.key_len ) ;
   memcpy( (void *)&data_ptr->num, (void *)&r, sizeof(r) ) ;
   b4->changed = 1 ;
}
#endif /* S4OFF_WRITE */

/* S4MDX */
B4KEY_DATA *S4FUNCTION b4key( B4BLOCK *b4, int i_key )
{
   return  (B4KEY_DATA *) ((char *) &b4->info.num + b4->tag->header.group_len * i_key) ;
}

/* S4MDX */
unsigned char *S4FUNCTION b4key_key( B4BLOCK *b4, int i_key )
{
   return (unsigned char *)( ( (B4KEY_DATA *)( (char *)&b4->info.num + b4->tag->header.group_len * i_key ) )->value ) ;
}

/* S4MDX */
int S4FUNCTION b4lastpos( B4BLOCK *b4 )
{
   if ( b4leaf(b4) )
      return b4->n_keys - 1 ;
   else
      return b4->n_keys ;
}

/* S4MDX */
int S4FUNCTION b4leaf( B4BLOCK *b4 )
{
   return( b4key( b4, b4->n_keys )->num == 0L ) ;
}

/* S4MDX */
long S4FUNCTION b4recno( B4BLOCK *b4, int i )
{
   return b4key( b4, i )->num ;
}

/* S4MDX */
#ifndef S4OFF_WRITE
void S4FUNCTION b4remove( B4BLOCK *b4 )
{
   B4KEY_DATA *key_cur, *key_next ;
   int left_len ;

   key_cur = b4key( b4, b4->key_on ) ;
   key_next = b4key( b4, b4->key_on + 1 ) ;

   left_len = b4->tag->index->header.block_rw - sizeof(b4->n_keys) - sizeof(b4->dummy)
            - (b4->key_on+1) * b4->tag->header.group_len ;

   #ifdef S4DEBUG
      if ( b4->key_on < 0  ||  b4->key_on > b4lastpos(b4) )
         e4severe( e4info, E4_B4REMOVE ) ;
   #endif  /* S4DEBUG */

   if ( left_len > 0 )
      memmove( key_cur, key_next, left_len ) ;

   b4->n_keys-- ;
   b4->changed = 1 ;

   if ( b4leaf( b4 ) )
      memset( b4key_key( b4, b4->n_keys ), 0, b4->tag->header.key_len ) ;
   #ifdef S4DEBUG
      else
         if ( b4->n_keys < b4->tag->header.keys_max )
            memset( b4key_key( b4, b4->n_keys ), 0, b4->tag->header.key_len ) ;
   #endif
}
#endif /* S4OFF_WRITE */

/* S4MDX */
int S4FUNCTION b4seek( B4BLOCK *b4, char *search_value, int len )
{
   int rc, key_lower, key_upper, save_rc, key_cur ;
   S4CMP_FUNCTION *cmp;

   /* key_cur must be between  key_lower and  key_upper */
   key_lower = -1 ;
   key_upper = b4->n_keys ;

   cmp = b4->tag->cmp ;

   if ( key_upper == 0 )
   {
      b4->key_on = 0 ;
      return r4after ;
   }

   save_rc = 1 ;

   for(;;)  /* Repeat until the key is found */
   {
      key_cur  = (key_upper + key_lower) / 2  ;
      rc = (*cmp)( b4key_key(b4,key_cur), search_value, len ) ;

      if ( rc >= 0 )
      {
         key_upper = key_cur ;
         save_rc = rc ;
      }
      else
         key_lower = key_cur ;

      if ( key_lower >= (key_upper-1) )  /* then there is no exact match */
      {
         b4->key_on =  key_upper ;
         if ( save_rc )
            return r4after ;
         return 0 ;
      }
   }
}

/* S4MDX */
int S4FUNCTION b4skip( B4BLOCK *b4, long n )
{
   int num_left ;

   if ( n > 0 )
   {
      num_left = b4->n_keys - b4->key_on ;
      if ( b4leaf(b4) )
         if ( num_left != 0 )
            num_left -- ;
   }
   else
      num_left = -b4->key_on ;

   if ( ( n <= 0L ) ?  ((long) num_left <= n)  :  ((long) num_left >= n)  )
   {
      b4->key_on = b4->key_on+ (int)n ;
      return (int)n ;
   }
   else
   {
      b4->key_on = b4->key_on + num_left ;
      return num_left ;
   }
}
#endif  /* S4MDX */

#ifdef S4FOX

/* S4FOX */
void S4FUNCTION b4leaf_init( B4BLOCK *b4 )
{
   TAG4 *t4 ;
   int t_len ;
   unsigned c_len, key_len ;
   unsigned long ff, r_len ;

   t4 = b4->tag ;
   key_len = t4->header.key_len ;
   ff = 0xFFFFFFFF ;

   for ( c_len = 0 ; key_len ; key_len >>= 1, c_len++ ) ;

   key_len = t4->header.key_len ;
   b4->node_hdr.trail_cnt_len = b4->node_hdr.dup_cnt_len = (unsigned char)c_len ;

   b4->node_hdr.trail_byte_cnt = (unsigned char)((unsigned char)0xFF >> (8 - c_len % 8) ) ;
   b4->node_hdr.dup_byte_cnt = b4->node_hdr.trail_byte_cnt ;

   r_len = d4reccount( b4->tag->index->data ) ;

   for ( c_len = 0 ; r_len ; r_len>>=1, c_len++ ) ;

   b4->node_hdr.rec_num_len = (unsigned char) (c_len + (( 8 - ( 2 * b4->node_hdr.trail_cnt_len % 8 )) % 8)) ;
   if ( b4->node_hdr.rec_num_len < 12 )
      b4->node_hdr.rec_num_len = 12 ;

   for( t_len = b4->node_hdr.rec_num_len + b4->node_hdr.trail_cnt_len + b4->node_hdr.dup_cnt_len ;
        (t_len / 8)*8 != t_len ; t_len++, b4->node_hdr.rec_num_len++ ) ;  /* make at an 8-bit offset */

   b4->node_hdr.rec_num_mask = ff >> ( sizeof(long)*8 - b4->node_hdr.rec_num_len ) ;
   b4->node_hdr.info_len = (unsigned char)((b4->node_hdr.rec_num_len + b4->node_hdr.trail_cnt_len + b4->node_hdr.dup_cnt_len) / 8) ;
   b4->node_hdr.free_space = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;
}

/* S4FOX */
int S4FUNCTION b4calc_blanks( char *key_val, int len, char p_char )
{
   int a ;

   for ( a = len ; a > 0; a-- )
      if ( key_val[a-1] != p_char )
         return ( len - a ) ;
   return len ;  /* all blanks */
}

/* S4FOX */
int S4FUNCTION b4calc_dups( char *ptr1, char *ptr2, int len )
{
   int a ;
   for ( a = 0 ; a < len; a++ )
      if ( ptr1[a] != ptr2[a] )
         return a ;
   return len ;  /* all duplicates */
}

/* S4FOX */
long S4FUNCTION x4recno( B4BLOCK *b4, int num_in_block )
{
   unsigned long *l_ptr = (unsigned long *)( b4->data + num_in_block * b4->node_hdr.info_len ) ;
   return ( *l_ptr & b4->node_hdr.rec_num_mask ) ;
}

/* S4FOX */
int S4FUNCTION x4dup_cnt( B4BLOCK *b4, int num_in_block )
{
   int pos ;
   unsigned long *l_ptr ;

   if ( b4->node_hdr.info_len > 4 )  /* > size of long, so must do careful shifting and copying */
   {
      #ifdef S4DEBUG
         if ( b4->node_hdr.rec_num_len <= 16 )
            e4severe( e4info, E4_INFO_CIB ) ;
      #endif  /* S4DEBUG */
      l_ptr = (unsigned long *)( b4->data + num_in_block * b4->node_hdr.info_len + 2 ) ;
      pos = b4->node_hdr.rec_num_len - 16 ;
   }
   else
   {
      l_ptr = (unsigned long *)( b4->data + num_in_block * b4->node_hdr.info_len ) ;
      pos = b4->node_hdr.rec_num_len ;
   }

   return (int)( ( *l_ptr >> pos ) & b4->node_hdr.dup_byte_cnt ) ;
}

/* S4FOX */
int S4FUNCTION x4trail_cnt( B4BLOCK *b4, int num_in_block )
{
   int pos ;
   unsigned long * l_ptr ;

   if ( b4->node_hdr.info_len > 4 )  /* > size of long, so must do careful shifting and copying */
   {
      #ifdef S4DEBUG
         if ( b4->node_hdr.rec_num_len <= 16 )
            e4severe( e4info, E4_INFO_CIB ) ;
      #endif  /* S4DEBUG */
      l_ptr = (unsigned long *)( b4->data + num_in_block * b4->node_hdr.info_len + 2 ) ;
      pos = b4->node_hdr.rec_num_len - 16 + b4->node_hdr.dup_cnt_len ;
   }
   else
   {
      l_ptr = (unsigned long *)( b4->data + num_in_block * b4->node_hdr.info_len ) ;
      pos = b4->node_hdr.rec_num_len + b4->node_hdr.dup_cnt_len;
   }

   return (int)( ( *l_ptr >> pos ) & b4->node_hdr.trail_byte_cnt ) ;
}

/* S4FOX */
#ifndef S4OFF_WRITE
void S4FUNCTION x4put_info( B4NODE_HEADER *b4node_hdr, void *buffer, long rec, int trail, int dup_cnt )
{
   int pos ;
   unsigned char *buf ;
   unsigned long *l_ptr ;

   buf = (unsigned char *) buffer ;
   l_ptr = (unsigned long *)buf ;

   memset( buf, 0, 6 ) ;
   *l_ptr = rec & b4node_hdr->rec_num_mask ;

   if ( b4node_hdr->info_len > 4 )  /* > size of long, so must do careful shifting and copying */
   {
      #ifdef S4DEBUG
         if ( b4node_hdr->rec_num_len <= 16 )
            e4severe( e4info, E4_INFO_CIB ) ;
      #endif  /* S4DEBUG */
      l_ptr = (unsigned long *)( buf + 2 ) ;  /* start at new pos */
      pos = b4node_hdr->rec_num_len - 16 ;
   }
   else
      pos = b4node_hdr->rec_num_len ;

   *l_ptr |= ((unsigned long)dup_cnt) << pos ;
   pos += b4node_hdr->dup_cnt_len ;
   *l_ptr |= ((unsigned long)trail) << pos ;
}

/* S4FOX */
int S4FUNCTION b4insert_leaf( B4BLOCK *b4, void *vkey_data, long rec )
{
   int left_blanks, right_blanks, left_dups, right_dups, left_len, right_len, reqd_len, mov_len, old_rec, rc, old_right_dups, extra_dups, save_dups ;
   unsigned char buffer[6], i_len ;
   char *key_data, *info_pos, a, b ;
   int key_len ;
   unsigned int rec_len ;
   unsigned long mask ;

   i_len = b4->node_hdr.info_len ;
   key_data = (char *)vkey_data ;
   key_len = b4->tag->header.key_len ;
   b4->built_on = -1 ;

   if ( b4->header.n_keys == 0 )
   {
      if( b4->node_hdr.free_space == 0 )  /* block needs initialization */
      {
         b4leaf_init( b4 ) ;
         i_len = b4->node_hdr.info_len ;
      }
      left_blanks = b4calc_blanks( key_data, key_len, b4->tag->p_char ) ;
      left_dups = 0 ;
      reqd_len = key_len - left_blanks ;
      b4->key_on = 0 ;
      b4->cur_pos = ((char *)&b4->header) + B4BLOCK_SIZE - reqd_len ;
      memcpy( b4->cur_pos, key_data, reqd_len ) ;  /* key */
      x4put_info( &b4->node_hdr, buffer, rec, left_blanks, 0 ) ;
      memcpy( b4->data, buffer, i_len ) ;
   }
   else
   {
      /* if the record is > than the mask, must reset the block with new parameters: */
      b = sizeof( long ) * 8 ;
      mask = 0x01L << (b-1) ;    /* set leftmost bit  */

      for( rec_len = 0, a = 0 ; a < b ; a++ )
      {
         if ( rec & mask )
         {
            rec_len = b - a ;
            break ;
         }
         mask >>= 1 ;
      }

      if ( rec_len > b4->node_hdr.rec_num_len )
      {
         old_rec = b4->key_on ;
         save_dups = b4->cur_dup_cnt ;
         rc = b4reindex( b4 ) ;
         if (rc )
            return rc ;
         i_len = b4->node_hdr.info_len ;
         b4top( b4 ) ;
         b4skip( b4, old_rec ) ;
         b4->cur_dup_cnt = save_dups ;
      }

      left_blanks = b4calc_blanks( key_data, key_len, b4->tag->p_char ) ;

      if ( b4->key_on == b4->header.n_keys )  /* at end */
      {
         left_dups = b4->cur_dup_cnt ;
         reqd_len = key_len - left_blanks - b4->cur_dup_cnt ;
         if ( b4->node_hdr.free_space < ( reqd_len + i_len ) )  /* no room to add */
            return 1 ;
         b4->cur_pos -= reqd_len ;
         memcpy( b4->cur_pos, key_data + b4->cur_dup_cnt, reqd_len ) ;  /* key */
         x4put_info( &b4->node_hdr, buffer, rec, left_blanks, left_dups ) ;
         memcpy( b4->data + b4->key_on * i_len , buffer, i_len ) ;
      }
      else
      {
         right_blanks = x4trail_cnt( b4, b4->key_on ) ;

         if ( b4->key_on == 0 )   /* insert at top */
         {
            old_right_dups = 0 ;
            right_dups = b4calc_dups( key_data, b4->cur_pos, key_len - ( right_blanks > left_blanks ? right_blanks : left_blanks ) ) ;
            extra_dups = right_dups ;
            left_dups = 0 ;
         }
         else /* insert in middle of block */
         {
            old_right_dups = x4dup_cnt( b4, b4->key_on) ;
            extra_dups = b4calc_dups( key_data + old_right_dups, b4->cur_pos, key_len - ( right_blanks > left_blanks ? right_blanks : left_blanks ) - old_right_dups ) ;
            right_dups = old_right_dups + extra_dups ;
            left_dups = b4->cur_dup_cnt ;
         }

         right_len = key_len - right_blanks - right_dups ;
         left_len = key_len - left_dups - left_blanks ;
         reqd_len = left_len - extra_dups ;

         #ifdef S4DEBUG
            if ( reqd_len < 0 )
               e4severe( e4info, E4_INFO_CIB ) ;
         #endif  /* S4DEBUG */

         if ( b4->node_hdr.free_space < ( reqd_len + i_len ) )  /* no room to add */
            return 1 ;

         if ( reqd_len != 0 )  /* shift data over */
         {
            mov_len = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - i_len * b4->header.n_keys
                      - sizeof( B4NODE_HEADER ) - b4->node_hdr.free_space
                      - ( ( (char *)&b4->header ) + B4BLOCK_SIZE - b4->cur_pos ) ;

            #ifdef S4DEBUG
               if ( mov_len < 0 )
                  e4severe( e4info, E4_INFO_CIB ) ;
            #endif  /* S4DEBUG */

            /* move and put keys */
            memmove( b4->cur_pos - mov_len - reqd_len, b4->cur_pos - mov_len, mov_len ) ;
         }
         b4->cur_pos += ( key_len - right_blanks - old_right_dups ) ;
         memcpy( b4->cur_pos - left_len - right_len, b4->cur_pos - ( key_len - right_dups - right_blanks ), right_len ) ;
         b4->cur_pos -= left_len ;
         memcpy( b4->cur_pos, key_data + left_dups, left_len ) ;

         /* move and put info */
         info_pos = b4->data + ( b4->key_on ) * i_len ;
         memmove( info_pos + i_len, info_pos ,i_len * ( b4->header.n_keys - b4->key_on ) ) ;
         x4put_info( &b4->node_hdr, buffer, rec, left_blanks, left_dups ) ;
         memcpy( info_pos, buffer, i_len ) ;
         x4put_info( &b4->node_hdr, buffer, x4recno( b4, b4->key_on + 1 ), right_blanks, right_dups ) ;
         memcpy( info_pos + i_len, buffer, i_len ) ;
      }
   }

   b4->changed = 1 ;
   b4->header.n_keys++ ;
   b4->node_hdr.free_space -= (reqd_len + i_len ) ;
   b4->cur_dup_cnt = left_dups ;
   b4->cur_trail_cnt = left_blanks ;
   return 0 ;
}

/* S4FOX */
int S4FUNCTION b4insert_branch( B4BLOCK *b4, void *k, long r, long r2, char new_flag )
{
   int left_len, move_len, g_len ;
   char *data_ptr, *next_ptr ;

   g_len = b4->tag->header.key_len + 2 * sizeof(long) ;

   #ifdef S4DEBUG
      if ( k == 0 || r <= 0L )
         e4severe( e4parm, E4_B4INSERT_BR ) ;
   #endif  /* S4DEBUG */

   data_ptr = ((char *)&b4->node_hdr) + b4->key_on * g_len ;
   next_ptr = data_ptr + g_len ;

   left_len = B4BLOCK_SIZE - sizeof( b4->header ) - g_len * b4->header.n_keys ;
   if ( left_len < g_len )  /* not enough room */
      return 1 ;

   #ifdef S4DEBUG
      if ( b4->key_on < 0  ||  b4->key_on > b4->header.n_keys )
         e4severe( e4info, E4_B4INSERT_BR ) ;
   #endif  /* S4DEBUG */

   move_len = g_len * (b4->header.n_keys - b4->key_on) ;

   memmove( next_ptr, data_ptr, move_len ) ;
   b4->header.n_keys++ ;
   memcpy( data_ptr, k, b4->tag->header.key_len ) ;
   memset( data_ptr + g_len - 2*sizeof(long), 0, sizeof(long) ) ;

   r2 = x4reverse_long( r2 ) ;
   memcpy( data_ptr + g_len - 2*sizeof(long), (void *)&r2, sizeof(long) ) ;

   r = x4reverse_long( r ) ;

   if ( !new_flag &&  (b4->key_on + 1) != b4->header.n_keys )
      memcpy(  next_ptr + g_len - sizeof(long), (void *)&r, sizeof(long) ) ;
   else
      memcpy( data_ptr + g_len - sizeof(long), (void *)&r, sizeof(long) ) ;

   b4->changed = 1 ;

   return 0 ;
}

/* S4FOX */
int S4FUNCTION b4insert( B4BLOCK *b4, void *key_data, long rec, long rec2, char new_flag )
{
   if( b4->header.node_attribute >= 2 ) /* leaf */
      return b4insert_leaf( b4, key_data, rec ) ;
   else
      return b4insert_branch( b4, key_data, rec, rec2, new_flag ) ;
}

/* S4FOX */
int S4FUNCTION b4reindex( B4BLOCK *b4 )
{
   int i, dup_cnt, trail ;
   int ni_len = b4->node_hdr.info_len + 1 ;
   unsigned char buffer[6] ;
   long rec ;
   int space_reqd = b4->header.n_keys ;   /* 1 byte xtra for each record */
   if ( space_reqd > b4->node_hdr.free_space )  /* not enough room */
      return 1 ;

   for ( i = b4->header.n_keys-1 ; i >= 0 ; i-- )
   {
      dup_cnt = x4dup_cnt( b4, i ) ;
      trail = x4trail_cnt( b4, i ) ;
      rec = x4recno( b4, i ) ;
      memset( b4->data + i * ni_len, 0, ni_len ) ;

      b4->node_hdr.rec_num_len += 8 ;  /* for the new info */
      b4->node_hdr.info_len++ ;
      x4put_info( &b4->node_hdr, buffer, rec, trail, dup_cnt ) ;
      b4->node_hdr.rec_num_len -= 8 ;  /* to get the old info */
      b4->node_hdr.info_len-- ;
      memcpy( b4->data + i * ni_len, buffer, ni_len ) ;
   }
   b4->node_hdr.rec_num_mask |= (unsigned long)(0x000000FFL << b4->node_hdr.rec_num_len );
   b4->node_hdr.info_len++ ;
   b4->node_hdr.rec_num_len += 8 ;
   b4->node_hdr.free_space -= b4->header.n_keys ;
   return 0 ;
}

/* S4FOX */
int S4FUNCTION b4flush( B4BLOCK *b4 )
{
   int rc ;
   INDEX4 *i4 ;

   if ( b4->changed )
   {
      i4 = b4->tag->index ;
      rc = file4write( &i4->file, (long)b4->file_block * I4MULTIPLY, &b4->header.node_attribute, B4BLOCK_SIZE ) ;
      if ( rc < 0 )
         return rc ;
      b4->changed = 0 ;
   }
   return 0 ;
}
#endif /* S4OFF_WRITE */

/* S4FOX */
void S4FUNCTION b4go( B4BLOCK *b4, int i_key )
{
   b4skip( b4, i_key - b4->key_on ) ;
}

/* S4FOX */
void S4FUNCTION b4top( B4BLOCK *b4 )
{
   b4->key_on = 0 ;
   if ( b4leaf( b4 ) )
   {
      b4->cur_dup_cnt = 0 ;
      b4->cur_pos = ((char *)&b4->header) + B4BLOCK_SIZE - b4->tag->header.key_len + x4trail_cnt( b4, 0 ) ;
   }
}

/* S4FOX */
void S4FUNCTION b4go_eof( B4BLOCK *b4 )
{
   b4->key_on = b4->header.n_keys ;
   b4->cur_pos = ((char *)&b4->header) + sizeof( B4STD_HEADER ) + sizeof( B4NODE_HEADER )
                 + b4->header.n_keys * b4->node_hdr.info_len + b4->node_hdr.free_space ;
}

/* S4FOX */
B4KEY_DATA *S4FUNCTION b4key( B4BLOCK *b4, int i_key )
{
   int len ;

   #ifdef S4DEBUG
      if ( i_key > b4->header.n_keys || i_key < 0 )
         e4severe( e4info, E4_INFO_CIF ) ;
   #endif  /* S4DEBUG */

   if ( i_key == b4->built_on )   /* already there! */
      return b4->built_key ;

   if ( b4->header.node_attribute >= 2 ) /* leaf */
   {
      if ( b4->built_on > i_key || b4->built_on == -1 )
      {
         b4->built_on = -1 ;
         b4->built_pos = ((char *)&b4->header) + B4BLOCK_SIZE ;
      }

      for ( ; b4->built_on != i_key ; )
      {
         b4->built_on++ ;
         b4->cur_dup_cnt = x4dup_cnt( b4, b4->built_on ) ;
         b4->cur_trail_cnt = x4trail_cnt( b4, b4->built_on ) ;
         len = b4->tag->header.key_len - b4->cur_dup_cnt - b4->cur_trail_cnt ;
         #ifdef S4DEBUG
            if (len < 0 || len > b4->tag->header.key_len || ( b4->built_pos - len ) < b4->data )
               e4severe( e4info, E4_INFO_CIF ) ;
         #endif  /* S4DEBUG */
         b4->built_pos -= len ;
         memcpy( b4->built_key->value + b4->cur_dup_cnt, b4->built_pos, len ) ;
         memset( b4->built_key->value + b4->tag->header.key_len - b4->cur_trail_cnt, b4->tag->p_char, b4->cur_trail_cnt ) ;
      }
      b4->built_key->num = x4recno( b4, b4->key_on ) ;
   }
   else /* branch */
   {
      memcpy( b4->built_key->value, (((char *)&b4->node_hdr) + i_key*(2*sizeof( long ) + b4->tag->header.key_len ) ), b4->tag->header.key_len ) ;
      b4->built_key->num = x4reverse_long( *(long *)( ((unsigned char *)&b4->node_hdr)
                          + (i_key+1)*(2*sizeof(long) + b4->tag->header.key_len)
                          - sizeof(long) ) ) ;
   }
   return b4->built_key ;
}

/* S4FOX */
unsigned char *S4FUNCTION b4key_key( B4BLOCK *b4, int i_key )
{
   return (unsigned char *)b4key( b4, i_key )->value ;
}

/* S4FOX */
int S4FUNCTION b4lastpos( B4BLOCK *b4 )
{
   return b4->header.n_keys - 1 ;
}

/* S4FOX */
int S4FUNCTION b4leaf( B4BLOCK *b4 )
{
   return( b4->header.node_attribute >= 2 ) ;
}

/* S4FOX */
long S4FUNCTION b4recno( B4BLOCK *b4, int i )
{
   if ( b4->header.node_attribute >= 2 ) /* leaf */
      return x4recno( b4, i ) ;
   else /* branch */
       return x4reverse_long( *(long *)(((unsigned char *)&b4->node_hdr) +
          i * (2*sizeof(long) + b4->tag->header.key_len) + b4->tag->header.key_len ) ) ;
}

/* S4FOX */
#ifndef S4OFF_WRITE
void S4FUNCTION b4br_replace( B4BLOCK *b4, char *str, long rec )
{
   int key_len ;
   char *put_pl ;

   key_len = b4->tag->header.key_len ;
   put_pl = ( (char *)&b4->node_hdr ) + b4->key_on * ( 2 * sizeof( long ) + key_len ) ;
   memcpy( put_pl, str, key_len ) ;
   memcpy( b4->built_key->value, str, key_len ) ;
   memset( put_pl + key_len, 0, sizeof(long) ) ;
   rec = x4reverse_long( rec ) ;
   memcpy( put_pl + key_len, (void *)&rec, sizeof(long) ) ;

   b4->changed = 1 ;
}
#endif /* S4OFF_WRITE */

/* S4FOX */
int S4FUNCTION b4r_brseek( B4BLOCK *b4, char *search_value, int len, long recno )
{
   int rc, key_len ;

   rc = b4seek( b4, search_value, len ) ;
   key_len = b4->tag->header.key_len ;

   if ( rc == 0 )   /* string matches, so search on recno */
      for(;;)
      {
         if ( u4memcmp( (void *)&recno, ((char *)&b4->node_hdr) + b4->key_on * ( 2 * sizeof(long) + key_len) + key_len, sizeof(long) ) <= 0
              || b4->key_on >= ( b4->header.n_keys - 1 ) )  /* best match, so done */
           break ;
         else
            if( b4->key_on < b4->header.n_keys )
               b4skip( b4, 1L ) ;

         if ( u4memcmp( b4key_key(b4,b4->key_on), search_value, len ) )  /* the key has changed, so stop here */
            break ;
      }

   return rc ;
}

/* S4FOX */
int S4FUNCTION b4seek( B4BLOCK *b4, char *search_value, int len )
{
   int rc, key_lower, key_upper, save_rc, key_cur, group_val ;
   if ( b4->header.n_keys == 0 )
   {
      b4->key_on = 0 ;
      return r4after ;
   }

   if ( b4leaf( b4 ) )
      return b4leaf_seek( b4, search_value, len ) ;

   /* key_cur must be between  key_lower and  key_upper */
   key_lower = -1 ;
   key_upper = b4->header.n_keys - 1 ;

   save_rc = 1 ;
   group_val = 2 * sizeof( long ) + b4->tag->header.key_len ;

   for(;;)  /* Repeat until the key is found */
   {
      key_cur = (key_upper + key_lower) / 2 ;
      /* only memcmp for branch blocks */
      rc = u4memcmp( (((char *)&b4->node_hdr) + key_cur * group_val ), search_value, len ) ;

      if ( rc >= 0 )
      {
         key_upper = key_cur ;
         save_rc = rc ;
      }
      else
         key_lower = key_cur ;

      if ( key_lower >= (key_upper-1) )  /* then there is no exact match */
      {
         b4->key_on = key_upper ;   /* branch block, just change key_on */
         if ( save_rc )
            return r4after ;
         return 0 ;
      }
   }
}

/* S4FOX */
int S4FUNCTION b4leaf_seek( B4BLOCK *b4, char *search_value, int len )
{
   int duplicates, bytes_same, compare_len, key_len, original_len, significant_bytes ;
   char all_blank ;

   original_len = len ;
   key_len = b4->tag->header.key_len ;
   len -= b4calc_blanks( search_value, len, b4->tag->p_char ) ;  /* don't compare blank bytes */
   if ( len == 0 )   /* if all blanks, watch for records < blank */
   {
      len = original_len ;
      all_blank = 1 ;
   }
   else
      all_blank = 0 ;
   duplicates = 0 ;
   b4top( b4 ) ;

   for(;;)
   {
      if ( b4->cur_dup_cnt == duplicates )
      {
         significant_bytes = key_len - x4trail_cnt( b4, b4->key_on ) ;
         if ( all_blank && significant_bytes == 0 )   /* found a blank record */
            len = 0 ;
         compare_len = ( len < significant_bytes ? len : significant_bytes ) - b4->cur_dup_cnt ;
         bytes_same = (*b4->tag->cmp)( b4->cur_pos, search_value + b4->cur_dup_cnt, compare_len ) ;

         if ( bytes_same == -1 )
            return r4after ;
         if ( bytes_same == compare_len )
            if ( b4->cur_dup_cnt + bytes_same == len )  /* for sure done */
            {
               if ( len != original_len && significant_bytes > len )
               {
                  if ( significant_bytes < original_len )
                     return r4after ;
                  #ifdef S4DEBUG
                     if ( len > original_len )
                        e4severe( e4info, E4_B4LEAF_SEEK ) ;
                  #endif
                  if ( (*b4->tag->cmp)( b4->cur_pos + compare_len, search_value + len, (original_len - len) ) != (original_len - len) )
                     return r4after ;
               }
               b4->cur_dup_cnt += bytes_same ;
               return 0 ;
            }

         b4->cur_dup_cnt += bytes_same ;
      }

      b4->key_on++ ;
      if ( b4->key_on >= b4->header.n_keys )
         return r4after ;
      duplicates = x4dup_cnt( b4, b4->key_on ) ;
      b4->cur_pos -= key_len - duplicates - x4trail_cnt( b4, b4->key_on ) ;
      if ( b4->cur_dup_cnt > duplicates )
         return r4after ;
   }
}

/* S4FOX */
#ifndef S4OFF_WRITE
void S4FUNCTION b4remove_leaf( B4BLOCK *b4 )
{
   int old_dup, left_dup, left_trail, key_len, remove_len, new_dup, old_bytes, new_bytes, mov_len, left_bytes ;
   unsigned char buffer[6], i_len ;
   char *old_pos, *info_pos ;

   b4->built_on = -1 ;

   if ( b4->header.n_keys == 1 )  /* removing last key */
   {
      b4->node_hdr.free_space = B4BLOCK_SIZE - sizeof( B4NODE_HEADER ) - sizeof( B4STD_HEADER ) ;
      memset( b4->data, 0, b4->node_hdr.free_space ) ;
      b4->header.n_keys = 0 ;
      b4->key_on = 0 ;
      b4->cur_pos = ((char *)&b4->header) + B4BLOCK_SIZE ;
      b4->changed = 1 ;
      return ;
   }

   key_len = b4->tag->header.key_len ;
   i_len = b4->node_hdr.info_len ;
   old_dup = x4dup_cnt( b4, b4->key_on) ;

   if ( b4->key_on == b4->header.n_keys - 1 )  /* at end */
   {
      remove_len = key_len - old_dup - x4trail_cnt( b4, b4->key_on ) ;

      #ifdef S4DEBUG
         if ( remove_len < 0 )
            e4severe( e4info, E4_INFO_CIF ) ;
      #endif  /* S4DEBUG */

      memset( b4->cur_pos, 0, remove_len ) ;
      memset( b4->data + b4->key_on * i_len, 0, i_len ) ;
      b4->key_on-- ;
   }
   else
   {
      old_bytes = key_len - old_dup - x4trail_cnt( b4, b4->key_on ) ;
      old_pos = b4->cur_pos ;
      b4skip( b4, 1 ) ;
      left_dup = x4dup_cnt( b4, b4->key_on) ;
      left_trail = x4trail_cnt( b4, b4->key_on ) ;
      new_dup = old_dup < left_dup ? old_dup : left_dup ;
      left_bytes = key_len - left_trail - left_dup ;
      new_bytes = key_len - left_trail - new_dup ;

      memcpy( b4->built_key->value, old_pos, left_dup - new_dup ) ;  /* copy prev dup bytes between the 2 keys only */
      memcpy( b4->built_key->value + ( left_dup - new_dup ), b4->cur_pos, left_bytes ) ;
      memcpy( old_pos + old_bytes - new_bytes, b4->built_key->value, new_bytes ) ;

      remove_len = old_bytes + left_bytes - new_bytes ;
      #ifdef S4DEBUG
         if ( remove_len < 0 )
            e4severe( e4info, E4_INFO_CIF ) ;
      #endif  /* S4DEBUG */

      mov_len = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - i_len * b4->header.n_keys
                - sizeof( B4NODE_HEADER ) - b4->node_hdr.free_space
                - ( ( (char *)&b4->header ) + B4BLOCK_SIZE - b4->cur_pos ) ;
      #ifdef S4DEBUG
         if ( mov_len < 0 )
            e4severe( e4info, E4_INFO_CIB ) ;
      #endif  /* S4DEBUG */

      memmove( b4->cur_pos - mov_len + remove_len, b4->cur_pos - mov_len, mov_len ) ;

      memset( b4->cur_pos - mov_len, 0, remove_len ) ;
      b4->key_on-- ;

      /* move and put info */
      info_pos = b4->data + ( b4->key_on ) * i_len ;
      memmove( info_pos, info_pos + i_len ,i_len * ( b4->header.n_keys - b4->key_on ) ) ;
      x4put_info( &b4->node_hdr, buffer, x4recno( b4, b4->key_on ), left_trail, new_dup ) ;
      memcpy( info_pos, buffer, i_len ) ;
      memset( b4->data + (b4->header.n_keys - 1 ) * i_len, 0, i_len ) ;
   }
   b4->changed = 1 ;
   b4->cur_pos += remove_len ;
   b4->header.n_keys-- ;
   b4->node_hdr.free_space += i_len + remove_len ;
}

/* S4FOX */
void S4FUNCTION b4remove( B4BLOCK *b4 )
{
   char *key_cur ;
   int len, i_len ;

   if( b4->header.node_attribute >= 2 ) /* leaf */
      b4remove_leaf( b4 ) ;
   else
   {
      i_len = b4->tag->header.key_len + 2 * sizeof( long ) ;
      key_cur = ((char *)&b4->node_hdr) + i_len * b4->key_on ;
      len = (b4->header.n_keys - b4->key_on - 1) * i_len ;

      #ifdef S4DEBUG
         if ( b4->key_on < 0  || b4->key_on > b4lastpos( b4 ) ||
              len < 0 || len > (B4BLOCK_SIZE - sizeof(B4STD_HEADER) - (unsigned)i_len ))
            e4severe( e4info, E4_B4REMOVE ) ;
      #endif  /* S4DEBUG */

      if ( len > 0 )
         memmove( key_cur, key_cur + i_len, len ) ;

      b4->header.n_keys-- ;
      memset( ((char *)&b4->node_hdr) + b4->header.n_keys * i_len, 0, i_len ) ;
      b4->changed = 1 ;
   }
}
#endif /* S4OFF_WRITE */

/* S4FOX */
int S4FUNCTION b4skip( B4BLOCK *b4, long n )
{
   int num_skipped ;

   if ( b4->header.node_attribute < 2 )  /* branch */
   {
      if ( n > 0 )
         num_skipped = b4->header.n_keys - b4->key_on ;
      else
         num_skipped = -b4->key_on ;

      if ( ( n <= 0L ) ?  (num_skipped <= (int)n)  :  (num_skipped >= (int)n)  )
      {
         b4->key_on += (int) n ;
         return (int) n ;
      }
      else
      {
         b4->key_on += num_skipped ;
         return num_skipped ;
      }
   }
   else  /* leaf */
   {
      if ( b4->header.n_keys == 0 )
         return 0 ;
      if (n > 0 )
      {
         if ( b4->key_on + n >= b4->header.n_keys )
         {
            b4->cur_pos = ((char *)&b4->header ) + sizeof( B4STD_HEADER )
                          + sizeof( B4NODE_HEADER ) + b4->node_hdr.info_len * b4->header.n_keys
                          + b4->node_hdr.free_space ;
            n = b4->header.n_keys - b4->key_on - ( b4->key_on != b4->header.n_keys ) ;
            b4->key_on = b4->header.n_keys ;
            return (int)n ;
         }
         for( num_skipped = (int)n ; num_skipped != 0 ; num_skipped-- )
         {
            b4->key_on++ ;
            b4->cur_pos -= ( b4->tag->header.key_len - x4dup_cnt( b4, b4->key_on ) - x4trail_cnt( b4, b4->key_on ) ) ;
         }
      }
      else
      {
         if ( b4->key_on + n < 0 )
         {
            n = -b4->key_on ;
            b4->key_on = 0 ;
            b4->cur_pos = ((char *)&b4->header ) + B4BLOCK_SIZE - b4->tag->header.key_len + x4trail_cnt( b4, b4->key_on ) ;
            return (int) n ;
         }
         for( num_skipped = (int)n ; num_skipped != 0 ; num_skipped++ )
         {
            b4->cur_pos += ( b4->tag->header.key_len - x4dup_cnt( b4, b4->key_on ) - x4trail_cnt( b4, b4->key_on ) ) ;
            b4->key_on-- ;
         }
      }

      return (int) n ;
   }
}
#endif  /* S4FOX */

#ifdef N4OTHER
#ifndef S4OFF_WRITE
int S4FUNCTION b4flush( B4BLOCK *b4 )
{
   int rc ;
   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int i ;
      long long_val ;
      short short_val ;
   #endif

   if ( b4->changed )
   {
      #ifdef S4INDEX_VERIFY
         if ( b4verify( b4 ) == -1 )
            e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4flush()", "" ) ;
      #endif
      #ifdef S4NDX
         #ifdef S4BYTE_SWAP
            swap = (char *) u4alloc_er( b4->tag->code_base, B4BLOCK_SIZE ) ;
            if ( swap == 0 )
               return -1 ;

            memcpy( (void *)swap, (void *)&b4->n_keys, B4BLOCK_SIZE ) ;
                             /* position swap_ptr at beginning of pointers */

            swap_ptr += swap + 2 + sizeof(short) ;

            for ( i = 0 ; i < (*(short *)swap) ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)swap_ptr+sizeof(long) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
               swap_ptr += r4->data->indexes->tags.header->group_len ;
            }

            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;

            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            rc = file4write( &b4->tag->file, (long)b4->file_block*I4MULTIPLY, swap, B4BLOCK_SIZE ) ;
            u4free( swap ) ;
         #else
            rc = file4write( &b4->tag->file, (long)b4->file_block*I4MULTIPLY, &b4->n_keys, B4BLOCK_SIZE ) ;
         #endif
      #endif  /* S4NDX */

      #ifdef S4CLIPPER
         #ifdef S4BYTE_SWAP
            swap = (char *)u4alloc_er( b4->tag->code_base, B4BLOCK_SIZE ) ;
            if ( swap == 0 )
               return -1 ;

            memcpy( (void *)swap, (void *)&b4->n_keys, B4BLOCK_SIZE ) ;

            /* position swap_ptr at beginning of pointers */
            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            swap_ptr = swap + 2 ;

            /* swap the short pointers to B4KEY_DATA */
            for ( i = 0 ; i < b4->tag->header.keys_max ; i++ )
            {
               short_val = x4reverse_short( *(short *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
               swap_ptr += sizeof(short) ;
            }
                            /* swap the B4KEY_DATA's */
            for ( i = 0 ; i < b4->tag->header.keys_max ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)swap_ptr+sizeof(long) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
               swap_ptr += b4->tag->header.group_len ;
            }

            rc = file4write( &b4->tag->file, (long)b4->file_block*I4MULTIPLY, swap, B4BLOCK_SIZE ) ;
            u4free( swap ) ;
         #else
            rc = file4write( &b4->tag->file, (long)b4->file_block*I4MULTIPLY, &b4->n_keys, B4BLOCK_SIZE ) ;
         #endif
      #endif  /* S4CLIPPER */

      if ( rc < 0 )  return rc ;
      b4->changed = 0 ;
   }
   return 0 ;
}

void S4FUNCTION b4append( B4BLOCK *b4, long pointer )
{
   #ifdef S4CLIPPER
      long adj_pointer = pointer * 512 ;
   #endif  /* S4CLIPPER */
   B4KEY_DATA *data_ptr ;
   #ifdef S4DEBUG
      int left_len ;
      if ( b4leaf( b4 ) || pointer <1L )
         e4severe( e4parm, E4_B4APPEND ) ;
   #endif  /* S4DEBUG */

   b4go_eof( b4 ) ;
   data_ptr = b4key( b4, b4->key_on ) ;

   #ifdef S4DEBUG
      left_len = B4BLOCK_SIZE - b4->key_on * b4->tag->header.group_len -
         sizeof(short) - sizeof(char[2]) - sizeof(pointer) ;
      if ( b4->key_on < 0  ||  b4->key_on != b4->n_keys ||  left_len < 0 )
          e4severe( e4info, E4_B4APPEND ) ;
   #endif  /* S4DEBUG */

   #ifdef S4NDX
      memcpy( &data_ptr->pointer, &pointer, sizeof(pointer) ) ;
   #endif  /* S4NDX */
   #ifdef S4CLIPPER
      memcpy( &data_ptr->pointer, &adj_pointer, sizeof(pointer) ) ;
   #endif  /* S4CLIPPER */
   b4->changed = 1 ;
   #ifdef S4INDEX_VERIFY
      if ( b4verify( b4 ) == -1 )
         e4describe( b4->tag->code_base, e4index, b4->tag->alias, "b4append()-after append", "" ) ;
   #endif
}
#endif /* S4OFF_WRITE */

#ifndef S4OFF_WRITE
#ifdef S4CLIPPER
void S4FUNCTION b4append2( B4BLOCK *b4, void *k, long r, long pointer )
{
   unsigned short temp ;
   B4KEY_DATA *data_ptr ;
   long adj_pointer ;

   b4go_eof( b4 ) ;

   if ( b4->n_keys > 0 )
   {
      if ( !b4leaf( b4 ) )   /* insert after branch */
         b4->key_on++ ;
      else
         if ( b4->key_on + 1 < b4->tag->header.keys_max )
         {
            temp = b4->pointers[b4->n_keys+1] ;
            b4->pointers[b4->n_keys+1] = b4->pointers[b4->n_keys] ;
            b4->pointers[b4->n_keys] = temp ;
         }
   }

   data_ptr  = b4key( b4, b4->key_on ) ;
   adj_pointer = pointer * 512 ;
   b4->n_keys++ ;

   memcpy( data_ptr->value, k, b4->tag->header.group_len-2*sizeof(long) ) ;
   memcpy( &data_ptr->num, &r, sizeof(r) ) ;
   memcpy( &data_ptr->pointer, &adj_pointer, sizeof(pointer) ) ;
   b4->changed = 1 ;
}
#endif  /* S4CLIPPER */

#ifdef S4NDX

/* S4NDX */
void  S4FUNCTION b4insert( B4BLOCK *b4, void *k, long r, long pointer )
{
   B4KEY_DATA *data_ptr, *next_ptr ;
   int left_len ;
   #ifdef S4DEBUG
      if ( k == 0 || r < 0L || pointer <0L )
         e4severe( e4parm, E4_B4INSERT ) ;
   #endif  /* S4DEBUG */

   data_ptr = b4key( b4, b4->key_on ) ;
   next_ptr= b4key( b4, b4->key_on + 1 ) ;

   left_len = B4BLOCK_SIZE -  (b4->key_on + 1) * b4->tag->header.group_len -
                   sizeof(short) - sizeof(char[2]) ;   /* 2 for S4NDX version, not 6 */

   #ifdef S4DEBUG
      if ( b4->key_on < 0  ||  ( b4->key_on >  b4->n_keys && b4leaf( b4 ) )  ||
         ( b4->key_on > (b4->n_keys+1)  &&  !b4leaf( b4 ) )  || left_len < 0 )
         e4severe( e4info, E4_B4INSERT ) ;
   #endif  /* S4DEBUG */

   memmove( next_ptr, data_ptr, left_len ) ;
   b4->n_keys++ ;
   memcpy( data_ptr->value, k, b4->tag->header.key_len ) ;
   memcpy( &data_ptr->num, &r, sizeof(r) ) ;
   memcpy( &data_ptr->pointer, &pointer, sizeof(pointer) ) ;
   b4->changed = 1 ;
}
#endif  /* S4NDX */

#ifdef S4CLIPPER
/* clipper just puts at end and inserts into the index part */
void  S4FUNCTION b4insert( B4BLOCK *b4, void *k, long r, long pointer )
{
   short temp, insert_pos ;
   #ifdef S4DEBUG
      if ( k == 0 || r < 0L || pointer <0L )
         e4severe( e4parm, E4_B4INSERT ) ;
   #endif  /* S4DEBUG */

   insert_pos = b4->key_on ;

   /* put at block end: */
   b4append2( b4, k, r, pointer ) ;

   temp = b4->pointers[b4->key_on] ;   /* save the placed position */
   memmove( &b4->pointers[insert_pos+1], &b4->pointers[insert_pos], sizeof(short) * ( b4lastpos( b4 ) - insert_pos - (!b4leaf( b4 ) && b4->n_keys < 2) ) ) ;
   b4->pointers[insert_pos] = temp ;
   if ( b4leaf( b4 ) )
      if ( b4key( b4, b4->n_keys )->pointer != 0 )
         b4key( b4, b4->n_keys )->pointer = 0L ;
}
#endif  /* S4CLIPPER */
#endif /* S4OFF_WRITE */

/* goes to one past the end of the block */
void S4FUNCTION b4go_eof( B4BLOCK *b4 )
{
   b4->key_on = b4->n_keys ;
}

B4KEY_DATA *S4FUNCTION b4key( B4BLOCK *b4, int i_key )
{
   #ifdef S4NDX
      return  (B4KEY_DATA *) ((char *) &b4->data + b4->tag->header.group_len * i_key) ;
   #endif  /* S4NDX */

   #ifdef S4CLIPPER
      #ifdef S4DEBUG
         if ( i_key > 2 + b4->tag->header.keys_max )
            e4severe( e4parm, E4_B4KEY ) ;
      #endif  /* S4DEBUG */
      return  (B4KEY_DATA *) ((char *) &b4->n_keys + b4->pointers[i_key] ) ;
   #endif  /* S4CLIPPER */
}

unsigned char *S4FUNCTION b4key_key( B4BLOCK *b4, int i_key )
{
   return  (unsigned char *) b4key( b4, i_key )->value ;
}

int S4FUNCTION b4lastpos( B4BLOCK *b4 )
{
   if ( !b4leaf( b4 ) )
      return b4->n_keys ;
   else
      return b4->n_keys - 1 ;
}

int S4FUNCTION b4leaf( B4BLOCK *b4 )
{
   return( b4key( b4, 0 )->pointer == 0L ) ;
}

long  S4FUNCTION b4recno( B4BLOCK *b4, int i )
{
   return b4key( b4, i )->num ;
}

#ifndef S4OFF_WRITE
#ifdef S4NDX
void  S4FUNCTION b4remove( B4BLOCK *b4 )
{
   B4KEY_DATA *key_on ;
   int left_len = -1 ;

   key_on = b4key( b4, b4->key_on ) ;

   if( !b4leaf( b4 ) )
   {
      if ( b4->key_on >= b4->n_keys )
         left_len = 0 ;
      else   /* on last pos, so copy only the rec_num */
         if ( b4lastpos( b4 ) + 1 == b4->n_keys )
            left_len = sizeof( long ) ;
   }

   if ( left_len == -1 )
      left_len = B4BLOCK_SIZE - sizeof(b4->n_keys) - sizeof(b4->dummy) - (b4->key_on+1) * b4->tag->header.group_len ;

   #ifdef S4DEBUG
      if ( b4->key_on < 0  ||  b4->key_on > b4lastpos( b4 )  ||  left_len < 0 )
         e4severe( e4info, E4_B4REMOVE ) ;
   #endif  /* S4DEBUG */

   if ( left_len > 0 )
   {
      B4KEY_DATA *key_next= b4key( b4, b4->key_on+1 ) ;
      memmove( key_on, key_next, left_len ) ;
   }
   b4->n_keys-- ;
   b4->changed = 1 ;
}
#endif  /* S4NDX */

#ifdef S4CLIPPER
void  S4FUNCTION b4remove( B4BLOCK *b4 )
{
   short temp ;

   /* just delete this entry */
   temp = b4->pointers[b4->key_on] ;
   memmove( &b4->pointers[b4->key_on], &b4->pointers[b4->key_on+1],
            sizeof(short) * (b4lastpos( b4 ) - b4->key_on) ) ;
   b4->pointers[b4lastpos( b4 )] = temp ;
   b4->n_keys-- ;
   b4->changed = 1 ;

   if ( b4leaf( b4 ) )
      memset( b4key( b4, b4lastpos( b4 ) + 1 ), 0, b4->tag->header.key_len + 2 * sizeof( long ) ) ;
}
#endif  /* S4CLIPPER */

int S4FUNCTION b4room( B4BLOCK *b4 )
{
   if ( b4leaf( b4 ) )
      return ( b4->n_keys < b4->tag->header.keys_max ) ;

   return( ( b4->n_keys < b4->tag->header.keys_max ) && ( ( B4BLOCK_SIZE -
      b4->n_keys * b4->tag->header.group_len - sizeof(short) - 2*sizeof(char)) >= sizeof(long) ) ) ;
}
#endif /* S4OFF_WRITE */

int  S4FUNCTION b4seek( B4BLOCK *b4, char *search_value, int len )
{
   int rc, save_rc, key_cur ;
   S4CMP_FUNCTION *cmp = b4->tag->cmp ;

   /* key_on must be between  key_lower and  key_upper */
   int key_lower = -1 ;
   int key_upper = b4->n_keys ;

   if ( key_upper == 0 )
   {
      b4->key_on = 0 ;
      return r4after ;
   }

   save_rc = 1 ;

   for(;;)  /* Repeat until the key is found */
   {
      key_cur  = (key_upper + key_lower) / 2  ;
      rc = (*cmp)( b4key_key(b4,key_cur), search_value, len ) ;

      if ( rc >= 0 )
      {
         key_upper = key_cur ;
         save_rc = rc ;
      }
      else
         key_lower = key_cur ;

      if ( key_lower >= (key_upper-1) )  /* then there is no exact match */
      {
         b4->key_on = key_upper ;
         if ( save_rc )
            return r4after ;
         return 0 ;
      }
   }
}

int S4FUNCTION b4skip( B4BLOCK *b4, long n )
{
   int num_left ;

   if ( n > 0 )
   {
      num_left = b4->n_keys - b4->key_on ;
      if ( b4leaf( b4 ) )
      if ( num_left != 0 )
         num_left -- ;
   }
   else
      num_left = -b4->key_on ;

   if ( ( n <= 0L ) ? ( (long)num_left <= n ) : ( (long)num_left >= n ) )
   {
      b4->key_on = b4->key_on + (int) n ;
      return (int) n ;
   }
   else
   {
      b4->key_on = b4->key_on + num_left ;
      return num_left ;
   }
}
#endif  /* N4OTHER */

#endif  /* S4INDEX_OFF */
