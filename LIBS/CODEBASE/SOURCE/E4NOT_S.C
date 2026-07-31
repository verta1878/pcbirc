/* e4not_s.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

/* contains error stubs for functions not supported under various compile
   switches */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4MEMO_OFF
int S4FUNCTION d4memo_compress( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4MEMO_COMP ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4MEMO_COMP ) ;
   #endif
   return e4( data->code_base, e4not_memo, E4_D4MEMO_COMP ) ;
}
#endif

#ifdef S4OFF_WRITE
void S4FUNCTION d4blank( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4BLANK ) ;
   #endif
   e4( data->code_base, e4not_write, E4_D4BLANK ) ;
}

int S4FUNCTION d4changed( DATA4 *data, int flag )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4CHANGED ) ;
   #endif
   return e4( data->code_base, e4not_write, E4_D4CHANGED ) ;
}

int S4FUNCTION d4append( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4APPEND ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND ) ;
   #endif

   return e4( data->code_base, e4not_write, E4_D4APPEND ) ;
}

int S4FUNCTION d4append_blank( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4APPEND_BL ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND_BL ) ;
   #endif

   return e4( data->code_base, e4not_write, E4_D4APPEND_BL ) ;
}

int S4FUNCTION d4append_data( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND_DATA ) ;
   #endif

   return e4( data->code_base, e4not_write, E4_D4APPEND_DATA ) ;
}

int S4FUNCTION d4append_start( DATA4 *data, int use_memo_entries )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4APPEND_STRT ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4APPEND_STRT ) ;
   #endif

   return e4( data->code_base, e4not_write, E4_D4APPEND_STRT ) ;
}

DATA4 *S4FUNCTION d4create( CODE4 *c4, char *name, FIELD4INFO *field_data, TAG4INFO *tag_info )
{
   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E4_D4CREATE ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( c4 == 0 )
         e4severe( e4parm, E4_D4CREATE ) ;
   #endif

   e4( c4, e4not_write, E4_D4CREATE ) ;
   return 0 ;
}

void S4FUNCTION d4delete( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4DELETE ) )
         return ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4DELETE ) ;
   #endif

   e4( data->code_base, e4not_write, E4_D4DELETE ) ;
}


#ifndef S4MEMO_OFF
int S4FUNCTION d4memo_compress( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4MEMO_COMP ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4MEMO_COMP ) ;
   #endif

   return e4( data->code_base, e4not_write, E4_D4MEMO_COMP ) ;
}
#endif

int S4FUNCTION d4pack( DATA4 *d4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4PACK ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4PACK ) ;
   #endif

   return e4( d4->code_base, e4not_write, E4_D4PACK ) ;
}

int S4FUNCTION d4pack_data( DATA4 *d4 )
{
   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4PACK_DATA ) ;
   #endif

   return e4( d4->code_base, e4not_write, E4_D4PACK_DATA ) ;
}

void S4FUNCTION d4recall( DATA4 *data )
{
   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4RECALL ) ;
   #endif

   e4( data->code_base, e4not_write, E4_D4RECALL ) ;
}

int S4FUNCTION d4reindex( DATA4 *data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( data, 2, E4_D4REINDEX ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( data == 0 )
         e4severe( e4parm, E4_D4REINDEX ) ;
   #endif

   return e4( data->code_base, e4not_write, E4_D4REINDEX ) ;
}

int S4FUNCTION d4write( DATA4 *d4, long rec )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4WRITE ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4WRITE ) ;
   #endif  /* S4DEBUG */

   return e4( d4->code_base, e4not_write, E4_D4WRITE ) ;
}

int S4FUNCTION d4write_data( DATA4 *d4, long rec )
{
   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4WRITE_DATA ) ;
   #endif  /* S4DEBUG */

   return e4( d4->code_base, e4not_write, E4_D4WRITE_DATA ) ;
}

int S4FUNCTION d4write_keys( DATA4 *d4, long rec )
{
   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4WRITE_KEYS ) ;
   #endif  /* S4DEBUG */

   return e4( d4->code_base, e4not_write, E4_D4WRITE_KEYS ) ;
}

int S4FUNCTION d4zap( DATA4 *d4, long r1, long r2 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4ZAP ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4ZAP ) ;
   #endif  /* S4DEBUG */

   return e4( d4->code_base, e4not_write, E4_D4ZAP ) ;
}

int S4FUNCTION d4zap_data( DATA4 *d4, long start_rec, long end_rec )
{
   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4ZAP_DATA ) ;
   #endif  /* S4DEBUG */

   return e4( d4->code_base, e4not_write, E4_D4ZAP_DATA ) ;
}

void S4FUNCTION f4assign( FIELD4 *field, char *str )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN ) ;
}

void S4FUNCTION f4assign_char( FIELD4 *field, int chr )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN_CHAR ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_CHAR ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN_CHAR ) ;
}

void S4FUNCTION f4assign_double( FIELD4 *field, double d_value )
{
   #ifdef S4VBASIC
      if ( c4parm_check ( field, 3, E4_F4ASSIGN_DBL )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_DBL ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN_DBL ) ;
}

void S4FUNCTION f4assign_field( FIELD4 *field_to, FIELD4 *field_from )
{
   #ifdef S4DEBUG
      if ( field_to == 0 )
         e4severe( e4parm, E4_F4ASSIGN_FLD ) ;
   #endif

   e4( field_to->data->code_base, e4not_write, E4_F4ASSIGN_FLD ) ;
}

void S4FUNCTION f4assign_int( FIELD4 *field, int i_value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN_INT ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_INT ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN_INT ) ;
}

void S4FUNCTION f4assign_long( FIELD4 *field, long l_value )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN_LONG ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_LONG ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN_LONG ) ;
}

void S4FUNCTION f4assign_n( FIELD4 *field, char *ptr, unsigned ptr_len )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_N ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN_N ) ;
}

char *S4FUNCTION f4assign_ptr( FIELD4 *field )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4ASSIGN_PTR ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4ASSIGN_PTR ) ;
   return 0 ;
}

void S4FUNCTION f4blank( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4BLANK ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4BLANK ) ;
   #endif

   e4( field->data->code_base, e4not_write, E4_F4BLANK ) ;
}

int S4FUNCTION f4memo_assign( FIELD4 *field, char *ptr )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_ASSIGN ) ;
   #endif

   return e4( field->data->code_base, e4not_write, E4_F4MEMO_ASSIGN ) ;
}

int S4FUNCTION f4memo_assign_n( FIELD4 *field, char *ptr, unsigned ptr_len )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4MEMO_ASS_N ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_ASS_N ) ;
   #endif

   return e4( field->data->code_base, e4not_write, E4_F4MEMO_ASS_N ) ;
}

int S4FUNCTION i4add_tag( INDEX4 *i4, TAG4INFO *tag_data )
{
   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4ADD_TAG ) ;
   #endif

   return e4( i4->code_base, e4not_write, E4_I4ADD_TAG ) ;
}

INDEX4 *S4FUNCTION i4create( DATA4 *d4, char *file_name, TAG4INFO *tag_data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_I4CREATE ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_I4CREATE ) ;
   #endif

   e4( d4->code_base, e4not_write, E4_I4CREATE ) ;
   return 0 ;
}

int S4FUNCTION i4reindex( INDEX4 *i4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4REINDEX ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( i4 == 0  )
         e4severe( e4parm, E4_I4REINDEX ) ;
   #endif

   return e4( i4->code_base, e4not_write, E4_I4REINDEX ) ;
}

int S4FUNCTION t4add( TAG4 *t4, unsigned char *key_info, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4ADD ) ;
   #endif

   return e4( t4->code_base, e4not_write, E4_T4ADD ) ;
}

int S4FUNCTION t4add_calc( TAG4 *t4, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4ADD_CALC ) ;
   #endif

   return e4( t4->code_base, e4not_write, E4_T4ADD_CALC ) ;
}

int S4FUNCTION t4flush( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4FLUSH ) ;
   #endif

   return e4( t4->code_base, e4not_write, E4_T4FLUSH ) ;
}

int S4FUNCTION t4remove( TAG4 *t4, char *ptr, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4REMOVE ) ;
   #endif

   return e4( t4->code_base, e4not_write, E4_T4REMOVE ) ;
}

int S4FUNCTION t4remove_calc( TAG4 *t4, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4REM_CALC ) ;
   #endif

   return e4( t4->code_base, e4not_write, E4_T4REM_CALC ) ;
}

#endif

#ifdef S4INDEX_OFF

int S4FUNCTION d4seek( DATA4 *d4, char *str )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4SEEK ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4SEEK ) ;
   #endif

   return e4( d4->code_base, e4not_write, E4_D4SEEK ) ;
}

int S4FUNCTION d4seek_double( DATA4 *d4, double dkey )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4SEEK_DBL ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4SEEK_DBL ) ;
   #endif

   return e4( d4->code_base, e4not_write, E4_D4SEEK_DBL ) ;
}

int S4FUNCTION i4close( INDEX4 *i4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4CLOSE ) )
         return -1 ;
   #endif

   if ( i4 == 0 )
      return -1 ;

   return e4( i4->code_base, e4not_index, E4_I4CLOSE ) ;
}

#ifndef S4OFF_WRITE
INDEX4 *S4FUNCTION i4create( DATA4 *d4, char *file_name, TAG4INFO *tag_data )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_I4CREATE ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_I4CREATE ) ;
   #endif

   e4( d4->code_base, e4not_index, E4_I4CREATE ) ;
   return 0 ;
}
#endif

int S4FUNCTION i4lock( INDEX4 *i4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4LOCK ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4LOCK ) ;
   #endif

   return e4( i4->code_base, e4not_index, E4_I4LOCK ) ;
}

INDEX4 *S4FUNCTION i4open( DATA4 *d4, char *file_name )
{
   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_I4OPEN ) ;
   #endif
   e4( d4->code_base, e4not_index, E4_I4OPEN ) ;
   return 0 ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION i4reindex( INDEX4 *i4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4REINDEX ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0  )
         e4severe( e4parm, E4_I4REINDEX ) ;
   #endif

   return e4( i4->code_base, e4not_index, E4_I4REINDEX ) ;
}
#endif

TAG4 *S4FUNCTION i4tag( INDEX4 *i4, char *tag_name )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4TAG ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4TAG ) ;
   #endif

   e4( i4->code_base, e4not_index, E4_I4TAG ) ;
   return 0 ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION i4add_tag( INDEX4 *i4, TAG4INFO *tag_data )
{
   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4ADD_TAG ) ;
   #endif

   e4( i4->code_base, e4not_index, E4_I4ADD_TAG ) ;
}
#endif

TAG4INFO *S4FUNCTION i4tag_info( INDEX4 *index )
{
   #ifdef S4DEBUG
      if ( index == 0 )
         e4severe( e4parm, E4_I4TAG_INFO ) ;
   #endif

   e4( index->code_base, e4not_index, E4_I4TAG_INFO ) ;
   return 0 ;
}

int S4FUNCTION i4unlock( INDEX4 *i4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4UNLOCK ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4UNLOCK ) ;
   #endif

   e4( i4->code_base, e4not_index, E4_I4UNLOCK ) ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION t4add( TAG4 *t4, unsigned char *key_info, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4ADD ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4ADD ) ;
}

int S4FUNCTION t4add_calc( TAG4 *t4, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4ADD_CALC ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4ADD_CALC ) ;
}
#endif

int S4FUNCTION t4bottom( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4BOTTOM ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4BOTTOM ) ;
}

int S4FUNCTION t4down( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4DOWN ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4DOWN ) ;
}

int S4FUNCTION t4dump( TAG4 *t4, int out_handle, int display_all )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4DUMP ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4DUMP ) ;
}

int S4FUNCTION t4eof( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4EOF ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4EOF ) ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION t4flush( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4FLUSH ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4FLUSH ) ;
}
#endif

int S4FUNCTION t4free_all( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4FREE_ALL ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4FREE_ALL ) ;
}

int S4FUNCTION t4go( TAG4 *t4, char *ptr, long rec_num )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4GO ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4GO ) ;
}

char S4PTR *S4FUNCTION t4key( TAG4 S4PTR *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4KEY ) ;
   #endif

   e4( t4->code_base, e4not_index, E4_T4KEY ) ;
   return ;
}

TAG4 *S4FUNCTION t4open( DATA4 *d4, INDEX4 *i4ndx, char *file_name )
{
   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_T4OPEN ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_T4OPEN ) ;
   #endif

   e4( d4->code_base, e4not_index, E4_T4OPEN ) ;
   return 0 ;
}

double S4FUNCTION t4position( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4POSITION ) ;
}

int S4FUNCTION t4position_set( TAG4 *t4, double pos )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4POSITION_SET ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4POSITION_SET ) ;
}

long S4FUNCTION t4recno( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4RECNO ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4RECNO ) ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION t4remove( TAG4 *t4, char *ptr, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4REMOVE ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4REMOVE ) ;
}

int S4FUNCTION t4remove_calc( TAG4 *t4, long rec )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4REM_CALC ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4REM_CALC ) ;
}
#endif

int S4FUNCTION t4seek( TAG4 *t4, void *ptr, int len_ptr )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4SEEK ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4SEEK ) ;
}

long S4FUNCTION t4skip( TAG4 *t4, long num_skip )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4SKIP ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4SKIP ) ;
}

int S4FUNCTION t4top( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4TOP ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4TOP ) ;
}

int S4FUNCTION t4up( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4UP ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4UP ) ;
}

int  S4FUNCTION t4up_to_root( TAG4 *t4 )
{
   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4UP_TO_ROOT ) ;
   #endif

   return e4( t4->code_base, e4not_index, E4_T4UP_TO_ROOT ) ;
}

#endif
