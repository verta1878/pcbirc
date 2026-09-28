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


/******************************************************************************/

// cVARVAL Member Functions (String Manipulation)

cVARVAL LIBENTRY cVARVAL::strip(cVARVAL & cv)
{
    return replace(cv,cVARVAL(vtSTRING,""));
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::stripatx(void)
{
    cVARVAL s(vtSTRING);
    s = *this;
    removecodes(nul2mty(s));
    return cVARVAL(s);
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::len(void)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL rVal(vtSTRING);

    rVal = *this;
    dVal = (signed long int) strlen(nul2mty(rVal));

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::lower(void)
{
    cVARVAL dVal(vtSTRING);

    dVal = *this;

    strlwr(nul2mty(dVal));

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::upper(void)
{
    cVARVAL dVal(vtSTRING);

    dVal = *this;

    strupr(nul2mty(dVal));

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::mid(cVARVAL & pos, cVARVAL & len)
{
    cVARVAL sVal(vtSTRING);
    cVARVAL pVal(vtINTEGER);
    cVARVAL lVal(vtINTEGER);

    sVal = *this;
    pVal = pos;
    lVal = len;

    if (lVal.values.vINTEGER < 0)   lVal.values.vINTEGER = 0;
    if (lVal.values.vINTEGER > 255) lVal.values.vINTEGER = 255;

    char tmpbuf[MAX_STR_LEN+1], * tmpptr = tmpbuf;
    memset(tmpbuf,0,MAX_STR_LEN+1);
    int curlen = 0;

    --pVal;

    while (curlen < lVal.values.vINTEGER)
    {
        if (((pVal.values.vINTEGER+curlen) >= 0) &&
            ((pVal.values.vINTEGER+curlen) < strlen(nul2mty(sVal))))
        {
            *(tmpptr++) = *(nul2mty(sVal)+
                (int)pVal.values.vINTEGER+curlen);
        }
        else
        {
            *(tmpptr++) = ' ';
        }
        ++curlen;
    }

    cVARVAL dVal(vtSTRING);
    dVal = tmpbuf;
    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::left(cVARVAL & len)
{
    cVARVAL sVal(vtSTRING);
    cVARVAL pVal(vtINTEGER);
    cVARVAL lVal(vtINTEGER);

    sVal = *this;
    lVal = len;
    pVal = (signed long int) 1;

    return sVal.mid(pVal,lVal);
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::right(cVARVAL & len)
{
    cVARVAL sVal(vtSTRING);
    cVARVAL pVal(vtINTEGER);
    cVARVAL lVal(vtINTEGER);

    sVal = *this;
    lVal = len;
    pVal = strlen(nul2mty(sVal))-lVal.values.vINTEGER+1;

    return sVal.mid(pVal,lVal);
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::space(void)
{
    cVARVAL dVal(vtSTRING);
    cVARVAL tVal(vtINTEGER);

    tVal = *this;
    if (tVal.values.vINTEGER < 0)
        tVal.values.vINTEGER = 0;
    else if (tVal.values.vINTEGER > MAX_STR_LEN)
        tVal.values.vINTEGER = MAX_STR_LEN;

    char buf[MAX_STR_LEN+1];
    memset(buf,' ',(int)tVal.values.vINTEGER);
    buf[(int)tVal.values.vINTEGER] = '\0';
    dVal = buf;

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::chr(void)
{
    cVARVAL dVal(vtSTRING);
    cVARVAL tVal(vtINTEGER);

    char buf[2];
    buf[1] = '\0';

    tVal = *this;
    if (tVal.values.vINTEGER < 0)
        tVal.values.vINTEGER = 0;
    else if (tVal.values.vINTEGER > 255)
        tVal.values.vINTEGER = 255;

    buf[0] = (char) tVal.values.vINTEGER;

    dVal = buf;

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::asc(void)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL tVal(vtSTRING);

    tVal = *this;
    dVal.values.vINTEGER = *nul2mty(tVal);

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::instr(cVARVAL & rvalue)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL lVal(vtSTRING);
    cVARVAL rVal(vtSTRING);

    rVal = rvalue;

    char * sub = nul2mty(rVal);
    int slen = strlen(sub);

    if (slen == 0)
    {
        dVal.values.vINTEGER = 0;
        return dVal;
    }

    lVal = *this;
    char * ptr = nul2mty(lVal);

    char * tmp = strstr(ptr,sub);
    dVal.values.vINTEGER = (tmp ? (tmp-ptr+1) : 0L);

    return dVal;
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::replace(cVARVAL & sv, cVARVAL & dv)
{
    cVARVAL r(vtSTRING);
    cVARVAL s(vtSTRING);
    cVARVAL d(vtSTRING);

    r = *this;
    s = sv;
    d = dv;

    char sc = *nul2mty(s);
    char dc = *nul2mty(d);

    for (int i = 0; nul2mty(r)[i] != '\0'; ++i)
    {
        if (nul2mty(r)[i] != sc) continue;
        if (dc == 0)
        {
            strcpy(nul2mty(r)+i,nul2mty(r)+i+1);
            --i;
            continue;
        }
        nul2mty(r)[i] = dc;
    }

    return cVARVAL(r);
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::ltrim(cVARVAL & rvalue)
{
    return trimit(*this,rvalue,stripleft);
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::rtrim(cVARVAL & rvalue)
{
    return trimit(*this,rvalue,stripright);
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::trim(cVARVAL & rvalue)
{
    return ltrim(rvalue).rtrim(rvalue);
}

/******************************************************************************/

// cVARVAL Member Functions (Time Functions)

cVARVAL LIBENTRY cVARVAL::timeap(void)
{
    cVARVAL t(vtTIME);
    t = *this;

    tINTEGER hr = t.hour().values.vINTEGER;
    char * ap = ((hr < 12) ? "AM" : "PM");

    hr %= 12;
    if (!hr) hr += 12;

    char buf[80+1];
    sprintf(buf,"%2ld:%02ld:%02ld %s",hr,t.min().values.vINTEGER,
        t.sec().values.vINTEGER,ap);

    return cVARVAL(vtSTRING,buf);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::hour(void)
{
    return dayPart(*this,24L*60L*60L,60L*60L);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::min(void)
{
    return dayPart(*this,60L*60L,60L);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::sec(void)
{
    return dayPart(*this,60L,1L);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::valTime(void)
{
    cVARVAL tmp(vtSTRING);
    tmp = *this;
    char * p = nul2mty(tmp);

    int flag = TRUE;

    char * endptr;
    long hour = strtol(p,&endptr,10);
    if ((*endptr != *Country.TimeSep) && (*endptr != ':'))
        flag = FALSE;

    long min = strtol(++endptr,&endptr,10);
    if ((*endptr != *Country.TimeSep) && (*endptr != ':') && (*endptr != '\0'))
        flag = FALSE;

    long sec = 0;

    if (flag && (*(endptr++) != '\0'))
    {
        sec = strtol(endptr,&endptr,10);
        if (*endptr != '\0') flag = FALSE;
    }

    if ((hour < 0) || (hour > 23)) flag = FALSE;
    if ((min  < 0) || (min  > 59)) flag = FALSE;
    if ((sec  < 0) || (sec  > 59)) flag = FALSE;

    return cVARVAL(vtBOOLEAN,flag);
}

/******************************************************************************/

#endif

/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/*
#ifndef ___COMP___

/****************************************************************************/

// Pragmas

/****************************************************************************/

// Included Files

#include    <stdio.h>
#include    <stdlib.h>

#include    <lrand.hpp>

#include    <pcbmisc.hpp>

#include    <var.hpp>

#ifdef DEBUGSCR
#include    <memcheck.h>
#endif

/****************************************************************************/

// Defined Macros

/****************************************************************************/

// Types

/****************************************************************************/

// Variables

/****************************************************************************/

// Function Prototypes

void LIBENTRY breakdate    ( unsigned int * a, char * p );

/****************************************************************************/

// Inline Functions

/****************************************************************************/

// Functions

    /*------------------------------------------------------------------*/

#if defined(LIB)
//&& !defined(___COMP___)

void LIBENTRY breakdate(unsigned int * a, char * p)
{
    char * r = juliantodate(datetojulian(p));
    a[0] = (unsigned int) atol(r+0);
    a[1] = (unsigned int) atol(r+3);
    a[2] = (unsigned int) atol(r+6);
}

#endif

    /*------------------------------------------------------------------*/

#if !defined(___EXEC___)

//int LIBENTRY isTime(char * s)
//{
//    return (isdigit(*(s++)) && isdigit(*(s++)) && (*(s++) == *Country.TimeSep) &&
//            isdigit(*(s++)) && isdigit(*(s++)) && (*(s++) == *Country.TimeSep) &&
//            isdigit(*(s++)) && isdigit(*(s++)) && (*s     == '\0'));
//}

    /*------------------------------------------------------------------*/

//int LIBENTRY isDate(char * s)
//{
//    return (isdigit(*(s++)) && isdigit(*(s++)) && (*(s++) == ';') &&
//            isdigit(*(s++)) && isdigit(*(s++)) && (*(s++) == ';') &&
//            isdigit(*(s++)) && isdigit(*(s++)) && (*s     == '\0'));
//}

#endif

    /*------------------------------------------------------------------*/

#if !defined(___COMP___)

    /*------------------------------------------------------------------*/

    /*------------------------------------------------------------------*/

int LIBENTRY cVAR::varIDMatch(const cVAR & v, const void * i)
{
    return (v.id == *(int *)i);
}

    /*------------------------------------------------------------------*/

#endif

    /*------------------------------------------------------------------*/

#if !defined(___COMP___) && !defined(___EXEC___)

#endif


#endif

/****************************************************************************/

#endif
*/
