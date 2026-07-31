 /* d4date.c   (c)Copyright Sequiter Software Inc., 1990-1993.  All rights reserved. */

#include "d4all.h"
#ifndef S4UNIX
   #ifdef __TURBOC__
      #pragma hdrstop
   #endif  /* __TUROBC__ */
#endif  /* S4UNIX */

#include <time.h>

#define  JULIAN_ADJUSTMENT    1721425L
#define  S4NULL_DATE          1.0E100   /* may not compile on some Op.Sys. */
                                        /* Set to MAXDOUBLE ?              */
static int  month_tot[]=
    { 0, 0,  31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 } ;
         /* Jan Feb Mar  Apr  May  Jun        Jul  Aug  Sep  Oct  Nov  Dec
             31  28  31   30   31   30         31   31   30        31   30   31
         */

typedef struct
{
   char cdow[12] ;
}  DOW ;

typedef struct
{
   char cmonth[10] ;
} MONTH ;

static DOW day_of_week[] =
#ifndef S4LANGUAGE
{
   { "\0         	" },
   { "Sunday\0   	" },
   { "Monday\0   	" },
   { "Tuesday\0  	" },
   { "Wednesday\0	" },
   { "Thursday\0 	" },
   { "Friday\0   	" },
   { "Saturday\0 	" },
} ;
#else
   #ifdef S4GERMAN
   {
      { "\0         	" },
      { "Sonntag\0  	" },
      { "Montag\0   	" },
      { "Dienstag\0 	" },
      { "Mittwoch\0 	" },
      { "Donnerstag\0" },
      { "Freitag\0  	" },
      { "Samstag\0  	" },
   } ;
   #endif
   #ifdef S4FRENCH
   {
      { "\0         	" },
      { "Dimanche\0 	" },
      { "Lundi\0    	" },
      { "Mardi\0    	" },
      { "Mercredi\0 	" },
      { "Jeudi\0    	" },
      { "Vendredi\0 	" },
      { "Samedi\0   	" },
   } ;
   #endif
   #ifdef S4SWEDISH
   {
      { "\0         	" },
      { "M†ndag\0   	" },
      { "Tisdag\0   	" },
      { "Onsdag\0   	" },
      { "Torsdag\0  	" },
      { "Fredag\0   	" },
      { "L”rdag\0   	" },
      { "S”ndag\0   	" },
   } ;
   #endif
   #ifdef S4FINISH
   {
      { "\0         	" },
      { "Maanantai\0	" },
      { "Tiistai\0  	" },
      { "Keskiviikko" },
      { "Torstai\0  	" },
      { "Perjantai\0	" },
      { "Lauantai\0 	" },
      { "Suununtai\0	" },
   } ;
   #endif
   #ifdef S4NORWEGIAN
   {
      { "\0         	" },
      { "Mandag\0	" },
      { "Tirsdag\0  	" },
      { "Onsdag\0       " },
      { "Torsdag\0  	" },
      { "Fredag\0	" },
      { "L”rdag\0 	" },
      { "S”ndag\0  	" },
   } ;
   #endif
#endif

static MONTH month_of_year[] =
#ifndef S4LANGUAGE
{
   { "\0        " },
   { "January\0 " },
   { "February\0" },
   { "March\0   " },
   { "April\0   " },
   { "May\0     " },
   { "June\0    " },
   { "July\0    " },
   { "August\0  " },
   { "September" },
   { "October\0 " },
   { "November\0" },
   { "December\0" },
} ;
#else
#ifdef S4GERMAN
{
   { "\0        " },
   { "Januar\0  " },
   { "Februar\0 " },
   { "M„rz\0    " },
   { "April\0   " },
   { "Mai\0     " },
   { "Juni\0    " },
   { "Juli\0    " },
   { "August\0  " },
   { "September" },
   { "Oktober\0 " },
   { "November\0" },
   { "Dezember\0" },
} ;
#endif
#ifdef S4FRENCH
{
   { "\0        " },
   { "Janvier\0 " },
   { "F‚vrier\0 " },
   { "Mars\0    " },
   { "Avril\0   " },
   { "Mai\0     " },
   { "Juin\0    " },
   { "Juillet\0 " },
   { "Ao–t\0    " },
   { "Septembre" },
   { "Octobre\0 " },
   { "Novembre\0" },
   { "D‚cembre\0" },
} ;
#endif
#ifdef S4SWEDISH
{
   { "\0        " },
   { "Januari\0 " },
   { "Februari\0" },
   { "Mars\0    " },
   { "April\0   " },
   { "Maj\0     " },
   { "Juni\0    " },
   { "Juli\0    " },
   { "Augusti\0 " },
   { "September" },
   { "Oktober\0 " },
   { "November\0" },
   { "December\0" },
} ;
#endif
#ifdef S4FINISH
{
   { "\0        " },
   { "Tammikuu\0" },
   { "Helmikuu\0" },
   { "Maaliskuu" },
   { "Huhtikuu\0" },
   { "Toukokuu\0" },
   { "Kes„kuu\0 " },
   { "Hein„kuu\0" },
   { "Elokuu\0  " },
   { "Syyskuu\0 " },
   { "Lokakuu\0 " },
   { "Marraskuu" },
   { "Joulukuu\0" },
} ;
#endif
#ifdef S4NORWEGIAN
{
   { "\0        " },
   { "Januar\0  " },
   { "Februar\0 " },
   { "Mars\0    " },
   { "April\0   " },
   { "Mai\0     " },
   { "Juni\0    " },
   { "Juli\0    " },
   { "August \0 " },
   { "September" },
   { "Oktober\0 " },
   { "November\0" },
   { "Desember\0" },
} ;
#endif
#endif

static int c4Julian( int year, int month, int day )
{
   /*  Returns */
   /*     >0   The day of the year starting from 1 */
   /*          Ex.    Jan 1, returns  1 */
   /*     -1   Illegal Date */
   int is_leap, month_days ;

   is_leap =  ( year%4 == 0 && year%100 != 0 || year%400 == 0 ) ?  1 : 0 ;

   month_days = month_tot[ month+1 ] -  month_tot[ month] ;
   if ( month == 2 )  month_days += is_leap ;

   if ( year  < 0  ||
        month < 1  ||  month > 12  ||
        day   < 1  ||  day   > month_days )
        return( -1 ) ;        /* Illegal Date */

   if ( month <= 2 )  is_leap = 0 ;

   return(  month_tot[month] + day + is_leap ) ;
}

static int  c4mon_dy( int year, int days,  int *month_ptr,  int *day_ptr )
{
   /*  Given the year and the day of the year, returns the month and day of month. */
   int is_leap, i ;

   is_leap = ( year % 4 == 0 && year % 100 != 0 || year % 400 == 0 ) ?  1 : 0 ;
   if ( days <= 59 )
      is_leap = 0 ;

   for( i = 2; i <= 13; i++)
   {
      if ( days <= month_tot[i] + is_leap )
      {
         *month_ptr = --i ;
         if ( i <= 2)
            is_leap = 0 ;

         *day_ptr = days - month_tot[ i] - is_leap ;
         return 0 ;
      }
   }
   *day_ptr = 0 ;
   *month_ptr = 0 ;

   return -1 ;
}

static long c4ytoj( int yr )
{
   /*  Calculates the number of days to the year */
   /*  This calculation takes into account the fact that */
   /*     1)  Years divisible by 400 are always leap years. */
   /*     2)  Years divisible by 100 but not 400 are not leap years. */
   /*     3)  Otherwise, years divisible by four are leap years. */
   /*  Since we do not want to consider the current year, we will */
   /*  subtract the year by 1 before doing the calculation. */
   yr-- ;
   return( yr*365L +  yr/4L - yr/100L + yr/400L ) ;
}

void S4FUNCTION date4assign( char *date_ptr, long ldate )
{
   /* Converts from a Julian day to the dbf file date format. */
   long tot_days ;
   int  i_temp, year, n_days, max_days, month, day ;

   if ( ldate <= 0 )
   {
      memset( date_ptr, ' ',  8 ) ;
      return ;
   }

   tot_days = ldate - JULIAN_ADJUSTMENT ;
   i_temp = (int)( (double)tot_days / 365.2425 ) ;
   year = i_temp + 1 ;
   n_days = (int)( tot_days - c4ytoj( year ) ) ;
   if ( n_days <= 0 )
   {
      year-- ;
      n_days = (int)( tot_days - c4ytoj( year ) ) ;
   }

   if ( ( year % 4 == 0 ) && ( year % 100 ) || ( year % 400 == 0 ) )
      max_days = 366 ;
   else
      max_days = 365 ;

   if ( n_days > max_days )
   {
      year++ ;
      n_days -= max_days ;
   }

   #ifdef S4DEBUG
      if ( c4mon_dy( year, n_days, &month, &day ) < 0 )
         e4severe( e4result, E4_DATE4ASSIGN ) ;
   #else
      c4mon_dy( year, n_days, &month, &day ) ;
   #endif

   c4ltoa45( (long)year, date_ptr, -4 ) ;
   c4ltoa45( (long)month, date_ptr + 4, -2 ) ;
   c4ltoa45( (long)day, date_ptr + 6, -2 ) ;
}

char *S4FUNCTION date4cdow( char *date_ptr )
{
   return day_of_week[date4dow(date_ptr)].cdow ;
}

char *S4FUNCTION date4cmonth( char *date_ptr )
{
   return month_of_year[date4month( date_ptr )].cmonth ;
}

int S4FUNCTION date4day( char *date_ptr )
{
   return (int)c4atol( date_ptr + 6, 2 ) ;
}

int S4FUNCTION date4dow( char *date_ptr )
{
   long date ;
   date = date4long(date_ptr) ;
   if ( date < 0 )
      return 0 ;
   return (int)( ( date + 1 ) % 7 ) + 1 ;
}

void S4FUNCTION date4format( char *date_ptr, char *result, char *picture )
{
   int result_len, rest, m_num, length ;
   char *ptr_end, *month_ptr, t_char ;

   result_len = strlen(picture) ;
   memset( result, ' ', result_len ) ;

   c4upper( picture ) ;
   c4encode( result, date_ptr, picture, "CCYYMMDD") ;

   ptr_end = strchr( picture, 'M' ) ;
   if ( ptr_end )
   {
      month_ptr = result+  (int)(ptr_end-picture) ;
      length = 0 ;
      while ( *(ptr_end++) == 'M' )
	 length++ ;

      if ( length > 2)
      {
	 /* Convert from a numeric form to character format for month */
	 if (!memcmp( date_ptr+4, "  ", 2 ))   /* if blank month */
	 {
	    memset( month_ptr, ' ', length ) ;
	    return ;
	 }

	 m_num = c4atoi( date_ptr+4, 2) ;

	 if ( m_num < 1)
	    m_num = 1 ;
	 if ( m_num > 12)
	    m_num = 12 ;

	 rest = length - 9 ;
	 if (length > 9)
	    length = 9 ;

	 memcpy( month_ptr, month_of_year[m_num].cmonth, (size_t) length ) ;
	 if (rest > 0)
	    memset( month_ptr+length, (int) ' ', (size_t) rest ) ;

	 t_char = month_of_year[m_num].cmonth[length] ;
	 if( t_char == '\0' || t_char == ' ' )
         {
	    m_num = strlen(month_of_year[m_num].cmonth) ;
            if ( m_num != length )
               month_ptr[m_num] = ' ' ;
         }
      }
   }
}

double S4FUNCTION date4format_mdx( char *date_ptr )
{
   long ldate ;
   ldate = date4long(date_ptr) ;
   if ( ldate == 0 )
      return (double) S4NULL_DATE ;  /* Blank or Null date */
   return (double)ldate ;
}

int S4FUNCTION date4format_mdx2( char *date_ptr, double *doub_ptr )
{
   long ldate ;

   ldate = date4long(date_ptr) ;
   if ( ldate == 0 )
      *doub_ptr = (double)S4NULL_DATE ;  /* Blank or Null date */
   else
      *doub_ptr = (double)ldate ;
   return 0 ;
}

void S4FUNCTION date4init( char *date_ptr, char *date_data, char *picture )
{
   char *month_start, month_data[10], buf[2] ;
   int year_count, month_count, day_count, century_count, i, length ;

   day_count = 5 ;
   month_count = 3 ;
   year_count = 1 ;
   century_count= -1 ;

   memset( date_ptr, ' ', 8 ) ;

   c4upper( picture ) ;
   for ( i=0; picture[i] != '\0'; i++ )
   {
      switch( picture[i] )
      {
         case 'D':
            if ( ++day_count >= 8 )
               break ;
            date_ptr[day_count] = date_data[i] ;
            break ;
         case 'M':
            if ( ++month_count >=6 )
               break ;
            date_ptr[month_count] = date_data[i] ;
            break ;
         case 'Y':
            if ( ++year_count >= 4 )
               break ;
            date_ptr[year_count] = date_data[i] ;
            break ;
         case 'C':
            if ( ++century_count >= 2 )
               break ;
            date_ptr[century_count] = date_data[i] ;
            break ;
         default:
            break ;
      }
   }

   if ( strcmp( date_ptr, "        " ) == 0 )
      return ;

   if ( century_count == -1 )
      memcpy( date_ptr, "19", (size_t)2 ) ;
   if ( year_count ==  1 )
      memcpy( date_ptr + 2, "01", (size_t)2 ) ;
   if ( month_count == 3 )
      memcpy( date_ptr + 4, "01", (size_t)2 ) ;
   if ( day_count == 5 )
      memcpy( date_ptr + 6, "01", (size_t)2 ) ;

   if ( month_count >= 6 )
   {
      /* Convert the Month from Character Form to Date Format */
      month_start = strchr( picture, 'M' ) ;

      length = month_count - 3 ;        /* Number of 'M' characters in picture */

      memcpy( date_ptr+4, "  ", (size_t)2 ) ;

      if ( length > 3 )
         length = 3 ;
      memcpy( month_data, date_data + (int)( month_start - picture ), (size_t)length) ;
      while ( length > 0 )
         if ( month_data[length-1] == ' ' )
            length-- ;
         else
            break ;

      month_data[length] = '\0' ;

      c4lower( month_data ) ;
      buf[0] = month_data[0] ;
      buf[1] = 0 ;
      c4upper(buf) ;
      month_data[0] = buf[0] ;

      if ( length > 0 )
         for( i = 1 ; i <= 12; i++ )
         {
            if ( memcmp( month_of_year[i].cmonth, month_data, (size_t) length) == 0 )
            {
               c4ltoa45( (long) i, date_ptr+4, 2 ) ;  /* Found Month Match */
               break ;
            }
         }
   }

   for ( i = 0 ; i < 8 ; i++ )
      if ( date_ptr[i] == ' ' )
         date_ptr[i] = '0' ;
}

long S4FUNCTION date4long( char *date_ptr )
{
   /*  Returns: */
   /*    >0  -  Julian day */
   /*           That is the number of days since the date  Jan 1, 4713 BC */
   /*           Ex.  Jan 1, 1981 is  2444606 */
   /*     0  -  NULL Date (dbf_date is all blank) */
   /*    -1  -  Illegal Date */
   int  year, month, day, day_year ;

   year = c4atoi( date_ptr, 4 ) ;
   if ( year == 0)
      if ( memcmp( date_ptr, "        ", 8 ) == 0)
         return  0 ;

   month = c4atoi( date_ptr + 4, 2 ) ;
   day = c4atoi( date_ptr + 6, 2 ) ;
   day_year = c4Julian( year, month, day ) ;
   if ( day_year < 1 )    /* Illegal Date */
      return -1L ;

   return ( c4ytoj( year ) + day_year + JULIAN_ADJUSTMENT ) ;
}

int S4FUNCTION date4month( char *date_ptr )
{
   return (int)c4atol( date_ptr + 4, 2 ) ;
}

void S4FUNCTION date4time_now( char *time_data )
{
   long time_val ;
   struct tm *tm_ptr ;

   time( (time_t *)&time_val) ;
   tm_ptr = localtime( (time_t *)&time_val) ;

   c4ltoa45( (long)tm_ptr->tm_hour, time_data, -2) ;
   time_data[2] = ':' ;
   c4ltoa45( (long)tm_ptr->tm_min, time_data + 3, -2) ;
   time_data[5] = ':' ;
   c4ltoa45( (long)tm_ptr->tm_sec, time_data + 6, -2) ;
}

void S4FUNCTION date4today( char *date_ptr )
{
   long time_val ;
   struct tm *tm_ptr ;

   time( (time_t *)&time_val ) ;
   tm_ptr = localtime( (time_t *)&time_val ) ;

   c4ltoa45( 1900L + tm_ptr->tm_year, date_ptr, -4 ) ;
   c4ltoa45( (long)tm_ptr->tm_mon + 1, date_ptr + 4, -2 ) ;
   c4ltoa45( (long)tm_ptr->tm_mday, date_ptr + 6, -2 ) ;
}

int S4FUNCTION date4year( char *year_ptr )
{
   return (int)c4atol( year_ptr, 4 ) ;
}


#ifdef S4VB_DOS


/* DATE Functions */   

void date4assign_v( char *date, long julian_day )
{
  date4assign( c4buf, julian_day ) ;
  c4buf[8] = '\0' ;           
  u4ctov( date, c4buf );
}

char * date4cdow_v( char *date )
{
   return v4str( date4cdow( c4str(date) ) );
}

char * date4cmonth_v( char *date )
{
   return v4str( date4cmonth( c4str(date) ) ) ;
}

int date4day_v( char *date )
{
  return date4day( c4str(date) ) ;
}

int date4dow_v( char *date )
{
  return date4dow( c4str(date) ) ;
}

void date4format_v( char *v_date, char *v_res, char *v_pic )
{
 char c_date[9],c_res[255],c_pic[255];
     
 u4vtoc( c_date, sizeof(c_date), v_date );
 u4vtoc( c_pic, sizeof(c_pic), v_pic );
 
 date4format( c_date, c_res, c_pic ) ;
 u4ctov( v_res, c_res );

}

double date4formatMdx ( char *date )
{
 return date4format_mdx( c4str(date) ) ;
}

void date4init_v( char *v_res, char *v_date, char *v_pic )
{
 char c_res[9],c_date[255],c_pic[255];
     
 u4vtoc( c_date, sizeof(c_date), v_date );
 u4vtoc( c_pic, sizeof(c_pic), v_pic );
 
 date4init( c_res, c_date, c_pic ) ;

 c_res[8] = '\0' ;

 u4ctov( v_res, c_res ) ;
}

long date4long_v( char *date )
{
 return date4long( c4str(date) ) ;
}

int date4month_v( char *date )
{
 return date4month( c4str(date) ) ;
}

void date4today_v( char *v_date )
{
 date4today( c4buf ) ;
 u4ctov( v_date, c4buf ) ;
}

int date4year_v( char *date )
{
 return date4year( c4str(date) ) ;
}

#endif /* S4VB_DOS */
