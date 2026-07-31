#include "d4all.h"

int memcmp(const void * __s1, const void * __s2, size_t __n)
/*int memcmp(void * dest, const void * rc, size_t n)*/
{
   unsigned char *p1, *p2 ;
   int cnt ;

   p1 = (unsigned char *)__s1 ;
   p2 = (unsigned char *)__s2 ;

   for ( cnt = 0 ; cnt < __n ; cnt++ )
   {
      if ( p1[cnt] != p2[cnt] )
      {
         if ( p1[cnt] > p2[cnt] )
            return 1 ;
         return -1 ;
      }
   }
   return 0 ;
}

