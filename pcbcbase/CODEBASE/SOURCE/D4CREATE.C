/* d4create.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#ifndef S4OFF_WRITE

DATA4 *S4FUNCTION d4create( CODE4 *c4, char *name, FIELD4INFO *field_data, TAG4INFO *tag_info )
{
   unsigned n_flds ;
   int      is_memo, i ;
   long     calc_record_len, lheader_len ;
   char     buf[258], buffer[0x800] ;
   FILE4SEQ_WRITE seq_write ;
   DATA4  *d4 ;
   DATA4HEADER_FULL create_header ;
   FIELD4IMAGE   create_field_image ;
   FILE4   file ;
   MEMO4FILE m4file ;
   #ifdef S4DEBUG
      int  len, dec ;
   #endif  /* S4DEBUG */

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E4_D4CREATE ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( c4 == 0 || name == 0 || field_data == 0 )
         e4severe( e4parm, E4_D4CREATE ) ;

      u4name_piece( buf, sizeof( buf ), name, 0, 0 ) ;
      if ( d4data( c4, buf ) )
      {
         e4( c4, e4info, E4_INFO_DAO ) ;
         return 0 ;
      }
   #endif

   if ( c4->error_code < 0 )
      return 0 ;

   is_memo = 0 ;
   calc_record_len = 1L ;
   n_flds = 0 ;

   for ( ; field_data[n_flds].name ; n_flds++ )
   {
      #ifdef S4DEBUG
         if ( (field_data[n_flds].type > '\116') || (field_data[n_flds].type < '\103') )
            e4( c4, e4field_type, field_data[n_flds].name ) ;
      #endif  /* S4DEBUG */
      if ( field_data[n_flds].type == 'M' || field_data[n_flds].type == 'G' )
         is_memo = 1 ;
      switch( field_data[n_flds].type )
      {
         case 'N':
         case 'F':
         case 'C':
            calc_record_len += field_data[n_flds].len ;
            break ;
         case 'M':
         case 'G':
            #ifdef S4MEMO_OFF
               #ifdef S4DEBUG
                  e4severe( e4not_memo, E4_D4CREATE ) ;
               #endif
            #else
               calc_record_len += 10 ;
               break ;
            #endif
         case 'D':
            calc_record_len += 8 ;
            break ;
         case 'L':
            calc_record_len += 1 ;
            break ;
         default:
            #ifdef S4DEBUG
               e4severe( e4data, E4_DATA_ILL ) ;
            #endif
            break ;
      }
   }

   if ( calc_record_len >= USHRT_MAX ) /* Increment for deletion flag. */
   {
      e4( c4, e4record_len, name ) ;
      return 0 ;
   }

   lheader_len = (long)n_flds * 32 + 34 ;
   if ( lheader_len >= USHRT_MAX )
   {
      e4describe( c4, e4create, E4_CREATE_TOO, name, 0 ) ;
      return 0 ;
   }

   u4ncpy( buf, name, sizeof(buf) ) ;
   u4name_ext( buf, sizeof(buf), "DBF", 0 ) ;
   c4upper( buf ) ;

   if ( file4create( &file, c4, buf, 1 ) )
      return 0 ;

   file4seq_write_init( &seq_write, &file, 0L, buffer, sizeof(buffer) ) ;

   /* Write the header */
   memset( (void *)&create_header, 0, sizeof(create_header) ) ;
   if ( is_memo )
      #ifdef S4MFOX
         create_header.version = (char) 0xF5 ;
      #endif  /* S4MFOX */
      #ifdef S4MNDX
         create_header.version = (char) 0x83 ;
      #endif  /* S4MNDX */
      #ifdef S4MMDX
         create_header.version = (char) 0x8B ;
      #endif  /* S4MMDX */
   else
      create_header.version = (char) 0x03 ;

   u4yymmdd( &create_header.yy ) ;
   create_header.header_len = (unsigned short) (32*(n_flds+1) + 1) ;
   create_header.record_len = (unsigned short) calc_record_len ;

   #ifdef S4BYTE_SWAP
      create_header.num_recs = x4reverse_long( create_header.num_recs ) ;
      create_header.header_len = x4reverse_short( create_header.header_len ) ;
      create_header.record_len = x4reverse_short( create_header.record_len ) ;
/*      create_header.has_mdx = x4reverse_short( create_header.has_mdx ) ; */
   #endif  /* S4BYTE_SWAP */

   file4seq_write( &seq_write, (char *) &create_header, sizeof(create_header) ) ;

   for ( i = 0; i < (int) n_flds; i++ )
   {
      memset( (void *)&create_field_image, 0, sizeof(create_field_image) ) ;
      u4ncpy( create_field_image.name, field_data[i].name, sizeof(create_field_image.name));
      c4trim_n( create_field_image.name, sizeof(create_field_image.name) ) ;
      c4upper( create_field_image.name ) ;

      create_field_image.type = field_data[i].type ;
      c4upper( &create_field_image.type ) ;

      switch( create_field_image.type )
      {
         case 'C':
            create_field_image.len = (unsigned char)(field_data[i].len & 0xFF) ;
            create_field_image.dec = (unsigned char)(field_data[i].len>>8) ;
            break ;
         #ifndef S4MEMO_OFF
            case 'M':
            case 'G':
               create_field_image.len = 10 ;
               create_field_image.dec = 0 ;
               break ;
         #endif
         case 'D':
            create_field_image.len = 8 ;
            create_field_image.dec = 0 ;
            break ;
         case 'L':
            create_field_image.len = 1 ;
            create_field_image.dec = 0 ;
            break ;
         case 'N':
         case 'F':
            create_field_image.len = (unsigned char) field_data[i].len ;
            create_field_image.dec = (unsigned char) field_data[i].dec ;
            #ifdef S4DEBUG
               len = field_data[i].len ;
               dec = field_data[i].dec ;
               if ( ( len > 19 || len < 1 || len <= 2 && dec != 0 || dec < 0 ) ||
                    ( dec >= len - 1  && dec > 0 ) )
                  e4severe( e4data, E4_DATA_ILL ) ;
            #endif
            break ;
         default:
            #ifdef S4DEBUG
               e4severe( e4data, E4_DATA_ILL ) ;
            #endif
            break ;
      }

      if ( file4seq_write( &seq_write, &create_field_image, sizeof(create_field_image)) < 0 )  break ;
   }

   file4seq_write( &seq_write, "\015\032", 2 ) ;
   file4seq_write_flush( &seq_write ) ;

   file4close( &file ) ;

   #ifndef S4MEMO_OFF
      if ( create_header.version & 0x80 )
      {
         #ifdef S4MFOX
            u4name_ext( buf, sizeof(buf), "FPT", 1 ) ;
         #else
            u4name_ext( buf, sizeof(buf), "DBT", 1 ) ;
         #endif  /* S4MFOX */
         memo4file_create( &m4file, c4, 0, buf) ;
         file4close( &m4file.file ) ;
      }
   #endif

   #ifdef N4OTHER
      i = c4->auto_open ;
      c4->auto_open = 0 ;   /* don't open the index files */
   #endif  /* N4OTHER */
   d4 = d4open( c4, name ) ;
   if ( d4 == 0 )
      return 0 ;
   #ifdef N4OTHER
      c4->auto_open = i ;   /* reset the auto_open flag */
   #endif  /* N4OTHER */

   #ifdef S4INDEX_OFF
      #ifdef S4DEBUG
         if ( tag_info )
            e4severe( e4not_index, E4_D4CREATE ) ;
      #endif
   #else
      if ( tag_info )
      {
         #ifdef N4OTHER
            if ( i4create( d4, d4->file.name, tag_info ) == 0 )
         #else
            if ( i4create( d4, 0, tag_info ) == 0 )
         #endif
            {
               d4close( d4 ) ;
               return 0 ;
            }

         #ifdef S4DEBUG
            if ( c4->index_memory == 0 )
               e4severe( e4data, E4_D4CREATE ) ;
         #endif  /* S4DEBUG */
      }
   #endif
   return d4 ;
}
#endif  /* S4OFF_WRITE */


#ifdef S4VB_DOS

DATA4 *d4create_v ( CODE4 *c4 , char *name, FIELD4INFO *f4, TAG4INFO *t4 )
{
   return d4create( c4, c4str(name), f4, t4 ) ;
}

DATA4 *d4createData ( CODE4 *c4, char *name, FIELD4INFO *f4 )
{
   return d4create( c4, c4str(name), f4, 0 ) ;
}

#endif
