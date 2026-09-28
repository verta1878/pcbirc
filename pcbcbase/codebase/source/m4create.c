/* m4create.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved.  */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4MEMO_OFF
#ifndef S4OFF_WRITE

int memo4file_create( MEMO4FILE *m4file, CODE4 *c4, DATA4 *d4, char *name )
{
   MEMO4HEADER *header_ptr ;
   char buf[258] ;
   int rc, save_flag, safety ;
   #ifdef S4BYTE_SWAP
      MEMO4HEADER swap ;
   #endif
   #ifndef S4MFOX
      #ifndef S4MNDX
         char memo_name[9] ;
      #endif
   #endif

   m4file->data = d4 ;

   #ifndef S4MNDX
      #ifdef S4DEBUG
         if ( sizeof( MEMO4HEADER ) > c4->mem_size_memo )
            e4severe( e4info, E4_MEMO4FILE_CR ) ;

         #ifdef S4MFOX
            if ( c4->mem_size_memo <= 32 || c4->mem_size_memo > 512 * 32 )
         #else
            if ( c4->mem_size_memo % 512 != 0 || c4->mem_size_memo > 512 * 63 )
         #endif
               e4severe( e4info, E4_INFO_C4C ) ;
      #endif
   #endif

   header_ptr = (MEMO4HEADER *)u4alloc_er( c4, c4->mem_size_memo ) ;
   if ( header_ptr == 0 )
      return -1 ;

   #ifdef S4MFOX
      m4file->block_size = c4->mem_size_memo ;
      if ( m4file->block_size > 512 )
         header_ptr->next_block = x4reverse_long( 1L ) ;
      else
         header_ptr->next_block = x4reverse_long( B4BLOCK_SIZE/m4file->block_size ) ;
      header_ptr->block_size = x4reverse_short( c4->mem_size_memo ) ;
   #else
      #ifdef S4MNDX
         m4file->block_size = MEMO4SIZE ;
         header_ptr->next_block = 1 ;
      #else
         header_ptr->next_block = 1 ;
         header_ptr->x102 = 0x102 ;
         m4file->block_size = header_ptr->block_size = c4->mem_size_memo ;
      #endif
   #endif

   if ( name == 0 )
   {
      u4name_piece( buf, sizeof( buf ), d4->file.name, 1, 0 ) ;
      u4name_ext( buf, sizeof( buf ), "TMP", 1 ) ;

      save_flag = c4->create_error ;
      safety = c4->safety ;
      c4->create_error = c4->safety = 0 ;
      rc = file4create( &m4file->file, c4, buf, 1 ) ;
      c4->create_error = save_flag ;
      c4->safety = safety ;

      if ( rc == 0 )
         m4file->file.is_temp = 1 ;
      else
         return -1 ;
   }
   else
   {
      u4ncpy( buf, name, sizeof( buf ) ) ;
      #ifndef S4MFOX
         #ifndef S4MNDX
            memset( memo_name, 0, sizeof( memo_name ) ) ;
            u4name_piece( memo_name, sizeof(memo_name), name, 0, 0 ) ;
            memcpy( header_ptr->file_name, memo_name, 8 ) ;
         #endif
      #endif
      #ifdef S4MFOX
         u4name_ext( buf, sizeof(buf), "FPT", 1 ) ;
      #else
         u4name_ext( buf, sizeof(buf), "DBT", 1 ) ;
      #endif

      file4create( &m4file->file, c4, name, 1 ) ;
   }
   #ifdef S4BYTE_SWAP
      memcpy( (void *)&swap, (void *)header_ptr, sizeof( MEMO4HEADER ) ) ;

      swap.next_block = x4reverse_long( swap.next_block ) ;
      swap.x102 = 0x0201 ;
      swap.block_size = x4reverse_short( swap.block_size ) ;

      rc = file4write( &m4file->file, 0L, &swap, sizeof( MEMO4HEADER ) ) ;
   #else
      rc = file4write( &m4file->file, 0L, header_ptr, sizeof(  MEMO4HEADER ) ) ;
   #endif
   #ifdef S4MFOX
      file4len_set( &m4file->file, 512 ) ;
   #endif
   u4free( header_ptr ) ;
   return rc ;
}

int memo4file_dump( MEMO4FILE *f4memo, long memo_id, char *ptr, unsigned len )
{
   long pos ;
   int  rc ;
   #ifdef S4MNDX
      char one_a = 0x1A ;
   #else
      MEMO4BLOCK  memo_block ;

      #ifdef S4MFOX
         memo_block.type = x4reverse_long( 1L ) ;
         memo_block.num_chars = x4reverse_long( len ) ;
      #else
         memo_block.minus_one = -1 ;
         memo_block.start_pos = sizeof(long) + 2 * sizeof( short ) ;
         memo_block.num_chars = memo_block.start_pos + len ;
         #ifdef S4BYTE_SWAP
            memo_block.start_pos = x4reverse_short( memo_block.start_pos ) ;
            memo_block.num_chars = x4reverse_long( memo_block.num_chars ) ;
         #endif
      #endif
   #endif

   pos = memo_id * f4memo->block_size ;

   #ifndef S4MNDX
      rc = file4write( &f4memo->file, pos, &memo_block, sizeof( MEMO4BLOCK ) ) ;
      if ( rc != 0 ) return rc ;
      #ifdef S4MFOX
         pos += sizeof( MEMO4BLOCK) ;
      #else
         pos += sizeof(long) + 2*sizeof(short) ;
      #endif
   #endif

   #ifdef S4MNDX
      rc = file4write( &f4memo->file, pos, ptr, len ) ;
      if ( rc != 0 ) return rc ;
      return file4write( &f4memo->file, pos+len, &one_a, 1 ) ;
   #else
      return  file4write( &f4memo->file, pos, ptr, len ) ;
   #endif
}

#endif  /* S4OFF_WRITE */
#endif  /* S4MEMO_OFF */
