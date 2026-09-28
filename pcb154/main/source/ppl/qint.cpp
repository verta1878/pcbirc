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


#include    <qint.hpp>

#include    <mem.h>

/****************************************************************************/

extern "C" void qwords_to_asc ( void );
extern "C" void asc_to_qwords ( void );
extern "C" void adds_qq       ( void );
extern "C" void subs_qq       ( void );
extern "C" void muls_qq       ( void );
extern "C" void divs_qq       ( void );
extern "C" void cmps_qq       ( void );

/****************************************************************************/

qint::qint(void)
{
    memset(qibuf,0,sizeof(qibuf));
}

    /*------------------------------------------------------------------*/

qint::qint(int i)
{
    memset(qibuf, ((i >= 0) ? 0x00 : 0xFF), sizeof(qibuf));
    memcpy(qibuf,&i,sizeof(i));
}

    /*------------------------------------------------------------------*/

qint::qint(unsigned int ui)
{
    memset(qibuf,0,sizeof(qibuf));
    memcpy(qibuf,&ui,sizeof(ui));
}

    /*------------------------------------------------------------------*/

qint::qint(long l)
{
    memset(qibuf, ((l >= 0) ? 0x00 : 0xFF), sizeof(qibuf));
    memcpy(qibuf,&l,sizeof(l));
}

    /*------------------------------------------------------------------*/

qint::qint(unsigned long ul)
{
    memset(qibuf,0,sizeof(qibuf));
    memcpy(qibuf,&ul,sizeof(ul));
}

    /*------------------------------------------------------------------*/

qint::qint(char * s, int radix)
{
    fromstr(s,radix);
}

/****************************************************************************/

char * qint::tostr(char * s, int radix)
{
    if (radix <  2) radix =  2;
    if (radix > 36) radix = 36;

    unsigned short int tds = (unsigned short int) (((unsigned long int) s)     >> 0x10);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) s)     &  0x0000FFFFUL);

    unsigned short int tes = (unsigned short int) (((unsigned long int) qibuf) >> 0x10);
    unsigned short int tdi = (unsigned short int) (((unsigned long int) qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push si
        push es
        push di
        push bx

        mov  bx,radix
        mov  ds,tds
        mov  si,tsi
        mov  es,tes
        mov  di,tdi
    }

    qwords_to_asc();

    asm {
        pop  bx
        pop  di
        pop  es
        pop  si
        pop  ds
    }

    return s;
}

    /*------------------------------------------------------------------*/

void qint::fromstr(char * s, int radix)
{
    if (radix <  2) radix =  2;
    if (radix > 36) radix = 36;

    unsigned short int tds = (unsigned short int) (((unsigned long int) s)     >> 0x10);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) s)     &  0x0000FFFFUL);

    unsigned short int tes = (unsigned short int) (((unsigned long int) qibuf) >> 0x10);
    unsigned short int tdi = (unsigned short int) (((unsigned long int) qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push si
        push es
        push di
        push bx

        mov  bx,radix
        mov  ds,tds
        mov  si,tsi
        mov  es,tes
        mov  di,tdi
    }

    asc_to_qwords();

    asm {
        pop  bx
        pop  di
        pop  es
        pop  si
        pop  ds
    }
}

    /*------------------------------------------------------------------*/

qint::operator int(void)
{
    int i;

    memcpy(&i,qibuf,sizeof(i));

    return i;
}

    /*------------------------------------------------------------------*/

qint::operator unsigned int(void)
{
    unsigned int ui;

    memcpy(&ui,qibuf,sizeof(ui));

    return ui;
}

    /*------------------------------------------------------------------*/

qint::operator long(void)
{
    long l;

    memcpy(&l,qibuf,sizeof(l));

    return l;
}

    /*------------------------------------------------------------------*/

qint::operator unsigned long(void)
{
    unsigned long ul;

    memcpy(&ul,qibuf,sizeof(ul));

    return ul;
}

/****************************************************************************/

qint qint::operator + (qint & tr)
{
    qint l = *this;
    qint r = tr;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    adds_qq();

    asm {
        pop  si
        pop  di
        pop  ds
    }

    return l;
}

    /*------------------------------------------------------------------*/

qint qint::operator - (qint & tr)
{
    qint l = *this;
    qint r = tr;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    subs_qq();

    asm {
        pop  si
        pop  di
        pop  ds
    }

    return l;
}

    /*------------------------------------------------------------------*/

qint qint::operator * (qint & tr)
{
    qint l = *this;
    qint r = tr;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tbx = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push bx

        mov  ds,tds
        mov  di,tdi
        mov  bx,tbx
    }

    muls_qq();

    asm {
        pop  bx
        pop  di
        pop  ds
    }

    return l;
}

    /*------------------------------------------------------------------*/

qint qint::operator / (qint & tr)
{
    qint l = *this;
    qint r = tr;
    qint m;

    divmod(l,r,m);

    return l;
}

    /*------------------------------------------------------------------*/

qint qint::operator % (qint & tr)
{
    qint l = *this;
    qint r = tr;
    qint m;

    divmod(l,r,m);

    return m;
}

/****************************************************************************/

int qint::operator == (qint & tr)
{
    qint l = *this;
    qint r = tr;

    int i;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    cmps_qq();

    asm {
        pop  si
        pop  di
        pop  ds
        je   true   // if condition is true then jump to true
    }

    i = 0;          // otherwise condition must be false
    goto exit;      // skip setting condition to true

#if !defined(__WATCOMC__) && (__BORLANDC__ < 0x500)
    // under BC31, this needs to be a 'C' label
true:
#else
    // under BC5 it needs to be an 'ASM' label
    asm true:
#endif
    i = 1;          // condition is true

exit:;
    return i;
}

    /*------------------------------------------------------------------*/

int qint::operator < (qint & tr)
{
    qint l = *this;
    qint r = tr;

    int i;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    cmps_qq();

    asm {
        pop  si
        pop  di
        pop  ds
        jl   true   // if condition is true then jump to true
    }

    i = 0;          // otherwise condition must be false
    goto exit;      // skip setting condition to true

#if !defined(__WATCOMC__) && (__BORLANDC__ < 0x500)
    // under BC31, this needs to be a 'C' label
true:
#else
    // under BC5 it needs to be an 'ASM' label
    asm true:
#endif
    i = 1;          // condition is true

exit:
    return i;
}

    /*------------------------------------------------------------------*/

#ifdef ___FAST_RELATIONAL___

int qint::operator != (qint & tr)
{
    qint l = *this;
    qint r = tr;

    int i;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    cmps_qq();

    asm {
        pop  si
        pop  di
        pop  ds
        jne  true   // if condition is true then jump to true
    }

    i = 0;          // otherwise condition must be false
    goto exit;      // skip setting condition to true

true:

    i = 1;          // condition is true

exit:

    return i;
}

#endif

    /*------------------------------------------------------------------*/

#ifdef ___FAST_RELATIONAL___

int qint::operator <= (qint & tr)
{
    qint l = *this;
    qint r = tr;

    int i;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    cmps_qq();

    asm {
        pop  si
        pop  di
        pop  ds
        jle  true   // if condition is true then jump to true
    }

    i = 0;          // otherwise condition must be false
    goto exit;      // skip setting condition to true

true:

    i = 1;          // condition is true

exit:

    return i;
}

#endif

    /*------------------------------------------------------------------*/

#ifdef ___FAST_RELATIONAL___

int qint::operator > (qint & tr)
{
    qint l = *this;
    qint r = tr;

    int i;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    cmps_qq();

    asm {
        pop  si
        pop  di
        pop  ds
        jg   true   // if condition is true then jump to true
    }

    i = 0;          // otherwise condition must be false
    goto exit;      // skip setting condition to true

true:

    i = 1;          // condition is true

exit:

    return i;
}

#endif

    /*------------------------------------------------------------------*/

#ifdef ___FAST_RELATIONAL___

int qint::operator >= (qint & tr)
{
    qint l = *this;
    qint r = tr;

    int i;

    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push si

        mov  ds,tds
        mov  di,tdi
        mov  si,tsi
    }

    cmps_qq();

    asm {
        pop  si
        pop  di
        pop  ds
        jge  true   // if condition is true then jump to true
    }

    i = 0;          // otherwise condition must be false
    goto exit;      // skip setting condition to true

true:

    i = 1;          // condition is true

exit:

    return i;
}

#endif

/****************************************************************************/

void qint::divmod(qint & l, qint & r, qint & m)
{
    unsigned short int tds = (unsigned short int) (((unsigned long int) l.qibuf) >> 0x10);

    unsigned short int tdi = (unsigned short int) (((unsigned long int) l.qibuf) &  0x0000FFFFUL);
    unsigned short int tbx = (unsigned short int) (((unsigned long int) r.qibuf) &  0x0000FFFFUL);
    unsigned short int tsi = (unsigned short int) (((unsigned long int) m.qibuf) &  0x0000FFFFUL);

    asm {
        push ds
        push di
        push bx
        push si

        mov  ds,tds
        mov  di,tdi
        mov  bx,tbx
        mov  si,tsi
    }

    divs_qq();

    asm {
        pop  si
        pop  bx
        pop  di
        pop  ds
    }
}

/****************************************************************************/

#ifdef ___TEST___

int main(void)
{
    qint qi;
    qi.fromstr("123456789012345",10);

    char str[64+1];
    qi.tostr(str,16);

    qi.fromstr("7FFFFFFFFFFFFFFF",16);
    qi.tostr(str,10);

    qi.fromstr("FFFFFFFFFFFFFFFF",16);
    qi.tostr(str,10);

    qi = 10;
    qi.tostr(str,10);
    qi = qi + qint(10);
    qi.tostr(str,10);
    qi = qi + qi;
    qi.tostr(str,10);

    qint a(100), b(50), c(25);

    qi = a = a-b-c;
    qi.tostr(str,10);

    qi = a = a*b*c;
    qi.tostr(str,10);

    qi = a = a*b*c*qint(3)/b/c;
    qi.tostr(str,10);

    qi = a = a*b*c%qint(3);
    qi.tostr(str,10);

    int i;

    i = (b == b);
    i = (b == c);

    i = (b != b);
    i = (b != c);

    i = (c < b);
    i = (b < c);

    i = (c > b);
    i = (b > c);

    i = (b < b);
    i = (c < c);

    i = (b > b);
    i = (c > c);

    i = (c <= b);
    i = (b <= c);

    i = (c >= b);
    i = (b >= c);

    i = (b <= b);
    i = (c <= c);

    i = (b >= b);
    i = (c >= c);

    return 0;
}

/****************************************************************************/

#endif
