/* r4code.c   (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif

int numtotals, numlabels, numexprs, for_windows;
int onumtotals, onumlabels, onumexprs;

void report4code_out( FILE4SEQ_WRITE *, char * );
void report4code_nl( FILE4SEQ_WRITE * );
void report4code_expr( FILE4SEQ_WRITE *, char * );
int  report4stolower( char * );
int  objects4code( FILE4SEQ_WRITE *, OBJECTS4 *, char * );
void report4code_relatev( FILE4SEQ_WRITE *, REPORT4 * );
void report4code_textv( FILE4SEQ_WRITE *, OBJECTS4 * );
void report4code_relate( FILE4SEQ_WRITE *, RELATE4 * );
void report4code_stylev( FILE4SEQ_WRITE *, REPORT4 * );
void report4code_groupv( FILE4SEQ_WRITE *, REPORT4 * );
void report4code_calcv(  FILE4SEQ_WRITE *, REPORT4 * );
void report4code_styles( FILE4SEQ_WRITE *, REPORT4 * );
void report4code_groups( FILE4SEQ_WRITE *, REPORT4 * );
void report4code_calcs ( FILE4SEQ_WRITE *, REPORT4 * );
void report4code_filename( char *, char * );

void report4code_filepath( char *buf, char *file_name )
{
   int i, j;

   for( i = 0, j = 0; i < strlen(file_name); i++,j++)
      {
      buf[j] = file_name[i];
      if( file_name[i] == '\\' )
         {
         j++;
         buf[j] = '\\';
         }
      }
   buf[j] = '\0';
}


void report4code_filename( char *buf, char *file_name )
{
   int i, j = 0, k = 0;

   i = strlen(file_name) - 1;
   while( file_name[i] != ':' && file_name[i] != '\\' && i >= 0 )
   {
      i--;
      j++;
   }

   i++;

   while( k <= j )
   {
      buf[k] = file_name[i];
      k++;
      i++;
   }

}

int S4FUNCTION report4program( REPORT4 *report, char *file_name, char *funcname, int for_wind )
{
   FILE4 file ;
   FILE4SEQ_WRITE seq;
   EXPR4CALC *calc_on;
   GROUP4 *group_on;
   char fname_buf[256], tname_buf[256] ;
   char buf[2048], outstring[512];
   int i, groupnum ;

   for_windows = for_wind;

   numtotals = numlabels = numexprs = 0;
   onumtotals = onumlabels = onumexprs = 0;
   u4ncpy( fname_buf, file_name, sizeof(fname_buf) ) ;
   u4name_ext( fname_buf, sizeof(fname_buf), "C", 0 ) ;

   report->cb->safety = 0;
   if( file4create(&file, report->cb, fname_buf,0) != 0)
   {
      e4set(report->cb,0);
      return -1 ;
   }
   report->cb->safety = 1;

   file4seq_write_init( &seq, &file, 0L, buf, sizeof(buf));
   report4code_out(&seq,"#include \"d4all.h\"" );
   report4code_nl( &seq );
   report4code_nl( &seq );

   M4PRINT( outstring, "REPORT4 *%s( CODE4 *code_base, int open_files )",funcname);
   report4code_out(&seq,outstring);
   report4code_nl( &seq );
   report4code_out(&seq,"{");
   report4code_nl( &seq );

   report4code_relatev( &seq, report ); 

   if(for_windows == 1)
   {
      report4code_out(&seq,"   R4LOGFONT lpLogFont;");
      report4code_nl(&seq);
   }

   if(for_windows == 2)
   {
      report4code_out( &seq,"   #ifdef S4WINDOWS");
      report4code_nl( &seq );
      report4code_out( &seq,"      R4LOGFONT lpLogFont;");
      report4code_nl( &seq );
      report4code_out(&seq,"   #endif");
      report4code_nl( &seq );
   }

   report4code_stylev(&seq, report);

   report4code_textv(&seq, &report->page_header);
   report4code_textv(&seq, &report->page_footer);
   report4code_textv(&seq, &report->title);
   report4code_textv(&seq, &report->summary);
   
   report4code_groupv( &seq, report );
   report4code_calcv ( &seq, report );

   report4code_out(&seq,"   REPORT4 *report ;");
   report4code_nl( &seq );report4code_nl( &seq );report4code_nl( &seq );
   report4code_out( &seq,"   code_base->auto_open = 0;");
   report4code_nl( &seq );
   report4code_relate( &seq, report->relate);


   report4code_nl( &seq );
   report4code_filename( fname_buf,report->relate->data->file.name);
   report4stolower(fname_buf);
   for(i = 0; (unsigned) i < strlen(fname_buf); i++ )
      if(fname_buf[i] == '.')
         fname_buf[i] = '\0';
   M4PRINT(outstring,"   report = report4init(rel%s);",fname_buf);
   report4code_out( &seq, outstring );
   report4code_nl( &seq );report4code_nl( &seq );

/* global settings */
   M4PRINT(outstring,"   report->margin_top = %d;",report->margin_top);
   report4code_out( &seq, outstring );
   report4code_nl( &seq );

   M4PRINT(outstring,"   report->margin_bottom = %d;",report->margin_bottom);
   report4code_out( &seq, outstring );
   report4code_nl( &seq );

   M4PRINT(outstring,"   report->margin_left = %d;",report->margin_left);
   report4code_out( &seq, outstring );
   report4code_nl( &seq );

   M4PRINT(outstring,"   report->margin_right = %d;",report->margin_right);
   report4code_out( &seq, outstring );
   report4code_nl( &seq );

   M4PRINT(outstring,"   report->report_width = %d;",report->report_width);
   report4code_out( &seq, outstring );
   report4code_nl( &seq );

   if( report->decimal_point != '.' || report->thousand_separator != ',')
   {
      M4PRINT(outstring,
      "   report4symbols_numeric(report,\'%c\',\'%c\');",
      report->thousand_separator,report->decimal_point);
      if( report->thousand_separator == '\0' )
         M4PRINT(outstring,
         "   report4symbols_numeric(report, 0,\'%c\');",
         report->decimal_point);
      if( report->decimal_point == '\0' )
        M4PRINT(outstring,
         "   report4symbols_numeric(report,\'%c\',0);",
         report->thousand_separator);
      if( report->decimal_point == '\0' && report->thousand_separator == '\0' )
         M4PRINT(outstring,
         "   report4symbols_numeric(report,0,0);");
      report4code_out( &seq, outstring );
      report4code_nl( &seq );
   }

   if(report->currency_sym != '$')
   {
      M4PRINT(outstring,"   report4currency(report,\'%c\');",report->currency_sym);
      report4code_out( &seq, outstring );
      report4code_nl( &seq );
   }

   if(report->leading_zero)
      M4PRINT(outstring,"   report->leading_zero = 1;");
   else
      M4PRINT(outstring,"   report->leading_zero = 0;");
   report4code_out( &seq, outstring );
   report4code_nl( &seq );
   report4code_nl( &seq );

   report4code_calcs( &seq, report );
   report4code_nl( &seq );

   report4code_styles( &seq, report );

   if(report->page_header.height > 0)
   {                             
      M4PRINT(outstring,"   objects4height(&report->page_header,%d);",report->page_header.height);
      report4code_out(&seq,outstring);
      report4code_nl( &seq );
      objects4code( &seq,&report->page_header,"report->page_header");
   }

   if(report->page_footer.height > 0)
   {
      M4PRINT(outstring,"   objects4height(&report->page_footer,%d);",report->page_footer.height);
      report4code_out(&seq,outstring);
      report4code_nl( &seq );
      objects4code( &seq, &report->page_footer,"report->page_footer");
   }

   if(report->title.height > 0)
   {
      M4PRINT(outstring,"   objects4height(&report->title,%d);\n",report->title.height);
      report4code_nl( &seq );
      report4code_out(&seq,outstring);
      objects4code( &seq, &report->title,"report->title");
   }
   
   if(report->summary.height >= 0)
   {
      M4PRINT(outstring,"   objects4height(&report->summary,%d);",report->summary.height);
      report4code_out(&seq,outstring);
      report4code_nl( &seq );
      objects4code( &seq, &report->summary,"report->summary");
   }
   report4code_nl( &seq );report4code_nl( &seq );

   report4code_groups( &seq, report );

   calc_on = 0;
   calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);
   while(calc_on)
   {
      strcpy( fname_buf, calc_on->name ) ;
      report4stolower( fname_buf ) ;
      groupnum = 0;
      group_on = 0;
      group_on = (GROUP4 *)l4next(&report->groups,group_on);
      while(group_on)
      {
         strcpy( tname_buf, "g");
         strcat( tname_buf, group_on->label ) ;
         report4stolower( tname_buf ) ;
         if( strlen( tname_buf ) == 1 && tname_buf[0] == ' ' )
         {
            M4PRINT( tname_buf, "group%d", groupnum);
            groupnum++;
         }
         if(calc_on->total && calc_on->total->reset_level == group_on)
         {
            M4PRINT(outstring,"   total4reset_level(%s,%s);",fname_buf,tname_buf);
            report4code_out(&seq,outstring);
            report4code_nl( &seq );
         }
         group_on = (GROUP4 *)l4next(&report->groups,group_on);
      }
      calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);
   }


   if(!for_windows)
   {
      M4PRINT(outstring,"   report4page_size(report,%d,%d);",report->swidth,
         report->sheight);
      report4code_out(&seq,outstring);
      report4code_nl( &seq );
      M4PRINT(outstring,"   report4output(report,1,NULL,%d);",report->use_styles);
      report4code_out( &seq, outstring );
      report4code_nl( & seq );
   }
   else
   {
      if(report->printer_name)
      {
         M4PRINT(outstring,"   report4output(report,%d,%s,1);",report->to_screen,
            report->printer_name);
         report4code_out( &seq, outstring );
         report4code_nl( &seq );
      }
      else
      {
         M4PRINT(outstring,"   report4output(report,%d,NULL,1);",report->to_screen);
         report4code_out( &seq, outstring );
         report4code_nl( &seq );
      }
   }
   
   report4code_nl( &seq );
   report4code_out(&seq,"   return(report);");
   report4code_nl( &seq );
   report4code_out(&seq,"}");
   report4code_nl( &seq );
   file4seq_write_flush(&seq);
   file4close(&file);
   return 0 ;
}

void report4code_out(FILE4SEQ_WRITE *seq, char *string)
{


   file4seq_write(seq,string,strlen(string));

}

void report4code_nl(FILE4SEQ_WRITE *seq)
{
   char out[4];

   out[0] = '\r';
   out[1] = '\n';
   file4seq_write(seq,out,2);
}

void report4code_expr(FILE4SEQ_WRITE *seq, char *expr)
{
   int len = strlen(expr);
   int i;
   char outstring[3];

   M4PRINT(outstring,"\"");
   file4seq_write(seq,outstring,1);
   for( i = 0; i < len; i++)
   {
      if( *(expr + i) == 0x22 || *(expr + i) == 0x27 )
      {
         M4PRINT(outstring,"\\");
         file4seq_write(seq,outstring,1);
      }
      file4seq_write(seq,expr+i,1);
   }
   M4PRINT(outstring,"\"");
   file4seq_write(seq,outstring,1);

}

int report4stolower( char *string )
{
   int i, len ;

   len = strlen( string ) ;
   for( i = 0; i < len; i++ )
      if( string[i] > 64 && string[i] < 91 )
         string[i] = string[i] + (char) 32 ;
   return 0;
}

int objects4code( FILE4SEQ_WRITE *seq, OBJECTS4 *o4list, char *group_name )
{
   unsigned i ;
   OBJ4 *obj ;
   TEXT4 *text ;
   char name_buf[256], gname_buf[256], sname_buf[256], outstring[512];
   int numgroups ;
   GROUP4 *group_on ;

   obj = 0;
   obj = (OBJ4 *) l4next( &o4list->list, obj);
   while(obj)
   {
      text =  (TEXT4 *) obj ;

      switch( obj->object_type )
      {
         case obj4expr:
            M4PRINT( outstring, "   textexpr%d = text4expr(&%s,%d,%d,",
            onumexprs,group_name,obj->x,obj->y);
            report4code_out( seq, outstring );
            report4code_expr( seq, expr4source(text->expr));
            report4code_out(seq, ");");
            report4code_nl(seq);
            M4PRINT(name_buf,"textexpr%d",onumexprs);
            onumexprs++;
            break ;

         case obj4label:
            M4PRINT(outstring,"   label%d = text4label(&%s,%d,%d,\"%s\");",
            onumlabels,group_name,obj->x,obj->y,text->ptr);
            report4code_out( seq, outstring );
            report4code_nl(seq);
            M4PRINT(name_buf,"label%d",onumlabels);
            onumlabels++;
            break ;

         case obj4field:
            break ;

         case obj4total:
            strcpy(name_buf,text->total->calc_ptr->name);
            report4stolower(name_buf);
            M4PRINT( outstring,"   texttotal%d = text4total(&%s,%d,%d,%s );",
            onumtotals,group_name,obj->x,obj->y,name_buf);
            report4code_out( seq, outstring );
            report4code_nl( seq );
            M4PRINT(name_buf,"texttotal%d",onumtotals);
            onumtotals++;
            break ;
      }

      numgroups = 0;
      group_on = 0;
      group_on = (GROUP4 *)l4next(&o4list->report->groups,group_on);
      while(group_on)
      {
         strcpy(gname_buf,"g");
         strcat(gname_buf, group_on->label);
         report4stolower(gname_buf);
         if(strlen(gname_buf)==1 && gname_buf[0] == ' ')
         {
            M4PRINT(gname_buf,"group%d",numgroups);
            numgroups++;
         }

         if(group_on == text->reset_group)
         {
            M4PRINT(outstring,"   text4display_once(%s,%s);",name_buf,gname_buf);
            report4code_out( seq, outstring );
            report4code_nl( seq );
         }
         group_on = (GROUP4 *)l4next(&o4list->report->groups,group_on);

      }

      if( text->date_format )
      {
         M4PRINT( outstring, "   text4date_format(%s,\"%s\");",name_buf,text->date_format);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }
      if(text->alignment != text4left)
      {
         M4PRINT( outstring, "   text4alignment(%s,",name_buf);
         switch( text->alignment)
         {
            case text4left:
               strcat(outstring,"text4left");
               break;
          
            case text4right:
               strcat(outstring, "text4right" );
               break;

            case text4center:
               strcat( outstring, "text4center" );
               break;
         }
         strcat(outstring,");");
         report4code_out( seq, outstring );
         report4code_nl( seq );
         M4PRINT( outstring, "   text4dec(%s,%d);",name_buf,text->dec);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }

      if(text->display_zero != 1)
      {
         M4PRINT( outstring, "   text4display_zero(%s,%d);",name_buf,text->display_zero);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }

      if(text->len_max)
      {
         M4PRINT( outstring, "   text4len_max(%s,%d);",name_buf,text->len_max);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }

      if(text->use_brackets)
      {
         M4PRINT( outstring, "   text4use_brackets(%s,%d);",name_buf,text->use_brackets);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }


      M4PRINT( outstring, "   text4width(%s,%d);",name_buf,text->len);
      report4code_out( seq, outstring );
      report4code_nl( seq );

      if(text->numeric_type != text4number)
      {
         M4PRINT( outstring, "   text4numeric(%s,",name_buf);
         switch(text->numeric_type)
         {
            case text4number:
               strcat(outstring,"text4number");
               break;
    
            case text4exponent:
               strcat(outstring,"text4exponent");
               break;

            case text4dollar:
               strcat(outstring,"text4dollar");
               break;

            case text4percent:
               strcat(outstring,"text4percent");
               break;

         }
         strcat(outstring,");");
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }

      if(text->style)
      {
         strcpy(sname_buf,text->style->name);
         report4stolower(sname_buf);
         for(i = 0; i < strlen(sname_buf); i++)
            if(sname_buf[i] == ' ')
               sname_buf[i] = '_';

         M4PRINT( outstring, "   text4style(%s,%s);",name_buf,sname_buf);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }
      obj = (OBJ4 *) l4next( &o4list->list, obj);

   }
   return 0 ;
}

void report4code_relatev( FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   char fname_buf[256], fname2_buf[256];
   RELATE4 *relate_on;
   INDEX4 *index_on;
   TAG4 *tag_on;
   unsigned i ;

   relate_on = report->relate;
   while(relate_on)
   {
      report4code_filename( fname_buf, relate_on->data->file.name );
      report4stolower( fname_buf ) ;
      for( i = 0; i < strlen(fname_buf); i++)
         if(fname_buf[i] == '.')
            fname_buf[i] = '\0';

      report4code_out(seq,"   DATA4 *");
      report4code_out(seq, fname_buf ) ;
      report4code_out(seq, " ;" ) ;
      report4code_nl( seq );

      report4code_out( seq, "   RELATE4 *rel" ) ;
      report4code_out( seq, fname_buf ) ;
      report4code_out( seq, " ;" ) ;
      report4code_nl( seq );

      index_on = (INDEX4 *)l4first( &relate_on->data->indexes);
      while( index_on )
      {
         #ifdef N4OTHER
            tag_on = (TAG4 *)l4first( &index_on->tags );
            report4code_filename(fname_buf,tag_on->file.name ) ;
         #else
            report4code_filename(fname_buf,index_on->file.name ) ;
         #endif
         report4stolower( fname_buf ) ;
         for( i = 0; i < strlen(fname_buf); i++)
            if(fname_buf[i] == '.')
               fname_buf[i] = '\0';

         report4code_out(seq, "   INDEX4 *ind" ) ;
         report4code_out(seq, fname_buf ) ;
         report4code_out(seq, " ;" ) ;
         report4code_nl( seq );

         if( relate_on->data_tag)
            if(relate_on->data_tag->index == index_on )
            {
               strcpy(fname2_buf,relate_on->data_tag->alias) ;
               report4stolower( fname2_buf ) ;
               for( i = 0; i < strlen(fname2_buf); i++)
                  if(fname2_buf[i] == '.')
                     fname2_buf[i] = '\0';

               report4code_out(seq, "   TAG4 *tag" ) ;
               report4code_out(seq, fname_buf ) ;
               report4code_out(seq, fname2_buf ) ;
               report4code_out(seq, " ;" );
               report4code_nl( seq );
            }
         index_on = (INDEX4 *)l4next( &relate_on->data->indexes, index_on );
      }

      relate4next( &relate_on ) ;
   }


}

void report4code_textv(FILE4SEQ_WRITE *seq, OBJECTS4 *objs)
{
   OBJ4 *obj_on;
   char outstring[512];

   obj_on = 0;
   obj_on = (OBJ4 *)l4next(&objs->list,obj_on);
   while(obj_on)
   {
      switch(obj_on->object_type)
      {
         case obj4label:
            M4PRINT(outstring,"   TEXT4 *label%d;",numlabels);
            report4code_out(seq,outstring);
            report4code_nl( seq );
            numlabels++;
            break;

         case obj4expr:
            M4PRINT(outstring,"   TEXT4 *textexpr%d;",numexprs);
            report4code_out(seq, outstring);
            report4code_nl( seq );
            numexprs++;
            break;

         case obj4total:
            M4PRINT(outstring,"   TEXT4 *texttotal%d;",numtotals);
            report4code_out(seq,outstring);
            report4code_nl( seq );
            numtotals++;
            break;
      }
      obj_on = (OBJ4 *)l4next(&objs->list,obj_on);
   }

}

void report4code_relate(FILE4SEQ_WRITE *seq, RELATE4 *relate)
{
   RELATE4 *relate_on, *master;
   INDEX4 *index_on;
   char    outstring[512], name_buf[256], iname_buf[256], tname_buf[256], iname2_buf[256];
   char path_buf[512];
   int first, code ;
   unsigned i;
   TAG4 *tag_on;

   for(relate_on = relate; relate_on != 0; )
   {
      report4code_filename(name_buf,relate_on->data->file.name);
      report4stolower( name_buf ) ;
      for( i = 0; i < strlen(name_buf); i++)
         if(name_buf[i] == '.')
            name_buf[i] = '\0';

      report4code_filepath( path_buf, relate_on->data->file.name );

      report4code_out(seq,"   if( open_files )");
      report4code_nl(seq);
      M4PRINT( outstring, "      %s = d4open(code_base,\"%s\");",name_buf,
         path_buf );
      report4code_out(seq,outstring);
      report4code_nl( seq );
      report4code_out( seq, "   else" );
      report4code_nl( seq );
      M4PRINT( outstring,"      %s = d4data( code_base,\"%s\");",name_buf,
         d4alias(relate_on->data));
      report4code_out( seq, outstring );
      report4code_nl( seq );
      report4code_nl( seq );
      M4PRINT(outstring,"   if( %s == NULL ) return 0;",name_buf);
      report4code_out(seq,outstring);
      report4code_nl( seq );
      report4code_nl( seq );

      index_on = (INDEX4 *)l4first( &relate_on->data->indexes);
      while( index_on )
      {
         #ifdef N4OTHER
            tag_on = (TAG4 *)l4first( &index_on->tags );
            report4code_filename(iname_buf,tag_on->file.name ) ;
            report4code_filepath(path_buf, tag_on->file.name );
         #else
            report4code_filename(iname_buf,index_on->file.name ) ;
            report4code_filepath(path_buf, index_on->file.name );
         #endif
         report4stolower( iname_buf ) ;
         for( i = 0; i < strlen(iname_buf); i++)
            if(iname_buf[i] == '.')
               iname_buf[i] = '\0';

         report4code_out( seq, "   if( open_files )");
         report4code_nl( seq );
         report4code_out( seq, "   {");
         report4code_nl( seq );
         #ifdef N4OTHER
            M4PRINT(outstring,"      ind%s = i4open( %s, \"%s\" ) ;",
               iname_buf,name_buf,path_buf);
         #else
            M4PRINT(outstring,"      ind%s = i4open( %s, \"%s\" ) ;",
               iname_buf,name_buf,path_buf);
         #endif
         report4code_out(seq, outstring);
         report4code_nl( seq );
         M4PRINT(outstring,"      if(ind%s == NULL) return 0;",iname_buf);
         report4code_out( seq, outstring );
         report4code_nl( seq );
         report4code_out( seq, "   }");
         report4code_nl( seq );
         report4code_nl( seq );

         if( relate_on->data_tag)
            if( relate_on->data_tag->index == index_on )
            {
               strcpy(tname_buf,relate_on->data_tag->alias);
               report4stolower( tname_buf ) ;
               for( i = 0; i < strlen(tname_buf); i++)
                  if(tname_buf[i] == '.')
                     tname_buf[i] = '\0';
         
               M4PRINT(outstring,"   tag%s%s = d4tag( %s,\"%s\" ) ;",
                  iname_buf, tname_buf,name_buf,relate_on->data_tag->alias);
               report4code_out(seq,outstring);
               report4code_nl( seq );
               M4PRINT(outstring,"   if( tag%s%s == NULL ) return 0;",iname_buf,tname_buf);
               report4code_out( seq, outstring );
               report4code_nl( seq );
               M4PRINT(outstring,"    d4tag_select( %s, tag%s%s ) ;",name_buf,iname_buf,tname_buf);
               report4code_out(seq,outstring);
               report4code_nl( seq );
            }
         index_on = (INDEX4 *)l4next(&relate_on->data->indexes,index_on);
      }
      report4code_nl( seq );
      relate4next( &relate_on ) ;
   }

   report4code_nl( seq );

   first = 1 ;
   for(relate_on = relate; relate_on != 0; )
   {
      if(first)
      {
         first = 0;
         report4code_filename(name_buf,relate_on->data->file.name);
         report4stolower(name_buf);
         for( i = 0; i < strlen(name_buf); i++)
            if(name_buf[i] == '.')
               name_buf[i] = '\0';
 
         M4PRINT(outstring,"   rel%s = relate4init(%s);",name_buf,name_buf);
         report4code_out(seq,outstring);
         report4code_nl( seq );

      }
      else
      {
         report4code_filename(name_buf,relate_on->data->file.name);
         report4stolower(name_buf);
         for( i = 0; i < strlen(name_buf); i++)
            if(name_buf[i] == '.')
               name_buf[i] = '\0';
 
         report4code_filename(tname_buf,master->data->file.name);
         report4stolower(tname_buf);
         for( i = 0; i < strlen(tname_buf); i++)
            if(tname_buf[i] == '.')
               tname_buf[i] = '\0';

         M4PRINT( outstring, "   rel%s = relate4create_slave(rel%s,%s,",
            name_buf,tname_buf,name_buf);
         report4code_out( seq, outstring );
         report4code_expr( seq, expr4source(relate_on->master_expr));
         M4PRINT(outstring,",d4tag_selected(%s));",name_buf);
         report4code_out( seq, outstring );
         report4code_nl( seq );

         if(expr4type(relate_on->master_expr) != r4num && expr4type(relate_on->master_expr) != r4num_doub )
         {
            M4PRINT( outstring,"   relate4match_len(rel%s,%d);",name_buf,relate_on->match_len);
            report4code_out( seq, outstring );
            report4code_nl( seq );
         }

         if(relate_on->relation_type != relate4exact)
         {
            M4PRINT( outstring, "   relate4type(rel%s,",name_buf);
            switch( relate_on->relation_type)
            {
               case relate4exact:
                  strcat(outstring,"relate4exact");
                  break;

               case relate4scan:
                  strcat(outstring,"relate4scan");
                  break;
           
               case relate4approx:
                  strcat(outstring,"relate4approx");
                  break;
            }
            strcat(outstring,");");
            report4code_out( seq, outstring );
            report4code_nl( seq );
         }

         if(relate_on->error_action != relate4blank)
         {
            M4PRINT( outstring, "   relate4error_action(rel%s,",name_buf);
            switch( relate_on->error_action)
            {
               case relate4blank:
                  strcat(outstring,"relate4blank");
                  break;

               case relate4skip_rec:
                  strcat(outstring,"relate4skip_rec");
                  break;

               case relate4terminate:
                  strcat(outstring,"relate4terminate");
                  break;
            }
            strcat(outstring,");");
            report4code_out( seq, outstring );
            report4code_nl( seq );
         }
      }
      master = relate_on;
      code = relate4next(&relate_on);
      if(code == 2) break ;
      while(code++ <= 0)
         master =  master->master;
      report4code_nl( seq );
   }

   if(relate->relation->expr_source != 0)
   {
      M4PRINT(outstring,"   relate4query_set(rel%s,",name_buf);
      report4code_out( seq, outstring );
      report4code_expr(seq,relate->relation->expr_source);
      report4code_out(seq,");");
      report4code_nl( seq );
   }

   if(relate->relation->sort_source != 0)
   {
      M4PRINT(outstring,"   relate4sort_set(rel%s,",name_buf);
      report4code_out( seq, outstring );
      report4code_expr( seq, relate->relation->sort_source);
      report4code_out(seq,");");
      report4code_nl( seq );
   }

}

void report4code_stylev( FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   STYLE4 *style_on ;
   char   sname_buf[256];
   unsigned i;

   style_on = 0;
   style_on=(STYLE4 *)l4next(&report->styles,style_on);
   while(style_on)
   {
      strcpy( sname_buf, style_on->name ) ;
      report4stolower( sname_buf ) ;
      for(i=0;i < strlen(sname_buf);i++)
         if(sname_buf[i] == ' ')
            sname_buf[i] = '_';

      report4code_out( seq, "   STYLE4 *" ) ;
      report4code_out( seq, sname_buf ) ;
      report4code_out( seq," ;");
      report4code_nl( seq );
      style_on=(STYLE4 *)l4next(&report->styles,style_on);
   }

}

void report4code_groupv( FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   char gname_buf[256], outstring[512];
   GROUP4 *group_on ;
   int groupnum;

   groupnum = 0 ;
   group_on = 0;
   group_on = (GROUP4 *) l4next( &report->groups, group_on);

   while(group_on)
   {            
      report4code_out( seq, "   GROUP4 *" ) ;
      strcpy( gname_buf, "g");
      strcat(gname_buf,group_on->label ) ;
      report4stolower( gname_buf ) ;
      if( strlen( gname_buf ) == 1 && gname_buf[0] == ' ' )
      {
         M4PRINT(outstring,"group%d;", groupnum);
         report4code_out( seq, outstring);
         report4code_nl( seq );
         if(group_on->expr)
         {
            M4PRINT(outstring, "   EXPR4 *exprgroup%d;",groupnum);
            report4code_out( seq, outstring );
            report4code_nl( seq );
         }
         groupnum++;
      }
      else
      {
            M4PRINT(outstring,"%s;", gname_buf ) ;
            report4code_out(seq,outstring);
            report4code_nl( seq );
            if(group_on->expr)
            {
               M4PRINT(outstring,"   EXPR4 *expr%s;",gname_buf);
               report4code_out( seq, outstring);
               report4code_nl( seq );
            }
      }
      
      report4code_textv(seq, &group_on->header);
      report4code_textv(seq, &group_on->footer);
      group_on = (GROUP4 *) l4next( &report->groups, group_on);

   }


}

void report4code_calcv(  FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   EXPR4CALC *calc_on ;
   char cname_buf[256], outstring[512];

   calc_on = 0;
   calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);
   while(calc_on)
   {
      strcpy( cname_buf, calc_on->name ) ;
      report4stolower( cname_buf ) ;
      if( calc_on->total != 0 )
         report4code_out( seq, "   TOTAL4 *" ) ;
      else
         report4code_out( seq, "   EXPR4CALC *" ) ;
      report4code_out( seq, cname_buf ) ;
      report4code_out( seq, ";");
      report4code_nl( seq );
      M4PRINT(outstring,"   EXPR4 *exp%s;",cname_buf);
      report4code_out( seq, outstring);
      report4code_nl( seq );
      calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);
   }

}

void report4code_styles( FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   int first;
   STYLE4 *style_on;
   char sname_buf[256], fname_buf[256], outstring[512];
   char bstring[256];
   char astring[256];
   unsigned i;

   first = 1;
   style_on = 0;
   style_on=(STYLE4 *)l4next(&report->styles,style_on);
   while(style_on)
   {
      strcpy(sname_buf,style_on->name);
      report4stolower(sname_buf);
      for(i = 0; i < strlen(sname_buf); i++)
         if(sname_buf[i] == ' ' )
            sname_buf[i] = '_';

      if( for_windows )
      {
         if(for_windows == 2)
         {
            report4code_out( seq,"   #ifdef S4WINDOWS");
            report4code_nl ( seq );
         }

         report4code_out( seq, "   memset( &lpLogFont, 0, sizeof(R4LOGFONT));");
         report4code_nl( seq );

         if(style_on->lf.lfHeight)
         {
            M4PRINT(outstring, "   lpLogFont.lfHeight = (short)%d;",style_on->lf.lfHeight );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfWidth)
         {
            M4PRINT(outstring, "   lpLogFont.lfWidth = (short)%d;",style_on->lf.lfWidth);
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfEscapement)
         {
            M4PRINT(outstring, "   lpLogFont.lfEscapement = (short)%d;",style_on->lf.lfEscapement);
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfOrientation)
         {
            M4PRINT(outstring, "   lpLogFont.lfOrientation = (short)%d;",style_on->lf.lfOrientation);
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfWeight)
         {
            M4PRINT(outstring, "   lpLogFont.lfWeight = (short)%d;",style_on->lf.lfWeight);
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfItalic)
         {
            M4PRINT(outstring, "   lpLogFont.lfItalic = (R4BYTE)%d;",style_on->lf.lfItalic);
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfUnderline)
         {
            M4PRINT(outstring, "   lpLogFont.lfUnderline = (R4BYTE)%d;",style_on->lf.lfUnderline );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfStrikeOut)
         {
            M4PRINT(outstring, "   lpLogFont.lfStrikeOut = (R4BYTE)%d;",style_on->lf.lfStrikeOut );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfCharSet)
         {
            M4PRINT(outstring, "   lpLogFont.lfCharSet = (R4BYTE)%d;",style_on->lf.lfCharSet );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfOutPrecision)
         {
            M4PRINT(outstring, "   lpLogFont.lfOutPrecision = (R4BYTE)%d;",style_on->lf.lfOutPrecision );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfClipPrecision)
         {
            M4PRINT(outstring, "   lpLogFont.lfClipPrecision = (R4BYTE)%d;",style_on->lf.lfClipPrecision );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfQuality)
         {
            M4PRINT(outstring, "   lpLogFont.lfQuality = (R4BYTE)%d;",style_on->lf.lfQuality );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         if(style_on->lf.lfPitchAndFamily)
         {
            M4PRINT(outstring, "   lpLogFont.lfPitchAndFamily = (R4BYTE)%d;",style_on->lf.lfPitchAndFamily );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }

         memset(fname_buf,0,sizeof(fname_buf));
         memcpy(fname_buf,style_on->lf.lfFaceName,32);

         if(strlen(fname_buf) > 0)
         {
            M4PRINT(outstring, "   u4ncpy(lpLogFont.lfFaceName,\"%s\",32);",fname_buf );
            report4code_out(seq,outstring);
            report4code_nl( seq );
         }


         M4PRINT(outstring, "   %s = style4create(report,\"%s\",&lpLogFont);",sname_buf,style_on->name);
         report4code_out(seq,outstring);
         report4code_nl( seq );
         M4PRINT(outstring, "   %s->iptsize = %d;",sname_buf, style_on->iptsize);
         report4code_out(seq,outstring);
         report4code_nl( seq );
         #ifdef S4WINDOWS
            M4PRINT( outstring, "   #ifdef S4WINDOWS" );
            report4code_out( seq, outstring );
            report4code_nl( seq );
            M4PRINT(outstring, "   %s->color = RGB(%d,%d,%d);",sname_buf,GetRValue(style_on->color),GetGValue(style_on->color),GetBValue(style_on->color) );
            report4code_out(seq,outstring);
            report4code_nl( seq );
            M4PRINT(outstring, "   #endif" );
            report4code_out( seq, outstring );
            report4code_nl( seq );
         #endif
         report4code_nl( seq );

         if(for_windows == 2)
         {
            report4code_out( seq,"   #else");
            report4code_nl( seq );
         }
      }

      if( for_windows == 2 || for_windows == 0)
      {
         report4unparse_sstring(style_on->codes_before,bstring,style_on->codes_before_len);
         report4unparse_sstring(style_on->codes_after,astring,style_on->codes_after_len);
         M4PRINT(outstring, "   %s = style4create(report,\"%s\",\"%s\",%d,\"%s\",%d);",
            sname_buf,style_on->name,bstring,style_on->codes_before_len,astring,style_on->codes_after_len);
         report4code_out(seq,outstring);
         report4code_nl( seq );

         if( for_windows == 2)
         {
            report4code_out(seq,"   #endif");
            report4code_nl(seq);
         }
      }

      if(first)
      {
         first = 0;
         M4PRINT(outstring, "   style4default_set(report,%s);",sname_buf);
         report4code_out(seq, outstring);
         report4code_nl( seq );
      }
      style_on=(STYLE4 *)l4next(&report->styles,style_on);
   }
  
   report4code_nl( seq );

}

void report4code_calcs ( FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   EXPR4CALC *calc_on ;
   char cname_buf[256], outstring[512], tname_buf[256];
   unsigned i;

   calc_on = 0;
   calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);
   while(calc_on)
   {
      strcpy( cname_buf, calc_on->name ) ;
      report4stolower( cname_buf ) ;
      report4code_filename(tname_buf,calc_on->expr->data->file.name);
      report4stolower(tname_buf);
      for(i = 0; i < strlen(tname_buf); i++)
         if(tname_buf[i] == '.')
            tname_buf[i] = '\0';

      M4PRINT(outstring,"   exp%s = expr4parse(%s,\"%s\");",cname_buf,tname_buf,
         expr4source(calc_on->expr));
      report4code_out(seq,outstring);
      report4code_nl( seq );
      if( calc_on->total != 0 )
      {
         M4PRINT(outstring,"   %s = total4create(report,\"%s\",",
         cname_buf,calc_on->name);
         report4code_out( seq, outstring);
         report4code_expr( seq, expr4source(calc_on->expr));
         M4PRINT(outstring,",");
         switch( calc_on->total->total_type )
         {
            case total4lowest:
               strcat(outstring,"total4lowest");
               break;

            case total4highest:
               strcat(outstring,"total4highest");
               break;

            case total4count:
               strcat(outstring,"total4count");
               break;

            case total4average:
               strcat(outstring,"total4average");
               break;

            case total4sum:
               strcat(outstring,"total4sum");
               break;
         }
         strcat(outstring,");");
         report4code_out(seq,outstring);
         report4code_nl( seq );
      }
      else
      {
          M4PRINT(outstring,"   %s = expr4calc_create( report->cb, exp%s,\"%s\" );", cname_buf, cname_buf, calc_on->name );
          report4code_out( seq, outstring );
          report4code_nl( seq );
      }
      calc_on = (EXPR4CALC *)l4next( &report->cb->calc_list,calc_on);
   }

   report4code_nl( seq );report4code_nl( seq );

}

void report4code_groups( FILE4SEQ_WRITE *seq, REPORT4 *report )
{
   GROUP4 *group_on;
   char gname_buf[256], tname_buf[256], outstring[512];
   int groupnum;

   groupnum = 0;
   group_on = 0;
   group_on = (GROUP4 *)l4next(&report->groups,group_on);
   while(group_on)
   {
      strcpy( gname_buf, "g");
      strcat( gname_buf,group_on->label ) ;
      report4stolower( gname_buf) ;
      if( strlen( gname_buf) == 1 && gname_buf[0] == ' ' )
      {
         M4PRINT( gname_buf, "group%d", groupnum);
         groupnum++;
      }
      report4code_nl( seq );
      M4PRINT(outstring,"   %s = group4create(report);",gname_buf);
      report4code_out(seq,outstring);
      report4code_nl( seq );
      if(group_on->expr)
      {
         M4PRINT(outstring,"   group4expr(%s,",gname_buf);
         report4code_out( seq, outstring);
         report4code_expr( seq, expr4source(group_on->expr));
         report4code_out( seq,");");
         report4code_nl( seq );
      }
      if(group_on->swap_header)
      {
         M4PRINT(outstring,"%s->swap_header = %d;",gname_buf, group_on->swap_header);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }
      if( group_on->repeat_header )
      {
         M4PRINT(outstring,"%s->repeat_header = %d;",gname_buf,
            group_on->repeat_header);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }
      if( group_on->swap_footer )
      {
         M4PRINT(outstring,"%s->swap_footer = %d;",gname_buf,
            group_on->swap_footer);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }
      if(group_on->reset_page)
      {
         M4PRINT(outstring,"%s->reset_page = %d;",gname_buf,
            group_on->reset_page);
         report4code_out( seq, outstring );
         report4code_nl( seq );
      }
      report4code_nl( seq );

      if(group_on->header.height > 0)
      {
         M4PRINT(outstring, "   objects4height(&%s->header,%d);",gname_buf,
            group_on->header.height);
         report4code_out(seq,outstring);
         report4code_nl( seq );
         strcpy(tname_buf,gname_buf);
         strcat(tname_buf,"->header");
         objects4code( seq, &group_on->header,tname_buf);
      }
      
      if(group_on->footer.height > 0)
      {
         M4PRINT(outstring,"   objects4height(&%s->footer,%d);",gname_buf,
            group_on->footer.height);
         report4code_out(seq,outstring);
         report4code_nl( seq );
         strcpy(tname_buf,gname_buf);
         strcat(tname_buf,"->footer");
         objects4code( seq, &group_on->footer,tname_buf);
      }
      report4code_nl( seq );
      group_on = (GROUP4 *)l4next(&report->groups,group_on);
   }


}

