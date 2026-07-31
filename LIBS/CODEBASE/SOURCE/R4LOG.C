/* r4log.c   (c)Copyright Sequiter Software Inc., 1992-1993.  All rights reserved. */

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

int S4FUNCTION e4is_constant( E4INFO *info_ptr )
{
   int pos ;

   if ( info_ptr->function_i == E4DOUBLE || info_ptr->function_i == E4STRING ||
        ( info_ptr->function_i >= E4LOG_LOW && info_ptr->function_i <= E4LOG_HIGH )  )
      return 1 ;

   if ( info_ptr->function_i == E4STOD || info_ptr->function_i == E4CTOD )   /* might be a constant */
   {
      for ( pos = info_ptr->num_entries - 1 ; pos >= 0 ; pos -- )
         if ( (info_ptr-pos)->field_ptr != 0 || (info_ptr-pos)->function_i >= E4CALC_FUNCTION )
            return 0 ;
      return 1 ;
   }

   return 0 ;
}

/* returns true if there is a tag that matches the desired condition type,
   AND if there is no filter on that tag */
int S4FUNCTION e4is_tag( E4INFO_REPORT *report_ptr, EXPR4 *expr, E4INFO *info_ptr, DATA4 *data )
{
   #ifndef S4INDEX_OFF
      TAG4 *tag_on ;
      int is_same, i ;
      E4INFO *info_on, *tag_info ;

      for( tag_on = 0;; )
      {
         tag_on = d4tag_next( data, tag_on ) ;
         if ( tag_on == 0 )
            break ;
         #ifdef S4NDX
            if (  t4unique( tag_on ) != r4unique_continue )  /* if unique, can't filter */
         #else
	    if ( tag_on->filter == 0 && ( t4unique( tag_on ) != r4unique_continue ) )  /* if unique a filter, than cannot bitmap optimize */
         #endif
         {
            tag_info = tag_on->expr->info + tag_on->expr->info_n -1 ;
            if ( tag_info->num_entries == info_ptr->num_entries )
            {
               is_same =  1 ;
               info_on = info_ptr ;
               for( i = 0; i < info_ptr->num_entries && is_same; i++, info_on--, tag_info-- )
               {
                  if ( memcmp( (void *)info_on, (void *)tag_info, sizeof(E4INFO)
                       - sizeof(info_on->i1) - sizeof(info_on->result_pos) 
                       - sizeof(info_on->function_i) - sizeof(info_on->function)) 
                       != 0 )
                  {
                     is_same = 0 ;
                     break ;
                  }

                  switch( info_on->function_i )
                  {
                     case E4DOUBLE:
                     case E4STRING:
                     case E4CTOD:
                     case E4DTOC:
                     case E4DTOC+1:
                        /* Compare Constant */
                        if( memcmp( tag_on->expr->constants + tag_info->i1, expr->constants + info_on->i1, tag_info->len ) != 0 )
                           is_same = 0 ;
                        break ;

                     default:
                        is_same =  (info_on->i1 == tag_info->i1) ;
                        break ;
                  }
                  if( info_on->function_i != tag_info->function_i )
                     if( info_on->function_i > E4LAST_FIELD
                                     || tag_info->function_i > E4LAST_FIELD )
                        is_same = 0 ;
               }
               if( is_same )
               {
                  report_ptr->tag = tag_on ;
                  return 1 ;
               }
            }
         }
      }
   #endif
   return 0 ;
}

int S4FUNCTION data_list4add( DATA4LIST *list, CODE4 *code_base, RELATE4 *new_pointer )
{
   if ( code_base->error_code < 0 )
      return -1 ;
   if ( new_pointer == 0 )
      return 0 ;
   if ( data_list4is_in( list, new_pointer ) )
      return 0 ;
   if( list->pointers_tot <= list->pointers_used )
   {
      list->pointers_tot += 5 ;
      if ( u4alloc_again( code_base, (char **)&list->pointers, &list->mem_allocated, list->pointers_tot * sizeof(RELATE4 *)) < 0 )
         return -1 ;
   }
   list->pointers[list->pointers_used++] = new_pointer ;
   return 0 ;
}

int S4FUNCTION data_list4expand_from_db_tree( DATA4LIST *list, CODE4 *code_base )
{
   int i ;
   RELATE4 *relate_parent ;

   for( i = list->pointers_used-1; i >= 0; i-- )
   {
      relate_parent = list->pointers[i]->master ;
      while( relate_parent != 0 )
      {
         if ( data_list4add( list, code_base, relate_parent ) < 0 )
            return -1 ;
         relate_parent = relate_parent->master ;
      }
   }
   if ( code_base->error_code < 0 )
      return -1 ;
   return 0 ;
}

int S4FUNCTION data_list4is_in( DATA4LIST *list, RELATE4 *new_pointer )
{
   int i ;
   for( i = 0 ; i < list->pointers_tot ; i++ )
      if ( list->pointers[i] == new_pointer )
         return 1 ;
   return 0 ;
}

int S4FUNCTION data_list4read_records( DATA4LIST *data_list )
{
   RELATE4 *cur ;
   int i, rc ;

   if ( data_list == 0 )
      return 0 ;

   for( i = data_list->pointers_used-1 ; i >= 0 ; i-- )
   {
      cur = data_list->pointers[i] ;
      rc = relate4read_in( cur ) ;
      if ( rc  == relate4filter_record || rc == r4terminate )
         return rc ;
      if ( rc < 0 )
         return -1 ;
   }
   return 0 ;
}

void S4FUNCTION data_list4remove( DATA4LIST *this_list, DATA4LIST *remove_list )
{
   int i ;

   #ifdef S4DEBUG
      if ( this_list == 0 || remove_list == 0 )
         e4severe( e4parm, E4_DATA_LIST4REM ) ;
   #endif

   for( i = 0; i < this_list->pointers_used; i++ )
      if( data_list4is_in( remove_list, this_list->pointers[i]) )
         this_list->pointers[i--] = this_list->pointers[--this_list->pointers_used] ;
}

int S4FUNCTION log4add_to_list( L4LOGICAL *log, E4INFO *info_ptr, DATA4LIST *list )
{
   int num_parms, i ;

   if ( info_ptr->function_i <= E4LAST_FIELD )
      if ( data_list4add( list, log->code_base, relate4lookup_relate( (RELATE4 *)&log->relation->relate, f4data(info_ptr->field_ptr)) ) < 0 )
         return -1 ;

   if ( info_ptr->num_entries == 1 )
      return 0 ;

   num_parms = info_ptr->num_parms ;
   info_ptr-- ;

   for ( i = 0; i < num_parms; i++ )
   {
      if ( log4add_to_list( log, info_ptr, list ) < 0 )
         return -1 ;
      info_ptr -= info_ptr->num_entries ;
   }
   if ( log->code_base->error_code < 0 )
      return -1 ;
   return 0 ;
}

static int log4build_database_lists( L4LOGICAL *log )
{
   int last_pos, pos, i ;
   E4INFO *info_last ;

   log->info_report = (E4INFO_REPORT *)u4alloc_er( log->code_base, (long)sizeof(E4INFO_REPORT) * log->expr->info_n ) ;
   if ( log->info_report == 0 )
      return -1 ;

   last_pos = log->expr->info_n - 1 ;
   info_last = (E4INFO *)log->expr->info + last_pos ;

   if ( info_last->function_i == E4AND )
   {
      pos = last_pos - 1 ;

      for ( i = 0; i < info_last->num_parms; i++ )
      {
         if ( log->info_report[pos].data_list == 0 )
         {
            log->info_report[pos].data_list = (DATA4LIST *)mem4create_alloc( log->code_base,
                   &log->relation->data_list_memory, 5, sizeof(DATA4LIST), 5, 0 ) ;
            if ( log->info_report[pos].data_list == 0 )
               return -1 ;
         }
         if ( log4add_to_list( log, log->expr->info+pos, log->info_report[pos].data_list ) < 0 )
            return -1 ;
         pos -= log->expr->info[pos].num_entries ;
      }
   }
   else
   {
      if ( log->info_report[last_pos].data_list == 0 )
      {
         log->info_report[last_pos].data_list = (DATA4LIST *)mem4create_alloc( log->code_base,
            &log->relation->data_list_memory, 5, sizeof( DATA4LIST ), 5, 0 ) ;
         if ( log->info_report[last_pos].data_list == 0 )
            return -1 ;
      }
      log4add_to_list( log, info_last, log->info_report[last_pos].data_list ) ;
   }

   if ( log->code_base->error_code < 0 )
      return -1 ;
   return 0 ;
}

int S4FUNCTION log4bitmap_do( L4LOGICAL *log )
{
   if ( log->code_base->error_code < 0 )
      return -1 ;

   log4build_database_lists( log ) ;
   #ifndef S4INDEX_OFF
      if ( bitmap4evaluate( log, log->expr->info_n - 1 ) < 0 )
         return -1 ;
   #endif
   if ( log->code_base->error_code < 0 )
      return -1 ;
   return 0 ;
}

int S4FUNCTION log4determine_evaluation_order( L4LOGICAL *log )
{
   /* Expand Lists due to Database Tree */
   int i, pos, num_left, cur_smallest_pos, cur_smallest_num, cur_pos ;
   int num_compare, last_pos = log->expr->info_n -1 ;
   E4INFO *info_last, *info_ptr ;
   E4INFO_REPORT *report_last, *report, *cur_report ;

   info_last = (E4INFO *)log->expr->info + last_pos ;
   report_last = log->info_report + last_pos ;

   if ( info_last->function_i != E4AND )
      return data_list4expand_from_db_tree( report_last->data_list, log->code_base ) ;

   info_ptr = info_last-1 ;
   report = report_last-1 ;
   for ( i = 0; i < info_last->num_parms; i++ )
   {
      if ( data_list4expand_from_db_tree(report->data_list, log->code_base) < 0 )
         return -1 ;

      report -= info_ptr->num_entries ;
      info_ptr -= info_ptr->num_entries ;
   }

   /* Change the evaluation orders by repeatedly determining the
      list with the smallest number of entries and puting it at the end.
      The idea is that we want the conditions which causes the fewest
      additional database records to be read in, to be evaluated first.
   */
   pos = last_pos - 1 ;  /* Position currently being made into the fewest. */

   for( num_left = info_last->num_parms; num_left > 1; num_left-- )
   {
      report = log->info_report + pos ;
      info_ptr = (E4INFO *)log->expr->info + pos ;

      /* Now determine which is the entry with the fewest data files. */
      cur_smallest_pos = pos ;
      cur_smallest_num = report->data_list->pointers_used ;

      cur_pos = pos - info_ptr->num_entries ;
      for( num_compare = num_left-1; num_compare > 0; num_compare-- )
      {
         cur_report = log->info_report + cur_pos ;

         if ( cur_report->data_list->pointers_used < cur_smallest_num )
         {
            cur_smallest_num = cur_report->data_list->pointers_used ;
            cur_smallest_pos = cur_pos ;
         }

         cur_pos -= log->expr->info[cur_pos].num_entries ;
      }

      if( pos != cur_smallest_pos )
          if ( log4swap_entries( log, pos, cur_smallest_pos ) < 0 )
             return -1 ;

      /* The next step is to remove the data list for the first evaluated
         condition from the data list of the rest of the conditions. */
      cur_pos = pos - info_ptr->num_entries ;
      for( i = num_left-1; i > 0; i-- )
      {
         cur_report = log->info_report + cur_pos ;
         data_list4remove( cur_report->data_list, report->data_list ) ;
         cur_pos -= log->expr->info[cur_pos].num_entries ;
      }

      pos -= info_ptr->num_entries ;
   }
   if ( log->code_base->error_code < 0 )
      return -1 ;
   return 0 ;
}

#define MAX_SWAP_ENTRIES  20

int S4FUNCTION log4swap_entries( L4LOGICAL *log, int a, int b )
{
   int large_entries, small_entries ;
   char   save_buf[sizeof(E4INFO) * MAX_SWAP_ENTRIES] ;
   E4INFO  *a_ptr, *b_ptr, *small, *large, *middle ;
   E4INFO_REPORT *small2, *large2, *middle2 ;
   int  small_pos, large_pos, middle_pos, middle_entries, move_positions ;

   if ( log->code_base->error_code < 0 )
      return -1 ;

   a_ptr = log->expr->info + a ;
   b_ptr = log->expr->info + b ;

   if ( a_ptr->num_entries > b_ptr->num_entries )
   {
      small = b_ptr ;
      large = a_ptr ;
      small_pos = b ;
      large_pos = a ;
   }
   else
   {
      small = a_ptr ;
      large = b_ptr ;
      small_pos = a ;
      large_pos = b ;
   }

   /* make copies of large and small entries because the info may be later
      lost as swaps take place... */
   large_entries = large->num_entries ;
   small_entries = small->num_entries ;
   if ( large_entries > MAX_SWAP_ENTRIES )
      return e4( log->code_base, e4overflow, E4_INFO_EXP ) ;

   move_positions = large_entries - small_entries ;
   if ( small_pos < large_pos )
   {
      middle_pos = small_pos + 1 ;
      middle_entries = large_pos - small_pos - large_entries ;
   }
   else
   {
      middle_pos = large_pos + 1 ;
      middle_entries = small_pos - large_pos - small_entries ;
      move_positions = -move_positions ;
   }
   middle = log->expr->info + middle_pos ;

   memcpy( save_buf, (void *)(large - large_entries + 1), sizeof(E4INFO) * large_entries ) ;
   if ( large_pos > small_pos )  /* want to move small to end of large pos... */
   {
      memcpy( (void *)(large - small_entries + 1 ), (void *)(small - small_entries + 1),
		  sizeof(E4INFO) *small_entries ) ;
      memmove( (void *)(middle + move_positions), middle, sizeof(E4INFO) * middle_entries ) ;
      memcpy( (void *)(small - small_entries + 1), save_buf, sizeof(E4INFO) * large_entries ) ;
   }
   else  /* want to move small to start of large pos... */
   {
      memcpy( (void *)(large - large_entries + 1 ), (void *)(small - small_entries + 1),
		  sizeof(E4INFO) *small_entries ) ;
      memmove( (void *)(middle + move_positions), middle, sizeof(E4INFO) * middle_entries ) ;
      memcpy( (void *)(small - large_entries + 1), save_buf, sizeof(E4INFO) * large_entries ) ;
   }

   large2  = log->info_report + large_pos ;
   small2  = log->info_report + small_pos ;
   middle2 = log->info_report + middle_pos ;

   memcpy( save_buf, (void *)(large2 - large_entries + 1), sizeof(E4INFO_REPORT) * large_entries ) ;
   if ( large_pos > small_pos )  /* want to move small to end of large pos... */
   {
      memcpy( (void *)(large2 - small_entries + 1), (void *)(small2 - small_entries + 1),
                  sizeof(E4INFO_REPORT) *small_entries ) ;
      memmove( middle2 + move_positions, middle2, sizeof(E4INFO_REPORT) * middle_entries ) ;
      memcpy( (void *)(small2 - small_entries + 1), save_buf, sizeof(E4INFO_REPORT) * large_entries ) ;
   }
   else  /* want to move small to start of large pos... */
   {
      memcpy( (void *)(large2 - large_entries + 1), (void *)(small2 - small_entries + 1),
                  sizeof(E4INFO_REPORT) *small_entries ) ;
      memmove( middle2 + move_positions, middle2, sizeof(E4INFO_REPORT) * middle_entries ) ;
      memcpy( (void *)(small2 - large_entries + 1), save_buf, sizeof(E4INFO_REPORT) * large_entries ) ;
   }

   return 0 ;
}

/* Must read in records, as appropriate, to evaluate the different parts of */
/* the expression. */
int S4FUNCTION log4true( L4LOGICAL *log )
{
   int cur_pos, rc, i, *result_ptr ;
   E4INFO *info_ptr ;
   E4INFO_REPORT *info_report_ptr ;
   int n_parms = 1 ;
   cur_pos = log->expr->info_n - 1 ;

   if( log->expr->info[cur_pos].function_i == E4AND )
   {
      n_parms = log->expr->info[cur_pos].num_parms ;
      cur_pos-- ;
   }

   /* Go through each of the & sub-expressions and evaluate them, first */
   /* reading in the appropriate database records for the sub-expression. */

   for( i = 0; i < n_parms; i++ )
   {
      info_ptr = log->expr->info + cur_pos ;
      info_report_ptr = log->info_report + cur_pos ;

      rc = data_list4read_records( info_report_ptr->data_list ) ;
      if ( rc == relate4filter_record )
         return 0 ;
      if ( rc == r4terminate )
         return rc ;
      if ( rc < 0 )
         return -1 ;

      if ( log->expr->info[cur_pos].num_parms < 2 )
      {
         if ( expr4execute( log->expr, cur_pos, (void **)&result_ptr ) < 0 )
            return -1 ;
         if ( *result_ptr == 0 )
            return 0 ;
      }
      else
      {
         #ifdef S4TEST
         #ifdef S4DEBUG
            /* in debug case, if tag, the result must always be true if we get here... */
            if ( log->code_base->bitmap_disable == 0 && !log->relation->bitmaps_freed )   /* then do check */
            {
               if ( expr4execute( log->expr, cur_pos, (void **)&result_ptr ) < 0 )
                  return -1 ;
               if ( ( (info_report_ptr-1)->tag || (info_report_ptr-2)->tag ) )
                  if ( *result_ptr == 0 )
                     e4severe( e4info, E4_LOG4TRUE ) ;
            }
         #endif
         #endif

         if ( ( (info_report_ptr-1)->tag == 0 && (info_report_ptr-2)->tag == 0 ) || log->relation->bitmaps_freed )
         {
            if ( expr4execute( log->expr, cur_pos, (void **)&result_ptr ) < 0 )
               return -1 ;
            if ( *result_ptr == 0 )
               return 0 ;
         }
      }
      cur_pos -= info_ptr->num_entries ;
   }
   if ( log->code_base->error_code < 0 )
      return -1 ;
   return 1 ;
}
