/* d4zap.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
int S4FUNCTION d4zap( DATA4 *d4, long r1, long r2 )
{
   int rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4ZAP ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( r1 < 0 || d4 == 0 || r2 < 0 )
         e4severe( e4parm, E4_D4ZAP ) ;
   #endif  /* S4DEBUG */

   #ifndef S4SINGLE
      rc = d4lock_all( d4 ) ;   /* returns -1 if code_base->error_code < 0 */
      if ( rc )
         return rc ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      has_opt = d4->code_base->has_opt && d4->code_base->opt.num_buffers ;
      d4opt_suspend( d4->code_base ) ;
   #endif

   rc = d4zap_data( d4, r1, r2 ) ;   /* returns -1 if code_base->error_code < 0 */
   if ( rc == 0 )
   {
      if ( d4reccount( d4 ) == 0 )
         d4->bof_flag = d4->eof_flag = 1 ;
      else
         d4->bof_flag = d4->eof_flag = 0 ;

      #ifndef S4INDEX_OFF
         rc = d4reindex( d4 ) ;
      #endif
      d4update_header( d4, 1, 1 ) ;
   }

   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( d4->code_base ) ;
   #endif

   return rc ;
}

int S4FUNCTION d4zap_data( DATA4 *d4, long start_rec, long end_rec )
{
   long cur_count, i_rec ;
   char *rd_buf, *wr_buf ;
   FILE4SEQ_READ   rd ;
   FILE4SEQ_WRITE  wr ;
   unsigned  buf_size ;
   int rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif

   #ifdef S4DEBUG
      if ( start_rec < 0 || d4 == 0 || end_rec < 0 )
         e4severe( e4parm, E4_D4ZAP_DATA ) ;
   #endif  /* S4DEBUG */

   #ifndef S4SINGLE
      rc = d4lock_file( d4 ) ;   /* returns -1 if code_base->error_code < 0 */
      if ( rc )
         return rc ;
   #endif

   rc = d4update( d4 ) ;   /* returns -1 if code_base->error_code < 0 */
   if ( rc )
      return rc ;

   d4->file_changed = 1 ;

   if ( start_rec == 0 )
      start_rec = 1 ;

   cur_count = d4reccount( d4 ) ;
   if ( cur_count < 0 )
      return -1 ;
   if ( start_rec > cur_count )
      return 0 ;
   if ( end_rec < start_rec )
      return 0 ;
   if ( end_rec > cur_count )
      end_rec = cur_count ;

   rd_buf = wr_buf = 0 ;
   buf_size = d4->code_base->mem_size_buffer ;

   #ifndef S4OPTIMIZE_OFF
      has_opt = d4->code_base->has_opt && d4->code_base->opt.num_buffers ;
      d4opt_suspend( d4->code_base ) ;
   #endif

   for ( ; buf_size > d4->record_width ; buf_size -= 0x800 )
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
   file4seq_read_init(  &rd, &d4->file, d4record_position(d4,end_rec+1), rd_buf, (rd_buf == 0) ? 0 : buf_size ) ;
   file4seq_write_init( &wr, &d4->file, d4record_position(d4,start_rec), wr_buf, (wr_buf == 0) ? 0 : buf_size ) ;

   for ( i_rec= end_rec+1L; i_rec <= cur_count; i_rec++ )
   {
      file4seq_read_all( &rd, d4->record, d4->record_width ) ;
      file4seq_write( &wr, d4->record, d4->record_width ) ;
   }

   file4seq_write( &wr, "\032", 1 ) ;
   rc = file4seq_write_flush( &wr ) ;
   u4free( rd_buf ) ;
   u4free( wr_buf ) ;

   #ifndef S4OPTIMIZE_OFF
      if ( has_opt )
         d4opt_start( d4->code_base ) ;
   #endif

   if ( rc < 0 )
      return -1 ;

   d4->num_recs = cur_count - ( end_rec - start_rec + 1 ) ;
   d4->rec_num = -1 ;
   d4->rec_num_old = -1 ;
   memset( d4->record, ' ', d4->record_width ) ;

   return file4len_set( &d4->file, d4record_position( d4, d4->num_recs + 1L ) + 1L ) ;
}
#endif  /* S4OFF_WRITE */
