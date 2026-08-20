/* o4opt.c   (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

#ifndef S4OPTIMIZE_OFF

static void       opt4block_add( OPT4BLOCK *, FILE4 *, unsigned, long, long ) ;
static int        opt4block_flush( OPT4BLOCK *, char ) ;

static OPT4BLOCK *opt4file_choose_block( FILE4 * ) ;
static OPT4BLOCK *opt4file_get_block( FILE4 * ) ;
static void       opt4file_lru_bottom( FILE4 *, LINK4 *, char ) ;
static unsigned   opt4file_read_file( FILE4 *, long, char * ) ;
static void       opt4file_read_sp_buffer( FILE4 *, unsigned long ) ;

static void opt4block_add( OPT4BLOCK *block, FILE4 *file, unsigned block_len, long hash_val, long position )
{
   #ifdef S4DEBUG
      if ( file == 0 || block == 0 || hash_val < 0 || position < 0 )
         e4severe( e4parm, E4_OPT4BLOCK_ADD ) ;
      if ( block->changed != 0 )
         e4severe( e4info, E4_INFO_UNI ) ;
   #endif

   l4add( &file->code_base->opt.lists[hash_val], block ) ;
   block->len = block_len ;
   block->pos = position ;
   block->file = file ;
}

/* also sets the priority scheme */
static OPT4BLOCK *opt4file_choose_block( FILE4 *file )
{
   LINK4 *lru_link ;
   LIST4 *choose_list ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 )
         e4severe( e4parm, E4_OPT4FILE_CH ) ;
   #endif

   opt = &file->code_base->opt ;

   if ( opt->avail.n_link )
      choose_list = &opt->avail ;
   else
   {
      #ifndef S4DETECT_OFF
         if ( ++opt->check_count > OPT4CHECK_RATE )
            d4update_prio( file->code_base ) ;
      #endif

      for(;;)
      {
         choose_list = opt->prio[2] ; 
         if ( opt->prio[2]->n_link <= opt->min_link )
         {
            choose_list = opt->prio[1] ;
            if ( opt->prio[1]->n_link <= opt->min_link )
               choose_list = opt->prio[0] ;
         }

         #ifdef S4DEBUG
            if ( choose_list->n_link == 0 || choose_list == 0 || opt->min_link == 0 )
                 
               e4severe( e4info, E4_OPT4FILE_CH ) ;
         #endif
         if ( choose_list->n_link != 0 )
            break ;
         if ( opt->min_link == 0 )
            return 0 ;  /* actually means a severe error has occurred */
         opt->min_link-- ;  /* no lists have more than minimum, so reduce the minimum */
      }
   }

   lru_link = (LINK4 *)l4first( choose_list ) ;
   l4remove( choose_list, lru_link ) ;
   opt4file_lru_bottom( file, lru_link, 0 ) ;
   return (OPT4BLOCK *)( lru_link - 1 ) ;
}

void S4FUNCTION opt4block_clear( OPT4BLOCK *block )
{
   #ifdef S4DEBUG
      if ( block == 0 )
         e4severe( e4parm, E4_OPT4BLOCK_CLR ) ;
   #endif

   block->changed = 0 ;
   block->len = 0 ;
   block->pos = 0 ;
   block->file = 0 ;
}

static int opt4block_flush( OPT4BLOCK *block, char buffer )
{
   OPT4 *opt ;
   int rc ;

   #ifdef S4DEBUG
      if ( block == 0 )
         e4severe( e4parm, E4_OPT4BLOCK_FLSH ) ;
   #endif

   opt = &block->file->code_base->opt ;

   if ( buffer )
   {
      if ( opt->write_cur_pos != (unsigned long)block->pos || opt->write_file != block->file || opt->write_block_count == opt->max_blocks )
      {
         if ( ( rc = opt4flush_write_buffer( opt ) ) != 0 )
            return rc ;
         opt->write_file = block->file ;
         opt->write_start_pos = opt->write_cur_pos = block->pos ;
      }
      memcpy( opt->write_buffer + (opt->write_cur_pos - opt->write_start_pos), block->data, block->len ) ;
      opt->write_cur_pos += block->len ;
      opt->write_block_count++ ;
   }
   else
   {
      if ( opt->write_file == block->file )
         if ( opt->write_cur_pos >= (unsigned long) block->pos &&
              (opt->write_cur_pos - opt->write_block_count * opt->block_size ) <= (unsigned long)block->pos )
            if ( ( rc = opt4flush_write_buffer( opt ) ) != 0 )
               return rc ;
      block->file->do_buffer = 0 ;
      if ( ( rc = file4write( block->file, block->pos, block->data, block->len ) ) != 0 )
         return rc ;
      block->file->do_buffer = 1 ;
   }

   block->changed = 0 ;

   return 0 ;
}

void S4FUNCTION opt4block_remove( OPT4BLOCK *block, int do_flush )
{
   #ifdef S4DEBUG
      if ( block == 0 )
         e4severe( e4parm, E4_OPT4BLOCK_REM ) ;
   #endif

   if ( block->file != 0 )
      l4remove( &block->file->code_base->opt.lists[opt4file_hash( block->file, block->pos )], block ) ;

   if ( do_flush && block->changed )
   {
      block->file->do_buffer = 0 ;
      opt4block_flush( block, 1 ) ;
      block->file->do_buffer = 1 ;
   }
   opt4block_clear( block ) ;
}

/* function to remove, without saving, parts of an optimized file */
void S4FUNCTION opt4file_delete( FILE4 *file, long low_pos, long hi_pos )
{
   long on_pos, end_delete_pos, hash_val ;
   OPT4BLOCK *block_on ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 || low_pos < 0 || hi_pos < 0 )
         e4severe( e4parm, E4_OPT4FILE_DEL ) ;
   #endif

   opt = &file->code_base->opt ;

   hash_val = opt4file_hash( file, low_pos ) ;
   if ( hash_val != opt4file_hash( file, low_pos + file->code_base->opt.block_size - 1L ) )  /* not on a block boundary, so delete partial */
   {
      block_on = opt4file_return_block( file, low_pos, hash_val ) ;
      if ( block_on )  /* block in memory, so partially delete */
      {
         if ( file->len <= hi_pos )   /* just removing all file, so merely delete */
            block_on->len = (unsigned)(low_pos - block_on->pos) ;
         else  /* read the old data into the block - ignore errors since could be eof, else catch later */
            file4read( file, low_pos, (char *)block_on->data + block_on->pos - low_pos ,
                  (unsigned)(opt->block_size - (low_pos - block_on->pos)) ) ;
      }
      on_pos = ( (low_pos + opt->block_size) >> opt->block_power ) << opt->block_power ;
   }
   else
      on_pos = low_pos ;

   end_delete_pos = hi_pos + opt->block_size - 1 ;
   while( on_pos < end_delete_pos )
   {
      block_on = opt4file_return_block( file, on_pos, opt4file_hash( file, on_pos ) ) ;
      if ( block_on )  /* block in memory, so delete */
      {
         opt4block_remove( block_on, 0 ) ;
         opt4file_lru_top( file, &block_on->lru_link, 0 ) ;
         l4add_before( &opt->avail, l4first( &opt->avail ), &block_on->lru_link ) ;
      }
      on_pos += opt->block_size ;
   }

   on_pos -= opt->block_size ;
   if ( on_pos < hi_pos )
   {
      block_on = opt4file_return_block( file, on_pos, opt4file_hash( file, on_pos ) ) ;
      if ( block_on )
      {
         if ( file->len <= hi_pos )
         {
            opt4block_remove( block_on, 0 ) ;
            opt4file_lru_top( file, &block_on->lru_link, 0 ) ;
            l4add_before( &opt->avail, l4first( &opt->avail ), &block_on->lru_link ) ;
         }
         else
            file4read( file, on_pos, block_on->data, (unsigned)(hi_pos - block_on->pos) ) ;
      }
   }
}

LIST4 *S4FUNCTION opt4file_find_list( FILE4 *file )
{
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 )
         e4severe( e4parm, E4_OPT4FILE_FL ) ;
   #endif

   opt = &file->code_base->opt ;

   switch( (int)file->type )
   {
      case OPT4AVAIL:
         return &opt->avail ;
      case OPT4DBF:
         return &opt->dbf_lru ;
      case OPT4INDEX:
         return &opt->index_lru ;
      case OPT4OTHER:
         return &opt->other_lru ;
      default:
         #ifdef S4DEBUG
            e4severe( e4parm, E4_OPT4FILE_FL ) ;
         #endif
         return (LIST4 *) 0 ;
   }
}

int S4FUNCTION opt4flush_all( OPT4 *opt, char do_free )
{
   OPT4BLOCK *block_on ;
   LINK4 *link_on, *next_link ;
   LIST4 *flush_list ;
   char i ;
   int rc, save_rc ;

   #ifdef S4DEBUG
      if ( opt == 0 )
         e4severe( e4parm, E4_OPT4FLUSH_ALL ) ;
   #endif

   save_rc = opt4flush_write_buffer( opt ) ;

   for ( i = 0 ; i < 3 ; i++ )
   {
      flush_list = opt->prio[i] ;

      for( link_on = (LINK4 *)l4first( flush_list ) ; link_on != 0; )
      {
         block_on = (OPT4BLOCK *)( link_on - 1 ) ;
         if( block_on->changed )
         {
            rc = opt4block_flush( block_on, 1 ) ;
            if ( rc != 0 )
               save_rc = rc ;
         }

         if ( do_free )
         {
            next_link = (LINK4 *)l4next( flush_list, link_on ) ;
            l4remove( &opt->lists[ opt4file_hash( block_on->file, block_on->pos )], block_on ) ;
            opt4file_lru_top( block_on->file, link_on, 0 ) ;
            l4add( &opt->avail, link_on ) ;
            link_on = next_link ;
            opt4block_clear( block_on ) ;
         }
         else
            link_on = (LINK4 *)l4next( flush_list, link_on ) ;
      }
   }
   return save_rc ;
}

int S4FUNCTION opt4file_flush_list( FILE4 *file, LIST4 *flush_list, int do_free )
{
   OPT4BLOCK *block_on ;
   LINK4 *link_on, *next_link ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 || flush_list == 0 )
         e4severe( e4parm, E4_OPT4FLUSH_LST ) ;
   #endif

   opt = &file->code_base->opt ;

   for( link_on = (LINK4 *)l4first( flush_list ) ; link_on != 0; )
   {
      block_on = (OPT4BLOCK *)( link_on - 1 ) ;
      if( block_on->file == file )
      {
         if( block_on->changed )
            if ( opt4block_flush( block_on, 1 ) < 0 )
               return -1 ;

         if ( do_free )
         {
            next_link = (LINK4 *)l4next( flush_list, link_on ) ;
            l4remove( &opt->lists[ opt4file_hash( file, block_on->pos )], block_on ) ;
            opt4file_lru_top( file, link_on, 0 ) ;
            l4add( &opt->avail, link_on ) ;
            link_on = next_link ;
            opt4block_clear( block_on ) ;
         }
         else
            link_on = (LINK4 *)l4next( flush_list, link_on ) ;
      }
      else
         link_on = (LINK4 *)l4next( flush_list, link_on ) ;
   }
   return 0 ;
}

int S4FUNCTION opt4flush_write_buffer( OPT4 *opt )
{
   int rc ;

   #ifdef S4DEBUG
      if ( opt == 0 )
         e4severe( e4parm, E4_OPT4FLUSH_WB ) ;
   #endif

   if ( opt->write_block_count != 0 )
   {
      opt->write_file->do_buffer = 0 ;
      opt->write_file->buffer_writes = 0 ;
      rc = file4write( opt->write_file, opt->write_start_pos, opt->write_buffer, 
                    (unsigned)(opt->write_cur_pos - opt->write_start_pos) ) ;
      opt->write_file->do_buffer = 1 ;
      opt->write_file->buffer_writes = 1 ;
      if ( rc != 0 )
         return rc ;
      opt->write_start_pos = opt->write_cur_pos = opt->write_block_count = 0 ;
   }

   return 0 ;
}

static OPT4BLOCK *opt4file_get_block( FILE4 *file )
{
   OPT4BLOCK *block ;

   #ifdef S4DEBUG
      if ( file == 0 )
         e4severe( e4parm, E4_OPT4FILE_GB ) ;
   #endif

   block = opt4file_choose_block( file ) ;
   opt4block_remove( block, 1 ) ;
   return block ;
}

long S4FUNCTION opt4file_hash( FILE4 *file, unsigned long pos )
{
   return ( (( file->hash_init + pos ) >> file->code_base->opt.block_power ) & file->code_base->opt.mask ) ;
}

int S4FUNCTION opt4file_write( FILE4 *file, long pos, unsigned len, void *data, char changed )
{
   long hash_val, adjusted_pos, len_written ;
   unsigned read_len, len_read, extra_read ;
   OPT4BLOCK *block_on ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 || pos < 0 || data == 0 )
         e4severe( e4parm, E4_OPT4FILE_WR ) ;
   #endif

   opt = &file->code_base->opt ;
   len_written = 0 ;
   extra_read = (unsigned) ((unsigned long)((unsigned long)pos << opt->num_shift ) >> opt->num_shift ) ;
   adjusted_pos = pos - extra_read ;
   len += extra_read ;
   if( (long)len > (long)( (long)opt->num_blocks * (long)opt->block_size ) )
   {
      /* case where amount to write > total bufferred length - do a piece at a time */
      adjusted_pos = (long)( (long)opt->num_blocks * (long)opt->block_size ) ;
      for ( len_written = 0 ; len > len_written ; len_written += adjusted_pos )
      {
         if ( ( len - len_written ) < adjusted_pos )
            adjusted_pos = len - len_written ;
         if ( opt4file_write( file, pos + len_written, adjusted_pos, (char *)data + len_written, changed ) != adjusted_pos )
            return len_written ;
      }
      return len ;
   }

   do
   {
      hash_val = opt4file_hash( file, adjusted_pos ) ;
      read_len = (unsigned) ((len / (unsigned)opt->block_size) ? opt->block_size : len) ;
      block_on = opt4file_return_block( file, adjusted_pos, hash_val ) ;
      if ( block_on == 0 )
      {
         block_on = opt4file_get_block( file ) ;
         if ( (unsigned long)( read_len - extra_read ) < opt->block_size && adjusted_pos < file4len( file ) )
            len_read = opt4file_read_file( file, adjusted_pos, (char *)block_on->data ) ;
         else
         {
            if ( file4len( file ) >= pos + opt->block_size )   /* mark block as full to avoid file len set below... */
               len_read = opt->block_size ;
            else
               len_read = 0 ;
         }
         opt4block_add( block_on, file, len_read, hash_val, adjusted_pos ) ;
      }

      opt4file_lru_bottom( file, &block_on->lru_link, 1 ) ;
      block_on->changed |= changed ;

      if ( read_len < (unsigned) extra_read )
         read_len = (unsigned) extra_read ;

      memcpy( (char *)block_on->data + extra_read, (char *)data + len_written, (unsigned)(read_len - extra_read) ) ;
      len -= read_len ;
      len_written += read_len - extra_read ;
      extra_read = 0 ;
      adjusted_pos += opt->block_size ;
      if ( pos + len_written > block_on->pos + block_on->len )
      {
         if (block_on->file->len < ( pos + len_written ) ) /* file size has grown */
         {
            block_on->len = (unsigned)( pos - block_on->pos + len_written ) ;
            if (  block_on->file->buffer_writes == 1 )
            {
               block_on->file->len = pos + len_written ;
               #ifdef S4DEBUG
                  /* make sure the file length really has grown */
                  if ( filelength( file->hand ) > block_on->file->len )
                     e4severe( e4info, E4_OPT4FILE_WR ) ;
               #endif
            }
         }
         else   /* we have written past our end of block, but not our block, update our block len, dummy info */
            block_on->len = (unsigned) opt->block_size ;
      }
   } while( len && block_on->len == (unsigned)opt->block_size ) ;

   return (unsigned) len_written ;
}

unsigned S4FUNCTION opt4file_read( FILE4 *file, long pos, void *data, unsigned len )
{
   long hash_val, adjusted_pos, len_read ;
   unsigned read_len, extra_read ;
   OPT4BLOCK *block_on ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 || pos < 0 || data == 0 )
         e4severe( e4parm, E4_OPT4FILE_RD ) ;
   #endif

   opt = &file->code_base->opt ;
   len_read = 0 ;
   extra_read = (unsigned)((unsigned long)((unsigned long)pos << opt->num_shift ) >> opt->num_shift ) ;
   adjusted_pos = pos - extra_read ;
   len += extra_read ;
   if( (unsigned long)len > ( opt->num_blocks * opt->block_size ))
   {
      /* case where amount to read > total bufferred length - do a piece at a time */
      adjusted_pos = (long)( (long)opt->num_blocks * (long)opt->block_size ) ;
      for ( len_read = 0 ; len > len_read ; len_read += adjusted_pos )
      {
         if ( ( len - len_read ) < adjusted_pos )
            adjusted_pos = len - len_read ;
         if ( opt4file_read( file, pos + len_read, (char *)data + len_read, adjusted_pos ) != adjusted_pos )
            return len_read ;
      }
      return len ;
   }

   do
   {
      hash_val = opt4file_hash( file, adjusted_pos ) ;
      read_len = (unsigned) ((len / opt->block_size) ? opt->block_size : len ) ;
      block_on = opt4file_return_block( file, adjusted_pos, hash_val ) ;
      if ( block_on == 0 )  /* read from disk */
      {
         if ( opt->is_skip != 0 )
         {
            if ( opt->is_skip == 1 && file->type == OPT4DBF || opt->is_skip > 1 && file->type == OPT4INDEX )
            {
               opt4file_read_sp_buffer( file, adjusted_pos ) ;
               block_on = opt4file_return_block( file, adjusted_pos, hash_val ) ;
            }
            else
               block_on = 0 ;

            if ( block_on == 0 )
            {
               block_on = opt4file_get_block( file ) ;
               opt4block_add( block_on, file, opt4file_read_file( file, adjusted_pos, (char *)block_on->data ), hash_val, adjusted_pos ) ;
            }
         }
         else
         {
            block_on = opt4file_get_block( file ) ;
            opt4block_add( block_on, file, opt4file_read_file( file, adjusted_pos, (char *)block_on->data ), hash_val, adjusted_pos ) ;
         }
      }
      else
         if ( opt->force_current == 1 )
         {
            if ( block_on->changed == 0 && file->is_exclusive == 0 &&
                 file->is_read_only == 0 && file->buffer_writes == 0 )
               opt4file_read_file( file, adjusted_pos, (char *)block_on->data ) ;
         }

      opt4file_lru_bottom( file, &block_on->lru_link, 1 ) ;

      if ( block_on->len < read_len )
         read_len = block_on->len ;
      if ( read_len < (unsigned) extra_read )
         read_len = (unsigned) extra_read ;
      memcpy( (char *)data + len_read, (char *)block_on->data + extra_read, (unsigned)(read_len - extra_read) ) ;
      len -= read_len ;
      len_read += read_len - extra_read ;
      extra_read = 0 ;
      adjusted_pos += opt->block_size ;
   } while( len && block_on->len == (unsigned) opt->block_size ) ;
   return (unsigned) len_read ;
}

static void opt4file_lru_bottom( FILE4 *file, LINK4 *l4link, char del )
{
   LIST4 *lru_list ;

   #ifdef S4DEBUG
      if ( file == 0 || l4link == 0 )
         e4severe( e4parm, E4_OPT4FILE_LRUB ) ;
   #endif

   lru_list = opt4file_find_list( file ) ;

   if ( del )
   {
      if ( l4link != lru_list->last_node )
      {
         l4remove( lru_list, l4link ) ;
         l4add( lru_list, l4link ) ;
      }
   }
   else
      l4add( lru_list, l4link ) ;
}

void S4FUNCTION opt4file_lru_top( FILE4 *file, LINK4 *l4link, char add )
{
   LIST4 *lru_list ;

   #ifdef S4DEBUG
      if ( file == 0 || l4link == 0 )
         e4severe( e4parm, E4_OPT4FILE_LRUT ) ;
   #endif

   lru_list = opt4file_find_list( file ) ;

   l4remove( lru_list, l4link ) ;
   if ( add )
      l4add_before( lru_list, l4first( lru_list ), l4link ) ;
}

static unsigned opt4file_read_file( FILE4 *file, long pos, char *buf )
{
   unsigned len ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 || pos < 0 || buf == 0 )
         e4severe( e4parm, E4_OPT4FILE_RD_FL ) ;
   #endif

   opt = &file->code_base->opt ;

   if ( opt->write_file == file )
      if ( (unsigned long)pos + opt->block_size >= opt->write_start_pos && (unsigned long) pos < opt->write_cur_pos )
         opt4flush_write_buffer( opt ) ;

   file->do_buffer = 0 ;
   len = file4read( file, pos, buf, (unsigned)(opt->block_size) ) ;
   file->do_buffer = 1 ;

   return len ;
}

static void opt4file_read_sp_buffer( FILE4 *file, unsigned long pos )
{
   unsigned long len, cur_pos, save_block_size ;
   OPT4BLOCK *block_on ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( file == 0 )
         e4severe( e4parm, E4_OPT4FILE_RD_SB ) ;
   #endif

   opt = &file->code_base->opt ;

   cur_pos = pos ;
   save_block_size = opt->block_size ;

   /* first do a sample to see which way to read */
   block_on = opt4file_return_block( file, pos+save_block_size, opt4file_hash( file, pos+save_block_size ) ) ;
   if ( block_on != 0 ) /* read previous blocks, not next blocks */
   {
      if ( pos > ( opt->buffer_size - save_block_size ) )
         pos -= ( opt->buffer_size - save_block_size ) ;
      else
         pos = 0 ;
   }
   opt->block_size = opt->buffer_size ;
   len = opt4file_read_file( file, pos, opt->read_buffer ) ;
   opt->block_size = save_block_size ;

   if ( len == 0 )
      return ;

   /* first do the last block, in case there is a length issue */
   cur_pos = pos + save_block_size * ( ( len - 1 ) >> opt->block_power ) ;
   block_on = opt4file_return_block( file, cur_pos, opt4file_hash( file, cur_pos ) ) ;
   if ( block_on == 0 )
   {
      block_on = opt4file_get_block( file ) ;
      memcpy( block_on->data, opt->read_buffer + ( cur_pos - pos ), (unsigned)( len - ( cur_pos - pos ) ) ) ;
      opt4block_add( block_on, file, (unsigned)( len - ( cur_pos - pos ) ), opt4file_hash( file, cur_pos ), cur_pos ) ;
   }

   if ( cur_pos != pos )
      for ( cur_pos -= save_block_size ;; cur_pos -= save_block_size )
      {
         block_on = opt4file_return_block( file, cur_pos, opt4file_hash( file, cur_pos ) ) ;
         if ( block_on == 0 )
         {
            block_on = opt4file_get_block( file ) ;
            memcpy( block_on->data, opt->read_buffer + ( cur_pos - pos ), (unsigned)(save_block_size) ) ;
            opt4block_add( block_on, file, (unsigned)(save_block_size), opt4file_hash( file, cur_pos ), cur_pos ) ;
         }
         else   /* update the lru status */
            opt4file_lru_bottom( file, &block_on->lru_link, 1 ) ;
         if ( cur_pos == pos )
            break ;
      }

   return ;
}

OPT4BLOCK *S4FUNCTION opt4file_return_block( FILE4 *file, long pos, long hash_val )
{
   OPT4CMP compare ;
   OPT4BLOCK *block_on ;

   #ifdef S4DEBUG
      if ( file->code_base->opt.num_buffers == 0 || file == 0  || hash_val < 0 || pos < 0 )
         e4severe( e4parm, E4_OPT4FILE_RB ) ;
   #endif

   if ( file->do_buffer == 0 )
      return 0 ;

   compare.file = file ;
   memcpy( (void *)&compare.pos, (void *)&pos, sizeof( compare.pos ) ) ;
   if ( (block_on = (OPT4BLOCK *)l4first( &file->code_base->opt.lists[hash_val] ) ) != 0 )
   {
      if ( !u4memcmp( (void *)&block_on->file, (void *)&compare, sizeof( compare ) ) )
         return block_on ;
      else
         for( ;; )
         {
            block_on = (OPT4BLOCK *)l4next( &file->code_base->opt.lists[hash_val], block_on ) ;
            if ( block_on == 0 )
               return 0 ;
            compare.file = file ;
            memcpy( (void *)&compare.pos, (void *)&pos, sizeof( compare.pos ) ) ;
            if ( !u4memcmp( (void *)&block_on->file, (void *)&compare, sizeof( compare ) ) )
               return block_on ;
         }
   }
   return 0 ;
}

void S4FUNCTION opt4set_priority( OPT4 *opt, char *modes )
{
   char i ;

   #ifdef S4DEBUG
      char chk ;

      chk = 0 ;
      if ( opt == 0 )
         e4severe( e4parm, E4_OPT4SET_PRIO ) ;
      for ( i = 0 ; i < 3 ; i++ )
         if ( (int)modes[i] >= 0 && (int)modes[i] <= 2 )
            chk |= 0x01 << modes[i] ;
      if ( chk != 7 )
         e4severe( e4parm, E4_OPT4SET_PRIO ) ;
   #endif

   for ( i = 0 ; i < 3 ; i++ )
      switch( modes[i] )
      {
         case OPT4DBF:
            opt->prio[i] = &opt->dbf_lru ;
            break;
         case OPT4INDEX:
            opt->prio[i] = &opt->index_lru ;
            break;
         case OPT4OTHER:
            opt->prio[i] = &opt->other_lru ;
            break;
         default:
            #ifdef S4DEBUG
               e4severe( e4info, E4_OPT4SET_PRIO ) ;
            #endif
            break;
      }
}

#ifndef S4DETECT_OFF
void S4FUNCTION d4update_prio( CODE4 *code_base )
{
   unsigned char tmp_mode ;
   OPT4 *opt ;

   #ifdef S4DEBUG
      if ( code_base == 0 )
	 e4severe( e4parm, E4_D4UPDATE_PRIO ) ;
   #endif

   if ( code_base->error_code < 0 )
      return ;

   opt = &code_base->opt ;

   if ( opt->old_mode == code_base->mode )
      return ;

   opt->old_mode = code_base->mode ;
   tmp_mode = code_base->mode ;
   code_base->mode = 0 ;
   opt->check_count = 0 ;

   opt->prio[2] = &opt->other_lru ;

   opt->is_skip = (char)(( tmp_mode >> 1 ) & 0X03 ) ;  /* =0 if go_dbf, 1 if skip_dbf, 2,3 if skip_ndx */

   tmp_mode >>= 3 ;
   if ( !tmp_mode )   /* d4go/d4skip_rec_order/i4skip mode only */
   {
      opt->prio[1] = &opt->index_lru ;
      opt->prio[0] = &opt->dbf_lru ;
      return ;
   }

   opt->is_skip = 0 ;
   opt->prio[1] = &opt->dbf_lru ;
   opt->prio[0] = &opt->index_lru ;

   tmp_mode >>= 1 ;
   if ( !tmp_mode )   /* i4seek mode */
      return ;

   tmp_mode >>= 2 ;
   if ( !tmp_mode )   /* d4append or write_keys mode */
   {
      opt->prio[1] = &opt->index_lru ;
      opt->prio[0] = &opt->dbf_lru ;
      return ;
   }

   tmp_mode >>= 2 ;

   if ( !tmp_mode )   /* start mode - just reset mode and check in a few reads */
      opt->check_count = OPT4CHECK_RATE - 5 ;  /* check again 5 times from now */
   else /* error */
      #ifdef S4DEBUG
	 e4severe( e4info, E4_OPT_INV ) ;
      #else
	 e4( code_base, e4info, E4_OPT_INV ) ;
      #endif
}
#endif

#endif   /* not S4OPTIMIZE_OFF */
