/* e4calc.c (c)Copyright Sequiter Software Inc., 1991-1993. All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef ___TURBOC__
      #pragma hdrstop
   #endif
#endif

void S4FUNCTION expr4calc_delete( EXPR4CALC *calc )
{
   CODE4 *c4 = calc->expr->code_base ;

   if( calc->total != 0 )
   {
      l4remove( &c4->total_list, calc->total ) ;
      mem4free( c4->total_memory, calc->total ) ;
   }
   l4remove( &c4->calc_list, calc ) ;
   expr4free( calc->expr ) ;
   mem4free( c4->calc_memory, calc ) ;
}

void S4FUNCTION expr4calc_reset( CODE4 *c4 ) 
{
   while( c4->calc_list.n_link > 0 )
      expr4calc_delete( (EXPR4CALC *)c4->calc_list.last_node ) ;
}

void S4FUNCTION expr4calc_massage( EXPR4CALC *calc )
{
   EXPR4 *exp4 = calc->expr;

   if( exp4->type == r4num )
   {
      /* Must consist of a single numeric field */
      exp4->type =  r4num_doub ;
      exp4->len =  sizeof(double) ;
      exp4->info->len =  f4len( exp4->info->field_ptr ) ;
      exp4->info->function_i =  E4FIELD_NUM_D ;
      exp4->info->function =  v4functions[E4FIELD_NUM_D].function_ptr ;
   }
}

EXPR4CALC *S4FUNCTION expr4calc_create( CODE4 *c4, EXPR4 *exp4, char *name )
{
   EXPR4CALC *calc_ptr = (EXPR4CALC *) mem4create_alloc( c4, &c4->calc_memory, 5, sizeof(EXPR4CALC), 5, 0 ) ;
   if ( calc_ptr == 0 )
      return 0 ;

   l4add( &c4->calc_list, calc_ptr ) ;
   calc_ptr->expr = exp4 ;
   u4ncpy( calc_ptr->name, name, sizeof(calc_ptr->name) ) ;
   c4upper( calc_ptr->name ) ;

   expr4calc_massage( calc_ptr );
   return calc_ptr ;
}

EXPR4CALC *S4FUNCTION expr4calc_lookup( CODE4 *c4, char *name, unsigned name_len )
{
   EXPR4CALC *calc_on ;
   char buf[sizeof(calc_on->name)] ;

   if ( name_len >= sizeof(calc_on->name) )
      name_len = sizeof(calc_on->name)-1 ;
   u4ncpy( buf, name, name_len+1 ) ;
   c4upper( buf ) ;
   for( calc_on = 0 ;; )
   {
      calc_on = (EXPR4CALC *) l4next( &c4->calc_list, calc_on ) ;
      if ( calc_on == 0 )
         return 0 ;
      if ( strcmp( calc_on->name, buf) == 0 )
         return calc_on ;
   }
}

void S4FUNCTION expr4calc_result_pos( EXPR4CALC *calc_ptr, int new_result_pos )
{
   E4INFO *info ;
   int i, offset = new_result_pos - calc_ptr->cur_result_pos ;
   if ( offset == 0 )
      return ;

   calc_ptr->cur_result_pos = new_result_pos ;

   info = calc_ptr->expr->info ;
   for( i = calc_ptr->expr->info_n; --i >= 0; )
      info[i].result_pos += offset ;
}

#ifndef S4UNIX
/* Expression source must be updated to refect the fact that a calculation had a name change */
/* Caller must ensure names are trimmed in front and back & are upper case */
int S4FUNCTION expr4calc_name_change( EXPR4 **expr_on, char *old_name, char *new_name )
{
   EXPR4 *new_expr ;
   char buf_mem[50] ;
   char *ptr, *buf ;
   unsigned pos, buf_len ;
   int old_name_len, ptr_len, new_name_len, did_alloc, buf_pos, did_change ;

   ptr = expr4source( *expr_on ) ;
   old_name_len = strlen( old_name ) ;
   ptr_len = strlen( ptr ) ;
   buf_len = sizeof( buf_mem ) ;
   buf = buf_mem ;
   did_alloc = buf_pos = did_change = 0 ;

   for( pos = 0; ptr[pos]; pos++)
   {
      buf[buf_pos++] = ptr[pos] ;
      if( (unsigned)buf_pos == buf_len )
      {
         if( did_alloc )
         {
            u4alloc_again( (*expr_on)->code_base, &buf, &buf_len, buf_len+50 ) ;
            if( buf == 0 )
               return -1 ;
         }
         else
         {
            buf_len += 50 ;
            buf = (char *) u4alloc_er( (*expr_on)->code_base, buf_len + 50 ) ;
            if( buf == 0 )
               return -1 ;
            memcpy( buf, buf_mem, sizeof(buf_mem) ) ;
            did_alloc = 1 ;
         }
      }

      if( ((unsigned) ptr_len - pos) < (unsigned) old_name_len )  continue ;
      if( memicmp( ptr+pos, old_name, old_name_len ) != 0 )  continue ;
      if( u4name_char( ptr[pos+old_name_len] )  )  continue ;
      if( pos > 0 )
         if( u4name_char(ptr[pos-1]) )  continue ;

      did_change = 1 ;
      new_name_len = strlen(new_name) ;
      if( buf_len <= (unsigned) (buf_pos + new_name_len) )
      {
         if( did_alloc )
         {
            u4alloc_again( (*expr_on)->code_base, &buf, &buf_len, buf_len+ new_name_len + 50 ) ;
            if( buf == 0 )
               return -1 ;
         }
         else
         {
            buf_len += new_name_len + 50 ;
            buf = (char *)u4alloc_er( (*expr_on)->code_base, buf_len ) ;
            if( buf == 0 )
               return -1 ;
            memcpy( buf, buf_mem, sizeof(buf_mem) ) ;
            did_alloc = 1 ;
         }
      }

      memcpy( buf+(--buf_pos), new_name, new_name_len ) ;
      buf_pos += new_name_len ;
      pos += old_name_len-1 ;
   }

   if( did_change )
   {
      buf[buf_pos] = 0 ;
      new_expr = expr4parse( (*expr_on)->data, buf ) ;
      if( new_expr )
      {
         expr4free( *expr_on ) ;
         *expr_on = new_expr ;
      }
      if( did_alloc )
         u4free( buf ) ;
      return 0 ;
   }

   return 1 ;
}
#endif
