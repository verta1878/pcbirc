/* c4code.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
      #ifndef __DLL__
         #ifndef S4OS2PM
            extern unsigned _stklen ;
         #endif
      #endif
   #endif 

   #ifdef _MSC_VER
       #ifndef __DLL__
          #include <malloc.h>
       #endif
   #endif
#endif

char f4memo_null_char = '\0' ;
char  *expr4buf = 0 ;

#ifdef S4OS2
#ifdef S4OS2SEM
   #include <time.h>

   static char sem4mem[20], sem4expr[20] ;
   static HMTX hmtx4mem, hmtx4expr ;
#endif
#endif

#ifdef __DLL__
#ifdef S4OS2   
ULONG _dllmain (ULONG termflag, HMODULE modhandle)
{
   #ifdef S4OS2SEM
      int i ;
      APIRET rc ;
      time_t t ;
   #endif

   if ( termflag == 0 )
   {
      mem4init() ;
      #ifdef S4OS2SEM
         strcpy( sem4expr, "\\SEM32\\S4A" ) ;
         strcpy( sem4mem, "\\SEM32\\S4B" ) ;
         for ( i = 0 ; i < 100 ; i++ )
         {
            u4delay_sec() ;
            time( &t ) ;
            t %= 10000L ;

            c4ltoa45( t, sem4expr + 10, -4 ) ;
            c4ltoa45( t, sem4mem + 10, -4 ) ;

            rc = DosCreateMutexSem( sem4expr, &hmtx4expr, 0, 0 ) ;
            if ( rc != 0 )
               continue ;

            rc = DosCreateMutexSem( sem4mem, &hmtx4mem, 0, 0 ) ;
            if ( rc != 0 )
            {
               DosCloseMutexSem( hmtx4expr ) ;
               continue ;
            }
            return 1 ;
         }
      #else
         return 1 ;
      #endif
   }
   else
   {
      mem4reset() ;
      #ifdef S4OS2SEM
         DosCloseMutexSem( hmtx4mem ) ;
         DosCloseMutexSem( hmtx4expr ) ;
      #endif
   }

   return 1;
}
#endif
#ifdef S4WINDOWS
int far PASCAL LibMain( HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize, LPSTR lpCmdLine )
{
   mem4init() ;
   return 1 ;
}

#ifdef _MSC_VER
   #if  _MSC_VER != 800
      int far PASCAL _export WEP( int nParameter )
      {
         mem4reset() ;
         return 1 ;
      }
   #endif 
#endif 
#endif   /* S4WINDOWS */
#endif  /* __DLL__ */

void S4FUNCTION d4init( CODE4 *c4 )
{
   #ifdef S4OS2SEM
   #ifdef S4OS2
      #ifndef __DLL__
         time_t t ;
         int i ;
         APIRET rc ;
      #endif
   #endif
   #endif

   #ifdef S4DEBUG
      if ( c4 == 0 )
         e4severe( e4parm, E4_D4INIT ) ;
   #endif

   memset( (void *)c4, 0, sizeof( CODE4 ) ) ;

   /* if a DLL, can't check the stack length since this is a separate executable */
   #ifndef S4WINDOWS
      #ifndef __DLL__
         #ifdef __TURBOC__
            #ifndef S4OS2PM
               if ( _stklen < 5000U ) /* 5000 seems to be an appropriate minimum */
                  #ifdef S4DEBUG
                     e4severe( e4result, E4_RESULT_STC ) ;
                  #else
                     e4( c4, e4result, E4_RESULT_STC ) ;
                  #endif
               #endif
         #endif
      #endif
      #ifdef _MSC_VER  
         if ( stackavail() < 5000U )
            #ifdef S4DEBUG
               e4severe( e4result, E4_RESULT_STC ) ;
            #else
               e4( c4, e4result, E4_RESULT_STC ) ;
            #endif
      #endif
   #endif

   #ifdef S4DEBUG
      c4->debug_int = 0x5281 ; /* Some random value for double checking. */
   #endif  /* S4DEBUG */

   c4->mem_size_block   = 0x400 ;   /* 1024 */

   #ifdef S4VBASIC
      c4->debug_int = 0x5281 ;
   #endif  /* S4VBASIC */

   c4->mem_size_sort_pool   = 0xF000 ;  /* 61440 */
   c4->mem_size_sort_buffer = 0x1000 ;  /* 4096 */
   c4->mem_size_buffer      = 0x4000 ;  /* 16384 */
   c4->mem_size_memo        =  0x200 ;  /*   512 */
   c4->mem_size_memo_expr   =  0x400 ;  /*  1024 */

   c4->default_unique_error = r4unique_continue ;

   #ifndef S4LANGUAGE
      u4ncpy( c4->date_format, "MM/DD/YY", sizeof(c4->date_format) ) ;
   #else
      #ifdef S4GERMAN
         u4ncpy( c4->date_format, "DD.MM.YY", sizeof(c4->date_format) ) ;
      #endif
      #ifdef S4FRENCH
         u4ncpy( c4->date_format, "MM/DD/YY", sizeof(c4->date_format) ) ;
      #endif
      #ifdef S4SWEDISH
         u4ncpy( c4->date_format, "YYYY-MM-DD", sizeof(c4->date_format) ) ;
      #endif
      #ifdef S4FINISH
         u4ncpy( c4->date_format, "YYYY-MM-DD", sizeof(c4->date_format) ) ;
      #endif
      #ifdef S4NORWEGIAN
         u4ncpy( c4->date_format, "DD-MM-YYYY", sizeof(c4->date_format) ) ;
      #endif
   #endif

   #ifdef S4CLIPPER
      c4->numeric_str_len  = 17 ;    /* default length for clipper numeric keys is 10  */
      c4->decimals         = 2 ;
   #endif  /* S4CLIPPER */

   /* Flags initialization */
   c4->go_error = c4->open_error = c4->create_error = 
                  c4->tag_name_error = c4->auto_open = c4->field_name_error =
                  c4->safety = c4->skip_error = c4->expr_error = 1 ;
   c4->lock_attempts = -1 ;   /* wait forever */

   c4->mem_start_index = c4->mem_expand_index =
                         c4->mem_expand_data  = c4->mem_start_data = 5 ;
   c4->mem_start_block = c4->mem_expand_block =
                         c4->mem_start_tag    = c4->mem_expand_tag = 10 ;

   c4->relate_error = 1 ; 
   c4->do_index_verify = 1 ;

   #ifndef S4OPTIMIZE_OFF
      c4->do_opt = 1 ;   /* by default do optimization */
      c4->optimize = -1 ;   /* by default optimize non-shared files */
      c4->optimize_write = -1 ; 
      #ifdef S4OS2
         c4->mem_start_max = 0xF0000L ;
      #else
         #ifdef S4UNIX
            c4->mem_start_max = 0xF0000L ;
         #else
            #ifdef S4WINDOWS
               c4->mem_start_max = 0xF0000L ;
            #else
               c4->mem_start_max = 0x50000L ;
            #endif  /* S4WINDOWS */
         #endif
      #endif
   #endif  /* not S4OPTIMIZE_OFF */

   /* set up the semaphores */
#ifdef S4OS2SEM
   #ifdef S4OS2
      #ifndef __DLL__
         /* create new ones */
         strcpy( sem4expr, "\\SEM32\\S4A" ) ;
         strcpy( sem4mem, "\\SEM32\\S4B" ) ;
         for ( i = 0 ; i < 100 ; i++ )
         {
            u4delay_sec() ;
            time( &t ) ;
            t %= 10000L ;

            c4ltoa45( t, sem4expr + 10, -4 ) ;
            c4ltoa45( t, sem4mem + 10, -4 ) ;

            rc = DosCreateMutexSem( sem4expr, &hmtx4expr, 0, 0 ) ;
            if ( rc != 0 )
               continue ;

            rc = DosCreateMutexSem( sem4mem, &hmtx4mem, 0, 0 ) ;
            if ( rc != 0 )
            {
               DosCloseMutexSem( hmtx4expr ) ;
               continue ;
            }
            break ;
         }
      #endif
      /* make sure created and can open... assign to code_base pointer */
      if ( DosOpenMutexSem( sem4expr, &c4->hmtx_expr ) != 0 )
         e4( c4, e4info, "OS/2 Semaphore open failed" ) ;
      if ( DosOpenMutexSem( sem4mem, &c4->hmtx_mem ) != 0 )
         e4( c4, e4info, "OS/2 Semaphore open failed" ) ;
   #endif
#endif
}


int S4FUNCTION d4init_undo( CODE4 *c4 )
{
   #ifdef S4DEBUG
      if ( c4 == 0 )
         e4severe( e4parm, E4_D4INIT_UNDO ) ;
   #endif

   #ifdef S4OS2   
      #ifdef S4OS2SEM
         DosCloseMutexSem( hmtx4mem ) ;
         DosCloseMutexSem( hmtx4expr ) ;
      #endif
   #endif

   #ifndef S4OPTIMIZE_OFF
      d4opt_suspend( c4 ) ;
   #endif
   d4close_all( c4 ) ;

   mem4release( c4->index_memory ) ;
   c4->index_memory = 0 ;

   mem4release( c4->bitmap_memory ) ;
   c4->bitmap_memory = 0 ;

   mem4release( c4->data_memory ) ;
   c4->data_memory = 0 ;

   mem4release( c4->tag_memory ) ;
   c4->tag_memory = 0 ;

   mem4release( c4->bitmap_memory ) ;
   c4->bitmap_memory = 0 ;

   u4free( c4->expr_work_buf ) ;
   c4->expr_buf_len = 0 ;

   u4free( c4->field_buffer ) ;
   c4->field_buffer = 0 ;
   c4->buf_len = 0 ;

   u4free ( c4->stored_key ) ;
   c4->stored_key = 0 ;
   c4->stored_key_len = 0 ;

   return c4->error_code ;
}

int S4FUNCTION d4close_all( CODE4 *c4 )
{
   DATA4 *data_on, *data_next ;
   int rc ;

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E4_D4CLOSE_ALL ) )
         return -1 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( c4 == 0 )
         e4severe( e4parm, E4_D4CLOSE_ALL ) ;
   #endif

   rc = 0 ;

   for ( data_next = (DATA4 *)l4first( &c4->data_list ) ; ; )
   {
      data_on = data_next ;
      if ( !data_on )
         break ;
      data_next = (DATA4 *)l4next( &c4->data_list, data_next ) ;

      if ( d4close( data_on ) < 0 )
         rc = -1 ;
   }

   if ( c4->error_code < 0 )
      return -1 ;
   return rc ;
}

DATA4 *S4FUNCTION d4data( CODE4 *c4, char *alias_name )
{
   char buf[12] ;
   DATA4 *data_on ;
   #ifdef S4DEBUG
      DATA4 *data_result ;
   #endif

   #ifdef S4VBASIC
      if ( c4parm_check( c4, 1, E4_D4DATA ) )
         return 0 ;
   #endif  /* S4VBASIC */

   #ifdef S4DEBUG
      if ( c4 == 0 || alias_name == 0 )
         e4severe( e4parm, E4_D4DATA ) ;
   #endif

   u4ncpy( buf, alias_name, sizeof( buf ) ) ;
   c4upper( buf ) ;

   data_on = 0 ;
   #ifdef S4DEBUG
      data_result = 0 ;
   #endif

   for(;;)
   {
      data_on = (DATA4 *)l4next( &c4->data_list, data_on ) ;
      if ( !data_on )
         break ;

      if ( strcmp( buf, data_on->alias ) == 0 )
      {
         #ifdef S4DEBUG
            if ( data_result != 0 )
               e4severe( e4info, E4_INFO_DUP ) ;
            data_result = data_on ;
         #else
            return data_on ;
         #endif  /* S4DEBUG */
      }
   }

   #ifdef S4DEBUG
      return data_result ;
   #else
      return data_on ;
   #endif
}

#ifdef S4FOX
#define S4FORMAT 1
#endif

#ifdef S4CLIPPER
#ifdef S4FORMAT
#error Choose only one CodeBase index file compatibility option.
#endif
#define S4FORMAT 2
#endif

#ifdef S4MDX
#ifdef S4FORMAT
#error Choose only one CodeBase index file compatibility option.
#endif
#define S4FORMAT 4
#endif

#ifdef S4NDX
#ifdef S4FORMAT
#error Choose only one CodeBase index file compatibility option.
#endif
#define S4FORMAT 8
#endif

#ifndef S4FORMAT
#error You must define either S4FOX, S4CLIPPER, S4NDX or S4MDX
#endif

#ifdef S4DLL_BUILD
#define S4OPERATING 0x10
#endif

#ifdef S4WINDOWS
#undef S4OPERATING
#define S4OPERATING 0x20
#endif

#ifdef S4OS2
#ifdef S4OPERATING
#error Choose one of CodeBase switches S4DLL/S4WINDOWS, S4OS2, and S4CODE_SCREENS
#endif
#define S4OPERATING 0x40
#endif

#ifdef S4CODE_SCREENS
#ifdef S4OPERATING
#error Choose one of CodeBase switches S4DLL/S4WINDOWS, S4OS2, and S4CODE_SCREENS
#endif
#define S4OPERATING 0x80
#endif

#ifndef S4OPERATING
#define S4OPERATING 0
#endif

#ifdef S4DEBUG
#define S4DEBUG_VAL  0x100
#else
#define S4DEBUG_VAL  0
#endif

#ifdef S4ERROR_HOOK
#define S4ERROR_HOOK_VAL 0x200
#else
#define S4ERROR_HOOK_VAL 0
#endif

#ifdef S4LOCK_CHECK
#define S4LOCK_CHECK_VAL 0x400
#else
#define S4LOCK_CHECK_VAL 0
#endif

#ifdef S4LOCK_HOOK
#define S4LOCK_HOOK_VAL 0x800
#else
#define S4LOCK_HOOK_VAL 0
#endif

#ifdef S4MAX
#define S4MAX_VAL 0x1000
#else
#define S4MAX_VAL 0
#endif

#ifdef S4MEMO_OFF
#define S4MEMO_OFF_VAL 0x2000
#else
#define S4MEMO_OFF_VAL 0
#endif

#ifdef S4OLD_CODE
#define S4OLD_CODE_VAL 0x4000
#else
#define S4OLD_CODE_VAL 0
#endif

#ifdef S4OPTIMIZE_OFF
#define S4OPTIMIZE_OFF_VAL 0x8000
#else
#define S4OPTIMIZE_OFF_VAL 0
#endif

#ifdef S4PORTABLE
#define S4PORTABLE_VAL 0x10000
#else
#define S4PORTABLE_VAL 0
#endif

#ifdef S4SAFE 
#define S4SAFE_VAL 0x20000
#else
#define S4SAFE_VAL 0
#endif

#ifdef S4SINGLE 
#define S4SINGLE_VAL 0x40000
#else
#define S4SINGLE_VAL 0
#endif

#if S4VERSION != 5002
#error Your CodeBase source version does not match your header file version.
#endif

long S4FUNCTION u4switch()
{
   return (long) ( S4FORMAT + S4OPERATING + S4DEBUG_VAL + S4ERROR_HOOK_VAL +
          S4LOCK_CHECK_VAL + S4LOCK_HOOK_VAL + S4MAX_VAL +
          S4MEMO_OFF_VAL + S4OLD_CODE_VAL + S4OPTIMIZE_OFF_VAL +
          S4PORTABLE_VAL + S4SAFE_VAL + S4SINGLE_VAL ) ;
}

#ifdef S4VB_DOS

DATA4 *d4data_v( CODE4 *c4, char *alias )
{
   return d4data( c4, c4str(alias) ) ;
}

#endif
