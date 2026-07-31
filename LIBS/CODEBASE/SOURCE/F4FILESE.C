/* f4filese.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

unsigned S4FUNCTION file4seq_read( FILE4SEQ_READ *seq_read, void *ptr, unsigned len )
{
   CODE4 *c4 ;
   unsigned urc, buffer_i, copy_bytes ;
   long lrc ;

   #ifdef S4DEBUG
      if ( seq_read == 0 || ( ptr == 0 && len ) )
         e4severe( e4parm, E4_F4SEQ_READ ) ;
      if ( seq_read->file == 0 )
         e4severe( e4info, E4_F4SEQ_READ ) ;
   #endif

   c4 = seq_read->file->code_base ;
   if ( c4->error_code < 0 )
      return 0 ;

   if ( seq_read->buffer == 0 )
   {
      urc = file4read( seq_read->file, seq_read->pos, ptr, len ) ;
      seq_read->pos += urc ;
      return urc ;
   }

   if ( seq_read->avail == 0 )
   {
      #ifdef S4DEBUG
         if ( seq_read->pos < 0 )
            e4severe( e4result, E4_F4SEQ_READ ) ;
      #endif

      #ifdef S4WINDOWS
         lrc = _llseek( seq_read->file->hand, seq_read->pos, 0 ) ;
      #else
         lrc = lseek( seq_read->file->hand, seq_read->pos, 0 ) ;
      #endif
      if ( lrc < 0 )
      {
         file4read_error( seq_read->file ) ;
         return 0 ;
      }

      #ifdef S4WINDOWS
         seq_read->avail = seq_read->working = (unsigned)_lread( seq_read->file->hand, seq_read->buffer, seq_read->next_read_len ) ;
      #else
         seq_read->avail = seq_read->working = (unsigned)read( seq_read->file->hand, seq_read->buffer, seq_read->next_read_len ) ;
      #endif

      if ( seq_read->working == UINT_MAX )
      {
         file4read_error( seq_read->file ) ;
         return 0 ;
      }

      #ifdef S4DEBUG
         /* Make sure reading is aligned correctly for maximum speed */
         if ( ( seq_read->pos + seq_read->next_read_len ) % 0x400 && seq_read->avail )
            e4severe( e4result, E4_F4SEQ_READ ) ;
      #endif
      seq_read->pos += seq_read->working ;
      seq_read->next_read_len = seq_read->total ;
   }

   if ( seq_read->avail >= len )
   {

      buffer_i = seq_read->working - seq_read->avail ;
      memcpy( ptr, seq_read->buffer + buffer_i, len ) ;
      seq_read->avail -= len ;
      return len ;
   }
   else
   {
      if ( seq_read->avail == 0 )
         return 0 ;

      buffer_i = seq_read->working - seq_read->avail ;
      memcpy( ptr, seq_read->buffer + buffer_i, seq_read->avail ) ;

      copy_bytes = seq_read->avail ;
      seq_read->avail = 0 ;

      urc = file4seq_read( seq_read, (char *)ptr + copy_bytes, len - copy_bytes ) ;
      if ( c4->error_code < 0 )
         return 0 ;

      return urc + copy_bytes ;
   }
}

int S4FUNCTION file4seq_read_all( FILE4SEQ_READ *seq_read, void *ptr, unsigned len )
{
   unsigned len_read ;

   #ifdef S4DEBUG
      if ( seq_read == 0 || ( ptr == 0 && len ) )
         e4severe( e4parm, E4_F4SEQ_READ_ALL ) ;
      if ( seq_read->file == 0 )
         e4severe( e4info, E4_F4SEQ_READ_ALL ) ;
   #endif

   len_read = file4seq_read( seq_read, ptr, len ) ;
   if ( seq_read->file->code_base->error_code < 0 )
      return -1 ;

   if ( len_read != len )
      return file4read_error( seq_read->file ) ;
   return 0 ;
}

void S4FUNCTION file4seq_read_init( FILE4SEQ_READ *seq_read, FILE4 *file, long start_pos, void *buffer, unsigned buffer_len )
{
   #ifdef S4DEBUG
      if ( start_pos < 0 || seq_read == 0 || file == 0 || ( buffer_len && buffer == 0 ) )
         e4severe( e4parm, E4_F4SEQ_READ_IN ) ;
   #endif

   memset( (void *)seq_read, 0, sizeof( FILE4SEQ_READ ) ) ;

   #ifndef S4OPTIMIZE_OFF
      file4low_flush( file, 1 ) ;
   #endif
   if ( buffer_len > 0 )
   {
      seq_read->total = buffer_len & 0xFC00 ;  /* Make it a multiple of 1K */
      if ( seq_read->total > (unsigned)( start_pos % 0x400 ) )
         seq_read->next_read_len = seq_read->total - (unsigned)( start_pos % 0x400 ) ;
      seq_read->buffer = (char *)buffer ;
   }
   seq_read->pos = start_pos ;
   seq_read->file = file ;
}

int S4FUNCTION file4seq_write( FILE4SEQ_WRITE *seq_write, void *buffer, unsigned ptr_len )
{
   int rc ;
   unsigned first_len ;

   #ifdef S4DEBUG
      if ( seq_write == 0 || ( ptr_len && buffer == 0 ) )
         e4severe( e4parm, E4_F4SEQ_WRITE ) ;
   #endif

   if ( seq_write->file->code_base->error_code < 0 )
      return -1 ;

   if ( seq_write->buffer == 0 )
   {
      rc = file4write( seq_write->file, seq_write->pos, buffer, ptr_len ) ;
      seq_write->pos += ptr_len ;
      return rc ;
   }

   if ( seq_write->avail == 0 )
      if ( file4seq_write_flush( seq_write ) < 0 )
         return -1 ;

   if ( seq_write->avail >= ptr_len )
   {
      #ifdef S4DEBUG
         if ( seq_write->working < seq_write->avail || ( seq_write->working - seq_write->avail + ptr_len ) > seq_write->total )
            e4severe( e4result, E4_F4SEQ_WRITE ) ;
      #endif
      memcpy( seq_write->buffer + ( seq_write->working - seq_write->avail ), buffer, ptr_len ) ;
      seq_write->avail -= ptr_len ;
      return 0 ;
   }
   else
   {
      first_len = seq_write->avail ;

      #ifdef S4DEBUG
         if ( seq_write->working < seq_write->avail || seq_write->working > seq_write->total )
            e4severe( e4result, E4_F4SEQ_WRITE ) ;
      #endif
      memcpy( seq_write->buffer + ( seq_write->working - seq_write->avail ), buffer, seq_write->avail ) ;
      seq_write->avail = 0 ;

      return file4seq_write( seq_write, (char *)buffer + first_len, ptr_len - first_len ) ;
   }
}

int S4FUNCTION file4seq_write_flush( FILE4SEQ_WRITE *seq_write )
{
   unsigned to_write ;

   #ifdef S4DEBUG
      if ( seq_write == 0 )
         e4severe( e4parm, E4_F4SEQ_WRT_FLSH ) ;
   #endif

   if ( seq_write->file->code_base->error_code < 0 )
      return -1 ;
   if ( seq_write->buffer == 0 )
      return 0 ;

   #ifdef S4DEBUG
      if( seq_write->pos < 0 )
         e4severe( e4result, (char *) 0 ) ;
   #endif

   to_write = seq_write->working - seq_write->avail ;

   #ifndef S4OPTIMIZE_OFF
      if ( seq_write->file->do_buffer == 1 && seq_write->file->buffer_writes == 1 )
      {
         if ( file4write( seq_write->file, seq_write->pos, seq_write->buffer, to_write ) != 0 )
            return e4( seq_write->file->code_base, e4write, seq_write->file->name ) ;
      }
      else
   #endif
   #ifdef S4WINDOWS
      {
         if ( _llseek( seq_write->file->hand, seq_write->pos, 0 ) != seq_write->pos )
            return e4( seq_write->file->code_base, e4write, seq_write->file->name ) ;
         if ( (unsigned)_lwrite( seq_write->file->hand, seq_write->buffer, to_write) != to_write)
            return e4( seq_write->file->code_base, e4write, seq_write->file->name ) ;
      }
   #else
      {
         if ( lseek( seq_write->file->hand, seq_write->pos, 0 ) != seq_write->pos )
            return e4( seq_write->file->code_base, e4write, seq_write->file->name ) ;
         if ( (unsigned)write( seq_write->file->hand, seq_write->buffer, to_write) != to_write)
            return e4( seq_write->file->code_base, e4write, seq_write->file->name ) ;
      }
   #endif

   seq_write->pos += to_write ;

   seq_write->avail = seq_write->working = seq_write->total ;
   return 0 ;
}

/* note that the sequential functions are not generally useful when hashing is in place since hashing effectively performs bufferring */
void  S4FUNCTION file4seq_write_init( FILE4SEQ_WRITE *seq_write, FILE4 *file, long start_offset, void *ptr, unsigned ptr_len )
{
   #ifdef S4DEBUG
      if ( seq_write == 0 || file == 0 || start_offset < 0 || ( ptr == 0 && ptr_len ) )
         e4severe( e4parm, E4_F4SEQ_WRT_INIT ) ;
   #endif

   memset( (void *)seq_write, 0, sizeof( FILE4SEQ_WRITE ) ) ;

   #ifndef S4OPTIMIZE_OFF
      file4low_flush( file, 1 ) ;
   #endif

   seq_write->file = file ;

   if ( ptr_len > 0 )
   {
      seq_write->total = ptr_len & 0xFC00 ;  /* Make it a multiple of 1K */
      seq_write->buffer = (char *)ptr ;
      if ( seq_write->total > (unsigned)( start_offset % 0x400 ) )
         seq_write->avail = seq_write->working = seq_write->total - (unsigned)( start_offset % 0x400 ) ;
   }
   seq_write->pos = start_offset ;
}

int  S4FUNCTION file4seq_write_repeat( FILE4SEQ_WRITE *seq_write, long num_repeat, char ch )
{
   char buf[512] ;

   #ifdef S4DEBUG
      if ( seq_write == 0 || num_repeat < 0 )
         e4severe( e4parm, E4_F4SEQ_WRT_REP ) ;
   #endif

   memset( (void *)buf, ch, sizeof(buf) ) ;

   while ( num_repeat > sizeof(buf) )
   {
      if ( file4seq_write( seq_write, buf, (unsigned)sizeof( buf ) ) < 0 )
         return -1 ;
      num_repeat -= sizeof(buf) ;
   }

   return file4seq_write( seq_write, buf, (unsigned)num_repeat ) ;
}
