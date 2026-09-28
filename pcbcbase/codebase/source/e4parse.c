/* e4parse.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

/* Restrictions - STR can only have a constant 2nd & 3rd parameters
                  SUBSTR can only have a constant 2nd & 3rd parameters
                  LEFT can only have a constant 3rd parameter
                  IIF must return a predictable length and type
                  TRIM and LTRIM returns an unpredictable length.  Its result
                       can be operated on by the concatenate operator only.
                       Ex. TRIM(L_NAME) + TRIM(F_NAME) is OK
                       SUBSTR( TRIM(L_NAME), 3, 2 ) is not OK
                  Memo field's evaluate to a maximum length.  Anything over
                  this maximum gets truncated.  
*/

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

#include <ctype.h>

/* e4massage
   -  Check the type returns to ensure that functions get the correct type
      result.  Use 'E4FUNCTIONS.code' to change the function where possible
      so that the correct function is used.
   -  Make sure that field parameters are put on the stack for the concatentate
      operators.
   -  Fill in the function pointers.
   -  Change (FIELD4 *) pointers in 'p1' to (char *) pointers.
   -  Where the result stack is used, make sure the correct values are filled
      into the E4INFO entires in order to adjust for the lengths needed.
   -  Check the length returns to make sure that 'code_base->expr_buf_len' is large enough
      to handle executing the expression.
   -  Calculate the length of the final result.
   -  Enforce restrictions to TRIM, STR and IIF
   -  Make sure an extra max. length character is added for e4upper() & e4trim()
*/

static int e4massage( E4PARSE *p4 )
{
   E4INFO *info ;
   int parm_pos, i_parm, is_ok, code_on, i_info, num_parms ;
   int type_should_be, len, length_status, i, done_trim_memo_or_calc ;
   int position[E4MAX_STACK_ENTRIES+1] ;
   long length[E4MAX_STACK_ENTRIES] ;
   long buf_len_needed ;
   int types[E4MAX_STACK_ENTRIES] ;
   int num_entries[E4MAX_STACK_ENTRIES] ;
   E4INFO *pointers[E4MAX_STACK_ENTRIES] ;
   CODE4 *code_base ;
   unsigned stored_key_len ;
   #ifdef S4PORTABLE
      int extra_len ;
   #endif

   code_base = p4->code_base ;
   num_parms = done_trim_memo_or_calc = 0 ;
   buf_len_needed = 0 ;

   position[0] = 0 ; /* The first parameter can be placed at position 0 */

   for( i_info = 0; i_info < p4->expr.info_n; i_info++ )
   {
      info = p4->expr.info + i_info ;

      /* Check the parameters types */
      code_on = v4functions[info->function_i].code ;
      if ( v4functions[info->function_i].num_parms != (char)info->num_parms )
         if ( v4functions[info->function_i].num_parms > 0 )
         {
            if( code_base->expr_error )
               e4( code_base, e4num_parms, p4->expr.source ) ;
            return -1 ;
         }

      for(;;)
      {
         if ( code_on != v4functions[info->function_i].code )
         {
            if( code_base->expr_error )
               e4( code_base, e4type_sub, p4->expr.source ) ;
            return -1 ;
         }

         is_ok = 1 ;

         for( i_parm = 0; i_parm < info->num_parms; i_parm++ )
         {
            if ( (int)v4functions[info->function_i].num_parms < 0 )
               type_should_be = v4functions[info->function_i].type[0] ;
            else
               type_should_be = v4functions[info->function_i].type[i_parm] ;

            parm_pos = num_parms - info->num_parms + i_parm ;

            if ( types[parm_pos] != type_should_be )
            {
               if ( types[parm_pos] == r4date && type_should_be == r4date_doub )
               {
                  pointers[parm_pos]->function_i = E4FIELD_DATE_D ;
                  length[parm_pos] = sizeof(double) ;
                  continue ;
               }
               if ( types[parm_pos] == r4num && type_should_be == r4num_doub )
               {
                  pointers[parm_pos]->function_i = E4FIELD_NUM_D ;
                  length[parm_pos] = sizeof(double) ;
                  continue ;
               }

               info->function_i++ ;
               is_ok = 0 ;
               break ;
            }
         }
         if ( is_ok )
            break ;
      }

      switch( info->function_i )
      {
         case E4CONCATENATE:
         case E4CONCAT_TWO:
         case E4TRIM:
         case E4LTRIM:
         case E4UPPER:
         case E4SUBSTR:
         #ifdef S4CLIPPER
            case E4DESCEND_STR:
/*          case E4DESCEND_STR+1: */
         #endif
         #ifndef S4MEMO_OFF
            case E4FIELD_MEMO:
         #endif
            for( i_parm = 1; i_parm <= info->num_parms; i_parm++ )
            {
               E4INFO *info_parm = pointers[num_parms-i_parm] ;
               if ( info_parm->function_i == E4FIELD_STR )
                  /* Make sure the parameter is put on the stack. */
                  info_parm->function_i = E4FIELD_STR_CAT ;
               if ( info->function_i == E4CONCATENATE  &&  done_trim_memo_or_calc )
                  info->function_i = E4CONCAT_TRIM ;
            }
            break ;
         default:
            break ;
      }

      num_parms -= info->num_parms ;
      if ( num_parms < 0 )
      {
         if( code_base->expr_error )
            e4( code_base, e4result, 0 ) ;
         return -1 ;
      }

      types[num_parms] = v4functions[info->function_i].return_type ;

      if ( info->function_i == E4CALC_FUNCTION )
         types[num_parms] = expr4type( ((EXPR4CALC *) info->p1)->expr ) ;
      switch( types[num_parms] )
      {
         case r4str:
            switch( info->function_i )
            {
               case E4FIELD_STR:
               case E4FIELD_STR_CAT:
                  length[num_parms] = f4len( info->field_ptr ) ;
                  break ;

               #ifndef S4MEMO_OFF
                  case E4FIELD_MEMO:
                     length[num_parms] = code_base->mem_size_memo_expr ;
                     done_trim_memo_or_calc = 1 ;
                     break ;
               #endif  /* S4MEMO_OFF */

               case E4CONCATENATE:
               case E4CONCAT_TWO:
               case E4CONCAT_TRIM:
                  info->i1 = (int) (length[num_parms]) ;
                  length[num_parms] += length[num_parms+1] ;
                  break ;

               case E4IIF:
                  if ( length[num_parms+1] != length[num_parms+2] )
                  {
                     if( code_base->expr_error )
                        e4describe( code_base, e4length_err, p4->expr.source, 0, 0 ) ;
                     return -1 ;
                  }
                  length[num_parms] = length[num_parms+1] ;
                  break ;

               case E4DTOS:
               case E4DTOS+1:
                  length[num_parms] = 8 ;
                  break ;

               case E4DTOC:
               case E4DTOC+1:
               case E4CTOD:
                  length[num_parms] = 8 ;
                  info->i1 = p4->constants.pos ;
                  len = strlen( code_base->date_format) ;
                  s4stack_push_str( &p4->constants, code_base->date_format,
                                    len + 1 ) ;
                  if ( info->function_i == E4DTOC || info->function_i == E4DTOC+1 )
                     length[num_parms] = len ;
                  break ;

               case E4DEL:
                  length[num_parms] = 1 ;
                  info->p1 = (char *)&p4->expr.data->record ;
                  break ;

               case E4CALC_FUNCTION:
                  done_trim_memo_or_calc = 1 ;
                  length[num_parms] = expr4len( ((EXPR4CALC *) info->p1)->expr ) ;
                  break ;

               case E4SUBSTR:
               case E4LEFT:
                  if ( info->i1 > (int)(length[num_parms]) )
                     info->i1 = (int)(length[num_parms]) ;
                  if ( info->i1 < 0 )
                     info->i1 = 0 ;
                  length[num_parms]  -= info->i1 ;
                  if ( info->len > (int)(length[num_parms]) )
                     info->len = (int)(length[num_parms]) ;
                  length[num_parms] = info->len ;
                  break ;

               case E4TIME:
                  length[num_parms] = 8 ;
                  break ;

               case E4TRIM:
               case E4LTRIM:
                  done_trim_memo_or_calc = 1 ;
                  p4->expr.has_trim = 1 ;
                  break ;

               case E4UPPER:
               case E4DESCEND_STR:
/*             case E4DESCEND_STR+1: */
                  break ;

               default:
                  length[num_parms] = info->len ;
            }
            break ;

         case r4num:
            length[num_parms] = f4len( info->field_ptr ) ;
            break ;

         case r4num_doub:
         case r4date_doub:
            length[num_parms] = sizeof(double) ;
            #ifdef S4PORTABLE
               if ( length[num_parms] < 8 )
                  length[num_parms] = 8 ;
            #endif
            if ( info->function_i == E4CTOD )
            {
               info->i1 = p4->constants.pos ;
               s4stack_push_str( &p4->constants, code_base->date_format,
                                 strlen( code_base->date_format) + 1 ) ;
            }
            if ( info->function_i == E4RECCOUNT || info->function_i == E4RECNO )
               info->p1 = (char *) p4->expr.data ;
            break ;

         case r4date:
            length[num_parms] = 8 ;
            break ;

         case r4log:
            if ( info->function_i != E4FIELD_LOG )
            {
               if ( info->function_i == E4DELETED )
                  info->p1 = (char *) &p4->expr.data->record ;
               else
               {
                  info->i1 = (int)(length[num_parms+1]) ;
                  length_status = 1 ;
                  if ( length[num_parms] < length[num_parms+1] )
                  {
                     info->i1 = (int)(length[num_parms]) ;
                     length_status = -1 ;
                  }
                  if ( length[num_parms] == length[num_parms+1] )
                     length_status = 0 ;

                  if ( info->function_i == E4GREATER )
                  {
                     if ( length_status > 0 ) 
                        info->p1 = (char *)1 ;
                     else
                        info->p1 = (char *)0 ;
                  }
                  if ( info->function_i == E4LESS )
                  {
                     if ( length_status < 0 ) 
                        info->p1 = (char *)1 ;
                     else
                        info->p1 = (char *)0 ;
                  }
               }
            }
            length[num_parms] = sizeof(int) ;
            break ;

         default:
            #ifdef S4DEBUG
               return e4( code_base, e4result, 0 ) ;
            #else
               return -1 ;
            #endif
      }


      /* make sure there is enough key space allocated for the type,
         in case a partial evaluation occurs */
      switch( types[num_parms] )
      {
         #ifdef S4FOX
            case r4num:
            case r4date:
            case r4num_doub:
               stored_key_len = sizeof( double ) ;
               break ;
         #endif  /*  ifdef S4FOX      */
         #ifdef S4CLIPPER
            case r4num:  /* numeric field return, must fix length problem */
               stored_key_len = f4len( info->field_ptr ) ;
               break ;
            case r4num_doub:
               stored_key_len = code_base->numeric_str_len ;
               break ;
         #endif  /*  ifdef S4CLIPPER  */
         #ifdef S4NDX
            case r4num:
            case r4date:
               stored_key_len = sizeof( double ) ;
               break ;
         #endif  /*  ifdef S4NDX  */
         #ifdef S4MDX
            case r4num:
               stored_key_len = (int)sizeof( C4BCD ) ;
               break ;
            case r4num_doub:
               stored_key_len = (int)sizeof( C4BCD ) ;
               break ;
            case r4date:
            case r4date_doub:
               stored_key_len = sizeof( double ) ;
               break ;
         #endif  /* S4MDX */
         default:
            stored_key_len = (unsigned)(length[num_parms]) ;
      }

      u4alloc_again( code_base, &code_base->stored_key, &code_base->stored_key_len, stored_key_len + 1 ) ;

      #ifdef S4PORTABLE
         if ( types[num_parms] != r4str )
         {
            extra_len = sizeof(double) - position[num_parms] % sizeof(double) ;
            if ( extra_len == sizeof(double) )
               extra_len = 0 ;
            position[num_parms] += extra_len ;
         }
      #endif

      if ( code_base->error_code < 0 )
         return -1 ;

      info->result_pos = position[num_parms] ;
      buf_len_needed = length[num_parms] ;
      if ( info->function_i == E4CALC_FUNCTION )
         buf_len_needed = ((EXPR4 *)info->p1)->len_eval ;
      if( buf_len_needed > INT_MAX )
      {
         e4( code_base, e4overflow, 0 ) ;
         return -1 ;
      }

      if( (types[num_parms] == r4num || types[num_parms] == r4date)  &&
                               length[num_parms] < sizeof(double) )
         position[num_parms+1] = position[num_parms] + sizeof(double) ;
      else
         position[num_parms+1] = position[num_parms] + (unsigned)length[num_parms] ;
      if ( position[num_parms] + buf_len_needed > p4->expr.len_eval )
         p4->expr.len_eval = position[num_parms] + (unsigned)buf_len_needed ;
      info->len = (int)(length[num_parms]) ;

      info->num_entries = 1 ;
      for( i = 0; i < info->num_parms; i++ )
         info->num_entries += num_entries[num_parms+i] ;

      num_entries[num_parms] = info->num_entries ;
      pointers[num_parms] = info ;

      num_parms++ ;
      if ( num_parms >= E4MAX_STACK_ENTRIES )
      {
         if( code_base->expr_error )
            e4( code_base, e4overflow, 0 ) ;
         return -1 ;
      }
   }

   if ( num_parms != 1 )
   {
      if( code_base->expr_error )
         e4( code_base, e4result, 0 ) ;
      return -1 ;
   }

   for( i = 0; i < p4->expr.info_n; i++ )
   {
      info = p4->expr.info + i ;
      info->function = v4functions[info->function_i].function_ptr ;
   }

   p4->expr.len_eval += 1 ;
   if ( code_base->expr_buf_len < (unsigned)p4->expr.len_eval )
      if ( u4alloc_again( code_base, &code_base->expr_work_buf, &code_base->expr_buf_len, p4->expr.len_eval ) == e4memory )
         return -1 ;

   p4->expr.len = (int)(length[0]) ;
   p4->expr.type = types[0] ;
   return 0 ;
}

int e4add_constant( E4PARSE *p4, int i_functions, void *cons_ptr, unsigned cons_len )
{
   E4INFO *info ;
   #ifdef S4PORTABLE
      int extra_len ;
      double d = 0 ;
   #endif

   info = e4function_add( &p4->expr, i_functions ) ;
   if ( info == 0 )  
      return -1 ;
   #ifdef S4PORTABLE
      if ( cons_len <= sizeof(double) )
      {
         extra_len = sizeof(double) - p4->constants.pos % sizeof(double) ;
         if ( extra_len == sizeof(double) )  extra_len = 0 ;
         s4stack_push_str( &p4->constants, &d, (int) extra_len ) ;
      }
   #endif
   info->i1 = p4->constants.pos ;
   info->len = cons_len ;
   return s4stack_push_str( &p4->constants, cons_ptr, (int) cons_len ) ;
}

E4INFO *e4function_add( EXPR4 *expr, int i_functions )
{
   E4INFO *info ;

   if ( (unsigned)((expr->info_n+1)*sizeof(E4INFO)) > expr->code_base->expr_buf_len )
      if ( u4alloc_again( expr->code_base, &expr->code_base->expr_work_buf, &expr->code_base->expr_buf_len, sizeof(E4INFO) * (expr->info_n+10) ) == e4memory )
         return 0 ;

   info = (E4INFO *)expr->code_base->expr_work_buf + expr->info_n++ ;

   info->function_i = i_functions ;
   info->num_parms = v4functions[i_functions].num_parms ;
   if ( info->num_parms < 0 )
      info->num_parms = 2 ;
   info->function = v4functions[i_functions].function_ptr ;
   return info ;
}

void e4function_pop( EXPR4 *expr )
{
   expr->info_n-- ;
}

EXPR4 *S4FUNCTION expr4parse( DATA4 *d4, char *expr_ptr )
{
   E4PARSE parse ;
   char    ops[128] ;
   char    constants[512] ;
   int     rc, info_len, pos_constants ;
   EXPR4 *express4 ;

   #ifdef S4DEBUG
      if ( d4 == 0 || expr_ptr == 0 )
         e4severe( e4parm, E4_EXPR4PARSE ) ;
   #endif

   if ( d4->code_base->error_code < 0 )
      return 0 ;

   if ( d4->code_base->expr_buf_len > 0 )
      memset( d4->code_base->expr_work_buf, 0, d4->code_base->expr_buf_len ) ;

   memset( (void *)&parse, 0, sizeof(E4PARSE) ) ;
   memset( ops, 0, sizeof(ops));

   parse.expr.data   = d4 ;
   parse.expr.source = expr_ptr ;
   parse.code_base   = d4->code_base ;
   parse.expr.code_base = d4->code_base ;

   parse.op.ptr = ops ;
   parse.op.len = sizeof(ops) ;
   parse.op.code_base = d4->code_base ;

   parse.constants.ptr = constants ;
   parse.constants.len = sizeof(constants) ;
   parse.constants.code_base = d4->code_base ;

   s4scan_init( &parse.scan, expr_ptr ) ;

   rc = expr4parse_expr( &parse ) ;
   if ( rc < 0 )
      return 0 ;

   if ( s4stack_cur( &parse.op ) != E4NO_FUNCTION )
   {
      if( parse.code_base->expr_error )
         e4( parse.code_base, e4complete, expr_ptr ) ;
      return 0 ;
   }

   parse.expr.info = (E4INFO *)parse.code_base->expr_work_buf ;
   if ( e4massage( &parse ) < 0 )   
      return 0 ;

   info_len = parse.expr.info_n * sizeof(E4INFO) ;
   pos_constants = sizeof(EXPR4) + info_len ;
   #ifdef S4PORTABLE
      pos_constants += sizeof(double) - pos_constants % sizeof(double) ;
   #endif

   express4 = (EXPR4 *) u4alloc_free( d4->code_base, pos_constants + parse.constants.len + parse.scan.len + 1 ) ;
   if ( express4 == 0 )  
      return 0 ;

   memcpy( (void *)express4, (void *)&parse.expr, sizeof(EXPR4) ) ;

   express4->data = d4 ;
   express4->info = (E4INFO *)( express4 + 1 ) ;
   express4->constants = (char *) express4 + pos_constants ;
   express4->source = express4->constants + parse.constants.len ;

   memcpy( (void *)express4->info, parse.code_base->expr_work_buf, info_len ) ;
   memcpy( express4->constants, constants, parse.constants.len ) ;
   strcpy( express4->source, expr_ptr ) ;

   #ifdef S4CLIPPER
      express4->key_len = parse.expr.key_len ;
      express4->key_dec = parse.expr.key_dec ;
   #endif

   return express4 ;
}

/*    Looks at the input string and returns and puts a character code on the
   result stack corresponding to the next operator.  The operators all operate
   on two operands.  Ex. +,-,*,/, >=, <, .AND., ...

      If the operator is ambiguous, return the arithmatic choice.

   Returns -2 (Done), 0, -1 (Error)
*/

int  e4get_operator( E4PARSE *p4, int *op_return)
{
   char ch ;
   int  op ;

   s4scan_range( &p4->scan, 1, ' ' ) ;
   ch = s4scan_char(&p4->scan) ;
   if ( ch==0 || ch==')' || ch==',')
   {
      *op_return = E4DONE ;
      return(0) ; /* Done */
   }

   op  = e4lookup( p4->scan.ptr+p4->scan.pos, -1, E4FIRST_OPERATOR, E4LAST_OPERATOR ) ;
   if ( op < 0 )
   {
      if( p4->code_base->expr_error )
         e4( p4->code_base, e4unrec_operator, p4->scan.ptr ) ;
      return -1 ;
   }

   p4->scan.pos += v4functions[op].name_len ;
   *op_return = op ;

   return 0 ;
}

/* e4lookup, searches 'v4functions' for an operator or function.

       str - the function name
       str_len - If 'str_len' is greater than or equal to zero it contains the
                 exact number of characters in 'str' to lookup.  Otherwise,
                 as many as needed, provided an ending null is not reached,
                 are compared.
*/

int S4FUNCTION  e4lookup( char *str, int str_len, int start_i, int end_i )
{
   char u_str[9] ;  /* Maximum # of function name characters plus one. */
   int  i ;

   u4ncpy( u_str, str, sizeof(u_str) ) ;
   c4upper( u_str ) ;

   for( i=start_i; i<= end_i; i++)
   {
      if ( v4functions[i].code < 0 )
         break ;
      if ( v4functions[i].name == 0 )
         continue ;
      #ifdef S4DEBUG
         if ( v4functions[i].name_len >= (char)sizeof(u_str) )
            e4severe( e4result, E4_EXPR4LOOKUP ) ;
      #endif

      if ( v4functions[i].name[0] == u_str[0] )
         if( str_len == v4functions[i].name_len || str_len < 0 )
            if (strncmp(u_str, v4functions[i].name, (size_t) v4functions[i].name_len) == 0)
               return( i ) ;
   }
   return -1 ;
}

static int op_to_expr( E4PARSE *p4 )
{
   E4INFO *info ;

   info = e4function_add( &p4->expr, s4stack_pop(&p4->op) ) ;
   if ( info == 0 )  
      return -1 ;

   for(; s4stack_cur(&p4->op) == E4ANOTHER_PARM; )
   {
      s4stack_pop(&p4->op) ;
      info->num_parms++ ;
   }

   return 0 ;
}

/*
     Parses an expression consisting of value [[operator value] ...]
   The expression is ended by a ')', a ',' or a '\0'.
   Operators are only popped until a '(', a ',' or the start of the stack.
   Left to right evaluation for operators of equal priority.

      An ambiguous operator is one which can be interpreted differently
   depending on its operands.  However, its operands depend on the
   priority of the operators and the evaluation order. Fortunately, the
   priority of an ambigous operator is constant regardless of its
   interpretation.  Consequently, the evaluation order is determined first.
   Then ambiguous operators can be exactly determined.

   Ambigous operators:+, -,  >, <, <=, >=, =, <>, #

   Return

       0  Normal
      -1  Error
*/

int  expr4parse_expr( E4PARSE *p4 )
{
   int  op_value, op_on_stack ;

   if ( expr4parse_value(p4) < 0 )  
      return -1 ;

   for(;;)
   {
      if ( e4get_operator(p4, &op_value) < 0 )  
         return -1 ;
      if ( op_value == E4DONE )  /* Done */
      {
         while( s4stack_cur(&p4->op) != E4L_BRACKET
                     && s4stack_cur(&p4->op) != E4COMMA
                     && s4stack_cur(&p4->op) != E4NO_FUNCTION )
            if( op_to_expr( p4 ) < 0 )
               return -1 ;
         return( 0) ;
      }

      /* Everything with a higher or equal priority than 'op_value' must be
         executed first. (equal because of left to right evaluation order)
         Consequently, all high priority operators are sent to the result
         stack.
      */
      while ( s4stack_cur(&p4->op) >= 0 )
      {
         op_on_stack = s4stack_cur(&p4->op) ;
         if ( v4functions[op_value].priority <=
              v4functions[op_on_stack].priority)
         {
            if ( op_value == op_on_stack && (int)v4functions[op_value].num_parms < 0)
            {
               /* If repeated AND or OR operator, combine them into one with an
                  extra paramter.  This makes the relate module optimization
                  algorithms easier. */
               s4stack_pop(&p4->op) ;
               s4stack_push_int( &p4->op, E4ANOTHER_PARM ) ;
               break ;
            }
            else
               if( op_to_expr( p4 ) < 0 )
                  return -1 ;
         }
         else
            break ;
      }

      s4stack_push_int( &p4->op, op_value) ;

      if ( expr4parse_value(p4) < 0 )  
         return -1 ;
   }
}

int  expr4parse_function( E4PARSE *p4, char *start_ptr, int f_len )
{
   int f_num, num_parms, info_i1, info_len ;
   char ch ;
   E4INFO *info ;
   void *new_or_total_ptr = 0 ;
   EXPR4CALC *calc ;
   info_i1 = info_len = 0 ;

   if ( p4->code_base->error_code < 0 )  
      return -1 ;

   f_num = e4lookup( start_ptr, f_len, E4FIRST_FUNCTION, 0x7FFF) ;
   if( f_num < 0 )
   {
      new_or_total_ptr = calc = expr4calc_lookup( p4->code_base, start_ptr, f_len ) ;
      if( calc == 0 )
      {
         if( p4->code_base->expr_error )
            e4( p4->code_base, e4unrec_function, p4->scan.ptr ) ;
         return -1 ;
      }
      else
      {
         f_num = E4CALC_FUNCTION ;
         if( calc->total != 0 )
         {
            f_num = E4TOTAL ;
            new_or_total_ptr = calc->total ;
         }
      }
   }

   s4stack_push_int( &p4->op, E4L_BRACKET ) ;
   p4->scan.pos++ ;

   num_parms = 0 ;
   for(;;)
   {
      ch = s4scan_char( &p4->scan ) ;
      if ( ch == 0 )
      {
         if( p4->code_base->expr_error )
            e4( p4->code_base, e4right_missing, p4->scan.ptr ) ;
         return -1 ;
      }
      if ( ch == ')')
      {
         p4->scan.pos++ ;
         break ;
      }

      if ( expr4parse_expr(p4) < 0 )  
         return -1 ;
      num_parms++ ;

      while( s4scan_char( &p4->scan ) <= ' ' &&
             s4scan_char( &p4->scan ) >='\1')  p4->scan.pos++ ;

      if ( s4scan_char( &p4->scan ) == ')')
      {
         p4->scan.pos++ ;
         break ;
      }
      if ( s4scan_char( &p4->scan ) != ',')
      {
         if( p4->code_base->expr_error )
            e4( p4->code_base, e4comma_expected, p4->scan.ptr ) ;
         return -1 ;
      }
      p4->scan.pos++ ;
   }

   s4stack_pop( &p4->op ) ;  /* pop the left bracket */

   if ( f_num == E4STR )
   {
      info_len= 10 ;

      if ( num_parms == 3  )
      {
         info = (E4INFO *) p4->code_base->expr_work_buf + p4->expr.info_n -1 ;
         if ( info->function_i != E4DOUBLE )
         {
            if( p4->code_base->expr_error )
               e4( p4->code_base, e4not_constant, p4->expr.source ) ;
            return -1 ;
         }
         info_i1 = (int) *(double *) (p4->constants.ptr + info->i1) ;
         e4function_pop( &p4->expr ) ;
         num_parms-- ;
      }
      if ( num_parms == 2  )
      {
         info = (E4INFO *)p4->code_base->expr_work_buf + p4->expr.info_n -1 ;
         if ( info->function_i != E4DOUBLE )
         {
            if( p4->code_base->expr_error )
               e4( p4->code_base, e4not_constant, p4->expr.source ) ;
            return -1 ;
         }
         info_len = (int) *(double *) (p4->constants.ptr + info->i1) ;
         e4function_pop( &p4->expr ) ;
         num_parms-- ;
      }
      if ( info_len < 0 )
         info_len = 10 ;
      if ( info_len <= info_i1+1 )
         info_i1 = info_len - 2 ;
      if ( info_i1 < 0 )
         info_i1 = 0 ;
   }
   if ( num_parms == 3  &&  f_num == E4SUBSTR || num_parms == 2  &&  f_num == E4LEFT )
   {
      info = (E4INFO *)p4->code_base->expr_work_buf + p4->expr.info_n -1 ;
      if ( info->function_i != E4DOUBLE )
      {
         if( p4->code_base->expr_error )
            e4( p4->code_base, e4not_constant, p4->expr.source ) ;
         return -1 ;
      }
      info_len = (int) *(double *) (p4->constants.ptr + info->i1) ;
      e4function_pop( &p4->expr ) ;
      num_parms-- ;
   }
   if ( num_parms == 2  &&  f_num == E4SUBSTR )
   {
      info = (E4INFO *) p4->code_base->expr_work_buf + p4->expr.info_n -1 ;
      if ( info->function_i != E4DOUBLE )
      {
         if( p4->code_base->expr_error )
            e4( p4->code_base, e4not_constant, p4->expr.source ) ;
         return -1 ;
      }
      info_i1 = (int) *(double *) (p4->constants.ptr + info->i1) ;
      info_i1-- ;
      e4function_pop( &p4->expr ) ;
      num_parms-- ;
   }

   if ( p4->code_base->error_code < 0 )  
      return -1 ;

   if ( num_parms != v4functions[f_num].num_parms  &&
        (int)v4functions[f_num].num_parms >= 0 )
   {
      if( p4->code_base->expr_error )
         e4describe( p4->code_base, e4num_parms, p4->scan.ptr, E4_NUM_PARMS, v4functions[f_num].name ) ;
      return -1 ;
   }

   info = e4function_add( &p4->expr, f_num ) ;
   if ( info == 0 )  
      return -1 ;

   info->i1  = info_i1 ;
   info->len = info_len ;

   info->num_parms = num_parms ;
   if ( f_num == E4CALC_FUNCTION || f_num == E4TOTAL )
      info->p1 = (char *) new_or_total_ptr ;
   return 0 ;
}

int expr4parse_value( E4PARSE *p4 )
{
   FIELD4 * field_ptr ;
   char ch, *start_ptr, search_char ;
   int  i_functions, len, i_function, save_pos ;
   double d ;
   E4INFO *expr, *info ;
   DATA4 *base_ptr ;

   if ( p4->code_base->error_code < 0 )  
      return -1 ;

   s4scan_range( &p4->scan, ' ', ' ' ) ;

   /* expression */

   if ( s4scan_char( &p4->scan ) == '(')
   {
      p4->scan.pos++ ;

      s4stack_push_int( &p4->op, E4L_BRACKET) ;
      if ( expr4parse_expr(p4) < 0 )  return( -1 ) ;

      while ( s4scan_char( &p4->scan ) <= ' ' &&
         s4scan_char( &p4->scan ) != 0)   p4->scan.pos++ ;

      if ( s4scan_char( &p4->scan ) != ')' )
      {
         if( p4->code_base->expr_error )
            e4( p4->code_base, e4right_missing, p4->scan.ptr ) ;
         return -1 ;
      }
      p4->scan.pos++ ;
      s4stack_pop( &p4->op ) ;
      return( 0 ) ;
   }

   /* logical */
   if ( s4scan_char( &p4->scan ) == '.' )
   {
      i_functions = e4lookup( p4->scan.ptr+p4->scan.pos, -1, E4FIRST_LOG, E4LAST_LOG ) ;
      if ( i_functions >= 0 )
      {
         p4->scan.pos += v4functions[i_functions].name_len ;

         if ( strcmp( v4functions[i_functions].name, ".NOT." ) == 0 )
         {
            if ( expr4parse_value(p4) < 0 )  return( -1 ) ; /* One operand operation */
            s4stack_push_int( &p4->op, i_functions ) ;
            return 0 ;
         }

         expr = e4function_add( &p4->expr, i_functions ) ;
         if ( expr == 0 ) 
            return -1 ;
         return 0 ;
      }
   }

   /* string */
   ch = s4scan_char( &p4->scan ) ;
   if ( ch == '\'' || ch == '\"')
   {
      search_char = s4scan_char( &p4->scan ) ;

      p4->scan.pos++ ;
      start_ptr = p4->scan.ptr + p4->scan.pos ;

      len = s4scan_search( &p4->scan, search_char ) ;
      if ( s4scan_char( &p4->scan ) != search_char )
         if ( len < 0 )
         {
            if( p4->code_base->expr_error )
               e4( p4->code_base, e4unterminated, p4->scan.ptr ) ;
            return -1 ;
         }
      p4->scan.pos++ ;

      if ( e4add_constant( p4, E4STRING, start_ptr, len ) < 0 )
         return -1 ;
      return 0 ;
   }

   /* real */
   ch = s4scan_char( &p4->scan ) ;
   if ( ch >='0' && ch <='9' || ch == '-' || ch == '+' || ch == '.' )
   {
      start_ptr = p4->scan.ptr+p4->scan.pos ;
      save_pos = p4->scan.pos ;
      p4->scan.pos++ ;
      len = 1 ;

      while( s4scan_char( &p4->scan ) >= '0' &&
         s4scan_char( &p4->scan ) <= '9' ||
         s4scan_char( &p4->scan ) == '.' )
      {
         if ( s4scan_char( &p4->scan ) == '.' )
         {
            if ( strnicmp( p4->scan.ptr+p4->scan.pos, ".AND.",5) == 0 ||
                 strnicmp( p4->scan.ptr+p4->scan.pos, ".OR.",4) == 0 ||
                 strnicmp( p4->scan.ptr+p4->scan.pos, ".NOT.",5) == 0 )
               break ;
            /* if the next value is a character, then we have a database
               with a number as its name/alias.  (i.e. 111.afld), since
               numerics are invalid to being a field name, MUST be a 
               number if a numeric after the decimal point... */
            if ( toupper( s4scan_char( &p4->scan + 1 ) ) >= 'A' && toupper( s4scan_char( &p4->scan + 1 ) ) <= 'Z' )
            {
               p4->scan.pos++ ;   /* make sure a letter is identified below... */
               break ;
            }
         }
         len++ ;
         p4->scan.pos++ ;
      }

      /* check to see if maybe actually a database name starting with a numeric... */
      if ( toupper( s4scan_char( &p4->scan ) ) >= 'A' && toupper( s4scan_char( &p4->scan ) ) <= 'Z' )
         p4->scan.pos = save_pos ;
      else
      {
         d = c4atod( start_ptr, len ) ;
         if ( e4add_constant( p4, E4DOUBLE, &d, sizeof(d) ) < 0 )
            return -1 ;
         return 0 ;
      }
   }

   /* function or field */
   if (u4name_char(s4scan_char( &p4->scan )) )
   {
      char b_name[11], f_name[11] ;
      int is_base ;

      start_ptr = p4->scan.ptr + p4->scan.pos ;

      for( len=0; u4name_char(s4scan_char( &p4->scan )); len++ )
         p4->scan.pos++ ;

      s4scan_range( &p4->scan, (char)0, ' ' ) ;

      if ( s4scan_char( &p4->scan ) == '(' )
         return expr4parse_function( p4, start_ptr, len ) ;

      base_ptr = 0 ;

      #ifdef S4FOX
         if ( s4scan_char( &p4->scan ) == '.' )
         {  /* for fox, same as -> */
            if ( len > 10 )
               len = 10 ;
            memmove( b_name, start_ptr, (size_t)len ) ;
            b_name[len] = '\0' ;

            base_ptr = d4data( p4->code_base, b_name) ;
         }
      #endif

      if ( s4scan_char( &p4->scan ) == '-' )
         if ( p4->scan.ptr[p4->scan.pos+1] == '>')
         {
            if ( len > 10 )
               len = 10 ;
            memmove( b_name, start_ptr, (size_t)len ) ;
            b_name[len] = '\0' ;

            base_ptr = d4data( p4->code_base, b_name) ;

            if ( base_ptr == 0 )
            {
               if( p4->code_base->expr_error )
                  e4describe( p4->code_base, e4data_name, b_name, p4->scan.ptr, (char *) 0 ) ;
               return -1 ;
            }
            p4->scan.pos++ ;
         }


      if ( base_ptr != 0 )
      {
         p4->scan.pos++ ;

         start_ptr = p4->scan.ptr + p4->scan.pos ;
         for( len=0; u4name_char(s4scan_char( &p4->scan )); len++ )
            p4->scan.pos++ ;
      }
      else
         base_ptr = (DATA4 *) p4->expr.data ;

      if ( len <= 10)
      {
         memmove( f_name, start_ptr, (size_t) len ) ;
         f_name[len] = 0 ;
         field_ptr = d4field( base_ptr, f_name ) ;
         if ( field_ptr == 0 )  
            return -1 ;

         #ifdef S4CLIPPER
            p4->expr.key_len = field_ptr->len ;
            p4->expr.key_dec = field_ptr->dec ;
         #endif

         i_function = 0 ;
         switch( field_ptr->type )
         {
            case 'N':
            case 'F':
               i_function = E4FIELD_NUM_S ;
               break ;
            case 'C':
               i_function = E4FIELD_STR ;
               break ;
            case 'D':
               i_function = E4FIELD_DATE_S ;
               break ;
            case 'L':
               i_function = E4FIELD_LOG ;
               break ;
            case 'M':
               #ifdef S4MEMO_OFF
                  return e4( p4->code_base, e4not_memo, E4_EXPR4PV ) ;
               #else
                  i_function = E4FIELD_MEMO ;
                  break ;
            #endif
            default:
               if( p4->code_base->expr_error )
                  e4( p4->code_base, e4type_sub, E4_TYPE_UFT ) ;
               return -1 ;
         }

         info = e4function_add( &p4->expr, i_function ) ;
         if ( info == 0 )  
            return -1 ;
         info->field_ptr = field_ptr ;
         info->p1 = (char *) &base_ptr->record ;
         info->i1 = field_ptr->offset ;

         return 0 ;
      }
   }

   if( p4->code_base->expr_error )
      e4( p4->code_base, e4unrec_value, p4->scan.ptr ) ;
   return -1 ;
}

int s4stack_pop( S4STACK *s4 )
{
   int ret_value ;

   ret_value = s4stack_cur(s4) ;

   if ( s4->pos >= sizeof(int) )
      s4->pos -= sizeof(int) ;
   return ret_value ;
}

int s4stack_cur( S4STACK *s4 )
{
   int pos, cur_data ;

   if ( s4->pos < sizeof(int) )
      return E4NO_FUNCTION ;
   pos = s4->pos - sizeof(int) ;
   memcpy( (void *)&cur_data, s4->ptr+pos, sizeof(int) ) ;
   return cur_data ;
}

int s4stack_push_int( S4STACK *s4, int i )
{
   return s4stack_push_str( s4, &i, sizeof(i)) ;
}

int s4stack_push_str( S4STACK *s4, void *p, int len )
{
   char *old_ptr ;

   if ( s4->code_base->error_code < 0 )  
      return -1 ;

   if ( s4->pos+len > s4->len )
   {
      old_ptr = s4->ptr ;
      if ( ! s4->do_extend )
         s4->ptr = 0 ;
      else
         s4->ptr = (char *)u4alloc_free( s4->code_base, s4->len + 256 ) ;
      if ( s4->ptr == 0 )
      {
         s4->ptr = old_ptr ;
         if( s4->code_base->expr_error )
            e4( s4->code_base, e4memory, 0 ) ;
         return -1 ;
      }
      memcpy( s4->ptr, old_ptr, s4->len ) ;
      u4free( old_ptr ) ;
      s4->len += 256 ;

      return  s4stack_push_str( s4, p, len ) ;
   }
   else
   {
      memcpy( s4->ptr+s4->pos, p, len ) ;
      s4->pos += len ;
   }
   return 0 ;
}


char s4scan_char( S4SCAN *s4 )
{
   if ( s4->pos >= s4->len )
      return 0 ;
   return s4->ptr[s4->pos] ;
}

void s4scan_init( S4SCAN *s4, char *p )
{
   s4->ptr = p ;
   s4->pos = 0 ;
   s4->len = strlen(p) ;
}

int s4scan_range( S4SCAN *s4, int start_char, int end_char )
{
   int count ;

   for ( count = 0; s4->pos < s4->len; s4->pos++, count++ )
      if ( s4->ptr[s4->pos] < start_char || s4->ptr[s4->pos] > end_char )
	 return count ;
   return count ;
}

int s4scan_search( S4SCAN *s4, char search_char )
{
   int count ;

   for ( count = 0; s4->pos < s4->len; s4->pos++, count++ )
      if ( s4->ptr[s4->pos] == search_char )
         return count ;
   return count ;
}

#ifdef S4VB_DOS

EXPR4 *expr4parse_v( DATA4 *d4, char *expr )
{
 return expr4parse( d4, c4str(expr) ) ;
}

#endif
