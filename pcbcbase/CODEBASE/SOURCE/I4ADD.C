/* i4add.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef N4OTHER
#ifndef S4INDEX_OFF
#ifndef S4OFF_WRITE

#include "r4reinde.h"

#ifdef S4MDX
#define GARBAGE_LEN 518

int r4reindex_tag_headers_write_special( R4REINDEX *r4 )
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
                  e4severe( e4result, E4_I4REINDEX_THWS ) ;
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

   i4->header.eof = r4->lastblock + r4->lastblock_inc + 2 ;
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
         if ( tag_on->header_offset == 0 )
            tag_on->header_offset = file4len( &r4->index->file ) ;

         tag_info.header_pos = tag_on->header_offset / 512 ;

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

   return 0 ;
}
#endif

static int i4add_one_tag( INDEX4 *i4, TAG4INFO *tag_data )
{
   int rc ;
   TAG4 *tag_ptr ;
   R4REINDEX reindex ;
   DATA4 *d4 ;
   CODE4 *c4 ;
   char *ptr ;
   #ifdef S4FOX
      int keysmax, expr_type ;
      B4BLOCK *block ;
      long r_node, go_to ;
      int tot_len, expr_hdr_len ;
   #endif
   #ifdef S4MDX
      int len ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;       

   d4 = i4->data ; 
   c4 = d4->code_base ;
   rc = 0 ;

   if ( r4reindex_init( &reindex, i4 ) < 0 )
      return -1 ;

   tag_ptr = (TAG4 *) mem4alloc( c4->tag_memory ) ;
   if ( tag_ptr == 0 )
      e4(  c4, e4memory, 0 ) ;

   memset( (void *)tag_ptr, 0, sizeof( TAG4 ) ) ;
   tag_ptr->code_base = c4 ;
   tag_ptr->index = i4 ;

   u4ncpy( tag_ptr->alias, tag_data[0].name, sizeof(tag_ptr->alias) ) ;
   c4upper( tag_ptr->alias ) ;

   for ( ;; )
   {
   #ifdef S4MDX
      i4->header.num_tags++ ;

      tag_ptr->header.type_code = 0x10 ;
      if ( tag_data[0].unique )
      {
         tag_ptr->header.type_code += 0x40 ;
         tag_ptr->header.unique = 0x4000 ;
         tag_ptr->unique_error = tag_data[0].unique ;

         #ifdef S4DEBUG
            if ( tag_data[0].unique != e4unique &&
                 tag_data[0].unique != r4unique &&
                 tag_data[0].unique != r4unique_continue )
               e4severe( e4parm, E4_PARM_UNI ) ;
         #endif
      }
      if ( tag_data[0].descending)
      {
         tag_ptr->header.type_code += 0x08 ;
         #ifdef S4DEBUG
            if ( tag_data[0].descending != r4descending )
               e4severe( e4parm, E4_PARM_FLA ) ;
         #endif
      }

      #ifdef S4DEBUG
         if ( tag_data[0].expression == 0 )
            e4severe( e4parm, E4_PARM_TAG ) ;
      #endif

      tag_ptr->expr = expr4parse( d4, tag_data[0].expression ) ;
      if( tag_ptr->expr == 0 )
      {
         rc = -1 ;
         break ;
      }
      if ( tag_data[0].filter != 0 )
         if ( *(tag_data[0].filter) != '\0' )
            tag_ptr->filter = expr4parse( d4, tag_data[0].filter ) ;

      if ( c4->error_code < 0 )
         break ;
      l4add( &i4->tags, tag_ptr ) ;

      tag_ptr->header.key_len = expr4key_len( tag_ptr->expr ) ;
      if ( tag_ptr->header.key_len < 0 )
      {
         rc = -1 ;
         break ;
      }
      tag_ptr->header.type = (char)expr4type( tag_ptr->expr ) ;
      if ( tag_ptr->header.type == r4date_doub )
         tag_ptr->header.type = r4date ;
      if ( tag_ptr->header.type == r4num_doub )
         tag_ptr->header.type = r4num ;

      t4init_seek_conv( tag_ptr, tag_ptr->header.type ) ;
      tag_ptr->header.group_len = tag_ptr->header.key_len+ 2*sizeof(long)-1 ;
      tag_ptr->header.group_len-= tag_ptr->header.group_len % sizeof(long) ;

      tag_ptr->header.keys_max = (reindex.index->header.block_rw - sizeof(short) - 6 - sizeof(long)) /
         tag_ptr->header.group_len;
      if ( tag_ptr->header.keys_max < reindex.min_keysmax )
         reindex.min_keysmax = tag_ptr->header.keys_max ;
      tag_ptr->has_keys = 0 ;
      tag_ptr->had_keys = 1 ;
      if ( r4reindex_blocks_alloc(&reindex) < 0 )
      {
         rc = -1 ;
         break ;
      }

      reindex.n_tags = i4->header.num_tags ;

      reindex.lastblock_inc = reindex.index->header.block_rw / 512 ;
      reindex.lastblock = file4len( &reindex.index->file ) / 512 - reindex.lastblock_inc ;

      reindex.tag = tag_ptr ;

      rc = r4reindex_supply_keys( &reindex ) ;
      if ( rc )
         break ;

      rc = r4reindex_write_keys( &reindex ) ;
      if ( rc )
         break ;

      /* regenerate the tag_headers special... */
      rc = r4reindex_tag_headers_write_special( &reindex ) ;

      if ( file4seq_write_flush(&reindex.seqwrite) < 0 )
      {
         rc = -1 ;
         break ;
      }

      file4seq_write_init( &reindex.seqwrite, &i4->file, tag_ptr->header_offset, reindex.buffer, reindex.buffer_len ) ;

      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swap_tag_header, (void *)&tag_ptr->header, sizeof(T4HEADER) ) ;

         swap_tag_header.root = x4reverse_long( swap_tag_header.root ) ;
         swap_tag_header.key_len = x4reverse_short( swap_tag_header.key_len ) ;
         swap_tag_header.keys_max = x4reverse_short( swap_tag_header.keys_max ) ;
         swap_tag_header.group_len = x4reverse_short( swap_tag_header.group_len ) ;
         swap_tag_header.unique = x4reverse_short( swap_tag_header.unique ) ;

         if ( file4seq_write( &reindex.seqwrite, &swap_tag_header, sizeof(T4HEADER)) < 0 )
         {
            rc = -1 ;
            break ;
         }
      #else
         if ( file4seq_write( &reindex.seqwrite, &tag_ptr->header, sizeof(T4HEADER)) < 0 )
         {
            rc = -1 ;
            break ;
         }
      #endif  /* S4BYTE_SWAP */

      ptr = tag_ptr->expr->source ;
      len = strlen(ptr) ;
      file4seq_write( &reindex.seqwrite, ptr, len) ;
      file4seq_write_repeat( &reindex.seqwrite, 221-len, 0 ) ;

      if( tag_ptr->filter != 0 )
      {
         file4seq_write_repeat( &reindex.seqwrite, 1, 1 ) ;
         if (tag_ptr->has_keys)
            file4seq_write_repeat( &reindex.seqwrite, 1, 1 ) ;
         else
            file4seq_write_repeat( &reindex.seqwrite, 1, 0 ) ;
      }
      else
         file4seq_write_repeat( &reindex.seqwrite, 2, 0 ) ;

      /* write extra space up to filter write point */
      file4seq_write_repeat( &reindex.seqwrite, GARBAGE_LEN-3 , 0 ) ;

      if ( tag_ptr->filter == 0 )
         len = 0 ;
      else
      {
         ptr = tag_ptr->filter->source ;
         len = strlen(ptr) ;
         file4seq_write( &reindex.seqwrite, ptr, len ) ;
      }
      file4seq_write_repeat( &reindex.seqwrite,
         reindex.blocklen - GARBAGE_LEN - len - 220 - sizeof(tag_ptr->header), 0 );
   #endif  /* S4MDX */

   #ifdef S4FOX
      tag_ptr->header.type_code  = 0x60 ;  /* compact */
      if ( tag_data[0].unique )
      {
         tag_ptr->header.type_code += 0x01 ;
         tag_ptr->unique_error = tag_data[0].unique ;

         #ifdef S4DEBUG
            if ( tag_data[0].unique != e4unique &&
                 tag_data[0].unique != r4unique &&
                 tag_data[0].unique != r4unique_continue )
               e4severe( e4parm, E4_PARM_UNI ) ;
         #endif
      }
      if ( tag_data[0].descending)
      {
         tag_ptr->header.descending = 1 ;
         #ifdef S4DEBUG
            if ( tag_data[0].descending != r4descending )
               e4severe( e4parm, E4_PARM_FLA ) ;
         #endif
      }

      #ifdef S4DEBUG
         if ( tag_data[0].expression == 0 )
            e4severe( e4parm, E4_PARM_TAG ) ;
      #endif

      tag_ptr->expr = expr4parse( d4, tag_data[0].expression ) ;
      if ( tag_ptr->expr == 0 )
      {
         rc = -1 ;
         break ;
      }

      tag_ptr->header.expr_len = strlen( tag_ptr->expr->source ) + 1 ;
      if ( tag_data[0].filter != 0 )
         if ( *( tag_data[0].filter ) != '\0' )
         {
            tag_ptr->header.type_code += 0x08 ;
            tag_ptr->filter = expr4parse( d4, tag_data[0].filter ) ;
            if (tag_ptr->filter)
               tag_ptr->header.filter_len = strlen( tag_ptr->filter->source ) ;
         }

      tag_ptr->header.filter_len++ ;  /* minimum of 1, for the '\0' */
      tag_ptr->header.filter_pos = tag_ptr->header.expr_len ;

      if ( c4->error_code < 0 || c4->error_code == r4unique )
         break ;

      reindex.tag = tag_ptr ;
      reindex.n_blocks_used = 0 ;

      reindex.lastblock = file4len( &i4->file ) - B4BLOCK_SIZE ;

      tag_ptr->header.key_len = expr4key_len( tag_ptr->expr ) ;

      expr_type = expr4type( tag_ptr->expr ) ;
      if ( expr_type < 0 )
      {
         rc = -1 ;
         break ;
      }
      t4init_seek_conv( tag_ptr, expr_type ) ;
      if ( tag_ptr->header.key_len < 0 )
      {
         rc = -1 ;
         break ;
      }

      keysmax = ( B4BLOCK_SIZE - sizeof(B4STD_HEADER) ) / ( tag_ptr->header.key_len + 2*sizeof(long) ) ;

      if ( keysmax < reindex.min_keysmax )
         reindex.min_keysmax = keysmax ;

      if ( r4reindex_blocks_alloc(&reindex) < 0 )
      {
         rc = -1 ;
         break ;
      }

      rc = r4reindex_supply_keys( &reindex ) ;
      if ( rc < 0 )
         break ;

      rc = r4reindex_write_keys( &reindex ) ;
      if ( rc < 0 )
         break ;

      /* now must fix the right node branches for all blocks by moving leftwards */
      if ( rc == 0 )
      {
         for( t4rl_bottom( tag_ptr ) ; tag_ptr->blocks.last_node ; t4up( tag_ptr ) )
         {
            block = t4block( tag_ptr ) ;
            r_node = block->header.left_node ;
            go_to = block->header.left_node ;

            while ( go_to != -1 )
            {
               r_node = block->file_block ;
               if ( block->changed )
                  if ( b4flush( block ) < 0 )
                     return -1 ;
               if ( file4read_all( &tag_ptr->index->file, I4MULTIPLY*go_to, &block->header, B4BLOCK_SIZE) < 0 )
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

      if ( file4seq_write( &reindex.seqwrite, &tag_ptr->header, T4HEADER_WR_LEN) < 0 )
         rc = -1 ;

      if ( rc == 0 )
      {
         file4seq_write_repeat( &reindex.seqwrite, 486, 0 ) ;
         expr_hdr_len = 3*sizeof(short) + 4*sizeof(char) ;
         file4seq_write( &reindex.seqwrite, &tag_ptr->header.descending, expr_hdr_len ) ;

         ptr = tag_ptr->expr->source ;
         tot_len = tag_ptr->header.expr_len ;
         file4seq_write( &reindex.seqwrite, ptr, tag_ptr->header.expr_len ) ;

         if ( tag_ptr->filter != 0 )
         {
            ptr = tag_ptr->filter->source ;
            file4seq_write( &reindex.seqwrite, ptr, tag_ptr->header.filter_len ) ;
            tot_len += tag_ptr->header.filter_len ;
         }
         file4seq_write_repeat( &reindex.seqwrite, B4BLOCK_SIZE - tot_len, 0 );

         l4add( &i4->tags, tag_ptr ) ;
         tag_ptr->header_offset = reindex.lastblock + B4BLOCK_SIZE ;
         t4add( i4->tag_index, tag_ptr->alias, tag_ptr->header_offset ) ;
      }
   #endif
      break ;
   }
   if ( rc == 0 )
   {
      if ( file4seq_write_flush(&reindex.seqwrite) < 0 )
          rc = -1 ;
      #ifdef S4FOX
	 i4->eof = reindex.lastblock + 3 * B4BLOCK_SIZE ;
         rc = file4len_set( &i4->file, i4->eof ) ;
      #endif
      #ifdef S4MDX
         rc = file4len_set( &i4->file, i4->header.eof * 512) ;
      #endif
   }

   r4reindex_free( &reindex ) ;
   if ( rc != 0 || i4->code_base->error_code < 0 )
   {
      mem4free( c4->tag_memory, tag_ptr ) ;
      return rc ;       
   }

   return 0 ;
}

/* takes an array of TAG4INFO and adds the input tags to the already existing
   index file i4 */
int S4FUNCTION i4add_tag( INDEX4 *i4, TAG4INFO *tag_data )
{
   int i, save_rc, rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif
   #ifdef S4DEBUG
      int old_tag_error ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 || tag_data == 0 )
         e4severe( e4parm, E4_I4ADD_TAG ) ;
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

   #ifndef S4SINGLE
      #ifdef S4FOX
         if ( i4->file.is_exclusive == 0 )
            i4->tag_index->header.version =  i4->version_old+1 ;
      #endif
   #endif

   save_rc = 0 ;

   for ( i = 0; tag_data[i].name; i++ )
   {
      #ifdef S4DEBUG
	 old_tag_error = i4->code_base->tag_name_error ;
	 i4->code_base->tag_name_error = 0 ;
	 if ( d4tag( i4->data, tag_data[i].name ) != 0 )
            e4severe( e4parm, E4_PARM_TAA ) ;
	 i4->code_base->tag_name_error = old_tag_error ;
      #endif

      rc = i4add_one_tag( i4, &tag_data[i] ) ;
      if ( rc != 0 )
      {
         save_rc = rc ;
         break ;
      }
   }

   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( i4->code_base ) ;
   #endif  /* not S4OPTIMIZE_OFF */

   if ( save_rc < 0 )
      return -1 ;       

   return 0 ;
}

#endif
#endif
#endif
