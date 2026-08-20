/* d4data.hpp (c)Copyright Sequiter Software Inc., 1990-1993 All rights reserved. */

class S4CLASS Data4 ;
class S4CLASS Index4 ;
class S4CLASS Str4len ;

class S4CLASS Code4 : public CODE4
{
public:
   Code4() ;
   Code4( Code4& ) ;         /* Illegal operation, force link error */

   int     close_all()         { return d4close_all( this ) ; }
   Data4   data( char *alias ) ;
   int     error()             { return this->error_code ; }
   int     error_set( int er ) { return e4set( this, er ) ; }
   int     error( int er, char *p1 = 0, char *p2 = 0, char *p3 = 0 )
                               { return e4describe( this, er, p1,p2,p3 ) ; }
   void    exit()              {        e4exit( this ) ; }
   void    exit_test()         {        e4exit_test( this ) ; }
   int     flush_files()       { return d4flush_files( this ) ; }
   int     init_undo()         { return d4init_undo( this ) ; }
   int     opt_start()         { return d4opt_start( this ) ; }
   int     opt_suspend()       { return d4opt_suspend( this ) ; }
   int     unlock_files()      { return d4unlock_files( this ) ; }

   MEM4 *string_types[16] ;
} ;

class S4CLASS Data4
{
public:
   Data4()                           { data = 0 ; }
   Data4( Code4 *code, char *name ) { data = d4open( code, name ) ; }

   char * alias()                   { return d4alias( data ) ; }
   void   alias_set( char *ptr )    {        d4alias_set( data, ptr ) ; }
   int    append()                  { return d4append( data ) ; }
   int    append_data()             { return d4append_data( data ) ; }
   int    append_blank()            { return d4append_blank( data ) ; }
   int    append_start( int memo )  { return d4append_start( data, memo ) ; }
   void   blank()                   {        d4blank( data ) ; }
   int    bof()                     { return d4bof( data ) ; }
   int    bottom()                  { return d4bottom( data ) ; }
   int    changed( int flag )       { return d4changed( data, flag ) ; }
   int    check()                   { return d4check( data ) ; }
   int    close()            { int rc = d4close( data ) ; data = 0; return rc; }
   int    create( Code4 *cd, char *name, FIELD4INFO *f, TAG4INFO *t = 0 )
                                    { data = d4create( cd, name, f, t ) ;
                                      return cd->error_code ; }
   void   delete_rec()              {        d4delete( data ) ; }
   int    deleted()                 { return d4deleted( data ) ; }
   int    eof()                     { return d4eof( data ) ; }
   FIELD4INFO * field_info()        { return d4field_info( data ) ; }
   int    field_number( char *name) { return d4field_number( data, name) ; }
   int    flush()                   { return d4flush( data ) ; }
   int    flush_data()              { return d4flush_data( data ) ; }
   int    free_blocks()             { return d4free_blocks( data ) ; }
   int    go( long rec )            { return d4go( data, rec ) ; }
   int    go_data( long rec )       { return d4go_data( data, rec ) ; }
   int    go_eof()                  { return d4go_eof( data ) ; }
   Index4 index( char *name ) ;
   int    lock( long rec_num )      { return d4lock( data, rec_num ) ; }
   int    lock_all()                { return d4lock_all( data ) ; }
   int    lock_append()             { return d4lock_append( data ) ; }
   int    lock_file()               { return d4lock_file( data ) ; }
   int    lock_group(long *r, int n){ return d4lock_group( data, r, n ) ; }
   int    lock_index()              { return d4lock_index( data ) ; }
   int    lock_test( long rec )     { return d4lock_test( data, rec ) ; }
   int    lock_test_append()        { return d4lock_test_append( data ) ; }
   int    lock_test_file()          { return d4lock_test_file( data ) ; }
   int    lock_test_index()         { return d4lock_test_index( data ) ; }
   int    memo_compress()           { return d4memo_compress( data ) ; }
   int    num_fields()              { return d4num_fields( data ) ; }
   int    open( Code4 *code, char *name ) { data =  d4open( code, name ) ;
                                      return code->error_code ; }
   int    optimize( int opt_flag )  { return d4optimize( data, opt_flag ) ; }
   int    optimize_write( int opt_flag )  { return d4optimize_write( data,  opt_flag ) ; }
   int    pack()                    { return d4pack( data ) ; }
   int    pack_data()               { return d4pack_data( data ) ; }
   double position()                { return d4position( data ) ; }
   int    position_set( double d )  { return d4position_set( data, d ) ; }
   int    read( long rec,char *buf) { return d4read( data, rec, buf ) ; }
   int    read_old( long rec )      { return d4read_old( data, rec ) ; }
   void   recall()                  {        d4recall( data ) ; }
   long   reccount()                { return d4reccount( data ) ; }
   long   recno()                   { return d4recno( data ) ; }
   char  *record()                  { return d4record( data ) ; }
   long   record_position( long r ) { return d4record_position( data, r ) ; }
   long   record_width()            { return d4record_width( data ) ; }
   int    refresh()                 { return d4refresh( data ) ; }
   int    refresh_record()          { return d4refresh_record( data ) ; }
   int    reindex()                 { return d4reindex( data ) ; }
   int    seek( char *ptr )         { return d4seek( data, ptr ) ; }
   int    seek_double( double d )   { return d4seek_double( data, d ) ; }
   int    skip( long n = 1 )        { return d4skip( data, n ) ; }
   TAG4  *tag( char *name )         { return d4tag( data, name ) ; }
   TAG4  *tag_default()             { return d4tag_default( data ) ; }
   TAG4  *tag_next( TAG4 *tag )     { return d4tag_next( data, tag ) ; }
   TAG4  *tag_prev( TAG4 *tag )     { return d4tag_prev( data, tag ) ; }
   void   tag_select( TAG4 *tag )   {        d4tag_select( data, tag ) ; }
   TAG4  *tag_selected()            { return d4tag_selected( data ) ; }
   int    top()                     { return d4top( data ) ; }
   int    unlock()                  { return d4unlock( data ) ; }
   int    update_header(int t,int c){ return d4update_header( data, t, c ) ; }
   int    validate_memo_ids()       { return d4validate_memo_ids( data ) ; }
   int    write( long rec )         { return d4write( data, rec ) ; }
   int    write_data( long rec )    { return d4write_data( data, rec ) ; }
   int    write_keys( long rec )    { return d4write_keys( data, rec ) ; }
   int    zap( long f, long last)   { return d4zap( data, f, last ) ; }
   int    zap_data(long f,long last){ return d4zap_data( data, f, last ) ; }
   
   DATA4 *data ;
} ;

class S4CLASS Index4 
{
public:
   Index4()  { index = 0 ; }

   int    close()        { int rc =  i4close( index ) ; index = 0 ; return rc;}
   int    create( Data4& data, char *name, TAG4INFO *info ) 
                            { index = i4create( data.data, name, info ) ;
                              return data.data->code_base->error_code ; }
   int    lock()            { return i4lock( index ) ; }
   int    open( Data4& data, char *file )  { index = i4open( data.data, file ) ;
                              return data.data->code_base->error_code ; }
   int    reindex()         { return i4reindex( index ) ; }
   TAG4 *tag( char *name ) { return i4tag( index, name ) ; }
   int    unlock()          { return i4unlock( index ) ; }

   INDEX4 *index ;
} ;

class S4CLASS Str4len ;

class S4CLASS Str4
{
public:
   #ifdef __ZTC__
      virtual operator char() ;
      virtual operator double() ;
      virtual operator int() ;
   #else
      operator char() ;
      operator double() ;
      operator int() ;
   #endif
   virtual operator long() ;

   int  operator==( Str4& ) ;  /* Equal, including length */
   int  operator>=( Str4& ) ;
   int  operator<=( Str4& ) ;
   int  operator< ( Str4& ) ;
   int  operator> ( Str4& ) ;

   int    add( Str4& ) ;
   int    assign( char *) ;
   int    assign( char *ptr, unsigned ptr_len ) ;
   int    assign( Str4& from ) ;
   void   assign_double( double, int new_len = -1, int n_dec = -1 ) ;
   void   assign_long( long, int new_len = -1, int zeros_in_front = 0 ) ;
   char  *end_ptr() ;
   int    insert( Str4&, unsigned pos= 0 ) ;
   void   lower() ;
   unsigned ncpy( char *to_ptr, unsigned to_len ) ;
   int    replace( Str4&, unsigned pos= 0 ) ;
   void   set( int chr_value ) ;
   void   trim() ;
   void   upper() ;

   /* Get information */
   int    at( Str4& ) ;
   Str4len  left( unsigned l ) ;
   Str4len  right( unsigned l ) ;
   Str4len  substr( unsigned pos, unsigned len ) ;
   int    true() ;

   virtual void      changed()          {}
   virtual int       decimals()         { return 0 ; }
   virtual unsigned  len() ;
   virtual unsigned  maximum()          { return len() ; }
   virtual char     *ptr()  = 0 ;
   virtual char     *str() ;
   virtual int       set_len(unsigned)  { return -1 ; }
   virtual int       set_max(unsigned)  { return -1 ; }
} ;

class S4CLASS Str4char : public Str4
{
public:
   Str4char( char ch );
   char    *ptr()     { return &c ; }
   unsigned len()     { return 1 ; }

private:
   char c ;
} ;

class S4CLASS Str4ptr : public Str4
{
public:
   Str4ptr( char *ptr )   { p =   ptr ; }
   char *ptr()          { return p ; }
   char *str()          { return p ; }

   char *p ;
} ;

class S4CLASS Str4len : public Str4ptr
{
public:
   Str4len( void *ptr, unsigned ptr_len ) : Str4ptr((char *)ptr)
                   { cur_len =  ptr_len; }
   unsigned len()  { return cur_len ; }
   char *str()     { return Str4::str() ; }

   unsigned cur_len ;
} ;

class S4CLASS Str4max : public Str4len
{
public:
   Str4max( void *ptr, unsigned m ) : Str4len(ptr,m)  { max_len = m; }

   unsigned maximum() { return max_len ; }
   int      set_len( unsigned) ;

   unsigned max_len ;
} ;

/* The following classes  are always guaranteed to be null ended. */
class S4CLASS Str4ten : public Str4
{
public:
   Str4ten();
   Str4ten( char *p );

   unsigned maximum()      { return 10 ; }
   unsigned len()          { return cur_len; }
   char    *ptr()          { return buf ; }
   char    *str()          { return buf ; }
   int      set_len(unsigned new_len) ;

private:
   unsigned cur_len ;
   char buf[12] ;
} ;

class S4CLASS Str4large: public Str4
{
public:
   Str4large();
   Str4large( char *p );

   unsigned maximum()     { return 255 ; }
   unsigned len()     { return cur_len; }
   char    *ptr()     { return buf ; }
   char    *str()     { return buf ; }
   int      set_len(unsigned len) ;

private:
   unsigned cur_len ;
   char buf[256] ;
} ;

class S4CLASS Str4flex : public Str4max
{
public:
   Str4flex( Code4 * ) ;
   Str4flex( Str4flex& ) ;
  ~Str4flex() ;

   void *operator new( size_t ) ;
   void operator delete( void * ) ;

   void    free() ;
   int     set_max( unsigned ) ;
   char   *str()        { return ptr() ; }

   Code4 *code_base ;
} ;

class S4CLASS Field4 : public Str4
{
public:
   Field4()                       { field =  0 ; }
   Field4( Data4& d, int j )      { field =  d4field_j( d.data, j ) ; }
   Field4( Data4& d, char *name ) { field =  d4field( d.data, name ) ; }

   void          changed()   { field->data->record_changed = 1 ; }
   Data4         data() ;
   int           decimals()  { return f4decimals(field) ; }
   int           init( Data4&d, char *name )  
                             { field = d4field( d.data, name ) ; 
                               return (field == 0) ? -1 : 0 ; }
   int           init( Data4&d, int j )  
                             { field = d4field_j( d.data, j ) ; 
                               return (field == 0) ? -1 : 0 ; }
   unsigned      len()       { return f4len(field) ; }
   char         *name()      { return f4name(field) ; }
   int           type()      { return f4type(field) ; }
   char         *ptr()       { return f4ptr(field) ; }

   FIELD4    *field ;
} ;

class S4CLASS Field4memo : public Field4
{
public:
   Field4memo( Data4& data, int j ) ;
   Field4memo( Data4& data, char *name ) ;

   void changed() ;
   void free()       { f4memo_free(field) ; }
   unsigned len()    { return f4memo_len(field) ; }
   int  set_len(unsigned new_len)   { return f4memo_set_len(field,new_len) ; }

   char *ptr()       { return f4memo_ptr(field) ; }
   char *str()       { return f4memo_str(field) ; }
} ;

class S4CLASS  Date4 : public Str4
{
public:
   Date4();
   Date4( long l );
   Date4( char *p );

   operator long()       { return date4long(dt) ; }

   void   assign(long l) { date4assign(dt,l) ; dt[8] = 0 ; }
   char  *cdow()         { return date4cdow(dt) ; }
   char  *cmonth()       { return date4cmonth(dt) ; }
   int    day()          { return date4day(dt) ; }    /* Day of month  (1-31) */
   int    dow()          { return date4dow(dt) ; }    /* Day of week   (1-7) */
   void   format( char *result, char *pict ) { date4format( dt, result, pict ) ; }
   double format_mdx()   { return date4format_mdx(dt) ; }
   void   init( char *value, char *picture ) { date4init( dt, value, picture ) ; }
   long   julian_day()   { return date4long(dt) ; }
   int    month()        { return date4month(dt) ; }      /* Month of year (1-12) */
   char  *str()          { return dt ; }
   void   today()        { date4today(dt) ; }
   int    year()         { return date4year(dt) ; } ;

   unsigned  len()       { return 8 ; }
   char *ptr()           { return dt ; }

private:
   char dt[9] ;
} ;

class S4CLASS Relate4
{
public:
   Relate4() { relate = 0 ; }
   Relate4( Data4& data) { relate = relate4init( data.data ) ; }
   Relate4( Relate4& master, Data4& slave, char *m_expr, TAG4 *t )
                         { relate = relate4create_slave( master.relate, slave.data, m_expr, t ) ; }

   bottom()              { return relate4bottom( relate ) ; }
   void changed()        { relate4changed( relate ) ; }
   create_slave( Relate4& master, Data4& slave, char *m_expr, TAG4 *t )
			 { relate = relate4create_slave( master.relate, slave.data, m_expr, t ) ;
			   return slave.data->code_base->error_code ; }
   do_all()              { return relate4do( relate ) ; }
   do_one()              { return relate4do_one( relate ) ; }
   error_action( int a ) { return relate4error_action( relate, a ) ; }
   free( int p )         { return relate4free( relate, p ) ; }
   init( Data4& data )   { relate = relate4init( data.data ) ; return data.data->code_base->error_code ; }
   lock()                { return relate4lock( relate ) ; }
   int next()            { return relate4next( &relate ) ; }
   query_set( char *p )  { return relate4query_set( relate, p ) ; }
   skip( long l = 1)     { return relate4skip( relate, l ) ; }
   skip_enable( int do_enable ) { return relate4skip_enable( relate, do_enable ) ; }
   sort_set( char *p )   { return relate4sort_set( relate, p ) ; }
   top()                 { return relate4top( relate ) ; }
   type( int p )         { return relate4type( relate, p ) ; }
   unlock()              { return relate4unlock( relate ) ; }

   RELATE4 *relate ;
} ;
