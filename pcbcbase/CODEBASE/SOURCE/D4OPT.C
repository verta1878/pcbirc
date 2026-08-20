/* d4opt.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

static void opt4free_alloc( OPT4 * ) ;
static int opt4init_alloc( CODE4 *, int ) ;

int S4FUNCTION d4opt_start( CODE4 *code_base )
{
   #ifndef S4OPTIMIZE_OFF
      OPT4 *opt ;
      unsigned num_alloc, num_buffers ;
      FILE4 *file_on ;

      #ifdef S4DEBUG
         if ( code_base == 0 )
            e4severe( e4parm, E4_D4OPT_START ) ;
         if ( code_base->debug_int != 0x5281 )
            e4severe( e4result, E4_RESULT_D4I ) ;
      #endif

      if( code_base->error_code < 0 )
         return -1 ;

      opt = &code_base->opt ;

      if ( opt->num_buffers || code_base->has_opt )  /* no initialization required */
         return 0 ;

      if ( code_base->mem_size_block == 0 || code_base->mem_size_buffer == 0 ||
           code_base->mem_size_block > code_base->mem_size_buffer )
      {
         #ifdef S4DEBUG
            e4( code_base, e4parm, E4_PARM_OPT ) ;
         #endif
         return -1 ;
      }

      opt->block_size = code_base->mem_size_block ;
      if ( code_base->mem_size_block != 0 )
         opt->buffer_size = code_base->mem_size_block * ( code_base->mem_size_buffer / code_base->mem_size_block ) ;
      else
         opt->buffer_size = 0 ;
      opt->hash_trail = 0 ;
      opt->prio[0] = &opt->dbf_lru ;
      opt->prio[1] = &opt->index_lru ;
      opt->prio[2] = &opt->other_lru ;
      opt->do_update = 1 ;
      code_base->mode = OPT4START_MODE ;
      opt->check_count = OPT4CHECK_RATE ;   /* set to do analysis when first block removed */

      for( ; ( (num_buffers = (int)(code_base->mem_start_max / opt->buffer_size - 2 )) < 4) ; ) 
      {
         opt->buffer_size -= opt->block_size ;
         if ( opt->buffer_size == 0 )
            return -1 ;
      }

      for(;;)
      {
         code_base->has_opt = 1 ;
         opt->min_link = opt->max_blocks = (int)( opt->buffer_size / opt->block_size ) ;
         opt->block_power = (char) c4calc_type( opt->block_size ) ;
         opt->num_shift = (char) (8*sizeof( long ) - (opt->block_power)) ;
         opt->num_lists = OPT4BLOCK_DENSITY << c4calc_type( (long)num_buffers * opt->max_blocks ) ;

         num_alloc = opt4init_alloc( code_base, num_buffers ) ;

         if ( num_alloc == 0 )
         {
            d4opt_suspend( code_base ) ;
            return -1 ;
         }

         opt->num_buffers = num_alloc ;

         if ( num_alloc < 4 )   /* couldn't do a minimum allocation, try again */
         {
            opt4free_alloc( opt ) ;   /* free allocs */
            if ( num_buffers > 4 )
               num_buffers = 4 ; 
            opt->buffer_size /= 2 ;
            opt->buffer_size = opt->buffer_size - opt->buffer_size % opt->block_size ;  /* round it down to a block_size multiple */
            if ( opt->buffer_size == 0 )
               return -1 ;
            continue ;
         }
         break ;
      }

      opt->num_blocks = (long)opt->num_buffers * opt->max_blocks ;
      opt->num_lists = OPT4BLOCK_DENSITY << c4calc_type( opt->num_blocks ) ;
      opt->mask = opt->num_lists - 1 ;

      /* now actually optimize those files reqd */
      for ( file_on = 0 ;; )
      {
         file_on = (FILE4 *)l4next( &opt->opt_files, file_on ) ;
         if ( file_on == 0 )
            break ;
         file_on->len = -1 ;   /* in case the file length changed during suspension */
         file_on->do_buffer = 1 ;   /* re-add the reference */
         if ( file_on->hash_init == -1 )
         {
            file_on->hash_init = opt->hash_trail * opt->block_size ;
            #ifdef S4DEBUG
               if ( file4len( file_on ) < 0 || opt->block_size == 0 )
                  e4severe( e4info, E4_D4OPT_START ) ;
            #endif
            opt->hash_trail = (opt->hash_trail + file4len( file_on ) / opt->block_size) % opt->num_blocks ;
         }
      }
   #endif
   return 0 ;
}

int S4FUNCTION d4opt_suspend( CODE4 *code_base )
{
   #ifndef S4OPTIMIZE_OFF
      OPT4 *opt ;
      FILE4 *file_on ;
      int rc ;

      #ifdef S4DEBUG
         if ( code_base == 0 )
            e4severe( e4parm, E4_D4OPT_SUSPEND ) ;
      #endif

      opt = &code_base->opt ;
      if ( opt->num_buffers == 0 || code_base->has_opt == 0 )
         return 0 ;

      rc = 0 ;

      /* first remove any optimized files */
      for ( file_on = 0 ;; )
      {
         file_on = (FILE4 *)l4next( &opt->opt_files, file_on ) ;
         if ( file_on == 0 )
            break ;
         if ( ( rc = file4low_flush( file_on, 1 ) ) < 0 )
            rc = -1 ;
         file_on->do_buffer = 0 ;  /* remove the reference */
      }


      code_base->has_opt = 0 ;

      opt4free_alloc( opt ) ;

      opt->num_buffers = 0 ;  /* mark as freed */

      if ( rc < 0 )
         return e4( code_base, e4opt_suspend, 0 ) ;
      else
   #endif
   return 0 ;
}

int S4FUNCTION d4optimize( DATA4 *d4, int opt_flag )
{
   #ifndef S4OPTIMIZE_OFF
      #ifdef N4OTHER
         TAG4 *tag_on ;
      #else
         INDEX4 *index_on ;
      #endif

      #ifdef S4DEBUG
         if ( d4 == 0 || opt_flag < -1 || opt_flag > 1 )
            e4severe( e4parm, E4_D4OPTIMIZE ) ;
      #endif

      if ( file4optimize( &d4->file, opt_flag, OPT4DBF ) < 0 )
         return -1 ;

      #ifdef N4OTHER
         for( tag_on = 0;;)
         {
            tag_on = d4tag_next( d4, tag_on ) ;
            if ( tag_on == 0 )
               break ;

            if ( file4optimize( &tag_on->file, opt_flag, OPT4INDEX ) < 0 )
               return -1 ;
         }
      #else
         for ( index_on = 0 ;; )
         {
            index_on = (INDEX4 *)l4next( &d4->indexes, index_on ) ;
            if ( index_on == 0 )
               break ;
            if ( file4optimize( &index_on->file, opt_flag, OPT4INDEX ) < 0 )
               return -1 ;
         }
      #endif

      if ( d4->n_fields_memo > 0 && d4->memo_file.file.hand != -1 )
         return file4optimize( &d4->memo_file.file, opt_flag, OPT4OTHER ) ;
      else
         return 0 ;
   #else
      return 0 ;
   #endif
}

int S4FUNCTION d4optimize_write( DATA4 *d4, int opt_flag )
{
   #ifndef S4OFF_WRITE
      #ifndef S4OPTIMIZE_OFF
         #ifndef N4OTHER
            INDEX4 *index_on ;
         #else
            TAG4 *tag_on ;
         #endif

         #ifdef S4DEBUG
            if ( d4 == 0 || opt_flag < -1 || opt_flag > 1 )
               e4severe( e4parm, E4_D4OPT_WRITE ) ;
         #endif

         if ( file4optimize_write( &d4->file, opt_flag ) < 0 )
            return -1 ;

         if ( d4->n_fields_memo > 0 && d4->memo_file.file.hand != -1 )
            if ( file4optimize_write( &d4->memo_file.file, opt_flag ) < 0 )
               return -1 ;

         #ifndef N4OTHER
            index_on = (INDEX4 *) l4first( &d4->indexes ) ;
            if ( index_on != 0 )
               do
               {
                  if ( file4optimize_write( &index_on->file, opt_flag ) < 0 )
                     return -1 ;
                  index_on = (INDEX4 *)l4next( &d4->indexes, index_on ) ;
               } while ( index_on != 0 ) ;
         #else
            for( tag_on = 0;;)
            {
               tag_on = d4tag_next( d4, tag_on ) ;
               if ( tag_on == 0 )
                  break ;
               if ( file4optimize_write( &tag_on->file, opt_flag ) < 0 )
                  return -1 ;
            }
         #endif
      #endif
   #endif
   return 0 ;
}

#ifndef S4OPTIMIZE_OFF
static void opt4free_alloc( OPT4 *opt )
{
   OPT4BLOCK *cur_block ;
   int i ;

   #ifdef S4DEBUG
      if ( opt == 0 )
         e4severe( e4parm, E4_OPT4FREE ) ;
   #endif

   if ( opt->buffers )
   {
      for ( --opt->num_buffers; opt->num_buffers >= 0 ; opt->num_buffers-- )
      {
         for ( i = 0 ; i < (int)opt->max_blocks ; i++ )
         {
            cur_block = &opt->blocks[ opt->num_buffers * opt->max_blocks + i] ;
            l4remove( &opt->avail, &cur_block->lru_link ) ;
         }
         if ( opt->buffers[opt->num_buffers] != 0 )
            u4free( opt->buffers[opt->num_buffers] ) ;
      }
      u4free( opt->buffers ) ;
      opt->buffers = 0 ;
   }

   u4free( opt->write_buffer ) ;
   opt->write_buffer = 0 ;
   u4free( opt->read_buffer ) ;
   opt->read_buffer = 0 ;
   u4free( opt->blocks ) ;
   opt->blocks = 0 ;
   u4free( opt->lists ) ;
   opt->lists = 0 ;
   u4free( opt->lists ) ;
   opt->lists = 0 ;
}

static int opt4init_alloc( CODE4 *code_base, int num_buffers )
{
   OPT4BLOCK *cur_block ;
   OPT4 *opt ;
   int num_alloc, i ;

   #ifdef S4DEBUG
      if ( code_base == 0 )
         e4severe( e4parm, E4_OPT4INIT ) ;
   #endif

   opt = &code_base->opt ;

   opt->buffers = (void **)u4alloc( ( (long)num_buffers + 2 ) * sizeof( void * ) ) ;
   if ( opt->buffers == 0 )
   {
      d4opt_suspend( code_base ) ;
      return -1 ;
   }

   opt->lists = (LIST4 *)u4alloc( opt->num_lists * sizeof (LIST4) ) ;
   if ( opt->lists == 0 )
   {
      d4opt_suspend( code_base ) ;
      return -1 ;
   }

   opt->blocks = (OPT4BLOCK *)u4alloc( (long)num_buffers * opt->max_blocks * sizeof( OPT4BLOCK ) ) ;
   if ( opt->blocks == 0 )
   {
      d4opt_suspend( code_base ) ;
      return -1 ;
   }

   opt->write_buffer = (char *)u4alloc( opt->buffer_size ) ;
   if( opt->write_buffer == 0 )
      return 0 ;

   opt->write_block_count = 0 ;
   opt->write_cur_pos = 0 ;
   opt->write_start_pos = 0 ;
   opt->write_file = 0 ;

   opt->read_buffer = (char *)u4alloc( opt->buffer_size ) ;
   if( opt->read_buffer == 0 )
      return 0 ;

   for ( num_alloc = 0 ; num_alloc < num_buffers ; num_alloc++ )
   {
      opt->buffers[num_alloc] = u4alloc( opt->buffer_size ) ;
      if( opt->buffers[num_alloc] == 0 )
         break ;
      for ( i = 0 ; i < (int)opt->max_blocks ; i++ )
      {
         cur_block = &opt->blocks[ num_alloc * opt->max_blocks + i] ;
         cur_block->data = (OPT4BLOCK *)( (char *)opt->buffers[num_alloc] + i * opt->block_size ) ;
         l4add( &opt->avail, &cur_block->lru_link ) ;
      }
   }

   return num_alloc ;
}

#endif
