/* i4index.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF
#ifndef N4OTHER

int S4FUNCTION i4close( INDEX4 *i4 )
{
   int final_rc, save_attempts ;
   CODE4 *c4 ;
   TAG4 *tag_on ;

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4CLOSE ) )
         return -1 ;
   #endif

   if ( i4 == 0 )
      return -1 ;

   c4 = i4->code_base ;

   final_rc = c4->error_code ;
   #ifndef S4SINGLE
      save_attempts = c4->lock_attempts ;
      c4->lock_attempts = -1 ;
   #endif

   #ifndef S4OFF_WRITE
      if ( i4->data )
         if ( d4update( i4->data ) < 0 )
            final_rc = e4set( c4, 0 ) ;
   #endif

   #ifndef S4SINGLE
      if ( i4unlock( i4 ) < 0 )
         final_rc = e4set( c4, 0 ) ;
   #endif

   #ifdef S4FOX
      if ( i4->tag_index )
         if ( i4->tag_index->header.type_code >= 64 )  /* compound index */
   #endif

   for( ;; )
   {
      tag_on = (TAG4 *)l4pop( &i4->tags ) ;
      if ( tag_on == 0 )
         break ;
      if ( t4free_all( tag_on ) < 0 )
      {
         final_rc = e4set( c4, 0 ) ;
         break ;
      }
      expr4free( tag_on->expr ) ;
      expr4free( tag_on->filter ) ;
      mem4free( c4->tag_memory, tag_on ) ;
   }

   #ifdef S4FOX
      if ( t4free_all( i4->tag_index ) < 0 )
         final_rc = e4set( c4, 0 ) ;
      else
      {
         expr4free( i4->tag_index->expr ) ;
         expr4free( i4->tag_index->filter ) ;
         mem4free( c4->tag_memory, i4->tag_index ) ;
      }
   #endif

   mem4release( i4->block_memory ) ;

   if ( file4open_test( &i4->file ) )
   {
      if ( i4->data )
         l4remove( &i4->data->indexes, i4 ) ;
      if ( file4close( &i4->file ) < 0 )
         final_rc = e4set( c4, 0 ) ;
   }

   mem4free( c4->index_memory, i4 ) ;
   #ifndef S4SINGLE
      c4->lock_attempts = save_attempts ;
   #endif
   e4set( c4, final_rc ) ;
   return final_rc ;
}

#ifndef S4OFF_WRITE
long S4FUNCTION i4extend( INDEX4 *i4 )
{
   long old_eof ;
   unsigned len ;

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4EXTEND ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4FOX
      old_eof = i4->header.free_list ;

      if( i4->header.free_list == 0L )  /* case where no free list */
      {
         old_eof = i4->header.eof ;
         i4->header.eof = i4->header.eof + i4->header.block_rw/I4MULTIPLY ;
      }
      else
      {
         len = file4read( &i4->file, i4->header.free_list*I4MULTIPLY + sizeof(long),
                 (char *)&i4->header.free_list, sizeof(i4->header.free_list)) ;

         #ifdef S4BYTE_SWAP
            i4->header.free_list = x4reverse_long( i4->header.free_list ) ;
         #endif

         if ( i4->code_base->error_code < 0 )  return -1 ;

         switch( len )
         {
            case 0:
               #ifdef S4DEBUG
                  e4severe( e4info, E4_I4EXTEND ) ;
               #endif

               /* else fix up */
               i4->header.free_list = 0L ;
               old_eof = i4->header.eof ;
               i4->header.eof = i4->header.eof + i4->header.block_rw/I4MULTIPLY ;
               break ;

            case sizeof(i4->header.free_list):
               break ;

            default:
               return file4read_error( &i4->file ) ;
         }
      }
   #endif

   #ifdef S4FOX
      #ifdef S4DEBUG
         if ( i4->tag_index->header.version == i4->version_old )
            e4severe( e4info, E4_I4EXTEND ) ;
      #endif

      old_eof = i4->tag_index->header.free_list ;

      if( old_eof == 0L )  /* case where no free list */
      {
         old_eof = i4->eof ;
         i4->eof += B4BLOCK_SIZE ;
      }
      else
      {
         len = file4read( &i4->file, i4->tag_index->header.free_list*I4MULTIPLY,
                 (char *)&i4->tag_index->header.free_list, sizeof(i4->tag_index->header.free_list)) ;

         #ifdef S4BYTE_SWAP
            i4->tag_index->header.free_list = x4reverse_long( i4->tag_index->header.free_list ) ;
         #endif

         if ( i4->code_base->error_code < 0 )  return -1 ;

         switch( len )
         {
            case 0:
               #ifdef S4DEBUG
                  e4severe( e4info, E4_I4EXTEND ) ;
               #endif

               /* else fix up */
               i4->tag_index->header.free_list = 0L ;
               old_eof = i4->eof ;
               i4->eof += B4BLOCK_SIZE ;
               break ;

            case sizeof(i4->tag_index->header.free_list):
               break ;

            default:
               return file4read_error( &i4->file ) ;
         }
      }
   #endif

   return old_eof ;
}

int S4FUNCTION i4flush( INDEX4 *i4 )
{
   int rc ;

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4FLUSH ) ;
   #endif

   rc = i4update( i4 ) ;
   #ifndef S4OPTIMIZE_OFF
      if ( rc )
         rc = file4flush( &i4->file ) ;
   #endif

   return rc ;
}

int S4FUNCTION i4update( INDEX4 *i4 )
{
   TAG4 *tag_on ;

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4UPDATE ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4SINGLE
      if ( i4->file_locked )
      {
   #endif

   if ( i4update_header( i4 ) < 0 )
      return -1 ;
   #ifdef S4FOX
      if ( t4update(i4->tag_index) < 0 )
         return -1 ;
      if ( i4->tag_index->header.type_code >= 64 )  /* compound index */
   #endif

   for ( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on == 0 )
         break ;
      if ( t4update( tag_on ) < 0 )
         return -1 ;
      tag_on->header.root = -1L ;
   }

   #ifndef S4SINGLE
      }
   #endif

   return 0 ;
}
#endif

INDEX4 *S4FUNCTION i4open( DATA4 *d4, char *file_name )
{
   INDEX4 *i4 ;
   CODE4  *c4 ;
   char   buf[258] ;
   int    rc ;
   TAG4 *tag_ptr ;
   #ifdef S4FOX
      B4BLOCK *b4 ;
      int old_file_lock ;
   #else
      T4DESC tag_info[47] ;
      int i_tag ;
   #endif

   #ifdef S4BYTE_SWAP
      T4DESC swap_tag ;
   #endif

   #ifdef S4DEBUG
      INDEX4 *i4_ptr ;
      if ( d4 == 0 )
         e4severe( e4parm, E4_I4OPEN ) ;

      if ( file_name )
         u4name_piece( buf, sizeof( buf ), file_name, 0, 0 ) ;
      else
         u4name_piece( buf, sizeof( buf ), d4->file.name, 0, 0 ) ;
      if ( d4index( d4, buf ) )
      {
         e4( d4->code_base, e4info, E4_INFO_IAO ) ;
         return 0 ;
      }
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_I4OPEN ) )
         return 0 ;
   #endif

   c4 = d4->code_base ;
   if ( c4->error_code < 0 )
      return 0 ;

   if ( c4->index_memory == 0 )
      c4->index_memory = mem4create( c4, c4->mem_start_index, sizeof(INDEX4),
                                      c4->mem_expand_index, 0 ) ;

   if ( c4->index_memory == 0 )
       return 0 ;

   if ( c4->tag_memory == 0 )
   {
      c4->tag_memory = mem4create( c4, c4->mem_start_tag, sizeof(TAG4),
                                    c4->mem_expand_tag, 0 ) ;
      if ( c4->tag_memory == 0 )
         return 0 ;
   }

   i4 = (INDEX4 *)mem4alloc( c4->index_memory ) ;
   if ( i4 == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      return 0 ;
   }

   i4->data = d4 ;
   i4->code_base = c4 ;

   #ifdef S4FOX
      if ( file_name == 0 )
      {
         u4ncpy( buf, d4->file.name, sizeof(buf) ) ;
         u4name_ext( buf, sizeof(buf), "CDX", 1 ) ;
      }
      else
      {
         u4ncpy( buf, file_name, sizeof(buf) ) ;
         c4upper(buf) ;
         u4name_ext( buf, sizeof(buf), "CDX", 0 ) ;
      }

      #ifdef S4DEBUG
         for ( i4_ptr = 0 ;; )
         {
            i4_ptr = (INDEX4 *)l4next( &d4->indexes, i4_ptr ) ;
            if ( i4_ptr == 0 )
               break ;
            if ( !memcmp(i4_ptr->file.name, buf, (size_t)strlen( buf ) ) )
               e4severe( e4parm, E4_PARM_IND ) ;
         }
      #endif

      rc = file4open( &i4->file, c4, buf, 1 ) ;
      if ( rc )
         return 0 ;
      l4add( &d4->indexes, i4 ) ;

      i4->eof = file4len( &i4->file ) ;

      i4->tag_index = (TAG4 *)mem4alloc( c4->tag_memory ) ;
      if ( i4->tag_index == 0 )
      {
         file4close( &i4->file ) ;
         e4( c4, e4memory, 0 ) ;
         return 0 ;
      }

      if ( file_name == 0 )
      {
         if ( t4init( i4->tag_index, i4, 0L, "") < 0 )
         {
            i4close( i4 ) ;
            return 0 ;
         }
      }
      else
      {
         u4name_piece( buf, 258, file_name, 0, 0 ) ;  /* get the tag_name based on the file_name */
         if ( t4init( i4->tag_index, i4, 0L, buf) < 0 )
         {
            i4close( i4 ) ;
            return 0 ;
         }
      }

      /* Perform some checks */
      if ( i4->tag_index->header.root <= 0L ||
           i4->tag_index->header.type_code < 32 )
      {
         #ifdef S4DEBUG_DEV
            e4describe( c4, e4index, buf, "i4open()", "Root <= 0L or type_code < 32" ) ;
         #endif
         i4close( i4 ) ;
         e4( c4, e4index, buf ) ;
         return 0 ;
      }

      i4->version_old = i4->tag_index->header.version ;
      i4->block_memory = mem4create( c4, c4->mem_start_block,
                                     sizeof(B4BLOCK) + B4BLOCK_SIZE - sizeof(B4STD_HEADER) - sizeof(B4NODE_HEADER),
                                     c4->mem_expand_block, 0 ) ;
      if ( i4->block_memory == 0 )
      {
         i4close(i4) ;
         return 0 ;
      }

      /* do an initial block allocation to make sure minimal is allocated while optimization is suspended */
      b4 = (B4BLOCK *)mem4alloc2( i4->block_memory, c4 ) ;
      if ( b4 == 0 )
      {
         i4close( i4 ) ;
         e4( c4, e4memory, E4_I4OPEN ) ;
         return 0 ;
      }
      else
         mem4free( i4->block_memory, b4 ) ;



      #ifndef S4SINGLE
         /* disable locking */
         old_file_lock = i4->file_locked ;
         i4->file_locked = 1 ;
      #endif

      t4top( i4->tag_index ) ;

      /* if we have a compound index, then load the tags, otherwise this is the only tag */
      if ( i4->tag_index->header.type_code >= 64 )
      {
         if ( t4block( i4->tag_index )->header.n_keys )
            do
            {
               tag_ptr = (TAG4 *)mem4alloc( c4->tag_memory ) ;
               if ( tag_ptr == 0 )
               {
                  i4close( i4 ) ;
                  e4( c4, e4memory, 0 ) ;
                  #ifndef S4SINGLE
                     i4->file_locked = old_file_lock ;
                  #endif
                  return 0 ;
               }

               if ( t4init( tag_ptr, i4, b4recno( t4block(i4->tag_index), t4block(i4->tag_index)->key_on ), t4key_data( i4->tag_index )->value ) < 0 )
               {
                  #ifndef S4SINGLE
                     i4->file_locked = old_file_lock ;
                  #endif
                  i4close( i4 ) ;
                  return 0 ;
               }

               l4add( &i4->tags, tag_ptr ) ;
            } while ( t4skip( i4->tag_index, 1L ) == 1L ) ;
      }
      else
      {
         #ifdef S4DEBUG
            if ( file_name == 0 )
            {
               i4close( i4 ) ;
               #ifdef S4DEBUG_DEV
                  e4describe( c4, e4index, E4_INDEX_EXP, "i4open()", "file_name == 0 invalid" ) ;
               #endif
               e4( c4, e4index, E4_INDEX_EXP ) ;
            }
         #endif
         l4add( &i4->tags, i4->tag_index ) ;   /* if an .idx, add single tag */
      }
      #ifndef S4SINGLE
         i4->file_locked = old_file_lock ;
      #endif
   #endif

   #ifndef S4FOX
      if ( file_name == 0 )
      {
         u4ncpy( buf, d4->file.name, sizeof(buf) ) ;
         u4name_ext( buf, sizeof(buf), "MDX", 1 ) ;
      }
      else
      {
         u4ncpy( buf, file_name, sizeof(buf) ) ;
         c4upper(buf) ;
         u4name_ext( buf, sizeof(buf), "MDX", 0 ) ;
      }

      #ifdef S4DEBUG
         for ( i4_ptr = 0 ;; )
         {
            i4_ptr = (INDEX4 *)l4next(&d4->indexes, i4_ptr) ;
            if ( i4_ptr == 0 )
               break ;
            if ( !memcmp(i4_ptr->file.name, buf, (size_t) strlen(buf) ) )
               e4severe( e4parm, E4_PARM_IND ) ;
         }
      #endif

      rc = file4open( &i4->file, c4, buf, 1 ) ;
      if ( rc )
         return 0 ;
      l4add( &d4->indexes, i4 ) ;

      if ( file4read_all( &i4->file, 0L, &i4->header, sizeof(I4HEADER) ) < 0 )
      {
         file4close( &i4->file ) ;
         return 0 ;
      }

      #ifdef S4BYTE_SWAP
         i4->header.block_chunks = x4reverse_short( i4->header.block_chunks ) ;
         i4->header.block_rw = x4reverse_short( i4->header.block_rw ) ;
         i4->header.slot_size = x4reverse_short( i4->header.slot_size ) ;
         i4->header.num_tags = x4reverse_short( i4->header.num_tags ) ;
         i4->header.eof = x4reverse_long( i4->header.eof ) ;
         i4->header.free_list = x4reverse_long( i4->header.free_list ) ;
         i4->header.version = x4reverse_long( i4->header.version ) ;
      #endif

      /* Perform some checks */
      if ( i4->header.block_rw != i4->header.block_chunks*512  ||
           i4->header.block_chunks <= 0 ||
           i4->header.block_chunks > 63 ||
           i4->header.num_tags < 0  || i4->header.num_tags > 47 ||
           i4->header.eof <= 0L )
      {
         #ifdef S4DEBUG_DEV
            e4describe( c4, e4index, buf, "i4open()", "checks failed" ) ;
         #endif
         i4close( i4 ) ;
         e4( c4, e4index, buf ) ;
         return 0 ;
      }

      if ( file4read_all( &i4->file, 544L, tag_info, sizeof(tag_info)) < 0 )
      {
         i4close( i4 ) ;
         return 0 ;
      }

      for ( i_tag = 0; i_tag < (int) i4->header.num_tags; i_tag++ )
      {
         tag_ptr = (TAG4 *)mem4alloc( c4->tag_memory ) ;
         if ( tag_ptr == 0 )
         {
            i4close( i4 ) ;
            e4( c4, e4memory, 0 ) ;
            return 0 ;
         }

         #ifdef S4BYTE_SWAP
            tag_info[i_tag].header_pos = x4reverse_long( tag_info[i_tag].header_pos ) ;
            tag_info[i_tag].x1000 = 0x1000 ;
         #endif

         if ( t4init( tag_ptr, i4, tag_info + i_tag ) < 0 )
         {
            i4close( i4 ) ;
            return 0 ;
         }

         l4add( &i4->tags, tag_ptr ) ;
      }

      i4->block_memory = mem4create( c4, c4->mem_start_block,
                                     sizeof(B4BLOCK) + i4->header.block_rw -
                                     sizeof(B4KEY_DATA) - sizeof(short) - sizeof(char[6]),
                                     c4->mem_expand_block, 0 ) ;
      if ( i4->block_memory == 0 )
      {
         i4close(i4) ;
         return 0 ;
      }
   #endif

   #ifndef S4OPTIMIZE_OFF
      file4optimize( &i4->file, c4->optimize, OPT4INDEX ) ;
   #endif

   return i4 ;
}

#ifndef S4OFF_WRITE
int S4FUNCTION i4shrink( INDEX4 *i4, long block_no )
{
   #ifdef S4DEBUG
      if ( i4 == 0 || block_no < 0 )
         e4severe( e4parm, E4_I4SHRINK ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   #ifdef S4FOX
      #ifdef S4BYTE_SWAP
         i4->tag_index->header.free_list = x4reverse_long( i4->tag_index->header.free_list ) ;
      #endif
      if ( file4write( &i4->file, block_no, (char *)&i4->tag_index->header.free_list,
           sizeof(i4->tag_index->header.free_list)) < 0) return -1 ;
      i4->tag_index->header.free_list = block_no ;
   #else
      #ifdef S4BYTE_SWAP
         i4->header.free_list = x4reverse_long( i4->header.free_list ) ;
      #endif
      if ( file4write( &i4->file, block_no*I4MULTIPLY + sizeof(long),
           (char *)&i4->header.free_list, sizeof(i4->header.free_list)) < 0) return -1 ;
      i4->header.free_list = block_no ;
   #endif

   return 0 ;
}
#endif

TAG4 *S4FUNCTION i4tag( INDEX4 *i4, char *tag_name )
{
   char tag_lookup[11] ;
   TAG4 *tag_on ;

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4TAG ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 || tag_name == 0 )
         e4severe( e4parm, E4_I4TAG ) ;
   #endif

   u4ncpy( tag_lookup, tag_name, sizeof( tag_lookup ) ) ;
   c4upper( tag_lookup ) ;

   for( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
      if ( tag_on == 0 )
         break ;
      if ( strcmp( tag_on->alias, tag_lookup ) == 0 )
         return tag_on ;
   }

   if ( i4->code_base->tag_name_error )
      e4( i4->code_base, e4tag_name, tag_name ) ;
   return 0 ;
}

#ifndef S4OFF_WRITE
/* Updates the header if the version has changed */
int S4FUNCTION i4update_header( INDEX4 *i4 )
{
   #ifdef S4BYTE_SWAP
      I4HEADER swap ;
   #endif

   #ifdef S4MDX
      TAG4 *tag_on ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4UPDATE_HDR ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   #ifdef S4FOX
      if ( i4->version_old != i4->tag_index->header.version )
      {
         i4->tag_index->header.version = x4reverse_long( i4->tag_index->header.version ) ;
         if (file4write(&i4->file,0L, (char *)&i4->tag_index->header, T4HEADER_WR_LEN) < 0) return -1;
         i4->tag_index->header.version = x4reverse_long( i4->tag_index->header.version ) ;
         i4->version_old = i4->tag_index->header.version ;
      }
   #else
      if ( i4->changed )
      {
         #ifdef S4BYTE_SWAP
            memcpy( (void *)&swap, (void *)&i4->header, sizeof(I4HEADER) ) ;

            swap.block_chunks = x4reverse_short( swap.block_chunks ) ;
            swap.block_rw = x4reverse_short( swap.block_rw ) ;
            swap.slot_size = x4reverse_short( swap.slot_size ) ;
            swap.num_tags = x4reverse_short( swap.num_tags ) ;
            swap.eof = x4reverse_long( swap.eof ) ;
            swap.free_list = x4reverse_long( swap.free_list ) ;
            swap.version = x4reverse_long( swap.version ) ;

            if ( file4write(&i4->file,0L, (char *)&swap, sizeof(I4HEADER)) < 0 )
               return -1;
         #else
            if ( file4write(&i4->file,0L, (char *)&i4->header, sizeof(I4HEADER) ) < 0)
               return -1;
         #endif
         i4->changed = 0 ;
         for( tag_on = 0 ;; )
         {
            tag_on = d4tag_next( i4->data, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            if ( tag_on->changed == 1 )
            {
               tag_on->header.version++ ;
               tag_on->changed = 0 ;
               if ( file4write( &i4->file, tag_on->header_offset + 20L, &tag_on->header.version, sizeof(char) ) < 0)
                  return -1;
               if ( tag_on->had_keys != tag_on->has_keys )
               {
                  if ( file4write( &i4->file, tag_on->header_offset + 222 + sizeof( T4HEADER ), &tag_on->has_keys, sizeof(char) ) < 0)
                     return -1;
                  tag_on->had_keys = tag_on->has_keys ;
               }
            }
         }
      }
   #endif

   return 0 ;
}
#endif

int S4FUNCTION t4version_check( TAG4 *t4, int do_seek )
{
   #ifndef S4OPTIMIZE_OFF
      if ( t4->index->file.do_buffer == 0 )
         return i4version_check( t4->index, do_seek ) ;
      else
         return 0 ;
   #else
      return i4version_check( t4->index, do_seek ) ;
   #endif
}

/* Reads the header, checks the version to see if the blocks need to be freed. */
int S4FUNCTION i4version_check( INDEX4 *i4, int do_seek )
{
   #ifndef S4SINGLE
      TAG4 *tag_on, *save_tag ;
      int rc, need_seek ;
      B4BLOCK *b4 ;

      #ifdef S4DEBUG
         if ( i4 == 0 )
            e4severe( e4parm, E4_I4VERSION_CHK ) ;
      #endif

      if ( i4->code_base->error_code < 0 )
         return -1 ;

      if ( i4->file.is_exclusive || i4->file.is_read_only || i4->file_locked )
         return 0 ;

      #ifndef S4OPTIMIZE_OFF
         /* make sure read from disk unless file locked, etc. */
         if ( i4->file.do_buffer )  /* also makes sure 'opt' should exist */
            i4->code_base->opt.force_current = 1 ;
      #endif

      #ifdef S4FOX
         rc = file4read_all( &i4->file, 0L, &i4->tag_index->header, T4HEADER_WR_LEN ) ;
         #ifndef S4OPTIMIZE_OFF
            if ( i4->file.do_buffer )
               i4->code_base->opt.force_current = 0 ;
         #endif
         if ( rc < 0 )
            return rc ;
         i4->tag_index->header.version = x4reverse_long( i4->tag_index->header.version ) ;
         if ( i4->tag_index->header.version == i4->version_old )
            return 0 ;

         i4->version_old =  i4->tag_index->header.version ;
      #else
         rc = file4read_all( &i4->file, 0L, &i4->header, sizeof(I4HEADER) ) ;
         #ifndef S4OPTIMIZE_OFF
            if ( i4->file.do_buffer )
               i4->code_base->opt.force_current = 0 ;
         #endif
         if ( rc < 0 )
            return rc ;

         #ifdef S4BYTE_SWAP
            i4->header.block_chunks = x4reverse_short( i4->header.block_chunks ) ;
            i4->header.block_rw = x4reverse_short( i4->header.block_rw ) ;
            i4->header.slot_size = x4reverse_short( i4->header.slot_size ) ;
            i4->header.num_tags = x4reverse_short( i4->header.num_tags ) ;
            i4->header.eof = x4reverse_long( i4->header.eof ) ;
            i4->header.free_list = x4reverse_long( i4->header.free_list ) ;
            i4->header.version = x4reverse_long( i4->header.version ) ;
         #endif
         #ifndef S4OPTIMIZE_OFF
            /* make sure read from disk unless file locked, etc. */
            if ( i4->file.do_buffer )  /* also makes sure 'opt' should exist */
              i4->code_base->opt.force_current = 1 ;
         #endif   
         for( tag_on = 0 ;; )
         {
            tag_on = d4tag_next( i4->data, tag_on ) ;
            if ( tag_on == 0 )
               break ;
            if ( file4read_all( &i4->file, tag_on->header_offset + 20L, &tag_on->header.version, sizeof(char) ) < 0 )
            {
               #ifndef S4OPTIMIZE_OFF
                  if ( i4->file.do_buffer )
                     i4->code_base->opt.force_current = 0 ;
               #endif
               return -1;
            }
            if ( file4read_all( &i4->file, tag_on->header_offset + 222 + sizeof( T4HEADER ), &tag_on->has_keys, sizeof(char) ) < 0 )
            {
               #ifndef S4OPTIMIZE_OFF
                  if ( i4->file.do_buffer )
                     i4->code_base->opt.force_current = 0 ;
               #endif
               return -1;
            }
            tag_on->had_keys = tag_on->has_keys ;
         }

         #ifndef S4OPTIMIZE_OFF
            if ( i4->file.do_buffer )
               i4->code_base->opt.force_current = 0 ;
         #endif
      #endif

      need_seek = 0 ;
      save_tag = d4tag_selected( i4->data ) ;
      if ( save_tag != 0 )
      {
         /* remember the old position */
         if ( do_seek )
         {
            b4 = (B4BLOCK *)save_tag->blocks.last_node ;
            if ( b4 != 0 )
               #ifdef S4FOX
                  if ( b4leaf( b4 ) && b4->header.n_keys != 0 )
               #else
                  if ( b4leaf( b4 ) && b4->n_keys != 0 && b4->key_on < b4->n_keys )
               #endif
                  {
                     memcpy( save_tag->code_base->saved_key, (void *)(b4key( b4, b4->key_on )), save_tag->header.key_len + 2 * sizeof( long ) ) ;
                     need_seek = 1 ;
                  }
         }
      }

      for ( tag_on = 0 ;; )
      {
         tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
         if ( tag_on == 0 )
            break ;
         if ( t4free_all(tag_on) < 0 )  /* Should be a memory operation only */
         #ifdef S4DEBUG
            e4severe( e4result, E4_I4VERSION_CHK ) ;
         #else
            return -1 ;
         #endif
      }

      if ( need_seek )
         t4go( save_tag, ((B4KEY_DATA *)save_tag->code_base->saved_key)->value, ((B4KEY_DATA *)save_tag->code_base->saved_key)->num ) ;
   #endif
   return 0 ;
}

#endif  /*  ifndef  N4OTHER  */


#ifdef N4OTHER

/* This function closes all the tags corresponding with the index */
int S4FUNCTION i4close( INDEX4 *i4 )
{
   int final_rc, save_attempts ;
   CODE4 *c4 ;
   TAG4 *tag_on ;

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4CLOSE ) )
         return -1 ;
   #endif

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4CLOSE ) ;
   #endif

   c4 = i4->code_base ;

   final_rc = c4->error_code ;
   #ifndef S4SINGLE
      save_attempts = c4->lock_attempts ;
   #endif
   c4->lock_attempts = -1 ;

   #ifndef S4OFF_WRITE
      if ( i4->data )
         if ( d4update( i4->data ) < 0 )
            final_rc = e4set( c4, 0 ) ;
   #endif

   #ifndef S4SINGLE
      if ( i4unlock(i4) < 0 )
         final_rc = e4set( c4, 0 ) ;
   #endif

   for( ;; )
   {
      tag_on = (TAG4 *)l4pop( &i4->tags ) ;
      if ( tag_on == 0 )
         break ;
      t4close( tag_on ) ;
      if ( t4free_all(tag_on) < 0 )
      {
         final_rc = e4set( c4, 0) ;
         break ;
      }

      expr4free( tag_on->expr ) ;
      expr4free( tag_on->filter ) ;
      mem4free( c4->tag_memory, tag_on ) ;
   }

   mem4release( i4->block_memory ) ;
   i4->block_memory = 0 ;

   if ( i4->data )
      l4remove( &i4->data->indexes, i4 ) ;

   mem4free( c4->index_memory, i4 ) ;
   #ifndef S4SINGLE
      c4->lock_attempts = save_attempts ;
   #endif
   e4set( c4, final_rc ) ;
   return  final_rc ;
}

#ifndef S4OFF_WRITE
/* This function flushes all the tags corresponding with the index */
int S4FUNCTION i4flush( INDEX4 *i4 )
{
   TAG4 *tag_on ;

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4FLUSH ) ;
   #endif

   #ifndef S4SINGLE
      if ( i4->file_locked )
   #endif
      for ( tag_on = 0 ;; )
      {
         tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
         if ( tag_on == 0 )
            break ;
         if ( t4flush(tag_on) < 0 )
            return -1 ;
         tag_on->header.root = -1L ;
      }
      return 0 ;
}

int S4FUNCTION i4update( INDEX4 *i4 )
{
   TAG4 *tag_on ;

   #ifdef S4DEBUG
      if ( i4 == 0 )
         e4severe( e4parm, E4_I4UPDATE ) ;
   #endif

   if ( i4->code_base->error_code < 0 )
      return -1 ;

   #ifndef S4SINGLE
      if ( i4->file_locked )
   #endif
      for ( tag_on = 0 ;; )
      {
         tag_on = (TAG4 *)l4next( &i4->tags, tag_on ) ;
         if ( tag_on == 0 )
            break ;
         if ( t4update( tag_on ) < 0 )
            return -1 ;
         tag_on->header.root = -1L ;
      }
      return 0 ;
}
#endif

INDEX4 *S4FUNCTION i4open( DATA4 *d4, char *file_name )
{
   INDEX4 *i4 ;
   CODE4 *c4 ;
   TAG4 *tag ;
   int len, rc, i ;
   char buf[258], tag_buf[258], ext[3] ;
   int num_files ;
   FILE4SEQ_READ seqread ;
   char buffer[1024], t_names[258], first_byte ;
   int pos, i_pos, save_len, temp_len ;

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_I4OPEN ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 )
         e4severe( e4parm, E4_I4OPEN ) ;

      if ( file_name == 0 )
         u4name_piece( buf, sizeof( buf ), d4->file.name, 0, 0 ) ;
      else
         u4name_piece( buf, sizeof( buf ), file_name, 0, 0 ) ;
      if ( d4index( d4, buf ) )
      {
         e4( d4->code_base, e4info, E4_INFO_IAO ) ;
         return 0 ;
      }
   #endif

   c4 = d4->code_base ;
   if ( c4->error_code < 0 )
      return 0 ;

   if ( file_name == 0 )
   {
      u4ncpy( buf, d4->file.name, sizeof( buf ) ) ;
      u4name_ext( buf, sizeof( buf ), "CGP", 1 ) ;
   }
   else
   {
      u4ncpy( buf, file_name, sizeof(buf) ) ;
      c4upper( buf ) ;
      rc = u4name_ret_ext( ext, 3, buf ) ;
      if ( rc )  /* extension provided */
      {
         if ( rc == 3 )  /* check for .ndx or .cdx */
            #ifdef S4NDX
               if ( memcmp( ext, "NDX", 3 ) == 0 )
            #else
               if ( memcmp( ext, "NTX", 3 ) == 0 )
            #endif
            {
               tag = t4open( d4, (INDEX4 *)0, file_name ) ;
               if ( tag == 0 )
                  return 0 ;
               return tag->index ;
            }
      }
      else
         u4name_ext( buf, sizeof(buf), "CGP", 0 ) ;
   }

   if ( c4->index_memory == 0 )
   {
      c4->index_memory = mem4create( c4, c4->mem_start_index, sizeof( INDEX4 ), c4->mem_expand_index, 0 ) ;
      if ( c4->index_memory == 0 )
         return 0 ;
   }

   if ( c4->tag_memory == 0 )
   {
      c4->tag_memory = mem4create( c4, c4->mem_start_tag, sizeof( TAG4 ), c4->mem_expand_tag, 0 ) ;
      if ( c4->tag_memory == 0 )
         return 0 ;
   }

   i4 = (INDEX4 *)mem4alloc( c4->index_memory ) ;
   if ( i4 == 0 )
      return 0 ;

   i4->code_base = c4 ;
   u4name_piece( i4->alias, sizeof( i4->alias ), buf, 0, 0 ) ;

   if ( file4open( &i4->file, c4, buf, 1 ) )
   {
      i4close( i4 ) ;
      if ( c4->open_error )
         e4( c4, e4open, E4_INFO_SET ) ;
      return 0 ;
   }

   file4seq_read_init( &seqread, &i4->file, 0, buffer, sizeof(buffer) ) ;

   pos = 0L ;
   rc = file4seq_read_all( &seqread, &first_byte, sizeof( first_byte ) ) ;
   if ( rc )
   {
      if ( c4->open_error )
         e4( c4, e4info, E4_INFO_REA ) ;
      return 0 ;
   }

   if ( first_byte < 65 )   /* old format - potential problem if >= 65 files in an old format file. */
   {
      num_files = first_byte ;
      if ( file4seq_read_all( &seqread, &first_byte, sizeof( first_byte ) ) )
      {
         if ( c4->open_error )
            e4( c4, e4info, E4_INFO_REA ) ;
         return 0 ;
      }

      for ( i = 0 ; i < num_files ; i++ )
      {
         if ( file4seq_read_all( &seqread, &len, sizeof( len ) ) )
         {
            file4close( &i4->file ) ;
            i4close( i4 ) ;
            if ( c4->open_error )
               e4( c4, e4info, E4_INFO_REA ) ;
            return 0 ;
         }
         pos += sizeof( len ) ;
         rc = u4name_path( tag_buf, sizeof( tag_buf ), buf ) ;
         tag_buf[rc+len] = '\0' ;
         if ( sizeof( tag_buf ) > rc + len ) /* make sure room to read */
            rc = file4seq_read_all( &seqread, tag_buf+rc, len ) ;
         else
            rc = -1 ;

         if ( rc )
         {
            if ( c4->open_error )
               e4( c4, e4info, E4_INFO_REA ) ;
            file4close( &i4->file ) ;
            i4close( i4 ) ;
            return 0 ;
         }

         pos += len ;
         if  ( t4open( d4, i4, tag_buf ) == 0 )
         {
            file4close( &i4->file ) ;
            i4close( i4 ) ;
            return 0 ;
         }
      }
   }
   else
   {
      file4seq_read_init( &seqread, &i4->file, 0, buffer, sizeof(buffer) ) ;
      save_len = 0 ;

      for( ;; )
      {
         len = file4seq_read( &seqread, t_names, sizeof( t_names )) ;
         if ( len == 0 )
            break ;
         for( i_pos = 0, pos = 0 ; pos < len ; )
         {
            switch( t_names[pos] )
            {
               /* cases where the values are ignored, or found name */
               case ' ':
               case '\r':
               case '\n':
               case '\t':
                  if ( i_pos < pos )  /* try to open the file */
                  {
                     temp_len = pos - i_pos ;
                     if ( save_len == 0 )
                     {
                        rc = u4name_path( tag_buf, sizeof( tag_buf ), buf ) ;
                        tag_buf[rc+temp_len] = '\0' ;
                     }
                     else
                        rc = save_len ;
                     memcpy( tag_buf + rc, &t_names[i_pos], temp_len ) ;

                     if  ( t4open( d4, i4, tag_buf ) == 0 )
                     {
                        file4close( &i4->file ) ;
                        i4close( i4 ) ;
                        return 0 ;
                     }
                  }
                  i_pos = ++pos ;
                  break ;

               /* case where a name is attempted to be read in */
               default:
                  pos++ ;
            }
         }
         temp_len = pos - i_pos ;
         rc = u4name_path( tag_buf, sizeof( tag_buf ), buf ) ;
         tag_buf[rc+temp_len] = '\0' ;
         memcpy( tag_buf + rc, &t_names[i_pos], temp_len ) ;
         save_len = rc + temp_len ;
      }

      if ( len )  /* try to open the file */
      {
         if  ( t4open( d4, i4, tag_buf ) == 0 )
         {
            file4close( &i4->file ) ;
            i4close( i4 ) ;
            return 0 ;
         }
      }
   }

   if ( file4close ( &i4->file ) )
   {
      i4close( i4 ) ;
      if ( c4->open_error )
         e4( c4, e4info, E4_INFO_CLO ) ;
      return 0 ;
   }

   l4add( &d4->indexes, i4 ) ;
   i4->data = d4 ;
   return i4 ;
}

TAG4 *S4FUNCTION i4tag( INDEX4 *i4, char *tag_name )
{
   char tag_lookup[11] ;
   TAG4 *tag_on ;

   #ifdef S4VBASIC
      if ( c4parm_check( i4, 0, E4_I4TAG ) )
         return 0 ;
   #endif

   if ( i4 == 0 || tag_name == 0 )
      return 0 ;

   u4ncpy( tag_lookup, tag_name, sizeof(tag_lookup) ) ;
   c4upper( tag_lookup ) ;

   for( tag_on = 0 ;; )
   {
      tag_on = (TAG4 *)l4next( &i4->tags, tag_on) ;
      if ( tag_on == 0 )
         break ;
      if ( strcmp( tag_on->alias, tag_lookup) == 0 )
         return tag_on ;
   }

   if ( i4->code_base->tag_name_error )
      e4( i4->code_base, e4tag_name, tag_name ) ;
   return 0 ;
}
#endif   /*  ifdef N4OTHER  */


int S4FUNCTION i4is_production( INDEX4 *i4 )
{
   #ifdef N4OTHER
      return 0 ;
   #else
      #ifdef S4MDX
         return i4->header.is_production ;
      #endif
      #ifdef S4FOX
         int l1, l2 ;

         if ( i4->data->has_mdx )
         {
            l1 = strlen( i4->file.name ) - 4 ;  /* remove extenstion */
            l2 = strlen( i4->data->alias ) ;
            if ( l1 == l2 )
               return( memcmp( i4->file.name, i4->data->alias, l1 ) ? 0 : 1 ) ;
         }
         return 0 ;
      #endif
   #endif
}

#endif  /* S4INDEX_OFF */


#ifdef S4VB_DOS

INDEX4 * i4open_v( DATA4 *d4, char *name )
{
   char *name_ptr ;

   name_ptr = c4str(name) ;

   if (name_ptr[0] == '\0' )
      return i4open( d4, 0 ) ;

   return i4open( d4, name_ptr ) ;
}


TAG4 *i4tag_v( INDEX4 *ind, char *tag_name )
{
 return i4tag( ind, c4str(tag_name) ) ;
}

#endif
