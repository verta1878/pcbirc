/* r4driver.c  (c)Copyright Sequiter Software Inc., 1992-1993.  All rights reserved. */

#include "d4all.h"

#ifndef S4WINDOWS
#ifndef S4UNIX
   #include <conio.h>
#endif  /* S4UNIX */
#endif  /* not S4WINDOWS */

#ifdef __TURBOC__
   #pragma hdrstop
#endif

#ifdef S4CODE_SCREENS
  #include "w4.h"
  static int output_screen;
#endif

static int cx = 0, cline = 0, output_code = 1, use_styles = 1;
static int p_width = 80, p_height = 25;
static char output_device[256];

void S4FUNCTION report4output( REPORT4 *report, int out_code,
                               char *out_device, int usestyles )
{

   if( report == NULL )
      return ;

   #ifdef S4WINDOWS
      report4printer(report,out_device);
      if(out_device && out_device[0] )
      {
         strcpy(output_device,out_device);
      }

      if( out_code == 1)
         report->to_screen = output_code = 1;
      else
         report->to_screen = output_code = 0;

      report->use_styles = usestyles;

   #else

      #ifdef S4CODE_SCREENS
         report->output_handle = out_code;
         report->to_screen = out_code;
         output_code = out_code;
         report->use_styles = use_styles = usestyles;
      #else
         report->output_handle = out_code;
         report->to_screen = out_code;
         output_code = out_code;
         report->use_styles = use_styles = usestyles;
      #endif
   #endif
}


int S4FUNCTION report4driver_init( REPORT4 *report, int outputcode,
                               char *outputdevice )
{

   cx = cline = 0;

   #ifdef S4CODE_SCREENS
      
      if( outputcode == 1 )
      {

         output_screen = w4define(0,0,p_height-1,p_width-1);
         w4border(SINGLE,F_WHITE);
         w4activate(output_screen);
         p_height -= 2;
         p_width -= 2;
         w4cursor(p_height-1,0);
      }
   #endif
   return 0;
}

void S4FUNCTION report4driver_init_undo()
{

   #ifdef S4CODE_SCREENS
      w4deactivate(output_screen);
      w4close(output_screen);
      p_width += 2;
      p_height += 2;
      return;
   #else

      return;
   #endif
}

int S4FUNCTION report4driver_new_page()
{


   #ifdef S4CODE_SCREENS
      if( output_code == 1)
      {
         if(getch() == 0x1b)
            return -1;
         w4clear(1);
         w4cursor(p_height-1,0);
      }
      else
      {
         cx = 0;
         cline = 0;
         write(output_code,"\f",1);
      }

      return 0;
   #else
      #ifndef S4WINDOWS
        if(output_code == 1)
        {
           report4position(0,p_height-1);
            cx = 0;
            cline = 0;
            if(getch() == 0x1b)
              return -1;
           write(output_code,"\r\n",2);
         }
         else
         {
            cx = 0;
            cline = 0;
            write(output_code,"\f",1);
         }
         return 0;
      #endif
   #endif
   return 0;
}

int S4FUNCTION report4driver_write( int x, int y, char *string, int string_length, 
                   char *style_start, int style_start_len,
                   char *style_end, int style_end_len )
{

   #ifdef S4CODE_SCREENS
      char *outstring;

      if( output_code == 1 )
      {
         outstring = (char *) u4alloc( string_length + 1 );
         if( outstring )
         {
            memset( outstring, 0, string_length + 1 );
            memcpy( outstring, string, ( x+string_length > p_width )?p_width-x:string_length);
            if( x <= p_width )
               w4( y, x, outstring );
            u4free( outstring );
         }
      }
      else
      {
         if((report4position(x,y)) < 0)
            return -1;
         if(use_styles == 0)
            write(output_code,string,(x + string_length > p_width)?p_width - x : string_length);
         else
         {
            write(output_code,style_start,style_start_len);
            write(output_code,string,(x + string_length > p_width)?p_width - x : string_length);
            write(output_code,style_end,style_end_len);
         }
         cx += string_length;
      }
      return 0;
   #else
      if((report4position(x,y)) < 0)
         return -1;
      if(output_code == 1 || use_styles == 0)
         write(output_code,string,(x + string_length > p_width)?p_width - x : string_length);
      else
      {
         write(output_code,style_start,style_start_len);
         write(output_code,string,(x + string_length > p_width)?p_width - x : string_length);
         write(output_code,style_end,style_end_len);
      }
      cx += string_length;
      return 0;
   #endif
}


int S4FUNCTION  report4position( int x, int y )
{
   char blanks[80] ;

   if( x > p_width || y > p_height)
      return -1;

   if ( y < cline  ||  y == cline && x < cx )
      return -1;

   while ( cline < y )
   {
      cx =  0 ;
      cline++ ;
      write( output_code, "\r\n",2 );
   }

   while ( cx < x )
   {

      memset( blanks, (int) ' ', (size_t) sizeof(blanks) ) ;
      while ( x - cx > sizeof(blanks) )
      {
         cx += sizeof(blanks) ;
         write( output_code, blanks, (unsigned int) sizeof(blanks) ) ;
      }

      write( output_code, blanks, (unsigned int) (x - cx) ) ;
      cx =  x ;
   }

   return 0;
}

void S4FUNCTION report4page_size( REPORT4 *report, int width, int height )
{
   if( width >= 0 && height >= 0 )
   {
      report->sheight = p_height = height ;
      report->swidth = p_width = width ;
   }
}
