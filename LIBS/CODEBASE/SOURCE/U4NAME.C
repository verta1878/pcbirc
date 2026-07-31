/* u4name.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

void S4FUNCTION u4name_ext( char *name, int len_result, char *new_ext, int do_replace )
{
   int file_name_len, ext_pos, on_pos, ext_len ;

   ext_pos = file_name_len = strlen( name ) ;

   for( on_pos = ext_pos-1 ;; on_pos-- )
   {
      if ( name[on_pos] == '.' )
      {
         ext_pos = on_pos ;
         break ;
      }
      #ifdef S4UNIX
         if ( name[on_pos] == '/' )  break ;
      #else
         if ( name[on_pos] == '\\' )  break ;
      #endif
      if ( on_pos == 0 )  break ;
   }

   if ( file_name_len != ext_pos &&  !do_replace )
   {
      #ifdef S4UNIX
         c4lower( name ) ;
      #else
         c4upper( name ) ;
      #endif
      return ;
   }

   if ( *new_ext == '.' )
      new_ext++ ;
   ext_len = strlen( new_ext ) ;

   if ( ext_len > 3 )
      ext_len = 3 ;

   #ifdef S4DEBUG
      if ( len_result <= ext_pos + ext_len + 2 )
	 e4severe( e4result, E4_U4NAME_EXT ) ;
   #endif

   name[ext_pos++] = '.' ;
   strcpy( name + ext_pos, new_ext ) ;

   #ifdef S4UNIX
      c4lower(name) ;
   #else
      c4upper(name) ;
   #endif
}

void S4FUNCTION u4name_make( char *buf, int buf_len, char *default_drive, char *default_directory, char *file_name )
{
   int default_drive_len, default_directory_len, pos ;

   pos = 0 ;

   if( file_name[1] != ':' )
      if ( default_drive != 0 )
      {
         default_drive_len = strlen( default_drive ) ;
         if ( default_drive_len != 2 )
            default_drive_len = 0 ;
         else
            u4ncpy( buf, default_drive, default_drive_len ) ;
         pos += default_drive_len ;
      }

   if ( default_directory != 0 )
      default_directory_len = strlen( default_directory ) ;
   else
      default_directory_len = 0 ;

   #ifdef S4UNIX
      if ( file_name[0] != '/'  &&  default_directory_len > 0 )
   #else
      if ( file_name[0] != '\\'  &&  default_directory_len > 0 )
   #endif
   {
      if ( pos+2 >= buf_len )
         return ;
      #ifdef S4UNIX
         buf[pos++] = '/' ;
         if ( default_directory[0] == '/' )
      #else
         buf[pos++] = '\\' ;
         if ( default_directory[0] == '\\' )
      #endif
         default_directory++ ;

      default_directory_len = strlen(default_directory) ;

      u4ncpy( buf+pos, default_directory, buf_len - pos ) ;
      pos += default_directory_len ;
   }

   if ( pos >= buf_len )
      return ;

   if ( pos > 0 )
   {
      #ifdef S4UNIX
         if ( buf[pos-1] != '/' )
            buf[pos++] = '/' ;
         if ( file_name[0] == '/'  )
            file_name++ ;
      #else
         if ( buf[pos-1] != '\\' )
            buf[pos++] = '\\' ;
         if ( file_name[0] == '\\'  )
            file_name++ ;
      #endif
   }

   u4ncpy( buf+pos, file_name, buf_len-pos ) ;
}

void  S4FUNCTION u4name_piece( char *result, int len_result, char *from, int give_path, int give_ext )
{
   unsigned name_pos, ext_pos, on_pos, pos, new_len, from_len ;
   int are_past_ext ;

   name_pos = 0 ;
   are_past_ext = 0 ;
   ext_pos = from_len = strlen(from) ;
   if ( ext_pos == 0 )
   {
      *result = 0 ;
      return ;
   }

   for( on_pos = ext_pos-1;; on_pos-- )
   {
      switch ( from[on_pos] )
      {
         #ifdef S4UNIX
            case '/':
         #else
            case '\\':
         #endif
         case ':':
            if (name_pos == 0)  name_pos = on_pos + 1 ;
            are_past_ext = 1 ;
            break ;

         case '.':
            if ( ! are_past_ext )
            {
               ext_pos = on_pos ;
               are_past_ext = 1 ;
            }
            break ;
         default:
            break ;
      }

      if ( on_pos == 0 )
         break ;
   }

   pos = 0 ;
   new_len = from_len ;
   if ( !give_path )
   {
      pos = name_pos ;
      new_len -= name_pos ;
   }

   if ( !give_ext )
      new_len -= from_len - ext_pos ;

   #ifdef S4DEBUG
      if ( new_len >= (unsigned) len_result )
	 e4severe( e4result, E4_U4NAME_PIECE ) ;
   #endif

   memcpy( result, from+ pos, new_len ) ;
   result[new_len] = 0 ;

   #ifndef S4UNIX
      c4upper(result) ;
   #endif
}

/* u4name_char.c  Returns TRUE iff it is a valid dBase field or function name character */
int S4FUNCTION u4name_char( unsigned char ch)
{
   return ( ch>='a' && ch<='z'  || ch>='A' && ch<='Z'  ||
      #ifdef S4MDX
         ch>='0' && ch<='9'  || ch=='&' || ch=='@' ||
      #else
         ch>='0' && ch<='9'  ||
      #endif
         ch=='_'
/*            ch=='\\'  ||  ch=='.'  || ch=='_'  ||  ch==':' */
   #ifdef S4GERMAN
      #ifdef S4ANSI
         || ch== 196  ||  ch== 214  || ch== 220  ||  ch== 223
         || ch== 228  ||  ch== 246  || ch== 252
      #else
         || ch== 129  ||  ch== 132  || ch== 142  ||  ch== 148
         || ch== 153  ||  ch== 154  || ch== 225
      #endif
   #endif
   #ifdef S4FRENCH
      #ifdef S4ANSI
         || ch== 192 || ch== 194 || ch== 206 || ch== 207
         || ch== 212 || ch== 219 || ch== 224 || ch== 226
         || ch== 238 || ch== 239 || ch== 244 || ch== 251
         || (ch>= 199 && ch <= 203) || (ch >= 231 && ch <= 235)
      #else
         || ch== 128 || ch== 130 || ch== 131 || ch== 133
         || ch== 144 || ch== 147 || ch== 150 || (ch>= 135 && ch <= 140)
      #endif
   #endif
   #ifdef S4SWEDISH
      #ifdef S4ANSI
         || ch== 196 || ch== 197 || ch== 198 || ch== 201
         || ch== 214 || ch== 220 || ch== 228 || ch== 229
         || ch== 230 || ch== 233 || ch== 246 || ch== 252 )
      #else
         || ch== 129 || ch== 130 || ch== 132 || ch== 134
         || ch== 148 || ch== 153 || ch== 154 || (ch>= 142 && ch <= 146)
      #endif
   #endif
   #ifdef S4FINISH
      #ifdef S4ANSI
         || ch== 196 || ch== 197 || ch== 198 || ch== 201
         || ch== 214 || ch== 220 || ch== 228 || ch== 229
         || ch== 230 || ch== 233 || ch== 246 || ch== 252 )
      #else
         || ch== 129 || ch== 130 || ch== 132 || ch== 134
         || ch== 148 || ch== 153 || ch== 154 || (ch>= 142 && ch <= 146)
      #endif
   #endif
   #ifdef S4NORWEGIAN
      #ifdef S4ANSI
         || ch== 196 || ch== 197 || ch== 198 || ch== 201
         || ch== 214 || ch== 220 || ch== 228 || ch== 229
         || ch== 230 || ch== 233 || ch== 246 || ch== 252 )
      #else
         || ch== 129 || ch== 130 || ch== 132 || ch== 134
         || ch== 148 || ch== 153 || ch== 154 || (ch>= 142 && ch <= 146)
      #endif
   #endif
         ) ;
}

/* returns the length of the path in 'from', and copies the path in 'from' to result */
int S4FUNCTION u4name_path( char *result, int len_result, char *from )
{
   int on_pos ;

   u4name_piece( result, len_result, from, 1, 0 ) ;
   for( on_pos = 0 ; result[on_pos] != 0 ; on_pos++ ) ;

   for( ; on_pos >= 0 ; on_pos-- )
      #ifdef S4UNIX
         if( result[on_pos] == '/' || result[on_pos] == ':' ) break ;   /* end of path */
      #else
         if( result[on_pos] == '\\' || result[on_pos] == ':' ) break ;   /* end of path */
      #endif

   if( on_pos < len_result )
      result[++on_pos] = '\0' ;
   return on_pos ;
}

/* returns the length of the extension in 'from', and copies the extension in 'from' to 'result' */
int S4FUNCTION u4name_ret_ext( char *result, int len_result, char *from )
{
   char on_pos, len, name[13] ;

   #ifdef S4DEBUG
      if ( result == 0 || len_result < 3 || from == 0 )
	 e4severe( e4parm, E4_U4NAME_RET_EXT ) ;
   #endif

   u4name_piece( name, 13, from, 0, 1 ) ;

   len = 0 ;
   for( on_pos = 0 ; from[on_pos] != 0 ; on_pos++ )
      if ( from[on_pos] == '.' )
      {
         for ( on_pos++ ; from[on_pos] != 0 && len_result-- > 0 ; on_pos++, len++ )
            result[len] = from[on_pos] ;
         break ;
      }

   return len ;
}
