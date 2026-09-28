/* r4save_m.c   (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif

#define CREP_MASK 0xA5
#define VERSION_MASK 0x10

int retrieve4string( FILE4SEQ_READ *, char *, int ) ;
int retrieve_object_list( REPORT4 *, CODE4 *, OBJECTS4 *, FILE4SEQ_READ * ) ;
void report4cleanup(RELATE4 *, REPORT4 *,FILE4 *, int ) ;

/* open_files: 0 Never;  -1 Always;  1 If necessary */
REPORT4 * S4FUNCTION report4retrieve(CODE4 *c4,char *file_name,int open_files)
{
   FILE4 file ;
   FILE4SEQ_READ seq ;
   char buf[2048] ;
   char str_buf[512], str_buf2[512], dname_buf[512], iname_buf[512] ;
   char tname_buf[512], expr_buf[512];
   char name_buf[512], *cpointer ;
   RELATE4 *master = 0, *relate = 0 ;
   RELATE4 relate_buf ;
   DATA4 *d4 ;
   TAG4 *tag ;
   REPORT4 *report = NULL ;
   REPORT4 report_buf ;
   STYLE4 *stemp ;
   GROUP4 *group_on, group_buf ;
   EXPR4CALC calc_buf, *calc_new ;
   EXPR4CALC *calc_on ;
   EXPR4 *texpr, *expr;
   TOTAL4 total_buf, *total_new ;
   int rc, n ;
   int save_tag_error ;
   int code ;
   int n_link ;
   int temp_len ;
   int start_error, i ;
   int tauto_open, tcreate_error, texpr_error;

   if( c4->num_reports != 0 )
   {
      e4( c4, e4report, E4_REPORT_ONE ) ;
      return 0 ;
   }

   u4ncpy( name_buf, file_name, sizeof(name_buf) ) ;
   u4name_ext( name_buf, sizeof(name_buf), "REP", 0 ) ;
   c4upper( name_buf ) ;

   if( file4open( &file, c4, name_buf, 1 ) != 0 )
   {
      e4set( c4, 0 ) ;
      return 0 ;
   }

   file4seq_read_init( &seq, &file, 0L, buf, sizeof(buf) ) ;

   if((file4seq_read(&seq,name_buf,2)) < 2)
   {
      report4cleanup( master, report, &file, open_files ) ;
      return 0;
   }

   if( !( name_buf[0] & CREP_MASK) || !(name_buf[1] & VERSION_MASK))
   {
      e4( seq.file->code_base, e4result, E4_REPORT_FILE ) ;
      report4cleanup( master, report, &file, open_files ) ;
      return 0;
   }

   tauto_open = c4->auto_open;
   tcreate_error = c4->create_error;
   texpr_error = c4->expr_error;
   c4->auto_open = c4->create_error = c4->expr_error = 0;

   
   /* First build the 'relate' */

   for(;;)
   {
      if((retrieve4string( &seq, dname_buf, sizeof(dname_buf) )) < 0)
      {
         report4cleanup( master, report, &file, open_files ) ;
         return 0;
      }
      file4seq_read( &seq, (char *) &relate_buf, sizeof(relate_buf) );



      d4 = 0 ;
      if( open_files >= 0 )
      {
         u4name_piece( str_buf2, sizeof(str_buf2), dname_buf, 0, 0 ) ;
         d4 =  d4data( c4, str_buf2 ) ;
      }
      if ( d4 == 0 )
      {
         if( open_files == 0 )
         {
            e4describe( c4, e4result, E4_RESULT_LCF, dname_buf, (char *)0 );
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }
         d4 =  d4open( c4, dname_buf ) ;
         if ( d4 == 0 )
         {
            e4describe( c4, e4report, E4_REPORT_DFILE, dname_buf, (char *)0 ) ;
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }
      }
                  

      
      save_tag_error = c4->tag_name_error ;
      c4->tag_name_error = 0 ;

      file4seq_read( &seq, &n_link, sizeof(n_link));
      for( i = 0; i < n_link; i++ )
      {
         if( retrieve4string( &seq, iname_buf, sizeof(iname_buf) ) < 0)
         {
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }

         u4name_piece( tname_buf, sizeof(tname_buf)-1, iname_buf, 0, 0 );
         if( d4index( d4, tname_buf ) == 0 )
         {
            if( open_files == 0 )
            {
               e4describe( c4, e4result, E4_RESULT_LCF, iname_buf, (char *)0 );
               report4cleanup( master, report, &file, open_files ) ;
               return NULL;
            }

            if ( i4open( d4, iname_buf ) == 0 )
            {
               c4->tag_name_error =  save_tag_error ;
               e4describe( c4, e4report, E4_REPORT_IFILE, iname_buf, (char *) 0);
               report4cleanup( master, report, &file, open_files ) ;
               return NULL;
            }
         }

      }

      tag =  0 ;
      if( relate_buf.data_tag != 0)
      {

         if((retrieve4string( &seq, tname_buf, sizeof(tname_buf) )) < 0)
         {
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }

         tag =  d4tag( d4, tname_buf ) ;
         if ( tag == 0 )
         {
            e4describe( c4, e4result, E4_PARM_TAG, iname_buf, tname_buf);
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }
      }

      if ( relate == 0 )
      {
         master =  relate =  relate4init( d4 ) ;
         if( master == 0 )
         {
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }
         relate->data_tag = tag ;
         d4tag_select(relate->data,relate->data_tag);
      }
      else
      {
         if((retrieve4string( &seq, expr_buf, sizeof(expr_buf) )) < 0 )
         {
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }

         relate =  relate4create_slave( master, d4, expr_buf, tag ) ;
         if ( relate == 0 )
         {
            /* Unable to create slave */
            report4cleanup( master, report, &file, open_files ) ;
            return NULL;
         }
         relate->data_tag = tag ;
      }

      relate->error_action  =  relate_buf.error_action ;
      relate->relation_type =  relate_buf.relation_type ;
      relate->match_len = relate_buf.match_len;

      c4->error_code = 0;
      rc = file4seq_read( &seq, &code, sizeof(code) ) ;

      if( rc < 0 || code == 2 )
         break ;

      master =  relate ;
      while( code++ <= 0 )
         master =  master->master ;
   }

   /* Now for the report definition */
   if( master == NULL )
      return NULL;

   while( master->master )
      master = master->master ;

   report =  report4init( master ) ;
   if( report == 0 )
   {
      report4cleanup( master, report, &file, open_files ) ;
      return 0 ;
   }

   file4seq_read( &seq, &report_buf, sizeof(report_buf) ) ;

   for( n = report_buf.styles.n_link; n > 0; n-- )
   {
      STYLE4 style_buf ;
      rc = file4seq_read( &seq, &style_buf, sizeof(STYLE4) ) ;
      if( rc < 0 )
      {
         report4cleanup( master, report, &file, open_files ) ;
        return 0 ;
      }

      #ifdef S4WINDOWS
      stemp = style4create( report, style_buf.name, &style_buf.lf ) ;
      #else
      stemp = style4create( report, style_buf.name,0,0,0,0);
      #endif

      if(stemp== 0 )
      {
         report4cleanup( master, report, &file, open_files ) ;
         return 0 ;
      }
      stemp->color = style_buf.color ;
      stemp->iptsize = style_buf.iptsize ;
      stemp->codes_before_len = style_buf.codes_before_len;
      stemp->codes_after_len = style_buf.codes_after_len;

      if(stemp->codes_before_len > 0)
      {
         rc = retrieve4string( &seq, str_buf,sizeof(str_buf));
         stemp->codes_before = (char *) u4alloc_er( report->cb, rc+1 ) ;
         if( stemp->codes_before != NULL )
         {
            memset( stemp->codes_before, 0, rc+1 ) ;
            for(i = 0; i < rc; i++)
               stemp->codes_before[i] = str_buf[i] ;
         }
         else
         {
            stemp->codes_before_len = 0;
         }
      }

      if(stemp->codes_after_len > 0)
      {
         rc = retrieve4string( &seq, str_buf,sizeof(str_buf));
         stemp->codes_after = (char *) u4alloc_er ( report->cb, rc+1 ) ;
         if( stemp->codes_after != NULL )
         {
            memset( stemp->codes_after, 0, rc+1 ) ;
            for( i = 0; i < rc; i++)
               stemp->codes_after[i] = str_buf[i] ;
         }
         else
         {
            stemp->codes_after_len = 0;
         }
      }
   }
   rc = retrieve4string( &seq, str_buf, sizeof(str_buf) ) ;
   if( rc < 0 )
   {
      report4cleanup( master, 0, &file, open_files ) ;
      return 0 ;
   }

   report->styles.selected =  style4lookup( report, str_buf ) ;
   report4display( report, report_buf.to_screen ) ;
   report4leading_zero( report, report_buf.leading_zero ) ;
   report->hide_info =  report_buf.hide_info ;
   report->units =  report_buf.units ;
   report->sensitivity_x =  report_buf.sensitivity_x ;
   report->sensitivity_y =  report_buf.sensitivity_y ;
   report->sensitivity_adjust =  report_buf.sensitivity_adjust ;
   report->margin_top = report_buf.margin_top ;
   report->margin_bottom = report_buf.margin_bottom ;
   report->margin_left = report_buf.margin_left ;
   report->report_width = report_buf.report_width ;
   report->margin_right = report_buf.margin_right ;
   report->output_handle = 1 ;
   report->decimal_point = report_buf.decimal_point;
   report->thousand_separator = report_buf.thousand_separator;
   report->currency_sym = report_buf.currency_sym;
   report->leading_zero = report_buf.currency_sym;
   report->use_styles = report_buf.use_styles;
   report->swidth = report_buf.swidth;
   report->sheight = report_buf.sheight;

   for(; report_buf.groups.n_link-- > 0; )
   {
      group_on =  group4create( report ) ;

      if(group_on == 0)
      {
         report4cleanup( master, report, &file, open_files ) ;
         /* Unable to create group */
         return 0;
      }

      rc = file4seq_read( &seq, &group_buf, sizeof(GROUP4) ) ;
      if( rc < 0 )
      {
         /* Unable to retrieve group data */
         e4(c4,e4report,E4_REPORT_RGROUP);
         report4cleanup( master, report, &file, open_files ) ;
         return 0 ;
      }
      memcpy( group_on->label, group_buf.label, sizeof(group_on->label) ) ;

      group_on->header.height =  group_buf.header.height ;
      group_on->footer.height =  group_buf.footer.height ;
      group_on->reset_page    =  group_buf.reset_page ;
      group_on->swap_footer   =  group_buf.swap_footer;
      group_on->repeat_header =  group_buf.repeat_header ;
      group_on->swap_header=  group_buf.swap_header;

      rc = retrieve4string( &seq, str_buf, sizeof(str_buf) ) ;
      group_on->expr = NULL;

      if( rc < 0 )
      {
         /* Unable to read group expression */
         report4cleanup( master, report, &file, open_files ) ;
         return 0 ;
      }

      if ( strlen(str_buf) > 0 )
      {
         group_on->expr = (EXPR4 *)u4alloc( strlen( str_buf ) + 1 );
         if( group_on->expr != NULL )
            u4ncpy ( (char *)group_on->expr, str_buf, strlen(str_buf)+1 );
      }
   }


   texpr = expr4parse( report->relate->data, "1" );

   file4seq_read( &seq, &n_link, sizeof(n_link) ) ;

   for( n = n_link; n > 0; n-- )
   {
      rc = file4seq_read( &seq, &calc_buf, sizeof(EXPR4CALC) ) ;
      if( rc < 0 )
      {
        /* Unable to read info for calc */
        e4(c4,e4report,E4_REPORT_RCALC);
        report4cleanup( master, report, &file, open_files ) ;
        return 0 ;
      }

      if( calc_buf.total  )
      {
         rc = file4seq_read( &seq, &total_buf, sizeof(TOTAL4) ) ;
         if( rc < 0 )
         {
            /* Unable to read info for total */
            e4(c4,e4report,E4_REPORT_RTOTAL);
            break ;
         }
         total_new = total4create_total( report, calc_buf.name, texpr, total_buf.total_type );
         if(total_new == 0)
         {
            break;
         }
         calc_new = total_new->calc_ptr;

         temp_len = retrieve4string( &seq, str_buf2, sizeof(str_buf2) ) ;
         if( temp_len < 0 )
         {
            /* Unable to find reset group for total */
            e4(c4,e4report,E4_REPORT_TRG);
            break ;
         }

         if( temp_len > 0 )
         {
            group_on =  group4lookup( report, str_buf2 ) ;
            if( group_on == 0 )
            {
               e4( c4, e4report, E4_REPORT_GRO ) ;
               report4cleanup( master, report, &file, open_files ) ;
               return 0;
            }
            else
               total4reset_level( total_new, group_on ) ;
         }
      }
      else
         calc_new = expr4calc_create( c4, texpr, calc_buf.name );

      if(calc_new == 0)
         break;

      temp_len = retrieve4string( &seq, str_buf, sizeof(str_buf) ) ;

      if( temp_len < 0 )
         break ;

      if( calc_new )
      {
         char *temp_ptr = (char *) u4alloc_er( c4, temp_len + 1 );
         if( temp_ptr != NULL )
         {
            u4ncpy( temp_ptr, str_buf, temp_len+1 );
            calc_new->expr = (EXPR4 *)temp_ptr;
         }
         else
            calc_new->expr = NULL;
      }
   }

   expr4free( texpr );

   start_error =  c4->error_code ;

   calc_on = 0;
   calc_on = (EXPR4CALC *) l4first( &c4->calc_list);
   while(calc_on)
   {
      if( calc_on->expr )
      {
         expr = expr4parse( report->relate->data, (char *) calc_on->expr );
         u4free( calc_on->expr );
         calc_on->expr = expr;
      }

      if( expr == 0 )
      {

         expr4calc_delete( calc_on );
         if( start_error == 0  &&  c4->error_code != e4memory )
            e4set( c4, 0 ) ;
      }
      else
         expr4calc_massage( calc_on );
      calc_on=(EXPR4CALC *)l4next( &c4->calc_list, calc_on);
   }

   calc_on = 0;
   calc_on = (EXPR4CALC *) l4first( &c4->calc_list);
   while(calc_on)
   {
      expr = expr4parse( report->relate->data, expr4source( calc_on->expr ) );
      u4free( calc_on->expr );
      calc_on->expr = expr;

      if( expr == 0 )
      {

         expr4calc_delete( calc_on );
         if( start_error == 0  &&  c4->error_code != e4memory )
            e4set( c4, 0 ) ;
      }
      expr4calc_massage( calc_on );
      calc_on=(EXPR4CALC *)l4next( &c4->calc_list, calc_on);
   }

   group_on = 0;
   group_on = (GROUP4 *)l4first( &report->groups);
   while(group_on)
   {
      if( group_on->expr )
      {
         cpointer = (char *)group_on->expr;
         group_on->expr = NULL;
         if( group4expr( group_on, cpointer ) < 0 )
         {
            group_on->expr = NULL ;
            e4set( c4, 0 ) ;
         }
         u4free( cpointer );
      }

      if(retrieve_object_list( report, c4, &group_on->header, &seq ) < 0 )
      {
         /* unable to retrieve objects for group %s header */
         e4describe(c4,e4report,E4_REPORT_ROBJS,group_on->label,"Header");
         report4cleanup( master, report, &file, open_files ) ;
         return 0 ;
      }
      if(retrieve_object_list( report, c4, &group_on->footer, &seq ) < 0 )
      {
         /* unable to retrieve objects for group footer */
         e4describe(c4,e4report,E4_REPORT_ROBJS,group_on->label,"Footer");
         report4cleanup( master, report, &file, open_files ) ;
         return 0 ;
      }
      group_on = (GROUP4 *) l4next( &report->groups, group_on);
   }

   if(retrieve_object_list( report, c4, &report->page_header, &seq ) < 0 )
   {
      /* unable to retrieve objects for page header */
      e4describe(c4,e4report,E4_REPORT_ROBJS,"Page Header",(char *)0);
      report4cleanup( master, report, &file, open_files ) ;
      return 0 ;
   }
   if(retrieve_object_list( report, c4, &report->page_footer, &seq ) < 0 )
   {
      /* unable to retrieve objects for page footer */
      e4describe(c4,e4report,E4_REPORT_ROBJS,"Page Footer",(char *)0);
      report4cleanup( master, report, &file, open_files ) ;
      return 0 ;
   }
   if(retrieve_object_list( report, c4, &report->title, &seq ) < 0 )
   {
      /* unable to retrieve objects for report title */
      e4describe(c4,e4report,E4_REPORT_ROBJS,"Title Area",(char *)0);
      report4cleanup( master, report, &file, open_files ) ;
      return 0 ;
   }
   if(retrieve_object_list( report, c4, &report->summary, &seq ) < 0 )
   {
      /* unable to retrieve objects for summary area */
      e4describe(c4,e4report,E4_REPORT_ROBJS,"Summary Area",(char *)0);
      report4cleanup( master, report, &file, open_files ) ;
      return 0;
   }

   rc = retrieve4string( &seq, str_buf, sizeof(str_buf) ) ;
   if( rc >= 0 )
      if( str_buf[0] != 0 )
         relate4query_set( report->relate, str_buf ) ;


   rc = retrieve4string( &seq, str_buf, sizeof(str_buf) ) ;
   if( rc >= 0 )
      if(str_buf[0] != 0 )
         relate4sort_set( report->relate, str_buf ) ;

   rc = retrieve4string( &seq, str_buf, sizeof(str_buf) );
   if( rc >= 0 )
   {
      report->report_name = (char *)u4alloc_er( c4, strlen(str_buf) + 2 );
      if( report->report_name != NULL )
      {
         strcpy( report->report_name, str_buf );
      }
      else
      {
         report4cleanup( master, report, &file, open_files ) ;
         return NULL;
      }
   }

   rc = file4close( &file ) ;
   if( rc < 0 )
   {
      report4cleanup( master, 0, &file, open_files ) ;
      return 0 ;
   }


   c4->auto_open = tauto_open;
   c4->create_error = tcreate_error;
   c4->expr_error = texpr_error;

   return report ;
}

#ifdef S4VB_DOS

REPORT4 * report4retrieve_v( CODE4 *c4, char *file_name, int open_files )
{
   return report4retrieve( c4, c4str(file_name), open_files ) ;
}

#endif

