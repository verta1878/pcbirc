/* r4styles.c  (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif


void S4FUNCTION style4color( STYLE4 *style, COLORREF color)
{
   if(color != 0)
      style->color = color;
}

int S4FUNCTION style4default_create( REPORT4 *report )
{
   STYLE4 *style ;
   LIST4 *style_list ;
   #ifdef S4WINDOWS
      R4HDC hDC;
   #endif

   style_list =  &report->styles ;

   if ( style_list->n_link > 0 )
      return 0 ;

   style = (STYLE4 *) mem4create_alloc( report->cb,
            &report->style_memory, 5, sizeof(STYLE4), 5, 0);

   if ( style == 0 )  return -1 ;

   report->def_style = style;

   l4add( style_list, style ) ;
   style_list->selected =  style ;
   u4ncpy( style->name, "PlainText", sizeof(style->name) ) ;

   #ifdef S4WINDOWS
   hDC = report4dc_get( report ) ;
   if( hDC == 0 ) return -1 ;

   style->hFont =  GetStockObject( DEVICE_DEFAULT_FONT ) ;
   SelectObject( hDC, style->hFont ) ;
   GetObject( style->hFont, sizeof(R4LOGFONT), &style->lf ) ;
   report4dc_free( report ) ;
   #else
   style->codes_before_len = style->codes_after_len = 0;
   style->codes_before = style->codes_after = 0;
   #endif
   

   return 0 ;
}

#ifndef S4WINDOWS

STYLE4 * S4FUNCTION style4create( REPORT4 *report, char *name, 
char *codes_before, int codes_before_len, char *codes_after,
int codes_after_len)
{
   
   STYLE4 *cstyle = (STYLE4 *) mem4create_alloc( report->cb,
            &report->style_memory,5,sizeof(STYLE4),5,0);
            
   if(cstyle == 0)
   {
      e4(report->cb,e4report,E4_REPORT_ASTYLE);
      return 0;
   }
    
   u4ncpy( cstyle->name, name, sizeof(cstyle->name) ) ;

   cstyle->codes_before_len = strlen(codes_before);
   cstyle->codes_after_len  = strlen(codes_after);

   if(cstyle->codes_before != 0)
      u4free( cstyle->codes_before );

   if(cstyle->codes_after != 0)
      u4free( cstyle->codes_after );

   cstyle->codes_before = (char *) u4alloc_er(report->cb,
      cstyle->codes_before_len+1);
   cstyle->codes_after  = (char *) u4alloc_er(report->cb,
      cstyle->codes_after_len+1);

   if( cstyle->codes_before == NULL || cstyle->codes_after == NULL )
   {
      cstyle->codes_before_len = cstyle->codes_after_len = 0;
      u4free( cstyle );
      return 0;
   }
   memset(cstyle->codes_before,0,cstyle->codes_before_len);
   memset(cstyle->codes_after,0,cstyle->codes_after_len);

   cstyle->codes_before_len = report4parse_sstring(codes_before,cstyle->codes_before,codes_before_len);
   cstyle->codes_after_len = report4parse_sstring(codes_after,cstyle->codes_after,codes_after_len);
   
   l4add( &report->styles, cstyle ) ;

   if ( report->styles.selected == 0 )
      report->styles.selected =  cstyle ;


   return cstyle ;

}

#endif

#ifdef S4WINDOWS

STYLE4 * S4FUNCTION style4create( REPORT4 *report, char *name, R4LOGFONT *lf_ptr )
{
   STYLE4 *style = (STYLE4 *) mem4create_alloc( report->cb,
            &report->style_memory, 5, sizeof(STYLE4), 5, 0);

   if(style == 0)
   {
      e4(report->cb,e4report,E4_REPORT_ASTYLE);
      return 0;
   }

   u4ncpy( style->name, name, sizeof(style->name) ) ;
   memcpy( &style->lf, lf_ptr, sizeof(R4LOGFONT) ) ;

   style->hFont =  CreateFontIndirect( lf_ptr ) ;
   if ( style->hFont == 0 )
   {
      e4describe(report->cb, e4result, E4_REPORT_FON, lf_ptr->lfFaceName, (char *)0) ;
      return 0 ;
   }

   style->codes_before_len = style->codes_after_len = 0;
   style->codes_before = style->codes_after = 0;
   l4add( &report->styles, style ) ;

   if ( report->styles.selected == 0 )
      report->styles.selected =  style ;

   return style ;
}

#endif

STYLE4 * S4FUNCTION style4default_set( REPORT4 *report, STYLE4 *s4 )
{
   STYLE4 *prev_style =  (STYLE4 *) report->styles.selected ;

   if ( s4 != 0 )
      report->styles.selected =  s4 ;

   return prev_style ;
}

STYLE4 * S4FUNCTION style4lookup( REPORT4 *report, char *style_name )
{
   STYLE4 *style_on ;

   style_on = 0;
   style_on = (STYLE4 *) l4next( &report->styles, style_on);
   while(style_on)
   {
      if ( strcmp( style_on->name, style_name ) == 0 )
         return style_on ;
      style_on = (STYLE4 *) l4next( &report->styles, style_on);
   }

   return 0 ;
}


