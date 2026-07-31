/* m4check.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"

/* not supported for FoxPro FPT memo files */
#ifndef S4MFOX
#ifndef S4MNDX

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4MEMO_OFF
int S4FUNCTION f4memo_check( MEMO4FILE *f4memo )
{
   F4FLAG flags ;
   DATA4 *d4 ;
   int  rc, i_field ;
   long num_blocks, memo_id ;
   MEMO4CHAIN_ENTRY cur ;

   d4 = f4memo->data ;
   rc = d4lock_file( d4 ) ;
   if ( rc != 0 )
      return rc ;

   if ( ( rc = d4update( d4 ) ) != 0 )
      return rc ;

   if ( f4flag_init( &flags, d4->code_base, file4len( &f4memo->file ) / f4memo->block_size ) < 0 )
      return e4memory ;

   /* Set flags for the data file entries */
   for ( d4top( d4 ) ; ! d4eof( d4 ) ; d4skip( d4, 1L ) )
   {
      for ( i_field = 0 ; i_field < d4->n_fields_memo ; i_field++ )
      {
         num_blocks = (f4memo_len(d4->fields_memo[i_field].field) + f4memo->block_size - 1) / f4memo->block_size ;
         memo_id = f4long( d4->fields_memo[i_field].field ) ;

         if ( f4flag_is_any_set( &flags, memo_id, num_blocks ) )
            return e4describe( d4->code_base, e4info, E4_F4MEMO_CHECK, E4_INFO_AME, (char *) 0 ) ;
         if ( f4flag_set_range( &flags, memo_id, num_blocks ) < 0 )
            return -1 ;
      }
   }

   /* Set flags for the free chain */
   memset( (void *)&cur, 0, sizeof(cur) ) ;
   memo4file_chain_skip( f4memo, &cur ) ;  /* Read in root */

   for ( memo4file_chain_skip( f4memo, &cur ) ; cur.next != -1 ; memo4file_chain_skip( f4memo, &cur ) )
   {
      rc = f4flag_is_any_set( &flags, cur.block_no, cur.num) ;
      if ( rc < 0 )
         return -1 ;
      if ( rc )
         return e4describe( d4->code_base, e4info, E4_F4MEMO_CHECK, E4_INFO_AME, (char *) 0 ) ;
      if ( f4flag_set_range( &flags, cur.block_no, cur.num ) < 0 )
         return -1 ;
   }

   rc = f4flag_is_all_set( &flags, 1, file4len( &f4memo->file ) / f4memo->block_size-1 ) ;
   u4free( flags.flags ) ;
   if ( rc == 0 )
      return e4describe( d4->code_base, e4result, E4_F4MEMO_CHECK, E4_RESULT_WAS, (char *) 0 ) ;
   if ( rc < 0 )
      return -1 ;

   return 0 ;
}

#endif  /*   ifndef S4MFOX   */
#endif  /*   ifndef S4MNDX   */

#endif /*  ifndef S4MEMO_OFF */
