/* r4reindex.h   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

typedef struct
{
   /* must be before n_keys  */
   #ifdef S4CLIPPER
      short    S4PTR *block_index ;
      char     S4PTR *data;
   #endif

   #ifdef S4FOX
      B4STD_HEADER header ;
   #else
      short     n_keys ;
   #endif

   #ifdef S4MDX
      char      dummy[6] ;
      char      info[2] ;
   #endif

   #ifdef S4NDX
      char      dummy[2] ;
      char      info;
   #endif
} R4BLOCK_DATA ;

typedef struct  /* R4REINDEX  For 'i4reindex' */
{
   #ifndef N4OTHER
      TAG4 S4PTR   *tag ;
      INDEX4 S4PTR *index ;
      int           n_tags ;
   #endif

   #ifdef S4FOX
      char      S4PTR *cur_pos ;
      B4NODE_HEADER node_hdr ;
      int        n_blocks_used ;
   #else
      #ifdef S4CLIPPER
         B4KEY_DATA S4PTR *stranded ;
         int     keys_half ;
         int     lastblock_inc ;        /* How much to increment 'lastblock' by */
         int     n_blocks_used ;
      #else
         int     lastblock_inc ;        /* How much to increment 'lastblock' by */
         unsigned blocklen ;
      #endif
   #endif

   int           grouplen ;             /* Entire key length */
   int           valuelen ;             /* Length of sorted portion of key */
   int           keysmax ;
   long          lastblock ;            /* The block last written to */
   long          key_count ;

   SORT4         sort ;

   FILE4SEQ_WRITE     seqwrite ;
   char S4PTR   *buffer ;      /* Buffer for 'seqwrite' and 'seqread' */
   int           buffer_len ;

   R4BLOCK_DATA S4PTR *start_block ;
   int           n_blocks ;
   int           min_keysmax;  /* To calculate the number of blocks */

   DATA4 S4PTR  *data ;
   CODE4 S4PTR  *code_base ;
} R4REINDEX ;

#ifdef __cplusplus
   extern "C" {
#endif

void r4reindex_free( R4REINDEX S4PTR * ) ;

#ifdef N4OTHER
   int r4reindex_tag_headers_calc( R4REINDEX S4PTR *, TAG4 S4PTR * ) ;
   int r4reindex_supply_keys( R4REINDEX S4PTR *, TAG4 S4PTR * ) ;
   int r4reindex_write_keys( R4REINDEX S4PTR *, TAG4 S4PTR * ) ;
   int r4reindex_tag_headers_write( R4REINDEX S4PTR *, TAG4 S4PTR * ) ;
   int r4reindex_init( R4REINDEX S4PTR *, TAG4 S4PTR * ) ;
#else
   int r4reindex_tag_headers_calc( R4REINDEX S4PTR * ) ;
   int r4reindex_supply_keys( R4REINDEX S4PTR * ) ;
   int r4reindex_write_keys( R4REINDEX S4PTR * ) ;
   int r4reindex_tag_headers_write( R4REINDEX S4PTR * ) ;
   int r4reindex_init( R4REINDEX S4PTR *, INDEX4 S4PTR * ) ;
#endif
int r4reindex_blocks_alloc( R4REINDEX S4PTR * ) ;
TAG4 S4PTR *r4reindex_find_i_tag( R4REINDEX S4PTR *, int ) ;

/* Write key functions */
#ifdef S4FOX
   int r4reindex_add( R4REINDEX S4PTR *, long, char S4PTR *, char S4PTR *, int S4PTR * ) ;
   int r4reindex_finish( R4REINDEX S4PTR *, char S4PTR * ) ;
   int r4reindex_todisk( R4REINDEX S4PTR *, char S4PTR * ) ;
#else
   int r4reindex_add( R4REINDEX S4PTR *, long, char S4PTR * ) ;
   int r4reindex_finish( R4REINDEX S4PTR * ) ;
   #ifdef S4CLIPPER
      int r4reindex_todisk( R4REINDEX S4PTR *, long, char S4PTR * ) ;
   #else
      int r4reindex_todisk( R4REINDEX S4PTR * ) ;
   #endif
#endif

#ifdef __cplusplus
   }
#endif
