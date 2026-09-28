/* i4dump.c   (C)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved.

Displays the contents of a tag.
Currently uses printf */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

/* not supported for FoxPro compact index file structures */
#ifdef S4MDX    /*  ifndef S4FOX  */

static void output( int h, char * s )
{
  write( h, s, strlen(s) ) ;
}

static int  t4dump_do( TAG4 *t4, int out_handle, int level, int display_all )
{
   char out_buf[200] ;
   int rc, i_key ;
   B4BLOCK *block_on ;
   char buf[ I4MAX_KEY_SIZE ] ;
   int len ;

   rc = t4down( t4 ) ;
   if ( rc < 0 || rc == 2 )
      return -1 ;
   block_on = t4block( t4 ) ;

   if ( b4leaf( block_on ) && !display_all )
   {
      t4up( t4 ) ;
      return 0 ;
   }

   if ( b4leaf( block_on) )
   {
      M4PRINT( out_buf, "\r\n\r\nLeaf B4BLOCK #: %ld   Level %d\r\n\r\n", block_on->file_block, level ) ;
      output( out_handle, out_buf ) ;
      output( out_handle, "Record Number   Key\r\n" ) ;
   }
   else
   {
      M4PRINT( out_buf, "\r\n\r\nBranch B4BLOCK #: %ld   Level %d\r\n\r\n", block_on->file_block, level ) ;
      output( out_handle, out_buf ) ;
      output( out_handle, "FILE4 B4BLOCK      Key\r\n" ) ;
   }

   for ( i_key = 0; i_key < block_on->n_keys; i_key++ )
   {
      M4PRINT( out_buf, "\r\n%10ld      ", b4key(block_on,i_key)->num ) ;

      output( out_handle, out_buf ) ;
      if ( t4type( t4 ) == r4str )
      {
         len =  50 ;
         if ( len > t4->header.key_len )
            len =  t4->header.key_len ;

         memcpy( buf, b4key_key( block_on, i_key ), len ) ;
         buf[len] =  0 ;
         output( out_handle, buf ) ;
      }
   }

   if ( b4leaf( block_on ) )
   {
      t4up( t4 ) ;
      return 0 ;
   }

   M4PRINT( out_buf, "\r\n%10ld      ", b4key( block_on, i_key )->num ) ;
   output( out_handle, out_buf ) ;

   block_on->key_on = 0 ;
   do
   {
      rc = t4dump_do( t4, out_handle, level + 1, display_all ) ;
      if ( rc > 0 )
         e4severe( e4result, E4_T4DUMP ) ;
      if ( rc < 0 )
         return -1 ;
   } while ( b4skip( block_on, 1) == 1 ) ;

   t4up(t4) ;
   return 0 ;
}

int S4FUNCTION t4dump( TAG4 *t4, int out_handle, int display_all )
{
   int rc ;

   #ifdef S4DEBUG
      if ( t4 == 0 )
         e4severe( e4parm, E4_T4DUMP ) ;
   #endif

   if ( t4->code_base->error_code < 0 )
      return -1 ;

   rc = i4lock( t4->index ) ;
   if ( rc )
      return rc ;

   if ( t4free_all( t4 ) < 0 )
      return -1 ;

   output( out_handle, "\r\n\r\n" ) ;
   return t4dump_do( t4, out_handle, 0, display_all ) ;
}

#endif

#endif
