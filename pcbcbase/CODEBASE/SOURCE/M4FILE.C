/* m4file.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved.  */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4MEMO_OFF

#ifndef S4SINGLE
  /* ndx/clipper versions have no free chain--do not lock memo file */
#ifndef N4OTHER
/* the lock is forced since a memo file lock only lasts if the .dbf file is locked */
int S4FUNCTION memo4file_lock( MEMO4FILE *f4memo )
{
   int rc, old_attempts ;

   if ( f4memo->file_lock == 1 )
      return 0 ;

   old_attempts = f4memo->file.code_base->lock_attempts ;
   f4memo->file.code_base->lock_attempts = -1 ;

   #ifdef S4MDX
      rc = file4lock( &f4memo->file, L4LOCK_POS - 1UL, 2L ) ;
   #endif

   #ifdef S4FOX
      rc = file4lock( &f4memo->file, L4LOCK_POS, 1L ) ;
   #endif

   f4memo->file.code_base->lock_attempts = old_attempts ;
   if ( rc == 0 )
      f4memo->file_lock = 1 ;
   #ifndef S4OPTIMIZE_OFF
      file4refresh( &f4memo->file ) ;   /* make sure all up to date */
   #endif
   return rc ;
}

int S4FUNCTION memo4file_unlock( MEMO4FILE *f4memo )
{
   int rc ;

   if ( f4memo->file_lock == 0 )
      return 0 ;
   #ifdef S4MDX
      rc = file4unlock( &f4memo->file, L4LOCK_POS - 1UL, 2L ) ;
   #endif
   #ifdef S4FOX
      rc = file4unlock( &f4memo->file, L4LOCK_POS, 1L ) ;
   #endif
   if ( rc == 0 )
      f4memo->file_lock = 0 ;
   return rc ;
}
#endif
#endif

int memo4file_open( MEMO4FILE *f4memo, DATA4 *d4, char *name )
{
   MEMO4HEADER  header ;
   int rc ;

   f4memo->data = d4 ;

   if ( file4open( &f4memo->file, d4->code_base, name, 1 ) )
      return -1 ;

   if ( (rc = file4read_all(&f4memo->file, 0L, &header, sizeof(header))) < 0 )
      return -1 ;

   #ifdef S4BYTE_SWAP
      header.next_block = x4reverse_long( header.next_block ) ;
      #ifndef S4MNDX
         header.x102 = 0x201 ;
         header.block_size = x4reverse_short( header.block_size ) ;
      #endif
   #endif

   #ifdef S4MFOX
      f4memo->block_size = x4reverse_short( header.block_size ) ;
   #else
      #ifdef S4MNDX
         f4memo->block_size = MEMO4SIZE ;
      #else
         f4memo->block_size = header.block_size ;
      #endif
   #endif

   return rc ;
}

#ifdef S4MFOX
/* offset is # bytes from start of memo that reading should begin, read_max is
   the maximum possible that can be read (limited to an unsigned int, so is
   16-bit/32-bit compiler dependent.
*/
int memo4file_read_part( MEMO4FILE *f4memo, long memo_id, char **ptr_ptr, unsigned *len_ptr, unsigned long offset, unsigned read_max )
{
   unsigned long pos, avail ;
   MEMO4BLOCK memo_block ;

   if ( memo_id <= 0L )
   {
      *len_ptr = 0 ;
      return 0 ;
   }

   pos = memo_id * f4memo->block_size ;

   if ( file4read_all( &f4memo->file, pos, &memo_block, sizeof( MEMO4BLOCK ) ) < 0)
      return -1 ;

   #ifdef S4BYTE_SWAP
      memo_block.start_pos = x4reverse_short( memo_block.start_pos ) ;
      memo_block.num_chars = x4reverse_long( memo_block.num_chars ) ;
   #endif

   memo_block.num_chars = x4reverse_long( memo_block.num_chars ) ;

   avail = memo_block.num_chars - offset ;
   if ( avail > (unsigned long)read_max )
      avail = read_max ;

   if ( avail > (unsigned long)*len_ptr )
   {
      if ( *len_ptr > 0 )
         u4free( *ptr_ptr ) ;
      *ptr_ptr = (char *)u4alloc_er( f4memo->file.code_base, memo_block.num_chars + 1 ) ;
      if ( *ptr_ptr == 0 )
         return e4memory ;
   }

   *len_ptr = (unsigned)avail ;

   return (int)file4read_all( &f4memo->file, offset + pos + (long)(2*sizeof(long)), *ptr_ptr, *len_ptr ) ;
}
#endif

int memo4file_read( MEMO4FILE *f4memo, long memo_id, char **ptr_ptr, unsigned *len_ptr )
{
   #ifdef S4MFOX
      return memo4file_read_part( f4memo, memo_id, ptr_ptr, len_ptr, 0L, UINT_MAX ) ;
   #else
      long  pos ;
      #ifdef S4MNDX
         long amt_read, read_max, len_read ;
         char *t_ptr ;
      #else
         MEMO4BLOCK  memo_block ;
         unsigned final_len ;
      #endif

      if ( memo_id <= 0L )
      {
         *len_ptr = 0 ;
         return 0 ;
      }

      pos = memo_id * f4memo->block_size ;

      #ifdef S4MNDX
         amt_read = 0 ;
         read_max = *len_ptr ;

         for(;;)
         {
            if ( MEMO4SIZE > read_max )  /* must increase the memo buffer size */
            {
               t_ptr = (char *)u4alloc_er( f4memo->file.code_base, amt_read + MEMO4SIZE + 1 ) ;
               if ( t_ptr == 0 )
                  return e4memory ;
               if ( *len_ptr > 0 )
               {
                  memcpy( t_ptr, *ptr_ptr, *len_ptr ) ;
                  u4free( *ptr_ptr ) ;
               }
               *ptr_ptr = t_ptr ;
               *len_ptr = (unsigned)( amt_read + MEMO4SIZE + 1 ) ;
               read_max = MEMO4SIZE ;
            }

            len_read = file4read( &f4memo->file, pos + amt_read, *ptr_ptr + amt_read, read_max ) ;
            if ( len_read <= 0 )
               return -1 ;

            for ( ; len_read > 0 ; amt_read++, len_read-- )
               if ( (*ptr_ptr)[amt_read] == 0x1A ) /* if done */
               {
                  if ( amt_read > 0 )
                  {
                     t_ptr = (char *)u4alloc_free( f4memo->file.code_base, amt_read + 1 ) ;
                     if ( t_ptr == 0 )
                     {
                        /* this will result in a correct return, but extra */
                        /* space is still allocated at end of *ptr_ptr */
                        *ptr_ptr[amt_read] = 0 ;
                        *len_ptr = (unsigned)amt_read ;
                        return 0 ;
                     }
                     memcpy( t_ptr, *ptr_ptr, (int)amt_read ) ;
                     t_ptr[amt_read] = 0 ;
                  }
                  else
                     t_ptr = 0 ;

                  u4free( *ptr_ptr ) ;
                  *ptr_ptr = t_ptr ;
                  *len_ptr = (unsigned)amt_read ;
                  return 0 ;
               }
            read_max = 0 ;
         }
      #else
         if ( file4read_all( &f4memo->file, pos, &memo_block, sizeof( MEMO4BLOCK ) ) < 0)
            return -1 ;

         #ifdef S4BYTE_SWAP
            memo_block.start_pos = x4reverse_short( memo_block.start_pos ) ;
            memo_block.num_chars = x4reverse_long( memo_block.num_chars ) ;
         #endif

         if ( memo_block.minus_one != -1 )  /* must be an invalid entry, so return an empty entry */
         {
            *len_ptr = 0 ;
            return 0 ;
         }
         else
         {
            if ( memo_block.num_chars >= USHRT_MAX )
               return e4( f4memo->file.code_base, e4info, E4_MEMO4FILE_RD ) ;

            final_len = (unsigned)memo_block.num_chars - 2 * ( sizeof(short) ) - ( sizeof(long) ) ;
            if ( final_len > *len_ptr )
            {
               if ( *len_ptr > 0 )
                  u4free( *ptr_ptr ) ;
               *ptr_ptr = (char *)u4alloc_er( f4memo->file.code_base, final_len + 1 ) ;
               if ( *ptr_ptr == 0 )
                  return e4memory ;
            }
            *len_ptr = final_len ;

            return file4read_all( &f4memo->file, pos+ memo_block.start_pos, *ptr_ptr, final_len ) ;
         }
      #endif
   #endif
}

#ifdef S4MFOX
/* Writes partial data to a memo record.
   Usage rules:
      Must call this function with an offset == 0 to write 1st part of block
      before any additional writing.  In addition, the memo_len must be
      accurately set during the first call in order to reserve the correct
      amount of memo space ahead of time.  Later calls just fill in data to
      the reserved disk space.
      len_write is the amount of data to write, offset is the number of
      bytes from the beginning of the memo in which to write the data
      Secondary calls to this function assume that everything has been
      previously set up, and merely performs a file write to the reserved
      space.  The space is not checked to see whether or not it actually
      is in the bounds specified, so use with care.
*/
int memo4file_write_part( MEMO4FILE *f4memo, long *memo_id_ptr, char *ptr, long memo_len, long offset, unsigned len_write )
{
   int str_num_blocks ;
   long pos ;
   #ifndef S4SINGLE
      #ifndef N4OTHER
         int rc, lock_cond ;
      #endif
   #endif
   MEMO4BLOCK old_memo_block ;
   MEMO4HEADER mh ;
   unsigned len_read ;
   long block_no ;
   unsigned n_entry_blks = 0 ;

   #ifdef S4DEBUG
      if ( memo_id_ptr == 0 )
         e4severe( e4parm, E4_MEMO4FILE_WR ) ;
   #endif

   if ( offset == 0 )   /* must do the set-up work */
   {
      if ( memo_len == 0 )
      {
         *memo_id_ptr = 0L ;
         return 0 ;
      }

      #ifdef S4DEBUG
         if ( f4memo->block_size <= 1 )
            e4severe( e4info, E4_INFO_IMS ) ;
      #endif

      str_num_blocks = (memo_len + sizeof(MEMO4BLOCK) + f4memo->block_size-1) / f4memo->block_size ;
      if ( *memo_id_ptr <= 0L )
         block_no = 0L ;
      else
      {
         block_no = *memo_id_ptr ;
         pos = block_no * f4memo->block_size ;

         file4read_all( &f4memo->file, pos, (char *)&old_memo_block, sizeof(old_memo_block) ) ;
         #ifdef S4BYTE_SWAP
            old_memo_block.start_pos = x4reverse_short( old_memo_block.start_pos ) ;
            old_memo_block.num_chars = x4reverse_long( old_memo_block.num_chars ) ;
         #endif

         old_memo_block.num_chars = x4reverse_long( old_memo_block.num_chars ) ;
         n_entry_blks = (unsigned) ((old_memo_block.num_chars + f4memo->block_size-1)/ f4memo->block_size ) ;
      }
      if ( n_entry_blks >= ((unsigned)str_num_blocks) && block_no )  /* write to existing position */
         *memo_id_ptr = block_no ;
      else  /* read in header record */
      {
         #ifndef S4SINGLE
         #ifndef N4OTHER
            lock_cond = f4memo->data->memo_file.file_lock ;
            rc = memo4file_lock( &f4memo->data->memo_file ) ;
            if ( rc )
               return rc ;
         #endif
         #endif

         len_read = file4read( &f4memo->file, 0, &mh, sizeof( mh ) ) ;
         if ( f4memo->data->code_base->error_code < 0 )
         {
            #ifndef S4SINGLE
            #ifndef N4OTHER
               if ( !lock_cond )
                  memo4file_unlock( &f4memo->data->memo_file ) ;
            #endif
            #endif
            return -1 ;
         }

         if ( len_read != sizeof( mh ) )
         {
            #ifndef S4SINGLE
            #ifndef N4OTHER
               if ( !lock_cond )
                  memo4file_unlock( &f4memo->data->memo_file ) ;
            #endif
            #endif
            return file4read_error( &f4memo->file ) ;
         }

         *memo_id_ptr = x4reverse_long( mh.next_block ) ;
         mh.next_block = x4reverse_long( *memo_id_ptr + str_num_blocks ) ;

         file4write( &f4memo->file, 0, &mh, sizeof( mh ) ) ;

         #ifndef S4SINGLE
         #ifndef N4OTHER
            if ( !lock_cond )
               memo4file_unlock( &f4memo->data->memo_file ) ;
         #endif
         #endif
      }
      if ( memo4file_dump( f4memo, *memo_id_ptr, ptr, len_write ) < 0 )
         return -1 ;
   }
   else
      return file4write( &f4memo->file, *memo_id_ptr * f4memo->block_size + offset, ptr, len_write ) ;

   return 0 ;
}
#endif

int memo4file_write( MEMO4FILE *f4memo, long *memo_id_ptr, char *ptr, unsigned ptr_len )
{
   #ifdef S4MFOX
      return memo4file_write_part( f4memo, memo_id_ptr, ptr, ptr_len, 0L, ptr_len ) ;
   #else
      int str_num_blocks ;
      long pos ;
      #ifndef S4MDX
         #ifndef S4SINGLE
            #ifndef N4OTHER
               int rc, lock_cond ;
            #endif
         #endif
      #endif
      #ifdef S4MNDX
         MEMO4HEADER mh ;
         long len_read ;
         char buf[MEMO4SIZE] ;
         int read_size, i ;
      #else
         MEMO4BLOCK old_memo_block ;
         MEMO4CHAIN_ENTRY new_entry, cur, prev ;
         int  str_written ;
         long prev_prev_entry, prev_prev_num ;
         long file_len, extra_len ;
      #endif

      #ifdef S4DEBUG
         if ( memo_id_ptr == 0 )
            e4severe( e4parm, E4_MEMO4FILE_WR ) ;
      #endif

      #ifdef S4MNDX
         if ( ptr_len == 0 )
         {
            *memo_id_ptr = 0L ;
            return 0 ;
         }

         str_num_blocks = (ptr_len + f4memo->block_size-1) / f4memo->block_size ;

         if ( *memo_id_ptr <= 0L )
            *memo_id_ptr = 0L ;
         else    /* read in old record to see if new entry can fit */
         {
            read_size = 0 ;
            pos = *memo_id_ptr * f4memo->block_size ;

            do
            {
               read_size += MEMO4SIZE ;

               len_read = file4read( &f4memo->file, pos, buf, MEMO4SIZE ) ;
               if ( len_read <= 0 )
                  return file4read_error( &f4memo->file ) ;

               for ( i=0 ; ((unsigned) i) < len_read ; i++ )
                  if ( buf[i] == (char)0x1A )  break ;

               #ifdef S4DEBUG
                  if ( buf[i] != (char)0x1A && len_read != MEMO4SIZE )
                     return e4( f4memo->file.code_base, e4info, E4_INFO_CMF ) ;
               #endif

               pos += MEMO4SIZE ;
            } while ( i >= MEMO4SIZE && buf[i] != (char) 0x1A ) ;  /* Continue if Esc is not located */

            if ( ((unsigned)read_size) <= ptr_len )   /* there is not room */
               *memo_id_ptr = 0 ;
         }

         if ( *memo_id_ptr == 0 )   /* add entry at eof */
         {
            #ifndef S4SINGLE
            #ifndef N4OTHER
               lock_cond = f4memo->data->memo_file.file_lock ;
               rc = memo4file_lock( &f4memo->data->memo_file ) ;
               if ( rc )
                  return rc ;
            #endif
            #endif

            len_read = file4read( &f4memo->file, 0, &mh, sizeof( mh ) ) ;
            #ifdef S4BYTE_SWAP
               mh.next_block = x4reverse_long( mh.next_block ) ;
            #endif
            if ( f4memo->data->code_base->error_code < 0 )
            {
               #ifndef S4SINGLE
               #ifndef N4OTHER
                  if ( !lock_cond )
                     memo4file_unlock( &f4memo->data->memo_file ) ;
               #endif
               #endif
               return -1 ;
            }

            if ( len_read != sizeof( mh ) )
            {
               #ifndef S4SINGLE
               #ifndef N4OTHER
                  if ( !lock_cond )
                     memo4file_unlock( &f4memo->data->memo_file ) ;
               #endif
               #endif
               return file4read_error( &f4memo->file ) ;
            }

            *memo_id_ptr = mh.next_block ;
            mh.next_block = *memo_id_ptr + str_num_blocks ;
            #ifdef S4BYTE_SWAP
               mh.next_block = x4reverse_long( mh.next_block ) ;
            #endif

            file4write( &f4memo->file, 0, &mh, sizeof( mh ) ) ;

            #ifndef S4SINGLE
            #ifndef N4OTHER
               if ( !lock_cond )
                  memo4file_unlock( &f4memo->data->memo_file ) ;
            #endif
            #endif

            #ifdef S4BYTE_SWAP
               mh.next_block = x4reverse_long( mh.next_block ) ;
            #endif
         }


         if ( memo4file_dump( f4memo, *memo_id_ptr, ptr, ptr_len ) < 0 )
            return -1 ;

         return 0 ;
      #else
         /* S4MMDX */
         memset( (void *)&new_entry, 0, sizeof(new_entry) ) ;
         new_entry.block_no = *memo_id_ptr ;

         str_written = 0 ;
         if ( ptr_len == 0 )
         {
            str_written = 1 ;
            *memo_id_ptr = 0 ;
         }

         /* Initialize information about the old memo entry */
         if ( new_entry.block_no <= 0L )
         {
            if ( str_written )
            {
               *memo_id_ptr = 0L ;
               return 0 ;
            }
            new_entry.num = 0 ;
         }
         else
         {
            pos = new_entry.block_no * f4memo->block_size ;

            file4read_all( &f4memo->file, pos, (char *)&old_memo_block, sizeof(old_memo_block) ) ;
            #ifdef S4BYTE_SWAP
               old_memo_block.start_pos = x4reverse_short( old_memo_block.start_pos ) ;
               old_memo_block.num_chars = x4reverse_long( old_memo_block.num_chars ) ;
            #endif

            new_entry.num = ((unsigned)old_memo_block.num_chars + f4memo->block_size-1)/ f4memo->block_size ;
         }

         str_num_blocks = (ptr_len+2*(sizeof(short))+(sizeof(long))+ f4memo->block_size-1) / f4memo->block_size ;

         if ( new_entry.num >= str_num_blocks  &&  !str_written )
         {
            *memo_id_ptr = new_entry.block_no + new_entry.num - str_num_blocks ;
            if ( memo4file_dump( f4memo, *memo_id_ptr, ptr, ptr_len ) < 0 )
               return -1 ;

            str_written = 1 ;
            if ( new_entry.num == str_num_blocks )
               return 0 ;

            new_entry.num -= str_num_blocks ;
         }

         /* Initialize 'chain' */
         memset( (void *)&cur, 0, sizeof(cur) ) ;
         memset( (void *)&prev, 0, sizeof(prev) ) ;

         for(;;)
         {
            if ( f4memo->data->code_base->error_code < 0 )
               return -1 ;

            memo4file_chain_flush( f4memo, &prev ) ;
            prev_prev_entry = prev.block_no ;
            prev_prev_num = prev.num ;

            memcpy( (void *)&prev, (void *)&cur, sizeof(prev) ) ;

            if ( new_entry.block_no > 0  &&  prev.next > new_entry.block_no )
            {
               /* See if the new entry fits in here */
               memcpy( (void *)&cur, (void *)&new_entry, sizeof(cur) ) ;
               new_entry.block_no = 0 ;
               cur.next  = prev.next ;
               prev.next = cur.block_no ;
               cur.to_disk = prev.to_disk = 1 ;
            }
            else
               memo4file_chain_skip( f4memo, &cur ) ;

            /* See if the entries can be combined. */
            if ( prev.block_no + prev.num == cur.block_no && prev.num )
            {
               /* 'cur' becomes the combined groups. */
               prev.to_disk = 0 ;
               cur.to_disk  = 1 ;

               cur.block_no = prev.block_no ;
               if ( cur.num >= 0 )
                  cur.num  += prev.num ;
               prev.block_no = prev_prev_entry ;
               prev.num = prev_prev_num ;
            }

            if ( str_written )
            {
               if ( new_entry.block_no == 0 )
               {
                  memo4file_chain_flush( f4memo, &prev ) ;
                  memo4file_chain_flush( f4memo, &cur ) ;
                  return 0 ;
               }
            }
            else  /* 'str' is not yet written, try the current entry */
            {
               if ( cur.next == -1 )  /* End of file */
                  cur.num = str_num_blocks ;

               if ( cur.num >= str_num_blocks )
               {
                  cur.num -= str_num_blocks ;
                  *memo_id_ptr = cur.block_no + cur.num ;
                  memo4file_dump( f4memo, *memo_id_ptr, ptr, ptr_len ) ;
                  if ( cur.next == -1 ) /* if end of file */
                  {
                     /* For dBASE IV compatibility */
                     file_len  = file4len( &f4memo->file ) ;
                     extra_len = f4memo->block_size -  file_len % f4memo->block_size ;
                     if ( extra_len != f4memo->block_size )
                        file4len_set( &f4memo->file, file_len+extra_len ) ;
                  }

                  str_written = 1 ;

                  if ( cur.num == 0 )
                  {
                     if ( cur.next == -1 ) /* End of file */
                        prev.next = cur.block_no + str_num_blocks ;
                     else
                        prev.next = cur.next ;
                     prev.to_disk = 1 ;
                     cur.to_disk = 0 ;
                  }
                  else
                     cur.to_disk = 1 ;
               }
            }
         }
      #endif
   #endif
}
#endif
