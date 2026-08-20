/* i4init.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef N4OTHER

#ifndef S4FOX
int S4CALL t4cmp_doub( S4CMP_PARM data_ptr, S4CMP_PARM search_ptr, size_t len )
{
   double dif ;
   #ifdef S4PORTABLE
      double d1, d2 ;
      memcpy( (void *)&d1, data_ptr, sizeof(double) ) ;
      memcpy( (void *)&d2, search_ptr, sizeof(double) ) ;
      dif = d1 - d2 ;
   #else
      dif = *((double *)data_ptr) - *((double *)search_ptr) ;
   #endif

   if ( dif > E4ACCURACY )
      return r4after ;
   if ( dif < -E4ACCURACY )
      return -1 ;
   return r4success ;
}

int S4CALL t4desc_cmp_doub( S4CMP_PARM data_ptr, S4CMP_PARM search_ptr, size_t len )
{
   return -1 * t4cmp_doub( data_ptr, search_ptr, 0 ) ;
}

int S4CALL t4desc_bcd_cmp( S4CMP_PARM data_ptr, S4CMP_PARM search_ptr, size_t len )
{
   return -1 * c4bcd_cmp( data_ptr, search_ptr, 0 ) ;
}

int S4CALL t4desc_memcmp( S4CMP_PARM data_ptr, S4CMP_PARM search_ptr, size_t len )
{
   return -1 * u4memcmp( data_ptr, search_ptr, len ) ;
}

void t4str_to_date_mdx( char *result, char *input, int dummy )
{
   double d ;
   d = (double) date4long(input) ;
   memcpy( result, (void *)&d, sizeof(double) ) ;
}

void t4no_change_double( char *result, double d )
{
   memcpy( result, (void *)&d, sizeof(double) ) ;
}

void t4no_change_str( char *a, char *b, int l)
{
   memcpy(a,b,l) ;
}

#ifndef S4INDEX_OFF
int S4FUNCTION t4init( TAG4 *t4, INDEX4 *i4, T4DESC *tag_info )
{
   CODE4 *c4 ;
   FILE4SEQ_READ seqread ;
   char buffer[1024], garbage_buffer[518], expr_buf[221], *ptr ;
   int len ;

   #ifdef S4DEBUG
      if ( i4 == 0 || t4 == 0 || tag_info == 0 )
         e4severe( e4parm, E4_T4INIT ) ;
   #endif

   c4 = i4->code_base ;
   if ( c4->error_code < 0 )
      return -1 ;

   t4->index = i4 ;
   t4->code_base = c4 ;
   t4->unique_error =  c4->default_unique_error ;
   t4->cmp = u4memcmp ;

   t4->header_offset = tag_info->header_pos * 512 ;

   file4seq_read_init( &seqread, &i4->file, t4->header_offset, buffer, sizeof(buffer) ) ;
   if ( file4seq_read_all( &seqread, &t4->header, sizeof(T4HEADER)) < 0 )
      return -1 ;

   #ifdef S4BYTE_SWAP
      t4->header.key_len = x4reverse_short( t4->header.key_len ) ;
      t4->header.keys_max = x4reverse_short( t4->header.keys_max ) ;
      t4->header.group_len = x4reverse_short( t4->header.group_len ) ;
      t4->header.unique = x4reverse_short( t4->header.unique ) ;
   #endif

   t4->header.root = -1 ;

   u4ncpy( t4->alias, tag_info->tag, sizeof(t4->alias) ) ;
   c4trim_n( t4->alias, sizeof(t4->alias) ) ;
   c4upper( t4->alias ) ;

   file4seq_read_all( &seqread, expr_buf, sizeof(expr_buf)-1 ) ;
   c4trim_n( expr_buf, sizeof(expr_buf) ) ;
   t4->expr = expr4parse( i4->data, expr_buf ) ;
   if ( !t4->expr )
      return -1 ;

   len = expr4key_len( t4->expr ) ;
   if ( len < 0 )
      return -1 ;

   if ( t4->header.key_len != (short)len )
   {
      #ifdef S4DEBUG_DEV
         e4describe( c4, e4index, i4->file.name, "t4init()", "Expression length doesn't match tag length" ) ;
      #endif
      return e4( c4, e4index, i4->file.name ) ;
   }
   t4init_seek_conv( t4, t4->header.type ) ;

   file4seq_read_all( &seqread, garbage_buffer, sizeof(garbage_buffer) ) ;

   file4seq_read_all( &seqread, expr_buf, sizeof(expr_buf)-1 ) ;
   c4trim_n( expr_buf, sizeof(expr_buf) ) ;

   if ( garbage_buffer[1] == 1 )   /* Q&E support ... has filter */
   {
      if ( expr_buf[0] != 0 )
      {
         if ( garbage_buffer[2] == 1 )
            t4->has_keys = t4->had_keys = 1 ;
         else
            t4->has_keys = t4->had_keys = 0 ;

         t4->filter = expr4parse( i4->data, expr_buf ) ;
         if ( t4->filter == 0 )
            return -1 ;
         len = expr4key( t4->filter, &ptr ) ;
         if ( len < 0 )
            return -1 ;
         if ( expr4type( t4->filter ) != 'L' )
         {
            #ifdef S4DEBUG_DEV
               e4describe( c4, e4index, i4->file.name, "t4init()", "Filter type not expected logical" ) ;
            #endif
            return e4describe( c4, e4index, E4_INDEX_FIL, i4->file.name, (char *) 0 ) ;
         }
      }
   }
   return 0 ;
}

void S4FUNCTION t4init_seek_conv( TAG4 *t4, int key_type )
{
   int is_desc ;

   is_desc = t4->header.type_code & 8 ;

   switch( key_type )
   {
      case r4num:
         if ( is_desc )
            t4->cmp = t4desc_bcd_cmp ;
         else
            t4->cmp = c4bcd_cmp ;

         t4->stok = c4bcd_from_a ;
         t4->dtok = c4bcd_from_d ;
         break ;

      case r4date:
         if ( is_desc )
            t4->cmp = t4desc_cmp_doub ;
         else
            t4->cmp = t4cmp_doub ;
         t4->stok = t4str_to_date_mdx ;
         t4->dtok = t4no_change_double ;
         break ;

      case r4str:
         if ( is_desc )
            t4->cmp = t4desc_memcmp ;
         else
            t4->cmp = u4memcmp ;
         t4->stok = t4no_change_str ;
         t4->dtok = 0 ;
         break ;

      default:
         #ifdef S4DEBUG
            e4severe( e4info, E4_INFO_INV ) ;
         #else
            e4( t4->code_base, e4info, E4_INFO_INV ) ;
         #endif
   }
   #ifdef S4UNIX
      switch( key_type )
      {
         case r4num:
            t4->key_type = r4num ;
            break ;
   
         case r4date:
            t4->key_type = r4date ;
            break ;
   
         case r4str:
            t4->key_type = r4str ;
            break ;
      }
   #endif
}
#endif  /* ifndef S4INDEX_OFF */
#endif  /* ifndef S4FOX */

#ifdef S4FOX
#ifndef S4INDEX_OFF

#ifdef S4LANGUAGE 
   extern unsigned char v4map[256];
#else
   #ifdef S4ANSI
      extern unsigned char v4map[256];
   #endif
#endif

int S4CALL t4cdx_cmp( S4CMP_PARM data_ptr, S4CMP_PARM search_ptr, size_t len )
{
   unsigned char *data = (unsigned char *)data_ptr ;
   unsigned char *search = (unsigned char *)search_ptr ;
   unsigned on ;

   for( on = 0 ; on < len ; on++ )
   {
      if ( data[on] != search[on] )
      {
         #ifdef S4LANGUAGE 
            if ( v4map[data[on]] > v4map[search[on]] ) return -1 ;  /* gone too far */
         #else
            #ifdef S4ANSI
               if ( v4map[data[on]] > v4map[search[on]] ) return -1 ;  /* gone too far */
            #else
               if ( data[on] > search[on] ) return -1 ;  /* gone too far */
            #endif
         #endif
         break ;
      }
   }

   return on ;
}

void t4no_change_str( char *a, char *b, int l )
{
   memcpy(a,b,l) ;
}

void t4str_to_log( char *a, char *b, int l )
{
   int pos = 0 ;

   for ( ; l != pos ; pos++ )
      switch( a[pos] )
      {
         case 't':
         case 'T':
         case 'y':
         case 'Y':
            b[0] = 'T' ;
            return ;
         case 'f':
         case 'F':
         case 'n':
         case 'N':
            b[0] = 'F' ;
            return ;
         default:
            break ;
      }

   b[0] = 'F' ;
}

int S4FUNCTION t4init( TAG4 *t4, INDEX4 *i4, long file_pos, char *name )
{
   CODE4 *c4 ;
   char expr_buf[221], *ptr ;
   int len ;
   char top_size ;

   #ifdef S4DEBUG
      if ( i4 == 0 || t4 == 0 || name == 0 || file_pos < 0 )
         e4severe( e4parm, E4_T4INIT ) ;
   #endif

   c4 = i4->code_base ;
   if ( c4->error_code < 0 )
      return -1 ;

   t4->index = i4 ;
   t4->code_base = c4 ;
   t4->unique_error =  c4->default_unique_error ;
   t4->header_offset = file_pos ;
   t4->header.root = -1 ;
   t4->cmp = t4cdx_cmp ;

   top_size = 2 * sizeof(long) + 4*sizeof(char) + sizeof(short) + 2 * sizeof(unsigned char) ;
   if ( file4read_all( &i4->file, file_pos, &t4->header, top_size ) < 0 )
      return 0 ;
   if ( file4read_all( &i4->file, file_pos + (long)top_size + 486L, &t4->header.descending,
        ( 3 * sizeof(char) + 3 * sizeof(short) ) ) < 0 )
       return 0 ;

   u4ncpy( t4->alias, name, sizeof(t4->alias) ) ;
   c4trim_n( t4->alias, sizeof(t4->alias) ) ;
   c4upper( t4->alias ) ;

   if ( t4->header.type_code < 0x80 )  /* non-compound header; so expression */
   {
      #ifdef S4DEBUG
         if ( t4->header.expr_len+1 > sizeof( expr_buf ) )
             e4severe( e4info, E4_INFO_EXP ) ;
      #endif
      file4read_all( &i4->file, file_pos+B4BLOCK_SIZE, expr_buf, t4->header.expr_len ) ;
      expr_buf[t4->header.expr_len] = '\0' ;
      t4->expr = expr4parse( i4->data, expr_buf ) ;
      if ( t4->expr == 0 )
         return -1 ;

      len = expr4key_len( t4->expr ) ;
      if ( len < 0 )
         return -1 ;

      if ( t4->header.key_len != len )
      {
         #ifdef S4DEBUG_DEV
            e4describe( c4, e4index, i4->file.name, "t4init()", "Expression length doesn't match tag length" ) ;
         #endif
         return e4( c4, e4index, i4->file.name ) ;
      }

      t4init_seek_conv(t4, t4->expr->type ) ;

      if ( t4->header.type_code & 0x08 )   /* For clause (filter) exists */
      {
         file4read_all( &i4->file, file_pos+B4BLOCK_SIZE+t4->header.expr_len, expr_buf, t4->header.filter_len ) ;
         expr_buf[t4->header.filter_len] = '\0' ;

         t4->filter = expr4parse( i4->data, expr_buf ) ;
         if ( t4->filter == 0 )
            return -1 ;
         len = expr4key( t4->filter, &ptr ) ;
         if ( len < 0 )
            return -1 ;
         if ( expr4type( t4->filter ) != 'L' )
         {
            #ifdef S4DEBUG_DEV
               e4describe( c4, e4index, i4->file.name, "t4init()", "Filter type not expected logical" ) ;
            #endif
            return e4describe( c4, e4index, E4_INDEX_FIL, i4->file.name, (char *) 0 ) ;
         }
      }
   }
   return 0 ;
}

void S4FUNCTION t4init_seek_conv( TAG4 *t4, int type )
{
   t4->cmp = t4cdx_cmp ;

   switch( type )
   {
      case r4date:
      case r4date_doub:
         t4->stok = t4dtstr_to_fox ;
         t4->dtok = t4dbl_to_fox ;
         t4->p_char = '\0' ;
         break ;
      case r4num:
      case r4num_doub:
         t4->stok = t4str_to_fox ;
         t4->dtok = t4dbl_to_fox ;
         t4->p_char = '\0' ;
         break ;
      case r4str:
         t4->stok = t4no_change_str ;
         t4->dtok = 0 ;
         t4->p_char = ' ' ;
         break ;
      case r4log:
         t4->stok = t4str_to_log ;
         t4->dtok = 0 ;
         break ;
      default:
         #ifdef S4DEBUG
            e4severe( e4info, E4_INFO_INV ) ;
         #else
            e4( t4->code_base, e4info, E4_INFO_INV ) ;
         #endif
   }
   #ifdef S4UNIX
      switch( type )
      {
         case r4num:
         case r4num_doub:
            t4->key_type = r4num ;
            break ;
   
         case r4date:
         case r4date_doub:
            t4->key_type = r4date ;
            break ;
   
         case r4str:
            t4->key_type = r4str ;
            break ;

         case r4log:
            t4->key_type = r4log ;
            break ;
      }
   #endif
}

#endif   /*  ifdef S4FOX  */
#endif   /*  ifndef S4INDEX_OFF  */

#endif   /*  ifndef N4OTHER  */

#ifdef N4OTHER

int S4CALL t4cmp_doub( S4CMP_PARM data_ptr, S4CMP_PARM search_ptr, size_t len )
{
   #ifdef S4PORTABLE
      double d1, d2 ;
      memcpy( &d1, data_ptr, sizeof(double) ) ;
      memcpy( &d2, search_ptr, sizeof(double) ) ;
      double dif = d1 - d2 ;
   #else
      double dif = *((double *)data_ptr) - *((double *)search_ptr) ;
   #endif

   if ( dif > E4ACCURACY )  return r4after ;
   if ( dif < -E4ACCURACY ) return -1 ;
   return r4success ;
}

void  t4str_to_doub( char *result, char *input, int dummy )
{
   double d ;
   d = c4atod( input, strlen( input )) ;
   memcpy( result, &d, sizeof(double) ) ;
}

void t4str_to_date_mdx( char *result, char *input, int dummy )
{
   double d = (double) date4long(input) ;
   memcpy( result, &d, sizeof(double) ) ;
}

void t4no_change_double( char *result, double d )
{
   memcpy( result, &d, sizeof(double) ) ;
}

void t4no_change_str( char *a, char *b, int l)
{
   memcpy(a,b,l) ;
}

#ifdef S4CLIPPER
void  t4str_to_clip( char *result, char *input, int dummy )
{
   memcpy ( result, input, strlen( input ) ) ;
   c4clip( result, strlen( result ) ) ;
}
#endif

void  t4date_doub_to_str( char *result, double d )
{
   long  l ;

   l = (long) d ;
   date4assign( result, l ) ;
}

#ifndef S4INDEX_OFF

void S4FUNCTION t4init_seek_conv( TAG4 *t4, int key_type )
{
   switch( key_type )
   {
      #ifdef S4NDX
         case r4num:
         case r4num_doub:
            t4->cmp = t4cmp_doub ;
            t4->stok = t4str_to_doub ;
            t4->dtok = t4no_change_double ;
            break ;

         case r4date:
         case r4date_doub:
            t4->cmp = t4cmp_doub ;
            t4->stok = t4str_to_date_mdx ;
            t4->dtok = t4no_change_double ;
            break ;
      #else
         case r4date:
         case r4date_doub:
            t4->cmp = u4memcmp ;
            t4->stok = t4no_change_str ;
            t4->dtok = t4date_doub_to_str ;
            break ;

         case r4num:
         case r4num_doub:
            t4->cmp = u4memcmp ;
            t4->stok = t4str_to_clip ;
            t4->dtok = 0 ;
            break ;
      #endif

      case r4str:
         t4->cmp = u4memcmp ;
         t4->stok = t4no_change_str ;
         t4->dtok = 0 ;
         break ;

      default:
         #ifdef S4DEBUG
            e4severe( e4info, E4_INFO_INV ) ;
         #else
            e4( t4->code_base, e4info, E4_INFO_INV ) ;
         #endif
   }
   #ifdef S4UNIX
      switch( key_type )
      {
         case r4num:
         case r4num_doub:
            t4->key_type = r4num ;
            break ;
   
         case r4date:
         case r4date_doub:
            t4->key_type = r4date ;
            break ;
   
         case r4str:
            t4->key_type = r4str ;
            break ;
      }
   #endif
}
#endif   /* N4OTHER */

#endif   /* S4INDEX_OFF */
