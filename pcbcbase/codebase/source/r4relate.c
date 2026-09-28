/* r4relate.c   (c)Copyright Sequiter Software Inc., 1992-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#include <time.h>

/* Query Optimization

  Database Tree Example

  D1   D11
       D12  D121
            D122
       D13
       D14

  Condition Tree Example

  &    |      &
  C1
  C2   C21
       C22
       C23    C231
              C232
  C3

  NCj  The # of databases needed to evaluate the condition.
       Order condition evaluation by NCj.

  Rushmore
  - (Tag Operator Const) on a single database.
  - High Level & Condition Only on a Single Database.

Algorithm Stages
   1. Build the character expression into a pseudo coded query expression.
   2. Calculate database sets for each high level '&' sub-expression.
   3. Determine which top level '&' conditions are bitmap optimizable.
      - Entire top level '&' must reference only a single database.
      - Must be 'tag expression' comparison 'constant'.
      Not as much optimization is done as could theoretically be done.
      This code demands that all child '&' and '|' conditions be
      optimizable.
      Attach bitmap filter to databases and remove from
      condition tree.
   5. Determine evaluation order of & expressions using database sets.
   6. Build scan list in order they are to be tried.
      Includes all Relate classes of type 'scan'.  Includes top level Relate.
      Ex.   D1 - D11
               - D12  - D121
                      - D122
               - D13
            Assuming these are all scan classes, the order is as follows:
            D11, D121, D122, D12, D13, D1
   6. Go through each record in the extended record.
      I. Read the next record of the current scan if one exists.
         Otherwise go to the next scan data file using the scan list order.
         When trying a new data file, mark its & its slaves as not read.
         -  The scan data file's index file tag information could be used
            in some cases to determine whether we are at the end of the
            scan in some cases.
         Blank out appropriate scan data files and their slaves.
         This is any Relate which is not a child or parent of the
         scan Relate just read.
      III.If there is no next scan record, read the next high level record.
      IV. Perform filter optimizations by checking logical query condition.
      V.  Otherwise, attempt to read the rest of the extended record
          checking bitmap filters.  If filtered, repeat.

      Note:
         If the high level filter condition does not cause a new
         record to be read, it is not necessary to evaluate the condition again.
         It must already have been satisfied.

   7. Display the report line as specified by headers and footers.
*/

int S4FUNCTION f4flag_is_set_flip( F4FLAG *flag_ptr, unsigned long r )
{
   if ( flag_ptr->flags == 0 )
      return 1 ;

   if ( flag_ptr->is_flip )
      return ! f4flag_is_set( flag_ptr, r ) ;
   else
      return f4flag_is_set( flag_ptr, r ) ;
}

/* returns the position of the next flipped flag in the flag set - start at r */
unsigned long S4FUNCTION f4flag_get_next_flip( F4FLAG *f4, unsigned long r, char direction )
{
   unsigned char c_flag ;
   unsigned long low_val, on_val ;
   char i ;
   unsigned long high_val ;

   c_flag = 0 ;
   on_val = r ;
   if ( f4->flags == 0 || r > f4->num_flags )
      return 0 ;

   low_val = (unsigned long)( r & 0x7 ) ;
   high_val = (unsigned long)( r >> 3 ) ;

   if ( (int)direction == -1 )
   {
      c_flag = (unsigned char)( f4->flags[high_val] ) ;
      if ( f4->is_flip )
         c_flag = (unsigned char) ~c_flag ;

      c_flag = (unsigned char)( (unsigned char)( c_flag << ( 7 - low_val ) ) >> ( 7 - low_val )) ;

      on_val += ( 7 - low_val ) ;

      if ( c_flag == 0 )
         for( ; c_flag == 0 ; on_val -= 8 )
         {
            if ( high_val-- <= 1 )  /* if was zero, or is now zero */
            {
               if ( f4->flags[0] == 0 )
                  return r ;
               c_flag = f4->flags[0] ;
               on_val -= 8 ;  /* for sure if high_val == 0, else?? */
               break ;
            }
            c_flag = f4->flags[high_val] ;
            if ( f4->is_flip )
               c_flag = (unsigned char) ~c_flag ;
         }

      for( i = 7 ; (int)i >= 0 ; i--, on_val-- )
         if ( c_flag & ( 0x01 << i ) )
            break ;

      return (r - on_val) ;
   }
   else
   {
      c_flag = (unsigned char)f4->flags[high_val] ;
      if ( f4->is_flip )
         c_flag = (unsigned char) ~c_flag ;
      c_flag = (unsigned char) (c_flag >> low_val) ;
      if ( c_flag == 0 )
      {
         on_val -= low_val ;
         for( ; c_flag == 0 ; on_val += 8 )
         {
            if ( on_val >= f4->num_flags )
               return (f4->num_flags - r + 1) ;
            c_flag = f4->flags[++high_val] ;
            if ( f4->is_flip )
               c_flag = (unsigned char) ~c_flag ;
         }
      }

      for( i = 0 ; i <= 7 ; i++, on_val++ )
         if ( c_flag & ( 0x01 << i ) )
            break ;

      return (on_val - r) ;
   }
}

int S4FUNCTION r4data_list_add( LIST4 *l4, DATA4 *data, RELATE4 *relate )
{
   R4DATA_LIST *r4 ;

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   r4 = (R4DATA_LIST *)u4alloc_free( data->code_base, sizeof( R4DATA_LIST ) ) ;
   if ( r4 == 0 )
      return -1 ;
   r4->data = data ;
   r4->relate = relate ;
   l4add( l4, r4 ) ;
   return 0 ;
}

int S4FUNCTION r4data_list_find( LIST4 *l4, RELATE4 *r4 )
{
   R4DATA_LIST *link ;

   for ( link = 0 ;; )
   {
      link = (R4DATA_LIST *)l4next( l4, link ) ;
      if ( link == 0 )
         return 0 ;
      if ( link->relate == r4 )
         return 1 ;
   }
}

void S4FUNCTION r4data_list_free( LIST4 *l4 )
{
   R4DATA_LIST *r4data, *r4data2 ;

   for ( r4data = (R4DATA_LIST *)l4first( l4 ) ; r4data ; )
   {
      r4data->relate->sort_type = 0 ;
      r4data2 = (R4DATA_LIST *)l4next( l4, r4data ) ;
      l4remove( l4, r4data ) ;
      u4free( r4data ) ;
      r4data = r4data2 ;
   }
}

/* 1 - database added, 0 - database not added, -1 - error */
/* check_type gives the caller's status in terms of whether we should be included */
int S4FUNCTION r4data_list_build( LIST4 *l4, RELATE4 *relate, EXPR4 *expr, int check_type )
{
   int i ;
   char must_add ;
   E4INFO *info ;
   RELATE4 *slave_on ;

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   must_add = 0 ;

   /* 1st check if we must belong */
   for( i = 0 ; i < expr->info_n ; i++ )
   {
      info = expr->info + i ;
      if ( info->field_ptr )
      {
         if ( info->field_ptr->data == relate->data )
         {
            must_add = 1 ;
            break ;
         }
      }
   }

   relate->sort_type = relate4exact ;

   if ( must_add )
      check_type = relate4exact ;
   else
   {
      if ( relate->relation_type == relate4scan )
         check_type = relate4scan ;
      else
         if ( check_type != relate4scan )   /* non-scan parent must be added, so we add ourselves too, in order to save work later */
            must_add = 1 ;
   }

   /* if a child must be added, we must be too: */
   for ( slave_on = 0 ;; )
   {
      slave_on = (RELATE4 *)l4next( &relate->slaves, slave_on ) ;
      if ( slave_on == 0 )
         break ;
      if ( r4data_list_build( l4, slave_on, expr, check_type ) == 1 )
         must_add = 1 ;
   }

   if ( must_add )
      r4data_list_add( l4, relate->data, relate ) ;
   else
      if ( relate->relation_type == relate4scan )
         relate->sort_type = relate4sort_skip ;

   return must_add ;
}

int S4FUNCTION relate4blank_set( RELATE4 *relate, char direction )
{
   RELATE4 *slave ;
   int rc ;

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   relate->is_read = 1 ;
   if ( direction == 1 )
   {
      if ( d4go_eof( relate->data ) < 0 )
         return -1 ;
   }
   else
   {
      rc = d4top( relate->data ) ;
      if ( rc ) 
         return rc ;
      rc = d4skip( relate->data, -1L ) ;
      relate->data->rec_num = -1 ;
      d4blank( relate->data ) ;
      relate->data->record_changed = 0 ;
      if ( relate->code_base->error_code < 0 )
         return -1 ;
      #ifndef S4SINGLE
         if ( rc == r4locked || rc < 0 )
            return rc ;
      #endif
   }

   for( slave = 0 ;; )
   {
      slave = (RELATE4 *)l4next( &relate->slaves, slave ) ;
      if ( slave == 0 )
         return 0 ;
      rc = relate4blank_set( slave, direction ) ;
      if ( rc < 0 )
         return rc ;
   }
}

int S4FUNCTION relate4bottom( RELATE4 *relate )
{
   RELATION4 *relation ;
   int rc, rc2 ;
   long rec ;
   char *ptr ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4BOTTOM ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   relation = relate->relation ;
   relate = &relation->relate ;

   if ( relation->skip_backwards == 0 )
   {
      relate4sort_free( relation, 0 ) ;
      relate4skip_enable( relate, 1 ) ;
   }

   rc = relate4top( relate ) ;
   if ( rc )
      return rc ;

   relate4set_not_read( relate ) ;

   if ( relation->in_sort == relate4sort_done )
   {
      if ( relate4sort_get_record( relation, relation->sort_rec_count ) == r4eof )
         return r4eof ;
      relation->sort_rec_on = relation->sort_rec_count ;
   }
   else
   {
      if ( d4bottom( relate->data ) < 0 )
         return -1 ;
      if ( relation->expr_source )
      {
         rec = d4recno( relate->data ) ;
         #ifndef S4INDEX_OFF
            if ( relate->data_tag )
               while ( f4flag_is_set_flip( &relate->set, rec ) == 0 )
               {
                  rc = t4skip( relate->data_tag, -1L ) ;
                  if ( rc != -1 )
                  {
                     if ( rc == 0 )
                        return r4bof ;
                     return rc ;
                  }
                  rec = t4recno( relate->data_tag ) ;
               }
            else
            {
         #endif

         if ( f4flag_is_set_flip( &relate->set, rec ) == 0 )
         {
            rec = d4recno( relate->data ) - f4flag_get_next_flip( &relate->set, d4recno( relate->data), -1 ) ;
            if ( rec == 0 )
               return r4bof ;
         }
         #ifndef S4INDEX_OFF
            }
         #endif
         d4go( relate->data, rec ) ;
      }
      relate4set_not_read( relate ) ;
   }

   rc = relate4read_rest( relate, -1 ) ;
   if ( rc == relate4filter_record )
      rc = relate4skip( relate, -1L ) ;

   if ( rc < 0 || rc == r4terminate )
      return rc ;

   if ( relation->expr_source )
   {
      rc2 = log4true( &relation->log ) ;
      if ( rc2 == r4terminate )
         return r4terminate ;
      if ( rc2 == 0 )
      {
         if ( relation->in_sort == relate4sort_skip )  /* must temporarily disable in order to get a matching scan if available */
         {
            relation->in_sort = 0 ;
            rc = relate4skip( relate, -1L ) ;
            relation->in_sort = relate4sort_skip ;
         }
         else
            rc = relate4skip( relate, -1L ) ;
      }
   }

   return rc ;
}

int S4FUNCTION relate4build_scan_list( RELATE4 *relate, RELATION4 *relation )
{
   RELATE4 *relate_on ;
   RELATE4LIST *ptr ;

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   for( relate_on = 0 ;; )
   {
      relate_on = (RELATE4 *)l4next(&relate->slaves,relate_on) ;
      if ( relate_on  == 0 )
         break ;
      if ( relate4build_scan_list( relate_on, relation ) < 0 )
         return -1 ;
   }

   if ( relate->relation_type == relate4scan || relate == &relation->relate )
   {
      ptr = (RELATE4LIST *)mem4create_alloc( relate->code_base, &relation->relate_list_memory, 5, sizeof(RELATE4LIST), 5, 0 ) ;
      if ( ptr == 0 )
         return -1 ;
      ptr->ptr = relate ;
      l4add( &relation->relate_list, ptr ) ;
   }
   return 0 ;
}

void S4FUNCTION relate4changed( RELATE4 *relate )
{
   RELATION4 *relation ;
   int j ;
   void *ptr ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4CHANGED ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return ;

   u4free( relate->scan_value ) ;
   relate->scan_value = 0 ;
   relation = relate->relation ;
   relation->is_initialized = 0 ;
   relate4sort_free( relation, 0 ) ;

   for( ptr = 0 ;; )
   {
      ptr = l4last( &relation->relate_list) ;
      if ( ptr == 0 )
         break ;
      l4remove( &relation->relate_list, ptr ) ;
      mem4free( relation->relate_list_memory, ptr ) ;
   }

   u4free( relation->relate.set.flags ) ;
   memset( (void *)&relation->relate.set, 0, sizeof( F4FLAG ) ) ;

   if ( relation->log.expr != 0 )
   {
      for( j = relation->log.expr->info_n; --j >= 0; )
      {
         E4INFO_REPORT *info_ptr = relation->log.info_report + j ;
         if ( info_ptr->data_list != 0 )
         {
            u4free( (void *)info_ptr->data_list->pointers ) ;
            #ifdef S4DEBUG
               info_ptr->data_list->pointers = 0 ;
            #endif
            mem4free( relation->data_list_memory, info_ptr->data_list ) ;
            #ifdef S4DEBUG
               info_ptr->data_list = 0 ;
            #endif
         }
      }

      expr4free( relation->log.expr ) ;
      relation->log.expr = 0 ;
      u4free( relation->log.info_report ) ;
      relation->log.info_report = 0 ;
   }
   relation->in_sort = 0 ;
}

RELATE4 *S4FUNCTION relate4create_slave( RELATE4 *master, DATA4 *slave_data, char *master_expr, TAG4 *slave_tag )
{
   RELATION4 *relation ;
   RELATE4 *slave ;

   if ( master == 0 )
      return 0 ;

   if ( master->code_base->error_code < 0 )
      return 0 ;

   relation = master->relation ;

   #ifdef S4DEBUG
      if ( slave_data == 0 || master_expr == 0 )
         e4severe( e4parm, E4_R4CREATE_SLAVE ) ;

      /* check that the d4 doesn't belong to any existing relation */
      if ( relate4lookup_relate( &relation->relate, slave_data ) != 0 )
      {
         e4( master->code_base, e4parm, E4_PARM_REL ) ;
         return 0 ;
      }
   #endif

   relate4changed( master ) ;

   slave = (RELATE4 *)mem4create_alloc( master->code_base, &relation->relate_memory, 5, sizeof(RELATE4), 5, 0 ) ;
   if ( slave == 0 )
      return 0 ;

   relate4init_relate( slave, master->relation, slave_data, master->code_base ) ;

   slave->master_expr = expr4parse( master->data, master_expr ) ;
   if ( slave->master_expr == 0 )
   {
      mem4free( relation->relate_memory, slave ) ;
      return 0 ;
   }

   #ifndef S4INDEX_OFF
      if ( slave_tag != 0 )
         if ( t4type( slave_tag ) != expr4type( slave->master_expr ) )
         {
            if ( master->code_base->relate_error )
               e4( master->code_base, e4relate, E4_RELATE_RCS ) ;
            mem4free( relation->relate_memory, slave ) ;
            return 0 ;
         }
   #endif

   slave->data_tag = slave_tag ;
   slave->master = master ;

   l4add( &master->slaves, slave ) ;
   relate4match_len( slave, -1 ) ; /* Set to maximum */

   return slave ;
}

/* checks if the given dbf belongs to one of the relations in relation */
int S4FUNCTION relate4dbf_in_relation( RELATE4 *relate, DATA4 *dbf )
{
   RELATE4 *relate_on ;

   relate_on = &relate->relation->relate ;
   while( relate_on->master )
      relate_on = relate_on->master ;

   do
   {
      if ( relate_on->data == dbf )
         return 1 ;
   } while( relate4next( &relate_on ) != 2 ) ;

   return 0 ;
}

int S4FUNCTION relate4do( RELATE4 *relate )
{
   int rc ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4DO ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   relate4set_not_read( relate ) ;
   rc = relate4read_rest( relate, 0 ) ;
   if ( rc == relate4filter_record )  /* no match is an error */
   {
      if ( relate->code_base->relate_error )
         return e4( relate->code_base, e4lookup_err, relate->data->alias ) ;
      return r4terminate ;
   }

   return rc ;
}

int S4FUNCTION relate4do_one( RELATE4 *relate )
{
   int rc ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4DO_ONE ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   if ( relate->master == 0 )   /* no master, so we must be read */
      return 0 ;
   relate4set_not_read( relate ) ;
   rc = relate4lookup( relate, 0 ) ;
   relate->is_read = relate->master->is_read ;  /* we are read if master is read */
   if ( rc == relate4filter_record )  /* no match is an error */
   {
      if ( relate->code_base->relate_error )
         return e4( relate->code_base, e4lookup_err, relate->data->alias ) ;
      return r4terminate ;
   }
   return rc ;
}

int S4FUNCTION relate4eof( RELATE4 *relate )
{
   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4EOF ) ;
   
      if ( relate->relation->is_initialized == 0 )
      {
         e4( relate->code_base, e4info, E4_INFO_REL ) ;
         return -1 ;
      }
   #endif

   if ( relate->relation->in_sort == relate4sort_done )
      return relate->relation->sort_eof_flag ;
   else
      return d4eof( relate->relation->relate.data ) ;
}

int S4FUNCTION relate4error_action( RELATE4 *relate, int code )
{
   int rc ;

   if ( relate == 0 )
      return -1 ;

   #ifdef S4DEBUG
      if ( code != relate4blank && code != relate4skip_rec && code != relate4terminate )
         e4severe( e4parm, E4_INFO_IVE ) ;
   #endif
   rc = relate->error_action ;
   relate->error_action = code ;
   return rc ;
}

int S4FUNCTION relate4free( RELATE4 *relate, int close_files )
{
   int rc = 0 ;
   RELATION4 *relation ;
   RELATE4 *relate_on ;

   if ( relate == 0 )
      return -1 ;

   #ifndef S4SINGLE
      relate4unlock( relate ) ;
   #endif

   relate4changed( relate ) ;
   relation = relate->relation ;
   relate = &relation->relate ;

   if( close_files )
      if( d4close( relate->data ) < 0 )
         rc = -1 ;

   for( relate_on = 0 ;; )
   {
      relate_on = (RELATE4 *)l4last( &relate->slaves ) ;
      if ( relate_on == 0 )
         break ;
      if( relate4free_relate( relate_on, close_files ) < 0 )
         rc = -1 ;
   }

   mem4release( relation->relate_memory ) ;
   mem4release( relation->relate_list_memory ) ;
   mem4release( relation->data_list_memory ) ;

   relate4sort_free( relation, 1 ) ;
   u4free( relation->expr_source ) ;
   u4free( relation ) ;

   return rc ;
}

int S4FUNCTION relate4free_relate( RELATE4 *relate, int close_files )
{
   int rc = 0 ;
   RELATE4 *relate_on ;
   if ( relate->master == 0 )
      return relate4free( relate, close_files ) ;

   relate4changed( relate ) ;

   if( close_files )
      if( d4close( relate->data ) < 0 )
         rc = -1 ;

   for( ;; )
   {
      relate_on = (RELATE4 *)l4last( &relate->slaves) ;
      if ( relate_on == 0 )
         break ;
      if( relate4free_relate( relate_on, close_files ) < 0 )
         rc = -1 ;
   }

   expr4free( relate->master_expr ) ;
   u4free( relate->scan_value ) ;
   relate->scan_value = 0 ;
   u4free( relate->set.flags ) ;
   relate->set.flags = 0 ;

   l4remove( &relate->master->slaves, relate ) ;
   mem4free( relate->relation->relate_memory, relate ) ;
   relate = 0 ;

   return rc ;
}

RELATE4 *S4FUNCTION relate4init( DATA4 *master )
{
   RELATION4 *relation ;
   CODE4 *code_base ;

   #ifdef S4DEBUG
      if ( master == 0 )
         e4severe( e4parm, E4_R4INIT ) ;
   #endif

   code_base = master->code_base ;
   if ( code_base->error_code < 0 )
      return 0 ;

   relation = (RELATION4 *)u4alloc_er( code_base, sizeof( RELATION4 ) ) ;
   if ( relation == 0 )
      return 0 ;

   relate4init_relate( &relation->relate, relation, master, code_base ) ;
   relation->log.relation = relation ;
   relation->log.code_base = code_base ;
   relation->sort.file.hand = -1 ;
   relation->sorted_file.hand = -1 ;

   return &relation->relate ;
}

void S4FUNCTION relate4init_relate( RELATE4 *relate, RELATION4 *relation, DATA4 *data, CODE4 *code_ptr )
{
   relate->code_base = code_ptr ;
   relate->relation_type = relate4exact ;
   relate->data = data ;
   relate->error_action  = relate4blank ;
   relate->relation = relation ;
   relate->data->count = d4reccount( relate->data ) ;
}

int S4FUNCTION relate4lock( RELATE4 *relate )
{
   #ifdef S4SINGLE
      return 0 ;
   #else
      CODE4 *code_base ;
      int rc, old_attempts, count ;
      DATA4 *data_on ;

      if ( relate == 0 )
         return -1 ;

      code_base = relate->code_base ;
      if ( code_base->error_code < 0 )
         return -1 ;

      relate->relation->locked = 1 ;

      count = old_attempts = code_base->lock_attempts ;  /* take care of wait here */
      code_base->lock_attempts = 1 ;

      for(;;)
      {
         rc = 0 ;
         for ( data_on = (DATA4 *)l4first( &code_base->data_list) ; data_on ; data_on = (DATA4 *)l4next( &code_base->data_list, data_on ) )
            if ( relate4dbf_in_relation( relate, data_on ) )
            {
               rc = d4lock_all( data_on ) ;
               if ( rc != 0 )
                  break ;
            }

         if ( rc != r4locked )
            break ;

         relate4unlock( relate ) ;
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

      code_base->lock_attempts = old_attempts ;
      if ( code_base->error_code < 0 )
         return -1 ;

      return rc ;
   #endif
}

/* direction : -1 = look backwards, 0 = lookup only, 1 = look forwards */
int S4FUNCTION relate4lookup( RELATE4 *relate, char direction )
{
   int rc, len ;
   long recno ;
   char *ptr ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4LOOKUP ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   if ( direction != 0 && relate->relation->is_initialized == 0 )
   {
      #ifdef S4DEBUG
         e4( relate->code_base, e4info, E4_INFO_REL ) ;
      #endif
      return -1 ;
   }

   relate->is_read = 1 ;
   if ( relate->master == 0 )
      return 0 ;

   d4tag_select( relate->data, relate->data_tag ) ;

   #ifndef S4INDEX_OFF
   if ( relate->data_tag == 0 )
   {
   #endif
      recno = (long)expr4double( relate->master_expr ) ;
      if ( relate->code_base->error_code < 0 )
         return -1 ;

      if ( direction != 0 )
         if ( f4flag_is_set_flip( &relate->set, recno ) == 0 )
            return relate4filter_record ;

      rc = d4go( relate->data, recno ) ;
      if ( rc < 0 )
         return -1 ;
      if ( rc != r4entry )  /* if not error, then return */
         return 0 ;
      if ( relate->relation_type == relate4approx )
      {
         d4go_eof( relate->data ) ;
         return 0 ;
      }
   #ifndef S4INDEX_OFF
   }
   else
   {
      len = expr4key( relate->master_expr, &ptr ) ;
      if ( len < 0 )
         return -1 ;

      len = (len < relate->match_len) ? len : relate->match_len ;   /* min of len and match len */

      if ( relate->relation_type == relate4scan )
      {
         #ifdef S4DEBUG
            if ( relate->master == 0 )
               e4severe( e4info, E4_RELATE_MEN ) ;
         #endif
         if ( relate->master->scan_value == 0 )
         {
            relate->master->scan_value_len = len ;
            relate->master->scan_value = (char *)u4alloc_er( relate->code_base, len ) ;
            if ( relate->master->scan_value == 0 )
               return -1 ;
         }
         memcpy( relate->master->scan_value, ptr, len ) ;
      }

      rc = t4seek( relate->data_tag, ptr, len ) ;
      if ( rc < 0 )
         return -1 ;
      if ( relate->relation_type == relate4approx || rc == 0 )
      {
         if ( t4eof( relate->data_tag) )
         {
            d4go_eof( relate->data ) ;
            return 0 ;
         }

         if ( (int)direction < 0 && rc == 0 && relate->relation_type == relate4scan )  /* look for last one */
            for( ;; )
            {
               if ( !t4skip( relate->data_tag, 1L ) )
                  break ;
               if ( u4memcmp( t4key_data( relate->data_tag )->value, ptr, len ) )
               {
                  t4skip( relate->data_tag, -1L ) ;
                  break ;
               }
            }

         recno = t4key_data( relate->data_tag )->num ;
         if ( direction != 0 )
            if ( f4flag_is_set_flip( &relate->set, recno ) == 0 )
               return relate4filter_record ;
         if ( d4go( relate->data, recno ) < 0 )
            return -1 ;
         return 0 ;
      }
      else
         recno = t4key_data( relate->data_tag )->num ;
   }
   #endif

   #ifdef S4CHANGE_BACK
      if ( relate->relation_type == relate4scan )  /* no matching records, so filter */
         return relate4filter_record ;
   #endif

   switch( relate->error_action )  /* if got here, must be error condition */
   {
      case relate4blank:
         if ( d4go_eof( relate->data ) < 0 )
            return -1 ;
         if ( direction != 0 )
            if ( f4flag_is_set_flip( &relate->set, recno ) == 0 )
               return relate4filter_record ;
         return 0 ;
      case relate4skip_rec:
         return relate4filter_record ;
      case relate4terminate:
         if ( relate->code_base->relate_error )
            return e4( relate->code_base, e4lookup_err, relate->data->alias ) ;
         return r4terminate ;
      default:
         #ifdef S4DEBUG
            /* should never get this far */
            e4severe( e4info, E4_RELATE_EAI ) ;
         #endif
         return -1 ;
   }
}

RELATE4 *S4FUNCTION relate4lookup_relate( RELATE4 *relate, DATA4 *d4 )
{
   RELATE4 *relate_return, *relate_on ;

   if ( relate->data == d4 )
      return relate ;
   for( relate_on = 0 ;; )
   {
      relate_on = (RELATE4 *)l4next( &relate->slaves, relate_on) ;
      if ( relate_on == 0 )
         return 0 ;
      relate_return = relate4lookup_relate( relate_on, d4 ) ;
      if ( relate_return )
         return relate_return ;
   }
}

int S4FUNCTION relate4match_len( RELATE4 *relate, int match_len )
{
   int len ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4MATCH_LEN ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   len = expr4key_len( relate->master_expr ) ;

   #ifdef S4CLIPPER
      if ( match_len <= 0 )
         match_len = len ;
   #else
      if ( match_len <= 0 )
      {
         relate->match_len = len ;
         return match_len ;
      }
   #endif

   #ifndef S4INDEX_OFF
      #ifndef S4CLIPPER
         if ( relate->data_tag )
            if( expr4type(  relate->data_tag->expr ) != r4str)  /* make sure r4str only */
            {
               #ifdef S4DEBUG
                  e4( relate->code_base, e4relate, E4_RELATE_REL ) ;
               #endif
               return -1 ;
            }
      #endif
   #endif

   if ( match_len >= len )
      match_len = len ;

   #ifndef S4INDEX_OFF
      if ( relate->data_tag )
      {
         len = expr4key_len( relate->data_tag->expr ) ;
         if ( match_len >= len )
            match_len = len ;
      }
   #endif

   relate->match_len = match_len ;
   relate4changed( relate ) ;
   return match_len ;
}

int S4FUNCTION relate4next( RELATE4 **ptr_ptr )
{
   RELATE4 *cur ;
   void *next_link ;
   int rc ;

   #ifdef S4DEBUG
      if ( ptr_ptr == 0 )
         e4severe( e4parm, E4_R4NEXT ) ;
      if ( *ptr_ptr == 0 )
         e4severe( e4parm, E4_R4NEXT ) ;
   #endif

   cur = *ptr_ptr ;
   rc = 1 ;

   if ( cur->slaves.n_link > 0 )
   {
      *ptr_ptr = (RELATE4 *)l4first( &cur->slaves ) ;
      return 1 ;
   }

   for(;;)
   {
      rc -- ;
      if ( cur->master == 0 )
      {
         *ptr_ptr = 0 ;
         return 2 ;
      }

      next_link = l4next( &cur->master->slaves, cur ) ;
      if ( next_link )
      {
         *ptr_ptr = (RELATE4 *)next_link ;
         return rc ;
      }

      cur = cur->master ;
   }
}

int S4FUNCTION relate4next_record_in_scan( RELATE4 *relate )
{
   long next_rec ;
   int  rc, save_code ;
   B4KEY_DATA *key ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4NEXT_RIS ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   if ( relate->relation->is_initialized == 0 )
   {
      #ifdef S4DEBUG
         e4( relate->code_base, e4info, E4_INFO_REL ) ;
      #endif
      return -1 ;
   }

   if ( relate->relation->in_sort == relate4sort_skip && relate->sort_type == relate4sort_skip )
      return r4eof ;

   #ifndef S4INDEX_OFF
   if ( relate->data_tag == 0 )
   {
   #endif
      if ( d4bof( relate->data ) )
         next_rec = 1 ;
      else
         next_rec = d4recno( relate->data ) + 1 ;
      next_rec += f4flag_get_next_flip( &relate->set, next_rec, 1 ) ;
      if ( next_rec > relate->data->count )
      {
         relate->data->count = d4reccount( relate->data ) ;
         if ( next_rec > relate->data->count )
            return r4eof ;
      }
   #ifndef S4INDEX_OFF
   }
   else
      for(;;)
      {
         if ( d4bof( relate->data ) )
         {
            if ( relate->data->num_recs == 0 )
               return r4eof ;
            rc = (int)t4top( relate->data_tag ) ;
            if ( rc < 0 )
               return -1 ;
            if ( rc == 0 )
               rc = 1 ;
            else
               rc = 0 ;
         }
         else
            rc = (int)t4skip( relate->data_tag, 1 ) ;
         if ( rc < 0 )
            return -1 ;
         if ( rc != 1 )
            return r4eof ;

         key = t4key_data( relate->data_tag) ;
         next_rec = key->num ;

         if ( relate->master )
            if ( u4memcmp( key->value, relate->master->scan_value, relate->master->scan_value_len ) != 0 )
               return r4eof ;

         if ( f4flag_is_set_flip( &relate->set, next_rec ) )
            break ;
      }
   #endif

   save_code = relate->code_base->go_error ;
   relate->code_base->go_error = 0 ;
   rc = d4go( relate->data, next_rec ) ;
   relate->code_base->go_error = save_code ;
   if ( rc < 0 )
      return -1 ;
   if ( rc == r4entry )
      return r4eof ;
   relate->is_read = 1 ;   /* we have updated this one */
   return relate4skipped ;
}

int S4FUNCTION relate4next_scan_record( RELATION4 *relation )
{
   RELATE4 *relate ;
   int rc, rc2 ;

   if ( relation->relate.code_base->error_code < 0 )
      return -1 ;

   relation->relate_list.selected = (void *)l4first( &relation->relate_list ) ;
   for(;;)
   {
      relate = ((RELATE4LIST *)relation->relate_list.selected)->ptr ;
      relate4set_not_read( relate ) ;  /* This data file & its slaves */
      if ( relation->in_sort == relate4sort_done )
         if ( r4data_list_find( &relation->sort_data_list, relate ) )
            return relate4sort_next_record( relation ) ;

      rc = relate4next_record_in_scan( relate ) ;
      if ( rc == relate4skipped )
         return 0 ;
      if ( rc < 0 )
         return -1 ;
      rc2 = relate4blank_set( relate, (char)1 ) ;
      if ( rc2 == r4locked || rc2 < 0 )  /* error or locked */
         return rc2 ;
      if ( relate->master == 0 )
         if ( d4eof( relate->data ) )
            return r4eof ;
      relation->relate_list.selected = l4next( &relation->relate_list, relation->relate_list.selected);
   }
}

int S4FUNCTION relate4prev_record_in_scan( RELATE4 *relate )
{
   long next_rec ;
   int  rc, save_code ;
   B4KEY_DATA *key ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4PREV_RIS ) ;
   #endif

   if ( relate->relation->is_initialized == 0 )
   {
      #ifdef S4DEBUG
         e4( relate->code_base, e4info, E4_INFO_REL ) ;
      #endif
      return -1 ;
   }

   #ifndef S4INDEX_OFF
   if ( relate->data_tag == 0 )
   {
   #endif
      next_rec = d4recno( relate->data ) - 1 ;
      next_rec -= f4flag_get_next_flip( &relate->set, next_rec, -1 ) ;
      if ( next_rec <= 0 )
         return r4bof ;
      if ( next_rec > relate->data->count )
      {
         relate->data->count = d4reccount( relate->data ) ;
         if ( next_rec > relate->data->count )
            return r4eof ;
      }
   #ifndef S4INDEX_OFF
   }
   else
      for(;;)
      {
         if ( relate4eof( relate ) )   /* if eof in relate, just leave on last tag entry */
            rc = t4eof( relate->data_tag ) ? 0 : -1 ;
         else
         {
            if ( d4eof( relate->data ) == 1 )
            {
               if ( relate->data->num_recs == 0 )
                  return r4bof ;
               rc = (int)t4bottom( relate->data_tag ) ;
               if ( rc < 0 )
                  return -1 ;
               if ( rc == 0 )
                  rc = -1 ;
               else
                  rc = 0 ;
            }
            else
               rc = (int)t4skip( relate->data_tag, -1L ) ;
         }
         if ( rc > 0 )
            return -1 ;
         if ( rc != -1L )
            return r4bof ;

         key = t4key_data( relate->data_tag) ;
         next_rec = key->num ;

         if ( relate->master )
            if ( u4memcmp( key->value, relate->master->scan_value, relate->master->scan_value_len ) != 0 )
               return r4bof ;

         if ( f4flag_is_set_flip( &relate->set, next_rec ) )
            break ;
      }
   #endif

   save_code = relate->code_base->go_error ;
   relate->code_base->go_error = 0 ;
   rc = d4go( relate->data, next_rec ) ;
   relate->code_base->go_error = save_code ;
   if ( rc < 0 )
      return -1 ;
   if ( rc == r4entry )
      return r4eof ;
   relate->is_read = 1 ;   /* we have updated this one */
   return relate4skipped ;
}

int S4FUNCTION relate4prev_scan_record( RELATION4 *relation )
{
   RELATE4 *relate ;
   int rc, rc2 ;

   if ( relation->relate.code_base->error_code < 0 )
      return -1 ;

   relate = ((RELATE4LIST *)l4last( &relation->relate_list ))->ptr ;
   if ( relate4eof( relate ) )  /* at eof means we must read this record */
      if ( relation->in_sort != relate4sort_done )
      {
         rc2 = relate->code_base->read_lock ;  /* suspend the potential lock call... */
         relate->code_base->read_lock = 0 ;
         rc = relate4bottom( relate ) ;
         relate->code_base->read_lock = rc2 ;
         if ( rc == r4eof )   /* no records, so can't skip back */
            return r4bof ;
         else
            return rc ;
      }

   relation->relate_list.selected = (void *)l4first( &relation->relate_list ) ;
   for(;;)
   {
      relate = ((RELATE4LIST *)relation->relate_list.selected)->ptr ;
      relate4set_not_read( relate ) ;  /* This data file & its slaves */
      if ( relation->in_sort == relate4sort_done )
         if ( r4data_list_find( &relation->sort_data_list, relate ) )
            return relate4sort_prev_record( relation ) ;

      rc = relate4prev_record_in_scan(relate) ;
      if ( rc == relate4skipped )
      {
         if ( relate4eof( relate ) )
         {
            if ( relate->relation->in_sort == relate4sort_done && relate->relation->sort_eof_flag == 1 )
            {
               relation->sort_rec_on-- ;  /* move off eof on sort part */
               relate->relation->sort_eof_flag = 0 ;
            }
            else
               d4go( relate->relation->relate.data, d4reccount( relate->relation->relate.data ) ) ;
         }
         return 0 ;
      }
      if ( rc < 0 )
         return -1 ;
      rc2 = relate4blank_set( relate, (char)-1 ) ;
      if ( rc2 == r4locked || rc2 < 0  )  /* error or locked */
         return rc2 ;
      if ( relate->master == 0 )
      {
         if ( d4bof(relate->data) )
            return r4bof ;
         if ( d4eof(relate->data) )
            return r4eof ;
      }
      relation->relate_list.selected = l4next( &relation->relate_list, relation->relate_list.selected);
   }
}

int S4FUNCTION relate4query_set( RELATE4 *relate, char *expr )
{
   int len ;

   if ( relate == 0 )
      return -1 ;

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   relate4changed( relate ) ;
   u4free( relate->relation->expr_source ) ;
   relate->relation->expr_source = 0 ;
   if ( expr == 0 )
      return 0 ;
   if ( expr[0] == 0 )
      return 0 ;
   len = strlen( expr ) + 1 ;
   relate->relation->expr_source = (char *)u4alloc_er( relate->code_base, len ) ;
   if ( relate->relation->expr_source == 0 )
      return -1 ;
   memcpy( relate->relation->expr_source, expr, len ) ;
   return 0 ;
}

int S4FUNCTION relate4read_in( RELATE4 *relate )
{
   int rc ;
   if ( relate->code_base->error_code < 0 )
      return -1 ;
   if ( relate->is_read )
      return 0 ;
   if ( relate->master )
      if ( relate->master->is_read == 0 )
      {
         rc = relate4read_in( relate->master ) ;
         if ( rc == relate4filter_record || rc == r4terminate )
            return rc ;
      }

   return relate4lookup( relate, 1 ) ;
}

int S4FUNCTION relate4read_rest( RELATE4 *relate, char direction )
{
   RELATE4 *slave ;
   int rc ;
   int scan_done ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4READ_REST ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1;

   if ( relate->is_read == 0 )
   {
      rc = relate4lookup( relate, direction );
      if ( rc < 0 || rc == relate4filter_record || rc == r4terminate )
         return rc ;
   }

   scan_done = 0 ;
   for( slave = 0 ;; )
   {
      slave = (RELATE4 *)l4next( &relate->slaves, slave ) ;
      if ( slave == 0 )
         return 0 ;
      if ( slave->relation_type == relate4scan && scan_done == 1 )
      {
         relate4blank_set( slave, (char)(1 - direction) ) ;
         slave->is_read = 1 ;
         rc = relate4read_rest( slave, direction ) ;
      }
      else
      {
         rc = relate4read_rest( slave, direction ) ;
         if ( slave->relation_type == relate4scan && rc == 0 )
            if ( !d4eof( slave->data ) )
               scan_done = 1 ;
      }
      if ( rc < 0 || rc == relate4filter_record || rc == r4terminate )
         return rc ;
   }
}

void S4FUNCTION relate4set_not_read( RELATE4 *relate )
{
   RELATE4 *slave_on ;
   if ( relate->is_read )
   {
      relate->is_read = 0 ;
      for( slave_on = 0 ;; )
      {
         slave_on = (RELATE4 *)l4next(&relate->slaves,slave_on) ;
         if ( slave_on == 0 )
            return ;
         relate4set_not_read( slave_on ) ;
      }
   }
}

int S4FUNCTION relate4skip( RELATE4 *relate, long num_skip )
{
   int rc, sort_status, rc2 ;
   signed char sign ;
   RELATION4 *relation ;
   #ifdef S4REPORT
   #ifdef S4WINDOWS
      char countstring[22];
      static long int scanreccount = 0, selectreccount = 0, sortreccount = 0;
      HWND statwin;
   #endif
   #endif

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4SKIP ) ;
   #endif

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   if ( relate->relation->is_initialized == 0 )
   {
      #ifdef S4DEBUG
         e4( relate->code_base, e4info, E4_INFO_REL ) ;
      #endif
      return -1 ;
   }

   relation = relate->relation ;
   relate = &relation->relate ;

   if ( num_skip < 0 )
   {
      if ( relation->skip_backwards == 0 )
      {
         #ifdef S4DEBUG
            e4describe( relate->code_base, e4info, E4_R4SKIP, E4_INFO_BAC, (char *)0 ) ;
         #endif
         return -1 ;
      }
      sign = -1 ;
   }
   else
      sign = 1 ;

   sort_status = 0 ;
   rc = 0 ;
   for( ; num_skip ; )
   {

      #ifdef S4REPORT
      #ifdef S4WINDOWS
           if(GetWindowWord(relate->code_base->hWnd,8) == 666)
            statwin = relate->code_base->hWnd;

         if( statwin )
         {
            if(GetWindowWord(statwin,6)==0)
            {
               SetWindowWord(statwin,6,1);
               scanreccount = sortreccount = selectreccount = 0;
            }

            scanreccount++;
            if( scanreccount < 20 || (scanreccount % 20) == 0 )
            {
               c4ltoa45(scanreccount,countstring,sizeof(countstring)-1);
               countstring[21] = 0;
               SendMessage((HWND)GetWindowWord(statwin,0),WM_SETTEXT,0,(LPARAM)((LPSTR)countstring));
            }
         }
      #endif
      #endif
    
      if ( sign > 0 )
      {
         rc = relate4next_scan_record( relation ) ;
         if ( rc == r4eof )
            break ;
      }
      else
      {
         rc = relate4prev_scan_record( relation ) ;
         if ( rc == r4bof )
            break ;
      }

      #ifdef S4SINGLE
         if ( rc < 0 )
      #else
         if ( rc < 0 || rc == r4locked )
      #endif
            break ;

      rc = relate4read_rest( relate, sign ) ;
      if ( rc == relate4filter_record )
         continue ;

      if ( rc < 0 || rc == r4terminate )
         break ;

      if ( relation->expr_source )
      {
         rc2 = log4true(&relation->log ) ;
         if ( rc2 == r4terminate )
         {
            rc = r4terminate ;
            break ;
         }
         if ( rc2 == 0 )
         {
            if ( relation->in_sort == relate4sort_skip )  /* must temporarily disable in order to get a matching scan if available */
            {
               sort_status = 1 ;
               relation->in_sort = 0 ;
            }
            continue ;
         }
      }

      num_skip -= sign ;
   }

   #ifdef S4WINDOWS
   #ifdef S4REPORT
      if(GetWindowWord(relate->code_base->hWnd,8) == 666)
         statwin = relate->code_base->hWnd;

      if( statwin )
      {
         selectreccount++;
         if(selectreccount < 20 || (selectreccount % 20) == 0)
         {
            c4ltoa45(selectreccount,countstring,sizeof(countstring)-1);
            countstring[21] = 0;
            SendMessage((HWND)GetWindowWord(statwin,2),WM_SETTEXT,0,(LPARAM)((LPSTR)countstring));
         }
         if(relate->relation->in_sort)
         {
            sortreccount++;
            if( sortreccount < 20 || (sortreccount % 20) == 0)
            {
               c4ltoa45(sortreccount,countstring,sizeof(countstring)-1);
               countstring[21] = 0;
               SendMessage((HWND)GetWindowWord(statwin,4),WM_SETTEXT,0,(LPARAM)((LPSTR)countstring));
            }
         }
      }
   #endif
   #endif

   if ( sort_status == 1 )
      relation->in_sort = relate4sort_skip ;
   return rc ;
}

int S4FUNCTION relate4skip_enable( RELATE4 *relate, int do_enable )
{
   if ( relate == 0 )
      return -1 ;

   if ( relate->code_base->error_code < 0 )
      return -1 ;

   if ( relate->relation->skip_backwards != (char) do_enable )
   {
      relate->relation->skip_backwards = (char) do_enable ;
      relate4changed( relate ) ;
   }
   return 0 ;
}

int S4FUNCTION relate4sort( RELATE4 *relate )
{
   EXPR4 *sort_expr ;
   int rc, i, len ;
   long j, zero = 0L ;
   char n_dbf, *sort_key ;
   R4DATA_LIST *r4data ;
   RELATION4 *relation ;
   CODE4 *code_base ;

   #ifdef S4DEBUG
      if ( relate == 0 )
         e4severe( e4parm, E4_R4SORT ) ;
   #endif

   code_base = relate->code_base ;
   if ( code_base->error_code < 0 )
      return -1 ;

   relation = relate->relation ;
   relate = &relation->relate ;
   rc = 0 ;
   sort_expr = expr4parse( relate->data, relation->sort_source ) ;

   relation->in_sort = relate4sort_skip ;
   relation->sort_done_flag = 0 ;

   rc = relate4top( relate ) ;
   if ( rc )   /* no records satisfy the relate, or error */
   {
      expr4free( sort_expr ) ;
      return rc ;
   }

   len = expr4key( sort_expr, &sort_key ) ;
   if ( len <= 0 )
   {
      expr4free( sort_expr ) ;
      return -1 ;
   }

   #ifdef S4DEBUG
      if ( relation->sort_data_list.n_link != 0 )
         e4severe( e4parm, E4_PARM_NFD ) ;
   #endif

   if ( r4data_list_build( &relation->sort_data_list, relate, sort_expr, relate4exact ) < 0 )
   {
      expr4free( sort_expr ) ;
      return -1 ;
   }

   n_dbf = (char) relation->sort_data_list.n_link ;

   relation->sort_other_len = (unsigned)n_dbf * sizeof( long ) ;
   relation->other_data = (char *)u4alloc( relation->sort_other_len ) ;
   if ( relation->other_data == 0 )
      return -1 ;

   rc = sort4init_free( &relation->sort, code_base, len, relation->sort_other_len, relate ) ;
   if ( rc )
   {
      expr4free( sort_expr ) ;
      return rc ;
   }

   #ifndef S4FOX
   #ifndef S4CLIPPER
      switch( expr4type( sort_expr ) )
      {
         #ifdef S4NDX
            case r4num:
            case r4num_doub:
            case r4date:
            case r4date_doub:
               relation->sort.cmp = t4cmp_doub ;
               break ;
         #endif
         #ifdef S4MDX
            case r4num:
/*               if ( is_desc )                 */
/*                  relation->sort.cmp = t4desc_bcd_cmp ;  */
/*               else                           */
               relation->sort.cmp = c4bcd_cmp ;
               break ;
            case r4date:
/*               if ( is_desc )                 */
/*                  relation->sort.cmp = t4desc_cmp_doub ; */
/*               else                           */
               relation->sort.cmp = t4cmp_doub ;
               break ;
/*            case r4str:                       */
/*               if ( is_desc )                 */
/*                  relation->sort.cmp = t4desc_memcmp ;   */
/*               else                           */
/*                  relation->sort.cmp = u4memcmp ;        */

         #endif
         default:
            break ;
      }
   #endif
   #endif

   /* call relate4top() again in case free-ups occurred */
   rc = relate4top( relate ) ;
   if ( rc )   /* no records satisfy the relate, or error */
   {
      expr4free( sort_expr ) ;
      return rc ;
   }

   for ( j = 0L, rc = 0 ; !rc ; j++, rc = relate4skip( relate, 1L ) )
   {
      for ( i = 0, r4data = 0 ;; i++ )
      {
         r4data = (R4DATA_LIST *)l4next( &relation->sort_data_list, r4data ) ;
         if ( r4data == 0 )
            break ;
         if ( d4eof( r4data->data ) )   /* relate4blank case */
            memcpy( relation->other_data + i * sizeof(long), (void *)&zero, sizeof( long ) ) ;
         else
            memcpy( relation->other_data + i * sizeof(long), (void *)&r4data->data->rec_num, sizeof( long ) ) ;
      }
      if ( expr4key( sort_expr, &sort_key ) < 0 )
      {
         expr4free( sort_expr ) ;
         u4free( relation->other_data ) ;
         relation->other_data = 0 ;
         return -1 ;
      }
      if ( sort4put( &relation->sort, j, sort_key, relation->other_data ) < 0 )
      {
         expr4free( sort_expr ) ;
         u4free( relation->other_data ) ;
         relation->other_data = 0 ;
         return -1 ;
      }
   }

   expr4free( sort_expr ) ;

   if ( rc < 0 || rc == r4terminate )
   {
      u4free( relation->other_data ) ;
      relation->other_data = 0 ;
      return rc ;
   }

   relation->sort_rec_count = j ;
   relation->in_sort = relate4sort_done ;

   if ( relation->skip_backwards )
      if ( file4temp( &relation->sorted_file, code_base, relation->sorted_file_name, 1 ) < 0 )
      {
         u4free( relation->other_data ) ;
         relation->other_data = 0 ;
         return -1 ;
      }

   if ( sort4get_init_free( &relation->sort, relate ) < 0 )
      return -1 ;

   relation->sort_rec_on = relation->sort_file_pos = relation->sort_rec_to = 0L ;

   return 0 ;
}

void S4FUNCTION relate4sort_free( RELATION4 *relation, int delete_sort )
{
   if ( relation == 0 )
      return ;

   sort4free( &relation->sort ) ;
   u4free( relation->other_data ) ;
   relation->other_data = 0 ;
   if ( relation->sorted_file.hand >= 0 )
      file4close( &relation->sorted_file ) ;
   r4data_list_free( &relation->sort_data_list ) ;
   relation->in_sort = 0 ;
   if ( delete_sort )
   {
      u4free( relation->sort_source ) ;
      relation->sort_source = 0 ;
   }
}

int S4FUNCTION relate4sort_get_record( RELATION4 *relation, long num )
{
   int len, i, rc ;
   char *other, *key ;
   R4DATA_LIST *link_on ;
   long j, num_left ;

   if ( relation->relate.code_base->error_code < 0 )
      return -1 ;

   if ( num <= 0 )
      return r4bof ;

   relation->sort_eof_flag = 0 ;
   num_left = num - relation->sort_rec_to ;

   if ( num_left <= 0 )  /* already read, so just return from file */
   {
      if ( relation->skip_backwards == 0 )
         return -1 ;
      len = file4read( &relation->sorted_file, ( num - 1 ) * relation->sort_other_len, relation->other_data, relation->sort_other_len ) ;
      if ( len != relation->sort_other_len )  /* free up and exit */
         return -1 ;
      other = relation->other_data ;
   }
   else
      while ( num_left-- )
      {
         if ( relation->sort_done_flag == 1 )  /* sort is finished, therefore must be eof */
            return r4eof ;

         rc = sort4get( &relation->sort, &j, (void **)&key, (void **)&other ) ;
         if ( rc )  /* no more items, or error */
         {
            sort4free( &relation->sort ) ;
            if ( rc == 1 )
            {
               relation->sort_eof_flag = 1 ;
               relation->sort_done_flag = 1 ;
               return r4eof ;
            }
            else
               return rc ;
         }
         relation->sort_rec_to++ ;
         if ( relation->skip_backwards )
         {
            file4write( &relation->sorted_file, relation->sort_file_pos, other, relation->sort_other_len ) ;
            relation->sort_file_pos += relation->sort_other_len ;
         }
      }

   /* now read the database records in */
   for ( i = 0, link_on = 0 ;; i++ )
   {
      link_on = (R4DATA_LIST *)l4next( &relation->sort_data_list, link_on ) ;
      if ( link_on == 0 )
         return 0 ;
      if ( *((long *)other + i ) == 0 )  /* relate4blank case */
         d4go_eof( link_on->data ) ;
      else
         d4go( link_on->data, *((long *)other + i ) ) ;
      link_on->relate->is_read = 1 ;
   }
}

int S4FUNCTION relate4sort_next_record( RELATION4 *relation )
{
   int rc ;

   if ( relation->relate.code_base->error_code < 0 )
      return -1 ;

   rc = relate4sort_get_record( relation, relation->sort_rec_on + 1 ) ;
   if ( rc == 0 )
      relation->sort_rec_on++ ;
   if ( rc == r4eof )
      relation->sort_rec_on = relation->sort_rec_count + 1 ;
   return rc ;
}

int S4FUNCTION relate4sort_prev_record( RELATION4 *relation )
{
   int rc ;

   if ( relation->relate.code_base->error_code < 0 )
      return -1 ;

   rc = relate4sort_get_record( relation, relation->sort_rec_on - 1 ) ;
   if ( rc == 0 )
      relation->sort_rec_on-- ;
   return rc ;
}

int S4FUNCTION relate4sort_set( RELATE4 *relate, char *expr )
{
   RELATION4 *relation ;
   int len ;

   if ( relate == 0 )
      return -1 ;
   if ( relate->code_base->error_code < 0 )
      return -1 ;

   relation = relate->relation ;
   relate = &relation->relate ;
   relate4changed( relate ) ;
   u4free( relate->relation->sort_source ) ;
   relate->relation->sort_source = 0 ;
   if ( expr )
      if ( expr[0] )
      {
         len = strlen( expr ) ;
         relation->sort_source = (char *)u4alloc_er( relate->code_base, len + 1 ) ;
         if ( relation->sort_source == 0 )
            return -1 ;
         memcpy( relation->sort_source, expr, len ) ;
      }

   return 0 ;
}

int S4FUNCTION relate4top( RELATE4 *relate )
{
   RELATION4 *relation ;
   int rc, rc2 ;
   long rec ;
   char *ptr ;
   CODE4 *code_base ;
   #ifndef S4OPTIMIZE_OFF
      char has_opt ;
   #endif

   #ifdef S4DEBUG
      if( relate == 0 )
         e4severe( e4parm, E4_R4TOP ) ;
   #else
      if( relate == 0 )
         return -1 ;
   #endif

   code_base = relate->code_base ;
   if ( code_base->error_code < 0 )
      return -1 ;

   relation = relate->relation ;
   relate = &relation->relate ;

   rc = 0 ;

   if ( relation->is_initialized == 0 )
   {

      #ifndef S4SINGLE
         if ( code_base->read_lock )
            rc = relate4lock( relate ) ;
      #endif
      #ifndef S4OPTIMIZE_OFF
         has_opt = (char)code_base->has_opt ;
      #endif
      if ( rc < 0 )
         return rc ;
      relate->data_tag = d4tag_selected( relate->data ) ;
      relate->relation->bitmaps_freed = 0 ;
      if ( relation->expr_source )
      {
         relation->log.expr = expr4parse( relate->data, relation->expr_source ) ;
         if ( relation->log.expr == 0 )
            return -1 ;

         if ( log4bitmap_do( &relation->log ) < 0 )
            relate->relation->bitmaps_freed = 1 ;
         log4determine_evaluation_order( &relation->log ) ;
      }

      if ( relate4build_scan_list( relate, relation ) < 0 )
         return -1 ;
      relation->relate_list.selected = (void *)l4first( &relation->relate_list) ;

      relation->is_initialized = 1 ;

      if ( relation->sort_source )
      {
         rc = relate4sort( relate ) ;
         if ( rc < 0 || rc == r4terminate )
            return rc ;
      }

      #ifndef S4OPTIMIZE_OFF
         if ( has_opt )
            d4opt_start( code_base ) ;
      #endif
   }

   relate4set_not_read( relate ) ;

   if ( relation->in_sort == relate4sort_done )
   {
      relation->sort_rec_on = 0 ;
      rc = relate4sort_next_record( relation ) ;
   }
   else
      rc = d4top( relate->data ) ;

   if ( rc )    /* eof or error */
      return rc ;

   if ( relation->expr_source )
   {
      rec = d4recno( relate->data ) ;
      if ( f4flag_is_set_flip( &relate->set, rec ) == 0 )
      {
         #ifndef S4INDEX_OFF
            if ( relate->data_tag )
               while ( f4flag_is_set_flip( &relate->set, rec ) == 0 )
               {
                  rc = t4skip( relate->data_tag, 1L ) ;
                  if ( rc != 1 )
                  {
                     if ( rc == 0 )
                     {
                        d4go_eof( relate->data ) ;
                        return r4eof ;
                     }
                     return rc ;
                  }
                  rec = t4recno( relate->data_tag ) ;
               }
            else
            {
         #endif
            rec = f4flag_get_next_flip( &relate->set, 1L, 1 ) + 1L ;
            if ( rec > relate->data->count )
            {
               relate->data->count = d4reccount( relate->data ) ;
               if ( rec > relate->data->count )
               {
                  d4go_eof( relate->data ) ;
                  return r4eof ;
               }
            }
         #ifndef S4INDEX_OFF
            }
         #endif
      }
      d4go( relate->data, rec ) ;
   }

   rc = relate4read_rest( relate, 1 ) ;
   if ( rc == relate4filter_record )
      return relate4skip( relate, 1L ) ;

   if ( rc < 0 || rc == r4terminate )
      return rc ;

   if ( relation->expr_source )
   {
      rc2 = log4true( &relation->log ) ;
      if ( rc2 == r4terminate )
         return r4terminate ;
      if ( rc2 == 0 )
      {
         if ( relation->in_sort == relate4sort_skip )  /* must temporarily disable in order to get a matching scan if available */
         {
            relation->in_sort = 0 ;
            rc = relate4skip( relate, 1L ) ;
            relation->in_sort = relate4sort_skip ;
         }
         else
            rc = relate4skip( relate, 1L ) ;
      }
   }

   return rc ;
}

int S4FUNCTION relate4type( RELATE4 *relate, int relate_type )
{
   int rc ;

   if ( relate == 0 )
      return -1 ;

   #ifdef S4DEBUG
      if ( relate_type != relate4exact && relate_type != relate4scan && relate_type != relate4approx )
         e4severe( e4parm, E4_INFO_IVT ) ;
   #endif
   rc = relate->relation_type ;
   if ( rc != relate_type )
   {
      relate->relation_type = relate_type ;
      relate4changed( relate ) ;
   }
   return rc ;
}

int S4FUNCTION relate4unlock( RELATE4 *relate )
{
   #ifndef S4SINGLE
      DATA4 *data_on ;

      #ifdef S4DEBUG
         if ( relate == 0 )
            e4severe( e4parm, E4_R4UNLOCK ) ;
      #endif

      if ( !relate->relation->locked )
         return 0 ;

      for ( data_on = (DATA4 *)l4first( &relate->code_base->data_list) ; data_on ; data_on = (DATA4 *)l4next( &relate->code_base->data_list, data_on ) )
         if ( relate4dbf_in_relation( relate, data_on ) )
            d4unlock( data_on ) ;

      relate->relation->locked = 0 ;
   #endif

   return 0 ;
}

#ifdef S4VB_DOS

RELATE4 *S4FUNCTION relate4createSlave( RELATE4 *master, DATA4 *slave, char *master_expr, TAG4 *slave_tag)
{
   return relate4createSlave(master, slave, c4str(master_expr), slave_tag) ;
}

char * relate4masterExpr( RELATE4 *r4 )
{
   #ifdef S4VBASIC
      if ( c4parm_check( r4->code_base, 1, "relate4masterExpr():" ) ) return 0 ;
   #endif

   return v4str(r4->master_expr->source) ;
}

int S4FUNCTION relate4querySet ( RELATE4 *relate, char *expr )
{
   return relate4query_set( relate, c4str(expr) ) ;
}

int S4FUNCTION relate4sortSet ( RELATE4 *relate, char *expr )
{
   return relate4sort_set( relate, c4str(expr) ) ;
}

#endif
