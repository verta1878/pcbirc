/* f4file.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4TEMP
   #include "t4test.h"
#endif

#include <time.h>

#ifndef S4UNIX
   #ifndef __IBMC__
      #ifndef __TURBOC__
         #include <sys/locking.h>
         #define S4LOCKING
      #endif
      #ifdef __ZTC__
         extern int  errno ;
      #endif
      #ifdef _MSC_VER
         #include <sys/types.h>
         #include <sys/locking.h>
      #endif
      #ifdef __TURBOC__
   /*      extern int cdecl errno ; */
      #endif
   #endif

   #include <sys/stat.h>
   #include <share.h>
#endif

#include <fcntl.h>
#include <errno.h>

#ifdef S4DO_ERRNO
   extern int errno ;
#endif

#ifdef S4NO_FILELENGTH_2BYTE
   #include  <sys/types.h>
   #include  <sys/stat.h>
   
   long filelength( hand )
   int hand  ;
   {
      #ifndef S4UNIX
         force error: this code not supported under DOS
      #endif
      struct stat  *str_stat ;
      long  *filelen ;  
      char dummy[40] ;   /* allocate space for *str_stat to use since 'stat' */
                         /* return from 'fstat()' is larger than the 2-byte  */
                         /* stat defined, due to padding of 4-byte structure */
   
      str_stat = (struct stat *)dummy ;
   
      if (fstat( hand, str_stat ) ) 
         e4severe( e4result, "filelength()" ) ;
   
                         /* because of two-byte pad placed before 'st_size' */
                         /* in 4-byte struct, 2-byte struct must move two   */
                         /* bytes ahead to see true 'st_size' value         */
      filelen = (long *) ( ((char *)&str_stat->st_size) + 2) ;
      return( *filelen ) ;
   }
#else
   #ifdef S4NO_FILELENGTH_4BYTE
      #include  <sys/types.h>
      #include  <sys/stat.h>
      
      long filelength( hand )
      int hand  ;
      {
         struct stat   str_stat ;
      
         if (fstat( hand, &str_stat ) )
            e4severe( e4result, "filelength()" ) ;
      
         return( (long) str_stat.st_size ) ;
      }
   #endif
#endif

#ifdef S4NO_CHSIZE

#define E4MAXLINE 129   /* maximum file path length */

int file4change_size( FILE4 *f4, long size )
{
   char *buffer ;
   char temp_name[E4MAXLINE], file_name[E4MAXLINE] ;
   long buffer_size, filelen, amount_read = 0, extra_size = 0 ;
   CODE4 *c4 ;
   FILE4 temp ;
   FILE4SEQ_WRITE seq_write ;
   FILE4SEQ_READ seq_read ;

   if ( f4->is_read_only )
      return e4( f4->code_base, e4parm, E4_PARM_SIZ ) ;

   filelen = filelength( f4->hand ) ;
   if ( size == filelen )
      return 0 ;

   if ( size > filelen )    /* pad file to increase size */
   {
      file4seq_write_init( &seq_write, f4, filelen, 0, 0 ) ;
      file4seq_write_repeat( &seq_write, size - filelen, '\0' ) ;
      return 0 ;
   }

   c4 = f4->code_base ;

   memcpy( file_name, f4->name, E4MAXLINE ) ;
   u4name_piece( temp_name, E4MAXLINE, f4->name, 1, 0 ) ;
   u4name_ext( temp_name, E4MAXLINE, "TMP", 1 ) ;

   if ( file4create( &temp, c4, temp_name, 0 ) < 0 )
   {
      e4( c4, e4create, temp_name ) ;
      return -1 ;
   }

   buffer_size = c4->mem_size_buffer ;
   buffer = (char *)u4alloc_free( c4, buffer_size ) ;

   while ( !buffer )
   {
      buffer_size -= 0x400 ;

      if ( buffer_size <= 0 )
      {
         e4( c4, e4memory, 0 ) ;
         return -1 ;
      }

      buffer = (char *)u4alloc_free( c4, buffer_size ) ;
   }

   file4seq_write_init( &seq_write, &temp, 0, 0, buffer_size ) ;
   file4seq_read_init( &seq_read, f4, 0, 0, buffer_size ) ;

       /* copy the old file contents to the temp file */
   while ( size > 0 )
   {
      if ( size < buffer_size )
         amount_read = file4seq_read( &seq_read, buffer, size ) ;
      else
         amount_read = file4seq_read( &seq_read, buffer, buffer_size ) ;

      file4seq_write( &seq_write, buffer, amount_read ) ;
      size -= amount_read ;
   }

   file4seq_write_flush( &seq_write ) ;

   u4free( buffer ) ;

   file4close( f4 ) ;
   file4close( &temp ) ;

   if ( u4remove( file_name ) )
   {
      e4( c4, e4info, file_name ) ;
      return -1 ;
   }

   if ( u4rename( temp_name, file_name ) )
   {
      e4( c4, e4rename, temp_name ) ;
      return -1 ;
   }

   if ( file4open( f4, c4, file_name, 1 ) < 0 )
   {
      e4( c4, e4open, file_name ) ;
      return -1 ;
   }

   if ( file4lock( f4, L4LOCK_POS, L4LOCK_POS ) < 0 )
   {
      e4( c4, e4lock, file_name ) ;
      return -1 ;
   }

   return 0 ;
}
#endif

long S4FUNCTION file4len( FILE4 *file )
{
   long lrc ;

   #ifdef S4DEBUG
      if ( file == 0 )
         e4severe( e4parm, E4_FILE4LEN ) ;
      if ( file->hand < 0 )
         e4severe( e4parm, E4_FILE4LEN ) ;
   #endif

   #ifndef S4OPTIMIZE_OFF
      if ( file->is_temp == 1 && file->file_created == 0 && file->len == -1 )
         return 0 ;
      if ( file->do_buffer && file->len >= 0 )
         lrc = file->len ;
      else
   #endif
   lrc = filelength( file->hand ) ;
   if ( lrc < 0L )
      e4( file->code_base, e4len, file->name ) ;
   return lrc ;
}

int S4FUNCTION file4len_set( FILE4 *file, long new_len )
{
   int rc ;

   #ifdef S4DEBUG
      if ( file == 0 || new_len < 0 )
         e4severe( e4parm, E4_FILE4LEN_SET ) ;
      if ( file->hand < 0 )
         e4severe( e4parm, E4_FILE4LEN_SET ) ;
   #endif

   if ( file->code_base->error_code < 0 )
      return -1 ;

   if ( file->is_read_only )
      return e4( file->code_base, e4parm, E4_PARM_SIZ ) ;

   #ifndef S4OPTIMIZE_OFF
      if ( file->do_buffer )
      {
         if ( file->len > new_len )   /* must do a partial delete of memory */
            opt4file_delete( file, new_len, file->len ) ;
         file->len = new_len ;
      }
      #ifndef S4SAFE
         if ( file->do_buffer == 0 || file->buffer_writes == 0 )
      #endif
   #endif

   #ifdef S4NO_CHSIZE
      rc = file4change_size( file, new_len ) ;
   #else
      rc = chsize( file->hand, new_len ) ;
   #endif
   if ( rc < 0 )
   {
      e4describe( file->code_base, e4len_set, E4_CREATE_FIL, file->name, (char *)0 ) ;
      return -1 ;
   }

   return 0 ;
}

unsigned S4FUNCTION file4read( FILE4 *file, long pos, void *ptr, unsigned ptr_len )
{
   long rc ;
   unsigned urc ;

   #ifdef S4DEBUG
      if ( file == 0 || pos < 0 || ptr == 0  )
         e4severe( e4parm, E4_F4READ ) ;
      if ( file->hand < 0 )
         e4severe( e4parm, E4_F4READ ) ;
   #endif

   if ( file->code_base->error_code < 0 )
      return 0 ;

   #ifndef S4OPTIMIZE_OFF
      if ( file->do_buffer )
         urc = (unsigned)opt4file_read( file, pos, ptr, ptr_len )  ;
      else
      {
   #endif

      #ifdef S4WINDOWS
         rc = _llseek( file->hand, pos, 0 ) ;
      #else
         rc = lseek( file->hand, pos, 0 ) ;
      #endif

      if ( rc != pos )
      {
         file4read_error( file ) ;
         return 0 ;
      }

      #ifdef S4WINDOWS
         urc = (unsigned)_lread( file->hand, (char *)ptr, ptr_len ) ;
      #else
         urc = (unsigned)read( file->hand, ptr, ptr_len ) ;
      #endif

   #ifndef S4OPTIMIZE_OFF
      }
   #endif

   if ( urc != ptr_len )
      if ( urc > ptr_len )
      {
         file4read_error( file ) ;
         return 0 ;
      }

   return urc ;
}

int  S4FUNCTION file4read_all( FILE4 *file, long pos, void *ptr, unsigned ptr_len )
{
   long rc ;
   unsigned urc ;

   #ifdef S4DEBUG
      if ( file == 0 || pos < 0 || ptr == 0  )
         e4severe( e4parm, E4_F4READ_ALL ) ;
      if ( file->hand < 0 )
         e4severe( e4parm, E4_F4READ_ALL ) ;
   #endif

   if ( file->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4OPTIMIZE_OFF
      if ( file->do_buffer )
         urc = (unsigned) opt4file_read( file, pos, ptr, ptr_len )  ;
      else
      {
   #endif

      #ifdef S4WINDOWS
         rc = _llseek( file->hand, pos, 0 ) ;
      #else
         rc = lseek( file->hand, pos, 0 ) ;
      #endif
      if ( rc != pos )
         return file4read_error( file ) ;

      #ifdef S4WINDOWS
         urc = (unsigned)_lread( file->hand, (char *)ptr, ptr_len ) ;
      #else
         urc = (unsigned)read( file->hand, ptr, ptr_len ) ;
      #endif

   #ifndef S4OPTIMIZE_OFF
      }
   #endif

   if ( urc != ptr_len )
      return file4read_error( file ) ;

   return 0 ;
}

int S4FUNCTION file4read_error( FILE4 *file )
{
   return e4describe( file->code_base, e4read, E4_CREATE_FIL, file->name, (char *) 0 ) ;
}

int S4FUNCTION file4replace( FILE4 *keep, FILE4 *from )
{
   #ifdef S4NO_RENAME
      return e4( from->code_base, e4not_rename, E4_F4REPLACE ) ;
   #else
      FILE4 tmp ;
      int from_do_alloc_free, rc ;
      char *from_name ;
      #ifndef S4SINGLE
         char *buf ;
         unsigned buf_size ;
         #ifndef S4OPTIMIZE_OFF
            int has_opt ;
         #endif
         long pos, f_len ;
      #endif

      #ifdef S4DEBUG
         if ( keep == 0 || from == 0  )
            e4severe( e4parm, E4_F4REPLACE ) ;
      #endif

      rc = 0 ;
      if ( from->is_read_only || keep->is_read_only )
         return e4( from->code_base, e4parm, E4_PARM_REP ) ;

      #ifndef S4SINGLE
         if ( keep->is_exclusive )
         {
      #endif
         memcpy( &tmp, keep, sizeof ( FILE4 ) ) ;  /* remember settings */

         keep->hand = from->hand ;
         from->hand = tmp.hand ;
         from_do_alloc_free = from->do_alloc_free ;
         keep->do_alloc_free = 0 ;
         from->do_alloc_free = 0 ;
         from_name = from->name ;
         from->name = keep->name ;
         from->is_temp = 1 ;
         if ( file4close ( from ) )
            return -1 ;
         if ( file4close ( keep ) )
            return -1 ;

         if ( rename( from_name, tmp.name ) < 0 )
            return - 1 ;

         if ( from_do_alloc_free )
            u4free( from_name ) ;
         if ( file4open ( keep, tmp.code_base, tmp.name, 0 ) )
            return -1 ;
         keep->is_temp = tmp.is_temp ;
         keep->do_alloc_free = tmp.do_alloc_free ;
      #ifndef S4SINGLE
         }
         else  /* can't lose the file handle if other user's have the file open, so just do a copy */
         {
            file4len_set( keep, 0 ) ;

            buf_size = from->code_base->mem_size_buffer ;

            #ifndef S4OPTIMIZE_OFF
               has_opt = from->code_base->has_opt && from->code_base->opt.num_buffers ;
               d4opt_suspend( from->code_base ) ;
            #endif

            for ( ;; buf_size -= 0x800 )
            {
               if ( buf_size < 0x800 )  /* make one last try */
               {
                  buf_size = 100 ;
                  buf = (char *)u4alloc_er( from->code_base, buf_size ) ;
                  if ( buf == 0 )
                     return -1 ;
               }
               buf = (char *)u4alloc( buf_size ) ;
               if ( buf )
                  break ;
            }

            pos = 0 ;
            for( f_len = file4len( from ) ; f_len > 0 ; f_len -= buf_size )
            {
               buf_size = ((long)buf_size > f_len) ?  (unsigned)f_len : buf_size ;
               if ( file4read_all( from, pos, buf, (unsigned)buf_size ) < 0 )
               {
                  rc = -1 ;
                  break ;
               }
               if ( file4write( keep, pos, buf, (unsigned)buf_size ) < 0 )
               {
                  rc = -1 ;
                  break ;
               }
               pos += buf_size ;
            }
            from->is_temp = 1 ;
            file4close( from ) ;
            u4free( buf ) ;
            #ifndef S4OPTIMIZE_OFF
               if ( has_opt )
                  d4opt_start( keep->code_base ) ;
            #endif
         }
      #endif
      return rc ;
   #endif
}

