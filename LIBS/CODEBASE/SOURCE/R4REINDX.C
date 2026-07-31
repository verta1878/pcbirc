/* r4reindx.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"

#ifndef S4OFF_WRITE
#ifndef S4INDEX_OFF
#ifdef N4OTHER

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TURBOC__ */
#endif  /* S4UNIX */

#include "r4reinde.h"

#ifdef S4NDX
   B4KEY_DATA   *r4key( R4BLOCK_DATA *r4, int i, int keylen)
   {
      return (B4KEY_DATA *) (&r4->info+i*keylen) ;
   }
#endif  /* S4NDX */

#ifdef S4CLIPPER
   int  v4clipper_len = 17 ;
   int  v4clipper_dec = 2 ;

   /* CLIPPER */
   B4KEY_DATA   *r4key( R4BLOCK_DATA *r4, int i, int keylen)
   {
      return (B4KEY_DATA *) ((char *)&r4->n_keys + r4->block_index[i]) ;
   }
#endif  /* S4CLIPPER */

int S4FUNCTION i4reindex( INDEX4 *i4 )
{
   int rc ;
   TAG4 *tag_on ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4REINDEX ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0  )
         e4severe( e4parm, E4_I4REINDEX ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4OPTIMIZE_OFF
      has_opt = i4->code_base->has_opt && i4->code_base->opt.num_buffers ;
      if ( has_opt )
         d4opt_suspend( i4->code_base ) ;
   #endif

   #ifndef S4SINGLE
      rc = i4lock( i4 ) ;
      if ( rc )
         return rc  ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      #ifndef S4DETECT_OFF
         i4->code_base->mode |= 0x40 ;
      #endif
   #endif

   for( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on == 0 )
         break ;
      rc = t4reindex( tag_on ) ;
      if ( rc )
         return rc ;
   }
   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( i4->code_base ) ;
   #endif
   return 0 ;
}

int S4FUNCTION t4reindex( TAG4 *t4 )
{
   R4REINDEX reindex ;
   INDEX4 *i4 ;
   int rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif
   #ifdef S4CLIPPER
      B4KEY_DATA *bdata ;
      int i ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   i4 = t4->index ;

   #ifndef S4OPTIMIZE_OFF
      has_opt = i4->code_base->has_opt && i4->code_base->opt.num_buffers ;
      if ( has_opt )
         d4opt_suspend( i4->code_base ) ;
   #endif

   #ifndef S4SINGLE
      {
         rc = i4lock( i4 ) ;
         if ( rc )
            return rc  ;
      }
   #endif

   if ( r4reindex_init( &reindex, t4 ) < 0 )
      return -1 ;
   if ( r4reindex_tag_headers_calc( &reindex, t4 ) < 0 )
      return -1 ;
   if ( r4reindex_blocks_alloc(&reindex) < 0 )
      return -1 ;

   #ifdef S4CLIPPER
      reindex.n_blocks_used = 0 ; 
   #endif  /* S4CLIPPER */

   rc = r4reindex_supply_keys( &reindex, t4 ) ;
   if ( rc < 0 )
   {
      r4reindex_free( &reindex ) ;
      return rc ;
   }

   rc = r4reindex_write_keys( &reindex, t4 ) ;
   if ( rc )
   {
      r4reindex_free( &reindex ) ;
      return rc ;
   }

   rc = r4reindex_tag_headers_write( &reindex, t4 ) ;

   #ifdef S4CLIPPER
      if ( rc )
         return rc ;

      #ifdef S4DEBUG
         t4->check_eof = file4len( &t4->file ) - B4BLOCK_SIZE ;  /* reset verify eof variable */
      #endif

      t4->index->code_base->do_index_verify = 0 ;  /* avoid verify errors due to our partial removal */
      t4bottom( t4 ) ;
      t4->index->code_base->do_index_verify = 1 ;
      t4balance( t4, t4block( t4 ), 1 ) ;

      if ( reindex.stranded )   /* add stranded entry */
         t4add( t4, (unsigned char *)reindex.stranded->value, reindex.stranded->num ) ;

      /* and also add any extra block members */
      if ( reindex.start_block->n_keys < t4->header.keys_half && reindex.n_blocks_used > 1 )
      {
         for ( i = 0 ; i < reindex.start_block->n_keys ; i++ )
         {
            bdata = r4key( reindex.start_block, i, t4->header.key_len ) ;
            t4add( t4, (unsigned char *)bdata->value, bdata->num ) ;
         }
      }

      t4update( t4 ) ;
   #endif  /* S4CLIPPER */

   r4reindex_free( &reindex ) ;
   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( i4->code_base ) ;
   #endif
   return rc ;
}

int r4reindex_init( R4REINDEX *r4, TAG4 *t4 )
{
   INDEX4 *i4 ;

   i4 = t4->index ;

   memset( r4, 0, sizeof( R4REINDEX ) ) ;

   r4->data = t4->index->data ;
   r4->code_base = t4->code_base ;

   r4->min_keysmax = INT_MAX ;
   r4->start_block = 0 ;

   r4->buffer_len = i4->code_base->mem_size_sort_buffer ;
   if ( r4->buffer_len < 1024 )
      r4->buffer_len = 1024 ;

   r4->buffer = (char *)u4alloc_er( r4->code_base, r4->buffer_len ) ;
   if ( r4->buffer == 0 )
      return e4memory ;

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
   #ifdef S4CLIPPER
      long num_sub ;
   #endif  /* S4CLIPPER */

   #ifdef S4DEBUG
      if ( (unsigned) r4->min_keysmax > INT_MAX )
         e4severe( e4info, E4_I4REINDEX_BA ) ;
   #endif

   /* Calculate the block stack height */
   on_count = d4reccount( r4->data ) ;

   #ifdef S4DEBUG
      if ( on_count == - 1 )
         return e4( r4->code_base, e4info, E4_INFO_CAL ) ;
   #endif

   #ifdef S4NDX
      for ( r4->n_blocks = 2; on_count; r4->n_blocks++ )
         on_count /= r4->min_keysmax ;
      r4->start_block = (R4BLOCK_DATA *)u4alloc( (long) B4BLOCK_SIZE * r4->n_blocks ) ;
   #endif  /* S4NDX */

   #ifdef S4CLIPPER
      num_sub = r4->min_keysmax ;
      for ( r4->n_blocks = 0 ; on_count > 0L ; r4->n_blocks++ )
      {
         on_count -= num_sub ;
         num_sub *= r4->min_keysmax ;
      }
      r4->n_blocks ++ ;
      if( r4->n_blocks < 2 )
         r4->n_blocks = 2 ;
      r4->start_block = (R4BLOCK_DATA *) u4alloc( (long) ( B4BLOCK_SIZE + 2 * sizeof( void *) ) * r4->n_blocks ) ;
   #endif  /* S4CLIPPER */

   if ( r4->start_block == 0 )
      return e4( r4->code_base, e4memory, E4_MEMORY_B ) ;

   return 0 ;
}

int r4reindex_supply_keys( R4REINDEX *r4, TAG4 *t4 )
{
   FILE4SEQ_READ seq_read ;
   char *key_result ;
   int i ;
   long count, i_rec ;
   DATA4 *d4 ;

   d4 = r4->data ;
   #ifdef S4DEBUG
      r4->key_count = 0L ;
   #endif

   file4seq_read_init( &seq_read, &d4->file, d4record_position(d4,1L), r4->buffer, r4->buffer_len) ;

   if ( sort4init( &r4->sort, r4->code_base, t4->header.key_len, 0 ) < 0 )
      return -1 ;
   r4->sort.cmp = t4->cmp ;

   count = d4reccount( d4 ) ;

   for ( i_rec = 1L; i_rec <= count; i_rec++ )
   {
      if ( file4seq_read_all( &seq_read, d4->record, d4->record_width ) < 0 )
         return -1 ;
      d4->rec_num = i_rec ;

      #ifndef S4MEMO_OFF
         for ( i = 0; i < d4->n_fields_memo; i++ )
            f4memo_reset( d4->fields_memo[i].field ) ;
      #endif

      t4expr_key( t4, &key_result ) ;

      if ( sort4put( &r4->sort, i_rec, key_result, "" ) < 0)
         return -1 ;
      #ifdef S4DEBUG
         r4->key_count++ ;
      #endif
   }

   return 0 ;
}

#ifdef S4NDX
/* NDX */
int  r4reindex_tag_headers_calc( R4REINDEX *r4, TAG4 *t4 )
{

   if ( t4free_all( t4 ) < 0 )
      return -1 ;

   t4->header.key_len = expr4key_len( t4->expr ) ;
   if ( t4->header.key_len < 0 )
      return -1 ;

   t4->header.type = (char)expr4type( t4->expr ) ;
   if ( t4->header.type == r4date_doub )
      t4->header.type = r4date ;
   if ( t4->header.type == r4num_doub )
      t4->header.type = r4num ;
   if ( t4->header.type < 0 )
      return -1 ;
   t4init_seek_conv( t4, t4->header.type ) ;
   t4->header.group_len = t4->header.key_len + 3*sizeof(long) - t4->header.key_len % sizeof(long) ;
   if ( t4->header.key_len%sizeof(long) == 0 )
      t4->header.group_len -= sizeof(long) ;
   t4->header.int_or_date = ( t4->header.type == r4num || t4->header.type == r4date );
   t4->header.keys_max = ( B4BLOCK_SIZE - 2*sizeof(long)) / t4->header.group_len ;

   if ( t4->header.keys_max < r4->min_keysmax )
      r4->min_keysmax = t4->header.keys_max ;

   r4->lastblock_inc = B4BLOCK_SIZE / 512 ;
   r4->lastblock = 0 ;

   return 0 ;
}

/* NDX */
int r4reindex_tag_headers_write( R4REINDEX *r4, TAG4 *t4 )
{
   int len ;
   char *ptr ;
   #ifdef S4BYTE_SWAP
      I4IND_HEAD_WRITE *swap ;
   #endif

   /* Now write the headers */
   file4seq_write_init( &r4->seqwrite, &t4->file, 0L, r4->buffer, r4->buffer_len ) ;

   t4->header.eof = r4->lastblock + r4->lastblock_inc ;

   #ifdef S4BYTE_SWAP
      swap = (I4IND_HEAD_WRITE *)u4alloc_er( t4->code_base, sizeof( I4IND_HEAD_WRITE ) ) ;
      if ( swap == 0 )
         return -1 ;
      swap->root = x4reverse_long( t4->header.root ) ;
      swap->eof = x4reverse_long( t4->header.eof ) ;
      swap->key_len = x4reverse_short( t4->header.key_len ) ;
      swap->keys_max = x4reverse_short( t4->header.keys_max ) ;
      swap->int_or_date = x4reverse_short( t4->header.int_or_date ) ;
      swap->group_len = x4reverse_short( t4->header.group_len ) ;
      swap->dummy = x4reverse_short( t4->header.dummy ) ;
      swap->unique = x4reverse_short( t4->header.unique ) ;

      file4seq_write( &r4->seqwrite, swap, sizeof(I4IND_HEAD_WRITE) ) ;
      u4free( swap ) ;
   #else
      file4seq_write( &r4->seqwrite, &t4->header.root, sizeof(I4IND_HEAD_WRITE) ) ;
   #endif

   ptr = t4->expr->source ;
   len = strlen(ptr) ;
   file4seq_write( &r4->seqwrite, ptr, len) ;

   file4seq_write_repeat( &r4->seqwrite, 1, ' ' ) ;
   file4seq_write_repeat( &r4->seqwrite, I4MAX_EXPR_SIZE - len, 0 ) ;
   #ifdef S4BYTE_SWAP
      t4->header.version = x4reverse_long( t4->header.version ) ;
      file4seq_write( &r4->seqwrite, &t4->header.version, sizeof( t4->header.version ) ) ;
      t4->header.version = x4reverse_long( t4->header.version ) ;
   #else
      file4seq_write( &r4->seqwrite, &t4->header.version, sizeof( t4->header.version ) ) ;
   #endif
   file4len_set( &t4->file, t4->header.eof * 512) ;
   if ( file4seq_write_flush(&r4->seqwrite) < 0 )
      return -1 ;

   return 0 ;
}
#endif  /* S4NDX */

#ifdef S4CLIPPER
/* CLIPPER */
int  r4reindex_tag_headers_calc( R4REINDEX *r4, TAG4 *t4 )
{
   int expr_type ;

   if ( t4free_all( t4 ) < 0 )
      return -1 ;

   t4->expr->key_len = t4->header.key_len ;
   t4->expr->key_dec = t4->header.key_dec ;
   t4->header.key_len = expr4key_len( t4->expr ) ;
   if( t4->header.key_len < 0 )
      return -1 ;

   expr_type = expr4type( t4->expr ) ;
   if ( expr_type < 0 )
      return -1 ;
   t4init_seek_conv( t4, expr_type ) ;
   t4->header.group_len = t4->header.key_len+8 ;
   r4->keys_half = t4->header.keys_half = (1020/ (t4->header.group_len+2) - 1)/ 2;
   t4->header.sign   = 6 ;
   t4->header.keys_max  = t4->header.keys_half * 2 ;
   if ( t4->header.keys_max < 2 )                 
   {
      e4severe( e4info, E4_INFO_BDC ) ;
      return -1 ;
   }

   if ( t4->header.keys_max < r4->min_keysmax )
      r4->min_keysmax = t4->header.keys_max ;

   r4->lastblock_inc = B4BLOCK_SIZE / 512 ;
   r4->lastblock = 0 ;

   return 0 ;
}

/* CLIPPER */
int r4reindex_tag_headers_write( R4REINDEX *r4, TAG4 *t4 )
{
   int len ;
   char *ptr ;
   #ifdef S4BYTE_SWAP
      I4IND_HEAD_WRITE *swap ;
   #endif

   /* Now write the headers */
   file4seq_write_init( &r4->seqwrite, &t4->file, 0L, r4->buffer, r4->buffer_len ) ;

   t4->header.eof = 0 ;

   #ifdef S4BYTE_SWAP
      swap = (I4IND_HEAD_WRITE *) t4->code_base, u4alloc_er( sizeof(I4IND_HEAD_WRITE ) ) ;
      if ( swap == 0 )
         return -1 ;
      swap->sign = x4reverse_short( t4->header.sign ) ;
      swap->version = x4reverse_short( t4->header.version ) ;
      swap->root = x4reverse_long( t4->header.root ) ;
      swap->eof = x4reverse_long( t4->header.eof ) ;
      swap->group_len = x4reverse_short( t4->header.group_len ) ;
      swap->key_len = x4reverse_short( t4->header.key_len ) ;
      swap->key_dec = x4reverse_short( t4->header.key_dec ) ;
      swap->keys_max = x4reverse_short( t4->header.keys_max ) ;
      swap->keys_half = x4reverse_short( t4->header.keys_half ) ;

      #ifdef S4UNIX
         file4seq_write( &r4->seqwrite, swap, sizeof(I4IND_HEAD_WRITE) - sizeof(short) ) ;
      #else
         file4seq_write( &r4->seqwrite, swap, sizeof(I4IND_HEAD_WRITE) ) ;
      #endif
      u4free( swap ) ;
   #else
      #ifdef S4UNIX
         file4seq_write( &r4->seqwrite, &t4->header.sign, sizeof(I4IND_HEAD_WRITE) - sizeof(short) ) ;
      #else
         file4seq_write( &r4->seqwrite, &t4->header.sign, sizeof(I4IND_HEAD_WRITE) ) ;
      #endif
   #endif

   ptr = t4->expr->source ;
   len = strlen(ptr) ;

   file4seq_write( &r4->seqwrite, ptr, len) ;
   file4seq_write_repeat( &r4->seqwrite, I4MAX_EXPR_SIZE - len, 0 ) ;
   #ifdef S4BYTE_SWAP
      t4->header.unique = x4reverse_long( t4->header.unique ) ;
      file4seq_write( &r4->seqwrite, &t4->header.unique, sizeof( t4->header.unique ) ) ;
      t4->header.unique = x4reverse_long( t4->header.unique ) ;
   #else
      file4seq_write( &r4->seqwrite, &t4->header.unique, sizeof( t4->header.unique ) ) ;
   #endif

   file4seq_write_repeat( &r4->seqwrite, 1, (char)0 ) ;

   #ifdef S4BYTE_SWAP
      t4->header.descending = x4reverse_long( t4->header.descending ) ;
      file4seq_write( &r4->seqwrite, &t4->header.descending, sizeof( t4->header.descending ) ) ;
      t4->header.descending = x4reverse_long( t4->header.descending ) ;
   #else
      file4seq_write( &r4->seqwrite, &t4->header.descending, sizeof( t4->header.descending ) ) ;
   #endif

   if ( t4->filter != 0 )
   {
      ptr = t4->filter->source ;
      len = strlen(ptr) ;

      file4seq_write( &r4->seqwrite, ptr, len) ;
      file4seq_write_repeat( &r4->seqwrite, I4MAX_EXPR_SIZE - len, 0 ) ;
   }

   file4len_set( &t4->file, (r4->lastblock + r4->lastblock_inc) * 512) ;
   if ( file4seq_write_flush(&r4->seqwrite) < 0 )
      return -1 ;

   return 0 ;
}
#endif  /* S4CLIPPER */

int r4reindex_write_keys( R4REINDEX *r4, TAG4 *t4 )
{
   char  last_key[I4MAX_KEY_SIZE], *key_data ;
   int   is_unique, rc, is_first ;
   void *dummy_ptr ;
   long  key_rec ;

   r4->grouplen = t4->header.group_len ;
   r4->valuelen = t4->header.key_len ;
   r4->keysmax  = t4->header.keys_max ;

   #ifdef S4CLIPPER
      memset( r4->start_block, 0, (int)(( (long)B4BLOCK_SIZE + 2 * sizeof( void *) ) * r4->n_blocks) ) ;
   #endif  /* S4CLIPPER */

   #ifdef S4NDX
      memset( r4->start_block, 0, r4->n_blocks*B4BLOCK_SIZE ) ;
   #endif  /* S4NDX */

   if ( sort4get_init( &r4->sort ) < 0 )
      return -1 ;

   file4seq_write_init( &r4->seqwrite, &t4->file, (r4->lastblock+r4->lastblock_inc)*512, r4->buffer,r4->buffer_len) ;

   #ifdef S4DEBUG
      if ( I4MAX_KEY_SIZE < r4->sort.sort_len )
         e4severe( e4info, E4_INFO_EXK ) ;
   #endif

   memset( last_key, 0, sizeof(last_key) ) ;
   is_unique = t4->header.unique ;

   is_first = 1 ;

   for(;;)  /* For each key to write */
   {
      if ( (rc = sort4get( &r4->sort, &key_rec, (void **) &key_data, &dummy_ptr)) < 0)  return -1 ;

      #ifdef S4DEBUG
         if ( r4->key_count < 0L  ||  r4->key_count == 0L && rc != 1
              ||  r4->key_count > 0L && rc == 1 )
            e4severe( e4info, E4_I4REINDEX_WK ) ;
         r4->key_count-- ;
      #endif

      if ( rc == 1 )  /* No more keys */
      {
         if ( r4reindex_finish( r4 ) < 0 )
            return -1 ;
         if ( file4seq_write_flush( &r4->seqwrite ) < 0 )
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
   #ifdef S4NDX
      t4->header.root = r4->lastblock ;
   #endif  /* S4NDX */

   #ifdef S4CLIPPER
      t4->header.root = r4->lastblock * 512 ;
   #endif  /* S4CLIPPER */

   return 0 ;
}

int r4reindex_add( R4REINDEX *r4, long rec, char *key_value )
{
   B4KEY_DATA *key_to ;
   R4BLOCK_DATA *start_block ;
   #ifdef S4DEBUG
      long  dif ;
   #endif
   #ifdef S4CLIPPER
      short offset ;
      int i ;
   #endif  /* S4CLIPPER */

   start_block = r4->start_block ;

   /* for NTX, if keysmax, then todisk() with the latest value... */

   #ifdef S4NDX
      if ( start_block->n_keys >= r4->keysmax )
      {
         if ( r4reindex_todisk(r4) < 0 )
            return -1 ;
         memset( start_block, 0, B4BLOCK_SIZE ) ;
      }
   #endif  /* S4NDX */

   #ifdef S4CLIPPER
      if ( start_block->n_keys == 0 )   /* first, so add references */
      {
         offset = ( r4->keysmax + 2 + ( ( r4->keysmax / 2 ) * 2 != r4->keysmax ) ) * sizeof(short) ;
         start_block->block_index = &start_block->n_keys + 1 ;  /* 1 short off of n_keys */
         for ( i = 0 ; i <= r4->keysmax ; i++ )
             start_block->block_index[i] = r4->grouplen * i + offset ;
         start_block->data = (char *)&start_block->n_keys + start_block->block_index[0] ;  /* first entry */
      }
      if ( start_block->n_keys >= r4->keysmax )
      {
         if ( r4reindex_todisk( r4, rec, key_value ) < 0 )  return -1 ;
         memset( start_block, 0, B4BLOCK_SIZE + 2 * sizeof( void *) ) ;
         return 0 ;
      }
   #endif  /* S4CLIPPER */

   key_to = r4key( start_block, start_block->n_keys++, r4->grouplen ) ;

   #ifdef S4DEBUG
      dif = (char *) key_to -  (char *) start_block ;
      if ( dif + r4->grouplen > B4BLOCK_SIZE || dif < 0 )
         e4severe( e4result, E4_I4REINDEX_ADD ) ;
   #endif
   key_to->num = rec ;
   memcpy( key_to->value, key_value, r4->valuelen ) ;

   return 0 ;
}

#ifdef S4NDX
/* NDX */
int r4reindex_finish( R4REINDEX *r4 )
{
   B4KEY_DATA *key_to ;
   #ifdef S4DEBUG
      long dif ;
   #endif
   int i_block = 1 ;

   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int j ;
      long long_val ;
      short short_val ;
   #endif

   R4BLOCK_DATA *block = r4->start_block ;

   #ifdef S4BYTE_SWAP
      swap = (char *)u4alloc_er( r4->tag->code_base, B4BLOCK_SIZE ) ;
      if ( swap == 0 )
         return -1 ;

      memcpy( (void *)swap, (void *)&r4->start_block->n_keys, B4BLOCK_SIZE ) ;

      /* position swap_ptr at beginning of pointers */
      swap_ptr += swap + 2 + sizeof(short) ;

      for ( i = 0 ; i < (*(short *)swap) ; i++ )
      {
         long_val = x4reverse_long( *(long *)swap_ptr ) ;
         memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
         long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
         memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
         swap_ptr += r4->group_len ;
      }

      long_val = x4reverse_long( *(long *)swap_ptr ) ;
      memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;

      short_val = x4reverse_short( *(short *)swap ) ;
      memcpy( swap, (void *) &short_val, sizeof(short) ) ;

      if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
         return -1 ;
      u4free( swap ) ;
   #else
      if ( file4seq_write(&r4->seqwrite, &r4->start_block->n_keys, B4BLOCK_SIZE) < 0 ) return -1 ;
   #endif

   r4->lastblock += r4->lastblock_inc ;
   block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE ) ;
   for(; i_block < r4->n_blocks; i_block++ )
   {
      if ( block->n_keys >= 1 )
      {
         key_to = r4key( block, block->n_keys, r4->grouplen ) ;
         #ifdef S4DEBUG
            dif = (char *)key_to - (char *)block ;
            if ( dif+sizeof(long) > B4BLOCK_SIZE || dif<0 )
               e4severe( e4result, E4_I4REINDEX_FN ) ;
         #endif
         key_to->pointer = r4->lastblock ;

         #ifdef S4BYTE_SWAP
            swap = (char *)u4alloc_er( r4->tag->code_base, B4BLOCK_SIZE ) ;
            if ( swap == 0 )
               return -1 ;
   
            memcpy( (void *)swap, (void *)&block->n_keys, B4BLOCK_SIZE ) ;

            /* position swap_ptr at beginning of pointers */
            swap_ptr += swap + 2 + sizeof(short) ;

            for ( i = 0 ; i < (*(short *)swap) ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
               swap_ptr += r4->group_len ;
            }

            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;

            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
               return -1 ;
            u4free( swap ) ;
         #else
            if ( file4seq_write( &r4->seqwrite, &block->n_keys, B4BLOCK_SIZE) < 0 )
               return -1;
         #endif

         r4->lastblock += r4->lastblock_inc ;
      }
      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE) ;
   }

   return 0 ;
}

/* NDX */
int r4reindex_todisk( R4REINDEX *r4 )
{
   R4BLOCK_DATA *block ;
   int i_block ;
   B4KEY_DATA *key_on, *keyto ;
   #ifdef S4DEBUG
      long dif ;
   #endif
   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int i ;
      long long_val ;
      short short_val ;
   #endif

   /* Writes out the current block and adds references to higher blocks */
   block  = r4->start_block ;
   i_block= 0 ;

   key_on = r4key( block, block->n_keys-1, r4->grouplen ) ;

   #ifdef S4DEBUG
      dif = (char *) key_on -  (char *) block ;
      if ( dif+ r4->grouplen > B4BLOCK_SIZE || dif < 0 )
         e4severe( e4result, E4_I4REINDEX_TD ) ;
   #endif

   for(;;)
   {
      #ifdef S4BYTE_SWAP
         swap = (char *)u4alloc_er( r4->tag->code_base, B4BLOCK_SIZE ) ;
         if ( swap == 0 )
            return -1 ;

         memcpy( (void *)swap, (void *)&block->n_keys, B4BLOCK_SIZE ) ;
                          /* position swap_ptr at beginning of pointers */

         swap_ptr += swap + 2 + sizeof(short) ;

         for ( i = 0 ; i < (*(short *)swap) ; i++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
            long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
            memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
            swap_ptr += r4->group_len ;
         }

         long_val = x4reverse_long( *(long *)swap_ptr ) ;
         memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;

         short_val = x4reverse_short( *(short *)swap ) ;
         memcpy( swap, (void *) &short_val, sizeof(short) ) ;

         if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 ) return -1 ;
         u4free( swap ) ;
      #else
         if ( file4seq_write( &r4->seqwrite, &block->n_keys, B4BLOCK_SIZE) < 0) return -1;
      #endif

      if ( i_block != 0 )
         memset( block, 0, B4BLOCK_SIZE ) ;
      r4->lastblock += r4->lastblock_inc ;

      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE) ;
      i_block++ ;
      #ifdef S4DEBUG
         if ( i_block >= r4->n_blocks )
            e4severe( e4info, E4_I4REINDEX_TD ) ;
      #endif

      keyto = r4key( block, block->n_keys, r4->grouplen) ;
      #ifdef S4DEBUG
         dif = (char *) keyto -  (char *) block  ;
         if ( dif+sizeof(long) > B4BLOCK_SIZE || dif < 0 )
            e4severe( e4result, E4_I4REINDEX_TD ) ;
      #endif
      keyto->pointer = r4->lastblock ;

      if ( block->n_keys < r4->keysmax )
      {
         block->n_keys++ ;
         #ifdef S4DEBUG
            if ( dif+r4->grouplen > B4BLOCK_SIZE )
               e4severe( e4result, E4_I4REINDEX_TD ) ;
         #endif
         memcpy( keyto->value, key_on->value, r4->valuelen ) ;
         return 0 ;
      }
   }
}
#endif  /* S4NDX */

#ifdef S4CLIPPER
/* CLIPPER */
int r4reindex_finish( R4REINDEX *r4 )
{
   B4KEY_DATA *key_to ;
   #ifdef S4DEBUG
      long dif ;
   #endif
   int i_block = 0, t_block ;

   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int j ;
      long long_val ;
      short short_val ;
   #endif

   R4BLOCK_DATA *block = r4->start_block, *temp_block ;

   short offset ;
   int i ;
   long pointer ;

   if ( r4->n_blocks_used <= 1 )  /* empty database if n_keys = 0 */
   {
      if ( r4->start_block->n_keys == 0 )   /* first, so add references */
      {
         offset = ( r4->keysmax + 2 + ( (r4->keysmax/2)*2 != r4->keysmax ) ) * sizeof(short) ;
         r4->start_block->block_index = &r4->start_block->n_keys + 1 ;  /* 1 short off of n_keys */
         for ( i = 0 ; i <= r4->keysmax ; i++ )
             r4->start_block->block_index[i] = r4->grouplen * i + offset ;
         r4->start_block->data = (char *) &r4->start_block->n_keys + r4->start_block->block_index[0] ;
      }
      i_block ++ ;
      r4->stranded = 0 ;
      pointer = 0 ;

      #ifdef S4BYTE_SWAP
         swap = (char *)u4alloc_er( r4->code_base, B4BLOCK_SIZE ) ;
         if ( swap == 0 )
         {
            e4error( r4->code_base, e4memory, (char *) 0 ) ;
            return -1 ;
         }

         memcpy( (void *)swap, (void *)&r4->start_block->n_keys, B4BLOCK_SIZE ) ;
                          /* position swap_ptr at beginning of pointers */

         short_val = x4reverse_short( *(short *)swap ) ;
         memcpy( swap, (void *) &short_val, sizeof(short) ) ;

         swap_ptr = swap + 2 ;

         for ( j = 0 ; j < r4->keysmax ; j++ )
         {
            short_val = x4reverse_short( *(short *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
            swap_ptr += sizeof(short) ;
         }

         for ( j = 0 ; j < r4->keysmax ; j++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
            long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
            memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
            swap_ptr += r4->grouplen ;
         }

         if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
            return -1 ;
         u4free( swap ) ;
      #else
         if ( file4seq_write( &r4->seqwrite, &r4->start_block->n_keys, B4BLOCK_SIZE) < 0 )
            return -1 ;
      #endif

      r4->lastblock += r4->lastblock_inc ;
   }
   else if ( r4->start_block->n_keys >= r4->keys_half )
   {
      /* just grab the pointer for upward placement where belongs */
      r4->stranded = 0 ;

      #ifdef S4BYTE_SWAP
         swap = (char *) u4alloc_er( r4->code_base, B4BLOCK_SIZE ) ;
         if ( swap == 0 )
            return -1 ;

         memcpy( (void *)swap, (void *)&r4->start_block->n_keys, B4BLOCK_SIZE ) ;
                          /* position swap_ptr at beginning of pointers */

         short_val = x4reverse_short( *(short *)swap ) ;
         memcpy( swap, (void *) &short_val, sizeof(short) ) ;

         swap_ptr = swap + 2 ;
                         /* swap the short pointers to B4KEY_DATA */
         for ( i = 0 ; i < r4->keysmax ; i++ )
         {
            short_val = x4reverse_short( *(short *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
            swap_ptr += sizeof(short) ;
         }
                         /* swap the B4KEY_DATA's */
         for ( i = 0 ; i < r4->keysmax ; i++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
            long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
            memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
            swap_ptr += r4->grouplen ;
         }

         if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
            return -1 ;
         u4free( swap ) ;
      #else
         if ( file4seq_write( &r4->seqwrite, &r4->start_block->n_keys, B4BLOCK_SIZE) < 0 )
            return -1 ;
      #endif

      r4->lastblock += r4->lastblock_inc ;
      pointer = r4->lastblock*512 ;
      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE + 2*sizeof(void *) ) ;
      i_block++ ;
   }
   else       /* stranded entry, so add after */
   {
      /* if less than 1/2 entries, will re-add the required keys later... */
      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE + 2*sizeof(void *) ) ;
      i_block++ ;
      while( block->n_keys == 0 && i_block < r4->n_blocks_used )
      {
         block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE + 2*sizeof(void *) ) ;
         i_block++ ;
      }

      r4->stranded = r4key( block, block->n_keys - 1, 0 ) ;
      block->n_keys -- ;
      if( block->n_keys > 0 )
      {
         #ifdef S4BYTE_SWAP
            swap = (char *)u4alloc_er( r4->code_base, B4BLOCK_SIZE ) ;
            if ( swap == 0 )
               return -1 ;

            memcpy( (void *)swap, (void *)&block->n_keys, B4BLOCK_SIZE ) ;
                             /* position swap_ptr at beginning of pointers */

            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            swap_ptr = swap + 2 ;
                            /* swap the short pointers to B4KEY_DATA */
            for ( i = 0 ; i < r4->keysmax ; i++ )
            {
               short_val = x4reverse_short( *(short *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
               swap_ptr += sizeof(short) ;
            }
                            /* swap the B4KEY_DATA's */
            for ( i = 0 ; i < r4->keysmax ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
               swap_ptr += r4->grouplen ;
            }

            if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
               return -1 ;
            u4free( swap ) ;
         #else
            if ( file4seq_write( &r4->seqwrite, &block->n_keys, B4BLOCK_SIZE) < 0 )
               return -1 ;
         #endif

         r4->lastblock += r4->lastblock_inc ;
         pointer = 0 ;
      }
      else
         pointer = r4key( block, block->n_keys, 0 )->pointer ;
      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE + 2*sizeof(void *) ) ;
      i_block++ ;
   }

   /* now position to the last spot, and place the branch */
   if( i_block < r4->n_blocks_used )
   {
      if( block->n_keys <= r4->keysmax && pointer != 0 )
      {
         temp_block = block ;
         t_block = i_block ;
         while ( temp_block->n_keys == 0 && t_block < r4->n_blocks_used )
         {
            offset = ( r4->keysmax + 2 + ( ( r4->keysmax / 2 ) * 2 != r4->keysmax ) ) * sizeof(short) ;
            temp_block->block_index = &temp_block->n_keys + 1 ;  /* 1 short off of n_keys */
            for ( i = 0 ; i <= r4->keysmax ; i++ )
               temp_block->block_index[i] = r4->grouplen * i + offset ;
            temp_block->data = (char *)&temp_block->n_keys + temp_block->block_index[0] ;
            temp_block = (R4BLOCK_DATA *)((char *)temp_block + B4BLOCK_SIZE + 2 * sizeof(void *) ) ;
            t_block++ ;
         }

         /* now place the pointer for data that goes rightward */
         key_to = r4key( block, block->n_keys, 0 ) ;
         key_to->pointer = pointer ;
         pointer = 0 ;

         #ifdef S4BYTE_SWAP
            swap = (char *) u4alloc_er( r4->code_base, B4BLOCK_SIZE ) ;
            if ( swap == 0 )
               return -1 ;

            memcpy( (void *)swap, (void *)&block->n_keys, B4BLOCK_SIZE ) ;

            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            swap_ptr = swap + 2 ;
                            /* swap the short pointers to B4KEY_DATA */
            for ( i = 0 ; i < r4->keysmax ; i++ )
            {
               short_val = x4reverse_short( *(short *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
               swap_ptr += sizeof(short) ;
            }
                            /* swap the B4KEY_DATA's */
            for ( i = 0 ; i < r4->keysmax ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
               swap_ptr += r4->grouplen ;
            }

            if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
               return -1 ;
            u4free( swap ) ;
         #else
            if ( file4seq_write( &r4->seqwrite, &block->n_keys, B4BLOCK_SIZE) < 0 ) return -1 ;
         #endif

         r4->lastblock += r4->lastblock_inc ;
         block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE + 2*sizeof(void *) ) ;
         i_block++ ;
      }
   }
   for(; i_block < r4->n_blocks_used; i_block++ )
   {
      if ( block->n_keys == 0 )
      {
         offset = ( r4->keysmax + 2 + ( ( r4->keysmax / 2 ) * 2 != r4->keysmax ) ) * sizeof(short) ;
         block->block_index = &block->n_keys + 1 ;  /* 1 short off of n_keys */
         for ( i = 0 ; i <= r4->keysmax ; i++ )
            block->block_index[i] = r4->grouplen * i + offset ;
         block->data = (char *)&block->n_keys + block->block_index[0] ;
      }
      key_to = r4key( block, block->n_keys, r4->grouplen ) ;
      #ifdef S4DEBUG
         dif = (char *)key_to  -  (char *) block ;
         if ( dif + sizeof( long ) > B4BLOCK_SIZE  ||  dif < 0 )
            e4severe( e4result, E4_I4REINDEX_FN ) ;
      #endif
      key_to->pointer = r4->lastblock * 512 ;

      #ifdef S4BYTE_SWAP
            swap = (char *)u4alloc_er( r4->code_base, B4BLOCK_SIZE ) ;
            if ( swap == 0 )
               return -1 ;

            memcpy( (void *)swap, (void *)&block->n_keys, B4BLOCK_SIZE ) ;

            /* position swap_ptr at beginning of pointers */
            short_val = x4reverse_short( *(short *)swap ) ;
            memcpy( swap, (void *) &short_val, sizeof(short) ) ;

            swap_ptr = swap + 2 ;
            /* swap the short pointers to B4KEY_DATA */
            for ( i = 0 ; i < r4->keysmax ; i++ )
            {
               short_val = x4reverse_short( *(short *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
               swap_ptr += sizeof(short) ;
            }
            /* swap the B4KEY_DATA's */
            for ( i = 0 ; i < r4->keysmax ; i++ )
            {
               long_val = x4reverse_long( *(long *)swap_ptr ) ;
               memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
               long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
               memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
               swap_ptr += r4->grouplen ;
            }

            if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 ) return -1 ;
            u4free( swap ) ;
      #else
         if ( file4seq_write( &r4->seqwrite, &block->n_keys, B4BLOCK_SIZE) < 0) return -1;
      #endif

      r4->lastblock += r4->lastblock_inc ;
      block = (R4BLOCK_DATA *)( (char *)block + B4BLOCK_SIZE + 2 * sizeof(void *) ) ;
   }

   return 0 ;
}

/* CLIPPER */
int r4reindex_todisk( R4REINDEX *r4, long rec, char *key_value )
{
   R4BLOCK_DATA *block ;
   int tn_used, i_block, i ;
   B4KEY_DATA *key_to ;
   short offset ;
   #ifdef S4DEBUG
      long dif ;
      B4KEY_DATA *key_on ;
   #endif
   #ifdef S4BYTE_SWAP
      char *swap, *swap_ptr ;
      int j ;
      long long_val ;
      short short_val ;
   #endif

   tn_used = 1 ;

   /* Writes out the current block and adds references to higher blocks */
   block  = r4->start_block ;
   i_block= 0 ;

   #ifdef S4DEBUG
      key_on = r4key( block, block->n_keys, r4->grouplen ) ;
      dif = (char *) key_on -  (char *) &block->n_keys ;
      if ( dif+ r4->grouplen > B4BLOCK_SIZE || dif < 0 )
         e4severe( e4result, E4_I4REINDEX_TD ) ;
   #endif

   for(;;)
   {
      tn_used++ ;
      #ifdef S4BYTE_SWAP
         swap = (char *)u4alloc_er( r4->code_base, B4BLOCK_SIZE ) ;
         if ( swap == 0 )
            return -1 ;

         memcpy( (void *)swap, (void *)&block->n_keys, B4BLOCK_SIZE ) ;

         /* position swap_ptr at beginning of pointers */
         short_val = x4reverse_short( *(short *)swap ) ;
         memcpy( swap, (void *) &short_val, sizeof(short) ) ;

         swap_ptr = swap + 2 ;

         /* swap the short pointers to B4KEY_DATA */
         for ( j = 0 ; j < r4->keysmax ; j++ )
         {
            short_val = x4reverse_short( *(short *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &short_val, sizeof(short) ) ;
            swap_ptr += sizeof(short) ;
         }

         /* swap the B4KEY_DATA's */
         for ( j = 0 ; j < r4->keysmax ; j++ )
         {
            long_val = x4reverse_long( *(long *)swap_ptr ) ;
            memcpy( swap_ptr, (void *) &long_val, sizeof(long) ) ;
            long_val = x4reverse_long( *(long *)(swap_ptr+sizeof(long)) ) ;
            memcpy( swap_ptr+sizeof(long), (void *) &long_val, sizeof(long) ) ;
            swap_ptr += r4->grouplen ;
         }

         if ( file4seq_write( &r4->seqwrite, swap, B4BLOCK_SIZE) < 0 )
            return -1 ;
         u4free( swap ) ;
      #else
         if ( file4seq_write( &r4->seqwrite, &block->n_keys, B4BLOCK_SIZE) < 0 )
            return -1 ;
      #endif
      if ( i_block )
         memset( block, 0, B4BLOCK_SIZE ) ;
      r4->lastblock += r4->lastblock_inc ;

      block = (R4BLOCK_DATA *) ((char *)block + B4BLOCK_SIZE + 2*sizeof(void *) ) ;
      i_block++ ;
      #ifdef S4DEBUG
         if ( i_block >= r4->n_blocks )
            e4severe( e4info, E4_I4REINDEX_TD ) ;
      #endif

      if ( block->n_keys == 0 )   /* set up the branch block... */
      {
         offset = ( r4->keysmax + 2 + ( ( r4->keysmax / 2 ) * 2 != r4->keysmax ) ) * sizeof(short) ;
         block->block_index = &block->n_keys + 1 ;
         for ( i = 0 ; i <= r4->keysmax ; i++ )
            block->block_index[i] = r4->grouplen * i + offset ;
         block->data = (char *) &block->n_keys + block->block_index[ 0 ] ;
      }

      key_to = r4key( block, block->n_keys, r4->grouplen ) ;
      #ifdef S4DEBUG
         dif = (char *) key_to -  (char *) block  ;
         if ( dif+sizeof(long) > B4BLOCK_SIZE || dif < 0 )
            e4severe( e4result, E4_I4REINDEX_TD ) ;
      #endif
      key_to->pointer = r4->lastblock * 512 ;

      if ( block->n_keys < r4->keysmax )
      {
         if ( tn_used > r4->n_blocks_used )
            r4->n_blocks_used = tn_used ;
         #ifdef S4DEBUG
            if ( dif+r4->grouplen > B4BLOCK_SIZE )
               e4severe( e4result, E4_I4REINDEX_TD ) ;
         #endif
         key_to->num = rec ;
         memcpy( key_to->value, key_value, r4->valuelen ) ;
         block->n_keys++ ;
         return 0 ;
      }
      #ifdef S4DEBUG
         if ( block->n_keys > r4->keysmax )
             e4severe( e4info,  E4_INFO_NKE ) ;
      #endif
   }
}
#endif  /* S4CLIPPER */
#endif  /* N4OTHER */
#endif  /* S4INDEX_OFF */
#endif  /* S4WRITE_OFF */
