/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#ifndef ___QINT_HPP___

#define ___QINT_HPP___

class qint
{

    public:

        qint ( void );

        qint ( int           i );
        qint ( unsigned int  u );
        qint ( long          l );
        qint ( unsigned long u );

        qint ( char * s, int radix = 10 );

        char * tostr   ( char * s, int radix = 10 );
        void   fromstr ( char * s, int radix = 10 );

        operator int           ( void );
        operator unsigned int  ( void );
        operator long          ( void );
        operator unsigned long ( void );

        qint operator + ( qint & tr );
        qint operator - ( qint & tr );
        qint operator * ( qint & tr );
        qint operator / ( qint & tr );
        qint operator % ( qint & tr );

        int  operator == ( qint & tr );
        int  operator <  ( qint & tr );
#ifdef ___FAST_RELATIONAL___
        int  operator != ( qint & tr );
        int  operator <= ( qint & tr );
        int  operator >  ( qint & tr );
        int  operator >= ( qint & tr );
#else
        int  operator != ( qint & tr ) { return (!(*this == tr)); }
        int  operator <= ( qint & tr ) { return ((*this < tr) || (*this == tr)); }
        int  operator >  ( qint & tr ) { return (!(*this <= tr)); }
        int  operator >= ( qint & tr ) { return (!(*this < tr)); }
#endif

        int  operator == ( int tr ) { return (*this == qint(tr)); }
        int  operator != ( int tr ) { return (*this != qint(tr)); }
        int  operator <  ( int tr ) { return (*this <  qint(tr)); }
        int  operator <= ( int tr ) { return (*this <= qint(tr)); }
        int  operator >  ( int tr ) { return (*this >  qint(tr)); }
        int  operator >= ( int tr ) { return (*this >= qint(tr)); }

        int  operator == ( long tr ) { return (*this == qint(tr)); }
        int  operator != ( long tr ) { return (*this != qint(tr)); }
        int  operator <  ( long tr ) { return (*this <  qint(tr)); }
        int  operator <= ( long tr ) { return (*this <= qint(tr)); }
        int  operator >  ( long tr ) { return (*this >  qint(tr)); }
        int  operator >= ( long tr ) { return (*this >= qint(tr)); }

        int  operator == ( unsigned long tr ) { return (*this == qint(tr)); }
        int  operator != ( unsigned long tr ) { return (*this != qint(tr)); }
        int  operator <  ( unsigned long tr ) { return (*this <  qint(tr)); }
        int  operator <= ( unsigned long tr ) { return (*this <= qint(tr)); }
        int  operator >  ( unsigned long tr ) { return (*this >  qint(tr)); }
        int  operator >= ( unsigned long tr ) { return (*this >= qint(tr)); }

        qint operator += ( qint & tr ) { *this = *this + tr; return *this; }
        qint operator -= ( qint & tr ) { *this = *this - tr; return *this; }
        qint operator *= ( qint & tr ) { *this = *this * tr; return *this; }
        qint operator /= ( qint & tr ) { *this = *this / tr; return *this; }
        qint operator %= ( qint & tr ) { *this = *this % tr; return *this; }

    protected:

        char qibuf [ 8 ];

    private:

        void divmod ( qint & l, qint & r, qint & m );

};

#endif

