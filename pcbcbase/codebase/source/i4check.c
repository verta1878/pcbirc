/* i4check.c   (C)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

typedef struct
{
   F4FLAG    flag ;

   TAG4    *tag ;
   char     *old_key ;
   long      old_rec ;
   long      num_recs ;
   int       do_compare ;  /* Do not compare the first time */
   CODE4   *code_base ;
} C4CHECK ;

static int c4check_init( C4CHECK *check, CODE4 *cb, TAG4 *t4, long n_recs )
{
   memset( (void *)check, 0, sizeof(C4CHECK) ) ;

   if ( f4flag_init( &check->flag, cb, n_recs ) < 0 )
      return -1 ;

   check->code_base = cb ;
   check->tag = t4 ;
   check->num_recs = n_recs ;

   check->old_key = (char *)u4alloc_free( t4->code_base, t4->header.key_len ) ;
   if (check->old_key == 0)
      return -1 ;
   return 0 ;
}

static void c4check_free( C4CHECK *c4 )
{
   u4free( c4->flag.flags ) ;
   u4free( c4->old_key ) ;
}

static int  c4check_record( C4CHECK *check )
{
   B4KEY_DATA *key_data ;
   TAG4 *t4 ;
   char *new_ptr ;
   int len, rc ;

   t4 = check->tag ;

   key_data = t4key_data( check->tag ) ;
   if ( key_data == 0 )
      return e4( check->code_base, e4result, 0 ) ;

   if ( key_data->num < 1  ||  key_data->num > check->num_recs )
      return e4describe( check->code_base, e4info, E4_INFO_INC, check->tag->alias, (char *)0 ) ;

   if ( f4flag_is_set( &check->flag, key_data->num) )
      return e4describe( check->code_base, e4info, E4_INFO_REP, check->tag->alias, (char *)0 ) ;
   else
      f4flag_set( &check->flag, key_data->num ) ;

   if ( d4go( t4->index->data, key_data->num ) < 0)  return( -1) ;
   len = t4expr_key( t4, &new_ptr ) ;

   if ( len != t4->header.key_len )
      return e4describe( check->code_base, e4result, E4_RESULT_UNE, t4->alias, (char *)0 ) ;

   #ifdef S4MDX
      if ( expr4type( t4->expr ) == r4num )
      {
         if ( c4bcd_cmp( new_ptr, key_data->value, 0 ) != 0 )
            return e4describe( check->code_base, e4result, E4_RESULT_TAG, t4->alias, (char *)0 ) ;
      }
      else
   #endif
   if ( u4memcmp( new_ptr, key_data->value, t4->header.key_len ) != 0 )
      return e4describe( check->code_base, e4result, E4_RESULT_TAG, t4->alias, (char *)0 ) ;

   if ( check->do_compare )
   {
      #ifdef S4FOX
         rc = u4memcmp( check->old_key, new_ptr, check->tag->header.key_len ) ;
      #else
         rc = (*t4->cmp)( check->old_key, new_ptr, check->tag->header.key_len ) ;
      #endif

      if ( rc > 0)
         e4describe( check->code_base, e4result, E4_RESULT_THE, t4->alias, (char *)0 ) ;
      #ifdef S4FOX
         if ( rc == 0  &&  key_data->num <= check->old_rec )
            e4describe( check->code_base, e4result, E4_RESULT_REC, t4->alias, (char *)0 ) ;
      #endif /* S4FOX */

      #ifdef S4FOX
         if ( t4->header.type_code & 0x01 )
      #else
         if ( t4->header.unique )
      #endif /* S4FOX */
            if ( rc == 0 )
               e4describe( check->code_base, e4result, E4_RESULT_IDE, t4->alias, (char *)0 ) ;
   }
   else
      check->do_compare = 1 ;

   memcpy( check->old_key, new_ptr, t4->header.key_len ) ;

   check->old_rec = key_data->num ;

   if ( check->code_base->error_code < 0 )
      return -1 ;
   return 0 ;
}

#ifdef S4CLIPPER
static int t4block_check( TAG4 *t4, int first_time )
{
   B4BLOCK *b4 ;
   int i, b_type, rc ;
   CODE4 *c4 ;

   if ( first_time )
      t4up_to_root( t4 ) ;

   c4 = t4->code_base ;
   b4 = (B4BLOCK *)t4->blocks.last_node ;
   if ( b4 == 0 )
      return 0 ;
   if ( b4->n_keys < t4->header.keys_half && t4->header.root / 512 != b4->file_block )
      return e4describe( c4, e4result, E4_T4BLOCK_CHK, E4_RESULT_CII, (char *)0 ) ;
   if ( !b4leaf( b4 ) )
   {
      for ( i = 0 ; i <= b4->n_keys ; i++ )
      {
         b4->key_on = i ;
         rc = t4down( t4 ) ;
         if ( rc != 0 )
            return e4describe( c4, e4result, E4_T4BLOCK_CHK, E4_INFO_CIF, (char *)0 ) ;
         if ( i == 0 )
            b_type = b4leaf( (B4BLOCK *)t4->blocks.last_node ) ;
         else
            if ( b_type != b4leaf( (B4BLOCK *)t4->blocks.last_node ) )
               return e4describe( c4, e4result, E4_T4BLOCK_CHK, E4_RESULT_CII, (char *)0 ) ;
         rc = t4block_check( t4, 0 ) ;
         if ( rc != 0 )
            return rc ;
         rc = t4up( t4 ) ;
         if ( rc != 0 )
            return e4describe( c4, e4result, E4_T4BLOCK_CHK, E4_INFO_CIF, (char *)0 ) ;
      }
   }
   return 0 ;
}
#endif

int S4FUNCTION t4check( TAG4 *t4 )
{
   C4CHECK check ;
   int rc, is_record, keys_skip ;
   INDEX4 *i4 ;
   DATA4 *d4 ;
   CODE4 *c4 ;
   TAG4 *old_selected_tag ;
   B4BLOCK *block_on ;
   long base_size, on_rec ;
   char *ptr ;
   #ifdef S4FOX
      char *temp_val ;
      long temp_lng ;
   #endif
   #ifndef S4CLIPPER
      B4KEY_DATA *key_branch, *key_leaf ;
   #endif
   #ifdef S4PRINTF_OUT
      unsigned long loop ;
   #endif

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4CHECK ) ;
   #endif

   i4 = t4->index ;
   d4 = i4->data ;
   c4 = t4->code_base ;

   #ifndef S4SINGLE
      rc = d4lock_file( d4 ) ;
      if ( rc != 0 )
         return rc ;
      rc = i4lock( t4->index ) ;
      if ( rc != 0 )
         return rc ;
      rc = d4refresh( d4 ) ;
      if ( rc != 0 )
         return rc ;
   #endif

   rc = d4update_record( d4, 1 ) ;
   if ( rc < 0 )
      return -1 ;
   if ( rc )
      return e4describe( c4, e4result, E4_T4CHECK, E4_RESULT_D4F, (char *)0 ) ;

   old_selected_tag = d4tag_selected( d4 ) ;
   d4tag_select( d4, t4 ) ;

   #ifdef S4CLIPPER
      rc = t4block_check( t4, 1 ) ;
      if ( rc != 0 )
         return rc ;
   #endif

   base_size = d4reccount( d4 ) ;
   if ( base_size < 0L )
      return -1 ;

   rc = d4top( d4 ) ;
   if (rc < 0 )
      return -1 ;
   if (rc == 0)
      rc = 1 ;

   if ( base_size == 0L )
   {
      if ( t4skip( t4, 1L ) == 0 )
      {
         d4tag_select( d4, old_selected_tag ) ;
         return( 0 ) ;
      }
      else
         return e4describe( c4, e4info, E4_T4CHECK, E4_INFO_DAT, (char *)0 ) ;
   }

   if ( c4check_init( &check, c4, t4, base_size ) < 0 )
      return -1 ;

   #ifdef S4PRINTF_OUT
      loop = 0 ;
      printf( "On Rec %10ld\n", loop ) ;
   #endif
   while ( rc == 1 )
   {
      rc = c4check_record( &check ) ;
      if ( rc )
         break ;
      rc = (int)t4skip( t4, 1L ) ;
      if ( rc < 0 )
         break ;
      #ifdef S4PRINTF_OUT
         if ( (loop++ % 100) == 0 )
            printf( "\b\b\b\b\b\b\b\b\b\b%10ld", loop ) ;
      #endif
   }

   if ( rc < 0 )
   {
      c4check_free( &check ) ;
      return -1 ;
   }

   is_record = 1 ;

   /* Now Test for Duplication */
   for ( on_rec = 1;  on_rec <= base_size; on_rec++)
   {
      #ifndef S4NDX
         if ( t4->filter != 0 )
         {
            if ( d4go(d4,on_rec) < 0 )  break ;
            is_record = expr4true( t4->filter ) ;
         }
      #endif

      if ( f4flag_is_set(&check.flag,on_rec) )
      {
         if ( ! is_record )
         {
            e4describe( c4, e4info, E4_T4CHECK, t4->alias, (char *)0 ) ;
            break ;
         }
      }
      else
      {
         if ( ! is_record )
            continue ;

         #ifdef S4FOX
            if ( t4->header.type_code & 0x01 )
         #else
            if ( t4->header.unique )
         #endif
            {
               if ( d4go(d4,on_rec) < 0 )
                  break ;
               if ( t4expr_key( t4, &ptr) < 0 )
                  break ;
               if ( d4seek( d4, ptr ) == 0 )
                  continue ;
            }

         e4describe( c4, e4info, E4_T4CHECK, E4_INFO_REC, t4->alias ) ;
         break ;
      }
   }

   c4check_free( &check ) ;
   if ( t4->code_base->error_code < 0 )
      return -1 ;

   /* Now make sure the block key pointers match the blocks they point to. */
   /* This needs to be true for d4seek to function perfectly. */

   rc = d4bottom( d4 ) ;
   if ( rc < 0 )
      return -1 ;

   if ( rc == 3 )
   {
      d4tag_select( d4, old_selected_tag ) ;
      return 0 ;
   }

   for(;;)
   {
      #ifdef S4FOX
         keys_skip = -t4block(t4)->header.n_keys ;
      #else
         keys_skip = -t4block(t4)->n_keys ;
      #endif

      rc = (int)t4skip( t4, (long) keys_skip ) ;
      if ( c4->error_code < 0 )
         return -1 ;
      if ( rc != keys_skip )
      {
         d4tag_select( d4, old_selected_tag ) ;
         return 0 ;
      }

      block_on = (B4BLOCK *)t4->blocks.last_node ;
      if ( block_on == 0 )
         return e4describe( c4, e4info, E4_INFO_TAG, t4->alias, (char *)0 ) ;

      #ifdef S4FOX
         temp_val = (char *)u4alloc_free( c4, t4->header.key_len ) ;
         if ( temp_val == 0 )
            e4( t4->code_base, e4memory, 0 ) ;
         memcpy( temp_val, b4key_key( block_on, block_on->key_on ), t4->header.key_len ) ;
         temp_lng = b4recno( block_on, block_on->key_on ) ;

         if ( t4go( t4, temp_val, temp_lng ) )
         {
            u4free( temp_val ) ;
            return e4describe( c4, e4info, E4_INFO_TAG, t4->alias, (char *)0 ) ;
         }
         u4free( temp_val ) ;
      #endif

      #ifndef S4CLIPPER
         for ( ;; )
         {
            block_on = (B4BLOCK *)block_on->link.p ;
            if ( block_on == 0 )
               break ;
            if ( block_on == (B4BLOCK *)t4->blocks.last_node )
               break ;

            #ifdef S4FOX
               if ( block_on->key_on < block_on->header.n_keys )
            #else
               if ( block_on->key_on < block_on->n_keys )
            #endif
               {
                  key_branch = b4key( block_on, block_on->key_on ) ;
                  key_leaf = b4key( t4block(t4), t4block(t4)->key_on ) ;

                  if ( u4memcmp( key_branch->value, key_leaf->value, t4->header.key_len) != 0)
                     return e4describe( c4, e4info, E4_INFO_TAG, t4->alias, (char *)0 ) ;

                  break ;
               }
         }
         if ( block_on == 0 )
            return e4describe( c4, e4info, E4_INFO_TAG, t4->alias, (char *)0 ) ;
      #endif
   }
}

#ifndef S4NDX
static int flag_blocks( TAG4 *t4, F4FLAG *f4 )
{
   int i, rc ;
   B4BLOCK *block_on ;
   long flag_no ;

   rc = t4down( t4 ) ;
   if ( rc < 0 || rc == 2 )
      return -1 ;
   if ( rc == 1 )
      e4severe( e4result, (char *)0 ) ;

   block_on = t4block(t4) ;

   #ifdef S4FOX
      flag_no = block_on->file_block / B4BLOCK_SIZE ;
   #else
      #ifdef S4CLIPPER
         flag_no = (block_on->file_block) * I4MULTIPLY / B4BLOCK_SIZE ;
      #else
         #ifdef S4NDX
            flag_no = (block_on->file_block-4) * I4MULTIPLY / B4BLOCK_SIZE ;
         #else
            flag_no = (block_on->file_block-4) * I4MULTIPLY / t4->index->header.block_rw ;
         #endif
      #endif
   #endif

   if ( f4flag_is_set( f4, flag_no ) )
      e4severe( e4result, E4_RESULT_COR ) ;

   if ( f4flag_set( f4, flag_no ) < 0 )
      return -1 ;
   if ( ! b4leaf(block_on) )

      #ifdef S4MDX
         for( i = 0; i <= block_on->n_keys; i++ )
         {
            block_on->key_on = i ;
            if ( flag_blocks( t4, f4 ) < 0 )
               return -1 ;
         }
      #else
         #ifdef S4CLIPPER
            for( i = 0; i <= block_on->n_keys; i++ )
            {
               block_on->key_on = i ;
               if ( flag_blocks( t4, f4 ) < 0 )
                  return -1 ;
            }
         #else
            for( i = 0; i < block_on->header.n_keys; i++ )
            {
               b4go( block_on, i ) ;
               if ( flag_blocks( t4, f4 ) < 0 )
                  return -1 ;
            }
         #endif
      #endif

   t4up(t4) ;
   return 0 ;
}
#endif   /*  ifndef S4NDX  */

/* checks that all blocks in the file are on free list or are being used */
static int i4check_blocks( INDEX4 *i4 )
{
   #ifndef N4OTHER
      TAG4  *tag_on ;
      F4FLAG  flags ;
      int     i, flag_no ;
      long    tot_blocks, free_block, eof_block_no, len ;
      CODE4 *c4 ;
      #ifndef S4FOX
         T4DESC  desc[48] ;
      #endif

      c4 = i4->code_base ;

      #ifndef S4SINGLE
         if ( i4lock( i4 ) < 0 )
            return -1 ;
      #endif

      len = file4len(&i4->file) ;

      #ifdef S4MDX
         tot_blocks = (len-2048) / i4->header.block_rw ;
      #else
         tot_blocks = len / B4BLOCK_SIZE ;
      #endif

      /* First Flag for the Free Chain */
      f4flag_init( &flags, i4->code_base, tot_blocks ) ;

      eof_block_no = len/I4MULTIPLY ;
      #ifdef S4FOX
         for ( free_block = i4->tag_index->header.free_list ; free_block ; )
      #else
         for ( free_block = i4->header.free_list ; free_block ; )
      #endif
         {
            if ( free_block == eof_block_no  ||  c4->error_code < 0 )
               break ;

            #ifdef S4MDX
               flag_no = (int)((free_block-4)*I4MULTIPLY/i4->header.block_rw) ;
            #else
               flag_no = (int) (free_block / B4BLOCK_SIZE) ;
            #endif

            if ( free_block >= eof_block_no  || f4flag_is_set(&flags, flag_no) )
            {
               e4( c4, e4index, E4_INDEX_COR ) ;
               break ;
            }
            f4flag_set( &flags, flag_no ) ;

            #ifdef S4MDX
               file4read_all( &i4->file, free_block * I4MULTIPLY + sizeof(long), &free_block, sizeof(free_block) ) ;
            #else
               file4read_all( &i4->file, free_block * I4MULTIPLY, &free_block, sizeof(free_block) ) ;
            #endif

            #ifdef S4BYTE_SWAP
               free_block = x4reverse_long( free_block ) ;
            #endif
         }

      #ifdef S4FOX
         /* do the header tag */
         tag_on = i4->tag_index ;
         flag_no = (int)((tag_on->header_offset) / (long)B4BLOCK_SIZE) ;
         if ( f4flag_is_set( &flags, flag_no ) )
            e4severe( e4result, E4_RESULT_COR ) ;
         f4flag_set( &flags, flag_no ) ;
         f4flag_set( &flags, flag_no + 1L ) ;  /* tag header is 2 blocks long */

         if ( t4free_all( tag_on ) >= 0 )
         {
            flag_blocks( tag_on, &flags ) ;

            /* Now Flag for each block in each tag */
            i = 1 ;
            for ( tag_on = 0 ;; i++ )
            {
               tag_on = (TAG4 *)l4next( &i4->tags,tag_on ) ;
               if ( tag_on == 0 )
                  break ;
               flag_no = (int)( tag_on->header_offset / (long)B4BLOCK_SIZE) ;
               if ( f4flag_is_set( &flags, flag_no ) )
                  e4severe( e4result, E4_RESULT_COR ) ;
               f4flag_set( &flags, flag_no ) ;
               f4flag_set( &flags, flag_no + 1L ) ;  /* tag header is 2 blocks long */

               if ( t4free_all( tag_on ) < 0 )
                  break ;
               flag_blocks( tag_on, &flags ) ;
            }
         }
      #else
         /* Read header information to flag the tag header blocks */
         file4read_all( &i4->file, 512, desc, sizeof(desc) ) ;

         /* Now Flag for each block in each tag */
         i = 1 ;
         for ( tag_on = 0 ;; i++ )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            #ifdef S4BYTE_SWAP
               desc[i].header_pos = x4reverse_long( desc[i].header_pos ) ;
               desc[i].x1000 = 0x1000 ;
            #endif

            flag_no = (int) ((desc[i].header_pos * I4MULTIPLY - 2048) / (long) i4->header.block_rw) ;
            if ( f4flag_is_set( &flags, flag_no ) )
               e4severe( e4result, E4_RESULT_COR ) ;
            f4flag_set( &flags, flag_no ) ;

            if ( t4free_all(tag_on) < 0 )
               break ;
            flag_blocks( tag_on, &flags ) ;
         }
      #endif

      if ( f4flag_is_all_set( &flags, 0, tot_blocks - 1 ) == 0 )
         e4( i4->code_base, e4result, E4_RESULT_LOS ) ;

      u4free( flags.flags ) ;
      if ( i4->code_base->error_code < 0 )
          return -1 ;

   #endif
   return 0 ;
}

int S4FUNCTION i4check( INDEX4 *i4 )
{
   TAG4 *tag_on ;
   #ifdef S4HAS_DESCENDING
      int old_desc, rc ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4CHECK ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   if ( i4update( i4 ) < 0 )
      return -1 ;
   if ( i4check_blocks(i4) < 0 )
      return -1 ;

   for( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on == 0 )
         return 0 ;
      #ifdef S4HAS_DESCENDING
         old_desc = tag_on->header.descending ;
         tag_on->header.descending = 0 ;   /* force ascending */
         rc = t4check( tag_on ) ;
         tag_on->header.descending = old_desc ;   /* return to previous */
         if ( rc < 0 ) return rc ;
      #else
         if ( t4check(tag_on) < 0 )
            return -1 ;
      #endif
   }
}

#endif  /* S4INDEX_OFF */

int S4FUNCTION d4check( DATA4 *d4 )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      INDEX4 *index_on ;
      int rc ;

      #ifdef S4DEBUG
         if ( d4 == 0 )
            e4severe( e4parm, E4_D4CHECK ) ;
      #endif

      #ifndef S4SINGLE
         rc = d4lock_file( d4 ) ;   /* returns -1 if code_base->error_code < 0 */
         if ( rc )
            return rc ;
      #endif

      for( index_on = 0 ;; )
      {
         index_on = (INDEX4 *)l4next( &d4->indexes, index_on ) ;
         if ( index_on == 0 )
            return 0 ;
         if ( i4check( index_on ) < 0 )
            return -1 ;
      }
   #endif  /* S4INDEX_OFF */
}
