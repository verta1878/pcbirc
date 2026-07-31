/* d4index.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

int S4FUNCTION d4free_blocks( DATA4 *data )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      int rc ;
      TAG4 *tag_on ;

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4FREE_BLOCKS ) ;
      #endif

      rc = 0 ;
      for( tag_on = 0 ;; )
      {
         tag_on = (TAG4 *)d4tag_next( data, tag_on ) ;
         if ( tag_on == 0 )
            return rc ;
         if ( t4free_all( tag_on ) < 0 )
            rc = -1 ;
      }
   #endif
}

INDEX4 *S4FUNCTION d4index( DATA4 *data, char *index_name )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      char index_lookup[258], current[258] ;
      INDEX4 *index_on ;

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E4_D4INDEX ) )
            return 0 ;
      #endif  /* S4VBASIC */

      #ifdef S4DEBUG
         if ( data == 0 || index_name == 0 )
            e4severe( e4parm, E4_D4INDEX ) ;
      #endif

      u4name_piece( index_lookup, sizeof(index_lookup), index_name, 1, 0 ) ;
      c4upper( index_lookup ) ;

      for( index_on = 0 ;; )
      {
         index_on = (INDEX4 *) l4next( &data->indexes, index_on) ;
         if ( index_on == 0 )
            return 0 ;
         #ifdef N4OTHER
            u4name_piece( current, sizeof(current), index_on->alias, 1, 0 ) ;
         #else
            u4name_piece( current, sizeof(current), index_on->file.name, 1, 0 ) ;
         #endif  /* N4OTHER */
         c4upper( current ) ;
         if ( !strcmp( current, index_lookup ) )    /* check out data->alias? */
            return index_on ;
      }
   #endif
}

#ifndef S4OFF_WRITE
int S4FUNCTION d4reindex( DATA4 *data )
{
   #ifdef S4INDEX_OFF
      return 0 ;
   #else
      INDEX4 *index_on ;
      int rc ;
      #ifndef S4OPTIMIZE_OFF
         int has_opt ;
      #endif

      #ifdef S4VBASIC
         if ( c4parm_check( data, 2, E4_D4REINDEX ) )
            return -1 ;
      #endif  /* S4VBASIC */

      #ifdef S4DEBUG
         if ( data == 0 )
            e4severe( e4parm, E4_D4REINDEX ) ;
      #endif

      #ifdef S4SINGLE
         if ( data->code_base->error_code < 0 )
            return -1 ;
         rc = 0 ;
      #else
         rc = d4lock_all( data ) ;  /* updates the record, returns -1 if code_base->error_code < 0 */
         if ( rc )
            return rc ;
      #endif  /* S4SINGLE */

      #ifndef S4OPTIMIZE_OFF
         has_opt = data->code_base->has_opt && data->code_base->opt.num_buffers ;
         if ( has_opt )
            d4opt_suspend( data->code_base ) ;
      #endif  /* not S4OPTIMIZE_OFF */

      for ( index_on = 0 ;; )
      {
         index_on = (INDEX4 *)l4next( &data->indexes, index_on ) ;
         if ( index_on == 0 )
            break ;
         if ( i4reindex( index_on ) < 0 )
         {
            rc = -1 ;
            break ;
         }
      }

      #ifndef S4OPTIMIZE_OFF
         if ( has_opt )
            d4opt_start( data->code_base ) ;
      #endif  /* not S4OPTIMIZE_OFF */
      return rc ;
   #endif
}
#endif  /* S4OFF_WRITE */

#ifdef S4VB_DOS

INDEX4 *d4index_v( DATA4 *d4, char *index_name )
{
   return d4index( d4, c4str(index_name) ) ;
}

#endif
