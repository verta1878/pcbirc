/* r4object.c   (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"

#ifndef S4UNIX
#ifndef __IBMC__
   #include <dos.h>
#endif
#endif

#ifdef __TURBOC__
   #pragma hdrstop
#endif

int S4FUNCTION objects4display( OBJECTS4 *o4list )
{
   OBJ4 *display_on ;

   display_on = 0;
   display_on = (OBJ4 *)l4first( &o4list->list);
   while(display_on)
   {
      obj4display( display_on, o4list->report, o4list->report->y ) ;
      display_on = (OBJ4 *)l4next( &o4list->list, display_on);
   }

   #ifdef S4WINDOWS
      o4list->report->y +=  o4list->height_dev ;
   #else
      o4list->report->cline += o4list->height_dev;
      o4list->report->y = o4list->report->cline;
   #endif

   return 0;
}

int S4FUNCTION objects4height( OBJECTS4 *o4list, int h )
{
   int prev_height =  o4list->height ;
   if( h >= 0 )
      o4list->height =  h ;

   return prev_height ;
}

void S4FUNCTION objectsort4init(OBJECTS4 *o4list,REPORT4 *report, GROUP4 *group)
{
   o4list->group  =  group ;
   o4list->report =  report ;

   if( group != 0 )
      if( &group->header == o4list )

   o4list->height =  333 ;
}


void S4FUNCTION objects4purge( OBJECTS4 *o4list )
{
   OBJ4 *obj ;

   for( obj = (OBJ4 *) l4first( &o4list->list); obj != 0; )
   {
      OBJ4 *obj_next = (OBJ4 *) l4next( &o4list->list, obj ) ;
      obj4free( obj ) ;
      obj =  obj_next ;
   }
}


int S4FUNCTION obj4display( OBJ4 *o4, REPORT4 *r4, int y_offset )
{
   char *ptr, buf[256], *result_ptr ;
   TEXT4 *t4 = (TEXT4 *) o4 ;
   int len, right ;
   double *d_ptr ;
   double d ;
   char *date_format ;
   int text_width, x ;
   STYLE4 *style ;
   #ifdef S4WINDOWS
   POINT fwidth ;
   TEXTMETRIC tm ;
   #endif

   if ( o4->display_status == text4displayed )
      return 0 ;

   if ( o4->display_status == text4disp_once )
      o4->display_status =  text4displayed ;

   style = t4->style ;
   ptr =  buf ;
   len =  t4->len ;

   switch( o4->object_type )
   {
      case obj4field:
         result_ptr =  f4memo_ptr( t4->field ) ;
         len =  f4memo_len( t4->field ) ;

         switch( t4->field->type )
         {
            case 'N':
            case 'F':
               len = text4conv_double( t4, f4double(t4->field), buf ) ;
               break ;

            case 'D':
               if( *result_ptr == ' ' )
                  len = 0 ;
               else
               {
                  date_format =  t4->date_format ?
                  t4->date_format : r4->cb->date_format ;
                  date4format( result_ptr, buf, date_format ) ;
                  len =  strlen(buf) ;
               }
               break ;

            case 'L':
               len =  1 ;
               break ;

            default:
               ptr =  result_ptr ;
         }
         break ;

      case obj4expr:
         len = expr4vary( t4->expr, &result_ptr ) ;
         if(t4->len == 0) t4->len = len;
         switch( expr4type(t4->expr) )
         {
            case r4num: /* t4num_str */
               d = c4atod( result_ptr, expr4len(t4->expr) ) ;
               len = text4conv_double( t4, d, buf ) ;
               break ;

            case r4num_doub:
               d_ptr = (double *) result_ptr ;
               len = text4conv_double( t4, *d_ptr, buf ) ;
               break ;

            case r4date_doub:
               d_ptr = (double *) result_ptr ;
               if( *d_ptr >= 1.0E99 )
                  len = 0 ;
               else
               {
                  char date_buf[8] ;
                  date4assign( date_buf, (long) *d_ptr ) ;

                  date_format =  t4->date_format ?
                     t4->date_format : r4->cb->date_format ;

                  date4format( date_buf, buf, date_format ) ;
                  len =  strlen(buf) ;
               }
               break ;

            case r4date:
               if( *result_ptr == ' ' )
                  len = 0 ;
               else
               {
                  date_format =  t4->date_format ?
                     t4->date_format : r4->cb->date_format ;
                  date4format( result_ptr, buf, date_format ) ;
                  len =  strlen(buf) ;
               }
               break ;

            case r4log: 
               len = 1 ;
               if( *(int *)result_ptr == 1)
                  M4PRINT(ptr,"T");
               else
                  M4PRINT(ptr,"F");

               break ;

            default:
               ptr = result_ptr ;
               if(ptr[len-1] == 0)
                  len = strlen(ptr);
         }
         break ;

      case obj4label:
         ptr =  t4->ptr ;
         break ;

      case obj4total:
         len = text4conv_double( t4, total4value(t4->total), buf ) ;
         break ;

      default:
         e4severe( e4result, "obj4display()" ) ;
   }

   if( t4->len_max != 0  &&  len > t4->len_max )
      len = t4->len_max ;

   #ifdef S4WINDOWS
   SelectObject( r4->hDC, t4->style->hFont ) ;
   if( r4->to_screen != 0 )
      SetTextColor( r4->hDC, t4->style->color ) ;
   GetTextMetrics(r4->hDC,&tm) ;
   #endif

   if( t4->alignment != text4left )
   {

      #ifdef S4WINDOWS
      if( o4->w == 0 )
      {
         o4->r.right = o4->r.left ;
         o4->r.right += text4width_estimate((TEXT4 *) o4, &tm) ;
      }
      text_width = LOWORD( GetTextExtent( r4->hDC, ptr, len ) ) ;
      #else
      text_width = len;
      #endif

      right = o4->r.right ;

      x =  o4->r.right ;

      switch( t4->alignment )
      {
         case text4right:
            x =  right - text_width ;
            break ;

         case text4center:
            x =  o4->r.left + (right - o4->r.left - text_width)/2 ;
            break ;
      }

      if( x < 0 )
         x = 0;

      #ifdef S4WINDOWS
         TextOut( r4->hDC, x, o4->r.top + y_offset, ptr, len ) ;
      #else
         report4driver_write( x,o4->r.top + y_offset , ptr, len, style->codes_before,
          style->codes_before_len,style->codes_after,style->codes_after_len ) ;
      #endif
   }
   else
   {
      #ifdef S4WINDOWS
      if(o4->w)
      {
         dc4mapping(r4->hDC);
         fwidth.x = o4->w;
         LPtoDP(r4->hDC,&fwidth,1);
         SetMapMode(r4->hDC,MM_TEXT);
      }
      TextOut( r4->hDC, o4->r.left, o4->r.top + y_offset, ptr, len ) ;
      #else
         report4driver_write( o4->r.left,o4->r.top + y_offset , ptr, len, style->codes_before,
          style->codes_before_len,style->codes_after,style->codes_after_len ) ;
      #endif
   }
   return 0 ;
}

OBJ4 * S4FUNCTION obj4first( REPORT4 *r4 )
{
   OBJECTS4 *list =  objects4first(r4) ;
   for(;;)
   {
      OBJ4 *obj_on =  (OBJ4 *) l4first( &list->list ) ;
      if( obj_on )  return obj_on ;

      list = objects4next( list ) ;
      if( list == 0 )  return 0 ;
   }
}

OBJ4 * S4FUNCTION obj4next( OBJ4 *obj_on )
{
   OBJECTS4 *list ;

   if( obj_on == 0 )  return 0 ;

   list = obj_on->list ;

   obj_on =  (OBJ4 *) l4next( &list->list, obj_on ) ;
   if( obj_on )  return obj_on ;

   for(;;)
   {
      list = objects4next( list ) ;

      if( list == 0 )  return 0 ;

      obj_on =  (OBJ4 *) l4first( &list->list ) ;

      if( obj_on )  return obj_on ;
   }
}

OBJECTS4 * S4FUNCTION objects4first( REPORT4 *r4 )
{
   return &r4->page_header ;
}

OBJECTS4 * S4FUNCTION objects4next( OBJECTS4 *obj_list_on )
{
   REPORT4 *r4 ;
   GROUP4 *group ;

   if( obj_list_on == 0 )  return 0 ;

   r4 =  obj_list_on->report ;

   group =  obj_list_on->group ;

   if( group == 0 )
   {
      if( obj_list_on == &r4->page_header )   
         return &r4->page_footer ;

      if( obj_list_on == &r4->page_footer )   
         return &r4->title ;

      if( obj_list_on == &r4->title )   
         return &r4->summary ;

      group =  (GROUP4 *) l4first( &r4->groups ) ;

      if( group == 0 )  return 0 ;
         return &group->header ;
   }

   if( obj_list_on == &group->header )
      return &group->footer ;

   group =  (GROUP4 *) l4next( &r4->groups, group ) ;

   if( group == 0 )  return 0 ;
      return &group->header ;
}



void S4FUNCTION obj4free( OBJ4 *obj )
{
   OBJECTS4 *o4list =  obj->list ;
   if(o4list)
      l4remove( &o4list->list, obj ) ;

   u4free( ((TEXT4 *)obj)->date_format ) ;
   switch( obj->object_type )
   {
      case obj4label:
         u4free( ((TEXT4 *) obj)->ptr ) ;
         if(o4list)
            mem4free( o4list->report->text_memory, obj ) ;
         break ;

      case obj4expr:
         expr4free( ((TEXT4 *) obj)->expr ) ;
         if(o4list)
            mem4free( o4list->report->text_memory, obj ) ;
         break ;

      case obj4field:
         if(o4list)
            mem4free( o4list->report->text_memory, obj ) ;
         break ;
   }
}


