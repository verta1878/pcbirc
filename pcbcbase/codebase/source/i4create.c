/* i4create.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifndef S4INDEX_OFF
#ifndef S4OFF_WRITE

#include "r4reinde.h"

#ifndef N4OTHER

INDEX4 *S4FUNCTION i4create( DATA4 *d4, char *file_name, TAG4INFO *tag_data )
{
   INDEX4 *i4 ;
   CODE4 *c4 ;
   TAG4 *tag_ptr ;
   char buf[258] ;
   int i, rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_I4CREATE ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 || tag_data == 0 )
         e4severe( e4parm, E4_I4CREATE ) ;

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

   c4 = d4->code_base ;
   if ( c4->error_code < 0 )
      return 0 ;
   c4->error_code = 0 ;  /* Make sure it is not 'r4unique' or 'r4no_create'. */

   #ifndef S4OPTIMIZE_OFF
      if ( c4->has_opt )
      {
         has_opt = 1 ;
         d4opt_suspend( c4 ) ;
      }
      else
         has_opt = 0 ;
   #endif  /* not S4OPTIMIZE_OFF */

   #ifndef S4SINGLE
      if ( d4lock_file( d4 ) )
         return 0 ;
   #endif

   i4 = (INDEX4 *)mem4create_alloc( c4, &c4->index_memory, c4->mem_start_index, sizeof(INDEX4), c4->mem_expand_index, 0 ) ;
   if ( i4 == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      return 0 ;
   }

   i4->code_base = c4 ;
   i4->data = d4 ;

   memset(buf,0,sizeof(buf));
   if ( file_name )
      u4ncpy( buf, file_name, sizeof(buf) ) ;
   else
      u4name_piece( buf, sizeof( buf ), d4->file.name, 1, 0 ) ;

   #ifdef S4FOX
      u4name_ext( buf, sizeof(buf), "CDX", 0 ) ;
   #else
      u4name_ext( buf, sizeof(buf), "MDX", 0 ) ;
   #endif
   c4upper( buf ) ;

   rc = file4create( &i4->file, c4, buf, 1 ) ;
   if ( rc )
   {
      if ( rc > 0 )
         c4->error_code = rc ;

      i4close( i4 ) ;
      return 0 ;
   }

   l4add( &d4->indexes, i4 ) ;

   #ifdef S4FOX
      i4->block_memory = mem4create( c4, c4->mem_start_block,
           (sizeof(B4BLOCK)) + B4BLOCK_SIZE - (sizeof(B4STD_HEADER)) - (sizeof(B4NODE_HEADER)), c4->mem_expand_block, 0 ) ;
      if ( i4->block_memory == 0 )
      {
         i4close( i4 ) ;
         return 0 ;
      }

      if ( c4->tag_memory == 0 )
      {
         c4->tag_memory = mem4create( c4, c4->mem_start_tag, sizeof(TAG4), c4->mem_expand_tag, 0 ) ;
         if ( c4->tag_memory == 0 )
            return 0 ;
      }

      i4->tag_index = (TAG4 *) mem4alloc( c4->tag_memory ) ;
      if ( i4->tag_index == 0 )
      {
         e4( c4, e4memory, 0 ) ;
         return 0 ;
      }

      i4->tag_index->code_base = c4 ;
      i4->tag_index->index = i4 ;
      i4->tag_index->header.type_code = 0xE0 ;  /* compound, compact */
      i4->tag_index->header.filter_len = 1 ;
      i4->tag_index->header.filter_pos = 1 ;
      i4->tag_index->header.expr_len = 1 ;
      i4->tag_index->header.expr_pos = 0 ;
      i4->tag_index->header.key_len = 10 ;

      u4name_piece( i4->tag_index->alias, sizeof(i4->tag_index->alias), buf, 0, 0 ) ;

      for ( i = 0; tag_data[i].name; i++ )
      {
         tag_ptr = (TAG4 *) mem4alloc( c4->tag_memory ) ;
         if ( tag_ptr == 0 )
            e4(  c4, e4memory, 0 ) ;

         tag_ptr->code_base = c4 ;
         tag_ptr->index = i4 ;

         u4ncpy( tag_ptr->alias, tag_data[i].name, sizeof(tag_ptr->alias) ) ;
         c4upper( tag_ptr->alias ) ;

         tag_ptr->header.type_code  = 0x60 ;  /* compact */
         if ( tag_data[i].unique )
         {
            tag_ptr->header.type_code += 0x01 ;
            tag_ptr->unique_error = tag_data[i].unique ;

            #ifdef S4DEBUG
               if ( tag_data[i].unique != e4unique &&
                    tag_data[i].unique != r4unique &&
                    tag_data[i].unique != r4unique_continue )
                  e4severe( e4parm, E4_PARM_UNI ) ;
            #endif
         }
         if ( tag_data[i].descending)
         {
            tag_ptr->header.descending = 1 ;
            #ifdef S4DEBUG
               if ( tag_data[i].descending != r4descending )
                  e4severe( e4parm, E4_PARM_FLA ) ;
            #endif
         }

         #ifdef S4DEBUG
            if ( tag_data[i].expression == 0 )
               e4severe( e4parm, E4_PARM_TAG ) ;
         #endif

         tag_ptr->expr = expr4parse( d4, tag_data[i].expression ) ;
         if ( tag_ptr->expr == 0 )
         {
            mem4free( c4->tag_memory, tag_ptr ) ;
            i4close( i4 );
            return 0;
         }

         tag_ptr->header.expr_len = strlen( tag_ptr->expr->source ) + 1 ;
         if ( tag_data[i].filter != 0 )
            if ( *( tag_data[i].filter ) != '\0' )
            {
               tag_ptr->header.type_code += 0x08 ;
               tag_ptr->filter = expr4parse( d4, tag_data[i].filter ) ;
               if (tag_ptr->filter)
                  tag_ptr->header.filter_len = strlen( tag_ptr->filter->source ) ;
            }
         tag_ptr->header.filter_len++ ;  /* minimum of 1, for the '\0' */
         tag_ptr->header.filter_pos = tag_ptr->header.expr_len ;

         if ( c4->error_code < 0 )
            break ;
         l4add( &i4->tags, tag_ptr ) ;
      }
   #else                /* if not S4FOX   */
      i4->header.two = 2 ;
      u4yymmdd( i4->header.create_date ) ;

      if ( file_name == 0 )
         i4->header.is_production = 1 ;

      i4->header.num_slots = 0x30 ;
      i4->header.slot_size = 0x20 ;

      u4name_piece( i4->header.data_name, sizeof( i4->header.data_name ), d4->file.name, 0, 0 ) ;
      i4->header.block_chunks = c4->mem_size_block/512 ;

      #ifdef S4DEBUG
         if ( i4->header.block_chunks < 2 || i4->header.block_chunks > 63 )   /* disallowed for compatibility reasons */
            e4severe( e4info, E4_INFO_BLO ) ;
      #endif

      i4->header.block_rw = i4->header.block_chunks * I4MULTIPLY ;

      i4->block_memory = mem4create( c4, c4->mem_start_block,
                                     (sizeof(B4BLOCK)) + i4->header.block_rw -
                                     (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[6])),
                                     c4->mem_expand_block, 0 ) ;
      if ( i4->block_memory == 0 )
      {
         i4close( i4 ) ;
         return 0 ;
      }

      for ( i = 0 ; tag_data[i].name ; i++ )
      {
         i4->header.num_tags++ ;

         if ( c4->tag_memory == 0 )
         {
            c4->tag_memory = mem4create( c4, c4->mem_start_tag, sizeof(TAG4),
                                                     c4->mem_expand_tag, 0 ) ;
            if ( c4->tag_memory == 0 )
               return 0 ;
         }

         tag_ptr = (TAG4 *) mem4alloc( c4->tag_memory ) ;
         if ( tag_ptr == 0 )
         {
            e4(  c4, e4memory, 0 ) ;
            return 0 ;
         }
         memset( (void *)tag_ptr,0, sizeof(TAG4) ) ;

         tag_ptr->code_base = c4 ;
         tag_ptr->index = i4 ;

         u4ncpy( tag_ptr->alias, tag_data[i].name, sizeof(tag_ptr->alias) ) ;
         c4upper( tag_ptr->alias ) ;

         tag_ptr->header.type_code  = 0x10 ;
         if ( tag_data[i].unique )
         {
            tag_ptr->header.type_code += 0x40 ;
            tag_ptr->header.unique = 0x4000 ;
            tag_ptr->unique_error = tag_data[i].unique ;

            #ifdef S4DEBUG
               if ( tag_data[i].unique != e4unique &&
                    tag_data[i].unique != r4unique &&
                    tag_data[i].unique != r4unique_continue )
                  e4severe( e4parm, E4_PARM_UNI ) ;
            #endif
         }
         if ( tag_data[i].descending)
         {
            tag_ptr->header.type_code += 0x08 ;
            #ifdef S4DEBUG
               if ( tag_data[i].descending != r4descending )
                  e4severe( e4parm, E4_PARM_FLA ) ;
            #endif
         }

         #ifdef S4DEBUG
            if ( tag_data[i].expression == 0 )
               e4severe( e4parm, E4_PARM_TAG ) ;
         #endif

         tag_ptr->expr = expr4parse( d4, tag_data[i].expression ) ;
         if( tag_ptr->expr == 0 )
         {
            mem4free( c4->tag_memory, tag_ptr ) ;
            i4close( i4 );
            return 0;
         }
         if ( tag_data[i].filter != 0 )
            if ( *(tag_data[i].filter) != '\0' )
               tag_ptr->filter = expr4parse( d4, tag_data[i].filter ) ;

         if ( c4->error_code < 0 )
            break ;
         l4add( &i4->tags, tag_ptr ) ;
      }

      #ifdef S4DEBUG
         if ( i4->header.num_tags > 47 )
            e4severe( e4parm, E4_PARM_TOO ) ;
      #endif
   #endif

   if ( i4reindex( i4 ) == r4unique )
   {
      c4->error_code = r4unique ;
      i4close( i4 ) ;
      return 0 ;
   }

   if ( file_name == 0 )
   {
      #ifdef S4BYTE_SWAP
         d4->has_mdx = 0x100 ;
      #else
         d4->has_mdx = 1 ;
      #endif

      file4write( &d4->file, ( 4 + ( sizeof( long ) ) + 2 * ( sizeof( short ) ) + ( sizeof( char[16] ) ) ),
        &i4->data->has_mdx, sizeof( i4->data->has_mdx ) ) ;

      #ifdef S4BYTE_SWAP
         d4->has_mdx = 1 ;
      #endif
   }
   if ( c4->error_code < 0 || c4->error_code == r4unique )
   {
      i4close( i4 ) ;
      return 0 ;
   }
   #ifndef S4OPTIMIZE_OFF
      file4optimize( &i4->file, c4->optimize, OPT4INDEX ) ;
      if ( has_opt )
         d4opt_start( c4 ) ;
   #endif  /* not S4OPTIMIZE_OFF */
   return i4 ;
}

#endif   /*  ifndef N4OTHER  */

#ifdef N4OTHER

INDEX4 *S4FUNCTION i4create( DATA4 *d4, char *file_name, TAG4INFO *tag_data )
{
   INDEX4 *i4 ;
   CODE4 *c4 ;
   char buf[258] ;
   int i, rc ;
   #ifdef S4OLD_CODE
      int len, num_files ;
      long pos ;
   #else
      char buffer[1024] ;
      FILE4SEQ_WRITE seqwrite ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_I4CREATE ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 || tag_data == 0 )
         e4severe( e4parm, E4_I4CREATE ) ;

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

   c4 = d4->code_base ;
   if ( c4->error_code < 0 )
      return 0 ;
   c4->error_code = 0 ;  /* Make sure it is not 'r4unique' or 'r4no_create'. */

   #ifndef S4SINGLE
      if ( d4lock_file( d4 ) )
         return 0 ;
   #endif

   i4 = (INDEX4 *)mem4create_alloc( c4, &c4->index_memory, c4->mem_start_index, sizeof(INDEX4), c4->mem_expand_index, 0 ) ;

   if ( i4 == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      return 0 ;
   }

   i4->code_base = c4 ;
   i4->data = d4 ;

   memset( buf, 0, sizeof( buf ) ) ;

   if ( file_name )  /* create a group file */
   {
      u4ncpy( buf, file_name, sizeof( buf ) ) ;
      u4ncpy( i4->alias, buf, sizeof( i4->alias ) ) ;
      c4trim_n( i4->alias, sizeof( i4->alias ) ) ;
      c4upper( i4->alias ) ;

      u4name_ext( buf, sizeof( buf ), "CGP", 1 ) ;
      c4upper( buf ) ;

      rc = file4create( &i4->file, c4, buf, 1 ) ;
      if ( rc )
      {
         if ( rc > 0 )
            c4->error_code = rc ;

         file4close( &i4->file ) ;
         return 0 ;
      }

      #ifdef S4OLD_CODE
         /* calculate # of files */
         for ( num_files = 0; tag_data[num_files].name; num_files++ ) ;

         pos = 0L ;
         file4write( &i4->file, pos, &num_files, sizeof( num_files ) ) ;
         pos += sizeof( num_files ) ;

         /* create the group file */
         for ( i = 0; tag_data[i].name; i++ )
         {
            len = strlen( tag_data[i].name ) ;
            file4write( &i4->file, pos, &len, sizeof( len ) ) ;
            pos += sizeof( len ) ;
            file4write( &i4->file, pos, tag_data[i].name, len ) ;
            pos += len ;
         }
      #else
         file4seq_write_init( &seqwrite, &i4->file, 0L, buffer, sizeof( buffer ) ) ;

         /* create the group file */
         for ( i = 0; tag_data[i].name; i++ )
         {
            file4seq_write( &seqwrite, tag_data[i].name, strlen( tag_data[i].name ) ) ;
            file4seq_write( &seqwrite, "\r\n", 1 ) ;
         }

         file4seq_write_flush( &seqwrite ) ;
      #endif

      file4close ( &i4->file ) ;

      #ifndef S4SINGLE
         if ( rc )
         {
            if ( rc > 0 )
               c4->error_code = rc ;
            return 0 ;
         }
      #endif
      i4->path = buf ;
   }

   l4add( &d4->indexes, i4 ) ;

   /* now create the actual tag files */
   for ( i = 0 ; tag_data[i].name ; i++ )
      if( t4create( d4, &tag_data[i], i4 ) == 0 )
      {
         i4close( i4 ) ;
         return 0 ;
      }

   if ( c4->error_code < 0 || c4->error_code == r4unique )
   {
      i4close( i4 ) ;
      return 0 ;
   }

   if ( i4reindex( i4 ) == r4unique )
   {
      i4close( i4 ) ;
      return 0 ;
   }

   return i4 ;
}

/* this function does not reindex if an 'i4ndx' is passed as a parameter */
/* this allows several creations before an actual reindex must occur */
TAG4 *S4FUNCTION t4create( DATA4 *d4, TAG4INFO *tag_data, INDEX4 *i4ndx )
{
   CODE4  *c4 ;
   INDEX4 *i4 ;
   char   buf[258] ;
   TAG4 *t4 ;
   int rc ;
   #ifndef S4OPTIMIZE_OFF
      int has_opt ;
   #endif
   #ifdef S4DEBUG
      int old_tag_err ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( d4, 2, E4_T4CREATE ) )
         return 0 ;
   #endif

   #ifdef S4DEBUG
      if ( d4 == 0 || tag_data == 0 )
         e4severe( e4parm, E4_T4CREATE ) ;
      #ifdef S4NDX
         if( ( tag_data->filter != 0 && *tag_data->filter != '\0' ) || tag_data->descending != 0 )
            e4severe( e4parm, E4_PARM_FOR ) ;
      #endif

      u4name_piece( buf, sizeof( buf ), tag_data->name, 0, 0 ) ;

      if ( d4->code_base->error_code < 0 )
         return 0 ;

      old_tag_err = d4->code_base->tag_name_error ;
      d4->code_base->tag_name_error = 0 ;
      if ( d4tag( d4, buf ) )
      {
         e4( d4->code_base, e4info, E4_INFO_TAO ) ;
         d4->code_base->tag_name_error = old_tag_err ;
         return 0 ;
      }
      d4->code_base->tag_name_error = old_tag_err ;
      d4->code_base->error_code = 0 ;
   #endif

   c4 = d4->code_base ;
   if ( c4->error_code < 0 )
      return 0 ;
   c4->error_code = 0 ;  /* Make sure it is not 'r4unique' or 'r4no_create'. */

   #ifndef S4OPTIMIZE_OFF
      if ( c4->has_opt )
      {
         has_opt = 1 ;
         d4opt_suspend( c4 ) ;
      }
      else
         has_opt = 0 ;
   #endif  /* not S4OPTIMIZE_OFF */

   if ( i4ndx == 0 )   /* must create an index for the tag */
   {
      if ( c4->index_memory == 0 )
         c4->index_memory = mem4create( c4, c4->mem_start_index, sizeof(INDEX4),
                                        c4->mem_expand_index, 0 ) ;
      if ( c4->index_memory == 0 )
         return 0 ;
      i4 = (INDEX4 *) mem4alloc( c4->index_memory ) ;
      if ( i4 == 0 )
      {
         e4( c4, e4memory, 0 ) ;
         return 0 ;
      }
      i4->data = d4 ;
      i4->code_base = c4 ;
      #ifdef S4DEBUG
         u4name_piece( buf, sizeof( buf ), i4->alias, 0, 0 ) ;
         if ( d4index( d4, buf ) )
         {
            e4( d4->code_base, e4info, E4_INFO_IAO ) ;
            return 0 ;
         }
      #endif
      u4name_piece( i4->alias, sizeof( i4->alias ), tag_data->name, 0, 0 ) ;
   }
   else
      i4 = i4ndx ;

   if ( c4->tag_memory == 0 )
   {
      c4->tag_memory = mem4create( c4, c4->mem_start_tag, sizeof(TAG4),
                                               c4->mem_expand_tag, 0 ) ;
      if ( c4->tag_memory == 0 )
         return 0 ;
   }

   t4 = (TAG4 *) mem4alloc( c4->tag_memory ) ;
   if ( t4 == 0 )
   {
      e4( c4, e4memory, 0 ) ;
      return 0 ;
   }

   if ( i4->block_memory == 0 )
      i4->block_memory = mem4create( c4, c4->mem_start_block,
                                     sizeof(B4BLOCK) + B4BLOCK_SIZE -
                                     (sizeof(B4KEY_DATA)) - (sizeof(short)) - (sizeof(char[2])),
                                     c4->mem_expand_block, 0 ) ;

   if ( i4->block_memory == 0 )
   {
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   if ( i4ndx != 0 && i4->path != 0)
   {
      rc = u4name_path( buf, sizeof(buf), i4ndx->path ) ;
      u4ncpy( buf+rc, tag_data->name, sizeof(buf)-rc ) ;
   }
   else
      u4ncpy( buf, tag_data->name, sizeof(buf) ) ;

   c4upper(buf) ;

   #ifdef S4NDX
      u4name_ext( buf, sizeof(buf), "NDX", 0 ) ;
   #else
      #ifdef S4CLIPPER
         u4name_ext( buf, sizeof(buf), "NTX", 0 ) ;
      #endif
   #endif

   u4name_piece( t4->alias, sizeof( t4->alias ), tag_data->name, 0, 0 ) ;
   c4upper( t4->alias ) ;

   rc = file4create( &t4->file, c4, buf, 1 ) ;
   if ( rc )
   {
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   if ( tag_data->unique )
   {
      #ifdef S4NDX
         t4->header.unique     = 0x4000 ;
      #else
         #ifdef S4CLIPPER
            t4->header.unique     = 0x01 ;
         #endif
      #endif

      t4->unique_error = tag_data->unique ;

      #ifdef S4DEBUG
         if ( tag_data->unique != e4unique &&
              tag_data->unique != r4unique &&
              tag_data->unique != r4unique_continue )
            e4severe( e4parm, E4_PARM_UNI ) ;
      #endif
   }

   #ifdef S4CLIPPER
      if ( tag_data->descending)
      {
         t4->header.descending = 1 ;
         #ifdef S4DEBUG
            if ( tag_data->descending != r4descending )
               e4severe( e4parm, E4_PARM_FLA ) ;
         #endif
      }
   #endif

   #ifdef S4DEBUG
      if ( tag_data->expression == 0 )
         e4severe( e4parm, E4_PARM_TAG ) ;
   #endif

   t4->expr = expr4parse( d4, tag_data->expression ) ;
   if ( t4->expr == 0 )
   {
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   #ifdef S4CLIPPER
      if ( tag_data->filter != 0 )
         if ( *( tag_data->filter ) != '\0' )
            t4->filter = expr4parse( d4, tag_data->filter ) ;
   #endif

   #ifdef S4NDX
      t4->header.eof = 2 ;
      t4->header.root = 1 ;
   #else
      #ifdef S4CLIPPER
         t4->header.eof = 0 ;
         t4->header.root = 1024 ;
         t4->header.key_len = c4->numeric_str_len ;
         t4->header.key_dec = c4->decimals ;
         if( t4->expr->type == r4num )
         {
            t4->header.key_len = t4->expr->key_len ;
            t4->header.key_dec = t4->expr->key_dec ;
         }
      #endif
   #endif

   if ( c4->error_code < 0 )
   {
      mem4free( c4->tag_memory, t4 ) ;
      return 0 ;
   }

   t4->code_base = c4 ;
   t4->index = i4 ;

   /* add the tag to the index list */
   l4add( &i4->tags, t4 ) ;

   if ( i4ndx == 0 )   /* single create, so reindex now */
   {
      if ( t4reindex( t4 ) == r4unique )
      {
         c4->error_code = r4unique ;
         mem4free( c4->tag_memory, t4 ) ;
         return 0 ;
      }

      l4add( &i4->data->indexes, i4 ) ;
   }

   #ifndef S4OPTIMIZE_OFF
      file4optimize( &t4->file, c4->optimize, OPT4INDEX ) ;
      if ( has_opt )
         d4opt_start( c4 ) ;
   #endif
   return t4 ;
}
#endif   /* N4OTHER */

#endif   /* S4OFF_WRITE */
#endif   /* S4OFF_INDEX */

#ifdef S4VB_DOS

INDEX4 * i4create_v ( DATA4  *d4, char near *name, TAG4INFO *t4 )
{
   return i4create( d4, c4str(name), t4 ) ;
}

INDEX4 * i4createProd ( DATA4  *d4, TAG4INFO *t4 )
{
   return i4create( d4, 0, t4 ) ;
}

#endif
