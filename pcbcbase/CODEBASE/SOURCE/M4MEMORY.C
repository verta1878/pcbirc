/* m4memory.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#ifdef S4OS2
   #ifdef __DLL__
      #define  INCL_DOSMEMMGR
   #endif
#endif
#include "d4all.h"

#ifdef S4VB_DOS
   #include "malloc.h"
#endif

#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif
#endif

#ifdef S4OS2
   #ifdef __DLL__
      #include <bsememf.h>
   #endif
#endif

#ifdef S4MEM_PRINT
   int v4print = 0 ; /* if v4print == 0 then stdout, else stdprn */
#endif

#define mem4num_types 10

typedef struct
{
   LINK4  link ;
   MEM4  types[mem4num_types] ;
} MEMORY4GROUP ;

static LIST4  avail = { 0, 0, 0 } ;   /* A list of available MEM4 entries */
static LIST4  used = { 0, 0, 0 } ;   /* A list of used MEM4 entries */
static LIST4  groups = { 0, 0, 0 } ; /* A list of Allocated MEM4 groups */

#ifdef S4OS2
#ifdef S4OS2SEM
   int mem4start( CODE4 *c4 )
   {
      APIRET rc ;

      #ifdef S4DEBUG
         if ( c4 == 0 )
            return e4( c4, e4info, "OS/2 Semaphore Failure" ) ;
      #endif

      rc = DosRequestMutexSem( c4->hmtx_mem, -1 ) ;
      if ( rc != 0 )
         return e4( c4, e4info, "OS/2 Semaphore Failure" ) ;
      return 0 ;
   }

   void mem4stop( CODE4 *c4 )
   {
      #ifdef S4DEBUG
         if ( c4 == 0 )
         {
            e4( c4, e4info, "OS/2 Semaphore Failure" ) ;
            return ;
         }
      #endif

      DosReleaseMutexSem( c4->hmtx_mem ) ;
   }
#endif
#endif

#ifdef S4DEBUG

static char **mem4test_pointers ;
static int    mem4num_pointer = -1 ;
static int    mem4num_used = 0 ;

#ifdef S4UNIX
   #define mem4extra_chars 12
#else
   #define mem4extra_chars 10
#endif
#define mem4extra_tot   (mem4extra_chars*2 + sizeof(unsigned))
#define mem4check_char  0x55

/* Returns the pointer to be returned; is passed the pointer allocated by malloc ... */
static char *mem4fix_pointer( char *start_ptr, unsigned large_len )
{
   char *return_ptr ;            
   unsigned pos ;

   memset( start_ptr, mem4check_char, mem4extra_chars ) ;
   return_ptr = start_ptr + mem4extra_chars ;

   memcpy( return_ptr, (void *)&large_len, sizeof(large_len) ) ;
   pos = large_len - mem4extra_chars ;
   memset( start_ptr+ pos, mem4check_char, mem4extra_chars ) ;

   return return_ptr + sizeof(unsigned) ;
}

/* Returns the pointer allocated by malloc; */
/* passed by pointer returned by 'mem4fix_pointer' */
static char *mem4check_pointer( char *return_ptr, int clear )
{
   unsigned *large_len_ptr ;
   char *malloc_ptr, *test_ptr ;
   int i, j ;

   large_len_ptr = (unsigned *)(return_ptr - sizeof(unsigned)) ;
   malloc_ptr = return_ptr - sizeof(unsigned) - mem4extra_chars ;

   for ( j = 0; j < 2; j++ )
   {
      if (j == 0)
         test_ptr = malloc_ptr ;
      else
         test_ptr = malloc_ptr + *large_len_ptr - mem4extra_chars ;

      for ( i = 0 ; i < mem4extra_chars ; i++ )
         if ( test_ptr[i] != mem4check_char )
            e4severe( e4result, E4_RESULT_CMP ) ;
   }
   if ( clear == 1 ) /* null the memory to potentially detect re-use, including clearing check chars */
      memset( malloc_ptr, 0, *large_len_ptr ) ;
   return malloc_ptr ;
}

static void mem4push_pointer( char *ptr )
{
   #ifdef S4WINDOWS
      HANDLE  handle, *h_ptr, *old_h_ptr ;
      h_ptr = (HANDLE *)0 ;
   #endif

   if ( mem4num_pointer < 0 )
   {
      #ifdef S4WINDOWS
         #ifdef __DLL__
            handle = GlobalAlloc( GMEM_FIXED | GMEM_DDESHARE | GMEM_ZEROINIT, (DWORD) sizeof(char *) * 100 + sizeof(HANDLE) ) ;
         #else
            handle = GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, (DWORD) sizeof(char *) * 100 + sizeof(HANDLE) ) ;
         #endif

         if ( handle == (HANDLE) 0 )
            e4severe( e4memory, E4_MEMORY_YPU ) ;

         h_ptr = (HANDLE *)GlobalLock( handle ) ;
         *h_ptr++ = handle ;
         mem4test_pointers = (char **)h_ptr ;
      #else
         mem4test_pointers = (char **)malloc( sizeof(char *) * 100 ) ;
      #endif
      mem4num_pointer = 100 ;
   }
   if ( mem4num_pointer == mem4num_used )
   {
      mem4num_pointer += 100 ;
      if ( mem4num_pointer > 10000 )
         e4severe( e4result, E4_MEMORY_YPU ) ;

      #ifdef S4WINDOWS
         old_h_ptr = (HANDLE *)(mem4test_pointers) ;
         old_h_ptr-- ;  /* get the actual handle */

         #ifdef __DLL__
            handle = GlobalReAlloc( *old_h_ptr, (DWORD)sizeof(char *) * mem4num_pointer + sizeof( HANDLE ), GMEM_MOVEABLE ) ;
         #else
            handle = GlobalReAlloc( *old_h_ptr, (DWORD)sizeof(char *) * mem4num_pointer + sizeof( HANDLE ), GMEM_MOVEABLE ) ;
         #endif

         if ( handle == (HANDLE) 0 )
            e4severe( e4memory, E4_MEMORY_YPU ) ;

         h_ptr = (HANDLE *)GlobalLock( handle ) ;
         *h_ptr++ = handle ;
         mem4test_pointers = (char **)h_ptr ;
      #else
         mem4test_pointers = (char **)realloc( (void *)mem4test_pointers, sizeof(char *)*mem4num_pointer ) ;
      #endif
   }

   if ( mem4test_pointers == 0 )
      e4severe( e4memory, E4_MEMORY_YPU ) ;

   mem4test_pointers[mem4num_used++] = ptr ;
}

static void mem4pop_pointer( char *ptr )
{
   int i ;

   for ( i = mem4num_used - 1 ; i >= 0 ; i-- )
      if ( mem4test_pointers[i] == ptr )
      {
         /* This 'memmove' may create compile warning */
         memmove( mem4test_pointers+i, mem4test_pointers+i+1, (size_t) (sizeof(char *) * (mem4num_used-i-1))) ;
         mem4num_used-- ;
         return ;
      }
   e4severe( e4result, E4_MEMORY_YPO ) ;
}

void S4FUNCTION mem4check_memory()
{
   int i ;

   for ( i = 0; i < mem4num_used; i++ )
      mem4check_pointer( mem4test_pointers[i], 0 ) ;
}

int S4FUNCTION mem4free_check( int max_left )
{
   #ifdef S4MEM_PRINT
      int i ;
      if ( v4print )
         for ( i = 0; i < mem4num_used; i++ )
            fprintf( stdprn, "\r\nmem4free_check: %p", mem4test_pointers[i] ) ;
      else
         for ( i = 0; i < mem4num_used; i++ )
            printf( "\r\nmem4free_check: %p", mem4test_pointers[i] ) ;
   #endif

   if ( mem4num_used > max_left )
      e4severe( e4result, E4_RESULT_FRE ) ;

   return ( mem4num_used ) ;
}
#endif


void *S4FUNCTION mem4alloc( MEM4 *memory_type )
{
   #ifdef S4DEBUG
      if ( memory_type == 0 )
         e4severe( e4parm, E4_MEM4ALLOC ) ;
   #endif

   return mem4alloc2( memory_type, 0 ) ;
}

Y4CHUNK *S4FUNCTION mem4alloc_chunk( MEM4 *type_ptr )
{
   Y4CHUNK *chunk_ptr ;
   int  n_allocate, i ;
   char *ptr ;

   n_allocate = type_ptr->unit_expand ;
   if ( l4last( &type_ptr->chunks ) == 0 )
      n_allocate = type_ptr->unit_start ;

   chunk_ptr = (Y4CHUNK *)u4alloc_free( type_ptr->code_base, sizeof( LINK4 ) + (long)n_allocate*type_ptr->unit_size ) ;
   if ( chunk_ptr == 0 )
      return 0 ;
   ptr = (char *)&chunk_ptr->data ;
   for ( i = 0 ; i < n_allocate ; i++ )
      l4add( &type_ptr->pieces, (LINK4 *)( ptr + i * type_ptr->unit_size ) ) ;

   return  chunk_ptr ;
}

static void *mem4alloc_low( MEM4 *memory_type )
{
   LINK4 *next_piece ;
   Y4CHUNK *new_chunk ;
   #ifdef S4DEBUG
      char *ptr ;
   #endif
   #ifdef S4OS2SEM
      #ifdef S4OS2
         #ifdef __DLL__
            ULONG flags ;
         #endif
      #endif
   #endif

   if ( memory_type == 0 )
      return 0 ;
   next_piece = (LINK4 *)l4pop( &memory_type->pieces ) ;

   if ( next_piece != 0 )
   {
      #ifdef S4OS2
      #ifdef S4OS2SEM
         #ifdef __DLL__
            /* get access to the memory */
            flags = PAG_WRITE | PAG_READ  ;
            if ( DosGetSharedMem( next_piece, flags ) != 0 )
               return 0 ;
         #endif
      #endif
      #endif
      #ifdef S4DEBUG
         memory_type->n_used++ ;
         ptr = mem4fix_pointer( (char *)next_piece, memory_type->unit_size ) ;

         #ifdef S4MEM_PRINT
            if ( v4print )
               fprintf( stdprn, "\r\n  Y4ALLOC:  %p", ptr );
            else
               printf( "\r\n  Y4ALLOC:  %p", ptr);
         #endif

         mem4push_pointer( ptr ) ;
         return (void *)ptr ;
      #else
         return next_piece ;
      #endif
   }

   if ( (new_chunk = mem4alloc_chunk( memory_type )) == 0 )
      return 0 ;
   l4add( &memory_type->chunks, &new_chunk->link ) ;

   memory_type->n_used++ ;
   #ifdef S4DEBUG
      ptr = mem4fix_pointer( (char *)l4pop(&memory_type->pieces),
                            memory_type->unit_size ) ;
      #ifdef S4MEM_PRINT
         if ( v4print )
            fprintf(stdprn, "\r\n  Y4ALLOC:  %p", ptr);
         else
            printf( "\r\n  Y4ALLOC:  %p", ptr);
      #endif

      mem4push_pointer( ptr ) ;
      return (void *)ptr ;
   #else
      return l4pop( &memory_type->pieces ) ;
   #endif
}

void *S4FUNCTION mem4alloc2( MEM4 *memory_type, CODE4 *c4 )
{
   void *ptr ;

   if ( c4 )
      if ( c4->error_code < 0 )
         return 0 ;

   #ifdef S4OS2SEM
   #ifdef S4OS2
      if ( mem4start( memory_type->code_base ) != 0 )
         return 0 ;
   #endif
   #endif

   ptr = mem4alloc_low( memory_type ) ;

   #ifdef S4OS2SEM
   #ifdef S4OS2
      mem4stop( memory_type->code_base ) ;
   #endif
   #endif

   if ( ptr == 0 )
   {
      if ( c4 )
         e4set( c4, e4memory ) ;
      return 0 ;
   }
   #ifdef S4DEBUG
      memset( ptr, 0, memory_type->unit_size - mem4extra_tot ) ;
   #else
      memset( ptr, 0, memory_type->unit_size ) ;
   #endif
   return ptr ;
}

MEM4 *S4FUNCTION mem4create( CODE4 *c4, int start, unsigned unit_size, int expand, int is_temp )
{
   MEM4 *on_type ;
   #ifdef S4OS2SEM
   #ifdef S4OS2
      #ifdef __DLL__
         ULONG flags ;
      #endif
   #endif
   #endif

   #ifdef S4DEBUG
      if ( start < 0 || expand < 0 )
         e4severe( e4parm, E4_MEM4CREATE ) ;
      unit_size += 2 * mem4extra_chars + sizeof( unsigned ) ;
      #ifdef S4OS2
         if ( c4 == 0 )
            e4severe( e4info, "OS/2 mem4create() requires valid CodeBase pointer" ) ;
      #endif
   #endif

   if ( c4 )
      if ( c4->error_code < 0 )
         return 0 ;

   #ifdef S4OS2SEM
   #ifdef S4OS2
      if ( mem4start( c4 ) != 0 )
         return 0 ;
   #endif
   #endif

   if ( !is_temp )
      for( on_type = 0 ; ; )
      {
         on_type = (MEM4 *)l4next( &used, on_type ) ;
         if ( on_type == 0 )
            break ;

         #ifdef S4OS2
         #ifdef S4OS2SEM
            #ifdef __DLL__
               /* get access to the memory */
               flags = PAG_WRITE | PAG_READ  ;
               if ( DosGetSharedMem( on_type, flags ) != 0 )
                  return 0 ;
            #endif
         #endif
         #endif

         if ( on_type->unit_size == unit_size && on_type->n_repeat > 0 )
         {
            /* Match */
            if ( start > on_type->unit_start )
               on_type->unit_start = start ;
            if ( expand > on_type->unit_expand)
               on_type->unit_expand = expand ;
            on_type->n_repeat++ ;
            #ifdef S4OS2
            #ifdef S4OS2SEM
               mem4stop( c4 ) ;
            #endif
            #endif
            return on_type ;
         }
      }

   /* Allocate memory for another MEM4 */

   on_type = (MEM4 *)l4last( &avail ) ;
   if ( on_type == 0 )
   {
      MEMORY4GROUP *group ;
      int i ;

      group = (MEMORY4GROUP *)u4alloc_free( c4, sizeof( MEMORY4GROUP ) ) ;
      if ( group == 0 )
      {
         if ( c4 )
            e4set( c4, e4memory ) ;
         #ifdef S4OS2SEM
            #ifdef S4OS2
               mem4stop( c4 ) ;
            #endif
         #endif
         return 0 ;
      }

      for ( i = 0 ; i < mem4num_types ; i++ )
         l4add( &avail, group->types + i ) ;
      on_type = (MEM4 *)l4last( &avail ) ;
      l4add( &groups, group ) ;
   }

   l4remove( &avail, on_type ) ;
   memset( (void *)on_type, 0, sizeof( MEM4 ) ) ;
   l4add( &used, on_type ) ;

   #ifdef S4OS2SEM
   #ifdef S4OS2
      mem4stop( c4 ) ;
   #endif
   #endif

   on_type->unit_start = start ;
   on_type->unit_size = unit_size ;
   on_type->unit_expand= expand ;
   on_type->n_repeat = 1 ;
   on_type->n_used = 0 ;
   if ( is_temp )
      on_type->n_repeat = -1 ;

   #ifdef S4MEM_PRINT
      if ( v4print )
         fprintf( stdprn, "\r\n    MEM4: %p", on_type ) ;
      else
         printf( "\r\n    MEM4: %p", on_type ) ;
   #endif

   on_type->code_base = c4 ;
   return on_type ;
}

void *S4FUNCTION mem4create_alloc( CODE4 *c4, MEM4 **type_ptr_ptr, int start, unsigned unit_size, int expand, int is_temp)
{
   if ( *type_ptr_ptr == 0 )
   {
      *type_ptr_ptr = mem4create( c4, start, unit_size, expand, is_temp ) ;
      if ( *type_ptr_ptr == 0 )
         return 0 ;
   }

   return mem4alloc2( *type_ptr_ptr, c4 ) ;
}

void S4FUNCTION mem4free( MEM4 *memory_type, void *free_ptr )
{
   if ( memory_type == 0 || free_ptr == 0 )
      return ;

   memory_type->n_used-- ;
   #ifdef S4DEBUG
      if ( memory_type->n_used < 0 )
         e4severe( e4result, E4_MEM4FREE ) ;

      #ifdef S4MEM_PRINT
         if ( v4print )
            fprintf(stdprn, "\r\n  Y4FREE:  %p", free_ptr ) ;
         else
            printf( "\r\n  Y4FREE:  %p", free_ptr ) ;
      #endif

      mem4pop_pointer( (char *)free_ptr ) ;
      l4add( &memory_type->pieces, (LINK4 *)mem4check_pointer( (char *)free_ptr, 0 ) ) ;
      memset( (void *)&free_ptr, 0, sizeof(free_ptr) ) ;
   #else
      l4add( &memory_type->pieces, (LINK4 *)free_ptr ) ;
   #endif
}

void S4FUNCTION mem4release( MEM4 *memory_type )
{
   void *ptr ;

   if ( memory_type == 0 )
      return ;

   memory_type->n_repeat-- ;
   if ( memory_type->n_repeat <= 0 )
   {
      for(;;)
      {
         ptr = l4pop( &memory_type->chunks) ;
         if ( ptr == 0 )
            break ;
         u4free( ptr ) ;
      }

      l4remove( &used, memory_type ) ;
      l4add( &avail, memory_type ) ;

    #ifdef S4MEM_PRINT
         if ( v4print )
            fprintf( stdprn, "\r\n    Y4RELEASE: %p", memory_type ) ;
         else
            printf( "\r\n    Y4RELEASE: %p", memory_type ) ;
    #endif
   }
}

#ifdef S4OLD_CODE
MEM4 *S4FUNCTION mem4type( int start, unsigned unit_size, int expand, int is_temp)
{
   return mem4create( 0, start, unit_size, expand, is_temp ) ;
}
#endif

#ifdef S4MAX
   long  mem4max_memory = 0x4000 ;
   long  mem4allocated = 0L ;
   #ifndef S4DEBUG
      S4DEBUG should be set with S4MAX (force compile error.)
   #endif
#endif

void *S4FUNCTION u4alloc( long n )
{
   size_t s ;
   char *ptr ;
   #ifdef S4OS2SEM
   #ifdef __DLL__
      #ifdef S4OS2
         ULONG    flags;
         APIRET   rc;
      #endif
   #endif
   #endif

   #ifdef S4DEBUG
      if ( n == 0L )
         e4severe( e4parm, E4_PARM_ZER ) ;
      n += mem4extra_chars*2 + sizeof(unsigned) ;
   #endif

   #ifdef S4MAX
      /* Assumes 'mem4max_memory' is less than the actual maximum */
      if ( mem4allocated + n > mem4max_memory )
         return 0 ;
      mem4allocated += n ;
   #endif

   s = (size_t) n ;
   if ( n > (long) s )
      return 0 ;

   #ifdef S4WINDOWS
   {
      HANDLE  handle, *h_ptr ;
      h_ptr = (HANDLE *)0 ;

      #ifdef __DLL__
         handle = GlobalAlloc( GMEM_FIXED | GMEM_DDESHARE | GMEM_ZEROINIT, (DWORD) s+ sizeof(HANDLE) ) ;
      #else
         handle = GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, (DWORD) s+ sizeof(HANDLE) ) ;
      #endif

      if ( handle == (HANDLE) 0 )
         return 0 ;

      h_ptr = (HANDLE *)GlobalLock( handle ) ;
      *h_ptr++ = handle ;
      ptr = (char *)h_ptr ;
   }
   #else
      #ifdef __DLL__
         #ifdef S4OS2SEM
            #ifndef S4OS2
               #error invalid switch configuration
            #endif
               flags = PAG_WRITE | PAG_READ | OBJ_GETTABLE ;
               rc = DosAllocSharedMem( (void *)&ptr, 0, s, flags ) ;

               if (rc != 0)
                  return 0 ;
               flags = PAG_WRITE | PAG_READ ;
               rc = DosGetSharedMem( ptr, flags ) ;
               if ( rc != 0 )
                  return 0 ;
         #else
            ptr = (char *)malloc( s ) ;

            if ( ptr == 0 )
               return 0 ;
         #endif
      #else
         ptr = (char *)malloc( s ) ;

         if ( ptr == 0 )
            return 0 ;

                              /* Borland malloc of 64K (or close to) will */
         #ifdef __TURBOC__    /* result in corruption of segment memory due */
            if ( (ptr+s-1 <= ptr) && (s > 1 ))    /* to wrap-around problems  */
            {
               free( ptr ) ;
               return 0 ;
            }
         #endif

         #ifndef S4DEBUG
            memset( ptr, 0, s ) ;
         #endif
      #endif
   #endif

   #ifdef S4DEBUG
      ptr = mem4fix_pointer( ptr, s ) ;
      mem4push_pointer( ptr ) ;
      memset( ptr, 0, s-mem4extra_chars*2 - sizeof(unsigned) ) ;
   #endif

   #ifdef S4MEM_PRINT
      if ( v4print )
         fprintf( stdprn, "\r\nU4ALLOC:  %p   # bytes alloc: %ld", ptr, n );
      else
         printf("\r\nU4ALLOC:  %p   # bytes alloc: %ld", ptr, n);
   #endif

   return (void *)ptr ;
}

void *S4FUNCTION u4alloc_er( CODE4 S4PTR *c4, long n )
{
   void *ptr = u4alloc_free( c4, n ) ;
   if ( ptr == 0 && c4 )
      e4( c4, e4memory, 0 ) ;

   return ptr ;
}

void S4FUNCTION u4free( void *ptr )
{
   #ifdef S4WINDOWS
      HANDLE  hand ;
   #endif

   #ifdef S4MAX
      unsigned *amount ;
   #endif

   if ( ptr == 0 )
      return ;

   #ifdef S4MAX
      amount = (unsigned *)ptr ;
      mem4allocated -= amount[-1] ;
   #endif

   #ifdef S4WINDOWS
      #ifdef S4DEBUG
         mem4pop_pointer( (char *)ptr ) ;
         hand = ((HANDLE *)mem4check_pointer( (char *)ptr, 1 ))[-1] ;
      #else
         hand = ((HANDLE *)ptr)[-1] ;
      #endif

      GlobalUnlock( hand ) ;
      hand = GlobalFree( hand ) ;

      if ( hand != (HANDLE) 0 )
         e4severe( e4memory, E4_MEMORY_ERR ) ;
   #else
      #ifdef S4DEBUG
         mem4pop_pointer( (char *)ptr ) ;

         #ifdef S4MEM_PRINT
            if ( v4print )
               fprintf( stdprn, "\r\nU4FREE:  %p", ptr ) ;
            else
               printf( "\r\nU4FREE:  %p", ptr );
         #endif

         free(mem4check_pointer( (char *)ptr, 1 ) ) ;
      #else
         #ifdef S4MEM_PRINT
            if ( v4print )
               fprintf( stdprn, "\r\nU4FREE:  %p", ptr ) ;
            else
               printf( "\r\nU4FREE:  %p", ptr );
         #endif

         free( ptr ) ;
      #endif
   #endif
}

void S4FUNCTION mem4reset()
{
   MEM4 *on_type ;
   LINK4 *on_chunk, *on_group ;
   #ifdef S4WINDOWS
      HANDLE  hand ;
   #endif
   #ifdef S4LOCK_CHECK
      e4severe( e4result, E4_RESULT_S4L ) ;
   #endif

   for( on_type = 0 ;; )
   {
      on_type = (MEM4 *)l4next(&used,on_type) ;
      if ( on_type == 0 )
         break ;
      do
      {
         on_chunk = (LINK4 *)l4pop( &on_type->chunks) ;
         u4free( on_chunk ) ;  /* free of 0 still succeeds */
      } while ( on_chunk ) ;
   }

   for( ;; )
   {
      on_group = (LINK4 *)l4pop( &groups ) ;
      if ( on_group == 0 )
         break ;
      u4free( on_group ) ;
   }

   #ifdef S4DEBUG
      if ( mem4num_pointer > 0 )
      {
         #ifdef S4WINDOWS
            hand = ((HANDLE *)mem4test_pointers)[-1] ;

            GlobalUnlock( hand ) ;
            hand = GlobalFree( hand ) ;

            if ( hand != (HANDLE)0 )
               e4severe( e4memory, E4_MEMORY_ERR ) ;
         #else
            #ifdef S4MEM_PRINT
               if ( v4print )
                  fprintf( stdprn, "\r\nMEM4RESET:  %p", mem4test_pointers ) ;
               else
                  printf( "\r\nMEM4RESET:  %p", mem4test_pointers );
            #endif

            free( (void *)mem4test_pointers ) ;
         #endif

         mem4test_pointers = 0 ;
         mem4num_pointer = -1 ;
         mem4num_used = 0 ;
      }
   #endif

   mem4init() ;
}

void S4FUNCTION mem4init()
{
   memset( (void *)&avail, 0, sizeof( avail ) ) ;
   memset( (void *)&used, 0, sizeof( used ) ) ;
   memset( (void *)&groups, 0, sizeof( groups ) ) ;
}
