/* d4open.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4OFF_MEMO
extern char f4memo_null_char ;
#endif

DATA4 * S4FUNCTION d4open( CODE4 *c4, char *name )
{
   int rc, i_fields, i_memo ;
   unsigned field_data_len, count ;
   char name_buf[258], field_buf[2], *info ;
   DATA4 *d4 ;
   DATA4HEADER_FULL full_header ;
   INDEX4 *i4 ;
   FIELD4IMAGE *image ;

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E4_D4OPEN ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( c4 == 0 || name == 0 )
         e4severe( e4parm, E4_D4OPEN ) ;
   #endif

   if ( c4->error_code < 0 )
      return 0 ;

   #ifdef S4DEBUG
      if ( c4->debug_int != 0x5281 )
         e4severe( e4result, E4_RESULT_D4I ) ;

      u4name_piece( name_buf, sizeof( name_buf ), name, 0, 0 ) ;
      if ( d4data( c4, name_buf ) )
      {
         e4( c4, e4info, E4_INFO_DAO ) ;
         return 0 ;
      }
   #endif

   if ( c4->data_memory == 0 )
   {
      c4->data_memory = mem4create( c4, c4->mem_start_data, sizeof(DATA4), c4->mem_expand_data, 0 ) ;
      if ( c4->data_memory == 0 )
      {
         e4( c4, e4memory, 0 ) ;
         return 0 ;
      }
   }
   d4 = (DATA4 *) mem4alloc( c4->data_memory ) ;
   if ( d4 == 0 )
   {
      e4(  c4, e4memory, 0 ) ;
      return 0 ;
   }

   #ifdef S4VBASIC
      d4->debug_int = 0x5281 ;
   #endif
   d4->code_base = c4 ;

   #ifndef S4SINGLE
      d4->locks = &d4->locked_record ;
      d4->n_locks = 1 ;
   #endif
   d4->memo_file.file.hand = -1 ;

   u4ncpy( name_buf, name, sizeof(name_buf) ) ;
   u4name_ext( name_buf, sizeof(name_buf), "DBF", 0 ) ;
   c4upper( name_buf ) ;

   rc = file4open( &d4->file, c4, name_buf, 1 ) ;
   if ( rc )
   {
      d4close( d4 ) ;
      return 0 ;
   }
   l4add( &c4->data_list, &d4->link ) ;

   u4name_piece( d4->alias, sizeof(d4->alias), name_buf, 0,0 ) ;

   if ( file4read_all( &d4->file, 0L, &full_header, sizeof( full_header ) ) < 0 )
   {
      d4close( d4 ) ;
      return 0 ;
   }

   #ifdef S4BYTE_SWAP
      full_header.num_recs = x4reverse_long( full_header.num_recs ) ;
      full_header.header_len = x4reverse_short( full_header.header_len ) ;
      full_header.record_len = x4reverse_short( full_header.record_len ) ;
      full_header.has_mdx = x4reverse_short( full_header.has_mdx ) ;
   #endif

   #ifdef S4DEMO
      if ( full_header.num_recs > 200L)
      {
         e4( c4, e4demo, 0 ) ;
         d4close(d4) ;
         return 0 ;
      }
   #endif

   #ifdef S4DEBUG
      if ( full_header.num_recs < 0L || full_header.num_recs > ( 1 +  ( file4len ( &d4->file )  - full_header.header_len ) / full_header.record_len ) )
         e4severe( e4info, E4_DATA_COR ) ;
   #endif

   memcpy( (void *)&d4->version, (void *)&full_header.version, (4+(sizeof(long))+(sizeof(short))) ) ;

   d4->num_recs = -1L ;
   d4->has_mdx = full_header.has_mdx ;

   field_data_len = full_header.header_len-sizeof(full_header) ;
   if ( full_header.header_len <= sizeof(full_header) )
   {
      e4( c4, e4data, d4->file.name ) ;
      d4close(d4) ;
      return 0 ;
   }

   info = (char *) u4alloc_free( c4, field_data_len ) ;
   if ( info == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      d4close(d4) ;
      return 0 ;
   }

   if ( file4read_all( &d4->file, (long) sizeof(full_header), info,field_data_len) < 0 )
   {
      u4free(info) ;
      e4( c4, e4data, 0 ) ;
      d4close(d4) ;
      return 0 ;
   }

   /* count the number of fields */
   for (count=0; count < field_data_len; count+= 32 )
      if ( info[count] == 0xD )  break ;
   d4->n_fields = (int) (count/32) ;
   if ( field_data_len/32 < (unsigned) d4->n_fields )
   {
      u4free(info) ;
      e4( c4, e4data, d4->file.name ) ;
      d4close(d4) ;
      return 0 ;
   }

   d4->fields = (FIELD4 *) u4alloc_free( c4, sizeof(FIELD4) * (long) d4->n_fields ) ;
   if ( d4->fields == 0 )  e4( c4, e4memory, 0 ) ;

   d4->record_width = 1 ;

   if ( !(c4->error_code < 0)  )
      for ( i_fields = 0; i_fields < d4->n_fields; i_fields++ )
      {
         image = (FIELD4IMAGE *) (info+ i_fields*32) ;
         u4ncpy( d4->fields[i_fields].name, image->name, sizeof(d4->fields->name) ) ;

         u4ncpy( field_buf, &image->type, 2 ) ;
         c4upper( field_buf ) ;
         d4->fields[i_fields].type = *field_buf ;

         if ( d4->fields[i_fields].type == 'N' || d4->fields[i_fields].type == 'F')
         {
            d4->fields[i_fields].len = image->len ;
            d4->fields[i_fields].dec = image->dec ;
         }
         else
         {
           if ( d4->fields[i_fields].type == 'L' || d4->fields[i_fields].type == 'D' || d4->fields[i_fields].type == 'M' || d4->fields[i_fields].type == 'G' )
              d4->fields[i_fields].len = image->len ;
           else
              d4->fields[i_fields].len = image->len + (image->dec << 8) ;
         }

         if ( d4->fields[i_fields].type == 'M' || d4->fields[i_fields].type == 'G' )
         {
            d4->n_fields_memo++ ;
            #ifdef S4MEMO_OFF
               d4->fields[i_fields].memo = &f4memo_null_char ;
            #endif
         }

         #ifdef S4VBASIC
            d4->fields[i_fields].debug_int = 0x5281 ;
         #endif
         d4->fields[i_fields].offset = d4->record_width ;
         d4->record_width += d4->fields[i_fields].len ;
         d4->fields[i_fields].data = d4 ;
      }

   u4free( info ) ;

   #ifndef S4MEMO_OFF
      if ( d4->n_fields_memo > 0  &&  ! (c4->error_code< 0) )
      {
         i_memo = 0 ;

         d4->fields_memo = (F4MEMO *)u4alloc_free( c4, (long)sizeof(F4MEMO) * d4->n_fields_memo ) ;
         if ( d4->fields_memo == 0 )
            e4( c4, e4memory, 0 ) ;
         else
            for ( i_fields = 0; i_fields < d4->n_fields; i_fields++ )
               if ( d4->fields[i_fields].type == 'M' || d4->fields[i_fields].type == 'G' )
               {
                  d4->fields[i_fields].memo = d4->fields_memo+i_memo ;
                  d4->fields_memo[i_memo].status = 1 ;
                  d4->fields_memo[i_memo].field = d4->fields+i_fields ;
                  i_memo++ ;
               }
      }
   #endif

   if ( d4->record_width != full_header.record_len  &&  ! (c4->error_code<0) )
      e4( c4, e4data, d4->file.name ) ;

   if ( c4->error_code < 0 )
   {
      d4close(d4) ;
      return 0 ;
   }

   d4->record = (char *) u4alloc_free( c4, d4->record_width + 1 ) ;
   d4->record_old = (char *) u4alloc_free( c4, d4->record_width+1 ) ;
   if ( d4->record == 0  ||  d4->record_old == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      d4close(d4) ;
      return 0 ;
   }

   memset( d4->record, ' ', d4->record_width ) ;
   memset( d4->record_old, ' ', d4->record_width ) ;

   d4->rec_num = d4->rec_num_old = -1 ;

   #ifndef S4OPTIMIZE_OFF
      file4optimize( &d4->file, c4->optimize, OPT4DBF ) ;
   #endif

   #ifndef S4INDEX_OFF
      #ifdef N4OTHER
         if ( c4->auto_open )
         {
            i4 = i4open( d4, 0 ) ;
            if ( i4 == 0 )
            {
               d4close(d4) ;
               return 0 ;
            }
         }
      #else
         if ( d4->has_mdx && c4->auto_open )
         {
            i4 = i4open( d4, 0 ) ;
            if ( i4 == 0 )
            {
               d4close(d4) ;
               return 0 ;
            }
            #ifdef S4MDX
               if ( !i4->header.is_production )
                  i4close(i4);
            #endif
         }
      #endif
   #endif

   #ifndef S4MEMO_OFF
      if ( d4->version & 0x80 )
      {
         #ifdef S4MFOX
            u4name_ext( name_buf, sizeof(name_buf), "FPT", 1 ) ;
         #else
            u4name_ext( name_buf, sizeof(name_buf), "DBT", 1 ) ;
         #endif
         if ( memo4file_open( &d4->memo_file, d4, name_buf ) < 0 )
         {
            d4close(d4) ;
            return 0 ;
         }
      }
   #endif

   return d4 ;
}

#ifdef S4VB_DOS

DATA4 * d4open_v( CODE4 *c4, char *name )
{
   return d4open( c4, c4str(name) ) ;
}

#endif

