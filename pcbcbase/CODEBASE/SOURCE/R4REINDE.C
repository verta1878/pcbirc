/* r4reinde.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
#ifndef S4INDEX_OFF
#ifndef N4OTHER

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TURBOC__ */
#endif  /* S4UNIX */

#include "r4reinde.h"

int S4FUNCTION i4reindex( INDEX4 *i4 )
{
   R4REINDEX reindex ;
   int rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif
   #ifdef S4FOX
      TAG4 *tag_on ;
      B4BLOCK *block ;
      long r_node, go_to ;
      char tag_name[11] ;
      int i, len ;
   #endif  /* S4FOX */

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4REINDEX ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( i4 == 0  )
         e4severe( e4parm, E4_I4REINDEX ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;       

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         i4->code_base->mode |= 0x40 ;
      #endif  /* S4DETECT_OFF */

      has_opt = i4->code_base->has_opt ;
      d4opt_suspend( i4->code_base ) ;
   #endif  /* not S4OPTIMIZE_OFF */

   #ifndef S4SINGLE
      rc = i4lock( i4 ) ;
      if ( rc )
         return rc ;
   #endif  /* S4SINGLE */

   if ( r4reindex_init( &reindex, i4 ) < 0 )
      return -1 ;
   if ( r4reindex_tag_headers_calc( &reindex ) < 0 )
      return -1 ;
   if ( r4reindex_blocks_alloc(&reindex) < 0 )
      return -1 ;

   #ifndef S4SINGLE
      #ifdef S4FOX
         if ( i4->file.is_exclusive == 0 )
            i4->tag_index->header.version =  i4->version_old+1 ;
      #endif
   #endif

   #ifdef S4FOX
      reindex.n_blocks_used = 0 ;
      tag_name[10] = '\0' ;

      if ( i4->tag_index->header.type_code >= 64 )  /* if .cdx */
      {
         reindex.tag = i4->tag_index ;
         if ( sort4init( &reindex.sort, i4->code_base, i4->tag_index->header.key_len, 0 ) < 0 )
            return -1 ;
         reindex.sort.cmp = u4memcmp ;

         for( tag_on = 0, i = 1 ; ; i++ )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            len = strlen( tag_on->alias ) ;
            memset( tag_name, ' ', 10 ) ;
            memcpy( tag_name, tag_on->alias, len ) ;
            if ( sort4put( &reindex.sort, 2 * B4BLOCK_SIZE * i, tag_name, "" ) < 0)
               return -1 ;
            #ifdef S4DEBUG
               reindex.key_count++ ;
            #endif /* S4DEBUG */
         }

         if ( (rc = r4reindex_write_keys(&reindex)) != 0 )
         {
            r4reindex_free( &reindex ) ;
            return rc ;
         }
      }
      else
         if ( reindex.n_tags > 1 )   /* should only be 1 tag in an .idx */
            return e4( i4->code_base, e4index, E4_INFO_BDI ) ;
   #endif  /* S4FOX */

   for( reindex.tag = 0 ;; )
   {
      reindex.tag = (TAG4 *) l4next(&i4->tags, reindex.tag) ;
      if ( reindex.tag == 0 )
         break ;
      #ifdef S4FOX
         reindex.n_blocks_used = 0 ;
      #else  /* S4MDX */
         reindex.tag->header.version++ ;
      #endif  /* S4FOX */

      rc = r4reindex_supply_keys( &reindex ) ;
      if ( rc )
      {
         r4reindex_free( &reindex ) ;
         return rc ;
      }

      rc = r4reindex_write_keys( &reindex ) ;
      if ( rc )
      {
         r4reindex_free( &reindex ) ;
         return rc ;
      }
   }

   rc = r4reindex_tag_headers_write( &reindex ) ;

   #ifdef S4FOX
      /* now must fix the right node branches for all blocks by moving leftwards */
      for( tag_on = 0 ; ; )
      {
         tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
         if ( tag_on == 0 )
            break ;
         for( t4rl_bottom( tag_on ) ; tag_on->blocks.last_node ; t4up( tag_on ) )
         {
            block = t4block( tag_on ) ;
            r_node = block->header.left_node ;
            go_to = block->header.left_node ;

            while ( go_to != -1 )
            {
               r_node = block->file_block ;
               if ( block->changed )
                  if ( b4flush( block ) < 0 )
                     return -1 ;
               if ( file4read_all( &tag_on->index->file, I4MULTIPLY*go_to, &block->header, B4BLOCK_SIZE) < 0 )
                  return -1 ;
               block->file_block = go_to ;
               if ( block->header.right_node != r_node )  /* if a bad value */
               {
                  block->header.right_node = r_node ;
                  block->changed = 1 ;
               }
               go_to = block->header.left_node ;
            }
            block->built_on = -1 ;
            b4top( block ) ;
         }
      }
   #endif  /* S4FOX */

   r4reindex_free( &reindex ) ;
   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( i4->code_base ) ;
   #endif  /* not S4OPTIMIZE_OFF */

   return rc ;
}

int r4reindex_init( R4REINDEX *r4, INDEX4 *i4 )
{
   memset( (void *)r4, 0, sizeof( R4REINDEX ) ) ;

   r4->index = i4 ;
   r4->data = i4->data ;
   r4->code_base = i4->code_base ;

   r4->min_keysmax = INT_MAX ;
   r4->start_block = 0 ;

   #ifndef S4FOX
      r4->blocklen = i4->header.block_rw ;
   #endif  /* S4FOX */

   r4->buffer_len = i4->code_base->mem_size_sort_buffer ;
   if ( r4->buffer_len < 1024 )
      r4->buffer_len = 1024 ;

   r4->buffer = (char *)u4alloc_er( i4->code_base, r4->buffer_len ) ;
   if ( r4->buffer == 0 )
      return e4memory ;

   #ifdef S4FOX
      r4->lastblock = 1024 ;  /* leave space for the index header block  */
   #endif  /* S4FOX */

   return 0 ;
}

void r4reindex_free( R4REINDEX *r4 )
{
   u4free( r4->buffer ) ;
   u4free( r4->start_block ) ;
   sort4free( &r4->sort ) ;
}

int r4reindex_blocks_alloc( R4REINDEX *r4 )
{
   long on_count ;

   #ifdef S4DEBUG
      if ( (unsigned) r4->min_keysmax > INT_MAX )
         e4severe( e4info, E4_I4REINDEX_BA ) ;
   #endif  /* S4DEBUG */

   /* Calculate the block stack height */
   on_count = d4reccount( r4->data ) ;
   for ( r4->n_blocks = 2; on_count != 0L; r4->n_blocks++ )
      on_count /= r4->min_keysmax ;

   if ( r4->start_block == 0 )
   {
      #ifdef S4FOX
         r4->start_block = (R4BLOCK_DATA *)u4alloc_er( r4->code_base, (long)B4BLOCK_SIZE * r4->n_blocks ) ;
      #endif  /* S4FOX */

      #ifdef S4MDX
         r4->start_block = (R4BLOCK_DATA *)u4alloc_er( r4->code_base, (long)r4->blocklen * r4->n_blocks ) ;
      #endif  /* S4MDX */
   }

   if ( r4->start_block == 0 )
      return e4memory ;

   return 0 ;
}

int r4reindex_supply_keys( R4REINDEX *r4 )
{
   FILE4SEQ_READ seq_read ;
   EXPR4 *filter ;
   char *key_result ;
   int i, *filter_result ;
   long  count, i_rec ;
   DATA4 *d4 ;
   TAG4 *t4 ;

   d4 = r4->data ;
   t4 = r4->tag ;
   #ifdef S4DEBUG
      r4->key_count = 0L ;
   #endif  /* S4DEBUG */

   file4seq_read_init( &seq_read, &d4->file, d4record_position(d4,1L), r4->buffer, r4->buffer_len) ;

   if ( sort4init( &r4->sort, r4->code_base, t4->header.key_len, 0 ) < 0 )
      return -1 ;

   #ifdef S4FOX
      r4->sort.cmp = u4memcmp ;
   #endif  /* S4FOX */

   #ifdef S4MDX
      r4->sort.cmp = t4->cmp ;
   #endif  /* S4MDX */

   filter = t4->filter ;
   count = d4reccount( d4 ) ;

   for ( i_rec = 1L; i_rec <= count; i_rec++ )
   {
      if ( file4seq_read_all( &seq_read, d4->record, d4->record_width ) < 0 )
         return -1 ;
      d4->rec_num = i_rec ;

      #ifndef S4MEMO_OFF
         for ( i = 0; i < d4->n_fields_memo; i++ )
            f4memo_reset( d4->fields_memo[i].field ) ;
      #endif  /* S4MEMO_OFF */

      if ( filter )
      {
         expr4vary( filter, (char **)&filter_result ) ;
         #ifdef S4DEBUG
            if ( expr4type( filter ) != r4log )
               e4severe( e4result, E4_I4REINDEX_SK ) ;
         #endif  /* S4DEBUG */
         if ( ! *filter_result )
            continue ;
         t4->has_keys = 1 ;
         #ifdef S4MDX
            t4->had_keys = 0 ;
         #endif
      }

      t4expr_key( t4, &key_result ) ;

      if ( sort4put( &r4->sort, i_rec, key_result, "" ) < 0)
         return -1 ;
      #ifdef S4DEBUG
         r4->key_count++ ;
      #endif  /* S4DEBUG */
   }

   return 0 ;
}

int r4reindex_tag_headers_calc( R4REINDEX *r4 )
{
   TAG4 *tag ;
   #ifdef S4FOX
      int keysmax, expr_type ;
   #endif  /* S4FOX */

   r4->n_tags = 0 ;
   for( tag = 0 ;; )
   {
      tag = (TAG4 *) l4next( &r4->index->tags, tag ) ;
      if ( tag == 0 )
         break ;
      if ( t4free_all(tag) < 0 )
         return -1 ;

      tag->header.key_len = expr4key_len( tag->expr ) ;
      #ifdef S4FOX
         expr_type = expr4type( tag->expr ) ;
         if ( expr_type < 0 )
            return -1 ;
         t4init_seek_conv( tag, expr_type ) ;
         if ( tag->header.key_len < 0 )
            return -1 ;

         keysmax = ( B4BLOCK_SIZE - sizeof(B4STD_HEADER) ) / ( tag->header.key_len + 2*sizeof(long) ) ;

         if ( keysmax < r4->min_keysmax )
            r4->min_keysmax = keysmax ;
      #endif  /* S4FOX */

      #ifdef S4MDX
         if ( tag->header.key_len < 0 )
            return -1 ;

         tag->header.type = (char) expr4type( tag->expr ) ;
         if ( tag->header.type == r4date_doub )
            tag->header.type = r4date ;
         if ( tag->header.type == r4num_doub )
            tag->header.type = r4num ;

         t4init_seek_conv( tag, tag->header.type ) ;
         tag->header.group_len = tag->header.key_len+ 2*sizeof(long)-1 ;
         tag->header.group_len-= tag->header.group_len % sizeof(long) ;

         tag->header.keys_max =
                 (r4->index->header.block_rw - sizeof(short) - 6 - sizeof(long)) /
                      tag->header.group_len;
         if ( tag->header.keys_max < r4->min_keysmax )
            r4->min_keysmax = tag->header.keys_max ;
         tag->has_keys = 0 ;
         tag->had_keys = 1 ;
      #endif  /* S4MDX */

      r4->n_tags++ ;
   }

   #ifdef S4FOX
      if ( r4->index->tag_index->header.type_code >= 64 )
      {
         r4->lastblock += ((long)r4->n_tags-1)*2*B4BLOCK_SIZE + B4BLOCK_SIZE ;
         t4init_seek_conv( r4->index->tag_index, r4str ) ;
      }
      else
         r4->lastblock -= B4BLOCK_SIZE ;
   #endif  /* S4FOX */

   #ifdef S4MDX
      r4->lastblock_inc = r4->index->header.block_rw/ 512 ;
      r4->lastblock = 4 + (r4->n_tags-1)*r4->lastblock_inc ;
   #endif  /* S4MDX */

   return 0 ;
}

TAG4 *r4reindex_find_i_tag( R4REINDEX *r4, int i_tag )
{
   /* First 'i_tag' starts at '1' for this specific routine */
   TAG4 *tag_on ;
   tag_on = (TAG4 *) l4first( &r4->index->tags ) ;

   while ( --i_tag >= 1 )
   {
      tag_on = (TAG4 *) l4next( &r4->index->tags, tag_on ) ;
      if ( tag_on == 0 )
         return 0 ;
   }
   return tag_on ;
}

#ifdef S4MDX

#define GARBAGE_LEN 518

int r4reindex_tag_headers_write( R4REINDEX *r4 )
{
   /* First, calculate the T4DESC.left_chld, T4DESC.right_chld values, T4DESC.parent values */
   int  higher[49], lower[49], parent[49] ;
   TAG4 *tag_on, *tag_ptr ;
   INDEX4 *i4 ;
   DATA4  *d4 ;
   int n_tag, i_tag, j_field, len, save_code ;
   T4DESC tag_info ;
   char *ptr ;
   #ifdef S4BYTE_SWAP
      I4HEADER swap_header ;
      T4HEADER swap_tag_header ;
   #endif  /* S4BYTE_SWAP */

   memset( (void *)higher, 0, sizeof(higher) ) ;
   memset( (void *)lower,  0, sizeof(lower) ) ;
   memset( (void *)parent, 0, sizeof(parent) ) ;

   i4 = r4->index ;
   d4 = r4->data ;

   tag_on = (TAG4 *) l4first( &i4->tags ) ;
   if ( tag_on != 0 )
   {
      n_tag = 1 ;

      for ( ;; )
      {
         tag_on = (TAG4 *)l4next( &i4->tags, tag_on) ;
         if ( tag_on == 0 )
            break ;
         n_tag++ ;
         i_tag = 1 ;
         for (;;)
         {
            tag_ptr = r4reindex_find_i_tag( r4, i_tag ) ;
            #ifdef S4DEBUG
               if ( tag_ptr == 0 || i_tag < 0 || i_tag >= 48 || n_tag > 48 )
                  e4severe( e4result, E4_I4REINDEX_THW ) ;
            #endif  /* S4DEBUG */
            if ( u4memcmp( tag_on->alias, tag_ptr->alias, sizeof(tag_on->alias)) < 0)
            {
               if ( lower[i_tag] == 0 )
               {
                  lower[i_tag] = n_tag ;
                  parent[n_tag] = i_tag ;
                  break ;
               }
               else
                  i_tag = lower[i_tag] ;
            }
            else
            {
               if ( higher[i_tag] == 0 )
               {
                  higher[i_tag] = n_tag ;
                  parent[n_tag] = i_tag ;
                  break ;
               }
               else
                  i_tag = higher[i_tag] ;
            }
         }
      }
   }

   /* Now write the headers */
   file4seq_write_init( &r4->seqwrite, &i4->file, 0L, r4->buffer, r4->buffer_len ) ;

   i4->header.eof = r4->lastblock + r4->lastblock_inc ;
   i4->header.free_list = 0L ;
   u4yymmdd( i4->header.yymmdd ) ;

   #ifdef S4BYTE_SWAP
      memcpy( (void *)&swap_header, (void *)&i4->header, sizeof(I4HEADER) ) ;

      swap_header.block_chunks = x4reverse_short( swap_header.block_chunks ) ;
      swap_header.block_rw = x4reverse_short( swap_header.block_rw ) ;
      swap_header.slot_size = x4reverse_short( swap_header.slot_size ) ;
      swap_header.num_tags = x4reverse_short( swap_header.num_tags ) ;
      swap_header.eof = x4reverse_long( swap_header.eof ) ;
      swap_header.free_list = x4reverse_long( swap_header.free_list ) ;
      swap_header.version = x4reverse_long( swap_header.version ) ;

      file4seq_write( &r4->seqwrite, &swap_header, sizeof(I4HEADER) ) ;
   #else
      file4seq_write( &r4->seqwrite, &i4->header, sizeof(I4HEADER) ) ;
   #endif  /* S4BYTE_SWAP */

   file4seq_write_repeat( &r4->seqwrite, 512-sizeof(I4HEADER)+17, 0 ) ;
   /* There is a 0x01 on byte 17 of the first 32 bytes. */
   file4seq_write( &r4->seqwrite, "\001", 1 ) ;
   file4seq_write_repeat( &r4->seqwrite, 14, 0 ) ;

   tag_on = (TAG4 *) l4first( &i4->tags ) ;

   for ( i_tag = 0; i_tag < 47; i_tag++ )
   {
      memset( (void *)&tag_info, 0, sizeof(tag_info) ) ;

      if ( i_tag < r4->n_tags )
      {
         tag_info.header_pos = 4 + (long) i_tag * r4->lastblock_inc ;
         tag_on->header_offset = tag_info.header_pos * 512 ;

         memcpy( (void *)tag_info.tag, tag_on->alias, sizeof(tag_info.tag) ) ;

         tag_info.index_type = tag_on->header.type ;

         #ifdef S4BYTE_SWAP
            tag_info.header_pos = x4reverse_long( tag_info.header_pos ) ;
            tag_info.x1000 = 0x0010 ;
         #else
            tag_info.x1000 = 0x1000 ;
         #endif  /* S4BYTE_SWAP */

         tag_info.x2 = 2 ;
         tag_info.left_chld = (char) lower[i_tag+1] ;
         tag_info.right_chld = (char) higher[i_tag+1] ;
         tag_info.parent = (char) parent[i_tag+1] ;

         if ( i4->header.is_production )
         {
            save_code = i4->code_base->field_name_error ;
            i4->code_base->field_name_error = 0 ;
            j_field = d4field_number( d4, tag_on->expr->source ) ;
            i4->code_base->field_name_error = save_code ;
            if ( j_field > 0 )
               file4write( &d4->file, (j_field+1)*sizeof(FIELD4IMAGE)-1, "\001", 1 ) ;
         }
         tag_on = (TAG4 *) l4next( &i4->tags, tag_on ) ;
      }
      if ( file4seq_write( &r4->seqwrite, &tag_info, sizeof(T4DESC)) < 0 )
         return -1 ;
   }

   for (tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on == 0 )
         break ;
      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swap_tag_header, (void *)&tag_on->header, sizeof(T4HEADER) ) ;

         swap_tag_header.root = x4reverse_long( swap_tag_header.root ) ;
         swap_tag_header.key_len = x4reverse_short( swap_tag_header.key_len ) ;
         swap_tag_header.keys_max = x4reverse_short( swap_tag_header.keys_max ) ;
         swap_tag_header.group_len = x4reverse_short( swap_tag_header.group_len ) ;
         swap_tag_header.unique = x4reverse_short( swap_tag_header.unique ) ;

         if ( file4seq_write( &r4->seqwrite, &swap_tag_header, sizeof(T4HEADER)) < 0 )
            return -1 ;
      #else
         if ( file4seq_write( &r4->seqwrite, &tag_on->header, sizeof(T4HEADER)) < 0 )
            return -1 ;
      #endif  /* S4BYTE_SWAP */

      ptr = tag_on->expr->source ;
      len = strlen(ptr) ;
      file4seq_write( &r4->seqwrite, ptr, len) ;
      file4seq_write_repeat( &r4->seqwrite, 221-len, 0 ) ;

      if( tag_on->filter != 0 )
      {
         file4seq_write_repeat( &r4->seqwrite, 1, 1 ) ;
         if (tag_on->has_keys)
            file4seq_write_repeat( &r4->seqwrite, 1, 1 ) ;
         else
            file4seq_write_repeat( &r4->seqwrite, 1, 0 ) ;
      }
      else
         file4seq_write_repeat( &r4->seqwrite, 2, 0 ) ;

      /* write extra space up to filter write point */
      file4seq_write_repeat( &r4->seqwrite, GARBAGE_LEN-3 , 0 ) ;

      if ( tag_on->filter == 0 )
         len = 0 ;
      else
      {
         ptr = tag_on->filter->source ;
         len = strlen(ptr) ;
         file4seq_write( &r4->seqwrite, ptr, len ) ;
      }
      file4seq_write_repeat( &r4->seqwrite,
         r4->blocklen - GARBAGE_LEN - len - 220 - sizeof(tag_on->header), 0 );
   }
   file4len_set( &i4->file, i4->header.eof * 512) ;

   if ( file4seq_write_flush(&r4->seqwrite) < 0 )
      return -1 ;
   return 0 ;
}

int r4reindex_write_keys( R4REINDEX *r4 )
{
   TAG4 *t4 ;
   char  last_key[I4MAX_KEY_SIZE], *key_data ;
   int   is_unique, rc, is_first ;
   void *dummy_ptr ;
   long  key_rec ;

   t4 = r4->tag ;

   r4->grouplen = t4->header.group_len ;
   r4->valuelen = t4->header.key_len ;
   r4->keysmax = t4->header.keys_max ;
   memset( (void *)r4->start_block, 0, r4->n_blocks*r4->blocklen ) ;

   if ( sort4get_init( &r4->sort ) < 0 )
      return -1 ;

   file4seq_write_init( &r4->seqwrite, &r4->index->file, (r4->lastblock+r4->lastblock_inc)*512, r4->buffer,r4->buffer_len) ;

   #ifdef S4DEBUG
      if ( I4MAX_KEY_SIZE < r4->sort.sort_len )
         e4severe( e4info, E4_INFO_EXK ) ;
   #endif  /* S4DEBUG */

   is_first = 1 ;
   is_unique = t4->header.unique ;

   for(;;)  /* For each key to write */
   {
      rc = sort4get( &r4->sort, &key_rec, (void **) &key_data, &dummy_ptr) ;
      if ( rc < 0 )
         return -1 ;

      #ifdef S4DEBUG
         if ( r4->key_count < 0L  ||  r4->key_count == 0L && rc != 1
              ||  r4->key_count > 0L && rc == 1 )
            e4severe( e4info, E4_I4REINDEX_WK ) ;
         r4->key_count-- ;
      #endif  /* S4DEBUG */

      if ( rc == 1 )  /* No more keys */
      {
         if ( r4reindex_finish(r4) < 0 )
            return -1 ;
         if ( file4seq_write_flush(&r4->seqwrite) < 0 )
            return -1 ;
         break ;
      }

      if ( is_unique )
      {
         if( is_first )
            is_first = 0 ;
         else
            if ( (*t4->cmp)( key_data, last_key, r4->sort.sort_len) == 0 )
            {
               switch( t4->unique_error )
               {
                  case e4unique:
                     return e4describe( r4->code_base, e4unique, E4_UNIQUE, t4->alias, (char *) 0 ) ;
   
                  case r4unique:
                     return r4unique ;
   
                  default:
                     continue ;
               }
            }
         memcpy( last_key, key_data, r4->sort.sort_len ) ;
      }

      /* Add the key */
      if ( r4reindex_add( r4, key_rec, key_data) < 0 )
         return -1 ;
   }

   /* Now complete the tag header info. */
   t4->header.root = r4->lastblock ;
   return 0 ;
}

int r4reindex_add( R4REINDEX *r4, long rec, char *key_value )
{
   B4KEY_DATA *key_to ;
   R4BLOCK_DATA *start_block ;
   #ifdef S4DEBUG
      long  dif ;
   #endif  /* S4DEBUG */

   start_block = r4->start_block ;
   if ( start_block->n_keys >= r4->keysmax )
   {
      if ( r4reindex_todisk(r4) < 0 )
         return -1 ;
      memset( (void *)start_block, 0, r4->blocklen ) ;
   }

   key_to = (B4KEY_DATA *)
        (start_block->info + (start_block->n_keys++) * r4->grouplen) ;

   #ifdef S4DEBUG
      dif = (char *) key_to -  (char *) start_block ;
      if ( ((unsigned)dif + r4->grouplen) > r4->blocklen || dif < 0 )
         e4severe( e4result, E4_I4REINDEX_ADD ) ;
   #endif  /* S4DEBUG */
   key_to->num = rec ;
   memcpy( (void *)key_to->value, key_value, r4->valuelen ) ;

   return 0 ;
}

int r4reindex_finish( R4REINDEX *r4 )
{
   R4BLOCK_DATA *block ;
   int i_block ;
   B4KEY_DATA *key_to ;
   #ifdef S4DEBUG
      long dif ;
   #endif  /* S4DEBUG */

   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int i ;
      long long_val ;
      short short_val ;
   #endif  /* S4BYTE_SWAP */

   #ifdef S4BYTE_SWAP
      swap = (char *) u4alloc( r4->code_base, r4->blocklen ) ;
      if ( swap == 0 )
         return -1 ;

      memcpy( (void *)swap, (void *)r4->start_block, r4->blocklen ) ;
                       /* position swap_ptr at beginning of B4KEY's */
      swap_ptr = swap ;
      swap_ptr += 6 + sizeof(short) ;
                       /* move through all B4KEY's to swap 'long' */
      for ( i = 0 ; i < (* (short *)swap) ; i++ )
      {
         long_val = x4reverse_long( *(long *)swap_ptr ) ;
         memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
         swap_ptr += r4->grouplen ;
      }
                       /* swap the num_keys value */
      short_val = x4reverse_short( *(short *)swap ) ;
      memcpy( swap, (void *) &short_val, sizeof(short) ) ;

      if ( file4seq_write(&r4->seqwrite, swap ,r4->blocklen) < 0 )
      {
         u4free( swap ) ;
         return -1 ;
      }
   #else
      if ( file4seq_write(&r4->seqwrite, r4->start_block, r4->blocklen) < 0 )
         return -1 ;
   #endif  /* S4BYTE_SWAP */

   r4->lastblock += r4->lastblock_inc ;
   block = r4->start_block ;

   for( i_block=1; i_block < r4->n_blocks; i_block++ )
   {
      block = (R4BLOCK_DATA *) ((char *)block + r4->blocklen) ;
      if ( block->n_keys >= 1 )
      {
         key_to = (B4KEY_DATA *) (block->info + block->n_keys*r4->grouplen) ;
         #ifdef S4DEBUG
            dif = (char *) key_to  -  (char *) block ;
            if ( ( (unsigned)dif + sizeof( long ) ) > r4->blocklen  ||  dif < 0 )
               e4severe( e4result, E4_I4REINDEX_FN ) ;
         #endif  /* S4DEBUG */
         key_to->num = r4->lastblock ;

         #ifdef S4BYTE_SWAP
            memcpy( (void *)swap, (void *)block, r4->blocklen ) ;
                             /* position swap_ptr at beginning of B4KEY's */
            swap_ptr = swap ;
            swap_ptr += 6 + sizeof(short) ;
                             /* move through all B4KEY's to swap 'long' */
            for ( i = 0 ; i < (*(short *)swap) ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               swap_ptr += r4->grouplen ;
            }
                             /* this is a branch */
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;

                             /* swap the num_keys value */
            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            if ( file4seq_write( &r4->seqwrite, swap, r4->blocklen) < 0)
            {
               u4free( swap ) ;
               return -1 ;
            }
         #else
            if ( file4seq_write( &r4->seqwrite, block, r4->blocklen) < 0 )
               return -1;
         #endif  /* S4BYTE_SWAP */

         r4->lastblock += r4->lastblock_inc ;
      }
   }
   #ifdef S4BYTE_SWAP
      u4free( swap ) ;
   #endif  /* S4BYTE_SWAP */
   return 0 ;
}

int r4reindex_todisk( R4REINDEX *r4 )
{
   R4BLOCK_DATA *block ;
   int i_block ;
   B4KEY_DATA *key_on, *keyto ;
   #ifdef S4DEBUG
      long dif ;
   #endif  /* S4DEBUG */

   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int i ;
      long long_val ;
      short short_val ;
   #endif  /* S4BYTE_SWAP */

   /* Writes out the current block and adds references to higher blocks */
   block = r4->start_block ;
   i_block= 0 ;

   key_on = (B4KEY_DATA *) (block->info + (block->n_keys-1) * r4->grouplen) ;

   #ifdef S4DEBUG
      dif = (char *) key_on -  (char *) block ;
      if ( ( (unsigned)dif + r4->grouplen ) > r4->blocklen || dif < 0 )
         e4severe( e4result, E4_I4REINDEX_TD ) ;
   #endif  /* S4DEBUG */

   for(;;)
   {
      #ifdef S4BYTE_SWAP
         swap = (char *) u4alloc_er( r4->code_base, r4->blocklen + sizeof(long) ) ;
         if ( swap == 0 )
            return -1 ;

         memcpy( (void *)swap, (void *)block, r4->blocklen ) ;
                          /* position swap_ptr at beginning of B4KEY's */
         swap_ptr = swap ;
         swap_ptr += 6 + sizeof(short) ;
                          /* move through all B4KEY's to swap 'long' */
         for ( i = 0 ; i < (*(short *)swap) ; i++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
            swap_ptr += r4->grouplen ;
         }

         long_val = x4reverse_long( *(long *)swap_ptr ) ;
         memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;

                          /* swap the num_keys value */
         short_val = x4reverse_short( *(short *)swap ) ;
         memcpy( swap, (void *) &short_val, sizeof(short) ) ;

         i = file4seq_write( &r4->seqwrite, swap, r4->blocklen) ;
         u4free( swap ) ;
         if ( i < 0 )
            return -1 ;
      #else
         if ( file4seq_write( &r4->seqwrite, block, r4->blocklen) < 0 )
            return -1 ;
      #endif  /* S4BYTE_SWAP */

      if ( i_block )
         memset( (void *)block, 0, r4->blocklen ) ;
      r4->lastblock += r4->lastblock_inc ;

      block = (R4BLOCK_DATA *) ((char *)block + r4->blocklen) ;
      i_block++ ;
      #ifdef S4DEBUG
         if ( i_block >= r4->n_blocks )
            e4severe( e4info, E4_I4REINDEX_TD ) ;
      #endif  /* S4DEBUG */

      keyto = (B4KEY_DATA *) (block->info +  block->n_keys * r4->grouplen) ;
      #ifdef S4DEBUG
         dif = (char *) keyto -  (char *) block  ;
         if ( ( (unsigned)dif + sizeof( long ) ) > r4->blocklen || dif < 0 )
            e4severe( e4result, E4_I4REINDEX_TD ) ;
      #endif  /* S4DEBUG */
      keyto->num = r4->lastblock ;

      if ( block->n_keys < r4->keysmax )
      {
         block->n_keys++ ;
         #ifdef S4DEBUG
            if ( ((unsigned)dif+r4->grouplen) > r4->blocklen )
               e4severe( e4result, E4_I4REINDEX_TD ) ;
         #endif  /* S4DEBUG */
         memcpy( keyto->value, key_on->value, r4->valuelen ) ;
         return 0 ;
      }
   }
}
#endif  /* ifdef S4MDX */

#ifdef S4FOX
int r4reindex_tag_headers_write( R4REINDEX *r4 )
{
   TAG4 *tag_on ;
   INDEX4 *i4 ;
   int tot_len, expr_hdr_len ;
   int i_tag = 2 ;
   char *ptr ;

   i4 = r4->index ;

   /* Now write the headers  */
   file4seq_write_init( &r4->seqwrite, &i4->file, 0L, r4->buffer, r4->buffer_len ) ;

   i4->tag_index->header.free_list = 0L ;
   expr_hdr_len = 3*sizeof(short) + 4*sizeof(char) ;
   i4->eof = r4->lastblock + B4BLOCK_SIZE ;

   if ( i4->tag_index->header.type_code >= 64 )
   {
      file4seq_write( &r4->seqwrite, &i4->tag_index->header, T4HEADER_WR_LEN ) ;  /* write first header part */
      file4seq_write_repeat( &r4->seqwrite, 486, 0 ) ;
      file4seq_write( &r4->seqwrite, &i4->tag_index->header.descending, expr_hdr_len ) ;
      file4seq_write_repeat( &r4->seqwrite, 512, 0 ) ;  /* no expression */
   }

   for ( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on == 0 )
         break ;
      if ( i4->tag_index->header.type_code >= 64 )
         tag_on->header_offset = ((long)i_tag) * B4BLOCK_SIZE ;
      else
         tag_on->header_offset = 0L ;

      if ( file4seq_write( &r4->seqwrite, &tag_on->header, T4HEADER_WR_LEN) < 0 )
         return -1 ;
      file4seq_write_repeat( &r4->seqwrite, 486, 0 ) ;
      file4seq_write( &r4->seqwrite, &tag_on->header.descending, expr_hdr_len ) ;

      ptr = tag_on->expr->source ;
      tot_len = tag_on->header.expr_len ;
      file4seq_write( &r4->seqwrite, ptr, tag_on->header.expr_len ) ;

      if ( tag_on->filter != 0 )
      {
         ptr = tag_on->filter->source ;
         file4seq_write( &r4->seqwrite, ptr, tag_on->header.filter_len ) ;
         tot_len += tag_on->header.filter_len ;
      }
      file4seq_write_repeat( &r4->seqwrite, B4BLOCK_SIZE - tot_len, 0 );
      i_tag += 2 ;
   }
   file4len_set( &i4->file, i4->eof ) ;

   if ( file4seq_write_flush(&r4->seqwrite) < 0 )
      return -1 ;
   return 0 ;

}

int r4reindex_write_keys( R4REINDEX *r4 )
{
   char last_key[I4MAX_KEY_SIZE], *key_data ;
   int is_unique, rc, c_len, t_len, last_trail ;
   int on_count, is_first ;
   unsigned int k_len ;
   void *dummy_ptr ;
   long key_rec ;
   TAG4 *t4 ;
   unsigned long ff, r_len ;
   R4BLOCK_DATA *r4block ;

   t4 = r4->tag ;
   k_len = t4->header.key_len ;
   ff = 0xFFFFFFFF ;
   is_first = 1 ;

   memset( last_key, r4->tag->p_char, k_len ) ;

   for ( c_len = 0 ; k_len ; k_len >>= 1, c_len++ ) ;
   k_len = t4->header.key_len ;  /* reset the key length */
   r4->node_hdr.trail_cnt_len = r4->node_hdr.dup_cnt_len = (unsigned char)c_len ;

   r4->node_hdr.trail_byte_cnt = (unsigned char)(0xFF >> ( 8 - ((c_len / 8) * 8 + c_len % 8))) ;
   r4->node_hdr.dup_byte_cnt = r4->node_hdr.trail_byte_cnt ;

   r_len = d4reccount( r4->data ) ;
   for ( c_len = 0 ; r_len ; r_len>>=1, c_len++ ) ;
   r4->node_hdr.rec_num_len = (unsigned char) c_len ;
   if ( r4->node_hdr.rec_num_len < 12 )
      r4->node_hdr.rec_num_len = 12 ;

   for( t_len = r4->node_hdr.rec_num_len + r4->node_hdr.trail_cnt_len + r4->node_hdr.dup_cnt_len ;
        (t_len / 8)*8 != t_len ; t_len++, r4->node_hdr.rec_num_len++ ) ;  /* make at an 8-bit offset */

   r4->node_hdr.rec_num_mask = ff >> ( sizeof(long)*8 - r4->node_hdr.rec_num_len ) ;

   r4->node_hdr.info_len = (unsigned char)((r4->node_hdr.rec_num_len + r4->node_hdr.trail_cnt_len + r4->node_hdr.dup_cnt_len) / 8) ;
   r4->valuelen = t4->header.key_len ;
   r4->grouplen = t4->header.key_len + 2*sizeof(long) ;

   memset( (void *)r4->start_block, 0, r4->n_blocks * B4BLOCK_SIZE ) ;

   #ifdef S4FOX
      for ( r4block = r4->start_block, on_count = 0 ; on_count < r4->n_blocks;
            r4block = (R4BLOCK_DATA *) ( (char *)r4block + B4BLOCK_SIZE), on_count++ )
      {
         memset( (void *)r4block, 0, B4BLOCK_SIZE ) ;
         r4block->header.left_node = -1 ;
         r4block->header.right_node = -1 ;
      }
   #endif  /* S4FOX */

   r4->node_hdr.free_space = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;
   r4->keysmax = (B4BLOCK_SIZE - sizeof( B4STD_HEADER ) ) / r4->grouplen ;

   #ifdef S4DEBUG
      if ( r4->node_hdr.free_space <= 0 || r4->keysmax <= 0 )
         e4severe( e4info, E4_I4REINDEX_WK ) ;
   #endif

   if ( sort4get_init( &r4->sort ) < 0 )
      return -1 ;

   file4seq_write_init( &r4->seqwrite, &r4->index->file, r4->lastblock+B4BLOCK_SIZE, r4->buffer,r4->buffer_len) ;

   #ifdef S4DEBUG
      if ( I4MAX_KEY_SIZE < r4->sort.sort_len )
         e4severe( e4info, E4_I4REINDEX_WK ) ;
   #endif  /* S4DEBUG */
   last_trail = k_len ;   /* default is no available duplicates */
   is_unique = t4->header.type_code & 0x01 ;

   for(;;)  /* For each key to write */
   {
      if ( (rc = sort4get( &r4->sort, &key_rec, (void **) &key_data, &dummy_ptr)) < 0 )
         return -1 ;

      #ifdef S4DEBUG
         if ( r4->key_count < 0L || r4->key_count == 0L && rc != 1 || r4->key_count > 0L && rc == 1 )
            e4severe( e4info, E4_I4REINDEX_WK ) ;
         r4->key_count-- ;
      #endif  /* S4DEBUG */

      if ( rc == 1 )  /* No more keys */
      {
         if ( r4reindex_finish(r4, last_key ) < 0 )
            return -1 ;
         if ( file4seq_write_flush(&r4->seqwrite) < 0 )
            return -1 ;
         break ;
      }

      if ( is_unique )
      {
         if( is_first )
            is_first = 0 ;
         else
            if ( (memcmp)( key_data, last_key, r4->sort.sort_len) == 0 )
            {
               switch( t4->unique_error )
               {
                  case e4unique:
                     return e4describe( r4->code_base, e4unique, E4_UNIQUE, t4->alias, (char *) 0 ) ;
   
                  case r4unique:
                     return r4unique ;
   
                  default:
                     continue ;
               }
            }
      }

      /* Add the key */
      if ( r4reindex_add( r4, key_rec, key_data, last_key, &last_trail ) < 0 )
          return -1 ;
      memcpy( last_key, key_data, r4->sort.sort_len ) ;
   }

   /* Now complete the tag header info. */
   t4->header.root = r4->lastblock ;
   return 0 ;
}

/* for compact leaf nodes only */
int r4reindex_add( R4REINDEX *r4, long rec, char *key_value, char *last_key, int *last_trail )
{
   R4BLOCK_DATA *start_block ;
   int dup_cnt, trail, k_len, i_len, len ;
   unsigned char buffer[6] ;
   char *info_pos ;

   start_block = r4->start_block ;
   k_len = r4->valuelen ;
   i_len = r4->node_hdr.info_len ;

   if ( start_block->header.n_keys == 0 )   /* reset */
   {
      dup_cnt = 0 ;
      r4->cur_pos = ((char *)start_block) + B4BLOCK_SIZE ;
      memcpy( ((char *)start_block) + sizeof( B4STD_HEADER ), (void *)&r4->node_hdr, sizeof( B4NODE_HEADER ) ) ;
      start_block->header.node_attribute |= 2 ;   /* leaf block */
      *last_trail = k_len ;
   }
   else
      dup_cnt = b4calc_dups( key_value, last_key, k_len ) ;

   if ( dup_cnt > k_len - *last_trail )  /* don't allow duplicating trail bytes */
      dup_cnt = k_len - *last_trail ;

   if ( dup_cnt == k_len ) /* duplicate key */
      trail = 0 ;
   else
      trail = b4calc_blanks( key_value, k_len, r4->tag->p_char ) ;

   *last_trail = trail ;

   if ( dup_cnt > k_len - *last_trail )  /* watch for case where < ' ' exissts */
      dup_cnt = k_len - *last_trail ;

   len = k_len - dup_cnt - trail ;
   if ( r4->node_hdr.free_space < i_len + len )
   {
      if ( r4reindex_todisk(r4, last_key) < 0 )
          return -1 ;
      r4->node_hdr.free_space = B4BLOCK_SIZE - sizeof( B4STD_HEADER ) - sizeof( B4NODE_HEADER ) ;
      dup_cnt = 0 ;
      r4->cur_pos = ((char *)start_block) + B4BLOCK_SIZE ;
      memcpy( ((char *)&start_block->header) + sizeof( B4STD_HEADER ), (void *)&r4->node_hdr, sizeof( B4NODE_HEADER ) ) ;
      start_block->header.node_attribute |= 2 ;   /* leaf block */
      trail = b4calc_blanks( key_value, k_len, r4->tag->p_char ) ;
      len = k_len - trail ;
   }

   r4->cur_pos -= len ;
   memcpy( r4->cur_pos, key_value + dup_cnt, len ) ;

   info_pos = ((char *)&start_block->header) + sizeof(B4STD_HEADER) +
               sizeof(B4NODE_HEADER) + start_block->header.n_keys*i_len ;
   x4put_info( &r4->node_hdr, buffer, rec, trail, dup_cnt ) ;
   memcpy( info_pos, buffer, i_len ) ;

   r4->node_hdr.free_space -= ( len + i_len ) ;
   start_block->header.n_keys++ ;
   return 0 ;
}

int r4reindex_finish( R4REINDEX *r4, char *key_value )
{
   R4BLOCK_DATA *block ;
   int i_block ;
   long rev_lb ;
   char *keyto ;

   block = r4->start_block ;

   if ( r4->n_blocks_used <= 1 ) /* just output first block */
   {
      memcpy( ((char *)block) + sizeof( B4STD_HEADER ), (void *)&r4->node_hdr, sizeof( B4NODE_HEADER ) ) ;
      block->header.node_attribute |= (short)3 ;   /* leaf and root block */
      if ( file4seq_write( &r4->seqwrite, block,B4BLOCK_SIZE) < 0 )
         return -1;
      r4->lastblock += B4BLOCK_SIZE ;
   }
   else
   {
      long l_recno ;

      memcpy( (void *)&l_recno, ((char *) (&block->header)) + sizeof(B4STD_HEADER)
              + sizeof(B4NODE_HEADER) + (block->header.n_keys - 1) * r4->node_hdr.info_len, sizeof( long ) ) ;
      l_recno &= r4->node_hdr.rec_num_mask ;

      if ( block->header.node_attribute >= 2 )  /* if leaf, record free_space */
         memcpy( ((char *)block) + sizeof( B4STD_HEADER ), (void *)&r4->node_hdr, sizeof( B4NODE_HEADER ) ) ;

      if ( file4seq_write(&r4->seqwrite, r4->start_block,B4BLOCK_SIZE) < 0 )
         return -1 ;
      r4->lastblock += B4BLOCK_SIZE ;

      l_recno = x4reverse_long( l_recno ) ;

      for( i_block=1; i_block < r4->n_blocks_used ; i_block++ )
      {
         block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE) ;
         if ( block->header.n_keys >= 1 )
         {
            keyto = ((char *) (&block->header)) + sizeof(B4STD_HEADER) + block->header.n_keys * r4->grouplen ;
            block->header.n_keys++ ;
            #ifdef S4DEBUG
               if ( (char *) keyto -  (char *) block + r4->grouplen > B4BLOCK_SIZE ||
                    (char *) keyto -  (char *) block < 0 )
                  e4severe( e4result, E4_I4REINDEX_FN ) ;
            #endif  /* S4DEBUG */
            memcpy( keyto, (void *)key_value, r4->valuelen ) ;
            keyto += r4->valuelen ;
            memcpy( keyto, (void *)&l_recno, sizeof( long ) ) ;
            rev_lb = x4reverse_long( r4->lastblock ) ;
            memcpy( keyto + sizeof( long ), (void *)&rev_lb, sizeof( long ) ) ;

            if ( i_block == r4->n_blocks_used - 1 )
               block->header.node_attribute = 1 ;  /* root block */
            if ( file4seq_write( &r4->seqwrite, block, B4BLOCK_SIZE) < 0)
               return -1;
            r4->lastblock += B4BLOCK_SIZE ;
         }
      }
   }

   return 0 ;
}

/* Writes out the current block and adds references to higher blocks */
int r4reindex_todisk( R4REINDEX *r4, char* key_value )
{
   R4BLOCK_DATA *block ;
   long l_recno, rev_lb ;
   int tn_used ;
   char *keyto ;
   #ifdef S4DEBUG
      int i_block = 0 ;

      i_block = 0 ;
   #endif

   block = r4->start_block ;
   tn_used = 1 ;

   memcpy( (void *)&l_recno, ((unsigned char *) (&block->header)) + sizeof(B4STD_HEADER)
           + sizeof(B4NODE_HEADER) + (block->header.n_keys - 1) * r4->node_hdr.info_len, sizeof( long ) ) ;
   l_recno &= r4->node_hdr.rec_num_mask ;
   l_recno = x4reverse_long( l_recno ) ;

   for(;;)
   {
      tn_used++ ;
      r4->lastblock += B4BLOCK_SIZE ;
      /* next line only works when on leaf branches... */
      block->header.right_node = r4->lastblock + B4BLOCK_SIZE ;
      if ( block->header.node_attribute >= 2 )  /* if leaf, record free_space */
         memcpy( ((char *)block) + sizeof( B4STD_HEADER ), (void *)&r4->node_hdr.free_space, sizeof( r4->node_hdr.free_space ) ) ;
      if ( file4seq_write( &r4->seqwrite, block, B4BLOCK_SIZE) < 0 )
         return -1 ;
      memset( (void *)block, 0, B4BLOCK_SIZE ) ;
      block->header.left_node = r4->lastblock ;
      block->header.right_node = -1 ;

      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE) ;
      #ifdef S4DEBUG
         i_block++ ;
         if ( i_block >= r4->n_blocks )
            e4severe( e4info, E4_I4REINDEX_TD ) ;
      #endif  /* S4DEBUG */

      if ( block->header.n_keys < r4->keysmax )
      {
         keyto = ((char *) (&block->header)) + sizeof(B4STD_HEADER) + block->header.n_keys * r4->grouplen ;
         block->header.n_keys++ ;
         #ifdef S4DEBUG
            if ( (char *) keyto -  (char *) block + r4->grouplen > B4BLOCK_SIZE || (char *) keyto -  (char *) block < 0 )
               e4severe( e4result, E4_I4REINDEX_TD ) ;
         #endif  /* S4DEBUG */
         memcpy( keyto, (void *)key_value, r4->valuelen ) ;
         keyto += r4->valuelen ;
         memcpy( keyto, (void *)&l_recno, sizeof( long ) ) ;
         rev_lb = x4reverse_long( r4->lastblock ) ;
         memcpy( keyto + sizeof( long ), (void *)&rev_lb, sizeof( long ) ) ;

         if ( block->header.n_keys < r4->keysmax )  /* then done, else do next one up */
         {
            if ( tn_used > r4->n_blocks_used )
               r4->n_blocks_used = tn_used ;
            return 0 ;
         }
      }
      #ifdef S4DEBUG
         else  /* should never occur */
            e4severe( e4result, E4_I4REINDEX_TD ) ;
      #endif  /* S4DEBUG */
   }
}
#endif  /* S4FOX */
#endif  /* N4OTHER  */
#endif  /* S4INDEX_OFF */
#endif  /* S4WRITE_OFF */
