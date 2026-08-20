/* r4save.c   (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif
#define CREP_MASK 0xA5
#define VERSION_MASK 0x10

int retrieve4string( FILE4SEQ_READ *seq, char *buf, int buf_len)
{
   int str_len ;

   int rc = file4seq_read( seq, (char *) &str_len, sizeof(str_len) ) ;
   if( rc < 0 )
      return -1 ;
   if ( str_len >= buf_len )
   {
      e4( seq->file->code_base, e4result, E4_RESULT_INT ) ;
      return -1 ;
   }

   file4seq_read( seq, buf, str_len+1 ) ;
   return str_len ;
}

int retrieve_object_list( REPORT4 *r4, CODE4 *c4, OBJECTS4 *o4list,
FILE4SEQ_READ *seq )
{
   TEXT4 text_buf ;
   OBJECTS4 object_list_buf ;
   STYLE4  style_buf ;
   int len, dec ;
   char  str_buf[513] ;
   FIELD4 *f4 ;
   int rc ;
   TOTAL4 *tot ;
   GROUP4 *group_on ;
   TEXT4 *text ;
   DATA4 *d4 ;
   STYLE4 *style;

   rc = file4seq_read( seq, &object_list_buf, sizeof(object_list_buf) ) ;
   if( rc < 0 )
   {
      return -1 ;
   }

   objects4height( o4list, object_list_buf.height ) ;

   for(; object_list_buf.list.n_link-- >= 1; )
   {

      rc = file4seq_read( seq, (char *) &text_buf, sizeof(text_buf) ) ;
      if( rc < 0 )
      {
         e4(c4,e4report,E4_REPORT_RTEXT);
         return -1 ;
      }

      text =  0 ;
      file4seq_read( seq, (char *) &style_buf, sizeof(STYLE4) ) ;
      file4seq_read( seq, (char *) &len, sizeof(len) ) ;
      file4seq_read( seq, (char *) &dec, sizeof(dec) ) ;
      rc = retrieve4string( seq, (char *) str_buf, sizeof(str_buf) ) ;
      if(rc < 0 )
      {
         return rc ;
      }

      switch( text_buf.obj.object_type )
      {
         case obj4expr:
            text = text4expr( o4list, text_buf.obj.x, text_buf.obj.y, str_buf );
            break ;

         case obj4label:
            text = text4label( o4list, text_buf.obj.x, text_buf.obj.y, str_buf);
            break ;

         case obj4field:
            d4 =  d4data( c4, str_buf ) ;
            if ( d4 == 0 )
            {
               e4describe( c4, e4result, E4_RESULT_DAT, str_buf, (char *)0 );
               return -1 ;
            }

            retrieve4string( seq, str_buf, sizeof(str_buf) ) ;
            f4 =  d4field( d4, str_buf ) ;
            if ( f4 == 0 )
               break ;

            text =  text4field( o4list, text_buf.obj.x, text_buf.obj.y, f4 ) ;
            break ;

         case obj4total:
            tot =  total4lookup( r4, str_buf ) ;
            if( tot )
               text =  text4total( o4list, text_buf.obj.x, text_buf.obj.y, tot ) ;
            break ;
      }

      if( text )
      {
         style =  style4lookup( o4list->report, style_buf.name ) ;
         text4style( text, style ) ;
         text4alignment( text, text_buf.alignment ) ;
         text4len_max( text, text_buf.len_max ) ;
         ((OBJ4 *)text)->w = text_buf.obj.w;
         text4dec( text, text_buf.dec ) ;
         text4use_brackets( text, text_buf.use_brackets ) ;
         text4display_zero( text, text_buf.display_zero ) ;
         text4numeric( text, text_buf.numeric_type ) ;
         ((OBJ4 *)text)->display_status = text_buf.obj.display_status;
      }
      if( text_buf.reset_group)
      {
         rc = retrieve4string( seq, str_buf, sizeof(str_buf) ) ;
         if( rc < 0 )
         {
            return rc ;
         }
         group_on =  group4lookup( r4, str_buf ) ;
         if(text)
            text->reset_group = group_on ;
      }
      if( text_buf.date_format )
      {
         retrieve4string( seq, str_buf, sizeof(str_buf) ) ;
         if(text)
            text4date_format( text, str_buf ) ;
      }
   }
   
   return 0 ;
}

void report4cleanup(RELATE4 *relate, REPORT4 *report,FILE4 *file,int close_files)
{
   if(report)
   {
      report4free( report, 1, close_files ) ;
   }
   else
   {
      relate4free( relate, close_files );
   }
   file4close( file ) ;
}


static int save4string( FILE4SEQ_WRITE *seq, char *ptr )
{
   int str_len =  strlen(ptr) ;

   file4seq_write( seq, &str_len, sizeof(str_len) ) ;
   file4seq_write( seq, ptr, str_len+1 ) ;

   return 0 ;
}

static int save4ecode( FILE4SEQ_WRITE *seq,char *ptr, int len )
{
   file4seq_write( seq, &len, sizeof(len) ) ;
   file4seq_write( seq, ptr, len+1 ) ;
   return 0 ;

}

static int save_object_list( OBJECTS4 *o4list, FILE4SEQ_WRITE *seq )
{
   OBJ4 *obj ;
   TEXT4 *text ;
   file4seq_write( seq, o4list, sizeof(OBJECTS4) ) ;

   obj = 0;
   obj = (OBJ4 *)l4first( &o4list->list);
   while(obj)
   {
      file4seq_write( seq, obj, sizeof(TEXT4) ) ;

      text =  (TEXT4 *) obj ;

      switch( obj->object_type )
      {
         case obj4expr:
         case obj4label:
         case obj4field:
         case obj4total:
            file4seq_write( seq, text->style, sizeof(STYLE4) ) ;
            file4seq_write( seq, &text->len, sizeof(text->len) ) ;
            file4seq_write( seq, &text->dec, sizeof(text->dec) ) ;
         break ;
      }

      switch( obj->object_type )
      {
         case obj4expr:
            save4string( seq, expr4source( text->expr) ) ;
            break ;

         case obj4label:
            save4string( seq, text->ptr ) ;
            break ;

         case obj4field:
            save4string( seq, d4alias( text->field->data) ) ;
            save4string( seq, f4name( text->field) ) ;
            break ;

         case obj4total:
            save4string( seq, text->total->calc_ptr->name ) ;
            break ;
      }
      if(text->reset_group)
         save4string( seq,text->reset_group->label ) ;

      if( text->date_format )
         save4string( seq, text->date_format ) ;
   
      obj = (OBJ4 *) l4next( &o4list->list, obj); 
      
   }
   return 0 ;
}

int S4FUNCTION report4save( REPORT4 *report, char *file_name )
{
   FILE4 file ;
   char name_buf[258] ;
   int rc ;
   FILE4SEQ_WRITE seq ;
   char buf[2048] ;
   RELATE4 *relate_on ;
   int code, i ;
   char *rname, *tname ;
   STYLE4 *style_on ;
   GROUP4 *group_on ;
   EXPR4CALC *calc_on ;
   INDEX4 *index_on ;
   int str_len;
   TAG4 *tag_on;

   while( report->relate->master )
      report->relate = report->relate->master;

   u4ncpy( name_buf, file_name, sizeof(name_buf) ) ;
   u4name_ext( name_buf, sizeof(name_buf), "REP", 0 ) ;

   if( (rc = file4create( &file, report->cb, name_buf, 1 )) != 0 )
   {
      e4set( report->cb, 0 ) ;
      return rc ;
   }

   file4seq_write_init( &seq, &file, 0L, buf, sizeof(buf) ) ;

   for(rname = name_buf + (strlen(name_buf)-1);
       rname != name_buf; rname--)
      if(*rname == '\\') break;

   tname = (char *)u4alloc_free(report->cb,strlen(rname)+1);
   if( tname != NULL )
   {
      if(report->report_name)
         u4free(report->report_name);
      report->report_name = tname;
      strcpy(report->report_name,rname);
   }

   name_buf[0] = (char)CREP_MASK;
   name_buf[1] = VERSION_MASK;
   file4seq_write( &seq, name_buf,2);
   
   /* save for 'relate' first */
   for( relate_on = report->relate; relate_on != 0; )
   {
      save4string( &seq, relate_on->data->file.name ) ;

      file4seq_write( &seq, relate_on, sizeof(RELATE4) ) ;

      file4seq_write( &seq, &relate_on->data->indexes.n_link,sizeof(relate_on->data->indexes.n_link));
      index_on = (INDEX4 *) l4first( &relate_on->data->indexes );
      while(index_on)
      {
         #ifdef N4OTHER
         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &index_on->tags, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            save4string( &seq, tag_on->file.name );
         }

         #else
            save4string( &seq, index_on->file.name );
         #endif
         index_on = (INDEX4 *)l4next( &relate_on->data->indexes, index_on );
      }
      if ( relate_on->data_tag != 0 )
      {
         save4string( &seq, relate_on->data_tag->alias ) ;
      }
      if ( relate_on->master_expr != 0 )
         save4string( &seq, expr4source( relate_on->master_expr) ) ;

      code = relate4next( &relate_on ) ;

      file4seq_write( &seq, &code, sizeof(code) ) ;
   }


   file4seq_write( &seq, report, sizeof(REPORT4) ) ;

   style_on = 0;
   style_on = (STYLE4 *)l4first(&report->styles);
   while(style_on)
   {
      file4seq_write( &seq, style_on, sizeof(STYLE4) ) ;

      if(style_on->codes_before_len > 0)
         save4ecode( &seq,style_on->codes_before,style_on->codes_before_len );

      if(style_on->codes_after_len > 0)
         save4ecode( &seq, style_on->codes_after, style_on->codes_after_len );
      style_on=(STYLE4 *)l4next(&report->styles,style_on);
   }

   save4string( &seq, ((STYLE4 *)report->styles.selected)->name ) ;

   group_on = 0;
   group_on = (GROUP4 *)l4first(&report->groups);
   while(group_on)
   {
      /* First make sure the group label is unique */
      i = 1 ;
      while( group4lookup( report, group_on->label ) != group_on )
         c4ltoa45( i++, group_on->label, sizeof(group_on->label)-1 ) ;
         
      file4seq_write( &seq, group_on, sizeof(GROUP4) ) ;
      if ( group_on->expr == 0 )
         save4string( &seq, "" ) ;
      else
         save4string( &seq, expr4source( group_on->expr) ) ;
      group_on=(GROUP4 *)l4next(&report->groups,group_on);

   }

   file4seq_write(&seq,&report->cb->calc_list.n_link,
      sizeof(report->cb->calc_list.n_link) ) ;
   
   calc_on = 0;
   calc_on = (EXPR4CALC *)l4first(&report->cb->calc_list);
   while(calc_on)
   {
      file4seq_write( &seq, calc_on, sizeof(EXPR4CALC) ) ;
      if( calc_on->total != 0 )
      {
         file4seq_write( &seq, calc_on->total, sizeof(TOTAL4) ) ;
         if( calc_on->total->reset_level == 0 )
            save4string( &seq, "" ) ;
         else
            save4string( &seq, calc_on->total->reset_level->label ) ;
      }
      save4string( &seq, expr4source(calc_on->expr) ) ;
      calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);

   }

   group_on = 0;
   group_on = (GROUP4 *) l4next( &report->groups, group_on);
   while(group_on)
   {
      save_object_list( &group_on->header, &seq ) ;
      save_object_list( &group_on->footer, &seq ) ;
      group_on = (GROUP4 *) l4next( &report->groups, group_on);

   }

   save_object_list( &report->page_header, &seq ) ;
   save_object_list( &report->page_footer, &seq ) ;
   save_object_list( &report->title, &seq ) ;
   save_object_list( &report->summary, &seq ) ;

   if( ((RELATION4 *)report->relate->relation)->expr_source )
      save4string( &seq,
        ((RELATION4 *)report->relate->relation)->expr_source ) ;
   else
     save4string( &seq, "" ) ;

   if( ((RELATION4 *)report->relate->relation)->sort_source )
      save4string( &seq,
         ((RELATION4 *)report->relate->relation)->sort_source ) ;
   else
      save4string( &seq, "" ) ;

   str_len =  strlen(report->report_name) ;
   file4seq_write( &seq, &str_len, sizeof(str_len) ) ;
   file4seq_write( &seq, report->report_name, str_len+1 ) ;

   file4seq_write_flush( &seq ) ;
   file4close( &file ) ;
   return 0 ;
}

#ifdef S4VB_DOS

int report4save_v( REPORT4 *r4, char *file_name )
{
   return report4save( r4, c4str(file_name) ) ;
}

#endif
