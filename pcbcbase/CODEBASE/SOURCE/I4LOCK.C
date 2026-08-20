/* i4lock.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF

#ifdef N4OTHER
   #include <time.h>
#endif

#ifndef N4OTHER

int S4FUNCTION i4lock( INDEX4 *i4 )
{
   #ifndef S4SINGLE
      int rc ;

      #ifdef S4VBASIC
         if ( c4parm_check( i4, 0, E4_I4LOCK ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( i4 == 0 )
            e4severe( e4parm, E4_I4LOCK ) ;
      #endif

      if ( i4->code_base->error_code < 0 )
         return -1 ;

      if ( i4->file_locked )    /* Already locked */
         return 0 ;

      #ifdef S4MDX
         rc = file4lock( &i4->file, L4LOCK_POS - 1L, 1L ) ;
      #endif
      #ifdef S4FOX
         rc = file4lock( &i4->file, L4LOCK_POS, 1L ) ;
      #endif

      if ( rc )
         return rc ;

      #ifndef S4OPTIMIZE_OFF
         file4refresh( &i4->file ) ;   /* make sure all up to date */
      #endif


      if ( file4len( &i4->file ) != 0 )  /* if the file exists (not being created), update header */
         if ( i4version_check( i4, 1 ) < 0 )
         {
            #ifdef S4MDX
               file4unlock( &i4->file, L4LOCK_POS - 1L, 1L ) ;
            #endif
            #ifdef S4FOX
               file4unlock( &i4->file, L4LOCK_POS, 1L ) ;
            #endif
            return -1 ;
         }

      #ifdef S4FOX
         i4->eof = file4len( &i4->file ) ;
      #endif
      i4->file_locked = 1 ;  /* this flag must be set after the version_check is complete */

   #endif

   return 0 ;
}

int S4FUNCTION i4unlock( INDEX4 *i4 )
{
   #ifndef S4SINGLE
      #ifdef S4VBASIC
         if ( c4parm_check( i4, 0, E4_I4UNLOCK ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( i4 == 0 )
            e4severe( e4parm, E4_I4UNLOCK ) ;
      #endif

      if ( i4->file_locked )
      {
         #ifndef S4OFF_WRITE
            if ( i4update( i4 ) < 0 )
               return -1 ;
         #endif

         #ifdef S4MDX
            file4unlock( &i4->file, L4LOCK_POS - 1L, 1L ) ;
         #endif
         #ifdef S4FOX
            file4unlock( &i4->file, L4LOCK_POS, 1L ) ;
         #endif
         i4->file_locked = 0 ;
         return 0 ;
      }
   #endif
   return 0 ;
}

#endif  /*  ifndef  N4OTHER  */

#ifdef N4OTHER

int S4FUNCTION i4lock( INDEX4 *i4 )
{
   #ifndef S4SINGLE
      TAG4 *tag_on ;
      int rc, old_attempts, count ;

      #ifdef S4VBASIC
         if ( c4parm_check( i4, 0, E4_I4LOCK ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( i4 == 0 )
            e4severe( e4parm, E4_I4LOCK ) ;
      #endif

      if ( i4->code_base->error_code < 0 )
         return -1 ;

      if ( i4->file_locked )    /* Already locked */
         return 0 ;

      count = old_attempts = i4->code_base->lock_attempts ;  /* take care of wait here */
      i4->code_base->lock_attempts = 1 ;

      for(;;)
      {
         rc = 0 ;

         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on) ;
            if ( tag_on == 0 )
               break ;
            if ( tag_on->file_locked == 0 )
            {
               rc = file4lock( &tag_on->file, L4LOCK_POS, L4LOCK_POS ) ;
               if ( rc )
                  break ;
               tag_on->file_locked = 1 ;
            }
         }

         if ( rc != r4locked )
            break ;

         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on) ;
            if ( tag_on == 0 )
               break ;
            if ( tag_on->file_locked == 1 )
            {
               file4unlock( &tag_on->file, L4LOCK_POS, L4LOCK_POS ) ;
               tag_on->file_locked = 0 ;
            }
         }

         if ( count == 0 )
            break ;

         if ( count > 0 )
             count-- ;
         #ifdef S4TEMP
            if ( d4display_quit( &display ) )
               e4severe( e4result, E4_RESULT_EXI ) ;
         #endif

         u4delay_sec() ;   /* wait a second */
      }

      if ( rc == 0 )
         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
            if ( tag_on == 0 || rc )
               break ;
            if ( file4len( &tag_on->file ) != 0 )  /* if the file exists (not being created), update header */
               rc = t4do_version_check( tag_on, 0 ) ;
         }

      if ( rc == 0 )
      {
         i4->file_locked = 1 ;
         #ifndef S4OPTIMIZE_OFF
            for( tag_on = 0 ;; )
            {
               tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
               if ( tag_on == 0 )
                  break ;
               file4refresh( &tag_on->file ) ;   /* make sure all up to date */
            }
         #endif
      }
      else
         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            if ( tag_on->file_locked )
               if ( file4unlock( &tag_on->file, L4LOCK_POS, L4LOCK_POS ) == 0 )
                  tag_on->file_locked = 0 ;
         }

      i4->code_base->lock_attempts = old_attempts ;
      if ( i4->code_base->error_code < 0 )
         return -1 ;

      return rc ;
   #else
      return 0 ;
   #endif
}

/* unlocks all the corresponding tag files */
int S4FUNCTION i4unlock( INDEX4 *i4 )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      int rc, save_rc = 0 ;
      TAG4 *tag_on ;

      #ifdef S4VBASIC
         if ( c4parm_check( i4, 0, E4_I4UNLOCK ) )
            return -1 ;
      #endif

      #ifdef S4DEBUG
         if ( i4 == 0 )
            e4severe( e4parm, E4_I4UNLOCK ) ;
      #endif

      if ( i4->file_locked )
      {
         #ifndef S4OFF_WRITE
            if ( i4update( i4 ) < 0 )
               return -1 ;
         #endif

         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            rc = 0 ;
            if ( tag_on->file_locked == 1 )
               rc = file4unlock( &tag_on->file, L4LOCK_POS, L4LOCK_POS ) ;
            if ( rc )
               save_rc = rc ;
            else
               tag_on->file_locked = 0 ;
         }

         if ( save_rc == 0 )
            i4->file_locked = 0 ;
      }

      #ifdef S4DEBUG
         /* all tags should be unlocked in index file, do check */
         for( tag_on = 0 ;; )
         {
            tag_on = (TAG4 *)l4next( &i4->tags, tag_on) ;
            if ( tag_on == 0 )
               break ;
            if ( tag_on->file_locked )
            {
               e4describe( i4->code_base, e4result, E4_INFO_UNE, i4->file.name, tag_on->alias ) ;
               e4severe( e4result, tag_on->alias ) ;
            }
         }
      #endif

      return save_rc ;
   #endif
}

#endif  /* N4OTHER */

#endif  /* S4INDEX_OFF */
