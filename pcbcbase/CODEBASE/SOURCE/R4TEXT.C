/* r4text.c (c)Copyright Sequiter Software Inc., 1991-1993 All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif


void S4FUNCTION text4alignment( TEXT4 * t4, int alignment )
{
   t4->alignment =  alignment ;
}

void S4FUNCTION text4numeric( TEXT4 * t4, int numeric_type )
{
   t4->numeric_type = numeric_type ;
}

void S4FUNCTION text4len_max( TEXT4 * t4, int l )
{
   t4->len_max = l ;
}

void S4FUNCTION text4dec( TEXT4 * t4, int dec )
{
   t4->dec =  dec ;
}

void S4FUNCTION text4use_brackets( TEXT4 * t4, int use_brackets )
{
   t4->use_brackets =  (char) use_brackets ;
}

void S4FUNCTION text4display_zero( TEXT4 * t4, int display_zero )
{
   t4->display_zero =  (char) display_zero ;
}

int S4FUNCTION text4date_format( TEXT4 *t4, char *date_format )
{
   u4free( t4->date_format ) ;
   if( date_format )
   {
      t4->date_format = (char *) u4alloc_er( t4->obj.list->report->cb, 
         strlen(date_format)+1 ) ;
      
      if( t4->date_format == 0 )
         return -1 ;

      strcpy( t4->date_format, date_format ) ;
   }
   else
      t4->date_format =  0 ;

   return 0 ;
}

/* 'ptr' must point to a big enough buffer to handle any possible conversion. */
int S4FUNCTION text4conv_double( TEXT4 *t4, double doub_val, char *ptr )
{
   int sig_digs = 0, sign_val, n_dec, pos ;
   REPORT4 *r4 ;
   char *result ;
   int left ;

   r4 = t4->obj.list->report ;
   if( ! t4->display_zero )
      if( doub_val == 0.0 )
         return 0 ;

   pos = 0 ;
   n_dec =  t4->dec ;
   if( t4->numeric_type == text4percent )
      n_dec += 2 ;

   result =  fcvt( doub_val, n_dec, (int *)&sig_digs, (int *)&sign_val) ;
   if( result[0] == '0' )
      sig_digs = 0 ;

   if( t4->numeric_type == text4exponent )
   {
      ptr[pos++] =  *result++ ;
      ptr[pos++] =  r4->decimal_point ;
      if(t4->dec)
         left =  t4->dec - u4ncpy( ptr+pos, result, t4->dec) ;
      else
         left = 0;
      pos +=  left ;
      if( left )
      {
         memset( ptr+pos, '0', left ) ;
         pos += left ;
      }
      ptr[pos++] = 'e' ;
      ltoa( sig_digs-1, ptr+pos, 10 ) ;

      pos +=  strlen(ptr+pos) ;
      return pos ;
   }

   if( t4->numeric_type == text4percent )
   {
      sig_digs += 2 ;
      n_dec -= 2 ;
   }

   if( n_dec <= -sig_digs )
      sign_val =  0 ;

   if( sign_val )
   {
      if( t4->use_brackets )
         ptr[pos++] = '(' ;
      else
          ptr[pos++] = '-' ;
   }

   if( t4->numeric_type == text4dollar )
      ptr[pos++] =  r4->currency_sym ;

   if( sig_digs > 0 )
   {
      int first_few =  sig_digs % 3 ;
      if( first_few == 0 && sig_digs > 0 )
      first_few = 3 ;
      memcpy( ptr+pos, result, first_few ) ;
      pos+= first_few ;
      result +=  first_few ;
      sig_digs -=  first_few ;

      for(; sig_digs > 0; sig_digs -= 3 )
      {
         if( r4->thousand_separator )
            ptr[pos++] =  r4->thousand_separator ;

         memcpy( ptr+pos, result, 3 ) ;
         pos+= 3 ;
         result += 3 ;
      }

      if( t4->dec > 0 )
         ptr[pos++] =  r4->decimal_point ;

      memcpy( ptr+pos, result, t4->dec ) ;
      pos +=  t4->dec ;
   }
   else
   {
      if( r4->leading_zero )
         ptr[pos++] = '0' ;

      if( t4->dec > 0 )
      {
         ptr[pos++] =  r4->decimal_point ;
         memset( ptr+pos, '0', -sig_digs ) ;
         if( t4->dec > -sig_digs )
            memcpy( ptr+pos+ (-sig_digs), result, t4->dec- (-sig_digs) ) ;
         pos +=  t4->dec ;
      }

      if( pos == 0 )
         ptr[pos++] = '0' ;  /* Ensure at least a single zero */
   }

   if( sign_val  &&  t4->use_brackets )
      ptr[pos++] =  ')' ;

   if( t4->numeric_type == text4percent )
      ptr[pos++] =  '%' ;

   return pos ;
}

TEXT4 * S4FUNCTION text4create( OBJECTS4 *o4list, int x, int y )
{
   TEXT4 *text =  (TEXT4 *)  mem4create_alloc(o4list->report->cb,
                                             &o4list->report->text_memory,
                                             5,
                                             sizeof(TEXT4),
                                             5,
                                             0);

   if ( text == 0 )
   {
      e4(o4list->report->cb,e4report,E4_REPORT_ATEXT);
      return 0 ;
   }

   l4add( &o4list->list, &text->obj.link ) ;
   text->obj.display_status =  text4display_always ;

   text->obj.x =  x ;
   text->obj.y =  y ;
   text->alignment =  text4left ;
   text->display_zero = 1;

   text->obj.list =  o4list ;
   text->style = (STYLE4 *) o4list->report->styles.selected ;
   text->reset_group = NULL;
   return text ;
}

void S4FUNCTION text4len_set( TEXT4 *t4 )
{
   if(t4->expr)
   {
      if(t4->expr->type == 'D' || t4->expr->type == 'd')
      {
         if(t4->date_format)
            t4->len = strlen(t4->date_format);
         else
            t4->len = strlen(t4->obj.list->report->cb->date_format);
      }
      else
         t4->len = expr4len(t4->expr);

      if(expr4type(t4->expr) == r4num)
      {
         t4->len = t4->len + t4->len/3 - 1;
         if(t4->dec > 0)
            t4->len += t4->dec + 1;
         switch(t4->numeric_type)
         {
            case text4exponent:
               t4->len = 6 + t4->len/10;
               break;

            case text4dollar:
            case text4percent:
               t4->len += 1;
               break;
         }
      }
   }

   if( t4->field )
      t4->len =  f4len( t4->field) ;

   if( t4->ptr )
      t4->len =  strlen( t4->ptr) ;

   if( t4->total )
      t4->len = 8 ;
}

TEXT4 * S4FUNCTION text4expr( OBJECTS4 *o4list, int x, int y, char *expr )
{
   TEXT4 *text =  text4create( o4list, x, y ) ;
   if( text == 0 )  return 0 ;

   text->obj.object_type =  obj4expr ;

   text->expr = expr4parse( o4list->report->relate->data, expr ) ;
   if ( text->expr == 0 )
   {
      e4set( o4list->report->cb, 0 ) ;
      obj4free( &text->obj ) ;
      return 0 ;
   }

   if(text->expr->type == 'N' || text->expr->type == 'F' 
   || text->expr->type == 'n' || text->expr->type == 'f')
   {
      text->alignment = text4right;
      text->len =  expr4len( text->expr ) ;
   }
   else
   if(text->expr->type == 'D' || text->expr->type == 'd')
   {
      if(text->date_format)
         text->len = strlen(text->date_format);
      else
         text->len = strlen(o4list->report->cb->date_format);
   }
   else
      text->len =  expr4len( text->expr ) ;
   return text ;
}

TEXT4 * S4FUNCTION text4field( OBJECTS4 *o4list, int x, int y, FIELD4 *f4 )
{
   TEXT4 *text =  text4create( o4list, x, y ) ;
   if( text == 0 )  return 0 ;

   text->obj.object_type =  obj4field ;

   text->field =  f4 ;
   text->len =  f4len( f4 ) ;
   text->dec =  f4decimals( f4 ) ;

   if( f4->type == 'N' || f4->type == 'F' || f4->type == 'n' || f4->type == 'f')
      text->alignment = text4right;

   return text ;
}

TEXT4 * S4FUNCTION text4label( OBJECTS4 *o4list, int x, int y, char *label )
{
   TEXT4 *text =  text4create( o4list, x, y ) ;
   if( text == 0 )  return 0 ;

   text->len =  strlen( label ) ;
   text->obj.object_type =  obj4label ;

   text->ptr =  (char *) u4alloc_er( o4list->report->cb, text->len+ 1 ) ;
   if( text->ptr == NULL )
   {
       return NULL;
   }
   strcpy( text->ptr, label ) ;

   return text ;
}

STYLE4 * S4FUNCTION text4style( TEXT4 *t4, STYLE4 *s4 )
{
   STYLE4 *style_prev =  t4->style ;
   if ( s4 != 0 )
      t4->style =  s4 ;

   return style_prev ;
}

TEXT4 * S4FUNCTION text4total( OBJECTS4 *o4list, int x, int y, TOTAL4 *total )
{
   TEXT4 *text =  text4create( o4list, x, y ) ;
   if( text == 0 )  return 0 ;

   text->obj.object_type =  obj4total ;

   text->total =  total ;
   text->len =  expr4len(text->total->calc_ptr->expr);
   text->alignment = text4right ;
   return text ;
}

int S4FUNCTION text4width(TEXT4 *text, int w)
{
   int old_len = text->len ;
   text->len = w ;
   return old_len ;
}

#ifdef S4WINDOWS
int S4FUNCTION text4width_estimate( TEXT4 *t4, TEXTMETRIC *tm_ptr )
{
   EXPR4 *texpr;
   

   if( t4->obj.w > 0 )  return t4->obj.w ;

   if( t4->len_max )
      return t4->len_max * tm_ptr->tmAveCharWidth ;

   if(t4->total)
   {
      t4->len = 8;  
   }
   else
   if(t4->expr)
   {
      if(t4->expr->type == 'D' || t4->expr->type == 'd')
      {
         if(t4->date_format)
            t4->len = strlen(t4->date_format);
         else
            t4->len = strlen(t4->obj.list->report->cb->date_format);
      }
      else
         t4->len = expr4len(t4->expr);
      if(expr4type(t4->expr) == r4num)
      {
         t4->len = t4->len + t4->len/3 - 1 ;
         if(t4->dec > 0)
            t4->len += t4->dec + 1;
         switch(t4->numeric_type)
         {
            case text4exponent:
               t4->len = 6 + t4->len/10;
               break;

            case text4dollar:
            case text4percent:
               t4->len += 1;
               break;
         }
      }
   }
   else
   if(t4->field)
   {
      t4->len = t4->field->len;
   }

   return  t4->len * tm_ptr->tmAveCharWidth ;
}
#endif

void S4FUNCTION text4display_once( TEXT4 *t4, GROUP4 *reset_group )
{
   OBJ4 *obj ;
   obj = (OBJ4 *)t4 ;
   if(reset_group)
   {
      t4->reset_group = reset_group ;
      obj->display_status = text4disp_once ;
   }
   else
   {
      t4->reset_group = NULL;
      obj->display_status = text4display_always ;
   }
}
