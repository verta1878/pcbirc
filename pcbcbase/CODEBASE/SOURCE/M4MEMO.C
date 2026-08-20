/* m4memo.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved.  */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4MEMO_OFF

#ifdef S4MFOX
long S4FUNCTION memo4len_part( MEMO4FILE *f4memo, long memo_id )
{
   long  pos ;
   MEMO4BLOCK memo_block ;

   if ( memo_id <= 0L )
      return 0 ;

   pos = memo_id * f4memo->block_size ;
   if ( file4read_all( &f4memo->file, pos, &memo_block, sizeof( MEMO4BLOCK ) ) < 0)
      return -1 ;

   return x4reverse_long( memo_block.num_chars ) ;
}
#endif

#ifndef S4OFF_WRITE
int S4FUNCTION d4memo_compress( DATA4 *data )
{
   char *rd_buf, *wr_buf, *ptr ;
   FILE4SEQ_READ   rd ;
   FILE4SEQ_WRITE  wr ;
   MEMO4FILE new_file ;
   CODE4 *code_base ;
   unsigned buf_size, ptr_len ;
   long cur_count, i_rec, new_id, pos, memo_len ;
   int  rc, i, save_flag ;
   FIELD4 *field ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4MEMO_COMP ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4MEMO_COMP ) ;
   #endif

   code_base = data->code_base ;
   if ( code_base->error_code < 0 )
      return -1 ;

   if ( data->n_fields_memo == 0 )
      return 0 ;

   rc = d4update( data ) ;
   if ( rc )
      return rc ;

   #ifndef S4SINGLE
      rc = d4lock_file( data ) ;
      if ( rc )
         return rc ;
      #ifndef N4OTHER
         if ( !rc )
            if ( data->n_fields_memo > 0 && data->memo_file.file.hand != -1 )
               rc = memo4file_lock( &data->memo_file ) ;
      #endif
      if ( rc )
         return rc ;
   #endif

   save_flag = code_base->mem_size_memo ;
   code_base->mem_size_memo = data->memo_file.block_size ;

   if ( memo4file_create( &new_file, code_base, data, 0 ) < 0 )
      return -1 ;

   code_base->mem_size_memo = save_flag ;
   new_file.block_size = data->memo_file.block_size ;

   rd_buf = wr_buf = 0 ;
   buf_size = code_base->mem_size_buffer ;

   #ifndef S4OPTIMIZE_OFF
      has_opt = code_base->has_opt && code_base->opt.num_buffers ;
      d4opt_suspend( code_base ) ;
   #endif

   for (; buf_size > data->record_width; buf_size -= 0x800 )
   {
      rd_buf = (char *)u4alloc( buf_size ) ;
      if ( rd_buf == 0 )
         continue ;

      wr_buf = (char *)u4alloc( buf_size ) ;
      if ( wr_buf )
         break ;

      u4free( rd_buf ) ;
      rd_buf = 0 ;
   }

   file4seq_read_init( &rd, &data->file, d4record_position( data, 1L ), rd_buf, buf_size ) ;
   file4seq_write_init( &wr, &data->file, d4record_position( data, 1L ), wr_buf, buf_size ) ;

   cur_count = d4reccount( data ) ;

   ptr = 0 ;
   ptr_len = 0 ;

   for ( i_rec= 1L ; i_rec <= cur_count ; i_rec++ )
   {
      if ( file4seq_read_all( &rd, data->record, data->record_width ) < 0 )
         break ;

      for ( i = 0 ; i < data->n_fields_memo ; i++ )
      {
         field = data->fields_memo[i].field ;

         #ifdef S4FOX
            pos = 0L ;
            memo_len = memo4len_part( &data->memo_file, f4long( field ) ) ;

            if ( memo_len > 0L )
            {
               do
               {
                  #ifdef S4DEBUG
                     if ( pos > memo_len )
                        e4severe( e4info, E4_D4MEMO_COMP ) ;
                  #endif

                  if ( memo4file_read_part( &data->memo_file, f4long( field ), &ptr, &ptr_len, pos, UINT_MAX ) < 0 )
                  {                               
                     rc = -1 ;
                     break ;
                  }

                  new_id = 0L ;
                  if ( memo4file_write_part( &new_file, &new_id, ptr, memo_len, pos, ptr_len ) < 0 )
                  {                               
                     rc = -1 ;
                     break ;
                  }
                  pos += ptr_len ;
               } while( pos != memo_len ) ;
               c4ltoa45( new_id, f4ptr(field), -( (int)field->len ) ) ;
            }
            else
               c4ltoa45( 0L, f4ptr(field), -( (int)field->len) ) ;
         #else
            if ( memo4file_read( &data->memo_file, f4long(field), &ptr, &ptr_len ) < 0 )
            {                               
               rc = -1 ;
               break ;
            }

            new_id = 0L ;
            if ( memo4file_write( &new_file, &new_id, ptr, ptr_len ) < 0 )
            {                               
               rc = -1 ;
               break ;
            }

            c4ltoa45( new_id, f4ptr(field), - ((int) field->len) ) ;
            #endif
      }
      file4seq_write( &wr, data->record, data->record_width ) ;
   }

   if ( rc < 0 )
      file4close( &new_file.file ) ;  /* error occurred */
   else
      file4seq_write_flush(&wr) ;

   u4free( ptr ) ;
   u4free( rd_buf ) ;
   u4free( wr_buf ) ;

   if ( rc == 0 )
      rc = file4replace( &data->memo_file.file, &new_file.file ) ;

   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( code_base ) ;
   #endif

   return rc ;
}
#endif /* S4OFF_WRITE */

#ifndef S4MFOX
#ifndef S4MNDX

int memo4file_chain_flush( MEMO4FILE *f4memo, MEMO4CHAIN_ENTRY *chain )
{
   #ifdef S4BYTE_SWAP
      MEMO4CHAIN_ENTRY swap ;
   #endif

   if ( chain->to_disk )
   {
      chain->to_disk = 0 ;
      #ifdef S4BYTE_SWAP
         memcpy( (void *)&swap, (void *)chain, sizeof( MEMO4CHAIN_ENTRY ) ) ;
         swap.next = x4reverse_long( swap.next ) ;
         swap.num = x4reverse_long( swap.num ) ;

         return file4write( &f4memo->file, chain->block_no * f4memo->block_size, &swap, 2*sizeof(long) ) ;
      #else
         return file4write( &f4memo->file, chain->block_no * f4memo->block_size, chain, 2*sizeof(long) ) ;
      #endif
   }
   return 0 ;
}

int memo4file_chain_skip( MEMO4FILE *f4memo, MEMO4CHAIN_ENTRY *chain )
{
   unsigned len_read ;

   chain->to_disk = 0 ;
   chain->block_no = chain->next ;

   if ( chain->next < 0 )
      len_read = 0 ;
   else
   {
      len_read = file4read( &f4memo->file, chain->next * f4memo->block_size, chain, sizeof(chain->next)+sizeof(chain->num) ) ;
      #ifdef S4BYTE_SWAP
         chain->next = x4reverse_long( chain->next ) ;
         chain->num = x4reverse_long( chain->num ) ;
      #endif
   }

   if ( f4memo->data->code_base->error_code < 0 )
      return -1 ;
   if ( len_read == 0 )
   {
      chain->num = -1 ;
      chain->next = -1 ;
      return 0 ;
   }
   if ( len_read != sizeof(chain->next)+sizeof(chain->num) )
      return file4read_error( &f4memo->file ) ;
   return 0 ;
}
#endif
#endif

#endif  /* S4MEMO_OFF */

/* Make the memo file entries current */
int S4FUNCTION d4validate_memo_ids( DATA4 *data )
{
   #ifdef S4MEMO_OFF
      return 0 ;
   #else
      int rc, i ;
      char *from_ptr ;

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4VALID_MID ) ;
      #endif

      if ( data->memo_validated )
         return 0 ;

      #ifndef S4SINGLE
         if ( data->rec_num > 0 )
         {
            rc = d4lock( data, data->rec_num ) ;
            if ( rc )
               return rc ;
         }
      #endif

      if ( d4read_old( data, data->rec_num ) < 0 )
         return -1 ;

      for ( i = 0 ; i < data->n_fields_memo ; i++ )
      {
         from_ptr = data->record_old + data->fields_memo[i].field->offset ;
         memcpy( f4ptr( data->fields_memo[i].field), from_ptr, 10 ) ;
      }
      data->memo_validated = 1 ;
      return 0 ;
   #endif  /* S4MEMO_OFF */
}
