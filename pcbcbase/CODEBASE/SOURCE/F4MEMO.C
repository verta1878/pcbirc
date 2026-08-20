/* f4memo.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

extern char f4memo_null_char ;

#ifndef S4OFF_WRITE
int S4FUNCTION f4memo_assign( FIELD4 *field, char *ptr )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_ASSIGN ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   return f4memo_assign_n( field, ptr, (unsigned) strlen(ptr) ) ;
}

int S4FUNCTION f4memo_assign_n( FIELD4 *field, char *ptr, unsigned ptr_len )
{
   #ifndef S4MEMO_OFF
      F4MEMO *mfield ;
      int rc ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4MEMO_ASS_N ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_ASS_N ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4MEMO_OFF
      mfield = field->memo ;
      if ( !mfield )
   #else
      if ( !field->memo )
   #endif
   {
      f4assign_n( field, ptr, ptr_len ) ;
      return 0 ;
   }

   #ifdef S4MEMO_OFF
      return e4( field->data->code_base, e4not_memo, E4_F4MEMO_ASS_N ) ;
   #else
      rc = f4memo_set_len( field, ptr_len ) ;
      if ( rc )
         return  rc ;

      memcpy( mfield->contents, ptr, (size_t)ptr_len ) ;
   #endif
   return 0 ;
}
#endif

#ifdef S4OLD_CODE
int S4FUNCTION f4memo_flush( FIELD4 *field )
{
   #ifdef S4MEMO_OFF
      return 0 ;
   #else
      #ifdef S4DEBUG
         if ( field == 0 )
            e4severe( e4parm, E4_F4MEMO_FLUSH ) ;
      #endif

      return d4flush( field->data ) ;
   #endif
}
#endif

void S4FUNCTION f4memo_free( FIELD4 *field )
{
   #ifdef S4MEMO_OFF
      return ;
   #else
      F4MEMO *mfield ;

      #ifdef S4DEBUG
         if ( field == 0 )
            e4severe( e4parm, E4_F4MEMO_FREE ) ;
      #endif

      mfield = field->memo ;
      if ( !mfield )
         return ;

      if ( mfield->len_max > 0 )
         u4free( mfield->contents ) ;

      mfield->contents = &f4memo_null_char ;

      mfield->status = 1 ;
      mfield->len_max = 0 ;
   #endif
}

unsigned S4FUNCTION f4memo_len( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4MEMO_LEN ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_LEN ) ;
   #endif

   if ( !field->memo )
      return (unsigned)field->len ;

   #ifdef S4MEMO_OFF
      e4( field->data->code_base, e4not_memo, E4_F4MEMO_LEN ) ;
      return 0 ;
   #else
      if ( field->memo->status == 1 )
      {
         if ( f4memo_read( field ) )
            return 0 ;
         field->memo->status = 0 ;
      }
      return field->memo->len ;
   #endif
}

unsigned S4FUNCTION f4memo_ncpy( FIELD4 *field, char *mem_ptr, unsigned len )
{
   #ifndef S4MEMO_OFF
      unsigned num_cpy ;
      CODE4 *c4 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_NCPY ) ;
   #endif

   if ( !field->memo )
      return f4ncpy( field, mem_ptr, len ) ;

   #ifdef S4MEMO_OFF
      return e4( field->data->code_base, e4not_memo, E4_F4MEMO_NCPY ) ;
   #else
      if ( len == 0 )
         return 0 ;

      c4 = field->data->code_base ;
      if ( c4->error_code < 0 )
         return 0 ;
      c4->error_code = 0 ;

      num_cpy = f4memo_len( field ) ;
      if ( len < num_cpy )
         num_cpy = len - 1 ;

      memcpy( mem_ptr, f4memo_ptr( field ), (size_t)num_cpy ) ;

      mem_ptr[num_cpy] = '\000' ;

      return( num_cpy ) ;
   #endif
}

char *S4FUNCTION f4memo_ptr( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4MEMO_PTR ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_PTR ) ;
   #endif

   if ( !field->memo )
      return f4ptr(field) ;

   #ifdef S4MEMO_OFF
      e4( field->data->code_base, e4not_memo, E4_F4MEMO_PTR ) ;
      return (char *)0 ;
   #else
      if ( field->memo->status == 1 )
      {
         if ( f4memo_read( field ) )
            return 0 ;
         field->memo->status = 0 ;
      }
      return field->memo->contents ;
   #endif
}

#ifndef S4MEMO_OFF
int S4FUNCTION f4memo_read( FIELD4 *field )
{
   int rc ;
   F4MEMO *mfield ;

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_READ ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   mfield = field->memo ;
   mfield->is_changed = 0 ;

   if ( d4recno( field->data ) < 0 )
   {
      mfield->len = 0 ;
      return mfield->len ;
   }

   if ( field->data->code_base->read_lock )
   {
      rc = d4validate_memo_ids( field->data ) ;
      if ( rc )
         return rc ;
   }

   if ( f4memo_read_low( field ) )
      return -1 ;

   return 0 ;
}

int S4FUNCTION f4memo_read_low( FIELD4 *field )
{
   F4MEMO *mfield ;
   int rc ;

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_READ_LW ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   mfield = field->memo ;
   #ifdef S4DEBUG
      if ( !mfield )
         e4severe( e4info, E4_INFO_EMF ) ;
   #endif

   mfield->len = mfield->len_max  ;

   rc = memo4file_read( &field->data->memo_file, f4long( field ), &mfield->contents, &mfield->len ) ;

   #ifndef S4MNDX
      if ( mfield->len > mfield->len_max )
   #endif
      mfield->len_max = mfield->len  ;

   if ( mfield->len_max > 0 )
      mfield->contents[mfield->len] = 0 ;
   else
      mfield->contents = &f4memo_null_char ;

   return rc ;
}

void S4FUNCTION f4memo_reset( FIELD4 *field )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_RESET ) ;
   #endif

   field->memo->len = 0 ;
   field->memo->status = 1 ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION f4memo_set_len( FIELD4 *field, unsigned len )
{
   F4MEMO *mfield ;

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_SET_LEN ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   mfield = field->memo ;
   if ( mfield == 0 )
      return 0 ;

   if ( mfield->len_max < len )
   {
      if ( mfield->len_max > 0 )
         u4free( mfield->contents ) ;
      mfield->len_max = len ;

      mfield->contents = (char *)u4alloc_er( field->data->code_base, mfield->len_max + 1 ) ;
      if ( mfield->contents == 0 )
      {
         mfield->len_max = 0 ;
         mfield->status = 1 ;
         return e4memory ;
      }
   }

   mfield->len = len ;

   if ( mfield->len_max == 0 )
      mfield->contents = &f4memo_null_char ;
   else
      mfield->contents[mfield->len] = 0 ;

   mfield->status = 0 ;
   mfield->is_changed = 1 ;
   field->data->record_changed = 1 ;
   return 0 ;
}
#endif  /* S4OFF_WRITE */
#endif  /* S4OFF_MEMO */

char *S4FUNCTION f4memo_str( FIELD4 *field )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4MEMO_STR ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_STR ) ;
   #endif

   if ( field->memo == 0 )
      return f4str( field ) ;
   #ifdef S4MEMO_OFF
      e4( field->data->code_base, e4not_memo, E4_F4MEMO_STR ) ;
      return 0 ;
   #else
      return f4memo_ptr( field ) ;
   #endif
}

#ifndef S4OFF_WRITE
#ifndef S4MEMO_OFF
int  S4FUNCTION f4memo_update( FIELD4 *field )
{
   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_UPDATE ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   if ( field->memo )
      if ( field->memo->is_changed )
         return f4memo_write( field ) ;
   return 0 ;
}

int S4FUNCTION f4memo_write( FIELD4 *field )
{
   int rc ;
   long memo_id, new_id ;

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4MEMO_WRITE ) ;
   #endif

   if ( field->data->code_base->error_code < 0 )
      return -1 ;

   rc = d4validate_memo_ids( field->data ) ;
   if ( rc )
      return rc ;

   memo_id = f4long( field ) ;
   new_id = memo_id ;

   rc = memo4file_write( &field->data->memo_file, &new_id, field->memo->contents, field->memo->len ) ;

   if ( rc )
      return rc ;

   if ( new_id != memo_id )
   {
      if ( new_id )
         f4assign_long( field, new_id ) ;
      else
         f4assign( field, " " ) ;
   }

   field->memo->is_changed = 0 ;
   return 0 ;
}
#endif  /* S4MEMO_OFF */
#endif  /* S4OFF_WRITE */



#ifdef S4VBASIC

long S4FUNCTION f4memoLen( FIELD4 *f4 )
{
   return (long) f4memo_len( f4 );
}

#ifdef S4VB_DOS

int f4memoAssign ( FIELD4 *fld, char *data )
{
   return f4memoAssignN ( fld, data, StringLength( data ) ) ;
}


int f4memoAssignN( FIELD4 *fld, char *data, int len )
{
   char *c_buf ;
   int rc ;

   if( (c_buf = (char *) u4alloc(len + 1) ) )
   {
      u4vtoc( c_buf, len+1, data ) ;
      rc = f4memo_assign_n( fld, c_buf, len ) ;
      u4free( c_buf ) ;
      return rc ;
   }
   else
      e4severe( e4memory, E4_MEMORY_MEMO );
 
}

char * f4memoStr( FIELD4 *fld )
{
   return v4str( f4memo_str(fld) ) ;
}

#endif /* S4VB_DOS */

#endif /* S4VBASIC */
