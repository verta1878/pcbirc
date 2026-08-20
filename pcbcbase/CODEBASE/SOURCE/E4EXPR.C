/* e4expr.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifdef __TURBOC__
   #pragma hdrstop
#endif

char **expr4 ;
char  *expr4constants ;
E4INFO *expr4info_ptr ;
EXPR4 *expr4ptr ;

int S4FUNCTION expr4start( CODE4 *c4 )
{
   #ifdef S4OS2SEM
   #ifdef S4OS2
      APIRET rc ;

      rc = DosRequestMutexSem( c4->hmtx_expr, -1 ) ;
      if ( rc != 0 )
         return e4( c4, e4info, "OS/2 Semaphore Failure" ) ;
   #endif
   #endif
   expr4buf = c4->expr_work_buf ;  /* initialize the working buffer pointer */
   return 0 ;
}

static void expr4stop( CODE4 *c4 )
{
   /* clear the globals to ensure they are not used without initialization */
   expr4buf = 0 ;
   expr4 = 0 ;
   expr4ptr = 0 ;
   expr4info_ptr = 0 ;
   expr4constants = 0 ;
   #ifdef S4OS2SEM
   #ifdef S4OS2
      DosReleaseMutexSem( c4->hmtx_expr ) ;
   #endif
   #endif
}

int S4FUNCTION expr4compare_flip( int function_i )
{
   if ( function_i == E4GREATER )
      return  E4LESS_EQ ;
   if ( function_i == E4LESS )
      return  E4GREATER_EQ ;
   if ( function_i == E4GREATER_EQ )
      return  E4LESS ;
   if ( function_i == E4LESS_EQ )
      return E4GREATER ;
   return function_i ;   /* not equal, or equal do not change */
/*   return -1 ; */
}

#ifdef S4FOX
void t4dbl_to_fox( char *result, double doub ) 
{
   char i ;
   if ( doub >= 0 )
   {
      for ( i = 0 ; i < 8 ; i++ )
         result[i] = *( (char *)&doub + 7 - i ) ;
      result[0] += (unsigned)0x80 ;
   }
   else /* negative */
      for ( i = 0 ; i < 8 ; i++ )
         result[i] = (char) (~(*( (unsigned char *)&doub + 7 - i ))) ;
}
#endif

double S4FUNCTION expr4double( EXPR4 *e4expr )
{
   char *ptr ;
   int len ;

   len = expr4vary( e4expr, &ptr ) ;
   if ( len >= 0 )
      switch( expr4type( e4expr ) )
      {
         case r4num_doub:
         case r4date_doub:
            return  *( (double *)ptr ) ;
         case r4num:
         case r4str:
            return c4atod( ptr, len ) ;
         case r4date:
            return (double)date4long( ptr ) ;
         default:
           #ifdef S4DEBUG
              e4severe( e4info, E4_EXPR4DOUBLE ) ;
           #endif
           break ;
      }

   return 0.0 ;
}

int S4FUNCTION expr4execute( EXPR4 *expr, int pos, void **result_ptr_ptr )
{
   E4INFO *last_info ;
   char *pointers[E4MAX_STACK_ENTRIES] ;
   int info_pos  ;

   #ifdef S4DEBUG
      if ( expr == 0 || pos < 0 || result_ptr_ptr == 0 )
         e4severe( e4parm, E4_EXPR4EXEC ) ;
   #endif

   if ( expr->code_base->error_code < 0 )
      return -1 ;

   if ( expr4start( expr->code_base ) != 0 )
      return -1 ;

   expr4 = pointers ;
   expr4constants = expr->constants ;
   expr4ptr = expr ;
   last_info = expr->info + pos ;
   info_pos = pos - last_info->num_entries + 1 ;

   for( ; info_pos <= pos ; info_pos++ )
   {
      expr4info_ptr = expr->info+ info_pos ;
      (*expr4info_ptr->function)() ;
   }

   *result_ptr_ptr = pointers[0] ;
   #ifdef S4DEBUG
      if ( pointers[0] != expr4[-1] )
         return e4( expr->code_base, e4result, 0 ) ;
   #endif

   expr4stop( expr->code_base ) ;

   return expr->code_base->error_code ;
}

void S4FUNCTION expr4free( EXPR4 *e4expr )
{
   if ( e4expr != 0 )
      u4free( e4expr ) ;
}

int S4FUNCTION expr4key( EXPR4 *e4expr, char **ptr_ptr )
{
   int result_len ;

   #ifdef S4DEBUG
      if ( e4expr == 0 || ptr_ptr == 0 )
         e4severe( e4parm, E4_EXPR4KEY ) ;
   #endif

   if ( e4expr->code_base->error_code < 0 )
      return -1 ;

   result_len = expr4vary( e4expr, ptr_ptr ) ;
   if ( result_len < 0 )
       return -1 ;

   return expr4key_convert( e4expr, ptr_ptr, result_len, e4expr->type ) ;
}

int S4FUNCTION expr4key_convert( EXPR4 *e4expr, char **ptr_ptr, int result_len, int expr_type )
{
   int i ;
   double  d ;
   CODE4 *cb ;
   #ifdef S4MDX
      C4BCD  bcd ;
   #endif
   #ifdef S4CLIPPER
      long l ;
   #endif
   #ifndef N4OTHER
      double *d_ptr ;
   #endif

   cb = e4expr->code_base ;

   switch( expr_type )
   {
      #ifdef S4FOX
         case r4num:
            d = c4atod( *ptr_ptr, result_len ) ;
            t4dbl_to_fox( cb->stored_key, d ) ;
            result_len = (int)sizeof( double ) ;
            break ;
         case r4date:
            d = (double) date4long( *ptr_ptr ) ;
            t4dbl_to_fox( cb->stored_key, d ) ;
            result_len = (int)sizeof( double ) ;
            break ;
         case r4num_doub:
         case r4date_doub:
            d_ptr = (double *) (*ptr_ptr) ;
            t4dbl_to_fox( cb->stored_key, *d_ptr ) ;
            result_len = (int)sizeof( double ) ;
            break ;
         case r4log:
            switch( *(int *)*ptr_ptr )
            {
               case 1:
                  cb->stored_key[0] = 'T' ;
                  break ;
               case 0:
                  cb->stored_key[0] = 'F' ;
                  break ;
               default:
                  #ifdef S4DEBUG
                     e4severe( e4info, E4_INFO_FAI ) ;
                  #endif
                  cb->stored_key[0] = 'F' ;
            }
            result_len = 1 ;
            break ;
      #endif  /*  ifdef S4FOX      */
      #ifdef S4CLIPPER
         case r4num:  
            result_len = e4expr->key_len ;
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, *ptr_ptr, result_len ) ;
            c4clip( cb->stored_key, result_len ) ;
            break ;
         case r4num_doub:
            result_len = e4expr->key_len ;
            c4dtoa_clipper( *((double *)*ptr_ptr), cb->stored_key, result_len, cb->decimals ) ;
            break ;
         case r4date_doub:
            d = *( ( double  *)*ptr_ptr ) ;
            l = (long)d ;
            date4assign( cb->stored_key, l ) ;
            break ;
      #endif  /*  ifdef S4CLIPPER  */
      #ifdef S4NDX
         case r4num:
            d = c4atod( *ptr_ptr, result_len ) ;
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, &d, sizeof( d ) ) ;
            result_len = (int)sizeof( double ) ;
            break ;
         case r4date:
            date4format_mdx2( *ptr_ptr, &d ) ;
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, &d, sizeof(double) ) ;
            result_len = (int)sizeof( double ) ;
            break ;
      #endif  /*  ifdef S4NDX  */
      #ifdef S4MDX
         case r4num:
            c4bcd_from_a( (char *) &bcd, (char *) *ptr_ptr, result_len ) ;
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, (void *)&bcd, sizeof(C4BCD) ) ;
            result_len = (int)sizeof( C4BCD ) ;
            break ;
         case r4num_doub:
            d_ptr = (double *) (*ptr_ptr) ;
            c4bcd_from_d( (char *) &bcd, *d_ptr ) ;
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, (void *)&bcd, sizeof(C4BCD) ) ;
            result_len = (int)sizeof( C4BCD ) ;
            break ;
         case r4date:
            date4format_mdx2( *ptr_ptr, &d ) ;
            if ( d == 0 ) d = 1.0E300 ;
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, (void *)&d, sizeof(double) ) ;
            result_len = (int)sizeof( double ) ;
            break ;
         case r4date_doub:
            #ifdef S4DEBUG
               if ( cb->stored_key == 0 )
                  e4severe( e4info, E4_PARM_NSD ) ;
            #endif
            memcpy( cb->stored_key, *ptr_ptr, sizeof(double) ) ;
            d_ptr = (double *)( cb->stored_key ) ;
            if ( *d_ptr == 0 ) *d_ptr = 1.0E300 ;
            result_len = (int)sizeof( double ) ;
            break ;
      #endif  /* S4MDX */
      case r4str:
         #ifdef S4DEBUG
            if ( cb->stored_key == 0 )
               e4severe( e4info, E4_PARM_NSD ) ;
         #endif
         memcpy( cb->stored_key, *ptr_ptr, result_len ) ;
         /* if trim, then replace nulls with blanks */
         if ( e4expr->has_trim )
            for( i = result_len - 1 ; (i >= 0) && (cb->stored_key[i] == 0) ; i-- )
               cb->stored_key[i] = ' ' ;
         break ;
      default:
         #ifdef S4DEBUG
            if ( cb->stored_key == 0 )
               e4severe( e4info, E4_PARM_NSD ) ;
         #endif
         memcpy( cb->stored_key, *ptr_ptr, result_len ) ;
         break ;
   }

   #ifdef S4DEBUG
      if ( (unsigned)result_len >= cb->stored_key_len )
         e4severe( e4info, E4_INFO_SKL ) ;
   #endif

   cb->stored_key[result_len] = 0 ;    /* null end it */
   *ptr_ptr = cb->stored_key ;
   return result_len ;
}

int S4FUNCTION expr4key_len( EXPR4 *e4expr )
{
   switch( e4expr->type )
   {
      #ifdef S4FOX
         case r4num:
            return (int)sizeof( double ) ;
         case r4date:
            return (int)sizeof( double ) ;
         case r4num_doub:
            return (int)sizeof( double ) ;
         case r4log:
            return (int)sizeof( char ) ;
      #endif  /*  ifdef S4FOX      */
      #ifdef S4CLIPPER
         case r4num:  /* numeric field return, this fixex length problem */
            return f4len( e4expr->info[0].field_ptr ) ;
         case r4num_doub:
            return e4expr->data->code_base->numeric_str_len ;
      #endif  /*  ifdef S4CLIPPER  */
      #ifdef S4NDX
         case r4num:
            return (int)sizeof( double ) ;
         case r4date:
            return (int)sizeof( double ) ;
      #endif  /*  ifdef S4NDX  */
      #ifdef S4MDX
         case r4num:
            return (int)sizeof( C4BCD ) ;
         case r4num_doub:
            return (int)sizeof( C4BCD ) ;
         case r4date:
            return (int)sizeof( double ) ;
         case r4date_doub:
            return (int)sizeof( double ) ;
      #endif  /* S4MDX */
      default:
         return expr4len( e4expr ) ;
   }
}

int S4FUNCTION expr4len( EXPR4 *e4expr )
{
   return e4expr->len ;
}

static char e4null_char = '\0' ;

char *S4FUNCTION expr4source( EXPR4 *e4expr )
{
   if ( e4expr == 0 )
      return &e4null_char ;
   return e4expr->source ;
}

int S4FUNCTION expr4true( EXPR4 *e4expr )
{
   int result_len ;
   int *i_ptr ;

   result_len = expr4vary( e4expr, (char **)&i_ptr ) ;
   if ( result_len < 0 )
      return -1 ;

   if ( expr4type( e4expr ) != r4log )
      return e4describe( e4expr->data->code_base, e4result, E4_EXPR4TRUE, E4_RESULT_EXP, (char *)0 ) ;

   return *i_ptr ;
}

int S4FUNCTION expr4type( EXPR4 *e4expr )
{
   return e4expr->type ;
}

int S4FUNCTION expr4vary( EXPR4 *expr, char **result_ptr_ptr )
{
   char *pointers[E4MAX_STACK_ENTRIES] ;
   int info_pos ;

   #ifdef S4DEBUG
      if ( expr == 0 || result_ptr_ptr == 0 )
         e4severe( e4parm, E4_EXPR4VARY ) ;
   #endif

   if ( expr->code_base->error_code < 0 )
      return -1 ;

   if ( expr4start( expr->code_base ) != 0 )
      return -1 ;

   expr4 = pointers ;
   expr4constants = expr->constants ;
   expr4ptr = expr ;

   for( info_pos = 0; info_pos < expr->info_n; info_pos++ )
   {
      expr4info_ptr = expr->info+ info_pos ;
      (*expr4info_ptr->function)() ;
   }

   *result_ptr_ptr = pointers[0] ;
   #ifdef S4DEBUG
      if ( pointers[0] != expr4[-1] )
         return e4( expr->code_base, e4result, 0 ) ;
   #endif

   expr4stop( expr->code_base ) ;

   return expr->len ;
}
