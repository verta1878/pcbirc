/* d4data.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

char *S4FUNCTION d4alias( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4ALIAS ) ;
   #endif
   return data->alias ;
}

void S4FUNCTION d4alias_set( DATA4 *data, char *new_alias )
{
   #ifdef S4DEBUG
      if ( data == 0 || new_alias == 0 )
         e4severe( e4parm, E4_D4ALIAS_SET ) ;
   #endif
   c4upper( new_alias ) ;
   u4ncpy( data->alias, new_alias, sizeof( data->alias ) ) ;
}

#ifndef S4OFF_WRITE
void S4FUNCTION d4blank( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4BLANK ) )
         return ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4BLANK ) ;
   #endif

   memset( data->record, ' ', data->record_width ) ;
   data->record_changed = 1 ;
}
#endif

int S4FUNCTION d4bof( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4BOF ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   return data->bof_flag ;
}

int S4FUNCTION d4bottom( DATA4 *data )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
   #endif
   long rec ;
   int  rc ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4BOTTOM ) ;
   #endif

   #ifndef S4INDEX_OFF
      tag = d4tag_selected( data ) ;
      if ( tag == 0 )
      {
   #endif
      rec = d4reccount( data ) ;  /* updates the record, returns -1 if code_base->error_code < 0 */
      if ( rec > 0L )
         return d4go( data, rec ) ;
   #ifndef S4INDEX_OFF
      }
      else
      {
         #ifdef S4OFF_WRITE
            if ( data->code_base->error_code < 0 )
               return -1 ;
         #else
            rc = d4update_record( data, 1 ) ;  /* updates the record, returns -1 if code_base->error_code < 0 */
            if ( rc )
               return rc ;
         #endif
         t4version_check( tag, 0 ) ;
         rc = t4bottom( tag ) ;
         if ( rc )
            return rc ;
         if ( !t4eof(tag) )
            return d4go( data, t4recno( tag ) ) ;
      }
   #endif

   data->bof_flag = 1 ;
   return d4go_eof( data ) ;
}

#ifndef S4OFF_WRITE
void S4FUNCTION d4delete( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4DELETE ) )
         return ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4DELETE ) ;
      if ( data->record[0] != ' ' && data->record[0] != '*' )
         e4severe( e4info, E4_DATA_DEL ) ;
   #endif

   if ( data->record[0] != '*' )
   {
      data->record[0] = '*' ;
      data->record_changed = 1 ;
   }
}
#endif

int S4FUNCTION d4deleted( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4DELETED ) ;
      if ( data->record[0] != ' ' && data->record[0] != '*' )
         e4severe( e4info, E4_DATA_DEL ) ;
   #endif

   return *data->record != ' ' ;
}

int S4FUNCTION d4eof( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4EOF ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   return data->eof_flag ;
}

int S4FUNCTION d4num_fields( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4NUM_FIELDS ) ;
   #endif

   return data->n_fields ;
}

int S4FUNCTION d4read( DATA4 *data, long rec_num, char *ptr )
{
   unsigned len ;

   #ifdef S4DEBUG
      if ( data == 0 || rec_num <= 0 || ptr == 0 )
         e4severe( e4parm, E4_D4READ ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   len = file4read( &data->file, d4record_position( data, rec_num ), ptr, data->record_width ) ;
   if ( data->code_base->error_code < 0 )
      return -1 ;

   if ( len != data->record_width )
      return r4entry ;

   return 0 ;
}

int S4FUNCTION d4read_old( DATA4 *data, long rec_num )
{
   int rc ;

   #ifdef S4DEBUG
      if ( data == 0 || rec_num <= 0 )
         e4severe( e4parm, E4_D4READ_OLD ) ;
   #endif

   if ( data->code_base->error_code < 0 )
      return -1 ;

   if ( rec_num <= 0 )
   {
      data->rec_num_old = rec_num ;
      memset( data->record_old, ' ', data->record_width ) ;
   }

   if ( data->rec_num_old == rec_num )
      return 0 ;

   data->rec_num_old = -1 ;
   #ifndef S4OPTIMIZE_OFF
      /* make sure read from disk unless file locked, etc. */
      if ( data->file.do_buffer )
         data->code_base->opt.force_current = 1 ;
   #endif
   rc = d4read( data, rec_num, data->record_old) ;
   #ifndef S4OPTIMIZE_OFF
      if ( data->file.do_buffer )
         data->code_base->opt.force_current = 0 ;
   #endif
   if ( rc < 0 )
      return -1 ;
   if ( rc > 0 )
      memset( data->record_old, ' ', data->record_width ) ;
   data->rec_num_old = rec_num ;

   return 0 ;
}

#ifndef S4OFF_WRITE
void S4FUNCTION d4recall( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4RECALL ) ;
      if ( data->record[0] != ' ' && data->record[0] != '*' )
         e4severe( e4info, E4_DATA_RECALL ) ;
   #endif

   if ( *data->record != ' ' )
   {
      *data->record = ' ' ;
      data->record_changed = 1 ;
   }
}
#endif

long S4FUNCTION d4reccount( DATA4 *data )
{
   long count ;
   #ifndef S4CLIPPER
      unsigned len ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4RECCOUNT ) ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4RECCOUNT ) )
         return -1L ;
   #endif  /* S4VBASIC */

   if ( data->code_base->error_code < 0 )
      return -1 ;

   if ( data->num_recs >= 0L )
      return data->num_recs ;

   #ifdef S4CLIPPER
      count = file4len( &data->file ) ;
      if ( count < 0L )
         return -1L ;
      count = ( count - data->header_len )/ data->record_width ;
   #else
      len = file4read( &data->file, 4L, &count, sizeof( long ) ) ;
      if ( count < 0L || len != 4 )
         return -1L ;
   #endif

   #ifndef S4SINGLE
      if ( d4lock_test_append( data ) )
   #endif  /* S4SINGLE */
         data->num_recs = count ;

   return count ;
}

long S4FUNCTION d4recno( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4RECNO ) ;
   #endif

   return data->rec_num ;
}

char *S4FUNCTION d4record( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4RECORD ) ;
   #endif

   return data->record ;
}

long S4FUNCTION d4record_position( DATA4 *data, long rec )
{
   #ifdef S4DEBUG
      if ( data == 0 || rec <= 0 )
         e4severe( e4parm, E4_D4RECORD_POS ) ;
   #endif

   return data->header_len + data->record_width * ( rec - 1 ) ;
}

long S4FUNCTION d4record_width( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4RECORD_WIDTH ) ;
   #endif

   return (long)data->record_width ;
}

int S4FUNCTION d4top( DATA4 *data )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag ;
   #endif
   int rc, save_flag ;
   CODE4 *c4 ;

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4TOP ) ;
   #endif

   c4 = data->code_base ;
   if ( c4->error_code < 0 )
      return -1 ;

   #ifndef S4INDEX_OFF
      tag = d4tag_selected( data ) ;

      if ( tag == 0 )
      {
   #endif
      save_flag = c4->go_error ;
      c4->go_error = 0 ;
      rc = d4go( data, 1L ) ;
      c4->go_error = save_flag ;
      if ( rc <= 0 )
         return rc ;

      if ( d4reccount( data ) != 0L )
         return d4go( data ,1L ) ;
   #ifndef S4INDEX_OFF
      }
      else
      {
         #ifndef S4OFF_WRITE
            rc = d4update_record( data, 1 ) ;
            if ( rc )
               return rc ;
         #endif
         t4version_check( tag, 0 ) ;
         rc = t4top( tag ) ;
         if ( rc )
            return rc ;
         if ( !t4eof( tag ) )
            return d4go( data , t4recno( tag ) ) ;
      }
   #endif

   data->bof_flag = 1 ;
   return d4go_eof( data ) ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION d4update_header( DATA4 *data, int do_time_stamp, int do_count )
{
   long pos ;
   unsigned len ;
   #ifdef S4BYTE_SWAP
      DATA4_HEADER_FULL swap;
   #endif  /* S4BYTE_SWAP */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4UPDATE_HDR ) ;
   #endif

   pos = 0L ;
   len = 4 + ( sizeof( long ) ) + ( sizeof( short ) ) ;
   if ( do_time_stamp )
      u4yymmdd( &data->yy ) ;
   else
   {
      pos += 4 ;
      len -= 4 ;
   }

   #ifdef S4DEBUG
      if  ( do_count && ( data->num_recs < 0 || d4lock_test_append( data ) == 0 ) )
         e4severe( e4info, E4_DATA_UPDATE ) ;
   #endif

   if ( !do_count )
      len -= (sizeof( data->num_recs ) + sizeof( data->header_len ) ) ;

   #ifdef S4BYTE_SWAP
      memcpy( (void *)&swap, (void *)&data->version, len ) ;
      swap.num_recs = x4reverse_long( swap.num_recs ) ;
      swap.header_len = x4reverse_short( swap.header_len ) ;
      if ( file4write( &data->file, pos, &swap + pos, len ) < 0 )
         return -1 ;
   #else
      if ( file4write( &data->file, pos, (char *)&data->version + pos, len ) < 0 )
         return -1 ;
   #endif  /* S4BYTE_SWAP */

   data->file_changed = 0 ;
   return 0 ;
}
#endif  /* S4OFF_WRITE */


#ifdef S4VB_DOS

char * d4alias_v( DATA4 *d4 )
{
   return v4str( d4alias(d4) ) ;
}


void d4aliasSet ( DATA4 *d4, char *alias )
{
   d4alias_set( d4, c4str(alias) ) ;
}

#endif
