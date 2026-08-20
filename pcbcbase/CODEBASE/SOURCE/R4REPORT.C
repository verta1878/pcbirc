/* r4report.c   (c)Copyright Sequiter Software Inc., 1991-1993.  All rights reserved. */

#include "d4all.h"

#ifdef __TURBOC__
   #pragma hdrstop
#endif

#ifdef S4WINDOWS
#ifndef WPARAM
   #define WPARAM UINT
   #define LPARAM LONG
   #define COLOR_BTNHIGHLIGHT 20
#endif
#ifndef HINSTANCE
   #define HINSTANCE HANDLE
#endif
#endif

int units4points = 0 ;
int units4inches = 1 ;
int units4cent = 2 ;


void report4objects_destroy( OBJECTS4 * );


unsigned S4FUNCTION u4ncat( char *to, char *from, unsigned to_len )
{
   unsigned len =  strlen(to) ;
   if ( len >= to_len )  return to_len ;

   return u4ncpy( to + len, from, to_len-len ) + len ;
}



/* finds the lowest numbered group which changes */
GROUP4 * S4FUNCTION report4calc_first_change_group( REPORT4 *r4 )
{
   GROUP4 *first_change_group = 0 ;
   GROUP4 *group_on ;
   GROUP4 *last_group ;
   char  *ptr ;
   int len ;

   last_group =   (GROUP4 *) l4last( &r4->groups ) ;

   for ( group_on = 0;; )
   {
      if ( group_on == last_group )
	 break ;

      if(group_on != last_group)
	 group_on = (GROUP4 *) l4next( &r4->groups, group_on);

      if( group_on->expr == 0 )
      {
	 if( first_change_group == 0 )
	    first_change_group = group_on;
	 continue;
      }

      len =  expr4vary( group_on->expr, &ptr ) ;

      if( group_on->value == 0 )
      {
	 group_on->value =  (char *) u4alloc_er( r4->cb, len ) ;
	 if( group_on->value == 0 )  return 0 ;

	 group_on->value_len = len ;
      }

      if ( memcmp( group_on->value, ptr, len ) != 0 )
	 if ( first_change_group == 0 )
	    first_change_group =  group_on ;

      memcpy( group_on->value, ptr, len ) ;
   }

   if ( first_change_group == 0 && last_group->expr == 0 )
      first_change_group = last_group;

   return first_change_group ;
}


int S4FUNCTION report4display( REPORT4 *r4, int to_screen )
{
   int rc ;

   rc =  r4->to_screen ;

   if( to_screen >= 0 )
      r4->to_screen =  to_screen ;

   return rc ;
}

int S4FUNCTION report4layout( REPORT4 *r4 )
{
   GROUP4 *group =  (GROUP4 *) l4first( &r4->groups ) ;
   OBJECTS4 *objects ;
   OBJECTS4 *objects_labels ;
   int pos ;
   RELATE4 *relate_on ;
   int j ;
   TEXT4 *t4 ;
   #ifdef S4WINDOWS
      STYLE4 *style ;
      TEXTMETRIC tm ;
      R4HDC dc;
      POINT pt;
   #endif

   if( group == 0 )
   {
      group =  group4create( r4 ) ;
      if( group == 0 )  return -1 ;
   }

   objects = &group->header ;
   objects_labels = &r4->page_header ;

   report4objects_destroy( objects ) ;
   report4objects_destroy( objects_labels ) ;

   #ifdef S4WINDOWS
   dc = GetDC( r4->hWndParent ) ;

   dc4mapping( dc ) ;
   style =  (STYLE4 *) r4->styles.selected ;
   SelectObject( dc, style->hFont ) ;
   GetTextMetrics( dc, &tm );
   #endif

   objects->height =
   objects_labels->height =  333 ;

   pos = 0 ;

   for( relate_on = r4->relate; relate_on; relate4next( &relate_on))
   {
      DATA4 *d4 =  relate_on->data ;
      for( j = 1; j <= d4num_fields(d4); j++ )
      {
	 FIELD4 *f4 =  d4field_j( d4, j ) ;
	 int w1,w2;

	 char field_buf[25] ;
	 strcpy( field_buf, d4alias(f4data(f4)) ) ;
	 u4ncat( field_buf, "->", sizeof(field_buf) ) ;
	 u4ncat( field_buf, f4name(f4), sizeof(field_buf) ) ;
	 t4 = text4expr( objects, pos, 0, field_buf ) ;

	 if( t4 == 0 )  return -1 ;

	 #ifdef S4WINDOWS
	 w1 = text4width_estimate(t4,&tm);
	 #else
	 w1 = t4->len;
	 #endif

	 if( f4type(f4) == 'N' )
	    t4->alignment =  text4right ;

	 strcpy(field_buf,f4name(f4));
	 t4 = text4label( objects_labels, pos, 0, field_buf ) ;
	 if( t4 == 0 )  return -1 ;
	 #ifdef S4WINDOWS
	 w2 = text4width_estimate(t4,&tm);
	 #else
	 w2 = t4->len;
	 #endif

	 #ifdef S4WINDOWS
	 pos += (((w1 > w2) ? w1 : w2) + 2 * tm.tmAveCharWidth) ;
	 #else
	 pos += ((((w1 > w2) ? w1 : w2) + 2)*100) ;
	 #endif
	 if( pos > r4->report_width )  break ;
      }
   }
   #ifdef S4WINDOWS
      ReleaseDC(r4->hWndParent,dc);
   #endif
   return 0 ;
}



void S4FUNCTION report4leading_zero( REPORT4 *r4, int flag )
{
   r4->leading_zero = (char) flag ;
}

#ifdef S4WINDOWS
void S4FUNCTION report4parent( REPORT4 *r4, R4HWND hParent )
{
   r4->hWndParent =  hParent ;
}


int S4FUNCTION report4dc_free( REPORT4 *r4 )
{
   if ( r4->hDC == 0 )  return 0 ;
   if ( DeleteDC( r4->hDC ) == 0 )
      return e4( r4->cb, e4result, E4_RESULT_DEL );

   r4->hDC =  0 ;
   return 0 ;
}

R4HDC S4FUNCTION report4dc_get( REPORT4 *r4 )
{
   static char szPrinter[80] ;
   char *szDevice, *szDriver, *szOutput ;

   report4dc_free( r4 ) ;

   if ( r4->printer_name == 0 )
   {
      GetProfileString( "windows", "device", ",,,", szPrinter, 80 ) ;

      if ( ! (szDevice = strtok( szPrinter, ",")) )  return 0 ;
      if ( ! (szDriver = strtok( NULL, ", ")) ) return 0 ;
      if ( ! (szOutput = strtok( NULL, ", ")) ) return 0 ;
   }
   else
   {
      GetProfileString( "devices", r4->printer_name, "=,", szPrinter, 80) ;
      szDevice = r4->printer_name ;
      if ( ! (szDriver = strtok( szPrinter, ", ")) ) return 0 ;
      if ( ! (szOutput = strtok( NULL, ", ")) ) return 0 ;
   }

   r4->hDC = CreateDC( szDriver, szDevice, szOutput, 0 ) ;

   return r4->hDC ;
}
#endif

int S4FUNCTION report4display_repeat_headers( REPORT4 *r4, GROUP4 *group_stop )
{
   GROUP4 *group_on ;

   for( group_on=0;(group_on=(GROUP4 *)l4next(&r4->groups,group_on))!=group_stop;)
      if ( group_on->repeat_header )
	 objects4display( &group_on->header ) ;

   return 0 ;
}


REPORT4 * S4FUNCTION report4init( RELATE4 *relate )
{
   REPORT4 *r4 ;
   CODE4 *code_base = relate->code_base ;

   if( code_base->num_reports != 0 )
   {
      e4( code_base, e4report, E4_REPORT_ONE ) ;
      return 0 ;
   }
   code_base->num_reports = 1 ;

   r4 =  (REPORT4 *) u4alloc_er( code_base, sizeof(REPORT4) ) ;
   if ( r4 == 0 )  return 0 ;

   r4->cb =  code_base ;
   r4->relate =  relate ;

   r4->cb->auto_open = 0;

   objectsort4init( &r4->page_header, r4, 0 ) ;
   objectsort4init( &r4->page_footer, r4, 0 ) ;
   objectsort4init( &r4->title, r4, 0 ) ;
   objectsort4init( &r4->summary, r4, 0 ) ;

   r4->report_width =  8000 ;
   r4->margin_top = 166 ;
   r4->margin_bottom = 166 ;
   r4->decimal_point =  '.' ;
   r4->thousand_separator = ',' ;
   r4->currency_sym = '$' ;

   r4->units =  units4points ;

   r4->def_style = NULL;
   r4->sheight = 25 ;
   r4->swidth = 80 ;
   r4->output_handle = 1 ;
   r4->to_screen = 1 ;
   
   return r4 ;
}



int S4FUNCTION report4new_page( REPORT4 *r4, int margin_y )
{
   #ifdef S4WINDOWS
      BITMAP bm ;
   #endif
  
   r4->cb->pageno++;

   #ifdef S4WINDOWS
   if(r4->to_screen)
   {
      SetClassWord(r4->hWnd,GCW_HCURSOR,LoadCursor((HINSTANCE)NULL,IDC_ARROW));
      SetCursor(LoadCursor((HINSTANCE)NULL,IDC_ARROW));
   }
   #endif

   r4->page_no++ ;
   r4->y = r4->cline = margin_y ;

   if ( r4->first )
      r4->first =  0 ;

   #ifdef S4WINDOWS
   {
      if( r4->to_screen )
      {
	 r4->report_command = 0 ;
	 InvalidateRect( r4->hWnd, 0, 1 ) ;

	 report4message_loop( r4 ) ;

	 #ifdef S4WINDOWS
	 SetClassWord(r4->hWnd,GCW_HCURSOR,(WORD)NULL);
	 SetCursor(LoadCursor((HINSTANCE)NULL,IDC_WAIT));
	 #endif

	 GetObject( (HBITMAP)r4->hBitMap, sizeof(BITMAP), &bm ) ;
	 PatBlt( r4->hDC, 0, 0, bm.bmWidth, bm.bmHeight, WHITENESS ) ;
      }
      else
      {
	 if ( Escape( r4->hDC, NEWFRAME, 0, NULL, NULL) < 0 ) 
	 {
	    e4( r4->cb, e4result, E4_RESULT_ON ) ;
	    return -1 ;
	 }

	 if( r4->report_command < 0 )
	    Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
      }
   }
   #else
   {
      if( ( report4driver_new_page() ) < 0 )
	 return -1;
   }
   #endif


   return 0 ;
}

#ifdef S4WINDOWS
int S4FUNCTION dc4mapping( R4HDC hDC )
{
   DWORD dwExtent;

   if( hDC == 0 )  return -1 ;
   SetMapMode( hDC, MM_HIENGLISH ) ;
   SetMapMode( hDC, MM_ANISOTROPIC ) ;
   dwExtent =  GetViewportExt( hDC ) ;
   SetViewportExt( hDC, LOWORD(dwExtent), -((int)HIWORD(dwExtent)) ) ;
   return 0 ;
}

int S4FUNCTION report4dc_page_height( REPORT4 *r4 )
{
   int size_mm =  GetDeviceCaps( r4->hDC, VERTSIZE ) ;
   return  (int) (size_mm * 39.370078) ;
}

void S4FUNCTION report4message_loop( REPORT4 *r4 )
{
   MSG msg ;
   for(;;)
   {
      while( PeekMessage( &msg, (R4HWND)NULL, 0, 0, PM_REMOVE) )
      {
	 TranslateMessage( &msg ) ;
	 DispatchMessage( &msg ) ;
      }

      if( r4 == 0 )  break ;
      if( r4->report_command )  break ;
      if(r4->report_command < 0 ) break;
   }
}


BOOL FAR PASCAL _export report4abort_proc( R4HDC hdcPrn, int nCode )
{
   report4message_loop( 0 ) ;
   return TRUE ;
}

LONG FAR PASCAL _export countproc(R4HWND,UINT,WPARAM,LPARAM);

void S4FUNCTION report4register_classes( REPORT4 *r4 )
{
    WNDCLASS  wc;

    if( r4->is_registered ) return ;

    memset( &wc, 0, sizeof(wc) ) ;

    wc.cbWndExtra    = sizeof(void *) ;
    wc.hInstance     = r4->cb->hInst ;
    wc.hIcon         = LoadIcon( (HINSTANCE)NULL, IDI_APPLICATION ) ;
    wc.hCursor       = LoadCursor( (HINSTANCE)NULL, IDC_ARROW ) ;
    wc.hbrBackground = GetStockObject( WHITE_BRUSH ) ;

    wc.lpfnWndProc   = report4cancel_proc ;
    wc.lpszClassName = "cancel4" ;
    RegisterClass( &wc ) ;

    wc.cbWndExtra = 12;
    wc.lpfnWndProc   = report4output_proc ;
    wc.lpszClassName = "output4" ;
    wc.style =  CS_BYTEALIGNCLIENT ;
    RegisterClass( &wc ) ;

    wc.cbWndExtra = 12;
    wc.lpfnWndProc = countproc;
    wc.lpszClassName = "counter";
    wc.hbrBackground = GetStockObject( LTGRAY_BRUSH );
    RegisterClass(&wc);

    r4->is_registered = 1 ;
}
#endif

void report4sort(OBJECTS4 *objects)
{
   int flag = 1;
   OBJ4 *obj_on = 0, *obj_next = 0 ;

   while(flag)
   {
      flag = 0;
      obj_on = (OBJ4 *)l4first( &objects->list );
      while( obj_on )
      {
	 obj_next = (OBJ4 *) l4next( &objects->list, obj_on);
	 if( !obj_next )
	    break;

	 if(  obj_on->r.top > obj_next->r.top ||
	    ( obj_on->r.top == obj_next->r.top &&
	      obj_on->r.left > obj_next->r.left ) )
	 {
	    l4remove( &objects->list, obj_on );
	    l4add_after( &objects->list, obj_next, obj_on );
	    flag = 1;
	 }
	 else
	    obj_on = obj_next;
      }
   }
}

#ifdef S4WINDOWS

void Draw4Area(R4HDC hDC, R4RECT *rect)
{
   HPEN hOldPen,hPen;
   POINT points[6];
   int l,r,t,b;

   l = rect->left; r = rect->right; t = rect->top; b = rect->bottom;
   hPen = CreatePen(PS_SOLID,(int)NULL,GetSysColor(COLOR_BTNHIGHLIGHT));
   hOldPen = SelectObject(hDC,hPen);

   MoveTo(hDC,l,b-1);
   LineTo(hDC,l,t);
   LineTo(hDC,r-1,t);
   MoveTo(hDC,l+2,b-2);
   LineTo(hDC,r-2,b-2);
   LineTo(hDC,r-2,t+2);

   SelectObject( hDC, hOldPen );
   DeleteObject(hPen);

   hPen = CreatePen(PS_SOLID,(int)NULL,GetSysColor(COLOR_BTNSHADOW));
   SelectObject(hDC,hPen);

   MoveTo(hDC,l+1,b);
   LineTo(hDC,r,b);
   LineTo(hDC,r,t+1);
   MoveTo(hDC,l+2,b-2);
   LineTo(hDC,l+2,t+2);
   LineTo(hDC,r-2,t+2);

   SelectObject(hDC,hOldPen);
   DeleteObject(hPen);
}

LONG FAR PASCAL _export countproc(R4HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   R4HDC hDC;
   PAINTSTRUCT ps;
   R4RECT rect;
   TEXTMETRIC tm;

   switch(message)
   {
      case WM_COMMAND:
	 if(wParam == IDCANCEL)
	    ((REPORT4 *)GetWindowLong(GetParent(hWnd),0))->report_command = -1;
	 break ;

      case WM_PAINT:
	 hDC = BeginPaint(hWnd,&ps);
	 GetTextMetrics(hDC,&tm);
	 GetClientRect(hWnd,&rect);
	 Draw4Area(hDC,&rect);
	 SetBkMode(hDC,TRANSPARENT);
	 rect.bottom = tm.tmHeight + 2;
	 DrawText(hDC,"STATUS",6,&rect,DT_CENTER | DT_VCENTER);
	 rect.left = 2 * tm.tmAveCharWidth ;
	 rect.top = 2 * tm.tmHeight;
	 rect.bottom = rect.top + 2 + tm.tmHeight;
	 DrawText(hDC,"Records Scanned",15,&rect,DT_LEFT | DT_VCENTER);
	 rect.top = 3 * tm.tmHeight + tm.tmHeight/2;
	 rect.bottom = rect.top + 2 + tm.tmHeight;
	 DrawText(hDC,"Records Selected",16,&rect,DT_LEFT | DT_VCENTER);
	 rect.top = 5 * tm.tmHeight;
	 rect.bottom = rect.top + 2 + tm.tmHeight;
	 DrawText(hDC,"Records Sorted",14,&rect,DT_LEFT | DT_VCENTER);
	 EndPaint(hWnd,&ps);
	 break;

      case WM_CTLCOLOR:
	 if(HIWORD(lParam) == CTLCOLOR_STATIC)
	 {
	    SetBkMode(wParam,TRANSPARENT);
	    return((DWORD)(GetStockObject(LTGRAY_BRUSH)));
	 }
	 break;
   }

   return DefWindowProc(hWnd,message,wParam,lParam);
}
#endif


#ifdef S4REPORT
#ifdef S4WINDOWS
void report4statwin_close(REPORT4 *r4)
{
   DestroyWindow( r4->cb->hWnd );
}

R4HWND report4statwin (REPORT4 *r4)
{
   R4HWND statwin, textwin;
   R4HDC hDC;
   TEXTMETRIC tm;
   R4RECT parent;

   hDC = GetDC(r4->hWndParent);
   GetTextMetrics(hDC,&tm);
   ReleaseDC(r4->hWndParent,hDC);
   GetClientRect(r4->hWndParent,&parent);
   statwin = CreateWindow("counter","Status",WS_POPUP | WS_BORDER | WS_VISIBLE,
			       (parent.right/2) - ( 17 * tm.tmAveCharWidth),
			       (parent.bottom/2) - (4 * tm.tmHeight),
			       (44 * tm.tmAveCharWidth),
			       (8 * tm.tmHeight),
			       /*(r4->to_screen)?r4->hWnd:r4->hWndParent */
			       NULL,NULL,r4->cb->hInst,NULL);

   ShowWindow(statwin,SW_SHOW);
   UpdateWindow(statwin);
   SetWindowWord(statwin,0,0);

   GetClientRect(statwin,&parent);

   textwin = CreateWindow("STATIC","",WS_CHILD | WS_BORDER | WS_VISIBLE,
				20 * tm.tmAveCharWidth,
				2 * tm.tmHeight + 1,
				(20 * tm.tmAveCharWidth),
				tm.tmHeight,
				statwin,
				NULL,
				r4->cb->hInst,
				NULL);
   SetWindowWord(statwin,0,(WORD)textwin);
   textwin = CreateWindow("STATIC","",WS_CHILD | WS_BORDER | WS_VISIBLE,
				20 * tm.tmAveCharWidth,
				3 * tm.tmHeight + 1 + tm.tmHeight/2,
				(20 * tm.tmAveCharWidth),
				tm.tmHeight,
				statwin,
				NULL,
				r4->cb->hInst,
				NULL);
   SetWindowWord(statwin,2,(WORD)textwin);
   textwin = CreateWindow("STATIC","",WS_CHILD | WS_BORDER | WS_VISIBLE,
				20 * tm.tmAveCharWidth,
				5 * tm.tmHeight + 1,
				(20 * tm.tmAveCharWidth),
				tm.tmHeight,
				statwin,
				NULL,
				r4->cb->hInst,
				NULL);
   SetWindowWord(statwin,4,(WORD)textwin);
   SetWindowWord(statwin,8,(WORD)666);

   return statwin;

}
#endif
#endif

void report4make_old_rec(RELATE4 *relate)
{
   while(relate)
   {
      memcpy(relate->old_record,relate->data->record,relate->data->record_width);
      relate4next(&relate);
   }
}


#ifdef S4WINDOWS
int report4setup_screen(REPORT4 *r4)
{
   int width, height, max_pos, num_bits, planes, units_per_page;
   HMENU hMenu;
   POINT point;
   R4RECT r;

   width =  GetSystemMetrics( SM_CXFULLSCREEN ) ;
   height =  GetSystemMetrics( SM_CYFULLSCREEN ) ;

   hMenu = CreateMenu();
   AppendMenu( hMenu, MF_ENABLED | MF_STRING, 100, "&Next Page");
   AppendMenu( hMenu, MF_ENABLED | MF_STRING, 101, "&Close Window");

   r4->hWnd = CreateWindow( "output4", (r4->report_name)?(LPCSTR)(r4->report_name):(LPCSTR)"CodeReporter",
	       WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_HSCROLL | WS_MAXIMIZE,
	       0,0, width, height,
	       r4->hWndParent, hMenu, r4->cb->hInst, NULL ) ;

   if( r4->hWnd == 0 )
   {
      MessageBox( r4->hWndParent, E4_REPORT_CRE, E4_ERROR, MB_OK ) ;
      return -1 ;
   }

   SetWindowLong( r4->hWnd, 0, (LONG)(r4) ) ;

   r4->hDCdisplay =  GetDC( r4->hWnd ) ;

   if( r4->hDCdisplay == 0 )
   {
      MessageBox( r4->hWndParent, E4_REPORT_DIS, E4_ERROR, MB_OK ) ;
      DestroyWindow(r4->hWnd);
      return -1 ;
   }

   dc4mapping( r4->hDCdisplay ) ;

   point.x = r4->report_width ;
   LPtoDP( r4->hDCdisplay, &point, 1 ) ;
   SetMapMode( r4->hDCdisplay, MM_TEXT ) ;

   max_pos =  point.x - width ;
   if( max_pos < 0 )  max_pos =  0 ;
   SetScrollRange( r4->hWnd, SB_HORZ, 0, max_pos, 0 ) ;

   GetClientRect( r4->hWnd, &r ) ;
   units_per_page =  r.bottom ;

   r4->hDC = CreateCompatibleDC( r4->hDCdisplay ) ;

   if( r4->hDC == 0 )
   {
      MessageBox( r4->hWndParent, E4_REPORT_DIS, E4_ERROR, MB_OK ) ;
      ReleaseDC(r4->hWnd,r4->hDCdisplay);
      DestroyWindow(r4->hWnd);
      return -1 ;
   }
   
   num_bits = GetDeviceCaps( r4->hDCdisplay, BITSPIXEL ) ;
   planes = GetDeviceCaps( r4->hDCdisplay, PLANES ) ;
   (HBITMAP)r4->hBitMap = CreateBitmap( point.x, units_per_page, planes, num_bits, 0);

   if( r4->hBitMap == 0 )
   {
      MessageBox( r4->hWndParent, E4_REPORT_BIT, E4_ERROR, MB_OK ) ;
      ReleaseDC(r4->hWnd,r4->hDCdisplay);
      DestroyWindow(r4->hWnd);
      DeleteDC(r4->hDC);
      return -1 ;
   }

   SelectObject( r4->hDC, (HBITMAP)r4->hBitMap ) ;

   PatBlt( r4->hDC, 0, 0, point.x, units_per_page, WHITENESS ) ;
   ShowWindow( r4->hWnd, SW_SHOW ) ;

   return units_per_page;
}


int report4setup_printer(REPORT4 *r4, FARPROC *report4abort_proc_instance, R4HWND *hCancel, R4HWND *hButton )
{
   DWORD units;
   int units_x;
   int units_y;

   *report4abort_proc_instance =
      MakeProcInstance( (FARPROC) report4abort_proc, r4->cb->hInst ) ;

   if(!r4->hDC)
      report4dc_get( r4 ) ;

/*   SetAbortProc( r4->hDC, *report4abort_proc_instance ) ; */
   Escape(r4->hDC,SETABORTPROC, 0, (LPSTR) *report4abort_proc_instance, NULL);

   if ( Escape( r4->hDC, STARTDOC, 12, E4_REPORT_COD, 0 ) < 0 )
   {
      e4( r4->cb, e4result, E4_RESULT_STA ) ;
      report4dc_free( r4 ) ;
      FreeProcInstance( *report4abort_proc_instance ) ;
      return -1 ;
   }

   units = GetDialogBaseUnits() ;
   units_x = LOWORD(units) ;
   units_y = HIWORD(units) ;

   *hCancel = CreateWindow( "cancel4",  E4_REPORT_PRI,
      WS_OVERLAPPED | WS_VISIBLE | WS_THICKFRAME | WS_CAPTION,
      units_x * 10,
      units_y * 10,
      units_x * 18,
      units_y * 8,
      r4->hWndParent,
      0,
      r4->cb->hInst,
      0 ) ;

   SetWindowLong( *hCancel, 0, (LONG) r4 ) ;

   *hButton = CreateWindow( "button",  E4_MESSAG_CAN,
      WS_CHILD | WS_VISIBLE,
      units_x * 4,
      units_y * 2,
      units_x * 8,
      units_y * 3/2,
      *hCancel,
      IDCANCEL,
      r4->cb->hInst,
      0 ) ;

   return 0;
}

int report4cleanup_screen(REPORT4 *r4)
{
   if( r4->report_command >= 0 )
      report4new_page( r4, 0 ) ; /* Let the user see the last page. */

   if( r4->hWndParent )
      EnableWindow( r4->hWndParent, 1 ) ;

   DeleteDC( r4->hDC ) ;
   r4->hDC = 0 ;
   ReleaseDC( r4->hWnd, r4->hDCdisplay ) ;
   r4->hDCdisplay = 0 ;
   DestroyWindow( r4->hWnd ) ;
   r4->hWnd = 0 ;
   DeleteObject( (HBITMAP)r4->hBitMap ) ;
   r4->hBitMap = 0 ;

   return 0;
}


int report4cleanup_printer(REPORT4 *r4, FARPROC *report4abort_proc_instance, R4HWND *hCancel, R4HWND *hButton)
{
   STYLE4 *style_on;
   PR4LOGFONT plf ;

   report4dc_free( r4 ) ;
   FreeProcInstance( *report4abort_proc_instance ) ;

   if( r4->hWndParent )
      EnableWindow( r4->hWndParent, 1 ) ;

   DestroyWindow( *hCancel ) ;

   r4->hDCdisplay =  GetDC( r4->hWnd ) ;

   style_on = 0; 
   style_on = (STYLE4 *)l4next( &r4->styles, style_on);
   while(style_on)
   {

      DeleteObject( style_on->hFont ) ;
      plf = &(style_on->lf) ;
      plf->lfHeight = -MulDiv(style_on->iptsize,GetDeviceCaps(r4->hDCdisplay,
      LOGPIXELSY),72);
   
      style_on->hFont = CreateFontIndirect(plf);
      if ( style_on->hFont == 0 )
      {
	 e4describe( r4->cb, e4result, E4_REPORT_FON, plf->lfFaceName,
	 (char *) 0 ) ;
	 return 0 ;
      }
      style_on = (STYLE4 *)l4next( &r4->styles, style_on);
   }
   
   ReleaseDC( r4->hWnd, r4->hDCdisplay ) ;

   return 0;
}
#endif

int report4setup_margin_values( REPORT4 *r4)
{
   OBJ4 *obj_on ;
   TEXT4 *tx4;
   #ifdef S4WINDOWS
      TEXTMETRIC tm;
   #endif

   /* set margin values etc for each object */
   for( obj_on = obj4first(r4); obj_on; obj_on = obj4next(obj_on) )
   {
      if(obj_on->display_status == text4displayed)
	 obj_on->display_status = text4disp_once ;

      #ifdef S4WINDOWS
      SelectObject( r4->hDC, ((TEXT4 *) obj_on)->style->hFont ) ;
      GetTextMetrics( r4->hDC, &tm ) ;
      
      obj_on->r.left = obj_on->x + r4->margin_left ;
      obj_on->r.top = obj_on->y ;
      obj_on->r.right =  obj_on->x + obj_on->w ;

      if( obj_on->w == 0 )
	 obj_on->r.right +=  text4width_estimate( (TEXT4 *) obj_on, &tm ) ;

      obj_on->r.bottom =  obj_on->x + tm.tmHeight ;
      LPtoDP( r4->hDC, (POINT *) &obj_on->r, 2 ) ;
      #else
      obj_on->r.left = (int)((float)(obj_on->x + r4->margin_left)/100.0+0.5);
      obj_on->r.top = (int)((float)obj_on->y/1000.0*6.0+0.5) ;
      tx4 = (TEXT4 *)obj_on ;
      obj_on->r.right = obj_on->r.left + tx4->len ;
      #endif
   }
   return 0;
}

int report4setup_objheight( REPORT4 *r4)
{
   OBJECTS4 *objects_on;
   #ifdef S4WINDOWS
      POINT pt ;
   #endif

   /* calculate object heights in device coordinates */
   for( objects_on = objects4first(r4);
   objects_on;objects_on = objects4next(objects_on))
   {
      #ifdef S4WINDOWS
      pt.y =  objects_on->height ;
      LPtoDP( r4->hDC, &pt, 1 ) ;
      objects_on->height_dev =  pt.y ;
      #else
      report4sort(objects_on);
      objects_on->height_dev = (int)((float)objects_on->height/1000.0*6.0+0.5) ;
      #endif
   }

   return 0;
}

int report4setup_expressions( REPORT4 *r4 )
{
   int len;
   GROUP4 *group_on;
   char *ptr;

   group_on = 0; 
   group_on = (GROUP4 *)l4next( &r4->groups, group_on); 
   while(group_on)
   {
      if( group_on->expr == 0 )
      {
	 group_on = (GROUP4 *)l4next( &r4->groups, group_on);
	 continue ;
      }
      len = expr4vary( group_on->expr, &ptr ) ;
      if( group_on->value == 0 )
      {
	 group_on->value = (char *) u4alloc_er( r4->cb, len ) ;
	 group_on->value_len = len ;
	 if( group_on->value == NULL )
	    return -1;
      }
      memcpy( group_on->value, ptr, len ) ;
      group_on = (GROUP4 *)l4next( &r4->groups, group_on); 
   }
   return 0;
}


int report4check_area_heights( REPORT4 *r4, int units_except_page_footer )
{
   GROUP4 *group_on;

   group_on = 0;
   if(r4->title.height_dev >= units_except_page_footer)
   {
      return -1;
   }
   if(r4->summary.height_dev >= units_except_page_footer)
   {
      return -1;
   }
   if(r4->page_footer.height_dev >= units_except_page_footer)
   {
      return -1;
   }
   if(r4->page_header.height_dev >= units_except_page_footer)
   {
      return -1;
   }

   group_on = (GROUP4 *) l4next( &r4->groups, group_on);
   while(group_on)
   {
      if ( group_on->header.height_dev  >= units_except_page_footer - r4->title.height_dev)
      {
	 return -1 ;
      }
      if ( group_on->footer.height_dev >= units_except_page_footer - r4->title.height_dev)
      {
	 return -1 ;
      }
      group_on = (GROUP4 *) l4next( &r4->groups, group_on);
   }

   return 0;
}

#ifdef S4WINDOWS
int report4setup_winfonts( REPORT4 *r4 )
{
   STYLE4 *style_on;
   PR4LOGFONT plf ;

   /* create the fonts for the output */
   style_on = 0; 
   style_on = (STYLE4 *)l4next( &r4->styles, style_on); 
   while(style_on)
   {

      DeleteObject( style_on->hFont ) ;
      plf = &(style_on->lf) ;
      plf->lfHeight = -MulDiv(style_on->iptsize,
	 GetDeviceCaps(r4->hDC,LOGPIXELSY),72);

      style_on->hFont = CreateFontIndirect(plf);
      if ( style_on->hFont == 0 )
      {
	 e4describe( r4->cb, e4result, E4_REPORT_FON, plf->lfFaceName, (char *)0);
	 return 0 ;
      }
      style_on = (STYLE4 *)l4next( &r4->styles, style_on); 
   }
   return 0;
}
#endif
  
void S4FUNCTION report4free( REPORT4 *r4, int free_relate, int close_files )
{
   GROUP4 *group_on, *group_del ;
   STYLE4 *style_on, *style_next;
   RELATE4 *relate_on;

   if ( r4 == 0 )  return ;

   #ifdef S4DEBUG
      if( r4->cb->num_reports != 1 )
	 e4severe( e4result, E4_REPORT_ONE ) ;
   #endif

   r4->cb->num_reports = 0 ;

   relate_on = r4->relate;
   while( relate_on )
   {
      if( relate_on->old_record )
	 u4free( relate_on->old_record );
      relate4next( &relate_on );
   }

   if (free_relate)
      relate4free(r4->relate, close_files);

   objects4purge( &r4->page_header ) ;
   objects4purge( &r4->page_footer ) ;
   objects4purge( &r4->title ) ;
   objects4purge( &r4->summary ) ;

   group_on = 0;
   group_on=(GROUP4 *)l4next(&r4->groups,group_on);
   while(group_on)
   {
      objects4purge( &group_on->header ) ;
      objects4purge( &group_on->footer ) ;

      if( group_on->expr )
      {
	 expr4free( group_on->expr );
	 group_on->expr = 0 ;
      }

      if( group_on->value )
	 u4free( group_on->value );

      group_del = group_on;
      group_on=(GROUP4 *)l4next(&r4->groups,group_on);
      u4free( group_del );
   }

   #ifdef S4WINDOWS
   report4dc_free( r4 ) ;
   #endif

   style_on=0; 
   style_on=(STYLE4 *) l4first(&r4->styles );

   while( style_on )
   {
      style_next = (STYLE4 *) l4next( &r4->styles, style_on );
      l4remove( &r4->styles, style_on );
      #ifdef S4WINDOWS
      DeleteObject( style_on->hFont ) ;
      #else
      if( style_on->codes_before )
	 u4free( style_on->codes_before ) ;
      if( style_on->codes_after )
	 u4free( style_on->codes_after ) ;
      #endif
      mem4free( r4->style_memory, style_on ) ;
      style_on = style_next;
   }

   expr4calc_reset( r4->cb ) ;

   mem4release( r4->text_memory ) ;
   mem4release( r4->style_memory ) ;
   mem4release( r4->cb->total_memory ) ;
   r4->cb->total_memory =  0 ;
   mem4release( r4->cb->calc_memory ) ;
   r4->cb->calc_memory =  0 ;

   if ( r4->report_name ) 
      u4free( r4->report_name ) ;
   u4free( r4 ) ;
}


int S4FUNCTION report4printer( REPORT4 *report, char *printer )
{
   u4free( report->printer_name ) ;
   if( printer == 0 || printer[0] == 0 )
   {
      report->printer_name = 0 ;
      return 0 ;
   }
   report->printer_name =  (char *) u4alloc_er( report->cb, strlen(printer)+1);
   if( report->printer_name == 0 )
   {
      e4set( report->cb, 0 ) ;
      return -1 ;
   }
   strcpy( report->printer_name, printer ) ;
   return 0 ;
}

#ifdef S4WINDOWS
long FAR PASCAL _export report4cancel_proc( R4HWND hWnd, unsigned message,
unsigned wParam, LONG lParam )
{
   REPORT4 *r4;
   switch (message)
   {
      case WM_COMMAND:
	 r4 =  (REPORT4 *) GetWindowLong( hWnd, 0 ) ;
	 r4->report_command =  -1 ;
	 break ;

      default:
	 return( DefWindowProc( hWnd, message, wParam, lParam ) ) ;
   }
   return 0L ;
}

long FAR PASCAL _export report4output_proc( R4HWND hWnd, unsigned message,
unsigned wParam, LONG lParam )
{
   R4RECT r ;
   int cur_position, line_scroll, page_scroll, horz_vert ;
   int min_pos, max_pos, new_position ;
   R4HDC hDC;
   PAINTSTRUCT ps;

   REPORT4 *r4 =  (REPORT4 *) GetWindowLong( hWnd, 0 ) ;
   switch (message)
   {
      case WM_PAINT:
      {
	 hDC = BeginPaint( hWnd, &ps );
	 GetClientRect( r4->hWnd, &r ) ;
	 cur_position =  GetScrollPos( hWnd, SB_HORZ ) ;
	 BitBlt( r4->hDCdisplay, 0, 0, r.right, r.bottom, r4->hDC, cur_position,
	 0, SRCCOPY ) ;
	 ValidateRect( hWnd, 0 ) ;
	 EndPaint( hWnd, &ps );
	 break ;
      }

      case WM_COMMAND:
	 if ( LOWORD( lParam ) == 0 )
	 {
	    if( wParam == 100 ) /* Screen Pan */
	       r4->report_command = 1 ;

	    if( wParam == 101 ) /* Close Window */ 
	       r4->report_command = -1 ;
	 }
	 break ;

      case WM_HSCROLL:
      {
	 GetScrollRange( hWnd, SB_HORZ, (int *)&min_pos, (int *)&max_pos ) ;

	 line_scroll =  max_pos / 8 ;
	 page_scroll =  max_pos / 4 ;

	 cur_position =  new_position =  GetScrollPos( hWnd, SB_HORZ ) ;
	 if( line_scroll <= 0 )  line_scroll =  1 ;
	 if( page_scroll <= 0 )  page_scroll =  1 ;

	 switch( wParam )
	 {
	    case SB_LINEUP:
	       new_position -=  line_scroll ;
	       break ;

	    case SB_LINEDOWN:
	       new_position +=  line_scroll ;
	       break ;

	    case SB_PAGEUP:
	       new_position -=  page_scroll ;
	       break ;

	    case SB_PAGEDOWN:
	       new_position +=  page_scroll ;
	       break ;

	    case SB_THUMBPOSITION:
	       new_position =  LOWORD(lParam) ;
	       break ;
	 }

	 if( new_position != cur_position )
	 {
	    if( new_position < min_pos )
	       new_position =  min_pos ;

	    if( new_position > max_pos )
	       new_position =  max_pos ;

	    SetScrollPos( hWnd, SB_HORZ, new_position, TRUE ) ;
	    InvalidateRect( hWnd, 0, 1 ) ;
	 }
	 break ;
      }

      case WM_KEYDOWN:
	 if( wParam ==  VK_ESCAPE )
	    r4->report_command = -1 ;
	 break ;

      case WM_CLOSE:
	 r4->report_command = -1 ;
	 break ;

      default:
	 return( DefWindowProc( hWnd, message, wParam, lParam ) ) ;
   }

   return 0L ;
}
#endif

int S4FUNCTION report4currency( REPORT4 *r4, char currency_symbol )
{
   if (r4 == NULL) return 0;

   r4->currency_sym = currency_symbol;

   return currency_symbol;
}

int S4FUNCTION report4symbols_numeric( REPORT4 *r4,int thousand_separator,
int decimal_point)
{
   if(r4 == NULL) return 0;

   r4->thousand_separator = (char)thousand_separator;

   r4->decimal_point = (char)decimal_point;

   return 1;
}

void report4obj_destroy( OBJ4 *obj )
{
   if( obj == 0 )  return ;
   #ifdef S4WINDOWS
   if( obj->hWnd != 0 )
   {
      DestroyWindow(obj->hWnd) ;
      obj->hWnd =  0 ;
   }
   #endif
   obj4free( obj ) ;
}

void report4objects_destroy( OBJECTS4 *list )
{
   #ifdef S4WINDOWS
   if( list->hWnd == 0 )
      return ;
   #endif

   while( list->list.n_link > 0 )
      report4obj_destroy( (OBJ4 *) l4first(&list->list) ) ;
   #ifdef S4WINDOWS
   DestroyWindow(list->hWnd) ;
   if( list->hWndInfo != 0 )
   {
      DestroyWindow( list->hWndInfo ) ;
      list->hWndInfo =  0 ;
   }
   list->hWnd =  0 ;
   #endif
   return ;
}

void S4FUNCTION report4output_handle( REPORT4 *r4, int handle )
{
   r4->output_handle = handle ;
}


int S4FUNCTION report4parse_sstring( char *sstring, char *codes, int len)
{
   int i, j, temp;

   j = 0;
   i = 0;
   while( j < len )
   {
      if(sstring[i] == '\\')
      {
	 temp = 0;
	 if(sstring[i+1] != 'x' && sstring[i+1] != 'X')
	 {
	    temp = c4atoi(sstring+i+1,3);
	 }
	 else
	 {
	    if(sstring[i+2] > 47 && sstring[i+2] < 58)
	       temp += (int)sstring[i+2]-48;
	    if(sstring[i+2] > 64 && sstring[i+2] < 71)
	       temp += (int)sstring[i+2]-55;
	    if(sstring[i+2] > 96 && sstring[i+2] < 103)
	       temp += (int)sstring[i+2]-87;

	    temp *= 16;

	    if(sstring[i+3] > 47 && sstring[i+3] < 58)
	       temp += (int)sstring[i+3]-48;
	    if(sstring[i+3] > 64 && sstring[i+3] < 71)
	       temp += (int)sstring[i+3]-55;
	    if(sstring[i+3] > 96 && sstring[i+3] < 103)
	       temp += (int)sstring[i+3]-87;
	 }
	 codes[j] = (unsigned char)temp;
	 i+=3;
      }
      else
	 codes[j] = sstring[i];
      j++;
      i++;
   }
   return j;
}

void S4FUNCTION report4to_hex(char *hex, char *result)
{
   int value, d1, d2;
   value = (int)*hex;
   d1 = value / 16 ;
   if(d1 < 10)
      result[0] = (unsigned char) (d1 + 48) ;
   else
      result[0] = (unsigned char) (d1 + 55) ;

   d2 = value - (d1*16) ;
   if(d2 < 10)
      result[1] = (unsigned char) (d2 + 48) ;
   else
      result[1] = (unsigned char) (d2 + 55) ;
}

void S4FUNCTION report4unparse_sstring( char *sstring, char *codes, int len )
{
   int i, j ;

   j = 0 ;
   for(i = 0; i < len; i++)
   {
      codes[j] = '\\'; j++;
      codes[j] = 'x' ; j++;

      report4to_hex( &(sstring[i]), &(codes[j]) );

      j += 2;
   }
   codes[j] = '\0';
}

int S4FUNCTION report4do( REPORT4 *r4 )
{
   #ifdef S4WINDOWS
      int units_per_page, marg_bottom ;
      R4HWND hButton, hCancel = 0 ;
      POINT point ;
      R4RECT r ;
      TEXTMETRIC tm ;
      R4HWND tempwin;
      FARPROC report4abort_proc_instance ;
   #endif
   int skip_rc, ttt, i ;
   int units_except_page_footer, first=0, return_rc=0 ;
   GROUP4 *group_on, *group_first, *treset_group = 0 ;
   OBJ4 *obj_on ;
   POINT margin_y ;
   char *tempptr ;
   TOTAL4 *total_on ;
   TEXT4 *textreset ;
   RELATE4 *relate_on;
   #ifdef S4WINDOWS
      int perrflag = 0;
   #endif
   int len;
   char *ptr;

   #ifdef S4DEBUG
      #ifdef S4WINDOWS
	 if( r4->cb->hInst == NULL )
	    e4( r4->cb, e4report, E4_REPORT_HINST ) ;
	 if( r4->hWndParent == NULL )
	    e4( r4->cb, e4report, E4_REPORT_HWND );
      #endif
   #endif

   if( (report4driver_init( r4, r4->output_handle, r4->printer_name )) < 0 )
      return -1;

   relate4skip_enable(r4->relate,1);
   #ifdef S4WINDOWS
   report4register_classes( r4 ) ;         /* registers two windows classes */

   r4->report_command =  1 ;

   if( r4->to_screen )                     /*sets things for output to screen*/
   {
      units_per_page = report4setup_screen(r4);
      if( units_per_page == -1 )
      {
   	   report4cleanup_screen(r4);
	      report4driver_init_undo();
	      return -1;
      }
   }
   else    /* for output to printer */
   {
      if(report4setup_printer(r4, &report4abort_proc_instance, &hCancel, &hButton ) == -1)
      {
	 #ifdef S4WINDOWS
	    perrflag = 1;
	 #endif
	 if( r4->report_command < 0 )
	    Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	 else
	 {
	    if ( Escape( r4->hDC, NEWFRAME, 0, NULL, NULL) < 0 )
	       e4( r4->cb, e4result, "On New Frame" );

	    if( r4->report_command < 0 )
	       Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	    else
	       if ( Escape( r4->hDC, ENDDOC, 0, 0, 0 ) < 0 )
		  e4( r4->cb, e4result, "Ending Document" );
	 }
	 report4cleanup_printer( r4, &report4abort_proc_instance, &hCancel, &hButton );
	 report4driver_init_undo();
	 return -1;
      }
   }


   dc4mapping( r4->hDC ) ;
   #endif

   relate_on = r4->relate;
   while(relate_on)
   {
      relate_on->old_record = (char *)u4alloc_er(r4->cb,relate_on->data->record_width + 1);
      if( relate_on->old_record == NULL )
      {
         report4driver_init_undo();
         return -1;
      }
      relate4next(&relate_on);
   }

   report4setup_margin_values( r4 );
   report4setup_objheight( r4 );

   #ifdef S4WINDOWS
   margin_y.x = 0;
   margin_y.y = r4->margin_bottom ;
   LPtoDP( r4->hDC, &margin_y, 1 ) ;
   marg_bottom = margin_y.y;

   margin_y.y = r4->margin_top ;
   LPtoDP( r4->hDC, &margin_y, 1 ) ;
   SetMapMode( r4->hDC, MM_TEXT ) ;

   if( ! r4->to_screen )
      units_per_page =  GetDeviceCaps( r4->hDC, VERTRES ) ;

   /* calculate page height less page footer in device units */
   units_except_page_footer =
      units_per_page -  r4->page_footer.height_dev - marg_bottom ;

   r4->y =  margin_y.y ;
   #else
   margin_y.y = (int)((float)r4->margin_top/1000.0 * 6.0 + 0.5) ;
   margin_y.x = (int)((float)r4->margin_bottom/1000.0 * 6.0 + 0.5) ;
   units_except_page_footer = r4->sheight - r4->page_footer.height_dev - margin_y.x ;
   r4->y = margin_y.y ;
   r4->cline = r4->y;
   r4->swidth = r4->swidth - (int)((float)r4->margin_left/100.0 + 0.5)
      - (int)((float)r4->margin_right/100.0 + 0.5) ;

   #endif

   r4->first =  0 ;

   #ifdef S4WINDOWS
      if(r4->to_screen)
      {
	 SetClassWord(r4->hWnd,GCW_HCURSOR, (WORD)NULL);
	 SetCursor(LoadCursor((HINSTANCE)NULL,IDC_WAIT));
      }
      #ifdef S4REPORT
	 tempwin = r4->cb->hWnd;
	 r4->cb->hWnd = report4statwin(r4);
      #endif
      skip_rc = relate4top(r4->relate);
      #ifdef S4REPORT
	 report4statwin_close(r4);
	 r4->cb->hWnd = tempwin;
      #endif
      if(skip_rc != 0)
      {
	 if(r4->to_screen)
	    report4cleanup_screen(r4);
	 else
	 {
	    if( r4->report_command < 0 )
	       Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	    else
	    {
	       if ( Escape( r4->hDC, NEWFRAME, 0, NULL, NULL) < 0 )
		  e4( r4->cb, e4result, "On New Frame" );

	       if( r4->report_command < 0 )
		  Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	       else
		  if ( Escape( r4->hDC, ENDDOC, 0, NULL, NULL ) < 0 )
		     e4( r4->cb, e4result, "Ending Document" );
	    }
	    report4cleanup_printer( r4, &report4abort_proc_instance, &hCancel, &hButton );
	 }

	 SetFocus(r4->hWndParent);
	 report4driver_init_undo();
	 return skip_rc;
      }
   #else
   skip_rc =  relate4top( r4->relate ) ;
   if(skip_rc != 0)
   {
      report4driver_init_undo();
      return skip_rc;
   }
   #endif

   if( report4setup_expressions( r4 ) < 0 )
   {
      report4driver_init_undo();
      return -1;
   }

   if( report4check_area_heights( r4, units_except_page_footer ) < 0 )
   {
      #ifdef S4WINDOWS
      if( r4->to_screen )
	 report4cleanup_screen( r4 );
      else
      {
	 if( r4->report_command < 0 )
	    Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	 else
	 {
	    if ( Escape( r4->hDC, NEWFRAME, 0, NULL, NULL) < 0 )
	       e4( r4->cb, e4result, "On New Frame" );

	    if( r4->report_command < 0 )
	       Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	    else
	       if ( Escape( r4->hDC, ENDDOC, 0, 0, 0 ) < 0 )
		  e4( r4->cb, e4result, "Ending Document" );
	 }
	 report4cleanup_printer(r4,&report4abort_proc_instance,&hCancel,&hButton);
      }

      SetFocus(r4->hWndParent);
      #endif
      e4describe( r4->cb, e4report, "A Header or Footer area is too large for the display.",
		  "Try resizing the areas and displaying again.", NULL );
      report4driver_init_undo();
      return -1;
   }

   #ifdef S4WINDOWS
   report4setup_winfonts( r4 );
   #endif

   r4->cb->pageno = 1;

    /* display title and set first group */
   group_first =  (GROUP4 *) l4first( &r4->groups ) ;
   objects4display( &r4->title ) ;
   if( !group_first->swap_header )
      objects4display( &r4->page_header ) ;

   first = 0 ;

   #ifdef S4WINDOWS
   if( r4->hWndParent )
      EnableWindow( r4->hWndParent, 0 ) ;
   #endif

   total_on = 0; 
   total_on = (TOTAL4 *) l4next( &r4->cb->total_list,total_on);
   while(total_on)
   {
      total4value_reset(total_on);
      total_on = (TOTAL4 *) l4next( &r4->cb->total_list,total_on);
   }

    /* for each of the records in the relation */
   for (; skip_rc == 0;)
   {
      #ifdef S4WINDOWS
      report4message_loop( r4 ) ;
      if( r4->report_command < 0 )  break ;
      #endif

      /* Update Totals */
      total_on = 0;
      total_on = (TOTAL4 *) l4next( &r4->cb->total_list, total_on);
      while(total_on)
      {
	 total4value_update( total_on, treset_group ) ;
	 total_on = (TOTAL4 *) l4next( &r4->cb->total_list, total_on);
      }

      if(treset_group)treset_group = 0;

      group_on = (GROUP4 *)l4next( &r4->groups, group_first );
      while( group_on )
      {
	 if( group_on->expr )
	 {
	    len = expr4vary( group_on->expr, &ptr );

	    if( group_on->value == 0 )
	    {
	       group_on->value =  (char *) u4alloc_er( r4->cb, len ) ;
	       if( group_on->value == 0 )
	       {
		  report4driver_init_undo();
		  return 0 ;
	       }

	       group_on->value_len = len ;
	    }
	    memcpy( group_on->value, ptr, len ) ;
	 }
	 group_on = (GROUP4 *)l4next( &r4->groups, group_on );
      }

      /* for each group */
      for ( group_on = group_first; group_on != 0;
      group_on = (GROUP4 *) l4next( &r4->groups, group_on))
      {
	 /* if not enough room to print header before bottom of page */
	 #ifdef S4WINDOWS
	     if ( (group_on->header.height_dev + r4->y > units_except_page_footer) ||
	 ( group_on->reset_page && (group_on->position != 1 || first)) || (group_on->swap_header && (group_on->position != 1 || first)) )
	 #else
	 if ( (group_on->header.height_dev + r4->y - 1 > units_except_page_footer) ||
	 ( group_on->reset_page && (group_on->position != 1 || first)) || (group_on->swap_header && (group_on->position != 1 || first)) )
	 #endif
	 {
	    if( r4->y > margin_y.y )
	    {
	       r4->y =  units_except_page_footer ;
	       if( !group_on->swap_footer )
		  objects4display( &r4->page_footer ) ;
	 
	       return_rc = report4new_page( r4, margin_y.y ) ;
	       if( group_on->reset_page == 2 )
		  r4->cb->pageno = 1;

	       if( return_rc < 0 )  break ;

	       #ifdef S4WINDOWS
	       if( r4->report_command < 0 )  break ;
	       #endif
	    }

	    /* if swap headers not set display page header */
	    if ( ! group_on->swap_header )
	    {
	       objects4display( &r4->page_header ) ;
	       report4display_repeat_headers( r4, group_on ) ;
	       if( !group_on->swap_footer )
		  objects4display( &group_on->header ) ;
	    }
	    else
	    {
	       objects4display( &group_on->header ) ;
	       report4display_repeat_headers( r4, group_on ) ;
	    }
	 }
	 else
	    objects4display( &group_on->header ) ;

	 first = 1 ;
      }
      /*end of for loop */

      /*if there is a problem break out of for loop */
      if( return_rc < 0 )  break ;

      #ifdef S4WINDOWS
      if( r4->report_command < 0 )  break ;
      #endif

      report4make_old_rec(r4->relate);
      ttt = skip_rc = relate4skip( r4->relate, 1L ) ;
      if(skip_rc == r4terminate || skip_rc < 0)
      {
	 return_rc = skip_rc;
	 break;
      }

      while( ttt == 0 )
      {
	 if(r4->groups.n_link > 0)
	    group_first = report4calc_first_change_group( r4 );

	 if( group_first ) break;

	 total_on = 0;
	 total_on = (TOTAL4 *) l4next( &r4->cb->total_list, total_on);
	 while(total_on)
	 {
	    total4value_update( total_on, treset_group ) ;
	    total_on = (TOTAL4 *) l4next( &r4->cb->total_list, total_on);
	 }

	 report4make_old_rec(r4->relate);
	 ttt = skip_rc = relate4skip( r4->relate, 1L ) ;
      }

      if(skip_rc == r4terminate || skip_rc < 0)
      {
	 return_rc = skip_rc;
	 break;
      }


      treset_group = group_first;

      if( group_first == 0 )
	 group_first = (GROUP4 *) l4first( &r4->groups ) ;

      relate_on = r4->relate;
      while(relate_on)
      {
	 tempptr = relate_on->data->record;
	 relate_on->data->record = relate_on->old_record;
	 relate_on->old_record = tempptr;
	 if ( relate_on->data->fields_memo != 0 )
	 {
	    for ( i = 0; i < relate_on->data->n_fields_memo ; i++ )
	       relate_on->data->fields_memo[i].status = 1;
	 }
	 relate4next(&relate_on);
      }


      do
      {

	 group_on =  (GROUP4 *) l4prev( &r4->groups, group_on ) ;
	 if(group_on == NULL) break ;

	 /* if the current footer will go past the page end */
	 if(group_on->footer.height_dev+r4->y-1 > units_except_page_footer ||
	    group_on->swap_footer)
	 {
	    if(!group_on->swap_footer)
	    {
	       r4->y =  units_except_page_footer ;
	       objects4display( &r4->page_footer ) ;
	    }
	    else
	    {
	       r4->y = units_except_page_footer+r4->page_footer.height_dev-group_on->footer.height_dev;
	       objects4display( &group_on->footer );
	    }

	    if( !(group_on->swap_footer && group_on == group_first && r4->summary.height == 0 ) )
	    {
	       return_rc = report4new_page( r4, margin_y.y ) ;
	       if( group_on->reset_page == 2 )
		  r4->cb->pageno = 1;
      
	       if( return_rc < 0 )  break ;

	       if( r4->report_command < 0 )  break ;

	       if( !group_on->swap_header )
		  objects4display( &r4->page_header ) ;
	       else
		  objects4display( &group_on->header );

	       report4display_repeat_headers( r4, group_on ) ;
	       if( !group_on->swap_footer )
		  objects4display( &group_on->footer );
	    }
	 }
	 else
	    objects4display( &group_on->footer ) ;

      } while( group_on != group_first ) ;
      /* end of do loop */

      for ( group_on = group_first; group_on != 0;
      group_on = (GROUP4 *) l4next( &r4->groups, group_on))
      {
	 obj_on = 0;
	 obj_on = (OBJ4 *) l4next( &group_on->header.list, obj_on); 
	 while(obj_on)
	 {
	    textreset = (TEXT4 *)obj_on ;
	    if( obj_on->display_status == text4displayed &&
	    textreset->reset_group->position >= group_first->position )
	       obj_on->display_status =  text4disp_once ;
	    obj_on = (OBJ4 *) l4next( &group_on->header.list, obj_on); 
	 }

	 obj_on = 0;
	 obj_on = (OBJ4 *) l4next( &group_on->footer.list, obj_on); 
	 while(obj_on)
	 {
	    textreset = (TEXT4 *)obj_on ;
	    if( obj_on->display_status == text4displayed &&
	    textreset->reset_group->position >= group_first->position )
	       obj_on->display_status =  text4disp_once ;
	    obj_on = (OBJ4 *) l4next( &group_on->footer.list, obj_on); 
	 }

      }



      if( return_rc < 0 )  break ;

      #ifdef S4WINDOWS
      report4message_loop( r4 ) ;
      if( r4->report_command < 0 )  break ;
      #endif

      relate_on = r4->relate;
      while(relate_on)
      {
	 tempptr = relate_on->data->record;
	 relate_on->data->record = relate_on->old_record;
	 relate_on->old_record = tempptr;
	 if ( relate_on->data->fields_memo != 0 )
	 {
	    for ( i = 0; i < relate_on->data->n_fields_memo ; i++ )
	       relate_on->data->fields_memo[i].status = 1;
	 }
	 relate4next(&relate_on);
      }

   }
   /* end of for each record in relation */

   #ifdef S4WINDOWS
   if(r4->report_command >= 0 && return_rc >= 0)
   {
   #else
   if(return_rc >= 0)
   {
   #endif
      group_on = group_first;
      group_first = (GROUP4 *)l4first(&r4->groups);
      if(group_first != group_on)
	 do
	 {

	    group_on =  (GROUP4 *) l4prev( &r4->groups, group_on ) ;
	    if(group_on == NULL) break ;

	    /* if the current footer will go past the page end */
	    if(group_on->footer.height_dev+r4->y-1 > units_except_page_footer ||
	       group_on->swap_footer)
	    {
	       if(!group_on->swap_footer)
	       {
		  r4->y =  units_except_page_footer ;
		  objects4display( &r4->page_footer ) ;
	       }
	       else
	       {
		  r4->y = units_except_page_footer+r4->page_footer.height_dev-group_on->footer.height_dev;
		  objects4display( &group_on->footer );
	       }

		if( !(group_on->swap_footer && group_on == group_first && r4->summary.height == 0 ) )
		{
		  return_rc = report4new_page( r4, margin_y.y ) ;
		  if( group_on->reset_page == 2 )
		     r4->cb->pageno = 1;
	       
		  if( return_rc < 0 )  break ;

		  if( r4->report_command < 0 )  break ;

		  objects4display( &r4->page_header ) ;

		  report4display_repeat_headers( r4, group_on ) ;
		  if( !group_on->swap_footer )
		     objects4display( &group_on->footer );
	       }
	    }
	    else
	       objects4display( &group_on->footer ) ;

	 } while( group_on != group_first ) ;

      if( r4->summary.height > 0 )
      {
	 if(r4->summary.height_dev + r4->y-1 > units_except_page_footer)
	 {
	    r4->y = units_except_page_footer;
	    objects4display(&r4->page_footer);
	    return_rc = report4new_page(r4,margin_y.y);
	    objects4display(&r4->page_header);
	 }
	 objects4display(&r4->summary);
	 r4->y = units_except_page_footer;
	 objects4display(&r4->page_footer);
      }
      else
      {
	 if( r4->y <= units_except_page_footer )
	 {
	    r4->y = units_except_page_footer;
	    objects4display( &r4->page_footer );
	 }
      }
   }


   #ifdef S4WINDOWS
   if ( r4->to_screen )
   {
      report4cleanup_screen( r4 );
   }
   else
   {

      if( r4->report_command < 0 )
	 Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
      else
      if( !perrflag )
      {
	 if ( Escape( r4->hDC, NEWFRAME, 0, NULL, NULL) < 0 )
	    e4( r4->cb, e4result, "On New Frame" );

	 if( r4->report_command < 0 )
	    Escape(r4->hDC,ABORTDOC, 0, NULL, NULL);
	 else
	    if ( Escape( r4->hDC, ENDDOC, 0, 0, 0 ) < 0 )
	       e4( r4->cb, e4result, "Ending Document" );
      }
      report4cleanup_printer( r4, &report4abort_proc_instance, &hCancel, &hButton );
   }
   SetFocus(r4->hWndParent);
   #endif

   #ifndef S4WINDOWS
   if(return_rc >= 0)
      report4driver_new_page();
   #endif

   report4driver_init_undo();
   
   return return_rc ;
}
