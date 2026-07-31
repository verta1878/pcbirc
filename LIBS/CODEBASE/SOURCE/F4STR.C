/* f4str.c (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

/* Returns a pointer to static string corresponding to the field.
   This string will end in a NULL character.
*/

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4OFF_WRITE
void S4FUNCTION f4assign( FIELD4 *field, char *str )
{
   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4ASSIGN ) )
         return ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 || str == 0 )
         e4severe( e4parm, E4_F4ASSIGN ) ;
   #endif

   f4assign_n( field, str, (unsigned)strlen(str) ) ;
}

void S4FUNCTION f4assign_n( FIELD4 *field, char *ptr, unsigned ptr_len )
{
   char *f_ptr ;

   #ifdef S4DEBUG
      if ( field == 0 || ( ptr == 0 && ptr_len ) )
         e4severe( e4parm, E4_F4ASSIGN_N ) ;
   #endif

   f_ptr = f4assign_ptr( field ) ;

   if ( ptr_len > field->len )
      ptr_len = field->len ;

   /* Copy the data into the record buffer. */
   memcpy( f_ptr, ptr, (size_t)ptr_len ) ;

   /* Make the rest of the field blank. */
   memset( f_ptr + ptr_len, (int)' ', (size_t)( field->len - ptr_len ) ) ;
}
#endif

unsigned S4FUNCTION f4ncpy( FIELD4 *field, char *mem_ptr, unsigned mem_len )
{
   unsigned num_cpy ;

   if ( mem_len == 0 )
      return 0 ;

   #ifdef S4DEBUG
      if ( field == 0 || mem_ptr == 0 )
         e4severe( e4parm, E4_F4NCY ) ;
   #endif

   num_cpy = field->len ;
   if ( mem_len <= num_cpy )
      num_cpy = mem_len - 1 ;

   /* 'f4ptr' returns a pointer to the field within the database record buffer. */
   memcpy( mem_ptr, f4ptr( field ), (size_t)num_cpy ) ;

   mem_ptr[num_cpy] = '\000' ;

   return num_cpy ;
}

char *S4FUNCTION f4str( FIELD4 *field )
{
   CODE4 *code_base ;

   #ifdef S4VBASIC
      if ( c4parm_check( field, 3, E4_F4STR ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( field == 0 )
         e4severe( e4parm, E4_F4STR ) ;
   #endif

   code_base = field->data->code_base ;

   if ( code_base->buf_len <= field->len )   /* not room for field length + null */
      u4alloc_again( code_base, &code_base->field_buffer, &code_base->buf_len, field->len + 1 ) ;
   else
      code_base->field_buffer[field->len] = 0 ;

   memcpy( code_base->field_buffer, f4ptr( field ), field->len ) ;
   return code_base->field_buffer ;
}

#ifdef S4VB_DOS

void f4assign_v( FIELD4 *fld, char *data )
{
   f4assignN ( fld, data, StringLength((char near *)data) ) ;
}

void f4assignN ( FIELD4 *fld, char *data, int len )
{
   char *c_buf;

   if( (c_buf = (char *) u4alloc(len + 1) ) )
   {
      u4vtoc( c_buf, len+1, data ) ;
      f4assign_n( fld, c_buf, len ) ;
      u4free( c_buf );
   }
   else
      e4severe( e4memory, E4_MEMORY_FLD );
}
      
char *f4str_v( FIELD4 *fld )
{
 return v4str( f4str(fld) ) ;
}
      
#endif
