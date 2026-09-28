/* d4seek.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

/* function to ensure the integrity of the seeks return value */
/* in single user mode, exclusive, or if file is locked, the data is assumed up to date, and not checked */
static int d4seek_check( DATA4 *d4, TAG4 *tag, int rc, char *buf )
{
   int skipped, rc2 ;
   char *dbf_key ;

   #ifndef S4SINGLE
      if ( rc == r4locked )
         return r4locked ;
   #endif

   if ( t4eof( tag ) )
      return d4go_eof( d4 ) ;

   #ifndef S4SINGLE
      if ( d4->file.is_exclusive || d4lock_test_file( d4 ) )
      {
   #endif
      rc2 = d4go( d4, t4recno( tag ) ) ;
      if ( rc2 )
         return rc2 ;
      return rc ;
   #ifndef S4SINGLE
      }
      skipped = 0 ;
      for( ;; )
      {
         rc2 = d4go( d4, t4recno( tag ) ) ;
         if ( rc2 )
            return rc2 ;

         if ( t4expr_key( tag, &dbf_key ) < 0 )
            return -1 ;
         #ifdef S4FOX
            if ( !u4memcmp( t4key( tag ), dbf_key, expr4key_len( tag->expr ) ) )  /* matched */
         #else
            if ( !(*tag->cmp)( t4key( tag ), dbf_key, expr4key_len( tag->expr ) ) )  /* matched */
         #endif
         {
            if ( skipped )
            {
               #ifdef S4FOX
                  rc2 = u4memcmp( dbf_key, buf, expr4key_len( tag->expr ) ) ;
               #else
                  rc2 = (*tag->cmp)( dbf_key, buf, expr4key_len( tag->expr ) ) ;
               #endif
               if ( rc2 == 0 )   /* worked */
               return rc2 ;
               if ( rc2 > 0 )
                  return r4after ;
               /* other wise, if < 0, can't return r4after, so go down and skip next */
            }
            else
               return rc ;
         }

         /* try next record */
         if ( t4skip( tag, 1L ) == 0 )  /* eof */
            return d4go_eof( d4 ) ;
         if ( d4->code_base->error_code < 0 )
            return -1 ;
         skipped = 1 ;
      }
   #endif
}
int S4FUNCTION d4seek( DATA4 *d4, char *str )
{
   return d4seek_n( d4, str, strlen(str) ) ;
}

int S4FUNCTION d4seek_n( DATA4 *d4, char *str, int len )
{
   TAG4 *tag ;
   int rc ;
   char buf[I4MAX_KEY_SIZE] ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4SEEK ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 || str == 0 )
         e4severe( e4parm, E4_D4SEEK ) ;
   #endif

   if ( d4->code_base->error_code < 0 )
      return -1 ;

   tag = d4tag_default( d4 ) ;
   if ( tag == 0 )
      return r4no_tag ;

   #ifndef S4OFF_WRITE
      rc = d4update_record( d4, 1 ) ;
      if ( rc )
         return rc ;
   #endif

   #ifdef S4UNIX
      #ifdef S4MDX
         switch ( tag->key_type )
         {
            case r4num:
               c4bcd_from_a( buf, str, len ) ;
               break ;
            case r4date:
               t4str_to_date_mdx( buf, str, len ) ;
               break ;
            case r4str:
               t4no_change_str( buf, str, len ) ;
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
      #ifdef S4FOX
         switch ( tag->key_type )
         {
            case r4num:
            case r4num_doub:
               t4str_to_fox( buf, str, len ) ;
               break ;
            case r4date:
            case r4date_doub:
               t4dtstr_to_fox( buf, str, len ) ;
               break ;
            case r4str:
               t4no_change_str( buf, str, len ) ;
               break ;
            case r4log:
               t4str_to_log( buf, str, len ) ;
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
      #ifdef S4CLIPPER
         switch ( tag->key_type )
         {
            case r4num:
            case r4num_doub:
               t4str_to_clip( buf, str, len ) ;
               break ;
            case r4date:
            case r4date_doub:
            case r4str:
               t4no_change_str( buf, str, len ) ;
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
      #ifdef S4NDX
         switch ( tag->key_type )
         {
            case r4num:
            case r4num_doub:
               t4str_to_doub( buf, str, len ) ;
               break ;
            case r4date:
            case r4date_doub:
               t4str_to_date_mdx( buf, str, len ) ;
               break ;
            case r4str:
               t4no_change_str( buf, str, len ) ;
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
   #else
      (*tag->stok)( buf, str, len ) ;
   #endif

   if ( t4type( tag ) != r4str )
      len = tag->header.key_len ;
   else
      if ( len <= 0 )  len = strlen( str ) ;

   t4version_check( tag, 0 ) ;
   rc = t4seek( tag, buf, len ) ;

   return d4seek_check( d4, tag, rc, buf ) ;  /* return a valid value */
}

int S4FUNCTION d4seek_double( DATA4 *d4, double dkey )
{
   TAG4 *tag ;
   int    rc ;
   char buf[I4MAX_KEY_SIZE] ;
   #ifdef S4CLIPPER
      int len ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_D4SEEK_DBL ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_D4SEEK_DBL ) ;
   #endif

   if ( d4->code_base->error_code < 0 )
      return -1 ;

   tag = d4tag_default( d4 ) ;
   if ( tag == 0 )
      return r4no_tag ;

   #ifndef S4OFF_WRITE
      rc = d4update_record( d4, 1 ) ;
      if ( rc )
         return rc ;
   #endif

   #ifdef S4CLIPPER
      if ( tag->dtok == 0 )
      {
         len = tag->header.key_len ;
         c4dtoa45( dkey, buf, len, tag->header.key_dec ) ;
         if ( buf[0] == '*' )  return -1 ;  /* unknown overflow result */
         c4clip( buf, len ) ;
      }
      else
   #else
      #ifdef S4DEBUG
         if ( tag->dtok == 0 )
            e4severe( e4info, E4_INFO_WT4 ) ;
      #endif
   #endif

   #ifdef S4UNIX
      #ifdef S4MDX
         switch ( tag->key_type )
         {
            case r4num:
               c4bcd_from_d( buf, dkey ) ;
               break ;
            case r4date:
               t4no_change_double( buf, dkey ) ;
               break ;
            case r4str:
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
      #ifdef S4FOX
         switch ( tag->key_type )
         {
            case r4num:
            case r4num_doub:
            case r4date:
            case r4date_doub:
               t4dbl_to_fox( buf, dkey ) ;
               break ;
            case r4str:
            case r4log:
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
      #ifdef S4CLIPPER
         switch ( tag->key_type )
         {
            case r4num:
            case r4num_doub:
            case r4str:
               break ;
            case r4date:
            case r4date_doub:
               t4date_doub_to_str( buf, dkey ) ;
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
      #ifdef S4NDX
         switch ( tag->key_type )
         {
            case r4num:
            case r4num_doub:
            case r4date:
            case r4date_doub:
               t4no_change_double( buf, dkey ) ;
               break ;
            case r4str:
               break ;
            default:
               e4severe( e4info, E4_INFO_INV ) ;
         }
      #endif
   #else
      (*tag->dtok)( buf, dkey ) ;
   #endif

   t4version_check( tag, 0 ) ;
   rc = t4seek( tag, buf, tag->header.key_len ) ;

   return d4seek_check( d4, tag, rc, buf ) ;  /* return a valid value */
}

#endif  /* S4INDEX_OFF */

#ifdef S4VB_DOS

int d4seek_v( DATA4 *d4, char *seek )
{
   return d4seek( d4, c4str(seek) ) ;
}

#endif
