/* e4functi.c  (c)Copyright Sequiter Software Inc., 1990-1991.        All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
#ifdef __TURBOC__
   #pragma hdrstop
#endif
#endif

#ifndef S4NO_POWER
   #include <math.h>
#endif

E4FUNCTIONS  v4functions[] =
{
   /* function, name, code, name_len, priority, return_type, num_parms, type[]*/
   { e4field_add,     0, 0, 0,  0, r4str, 0 },       /* E4FIELD_STR */
   { e4field_copy,    0, 1, 0,  0, r4str, 0 },       /* E4FIELD_STR_CAT */
   { e4field_log,     0, 2, 0,  0, r4log, 0 },       /* E4FIELD_LOG */
   { e4field_date_d,  0, 3, 0,  0, r4date_doub, 0 }, /* E4FIELD_DATE_D */
   { e4field_add,     0, 4, 0,  0, r4date, 0 },      /* E4FIELD_DATE_S */
   { e4field_num_d,   0, 5, 0,  0, r4num_doub, 0 },  /* E4FIELD_NUM_D */
   { e4field_add,     0, 6, 0,  0, r4num, 0 },       /* E4FIELD_NUM_S */

   #ifdef S4MEMO_OFF
      {         0,    0, 7, 0,  0, r4str, 0 },
   #else
      { e4field_memo, 0, 7, 0,  0, r4str, 0 },       /* E4FIELD_MEMO */
   #endif

   { e4copy_constant, 0, 8, 0, 0, r4num_doub, 0 }, /* E4DOUBLE */
   { e4copy_constant, 0, 9, 0, 0, r4str,      0 }, /* E4STRING */

   { expr4true_function, ".TRUE.", 14, 6, 0, r4log, 0 },
   { expr4true_function, ".T.",    14, 3, 0, r4log, 0 },
   { e4false,         ".FALSE.",16, 7, 0, r4log, 0 },
   { e4false,         ".F.",    16, 3, 0, r4log, 0 },
   { e4not,           ".NOT.",  18, 5, 5, r4log, 1, r4log },

   { e4or,            ".OR.",   20, 4, 3, r4log, -1, r4log }, /* Flexible # of parms.*/
   { e4and,           ".AND.",  22, 5, 4, r4log, -1, r4log },

   { e4parm_remove, "+", 25, 1, 7, r4str, 2, r4str, r4str }, /* Concatenate */
   { e4concat_trim,   0, 25, 0, 7, r4str, 2, r4str, r4str }, /* Concatenate */
   { e4add,           0, 25, 0, 7, r4num_doub,  2, r4num_doub, r4num_doub },
   { e4add_date,      0, 25, 0, 7, r4date_doub, 2, r4num_doub, r4date_doub },
   { e4add_date,      0, 25, 0, 7, r4date_doub, 2, r4date_doub,r4num_doub },

   { e4concat_two,  "-", 30, 1, 7, r4str, 2, r4str, r4str },
   { e4sub,           0, 30, 0, 7, r4num_doub,  2, r4num_doub,  r4num_doub },
   { e4sub_date,           0, 30, 0, 7, r4num_doub,  2, r4date_doub, r4date_doub },
   { e4sub_date,           0, 30, 0, 7, r4date_doub, 2, r4date_doub, r4num_doub },

   { e4equal,            "=",  40, 1, 6, r4log, 2, r4str, r4str },
   { e4equal,              0,  40, 0, 6, r4log, 2, r4log, r4log },
   { e4equal,              0,  40, 0, 6, r4log, 2, r4num_doub,  r4num_doub },
   { e4equal,              0,  40, 0, 6, r4log, 2, r4date_doub, r4date_doub },

   { e4not_equal,        "#",  50, 1, 6, r4log, 2, r4str, r4str },
   { e4not_equal,       "<>",  50, 2, 6, r4log, 2, r4str, r4str },
   { e4not_equal,          0,  50, 0, 6, r4log, 2, r4num_doub, r4num_doub },
   { e4not_equal,          0,  50, 0, 6, r4log, 2, r4date_doub, r4date_doub },
   { e4not_equal,          0,  50, 0, 6, r4log, 2, r4log, r4log },

   { e4greater_eq,      ">=",  60, 2, 6, r4log, 2, r4str, r4str },
   { e4greater_eq_doub,    0,  60, 0, 6, r4log, 2, r4num_doub, r4num_doub },
   { e4greater_eq_doub,    0,  60, 0, 6, r4log, 2, r4date_doub,r4date_doub },

   { e4less_eq,         "<=",  70, 2, 6, r4log, 2, r4str, r4str },
   { e4less_eq_doub,       0,  70, 0, 6, r4log, 2, r4num_doub, r4num_doub },
   { e4less_eq_doub,       0,  70, 0, 6, r4log, 2, r4date_doub,r4date_doub },

   { e4greater,          ">",  80, 1, 6, r4log, 2, r4str, r4str },
   { e4greater_doub,       0,  80, 0, 6, r4log, 2, r4num_doub,  r4num_doub },
   { e4greater_doub,       0,  80, 0, 6, r4log, 2, r4date_doub, r4date_doub },

   { e4less,             "<",  90, 1, 6, r4log, 2, r4str, r4str },
   { e4less_doub,          0,  90, 0, 6, r4log, 2, r4num_doub,  r4num_doub },
   { e4less_doub,          0,  90, 0, 6, r4log, 2, r4date_doub, r4date_doub },

   #ifdef S4NO_POWER
   {       0,             0,   95, 0, 0, r4num_doub, 2, r4num_doub },
   {       0,             0,   95, 0, 0, r4num_doub, 2, r4num_doub },
   #else
   { e4power,           "^",  100, 1, 9, r4num_doub, 2, r4num_doub, r4num_doub },
   { e4power,          "**",  100, 2, 9, r4num_doub, 2, r4num_doub, r4num_doub },
   #endif

   { e4multiply,         "*", 102, 1, 8, r4num_doub, 2, r4num_doub, r4num_doub},
   { e4divide,           "/", 105, 1, 8, r4num_doub, 2, r4num_doub, r4num_doub},
   { e4contain,          "$", 110, 1, 6, r4log, 2, r4str, r4str },

   { e4del,    "DEL",   130, 3, 0, r4str, 0 },
   { e4str,    "STR",   140, 3, 0, r4str, 1, r4num_doub },
   { e4substr, "SUBSTR",150, 6, 0, r4str, 1, r4str },
   { e4time,   "TIME",  160, 4, 0, r4str, 0 },
   { e4upper,  "UPPER", 170, 5, 0, r4str, 1, r4str },
   { e4copy_parm,"DTOS",180, 4, 0, r4str, 1, r4date },
   { e4dtos_doub,   0,  180, 0, 0, r4str, 1, r4date_doub},
   { e4dtoc,     "DTOC",200, 4, 0, r4str, 1, r4date },
   { e4dtoc_doub,     0,200, 4, 0, r4str, 1, r4date_doub},

   { e4trim,     "TRIM",220, 4, 0, r4str, 1, r4str },
   { e4ltrim,   "LTRIM",230, 5, 0, r4str, 1, r4str },
   { e4substr,   "LEFT",240, 4, 0, r4str, 1, r4str },

   { e4iif,  "IIF", 250, 3, 0, r4str,       3, r4log, r4str, r4str },
   { e4iif,      0, 250, 0, 0, r4num_doub,  3, r4log, r4num_doub, r4num_doub},
   { e4iif,      0, 250, 0, 0, r4date_doub, 3, r4log, r4date_doub, r4date_doub},
   { e4iif,      0, 250, 0, 0, r4log,       3, r4log, r4log, r4log },
   { e4stod,          "STOD", 260, 4, 0, r4date_doub, 1, r4str },
   { e4ctod,          "CTOD", 270, 4, 0, r4date_doub, 1, r4str },
   { e4date,          "DATE", 280, 4, 0, r4date_doub },
   { e4day,           "DAY",  290, 3, 0, r4num_doub, 1, r4date },
   { e4day_doub,          0,  290, 0, 0, r4num_doub, 1, r4date_doub },
   { e4month,       "MONTH",  310, 5, 0, r4num_doub, 1, r4date },
   { e4month_doub,        0,  310, 0, 0, r4num_doub, 1, r4date_doub },
   { e4year,         "YEAR",  340, 4, 0, r4num_doub, 1, r4date  },
   { e4year_doub,         0,  340, 0, 0, r4num_doub, 1, r4date_doub },
   { e4deleted,   "DELETED",  350, 7, 0, r4log, 0 },
   { e4reccount, "RECCOUNT",  360, 8, 0, r4num_doub, 0 },
   { e4recno,       "RECNO",  370, 5, 0, r4num_doub, 0 },
   { e4val,           "VAL",  380, 3, 0, r4num_doub, 1, r4str },
   { e4calc_function,      0,  390, 0, 0, 0, 0 },
   { e4calc_total,         0,  400, 0, 0, r4num_doub, 0 },
   { e4pageno, "PAGENO",       410, 6, 0, r4num_doub, 0 },
#ifndef S4UNIX
#ifdef S4CLIPPER
/* DESCEND(NUM_VALUE) and DESCEND(DATE_VALUE) are now not supported.*/
/* { e4descend_num_doub, "DESCEND",420, 7, 0, r4num, 1, r4num_doub },*/
/* { e4descend_num_str,  "DESCEND",420, 7, 0, r4num, 1, r4num },*/
   { e4descend_str,    "DESCEND",  420, 7, 0, r4str,      1, r4str },
/* { e4descend_date_doub, 0,       420, 0, 0, r4date,1, r4date_doub }, */
#endif
#endif
   { 0,0,-1 },
} ;

void S4FUNCTION expr4functions( E4FUNCTIONS **fptr )
{
   *fptr = v4functions ;
}


void e4add()
{
   double *double_ptr = (double *) (expr4buf + expr4info_ptr->result_pos) ;
   *double_ptr = *(double *)expr4[-2] + *(double *)expr4[-1] ;
   expr4[-2] = (char *) double_ptr ;
   expr4-- ;
}

void e4add_date()
{
   if ( v4functions[expr4info_ptr->function_i].type[0] == r4date_doub )
   {
      if ( *(double *)expr4[-2] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }
   else
   {
      if ( *(double *)expr4[-1] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }

   e4add() ;
}

void e4and()
{
   int i ;

   expr4 -= expr4info_ptr->num_parms ;
   for( i = expr4info_ptr->num_parms-1 ; i > 0 ; i-- )
      *(int *) expr4[0] = * (int *) expr4[i]  &&  * (int *) expr4[0] ;
   expr4++ ;
}

void e4calc_function()
{
   EXPR4CALC *e4calc_ptr = (EXPR4CALC *) expr4info_ptr->p1 ;
   char **e4save = expr4 ;
   char *expr4constants_save = expr4constants ;
   char *result_ptr ;

   expr4calc_result_pos( e4calc_ptr, expr4info_ptr->result_pos ) ;
   expr4vary( e4calc_ptr->expr, &result_ptr ) ;
   expr4start( e4calc_ptr->expr->code_base ) ;  /* restart from vary */

   expr4 = e4save ;
   expr4constants = expr4constants_save ;
   *expr4++ = result_ptr ;
}

void e4calc_total()
{
   double d = total4value( (struct TOTAL4st *) expr4info_ptr->p1 ) ;
   *expr4 = expr4buf + expr4info_ptr->result_pos ;
   memcpy( *expr4++, (void *)&d, sizeof(d) ) ;
}

/* The total length of the result is in 'expr4info_ptr->len'. */
void e4concat_special( char move_char )
{
   int num_chars, pos, first_len = expr4info_ptr[-expr4info_ptr[-1].num_entries-1].len ;

   char *ptr = expr4[-2] ;
   for ( pos = first_len-1; pos >= 0; pos-- )
      if ( ptr[pos] != move_char )
         break ;
   if ( ++pos < first_len )
   {
      int len_two = expr4info_ptr->len - first_len ;
      memmove( ptr+ pos, expr4[-1], len_two ) ;

      num_chars = first_len - pos ;
      memset( ptr+expr4info_ptr->len-num_chars, move_char, num_chars ) ;
   }
   expr4-- ;
}

void e4concat_trim()
{
   e4concat_special(0) ;
}

void e4concat_two()
{
   e4concat_special(' ') ;
}

void e4contain()
{
   int   a_len, comp_len, i ;
   char  first_char, *b_ptr ;
   int   log_result = 0 ;

   a_len     = expr4info_ptr[-expr4info_ptr[-1].num_entries-1].len ;
   first_char = *expr4[-2] ;
   comp_len  = expr4info_ptr[-1].len - a_len ;
   b_ptr     = expr4[-1] ;

   /* See if there is a match */
   for ( i=0; i <= comp_len; i++ )
      if ( first_char == b_ptr[i] )
         if ( u4memcmp( expr4[-2], b_ptr+i, (size_t) a_len ) == 0 )
         {
            log_result = 1 ;
            break ;
         }

   expr4[-2] = expr4buf + expr4info_ptr->result_pos ;
   *(int *) expr4[-2] = log_result ;
   expr4-- ;
}

void e4copy_constant()
{
   void *ptr = *expr4++ = expr4buf+expr4info_ptr->result_pos ;
   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( ptr, expr4constants+ expr4info_ptr->i1, expr4info_ptr->len ) ;
}

void e4field_copy()
{
   void *ptr = *expr4++ = expr4buf+expr4info_ptr->result_pos ;
   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( ptr, *(char **)expr4info_ptr->p1 + expr4info_ptr->i1, expr4info_ptr->len ) ;
}

void e4copy_parm()
{
   void *ptr = expr4[-1] ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( expr4[-1], ptr, expr4info_ptr->len ) ;
}

void e4ctod()
{
   char buf[8] ;
   double d ;

   date4init( buf, expr4[-1], expr4constants+ expr4info_ptr->i1 ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   d = date4long( buf ) ;
   memcpy( expr4[-1], (void *)&d, sizeof(d) ) ;
}

void e4date()
{
   char date_buf[8] ;
   date4today( date_buf ) ;
   *expr4++ = expr4buf + expr4info_ptr->result_pos ;
   *((double *) expr4[-1]) = (double) date4long( date_buf ) ;
}

void e4day()
{
   double d ;
   d = (double) date4day( expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   *(double *) expr4[-1] = d ;
}

void e4day_doub()
{
   char date_buf[8] ;
   date4assign( date_buf, (long) *(double *)expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   *(double *) expr4[-1] = (double) date4day( date_buf ) ;
}

void e4del()
{
   expr4[0] = expr4buf + expr4info_ptr->result_pos ;
   expr4[0][0] = *( *(char **)expr4info_ptr->p1) ;
   expr4++ ;
}

void e4deleted()
{
   int result = 0 ;

   #ifdef S4DEBUG
      if ( *( *(char **)expr4info_ptr->p1 ) != '*' && *( *(char **)expr4info_ptr->p1 ) != ' ' )
         e4severe( e4info, E4_EXPR_DELETED ) ;
   #endif

   if ( *( *(char **)expr4info_ptr->p1 ) == '*' )
      result = 1 ;

   *(int *) (*expr4++ = expr4buf + expr4info_ptr->result_pos ) = result ;
}

void e4divide()
{
   double *result_ptr = (double *) (expr4buf + expr4info_ptr->result_pos) ;
   *result_ptr = *(double *)expr4[-2] / *(double *) expr4[-1] ;
   expr4[-2] = (char *) result_ptr ;
   expr4-- ;
}

void e4dtoc()
{
   char buf[sizeof(expr4ptr->code_base->date_format)] ;

   date4format( expr4[-1], buf, expr4constants+ expr4info_ptr->i1 ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   memcpy( expr4[-1], buf, expr4info_ptr->len ) ;
}

void e4dtoc_doub()
{
   e4dtos_doub() ;
   e4dtoc() ;
}

void e4dtos_doub()
{
   date4assign( expr4buf + expr4info_ptr->result_pos, (long) *(double *) expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
}

void e4equal()
{
   int *int_ptr = (int *) (expr4buf + expr4info_ptr->result_pos) ;
   *int_ptr = ! u4memcmp( expr4[-2], expr4[-1], expr4info_ptr->i1 )  ;

   expr4[-2] = (char *) int_ptr ;
   expr4-- ;
}

void e4false()
{
   int *ptr = (int *) (*expr4++ = expr4buf+expr4info_ptr->result_pos) ;
   *ptr = 0 ;
}

void e4field_date_d()
{
   void *ptr = *expr4++ = expr4buf+expr4info_ptr->result_pos ;
   double d = date4long( *(char **)expr4info_ptr->p1 + expr4info_ptr->i1 ) ;
   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
}

void e4field_log()
{
   int *ptr = (int *) (*expr4++ = expr4buf+expr4info_ptr->result_pos) ;
   char char_value = *(* (char **)expr4info_ptr->p1 + expr4info_ptr->i1 ) ;
   if ( char_value == 'Y'  ||  char_value == 'y'  ||
         char_value == 'T'  ||  char_value == 't'  )
      *ptr = 1 ;
   else
      *ptr = 0 ;
}

#ifndef S4MEMO_OFF
void e4field_memo()
{
   char *ptr, *memo_ptr ;
   unsigned memo_len, copy_len, zero_len ;

   ptr = *expr4++ = expr4buf + expr4info_ptr->result_pos ;
   memo_len = f4memo_len( expr4info_ptr->field_ptr ) ;
   memo_ptr = f4memo_ptr( expr4info_ptr->field_ptr ) ;
   if( expr4ptr->code_base->error_code < 0 )
      return ;

   copy_len = memo_len ;
   zero_len = 0 ;
   if( copy_len > (unsigned) expr4info_ptr->len )
      copy_len = expr4info_ptr->len ;
   else
      zero_len = expr4info_ptr->len - copy_len ;

   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( ptr, memo_ptr, copy_len ) ;
   memset( ptr + copy_len, 0, zero_len ) ;
}
#endif

void e4field_num_d()
{
   void *ptr ;
   double d ;

   ptr = *expr4++ = expr4buf + expr4info_ptr->result_pos ;
   d = c4atod( *(char **)expr4info_ptr->p1 + expr4info_ptr->i1, expr4info_ptr->len ) ;
   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
}

void e4greater()
{
   int *int_ptr, rc ;
   int_ptr = (int *)(expr4buf + expr4info_ptr->result_pos) ;
   rc = u4memcmp( expr4[-2], expr4[-1], expr4info_ptr->i1 ) ;

   if( rc > 0 )
      *int_ptr = 1 ;
   else
   {
      if( rc < 0 )
         *int_ptr = 0 ;
      else
         *int_ptr = (int) (expr4info_ptr->p1) ;
   }
   expr4[-2] = (char *) int_ptr ;
   expr4-- ;
}

void e4greater_doub()
{
   e4less_eq_doub() ;
   *(int *)expr4[-1] = ! *(int *)expr4[-1] ;
}

void e4greater_eq()
{
   e4less() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4greater_eq_doub()
{
   int *int_ptr = (int *) (expr4buf + expr4info_ptr->result_pos) ;

   int_ptr = (int *)(expr4buf + expr4info_ptr->result_pos) ;
   if ( *(double *)expr4[-2] >= *(double *)expr4[-1] )
      *int_ptr = 1 ;
   else
      *int_ptr = 0 ;
   expr4[-2] = (char *) int_ptr ;
   expr4-- ;
}

void e4iif()
{
   if ( *(int *) expr4[-3] )
      memmove( expr4buf + expr4info_ptr->result_pos, expr4[-2], expr4info_ptr->len ) ;
   else
      memmove( expr4buf + expr4info_ptr->result_pos, expr4[-1], expr4info_ptr->len ) ;
   expr4[-3] = expr4buf + expr4info_ptr->result_pos ;
   expr4-= 2 ;
}

void e4less()
{
   int *int_ptr, rc ;

   int_ptr = (int *)(expr4buf + expr4info_ptr->result_pos) ;
   rc = u4memcmp( expr4[-2], expr4[-1], expr4info_ptr->i1 ) ;

   if( rc < 0 )
      *int_ptr = 1 ;
   else
   {
      if( rc > 0 )
         *int_ptr = 0 ;
      else
         *int_ptr = (int) (expr4info_ptr->p1) ;
   }

   expr4[-2] = (char *) int_ptr ;
   expr4-- ;
}

void e4less_doub()
{
   e4greater_eq_doub() ;
   *(int *)expr4[-1] = ! *(int *)expr4[-1] ;
}

void e4less_eq()
{
   e4greater() ;
   *((int *)expr4[-1]) = ! *((int *)expr4[-1]) ;
}

void e4less_eq_doub()
{
   int *int_ptr ;

   int_ptr = (int *)(expr4buf + expr4info_ptr->result_pos) ;
   if ( *(double *)expr4[-2] <= *(double *)expr4[-1] )
      *int_ptr = 1 ;
   else
      *int_ptr = 0 ;
   expr4[-2] = (char *) int_ptr ;
   expr4-- ;
}

void e4ltrim()
{
   int n ;
   char *ptr ;

   for( n = 0; n < expr4info_ptr->len; n++ )
      if ( expr4[-1][n] != ' ' && expr4[-1][n] != 0 )
         break ;
   ptr = expr4buf +  expr4info_ptr->result_pos ;
   memmove( ptr, expr4[-1]+n, expr4info_ptr->len - n ) ;
   memset( ptr+ expr4info_ptr->len - n, 0, n ) ;
   expr4[-1] = ptr ;
}

void e4month()
{
   double *double_ptr = (double *) (expr4buf + expr4info_ptr->result_pos) ;
   *double_ptr = (double) date4month( expr4[-1] ) ;
   expr4[-1] = (char *) double_ptr ;
}

void e4month_doub()
{
   char date_buf[8] ;
   double *double_ptr ;

   double_ptr = (double *) (expr4buf + expr4info_ptr->result_pos) ;
   date4assign( date_buf, (long) *(double *)expr4[-1] ) ;
   *double_ptr = (double) date4month( date_buf ) ;
   expr4[-1] = (char *) double_ptr ;
}

void e4multiply()
{
   double *double_ptr ;

   double_ptr = (double *)(expr4buf + expr4info_ptr->result_pos) ;
   *double_ptr = *(double *)expr4[-2] * *(double *)expr4[-1] ;
   expr4[-2] = (char *) double_ptr ;
   expr4-- ;
}

void e4nop()
{
}

void e4not()
{
   int *ptr ;

   ptr = (int *)expr4[-1] ;
   *ptr = !*ptr ;
}

void e4not_equal()
{
   int *int_ptr ;

   int_ptr = (int *)(expr4buf + expr4info_ptr->result_pos) ;
   *int_ptr = u4memcmp( expr4[-2], expr4[-1], expr4info_ptr->i1 )  ;
   expr4[-2] = (char *) int_ptr ;
   expr4-- ;
}

void e4or()
{
   int i ;

   expr4 -= expr4info_ptr->num_parms ;
   for( i = expr4info_ptr->num_parms-1 ; i > 0 ; i-- )
      *(int *) expr4[0] = * (int *) expr4[i]  ||  * (int *) expr4[0] ;
   expr4++ ;
}

void e4field_add()
{
   *expr4++ = *(char **)expr4info_ptr->p1 + expr4info_ptr->i1 ;
}

void e4parm_remove()
{
   expr4-- ;
}

#ifndef S4NO_POWER
void e4power()
{
   double *double_ptr ;

   double_ptr = (double *) (expr4buf + expr4info_ptr->result_pos) ;
   *double_ptr = pow( *(double *) expr4[-2], *(double *) expr4[-1] ) ;
   expr4[-2] = (char *) double_ptr ;
   expr4-- ;
}
#endif

void e4reccount()
{
   double d ;

   d = (double)d4reccount( (DATA4 *) expr4info_ptr->p1 ) ;
   memcpy( *expr4++ = expr4buf+ expr4info_ptr->result_pos, (void *)&d, sizeof(d) ) ;
}

void e4recno()
{
   double d ;

   d = (double) d4recno( (DATA4 *) expr4info_ptr->p1 ) ;
   memcpy( *expr4++ = expr4buf+ expr4info_ptr->result_pos, (void *)&d, sizeof(d) ) ;
}

void e4stod()
{
   double *double_ptr ;

   double_ptr = (double *)(expr4buf + expr4info_ptr->result_pos) ;
   *double_ptr = (double) date4long( expr4[-1] ) ;
   expr4[-1] = (char *) double_ptr ;
}

void e4str()
{
   char *ptr ;

   ptr = expr4buf + expr4info_ptr->result_pos ;
   c4dtoa45( *(double *) expr4[-1], ptr, expr4info_ptr->len, expr4info_ptr->i1 ) ;
   expr4[-1] = ptr ;
}

void e4sub()
{
   double *double_ptr ;

   double_ptr = (double *)(expr4buf + expr4info_ptr->result_pos) ;
   *double_ptr -= *(double *) expr4[-1] ;
   expr4[-2] = (char *) double_ptr ;
   expr4-- ;
}

void e4sub_date()
{
   if ( v4functions[expr4info_ptr->function_i].type[0] == r4date_doub )
   {
      if ( *(double *)expr4[-2] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }

   if ( v4functions[expr4info_ptr->function_i].type[1] == r4date_doub )
   {
      if ( *(double *)expr4[-1] == 0.0 )
      {
         *(double *)expr4-- = 0.0 ;
         return ;
      }
   }

   e4sub() ;
}

void e4substr()
{
   memmove( expr4buf + expr4info_ptr->result_pos, 
            expr4buf + expr4info_ptr->result_pos + expr4info_ptr->i1,
            expr4info_ptr->len ) ;
}

void e4time()
{
   date4time_now( *expr4++ = expr4buf+expr4info_ptr->result_pos ) ;
}

void e4trim()
{
   c4trim_n( expr4[-1], expr4info_ptr->len+ 1 ) ;
}

void expr4true_function()
{
   int *ptr ;
   ptr = (int *)(*expr4++ = expr4buf+expr4info_ptr->result_pos) ;
   *ptr = 1 ;
}

void e4upper()
{
   expr4[-1][expr4info_ptr->len] = 0 ;
   c4upper( expr4[-1] ) ;
}

void e4val()
{
   char *ptr ;
   double d ;

   ptr = expr4buf + expr4info_ptr->result_pos ;
   d = c4atod( expr4[-1], expr4info_ptr[-1].len ) ;
   #ifdef S4DEBUG
      if ( ptr == 0 )
         e4severe( e4info, E4_PARM_NSD ) ;
   #endif
   memcpy( ptr, (void *)&d, sizeof(d) ) ;
   expr4[-1] = (char *) ptr ;
}

void e4year()
{
   double d ;

   d = (double) date4year( expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   *(double *)expr4[-1] = d ;
}

void e4year_doub()
{
   char date_buf[8] ;
   date4assign( date_buf, (long) *(double *)expr4[-1] ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   *(double *) expr4[-1] = date4year( date_buf ) ;
}


void e4pageno()
{
   double d = (double) expr4ptr->code_base->pageno ;
   memcpy( *expr4++ = expr4buf + expr4info_ptr->result_pos, (void *)&d, sizeof(d) ) ;
}
#ifndef S4UNIX
#ifdef S4CLIPPER
/*
void e4descend_num_doub()
{
   c4dtoa_clipper( *(double *) expr4[-1], expr4buf + expr4info_ptr->result_pos, expr4info_ptr->len, expr4ptr->key_dec ) ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   c4descend_num(  expr4[-1], expr4[-1], expr4info_ptr->len ) ;
}

void e4descend_num_str()
{
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   c4descend_num(  expr4[-1], expr4[-1], expr4info_ptr->len ) ;
}

void e4descend_date_doub()
{
   double d = *(double *) expr4[-1] ;
   expr4[-1] = expr4buf + expr4info_ptr->result_pos ;
   c4descend_date( expr4[-1], (long) d, expr4info_ptr->len ) ;
}
*/

void e4descend_str()
{
   c4descend_str( expr4[-1], expr4[-1], expr4info_ptr->len ) ;
}
#endif
#endif

double S4FUNCTION total4value( TOTAL4 *t4 )
{
   switch( t4->total_type )
   {
      case total4sum:
         return t4->total ;
      case total4count:
         return (double) t4->count ;
      case total4average:
         if( t4->count == 0 )
            return 0.0 ;
         return t4->total/t4->count ;
      case total4highest:
         return t4->high ;
      case total4lowest:
         return t4->low ;
      default:
         break ;
   }

   return t4->total ;
}
