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
/*                                                                            */
/*                                  VAR.CPP                                   */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                   Data and functions for the cVAR class.                   */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                Copyright (C) 1993 Clark Development Company                */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Included Files

#ifdef __BORLANDC__
  #include  <alloc.h>
#else
  #include  <malloc.h>
#endif

#include    <ctype.h>
#include    <math.h>
#include    <stdio.h>
#include    <stdlib.h>

#include    "var.hpp"

#include    "lrand.hpp"
#include    "pcbmisc.hpp"
#include    "scrmisc.hpp"
#include    "bug.h"

#if defined(DBASE) && !defined(___COMP___)
#include    "dbase.hpp"
//#include    "d4all.h"
#endif

#ifdef DEBUGSCR
#include    <memcheck.h>
#endif

/******************************************************************************/

// Types

#define defclass(t) class cVV##t:public cVARVAL{public:cVV##t(void):cVARVAL(vt##t){}};

defclass(BOOLEAN);              //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(UNSIGNED);             //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(DATE);                 //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(EDATE);                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(INTEGER);              //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(MONEY);                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(REAL);                 //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(STRING)                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(TIME);                 //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(BYTE);                 //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(WORD);                 //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(SBYTE);                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(SWORD);                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(BIGSTR);               //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(DREAL);                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(FUNCTION);             //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(PROCEDURE);            //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.
defclass(DDATE);                //lint !e19      Needed for arrays of cVARVAL objects. Lint says useless declaration.

/******************************************************************************/
// For some reason, when compiling with BC 5.0, this module compiles correctly
// (even without the lines below) but at link time, the linker tells us that
// cLIST<int>::add(int) and cLIST<cLABEL far *>::add(cLABEL far *) do not
// exist.  Well...  as best I can tell, they shouldn't need to exist!
//
// However, to please the BC5 compiler, I've added the lines below which will
// be compiled only under 5.0 which will cause those functions to be defined
// even though they will never be used!

#if __BORLANDC__ >= 0x500
cLIST<int>      Dummy1;
cLIST<cLABEL *> Dummy2;
#endif
/******************************************************************************/

// cVARVAL Member Functions

// *** NOTE!!! *** THIS CONSTANT MUST BE A POWER OF TWO!!! ***
const int maxFastVarVals = 16;

struct sFastVarVals {

    sFastVarVals ( void ) : inUse(false), var(vtBOOLEAN) { }
    ~sFastVarVals ( void ) { var.type = vtBOOLEAN; }

    bool    inUse;
    cVARVAL var;

} fastVarVals [ maxFastVarVals ];

#undef FAST_STATS
#ifdef FAST_STATS
long fastNewHit  = 0;
long fastNewMiss = 0;
long fastDelHit  = 0;
long fastDelMiss = 0;
#endif

// bool cVARVAL::fastNewDel = false;
int cVARVAL::fastNewDelPtr = 0;

void * cVARVAL::operator new ( size_t size )
{
    for ( int i = 0; i < maxFastVarVals; ++i,
        // *** NOTE!!! ***
        // *** THIS ONLY WORKS IF maxFastVarVals IS A POWER OF TWO!!! ***
        ++fastNewDelPtr, fastNewDelPtr &= (maxFastVarVals-1) )
    {
        if (!fastVarVals[fastNewDelPtr].inUse)
        {
           #ifdef FAST_STATS
            ++fastNewHit;
           #endif
            fastVarVals[fastNewDelPtr].inUse = true;
            return &fastVarVals[fastNewDelPtr].var;
        }
    }

   #ifdef FAST_STATS
    ++fastNewMiss;
   #endif

    return ::new char[size];

////if (fastNewDel && (size == sizeof(cVARVAL)))
////{
//      for ( int i = 0; i < maxFastVarVals; ++i )
//          if (!fastVarVals[i].inUse)
//          {
//             #ifdef FAST_STATS
//              ++fastNewHit;
//             #endif
//              fastVarVals[i].inUse = true;
//              return &fastVarVals[i].var;
//          }
//     #ifdef FAST_STATS
//      ++fastNewMiss;
//     #endif
////}
//
//  return ::new char[size];
}

void cVARVAL::operator delete ( void * ptr )
{
    // *** NOTE!!! ***
    // *** THIS CODE ASSUMES NORMALIZED POINTERS PASSED IN!!! ***
    if (((unsigned long) ptr >= (unsigned long) &fastVarVals[0].var) &&
        ((unsigned long) ptr <= (unsigned long) &fastVarVals[maxFastVarVals-1].var))
    {
       #ifdef FAST_STATS
        ++fastDelHit;
       #endif
        fastVarVals[(size_t) (((sFastVarVals*) ptr) - ((sFastVarVals*) &fastVarVals[0].var))].inUse = false;
        return;
    }

   #ifdef FAST_STATS
 // if (fastNewDel)
        ++fastDelMiss;
   #endif
    ::delete ptr;

//  for ( int i = 0; i < maxFastVarVals; ++i )
//      if (ptr == &fastVarVals[i].var)
//      {
//         #ifdef FAST_STATS
//          ++fastDelHit;
//         #endif
//          fastVarVals[i].inUse = false;
//          return;
//      }
//
// #ifdef FAST_STATS
////if (fastNewDel)
//      ++fastDelMiss;
// #endif
//  ::delete ptr;
}

    /*--------------------------------------------------------------------*/

cVARVAL::cVARVAL(int _type) :
    type(_type)
{
    initialize(); // Set the default value
}

    /*--------------------------------------------------------------------*/

cVARVAL::~cVARVAL(void)
{
    switch (type)
    {
        case vtSTRING:                      // If it is a STRING
            if (values.vSTRING)             // And it has a value
                free(values.vSTRING);       // Free the value  Used to be bfree
            values.vSTRING = NULL;
            break;

        case vtBIGSTR:                      // If it is a BIGSTR
            if (values.vBIGSTR.ptr)         // And it has a value
                 free(values.vBIGSTR.ptr);  // Free the value  Used to be bfree
            values.vBIGSTR.ptr = NULL;
            break;
    }
}

    /*--------------------------------------------------------------------*/

void LIBENTRY cVARVAL::initialize(void)
{
#ifdef ___COMP___
    memset(&values,0,sizeof(values));
#else
    values.vSTDVARSIZE.lo = values.vSTDVARSIZE.hi = 0;
#endif

    switch (type)
    {
//      case vtBOOLEAN:  values.vBOOLEAN = FALSE;                   break;
//      case vtUNSIGNED: values.vUNSIGNED = 0;                      break;
//      case vtDATE:   //values.vDATE = datetojulian("01-01-00");   break;
//      case vtEDATE:    str2ed("0001.01",values.vEDATE);           break;
//      case vtINTEGER:  values.vINTEGER = 0;                       break;
//      case vtMONEY:    str2money("0.00",values.vMONEY);           break;
        case vtREAL:     values.vREAL = 0.0;                        break;
//      case vtSTRING:   values.vSTRING = NULL;                     break;
//      case vtTIME:     values.vTIME = 0;                          break;
//      case vtBYTE:     values.vBYTE = 0;                          break;
//      case vtWORD:     values.vWORD = 0;                          break;
//      case vtSBYTE:    values.vSBYTE = 0;                         break;
//      case vtSWORD:    values.vSWORD = 0;                         break;
//      case vtBIGSTR:   memset(&values.vBIGSTR,0,sizeof(tBIGSTR)); break;
        case vtDREAL:    values.vDREAL = 0.0;                       break;
    }
}

/******************************************************************************/
/******************************************************************************/

#ifndef ___SCAN___

/******************************************************************************/

// Defined Macros

#define STR2NUM     vtINTEGER

/******************************************************************************/

// Constants

#if defined(___COMP___) || defined(___INFO___)

char * typeText [] =
{
    "BOOLEAN",
    "UNSIGNED",
    "DATE",
    "EDATE",
    "INTEGER",
    "MONEY",
    "REAL",
    "STRING",
    "TIME",
    "BYTE",
    "WORD",
    "SHORT",
    "INT",
    "BIGSTR",
    "DREAL",
    "FUNCTION",
    "PROCEDURE"
    "DDATE"
};

#endif

/******************************************************************************/

// Miscellaneous Functions

char * LIBENTRY nul2mty(const cVARVAL & v)
{
    switch (v.type)
    {
        case vtSTRING:
            return (v.values.vSTRING     ? v.values.vSTRING     : "");

        case vtBIGSTR:
            return (v.values.vBIGSTR.ptr ? v.values.vBIGSTR.ptr : "");

        default:
            return "";
    }
}

    /*--------------------------------------------------------------------*/

tINTEGER lpow(tINTEGER l, tINTEGER r)
{
    if ((l == 0) || (r < 0)) return 0;
    if ((l == 1) || (r == 0)) return 1;
    r &= 0x1F;
    signed long int result = 1;
    for (int i = 0; i < r; ++i) result *= l;
    return result;
}

/******************************************************************************/

// Type Identification Functions

int LIBENTRY isUInteger(char * s)
{
    if (s == NULL || *s == '\0') return FALSE;           // Must have 1 char min (0)

    // While we haven't reached the end of the string . . .
    while (*s != '\0')
    {
        // If it isn't a digit then break
        if (!isascii(*s) || !isdigit(*s)) break;
        ++s;                                // Point to the next digit
    }

    // If we reached the end of the string then it is an unsigned int
    return (*s == '\0');
}

/******************************************************************************/

// Floating Point Support Functions

void LIBENTRY str2double(char * s, double & d)
{
    char * p = strchr(s,*Country.FractionSep);
    if (p) *p = '.';

    d = atof(s);

    if (p) *p = *Country.FractionSep;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY double2str(double d, char * s)
{
    sprintf(s,"%f",d);

    if (strchr(s,'e'))
    {
        strupr(s);
    }
    else
    {
        stripright(s,'0');
        stripright(s,'.');
    }

    char * p = strchr(s,'.');
    if (p) *p = *Country.FractionSep;
}

/******************************************************************************/

// Earth Date Support Functions

void LIBENTRY str2ed(char * s, tEDATE & e)
{
    char * dotPtr = strchr(s,'.');          // Get ptr to dec point

    // If dec point ptr ok then 0 terminate yymm field and inc to dd field
    if (dotPtr) *(dotPtr++) = '\0';

    tEDATE l = (atoi(s) % 10000);           // Get and adjust the yymm field

    // Get and adjust the dd field
    tEDATE r = (dotPtr ? atoi(dotPtr) : 0);
    while (r >= 100) r /= 10;

    if (dotPtr) *(--dotPtr) = '.';          // Replace the dec point if ok

    char t[8+1];                            // Build a standard date string
    sprintf(t,"%02u-%02u-%02u",l%100,r,l/100);

    e = datetojulian(t);                    // Convert to julian date
}

    /*--------------------------------------------------------------------*/

void LIBENTRY ed2str(tEDATE e, char * s)
{
    char * t = juliantodate(e);
    s[0] = t[6];
    s[1] = t[7];
    s[2] = t[0];
    s[3] = t[1];
    s[4] = '.';
    s[5] = t[3];
    s[6] = t[4];
    s[7] = '\0';
}

/******************************************************************************/

// Money Support Functions

void LIBENTRY str2money(char * s, tMONEY & m)
{
    m = 0;                              // Default to 0 units of money

    char * buf = strdup(s);             // Dup the string s for processing
    if (!buf) return;                   // Exit if memory allocation error

    bool fractionSep = FALSE;           // Assume no fraction separator
    int  fractionLen = 0;

    char * p = buf;                     // Init temp pointer to buf
    while (*p)                          // While we haven't hit end of string
    {
        if (*p == *Country.FractionSep) // If it's a fraction separator
            fractionSep = TRUE;             // Then set flag appropriately

        if (isascii(*p) && isdigit(*p)) // If character is ascii and digit
        {
            ++p;                        // Move on to next character
            if (fractionSep)
                ++fractionLen;
        }
        else
            strcpy(p,p+1);              // Otherwise remove bad character
    }

    m = atol(buf);                      // Convert amount to units of money

    // While we don't have the right number of decimal digits
    while (fractionLen != Country.SigDigits)
    {
        // Scale m appropriate (down or up) and adjust the scaled digits
             if (fractionLen > Country.SigDigits) m /= 10, --fractionLen;
        else if (fractionLen < Country.SigDigits) m *= 10, ++fractionLen;
    }

    free(buf);                          // Free the processing buffer
}

    /*--------------------------------------------------------------------*/

tINTEGER lpow(tINTEGER l, tINTEGER r);

void LIBENTRY money2str(tMONEY m, char * s)
{
    char buf[80+1];

    if (!(Country.CurrencyFmt & 0x01))  // Does the symbol go in front?
    {
        strcpy(buf,Country.DollarSym);
        // Do we need a space after it?
        if (Country.CurrencyFmt & 0x02) strcat(buf," ");
    }
    else
        *buf = '\0';                    // No, so init the buffer

    if (m < 0)                          // If money is less than 0
    {
        strcat(buf,"-");                // Append a minus sign to the buffer
        m = -m;                         // Convert to positive
    }

    // Convert significant digits to a divisor
    tINTEGER decVal = lpow(10,Country.SigDigits);

    // Write out value to left of decimal point
    sprintf(buf+strlen(buf),"%ld",m/decVal);

    // If there are significant digits to the right of decimal point
    if (Country.SigDigits > 0)
        // Write out fraction separator and value to right of decimal point
        sprintf(buf+strlen(buf),"%s%0*ld",Country.FractionSep,
            int(Country.SigDigits),m%decVal);

    if (Country.CurrencyFmt & 0x01)     // Does the symbol go in back?
    {
        // Do we need a space before it?
        if (Country.CurrencyFmt & 0x02) strcat(buf," ");
        strcat(buf,Country.DollarSym);
    }

    strcpy(s,buf);
}

/******************************************************************************/

// Time Support Functions

void LIBENTRY str2time(char * str, tTIME & t)
{
    t = 0;

    char * buf = strdup(str);
    if (!buf) return;

    char * h = strtok(buf,Country.TimeSep);
    char * m = strtok(NULL,Country.TimeSep);
    char * s = strtok(NULL,Country.TimeSep);

    if (isUInteger(h) && isUInteger(m) && (!s || isUInteger(s)))
        t = atol(h)*60*60 + atol(m)*60 + (s ? atol(s) : 0);
    else
        t = 0;

    free(buf);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY time2str(tTIME t, char * str)
{
    signed long int h = (t / (60L * 60L));
    signed long int m = (t % (60L * 60L)) / 60L;
    signed long int s = (t % 60L);

    sprintf(str,"%02.2ld%s%02.2ld%s%02.2ld",
        h,Country.TimeSep,m,Country.TimeSep,s);
}

/******************************************************************************/

// cVAR Member Functions

cVAR::cVAR(char * _name, sVARINFO & _info) :
    name(_name ? strdup(_name) : NULL),
    info(_info),
    data(NULL),
    elems(0)
#if defined(___INFO___)
    ,
    refs(0)
#endif
{
    redimArray(info.dims,info.vs,info.ms,info.cs);   //lint !e534   Ignoring return value
}

    /*--------------------------------------------------------------------*/

#define defarray(t) case vt##t: data = new cVV##t[elems]; break

int LIBENTRY cVAR::redimArray(unsigned char _dims, int _vv, int _mv, int _cv)
{
    if (data) delete [] data;

    data = NULL;

    info.dims = _dims;
    info.vs = _vv;
    info.ms = _mv;
    info.cs = _cv;

    if (info.dims > 3) info.dims = 3;

    if (info.cs < 0) info.cs = 0;
    if (info.ms < 0) info.ms = 0;
    if (info.vs < 0) info.vs = 0;

    if ((info.dims == 3) && (info.cs == 0)) info.dims = 2;
    if ((info.dims == 2) && (info.ms == 0)) info.dims = 1;
    if ((info.dims == 1) && (info.vs == 0)) info.dims = 0;

    elems = 1;
    switch (info.dims)
    {
        case 3: elems *= (info.cs+1);
        case 2: elems *= (info.ms+1);       //lint !e616  Falls through
        case 1: elems *= (info.vs+1);       //lint !e616  Falls through
    }

    switch (info.type)
    {
        defarray(BOOLEAN);
        defarray(UNSIGNED);
        defarray(DATE);
        defarray(EDATE);
        defarray(INTEGER);
        defarray(MONEY);
        defarray(REAL);
        defarray(STRING);
        defarray(TIME);
        defarray(BYTE);
        defarray(WORD);
        defarray(SBYTE);
        defarray(SWORD);
        defarray(BIGSTR);
        defarray(DREAL);
        defarray(FUNCTION);
        defarray(PROCEDURE);
        defarray(DDATE);
    }

#ifdef ___EXEC___
    if (!data)
    {
        if (elems == 1)
        {
            elems = info.dims = info.vs = info.ms = info.cs = 0;
            return TRUE;
        }
        else
        {
            return redimArray(0,0,0,0);
        }
    }
#endif

    return FALSE;
}

    /*--------------------------------------------------------------------*/

cVAR::~cVAR(void)
{
    if (name) free(name);
    if (data) delete [] data;
}

    /*--------------------------------------------------------------------*/

cVARVAL * LIBENTRY cVAR::getVal(int vn, int mn, int cn)
{
    if (vn == 0 && mn == 0 && cn == 0)
      return data;

    if ((vn < 0) || (vn > info.vs)) return NULL;
    if ((mn < 0) || (mn > info.ms)) return NULL;
    if ((cn < 0) || (cn > info.cs)) return NULL;

    int en = 0;
    switch (info.dims)
    {
        case 3: en += (cn*(info.ms+1)*(info.vs+1));
        case 2: en += (mn*(info.vs+1));                //lint !e616 Fals through
        case 1: en += (vn);                            //lint !e616 Fals through
    }

    if ((en < 0) || (en >= elems)) return NULL;

    return &data[en];
}

/******************************************************************************/

// cVARVAL Member Functions

cVARVAL::cVARVAL(int _type, tINTEGER _value) :
    type(_type)
{
    // Set the default value
    initialize();

    // Build a temporary integer cVARVAL object and assign it a value
    cVARVAL tmp(vtINTEGER);
    tmp.values.vINTEGER = _value;

    // Now let the automatic type conversion built into
    // cVARVAL convert tmp to the appropriate type
    *this = tmp;
}

    /*--------------------------------------------------------------------*/

cVARVAL::cVARVAL(int _type, tSTRING _value) :
    type(_type)
{
    // Set the default value
    initialize();

    // Build a temporary string cVARVAL object and assign it a value
    cVARVAL tmp(vtBIGSTR);                          // changed to BIGSTR by stan
    if (_value) tmp.pokeBuf(_value,strlen(_value));

    // Now let the automatic type conversion built into
    // cVARVAL convert tmp to the appropriate type
    *this = tmp;
}

    /*--------------------------------------------------------------------*/

cVARVAL::cVARVAL(int _type, tDREAL _value) :
    type(_type)
{
    // Set the default value
    initialize();

    // Build a temporary floating point cVARVAL object and assign it a value
    cVARVAL tmp(vtDREAL);
    tmp.values.vDREAL = _value;

    // Now let the automatic type conversion built into
    // cVARVAL convert tmp to the appropriate type
    *this = tmp;
}

    /*------------------------------------------------------------------*/

#define pokeArg(type) case vt##type: values.v##type = *(t##type *)buf; break

void LIBENTRY cVARVAL::pokeBuf(char _FAR_ * buf, int bufLen)
{
    buf[bufLen] = '\0';

    switch (type)
    {
        case vtBOOLEAN:
            values.vBOOLEAN = (*(tBOOLEAN *)buf ? TRUE : FALSE);
            break;

        pokeArg(UNSIGNED);
        pokeArg(DATE);
        pokeArg(EDATE);
        pokeArg(INTEGER);
        pokeArg(MONEY);
        pokeArg(REAL);
        pokeArg(DDATE);

        case vtSTRING:
            if (bufLen > MAX_STR_LEN) buf[MAX_STR_LEN] = '\0';
            if (!values.vSTRING)
                values.vSTRING = strdup(buf);                    // USed to be bstrdup
            else if (strlen(values.vSTRING) >= strlen(buf))
                strcpy(values.vSTRING,buf);
            else
            {
                free(values.vSTRING);                             // USed to be bfree
                values.vSTRING = strdup(buf);                   // USed to be bstrdup
            }
            break;

        pokeArg(TIME);
        pokeArg(BYTE);
        pokeArg(WORD);
        pokeArg(SBYTE);
        pokeArg(SWORD);

        case vtBIGSTR:
            values.vBIGSTR.len = values.vBIGSTR.size = 0;
            if (values.vBIGSTR.ptr) free(values.vBIGSTR.ptr);      // USed to be bfree
            values.vBIGSTR.ptr = NULL;
            if (bufLen > 0)
            {
                values.vBIGSTR.ptr = (char *) malloc(bufLen+1);     // Used to be BMALLOC
                if (values.vBIGSTR.ptr)
                {
                    memcpy(values.vBIGSTR.ptr,buf,bufLen+1);
                    values.vBIGSTR.len = values.vBIGSTR.size = bufLen;
                }
            }
            break;

        pokeArg(DREAL);
    }
}

    /*--------------------------------------------------------------------*/

cVARVAL & cVARVAL::operator = (const cVARVAL & rvalue)
{
    char * p;

    if (type == rvalue.type)
    {
        switch (type) {
          case vtFUNCTION : // Funtions and procedures need hi word for firstId number
          case vtPROCEDURE: // fall through
          case vtDREAL  : values.vSTDVARSIZE.hi = rvalue.values.vSTDVARSIZE.hi;
                          // fall through

          case vtDDATE  :
          case vtDATE   :
          case vtEDATE  :
          case vtINTEGER:
          case vtMONEY  :
          case vtREAL   :
          case vtTIME   :
          case vtBYTE   :
          case vtWORD   :
          case vtSBYTE  :
          case vtSWORD  : values.vSTDVARSIZE.lo = rvalue.values.vSTDVARSIZE.lo;
                          return *this;

          case vtSTRING : p = nul2mty(rvalue);
                          pokeBuf(p,strlen(p));
                          return *this;

          case vtBIGSTR : p = nul2mty(rvalue);
                          pokeBuf(p,rvalue.values.vBIGSTR.len);
                          return *this;
        }
//  } else if ((type == vtSTRING || type == vtBIGSTR) && (rvalue.type == vtSTRING || rvalue.type == vtBIGSTR)) {
//    p = nul2mty(rvalue);
//    pokeBuf(p,(rvalue.type == vtBIGSTR ? rvalue.values.vBIGSTR.len : strlen(p)));
//    return *this;
    }

    // Variable to hold the binary rvalue
    tDREAL          rbin = 0.0;
    tINTEGER        ibin = 0;

    int             txtLen;
//    unsigned int    arr[3];                // Temporary array to hold 3 int fields
    unsigned short    arr[3];                // Temporary array to hold 3 int fields
    memset(arr,0,3);

    char            txt[MAX_BIGSTR_LEN+1]; // Variable to hold the text rvalue

    *txt = '\0';
//    memset(txt,0,MAX_BIGSTR_LEN+1);
    txtLen = 0;

    char typeOfLValue;
    switch (type)
    {
        case vtSTRING:
        case vtBIGSTR:
            typeOfLValue = 1; // Dynamic String Type
            break;

        case vtREAL:
        case vtDREAL:
            typeOfLValue = 2; // Binary Real Type
            break;

        default:
            typeOfLValue = 3; // Binary Integer Type
            break;
    }

    // Convert rvalue to binary and text forms
    switch (rvalue.type)
    {
        case vtBOOLEAN:
            switch (typeOfLValue)
            {
                case 1: strcpy(txt,rvalue.values.vBOOLEAN ? "1" : "0");   break;
                case 2: rbin = (rvalue.values.vBOOLEAN ? TRUE : FALSE);   break;
                case 3: ibin = (rvalue.values.vBOOLEAN ? TRUE : FALSE);   break;
            }
            break;

        case vtUNSIGNED:
            switch (typeOfLValue)
            {
                case 1: ultoa(rvalue.values.vUNSIGNED,txt,10);            break;
                case 2: rbin = rvalue.values.vUNSIGNED;                   break;
                case 3: ibin = rvalue.values.vUNSIGNED;                   break;
            }
            break;

        case vtDDATE:
            switch (typeOfLValue)
            {
                case 1:
                {
                   long l1 = rvalue.values.vDDATE;
                   l1 += 1721425L+693960L-365L;
                   #if defined(DBASE) && ! defined(___COMP___)
                     date4assign(txt,l1);
                   #endif
                   txt[8] = '\x0';
                   break;
                }
                case 2: rbin = rvalue.values.vDDATE;                      break;
                case 3: ibin = rvalue.values.vDDATE;                      break;
            }
            break;

        case vtDATE:
            switch (typeOfLValue)
            {
                case 1: strcpy(txt,countrydate(juliantodate(rvalue.values.vDATE)));
                                                                          break;
                case 2: rbin = rvalue.values.vDATE;                       break;
                case 3: ibin = rvalue.values.vDATE;                       break;
            }
            break;

        case vtEDATE:
            switch (typeOfLValue)
            {
                case 1: ed2str(rvalue.values.vEDATE,txt);                 break;
                case 2: rbin = rvalue.values.vEDATE;                      break;
                case 3: ibin = rvalue.values.vEDATE;                      break;
            }
            break;

        case vtINTEGER:
            switch (typeOfLValue)
            {
                case 1: lascii(txt,rvalue.values.vINTEGER);               break;
                case 2: rbin = rvalue.values.vINTEGER;                    break;
                case 3: ibin = rvalue.values.vINTEGER;                    break;
            }
            break;

        case vtMONEY:
            switch (typeOfLValue)
            {
                case 1: money2str(rvalue.values.vMONEY,txt);              break;
                case 2: rbin = rvalue.values.vMONEY;                      break;
                case 3: ibin = rvalue.values.vMONEY;                      break;
            }
            break;

        case vtREAL:
            switch (typeOfLValue)
            {
                case 1: double2str((double)(rvalue.values.vREAL),txt);    break;
                case 2: rbin = rvalue.values.vREAL;                       break;
                case 3: ibin = (long) rvalue.values.vREAL;                break;
            }
            break;

        case vtSTRING:
            strcpy(txt,nul2mty(rvalue));
            switch (typeOfLValue)
            {
                case 2: str2double(txt,rbin);                             break;
                case 3: ibin = atol(txt);                                 break;
            }
            break;

        case vtTIME:
            switch (typeOfLValue)
            {
                case 1: time2str(rvalue.values.vTIME,txt);                break;
                case 2: rbin = rvalue.values.vTIME;                       break;
                case 3: ibin = rvalue.values.vTIME;                       break;
            }
            break;

        case vtBYTE:
            switch (typeOfLValue)
            {
                case 1: itoa((tSWORD)rvalue.values.vBYTE,txt,10);         break;
                case 2: rbin = rvalue.values.vBYTE;                       break;
                case 3: ibin = rvalue.values.vBYTE;                       break;
            }
            break;

        case vtWORD:
            switch (typeOfLValue)
            {
                case 1: ultoa((tUNSIGNED)rvalue.values.vWORD,txt,10);     break;
                case 2: rbin = rvalue.values.vWORD;                       break;
                case 3: ibin = rvalue.values.vWORD;                       break;
            }
            break;

        case vtSBYTE:
            switch (typeOfLValue)
            {
                case 1: itoa((tSWORD)rvalue.values.vSBYTE,txt,10);        break;
                case 2: rbin = rvalue.values.vSBYTE;                      break;
                case 3: ibin = rvalue.values.vSBYTE;                      break;
            }
            break;

        case vtSWORD:
            switch (typeOfLValue)
            {
                case 1: itoa(rvalue.values.vSWORD,txt,10);                break;
                case 2: rbin = rvalue.values.vSWORD;                      break;
                case 3: ibin = rvalue.values.vSWORD;                      break;
            }
            break;

        case vtBIGSTR:
            txtLen = rvalue.values.vBIGSTR.len;
            if (rvalue.values.vBIGSTR.ptr && (rvalue.values.vBIGSTR.len > 0))
                memcpy(txt,rvalue.values.vBIGSTR.ptr,txtLen+1);
            switch (typeOfLValue)
            {
                case 2: str2double(txt,rbin);                             break;
                case 3: ibin = atol(txt);                                 break;
            }
            break;

        case vtDREAL:
            switch (typeOfLValue)
            {
                case 1: double2str((double)(rvalue.values.vDREAL),txt);   break;
                case 2: rbin = rvalue.values.vDREAL;                      break;
                case 3: ibin = (long) rvalue.values.vDREAL;               break;
            }
            break;

        default:
            ibin = 0;
            break;
    }

    txt[MAX_BIGSTR_LEN] = '\0';
    if (rvalue.type != vtBIGSTR)
        txtLen = strlen(txt);
    else
        txt[txtLen] = '\0';

    char typeOfRValue;
    switch (rvalue.type)
    {
        case vtSTRING:
        case vtBIGSTR:
            typeOfRValue = 1; // Dynamic String Type
            break;

        case vtREAL:
        case vtDREAL:
            typeOfRValue = 2; // Binary Real Type
            break;

        default:
            typeOfRValue = 3; // Binary Integer Type
            break;
    }

    // Convert rvalue to lvalue type
    switch (type)
    {
        case vtBOOLEAN:
            values.vBOOLEAN = (ibin ? TRUE : FALSE);
            break;

        case vtUNSIGNED:
            values.vUNSIGNED = ibin;
            break;

          case vtDDATE:
              if (typeOfRValue == 1)
                  values.vDDATE =
                     #if defined(DBASE) && ! defined(___COMP___)
                       date4long(txt) - (1721425L+693960L-365L);
                     #else
                       0;
                     #endif
              else
                  values.vDDATE = (signed long) ibin;
              break;

        case vtDATE:
            if (typeOfRValue == 1)
            {
                breakdate(arr,txt);
                sprintf(txt,"%02u%02u%02u",
                    arr[0],arr[1],arr[2]);
                uncountrydate2(txt);
                values.vDATE = datetojulian(txt);
            }
            else
                values.vDATE = (unsigned int) ibin;
            break;

        case vtEDATE:
            if (typeOfRValue == 1)
                str2ed(txt,values.vEDATE);
            else
                values.vEDATE = (unsigned int) ibin;
            break;

        case vtINTEGER:
            values.vINTEGER = ibin;
            break;

        case vtMONEY:
            if (typeOfRValue == 1)
                str2money(txt,values.vMONEY);
            else
                values.vMONEY = ibin;
            break;

        case vtREAL:
            values.vREAL = rbin;
            break;

        case vtSTRING:
            pokeBuf(txt,txtLen);
            break;

        case vtTIME:
            if (typeOfRValue == 1)
                str2time(txt,values.vTIME);
            else
                values.vTIME = ibin;
            values.vTIME %= (24L*60L*60L);
            break;

        case vtBYTE:
            values.vBYTE = (tBYTE) ibin;
            break;

        case vtWORD:
            values.vWORD = (tWORD) ibin;
            break;

        case vtSBYTE:
            values.vSBYTE = (tSBYTE) ibin;
            break;

        case vtSWORD:
            values.vSWORD = (tSWORD) ibin;
            break;

        case vtBIGSTR:
            pokeBuf(txt,txtLen);
            break;

        case vtDREAL:
            values.vDREAL = rbin;
            break;
    }

    return *this;
}

    /*--------------------------------------------------------------------*/

const cVARVAL & cVARVAL::operator = (tINTEGER rvalue)
{
    return (*this = cVARVAL(vtINTEGER,rvalue));
}

    /*--------------------------------------------------------------------*/

const cVARVAL & cVARVAL::operator = (tSTRING rvalue)
{
    return (*this = cVARVAL(vtBIGSTR,rvalue));  // Changed from string to bigstring by stan
}

    /*--------------------------------------------------------------------*/

const cVARVAL & cVARVAL::operator = (tDREAL rvalue)
{
    return (*this = cVARVAL(vtDREAL,rvalue));
}


    /*--------------------------------------------------------------------*/

cVARVAL::operator char * ()
{
    if(type == vtSTRING)
        return (nul2mty(*this));

    if(type == vtBIGSTR)
        return (nul2mty(*this));

    return NULL;
}

    /*--------------------------------------------------------------------*/

cVARVAL::operator long ()
{
    return values.vINTEGER;
}

/******************************************************************************/

#endif

/******************************************************************************/
/******************************************************************************/

#ifdef ___COMP___

/******************************************************************************/

// Types

/******************************************************************************/

// Functions

/******************************************************************************/

// Earth Date Support Functions

/******************************************************************************/

// Money Support Functions

/******************************************************************************/

// Time Support Functions

/******************************************************************************/

// Type Identification Functions

int LIBENTRY isInteger(char * s)
{
    if ((*s == '-') || (*s == '+')) ++s;    // Leading + or - ok
    return isUInteger(s);                   // Is remaining unsigned int?
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isReal(char * s)
{
    if (*s == '\0') return FALSE;           // No length, not real

    if ((*s == '-') || (*s == '+') || (*s == '.') ) ++s;    // Leading + or - or . ok
    if(!isdigit(*s)) return FALSE;
    if (*s == '\0') return FALSE;           // No length, not real

    while (isascii(*s) && isdigit(*s)) ++s; // Skip digits

    if (*s == '.')                          // Skip decimal point
    {
        ++s;
        while (isascii(*s) && isdigit(*s)) ++s; // Skip digits
    }

    if (toupper(*s) == 'E')                 // Skip exponent
    {
        ++s;
        if ((*s == '-') || (*s == '+')) ++s;    // Leading + or - ok
//        if (*s == '\0') return FALSE;           // No length after E[+/-], not real
        while (isascii(*s) && isdigit(*s)) ++s; // Skip digits
    }

    // If we reached the end of the string then it is real
    return (*s == '\0');
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isString(char * s)
{
    if (*s != '\"') return FALSE;           // First char must be quote

    char * quote = strchr(s+1,'\"');        // Get ptr to closing quote

    while (quote && (*(quote+1) == '\"'))
        quote = strchr(quote+2,'\"');

    // If there is a close quote and no more data after quote then it is string
    return (quote && (*(quote+1) == '\0'));
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isBase(char * s, char base, const char * digits)
{
    if (strlen(s) <= 1) return FALSE;       // Must have 2 chars min (0b)

    // If s's base character isn't the specified base then not of base
    if (toupper(s[strlen(s)-1]) != base) return FALSE;

    if (isascii(*s) && !isdigit(*s)) return FALSE;

    // While we haven't reached the end of the string . . .
    while (*(s+1) != '\0')
    {
        // If get cur digit isn't a valid digit then break
        if (!strchr(digits,toupper(*s))) break;
        ++s;                                // Point to the next digit
    }

    // If we reached the end of the string (base letter) then it is of base
    return (*(s+1) == '\0');
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isBinary(char * s)
{
    return isBase(s,'B',"01");
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isOctal(char * s)
{
    return isBase(s,'O',"01234567");
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isDecimal(char * s)
{
    return isBase(s,'D',"0123456789");
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isHexadecimal(char * s)
{
    return isBase(s,'H',"0123456789ABCDEF");
}

    /*--------------------------------------------------------------------*/

int LIBENTRY isMoney(char * s)
{
    if (strlen(s) < 4) return FALSE;        // Must have 4 chars min ($.00)
    if (*(s++) != '$') return FALSE;        // First char must be dollar sign

    // If the third char from the end isn't a dec point then not money
    if (s[strlen(s)-3] != '.') return FALSE;

    if ((*s == '-') || (*s == '+')) ++s;    // Leading + or - ok

    // While we haven't reached the end of the string . . .
    while (*s != '\0')
    {
        // If necessary, skip the decimal point
        if (strlen(s) == 3)
        {
            ++s;
            continue;
        }

        // If it isn't a digit then break
        if (!isascii(*s) || !isdigit(*s)) break;
        ++s;                                // Point to the next digit
    }

    // If we reached the end of the string then it is money
    return (*s == '\0');
}

/******************************************************************************/

// cVARLST Member Functions

cVARLST::cVARLST(int _size) :
    size(_size),
    next(0),
    list(NULL),
    firstSysVar(0)
{
    list = new cVARPTR[size];
    if (list == NULL) size = 0;
}

cVARLST::~cVARLST(void)
{
    if (list)
    {
        for (int i = 0; i < next; ++i)
            if (list[i])
                delete list[i];
        delete [] list;
    }
}

int LIBENTRY cVARLST::add(cVAR * data)
{
    if (next == size) return -1;
    for (int i = 0; i < next; ++i)
        if (strcmp(data->name,list[i]->name) < 0)
            break;
    if (i < next) memmove(&list[i+1],&list[i],(next-i)*sizeof(list[0]));
    ++next;
    list[i] = data;
    if ((*(unsigned char *)data->name) < '~') ++firstSysVar;
    return 0;
}

int LIBENTRY cVARLST::used(void)
{
    return next;
}

int cVARLST::findNameCmp(const void * lv, const void * rv)
{
    return strcmp((char *)lv,(*(cVAR**)rv)->name);
}

cVAR * LIBENTRY cVARLST::findName(char * name)
{
    cVARPTR * varptrptr = (cVARPTR *) bsearch(name,list,next,
        sizeof(list[0]),findNameCmp);
    return (varptrptr ? *varptrptr : NULL);
}

int cVARLST::findStringCmp(const void * lv, const void * rv)
{
    if ((*(cVAR**)rv)->info.type != vtSTRING) return 1;
    return (strcmp((char *)lv,(*(cVAR**)rv)->data->values.vSTRING) != 0);
}

cVAR * LIBENTRY cVARLST::findString(char * str)
{
    size_t nelem = next-firstSysVar;

    cVARPTR * varptrptr = (cVARPTR *) lfind(str,&list[firstSysVar],&nelem,
        sizeof(list[0]),findStringCmp);

    return (varptrptr ? *varptrptr : NULL);
}

int cVARLST::sortIDRevCmp(const void * lv, const void * rv)
{
    return ((*(cVAR**)rv)->info.id - (*(cVAR**)lv)->info.id);
}

void LIBENTRY cVARLST::sortIDReverse(void)
{
    qsort(list,next,sizeof(list[0]),sortIDRevCmp);
}

void LIBENTRY cVARLST::forEach(void LIBENTRY (* func)(cVAR & obj))
{
    for (int i = 0; i < next; ++i)
        func(*list[i]);
}

/******************************************************************************/

#endif

/******************************************************************************/
/******************************************************************************/

#ifdef ___EXEC___

/******************************************************************************/

// Types

/******************************************************************************/

// Constants

char _FAR_ *inkeyText[] =
{
    // 0x00
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           "SHIFT TAB",

    // 0x10
    "ALT Q",        "ALT W",        "ALT E",        "ALT R",
    "ALT T",        "ALT Y",        "ALT U",        "ALT I",
    "ALT O",        "ALT P",        NULL,           NULL,
    NULL,           NULL,           "ALT A",        "ALT S",

    // 0x20
    "ALT D",        "ALT F",        "ALT G",        "ALT H",
    "ALT J",        "ALT K",        "ALT L",        NULL,
    NULL,           NULL,           NULL,           NULL,
    "ALT Z",        "ALT X",        "ALT C",        "ALT V",

    // 0x30
    "ALT B",        "ALT N",        "ALT M",        NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           "F1",
    "F2",           "F3",           "F4",           "F5",

    // 0x40
    "F6",           "F7",           "F8",           "F9",
    "F10",          NULL,           NULL,           "HOME",
    "UP",           "PGUP",         NULL,           "LEFT",
    NULL,           "RIGHT",        NULL,           "END",

    // 0x50
    "DOWN",         "PGDN",         "INS",          "DEL",
    "SHIFT F1",     "SHIFT F2",     "SHIFT F3",     "SHIFT F4",
    "SHIFT F5",     "SHIFT F6",     "SHIFT F7",     "SHIFT F8",
    "SHIFT F9",     "SHIFT F10",    "CTRL F1",      "CTRL F2",

    // 0x60
    "CTRL F3",      "CTRL F4",      "CTRL F5",      "CTRL F6",
    "CTRL F7",      "CTRL F8",      "CTRL F9",      "CTRL F10",
    "ALT F1",       "ALT F2",       "ALT F3",       "ALT F4",
    "ALT F5",       "ALT F6",       "ALT F7",       "ALT F8",

    // 0x70
    "ALT F9",       "ALT F10",      NULL,           "CTRL LEFT",
    "CTRL RIGHT",   "CTRL END",     "CTRL PGDN",    "CTRL HOME",
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0x80
    NULL,           NULL,           NULL,           NULL,
    "CTRL PGUP",    NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0x90
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0xA0
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0xB0
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0xC0
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0xD0
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0xE0
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,

    // 0xF0
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL,
    NULL,           NULL,           NULL,           NULL
};

/******************************************************************************/

// Functions

/******************************************************************************/

// Type Identification Functions

int LIBENTRY isHEX(char c)
{
    return isascii(c) && isxdigit(c) && (isdigit(c) || isupper(c));
}

/******************************************************************************/

// Miscellaneous Functions

int LIBENTRY promoteTo(int l, int r)
{
    // If the types are the same then no promotion necessary
    if (l == r)
        return l;

    // If the types are both string (vtSTRING or vtBIGSTR) and they aren't
    //   the same, then promote to vtBIGSTR
    else if (((l == vtSTRING) || (l == vtBIGSTR)) &&
             ((r == vtSTRING) || (r == vtBIGSTR)))
        return vtBIGSTR;

    // If at least one of the types is real (vtREAL or vtDREAL), then promote
    //   to vtDREAL
    else if ((l == vtREAL) || (l == vtDREAL)  ||
             (r == vtREAL) || (r == vtDREAL))
        return vtDREAL;

    // If none of the above are true, just promote to integer
    else
        return vtINTEGER;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY dayPart(cVARVAL & v, long m, long d)
{
    cVARVAL t(vtTIME);
    t = v;
    return cVARVAL(vtINTEGER,t.values.vTIME%m/d);
}

    /*------------------------------------------------------------------*/

void LIBENTRY pplstripleft(char * str, sint & len, char ch)
{
    if ((str == NULL) || (len == 0)) return;

    char * tstr = str;
    int    tlen = len;

    while ((tlen > 0) && (*tstr == ch))
    {
        ++tstr;
        --tlen;
    }

    memcpy(str,tstr,tlen);
    len = tlen;
}

    /*------------------------------------------------------------------*/

void LIBENTRY pplstripright(char * str, sint & len, char ch)
{
    if ((str == NULL) || (len == 0)) return;

    char * p = str+len-1;

    while ((len > 0) && (*p == ch))
    {
        --p;
        --len;
    }
}

    /*------------------------------------------------------------------*/

cVARVAL LIBENTRY trimit(cVARVAL & l, cVARVAL & r,
    void LIBENTRY (*tf)(char * str, sint & len, char ch))
{
    cVARVAL dVal(vtBIGSTR);
    cVARVAL lVal(vtBIGSTR);
    cVARVAL rVal(vtBIGSTR);

    lVal = l;
    rVal = r;

    if (rVal.values.vBIGSTR.len > 0)
        tf(lVal.values.vBIGSTR.ptr,lVal.values.vBIGSTR.len,
            *rVal.values.vBIGSTR.ptr);

    dVal = lVal;

    return dVal;
}

/******************************************************************************/

// Date Support Functions

cVARVAL LIBENTRY datePart(cVARVAL & v, int o)
{
    cVARVAL d(vtDATE);
    d = v;

    char * b = juliantodate(d.values.vDATE);

    // NOTES:  36524 is the julian date for 31 Dec 1999
    //         6     is the offset within the string of the year field
    return cVARVAL(vtINTEGER,strtol(b+o,NULL,10) +
        ((o == 6) ? ((d.values.vDATE > 36524L) ? 2000 : 1900) : 0));
}

    /*--------------------------------------------------------------------*/

void LIBENTRY int2amerDate(unsigned short * a)
{
    unsigned short ti;
    switch (Country.DateFormat)
    {
        case 1:
            ti = a[0];    // MM-DD-YY -> DD-MM-YY
            a[0] = a[1];
            a[1] = ti;
            break;

        case 2:
            ti = a[0];    // MM-DD-YY -> YY-MM-DD
            a[0] = a[2];
            a[2] = a[1];
            a[1] = ti;
            break;
    }
}

/******************************************************************************/

// Keystroke Input Functions

cVARVAL LIBENTRY inkeysub(int LIBENTRY (*f)(void))
{
    char buf[80+1] = "BADKEY";

    int key = f();

    if ((key >= 0) && (key <= 255))
    {
        buf[0] = key;
        buf[1] = '\0';
    }
    else if ((key >= 1000) && (key <= 1255) && inkeyText[key-1000])
        strcpy(buf,inkeyText[key-1000]);

    return cVARVAL(vtSTRING,buf);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY inkey(void)
{
      return inkeysub(cinkey);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY kinkey(void)
{
    return inkeysub(kbdinkey);
}
cVARVAL LIBENTRY tinkey(long ticks)
{

cVARVAL  s(vtSTRING);

    settimer(10,standardizeTicks(ticks));

    #ifdef __OS2__
       if(ticks > 0) releasekbdblock(standardizeTicks(ticks));
    #endif

       do
       {
         #ifdef __OS2__
           s = inkeysub(waitforkey);
         #else
           s = inkey();
           giveup();
         #endif

       }while( nul2mty(s)[0] == '\x0' && ( ticks == 0 || !timerexpired(10) ) );
       return s;
}


    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY minkey(void)
{
   #ifdef COMM
    return inkeysub(commportinkey);
   #else
    return cVARVAL(vtSTRING);
   #endif
}

/******************************************************************************/

// Earth Date Support Functions

/******************************************************************************/

// Money Support Functions

/******************************************************************************/

// Time Support Functions

/******************************************************************************/

// Type Identification Functions

/******************************************************************************/

// cVAR Member Functions (Array sorting and support)

cVARVAL * sortCMP_arr;

int sortCMP(const void * l, const void * r)
{
    cVARVAL * lv = (cVARVAL *) l;
    cVARVAL * rv = (cVARVAL *) r;

    int li = int(lv->values.vINTEGER);
    int ri = int(rv->values.vINTEGER);

    int ordinality = 1;

    if (sortCMP_arr[li] <  sortCMP_arr[ri]) ordinality = -1;
    if (sortCMP_arr[li] == sortCMP_arr[ri]) ordinality =  0;
 /* if (sortCMP_arr[li] >  sortCMP_arr[ri]) ordinality =  1; */

    switch (sortCMP_arr->type)
    {
        case vtSTRING:
        case vtBIGSTR:
            if      (*nul2mty(sortCMP_arr[li]) == '\0')
                ordinality =  1;
            else if (*nul2mty(sortCMP_arr[ri]) == '\0')
                ordinality = -1;
            break;
    }

    return ordinality;
}

void LIBENTRY cVAR::sort(cVAR * i)
{
    i->info.type = vtINTEGER;

    if (i->redimArray(info.dims,info.vs,info.ms,info.cs)) return;

    for (int c = 0; c <= info.cs; ++c)
        for (int m = 0; m <= info.ms; ++m)
        {
            for (int v = 0; v <= info.vs; ++v)
            {
                cVARVAL * p = i->getVal(v,m,c);
                *p = v;
            }

            sortCMP_arr = getVal(0,m,c);

            qsort(i->getVal(0,m,c),info.vs+1,sizeof(cVARVAL),sortCMP);
        }
}

/******************************************************************************/

// cVARVAL Member Functions

cVARVAL::cVARVAL(const cVARVAL & a) :
    type(a.type),
    values(a.values)
{
    char * p;
    int    l;

    switch (type)
    {
        case vtSTRING:
            values.vSTRING = NULL;
            p = a.values.vSTRING;
            if (p) pokeBuf(p,strlen(p));
            break;

        case vtBIGSTR:
            values.vBIGSTR.len = values.vBIGSTR.size = 0;
            values.vBIGSTR.ptr = NULL;
            p = a.values.vBIGSTR.ptr;
            l = a.values.vBIGSTR.len;
            if (p && (l > 0)) pokeBuf(p,l);
            break;
    }
}

    /*------------------------------------------------------------------*/

cVARVAL::cVARVAL(int _type, int _value) :
    type(_type)
{
    initialize();
    *this = cVARVAL(_type,tINTEGER(_value));}

    /*--------------------------------------------------------------------*/

cVARVAL::cVARVAL(int _type, unsigned _value) :
    type(_type)
{
    initialize();
    *this = cVARVAL(_type,tINTEGER(_value));
}

    /*--------------------------------------------------------------------*/

cVARVAL::cVARVAL(int _type, unsigned long _value) :
    type(_type)
{
    initialize();
    *this = cVARVAL(_type,tINTEGER(_value));
}

    /*------------------------------------------------------------------*/

void _FAR_ * LIBENTRY cVARVAL::varAddr(void)
{
    switch (type)
    {
        case vtSTRING:
            return values.vSTRING;

        case vtBIGSTR:
            return values.vBIGSTR.ptr;

        default:
            return &values;
    }
}

    /*------------------------------------------------------------------*/

#define sizeArg(type) case vt##type: size = sizeof(t##type); break

int LIBENTRY cVARVAL::varSize(void)
{
    int size = 0;

    switch (type)
    {
        sizeArg(BOOLEAN);
        sizeArg(UNSIGNED);
        sizeArg(DATE);
        sizeArg(DDATE);
        sizeArg(EDATE);
        sizeArg(INTEGER);
        sizeArg(MONEY);
        sizeArg(REAL);

        case vtSTRING:
            size = strlen(nul2mty(*this));
            break;

        sizeArg(TIME);
        sizeArg(BYTE);
        sizeArg(WORD);
        sizeArg(SBYTE);
        sizeArg(SWORD);

        case vtBIGSTR:
            size = values.vBIGSTR.size;
            break;

        sizeArg(DREAL);
    }

    return size;
}

    /*------------------------------------------------------------------*/

int LIBENTRY cVARVAL::varMaxSize(void)
{
    int size = 0;

    switch (type)
    {
        sizeArg(BOOLEAN);
        sizeArg(UNSIGNED);
        sizeArg(DATE);
        sizeArg(DDATE);
        sizeArg(EDATE);
        sizeArg(INTEGER);
        sizeArg(MONEY);
        sizeArg(REAL);

        case vtSTRING:
            size = MAX_STR_LEN;
            break;

        sizeArg(TIME);
        sizeArg(BYTE);
        sizeArg(WORD);
        sizeArg(SBYTE);
        sizeArg(SWORD);

        case vtBIGSTR:
            size = MAX_BIGSTR_LEN;
            break;

        sizeArg(DREAL);
    }

    return size;
}

/******************************************************************************/

// cVARVAL Member Functions (Overloaded Operators)

const cVARVAL & cVARVAL::operator = (int rvalue)
{
    return operator = (tINTEGER(rvalue));
}

    /*--------------------------------------------------------------------*/

const cVARVAL & cVARVAL::operator = (unsigned rvalue)
{
    return operator = (tINTEGER(rvalue));
}

    /*------------------------------------------------------------------*/

#define incArg(t1,t2) case vt##t1: case vt##t2: ++dVal.values.v##t2; break

cVARVAL & cVARVAL::operator ++ (void)
{
    int dType = type;

    switch (dType)
    {
        case vtBOOLEAN:
            dType = vtBYTE;
            break;

        case vtDATE: case vtEDATE: case vtDDATE:
            dType = vtWORD;
            break;

        case vtMONEY:  case vtREAL:  case vtSTRING: case vtTIME:
        case vtBIGSTR: case vtDREAL:// case vtDDATE:
            dType = vtINTEGER;
            break;
    }

    cVARVAL dVal(dType);

    dVal = *this;

    switch (dType)
    {
        incArg(UNSIGNED,INTEGER);
        incArg(BYTE,    SBYTE);
        incArg(WORD,    SWORD);
    }

    *this = dVal;

    return *this;
}

    /*------------------------------------------------------------------*/

#define decArg(t1,t2) case vt##t1: case vt##t2: --dVal.values.v##t2; break

cVARVAL & cVARVAL::operator -- (void)
{
    int dType = type;

    switch (dType)
    {
        case vtBOOLEAN:
            dType = vtBYTE;
            break;

        case vtDATE: case vtEDATE:
            dType = vtWORD;
            break;

        case vtMONEY:  case vtREAL:  case vtSTRING: case vtTIME:
        case vtBIGSTR: case vtDREAL:
            dType = vtINTEGER;
            break;
    }

    cVARVAL dVal(dType);

    dVal = *this;

    switch (dType)
    {
        decArg(UNSIGNED,INTEGER);
        decArg(BYTE,    SBYTE);
        decArg(WORD,    SWORD);
    }

    *this = dVal;

    return *this;
}

    /*--------------------------------------------------------------------*/

#define negArg(t) case vt##t: dVal.values.v##t = -lVal.values.v##t; break

cVARVAL cVARVAL::operator - (void)
{
    int dType = type;

    switch (dType)
    {
        case vtBOOLEAN: case vtUNSIGNED: case vtDATE: case vtEDATE:
        case vtMONEY:   case vtTIME:     case vtBYTE: case vtWORD: case vtDDATE:
            dType = vtINTEGER;
            break;

        case vtSTRING:  case vtBIGSTR:
            dType = STR2NUM;
            break;
    }

    cVARVAL dVal(dType), lVal(dType);

    lVal = *this;

    switch (dType)
    {
        negArg(INTEGER);
        negArg(REAL);
        negArg(SBYTE);
        negArg(SWORD);
        negArg(DREAL);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define addArgs(t) case vt##t: dVal.values.v##t = lVal.values.v##t + rVal.values.v##t; break

cVARVAL cVARVAL::operator + (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    switch (dType)
    {
        case vtBOOLEAN: case vtDATE: case vtEDATE: case vtMONEY:
        case vtTIME: case vtDDATE:
            dType = vtINTEGER;
            break;
    }

    cVARVAL dVal(dType), lVal(dType),  rVal(dType);

    lVal = *this;
    rVal = rvalue;

    char tmp[MAX_BIGSTR_LEN+1];

    switch (dType)
    {
        addArgs(UNSIGNED);
        addArgs(INTEGER);
        addArgs(REAL);

        case vtSTRING:
            strcpy(tmp,nul2mty(lVal));
            strcat(tmp,nul2mty(rVal));
            dVal.pokeBuf(tmp,strlen(tmp));
            break;

        addArgs(BYTE);
        addArgs(WORD);
        addArgs(SBYTE);
        addArgs(SWORD);

        case vtBIGSTR:
            if ((lVal.values.vBIGSTR.len+rVal.values.vBIGSTR.len) >
                MAX_BIGSTR_LEN)
                rVal.values.vBIGSTR.len = MAX_BIGSTR_LEN -
                    lVal.values.vBIGSTR.len;

            memcpy(tmp,                        nul2mty(lVal),
                                               lVal.values.vBIGSTR.len);
            memcpy(tmp+lVal.values.vBIGSTR.len,nul2mty(rVal),
                                               rVal.values.vBIGSTR.len);

            dVal.pokeBuf(tmp,lVal.values.vBIGSTR.len+rVal.values.vBIGSTR.len);

            break;

        addArgs(DREAL);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define subArgs(t) case vt##t: dVal.values.v##t = lVal.values.v##t - rVal.values.v##t; break

cVARVAL cVARVAL::operator - (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    switch (dType)
    {
        case vtBOOLEAN: case vtDATE: case vtEDATE: case vtMONEY:
        case vtTIME: case vtDDATE:
            dType = vtINTEGER;
            break;

        case vtSTRING: case vtBIGSTR:
            dType = STR2NUM;
            break;
    }

    cVARVAL dVal(dType), lVal(dType),  rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        subArgs(UNSIGNED);
        subArgs(INTEGER);
        subArgs(REAL);
        subArgs(BYTE);
        subArgs(WORD);
        subArgs(SBYTE);
        subArgs(SWORD);
        subArgs(DREAL);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define mulArgs(t) case vt##t: dVal.values.v##t = lVal.values.v##t * rVal.values.v##t; break

cVARVAL cVARVAL::operator * (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    switch (dType)
    {
        case vtBOOLEAN: case vtDATE: case vtEDATE: case vtMONEY:
        case vtTIME: case vtDDATE:
            dType = vtINTEGER;
            break;

        case vtSTRING: case vtBIGSTR:
            dType = STR2NUM;
            break;
    }

    cVARVAL dVal(dType), lVal(dType),  rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        mulArgs(UNSIGNED);
        mulArgs(INTEGER);
        mulArgs(REAL);
        mulArgs(BYTE);
        mulArgs(WORD);
        mulArgs(SBYTE);
        mulArgs(SWORD);
        mulArgs(DREAL);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define divArgs(t) case vt##t: dVal.values.v##t = ((rVal.values.v##t != 0) ? (lVal.values.v##t / rVal.values.v##t) : 0); break

cVARVAL cVARVAL::operator / (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    switch (dType)
    {
        case vtBOOLEAN: case vtDATE: case vtEDATE: case vtMONEY:
        case vtTIME: case vtDDATE:
            dType = vtINTEGER;
            break;

        case vtSTRING: case vtBIGSTR:
            dType = STR2NUM;
            break;
    }

    cVARVAL dVal(dType), lVal(dType),  rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        divArgs(UNSIGNED);
        divArgs(INTEGER);
        divArgs(REAL);
        divArgs(BYTE);
        divArgs(WORD);
        divArgs(SBYTE);
        divArgs(SWORD);
        divArgs(DREAL);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define modArgs(t) case vt##t: dVal.values.v##t = ((rVal.values.v##t != 0) ? (lVal.values.v##t % rVal.values.v##t) : 0); break

cVARVAL cVARVAL::operator % (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    switch (dType)
    {
        case vtBOOLEAN: case vtDATE:   case vtEDATE: case vtMONEY:
        case vtREAL:    case vtSTRING: case vtTIME:  case vtBIGSTR:
        case vtDREAL: case vtDDATE:
            dType = vtINTEGER;
            break;
    }

    cVARVAL dVal(dType), lVal(dType),  rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        modArgs(UNSIGNED);
        modArgs(INTEGER);
        modArgs(BYTE);
        modArgs(WORD);
        modArgs(SBYTE);
        modArgs(SWORD);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

int cVARVAL::operator !  (void)
{
    cVARVAL lVal(vtBOOLEAN);

    lVal = *this;

    return !lVal.values.vBOOLEAN;
}

    /*--------------------------------------------------------------------*/

int cVARVAL::operator && (cVARVAL & rvalue)
{
    cVARVAL lVal(vtBOOLEAN);
    cVARVAL rVal(vtBOOLEAN);

    lVal = *this;
    rVal = rvalue;

    return (lVal.values.vBOOLEAN && rVal.values.vBOOLEAN);
}

    /*--------------------------------------------------------------------*/

int cVARVAL::operator || (cVARVAL & rvalue)
{
    cVARVAL lVal(vtBOOLEAN);
    cVARVAL rVal(vtBOOLEAN);

    lVal = *this;
    rVal = rvalue;

    return (lVal.values.vBOOLEAN || rVal.values.vBOOLEAN);
}

    /*--------------------------------------------------------------------*/

#define eqArgs(t) case vt##t: dVal = (lVal.values.v##t == rVal.values.v##t); break

int cVARVAL::operator == (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    int dVal;

    cVARVAL lVal(dType), rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        eqArgs(BOOLEAN);
        eqArgs(UNSIGNED);
        eqArgs(DATE);
        eqArgs(DDATE);
        eqArgs(EDATE);
        eqArgs(INTEGER);
        eqArgs(MONEY);
        eqArgs(REAL);

        case vtSTRING:
            dVal = (strcmp(nul2mty(lVal),nul2mty(rVal)) == 0);
            break;

        eqArgs(TIME);
        eqArgs(BYTE);
        eqArgs(WORD);
        eqArgs(SBYTE);
        eqArgs(SWORD);

        case vtBIGSTR:
            dVal = ((lVal.values.vBIGSTR.len == rVal.values.vBIGSTR.len) &&
                (memcmp(lVal.values.vBIGSTR.ptr,rVal.values.vBIGSTR.ptr,
                lVal.values.vBIGSTR.len) == 0));
            break;

        default:
            dVal = 0;                        // this case should never happen. Gotta satisfy lint.
            break;

        eqArgs(DREAL);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define ltArgs(t) case vt##t: dVal = (lVal.values.v##t < rVal.values.v##t); break

int cVARVAL::operator <  (cVARVAL & rvalue)
{
    int dType = promoteTo(type,rvalue.type);

    int dVal;

    cVARVAL lVal(dType), rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        ltArgs(BOOLEAN);
        ltArgs(UNSIGNED);
        ltArgs(DATE);
        ltArgs(DDATE);
        ltArgs(EDATE);
        ltArgs(INTEGER);
        ltArgs(MONEY);
        ltArgs(REAL);

        case vtSTRING:
            dVal = (strcmp(nul2mty(lVal),nul2mty(rVal)) < 0);
            break;

        ltArgs(TIME);
        ltArgs(BYTE);
        ltArgs(WORD);
        ltArgs(SBYTE);
        ltArgs(SWORD);

        case vtBIGSTR:
        {
            int leftshort = FALSE;
            int minlen = rVal.values.vBIGSTR.len;

            if (minlen > lVal.values.vBIGSTR.len)
            {
                leftshort = TRUE;
                minlen = lVal.values.vBIGSTR.len;
            }

            int cmpStat = memcmp(lVal.values.vBIGSTR.ptr,
                rVal.values.vBIGSTR.ptr,minlen);

            dVal = ((cmpStat < 0) || ((cmpStat == 0) && leftshort));

            break;
        }

        default:
            dVal = 0;                     // This case should never happen. gotta satisfy lint.
            break;

        ltArgs(DREAL);
    }

    return dVal;
}

/******************************************************************************/

// cVARVAL Member Functions (Credit Card Functions)

void LIBENTRY ccStripChars(char * s)
{
    while (*s != '\0')
    {
        if (isascii(*s) && isdigit(*s))
            ++s;
        else
            strcpy(s,s+1);              // Strip non-digits
    }
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::valCC(void)
{
    cVARVAL s(vtSTRING);
    s = *this;

    char * ptr = nul2mty(s);
    ccStripChars(ptr);

    int acc = 0;
    int slen = strlen(ptr);

    // If the card type is unknown, force no length to fail
    if (strcmp(s.ccType().values.vSTRING,"UNKNOWN") == 0) slen = 0;

    if (slen == 0) return cVARVAL(vtBOOLEAN,tINTEGER(FALSE));

    int prevchar = ptr[slen-1]-'0';

    for (int i = 0; i < slen-1; ++i)
    {
        int curchar = *(ptr+slen-2-i)-'0';
        if ((slen&1) == ((slen-1-i)&1))
        {
            acc += curchar;
        }
        else
        {
            curchar *= 2;
            if (curchar > 9) curchar -= 9;
            acc += curchar;
        }
    }

    acc %= 10;
    acc = (10-acc)%10;

    return cVARVAL(vtBOOLEAN,(int) (acc == prevchar));
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::fmtCC(void)
{
    cVARVAL s(vtSTRING);
    s = *this;

        char * ptr = nul2mty(s);
    ccStripChars(ptr);

    char buf[MAX_STR_LEN];

    switch (strlen(ptr))
    {
        case 13: // 4444 333 333 333
            sprintf(buf,"%-4.4s %-3.3s %-3.3s %-3.3s",
                ptr+0,ptr+4,ptr+7,ptr+10);
            break;

//        case 14: // EEEEEEEEEEEEEE
//            break;

        case 15: // 4444 666666 55555
            sprintf(buf,"%-4.4s %-6.6s %-5.5s",
                ptr+0,ptr+4,ptr+10);
            break;

        case 16: // 4444 4444 4444 4444
            sprintf(buf,"%-4.4s %-4.4s %-4.4s %-4.4s",
                ptr+0,ptr+4,ptr+8,ptr+12);
            break;

        default:
            strcpy(buf,ptr);
            break;
    }

    return cVARVAL(vtSTRING,buf);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::ccType(void)
{
    static bool inUse = FALSE;

    bool valid = TRUE;

    if (!inUse)
    {
        inUse = TRUE;
        valid = valCC().values.vBOOLEAN;
        inUse = FALSE;
    }

    cVARVAL s(vtSTRING);
    s = *this;

    char * ptr = nul2mty(s);
    ccStripChars(ptr);

    char * rv = "UNKNOWN";

    if (valid && (strlen(ptr) >= 13) && (strlen(ptr) <= 16))
    {
        char c = ptr[4];
        int prefix = atoi(ptr);
        ptr[4] = c;

        if ((((prefix >= 3088) && (prefix <= 3094)) ||
             ((prefix >= 3096) && (prefix <= 3102)) ||
                         ((prefix >= 3112) && (prefix <= 3120)) ||
                         ((prefix >= 3158) && (prefix <= 3159)) ||
                         ((prefix >= 3337) && (prefix <= 3349)) ||
                         ((prefix >= 3528) && (prefix <= 3589))) && (strlen(ptr) == 16))
                        rv = "JCB";
        else if ((memcmp(ptr,"60",  2) == 0) && (strlen(ptr) == 16))
                        rv = "DISCOVER";
        else if ((memcmp(ptr,"389", 3) == 0) && (strlen(ptr) == 14))
                        rv = "CARTE BLANCHE";
        else if ((memcmp(ptr,"38",  2) == 0) && (strlen(ptr) == 14))
                        rv = "DINERS CLUB";
        else if ((memcmp(ptr,"9",   1) == 0) && (strlen(ptr) == 14))
                        rv = "CARTE BLANCHE";
        else if ((memcmp(ptr,"36",  2) == 0) && (strlen(ptr) == 14))
                        rv = "DINERS CLUB";
        else if ((memcmp(ptr,"30",  2) == 0) && (strlen(ptr) == 14))
                        rv = "DINERS CLUB";
        else if ((memcmp(ptr,"3737",4) == 0) && (strlen(ptr) == 15))
                        rv = "OPTIMA";
        else if ((memcmp(ptr,"37",  2) == 0) && (strlen(ptr) == 15))
                        rv = "AMERICAN EXPRESS";
        else if ((memcmp(ptr,"34",  2) == 0) && (strlen(ptr) == 15))
                        rv = "AMERICAN EXPRESS";
        else if ((memcmp(ptr,"4",   1) == 0) && ((strlen(ptr) == 13) || (strlen(ptr) == 16)))
                        rv = "VISA";
        else if ((memcmp(ptr,"5",   1) == 0) && (strlen(ptr) == 16))
                        rv = "MASTERCARD";
    }

    return cVARVAL(vtSTRING,rv);
}

/******************************************************************************/

// cVARVAL Member Functions (Bit Functions)

cVARVAL LIBENTRY cVARVAL::bitStat(cVARVAL & bit)
{
    cVARVAL tmpVar (*this);
    cVARVAL tmpInt (vtINTEGER);
    cVARVAL tmpBool(vtBOOLEAN);

    tmpInt = bit;

    void _FAR_ * bitPtr = tmpVar.varAddr();
    int        bitMax = tmpVar.varSize()*8;

    if ((tmpInt.values.vINTEGER >= 0) && (tmpInt.values.vINTEGER < bitMax))
        tmpBool.values.vBOOLEAN = (isset(bitPtr,int(tmpInt.values.vINTEGER)) ?
            TRUE : FALSE);

    return tmpBool;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::bitSet(cVARVAL & bit)
{
    cVARVAL tmpVar(*this);
    cVARVAL tmpInt(vtINTEGER);

    tmpInt = bit;

    void _FAR_ * bitPtr = tmpVar.varAddr();
    int        bitMax = tmpVar.varSize()*8;

    if ((tmpInt.values.vINTEGER >= 0) && (tmpInt.values.vINTEGER < bitMax))
        setbit(bitPtr,int(tmpInt.values.vINTEGER));

    return tmpVar;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::bitClear(cVARVAL & bit)
{
    cVARVAL tmpVar(*this);
    cVARVAL tmpInt(vtINTEGER);

    tmpInt = bit;

    void _FAR_ * bitPtr = tmpVar.varAddr();
    int          bitMax = tmpVar.varSize()*8;

    if ((tmpInt.values.vINTEGER >= 0) && (tmpInt.values.vINTEGER < bitMax))
        unsetbit(bitPtr,int(tmpInt.values.vINTEGER));

    return tmpVar;
}

/******************************************************************************/

// cVARVAL Member Functions (Date Functions)

cVARVAL LIBENTRY cVARVAL::year(void)
{
    // NOTE:  6 is year offset within string
    return datePart(*this,6);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::month(void)
{
    // NOTE:  0 is month offset within string
    return datePart(*this,0);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::day(void)
{
    // NOTE:  3 is the day of the month offset within string
    return datePart(*this,3);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::dow(void)
{
    cVARVAL d(vtDATE);
    d = *this;

    // NOTE:  This formula returns 0 for sunday, 1 for monday, etc.
    //        1 Jan 1900 was a sunday; add 0 to get to the first sunday
    //        Then divide by 7 and return remainder
    return cVARVAL(vtINTEGER,(long(d.values.vDATE)+0)%7);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::valDate(void)
{
    cVARVAL tmp(vtSTRING);
    tmp = *this;
    char * p = nul2mty(tmp);

    int flag = TRUE;
    unsigned short arr[3];

    breakdate(arr,p);
    int2amerDate(arr);

    if ((arr[0] < 1) || (arr[0] > 12) ||
        (arr[1] < 1) || (arr[1] > 31) ||
                        (arr[2] > 99))
        flag = FALSE;

    return cVARVAL(vtBOOLEAN,flag);
}

/******************************************************************************/

// cVARVAL Member Functions (Numerical Functions)

cVARVAL LIBENTRY cVARVAL::random(void)
{
    cVARVAL i(vtINTEGER);
    i = *this;
    return cVARVAL(vtINTEGER,lrand()%(++i.values.vINTEGER));
}

    /*--------------------------------------------------------------------*/

#define iabs(x) abs(x)

#define absArg(t,p) case vt##t: dVal.values.v##t = ::p##abs(lVal.values.v##t); break

cVARVAL LIBENTRY cVARVAL::abs(void)
{
    int dType = type;

    switch (dType)
    {
        case vtBOOLEAN: case vtUNSIGNED: case vtDATE: case vtEDATE:
        case vtMONEY:   case vtTIME:     case vtBYTE: case vtWORD: case vtDDATE:
            dType = vtINTEGER;
            break;

        case vtSTRING:  case vtBIGSTR:
            dType = STR2NUM;
            break;
    }

    cVARVAL dVal(dType), lVal(dType);

    lVal = *this;

    switch (dType)
    {
        absArg(INTEGER,l);
        absArg(REAL,f);
        absArg(SBYTE,i);
        absArg(SWORD,i);
        absArg(DREAL,f);
    }

    return dVal;
}

    /*--------------------------------------------------------------------*/

#define fpow(x,y) pow(x,y)
#define ipow(x,y) lpow(x,y)

#define expArg(t1,p) case vt##t1: dVal.values.v##t1 = (t##t1)p##pow(lVal.values.v##t1,rVal.values.v##t1); break

cVARVAL LIBENTRY cVARVAL::exp(cVARVAL & rvalue)
{
    int dType = type;

    switch (dType)
    {
        case vtBOOLEAN: case vtUNSIGNED: case vtDATE: case vtEDATE:
        case vtMONEY:   case vtTIME:     case vtBYTE: case vtWORD: case vtDDATE:
            dType = vtINTEGER;
            break;

        case vtSTRING:  case vtBIGSTR:
            dType = STR2NUM;
            break;
    }

    cVARVAL dVal(dType), lVal(dType), rVal(dType);

    lVal = *this;
    rVal = rvalue;

    switch (dType)
    {
        expArg(INTEGER,l);
        expArg(REAL,f);
        expArg(SBYTE,i);
        expArg(SWORD,i);
        expArg(DREAL,f);
    }

    return dVal;
}

/******************************************************************************/

// *** NOTE *** Everything from this point forward is saved in the OLDVAR.CPP file ***

// cVARVAL Member Functions (String Manipulation)

cVARVAL LIBENTRY cVARVAL::stripatx(void)
{
    cVARVAL s(vtBIGSTR);
    s = *this;

    int    i;
    char * p;

    for (i = 0, p = s.values.vBIGSTR.ptr; i < (s.values.vBIGSTR.len-4+1);
        ++i, ++p)
    {
        if ((p[0] == '@') && (p[1] == 'X') && isHEX(p[2]) && isHEX(p[3]))
        {
            memcpy(p,p+4,s.values.vBIGSTR.len-i-4);
            s.values.vBIGSTR.len -= 4;
            --i;
            --p;
        }
    }

    return s;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::len(void)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL rVal(vtBIGSTR);

    rVal = *this;
    dVal = (signed long int) rVal.values.vBIGSTR.len;

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::lower(void)
{
    cVARVAL dVal(vtBIGSTR);

    dVal = *this;

    int    i;
    char * p;

    for (i = 0, p = dVal.values.vBIGSTR.ptr; i < dVal.values.vBIGSTR.len;
        ++i, ++p)
        if (isascii(*p) && isupper(*p))
            *p = tolower(*p);

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::upper(void)
{
    cVARVAL dVal(vtBIGSTR);

    dVal = *this;

    int    i;
    char * p;

    for (i = 0, p = dVal.values.vBIGSTR.ptr; i < dVal.values.vBIGSTR.len;
        ++i, ++p)
        if (isascii(*p) && islower(*p))
            *p = toupper(*p);

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::mixed(void)
{
    cVARVAL dVal(vtBIGSTR);
    char    nullMap[MAX_BIGSTR_LEN/8+1];
    int     i;

    dVal = *this;
    memset(nullMap,0,sizeof(nullMap));

    for (i = 0; i < dVal.values.vBIGSTR.len; ++i)
    {
        char * ptr = dVal.values.vBIGSTR.ptr+i;
        if (*ptr == '\0')
        {
            *ptr = ' ';
            setbit(nullMap,i);
        }
    }

    proper(nul2mty(dVal));

    for (i = 0; i < dVal.values.vBIGSTR.len; ++i)
        if (isset(nullMap,i))
            dVal.values.vBIGSTR.ptr[i] = '\0';

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::mid(cVARVAL & pos, cVARVAL & strLen)
{
    cVARVAL sVal(vtBIGSTR);
    cVARVAL pVal(vtINTEGER);
    cVARVAL lVal(vtINTEGER);

    sVal = *this;
    pVal = pos;
    lVal = strLen;

    if (lVal.values.vINTEGER < 0)
        lVal.values.vINTEGER = 0;
    else if (lVal.values.vINTEGER > MAX_BIGSTR_LEN)
        lVal.values.vINTEGER = MAX_BIGSTR_LEN;

    char tmpbuf[MAX_BIGSTR_LEN+1], * tmpptr = tmpbuf;

    int curlen = 0;

    --pVal;

    while (curlen < lVal.values.vINTEGER)
    {
        if (((pVal.values.vINTEGER+curlen) >= 0) &&
            ((pVal.values.vINTEGER+curlen) < sVal.values.vBIGSTR.len))
            *(tmpptr++) = *(sVal.values.vBIGSTR.ptr+
                (int)pVal.values.vINTEGER+curlen);
        else
            *(tmpptr++) = ' ';
        ++curlen;
    }

    cVARVAL dVal(vtBIGSTR);
    dVal.pokeBuf(tmpbuf,(int)lVal.values.vINTEGER);

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::left(cVARVAL & strLen)
{
    cVARVAL sVal(vtBIGSTR);
    cVARVAL pVal(vtINTEGER);
    cVARVAL lVal(vtINTEGER);

    sVal = *this;
    lVal = strLen;
    pVal = (signed long int) 1;

    return sVal.mid(pVal,lVal);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::right(cVARVAL & strLen)
{
    cVARVAL sVal(vtBIGSTR);
    cVARVAL pVal(vtINTEGER);
    cVARVAL lVal(vtINTEGER);

    sVal = *this;
    lVal = strLen;
    pVal = sVal.values.vBIGSTR.len-lVal.values.vINTEGER+1;

    return sVal.mid(pVal,lVal);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::space(void)
{
    cVARVAL dVal(vtBIGSTR);
    cVARVAL tVal(vtINTEGER);

    tVal = *this;

    if (tVal.values.vINTEGER < 0)
        tVal.values.vINTEGER = 0;
    else if (tVal.values.vINTEGER > MAX_BIGSTR_LEN)
        tVal.values.vINTEGER = MAX_BIGSTR_LEN;

    char buf[MAX_BIGSTR_LEN+1];
    memset(buf,' ',(int)tVal.values.vINTEGER);
    buf[(int)tVal.values.vINTEGER] = '\0';

    dVal.pokeBuf(buf,(int)tVal.values.vINTEGER);

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::chr(void)
{
    cVARVAL dVal(vtBIGSTR);
    cVARVAL tVal(vtINTEGER);

    char buf[2];
    buf[0] = '*';
    buf[1] = '\0';

    tVal = *this;

    if (tVal.values.vINTEGER < 0)
        tVal.values.vINTEGER = 0;
    else if (tVal.values.vINTEGER > 255)
        tVal.values.vINTEGER = 255;

    *buf = char(tVal.values.vINTEGER);
    dVal.pokeBuf(buf,1);

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::asc(void)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL tVal(vtSTRING);

    tVal = *this;

    dVal.values.vINTEGER = *(unsigned char *)(nul2mty(tVal));

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::instr(cVARVAL & rvalue)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL lVal(vtBIGSTR);
    cVARVAL rVal(vtBIGSTR);

    lVal = *this;
    char * ptr = lVal.values.vBIGSTR.ptr;
    int plen = lVal.values.vBIGSTR.len;

    if (plen == 0)
    {
        dVal.values.vINTEGER = 0;
        return dVal;
    }

    rVal = rvalue;
    char * sub = rVal.values.vBIGSTR.ptr;
    int slen = rVal.values.vBIGSTR.len;

    if ((slen == 0) || (slen > plen))
    {
        dVal.values.vINTEGER = 0;
        return dVal;
    }

    int    i;
    char * p;

    for (i = 0, p = ptr; i < (plen-slen+1); ++i, ++p)
        if ((*p == *sub) && (memcmp(p,sub,slen) == 0))
            break;

    dVal.values.vINTEGER = ((i == (plen-slen+1)) ? 0L : i+1);

    return dVal;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::instrr(cVARVAL & rvalue)
{
    cVARVAL dVal(vtINTEGER);
    cVARVAL lVal(vtBIGSTR);
    cVARVAL rVal(vtBIGSTR);

    lVal = *this;
    char * ptr = lVal.values.vBIGSTR.ptr;
    int plen = lVal.values.vBIGSTR.len;

    if (plen == 0)
    {
        dVal.values.vINTEGER = 0;
        return dVal;
    }

    rVal = rvalue;
    char * sub = rVal.values.vBIGSTR.ptr;
    int slen = rVal.values.vBIGSTR.len;

    if ((slen == 0) || (slen > plen))
    {
        dVal.values.vINTEGER = 0;
        return dVal;
    }

    int    i;
    char * p;

    for (i = (plen-slen+1), p = ptr+(plen-slen); i > 0; --i, --p)
        if ((*p == *sub) && (memcmp(p,sub,slen) == 0))
            break;

    dVal.values.vINTEGER = (i == 0 ? 0L : i+1);

    return dVal;
}


    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::strip(cVARVAL & cv)
{
    return replace(cv,cVARVAL(vtSTRING,""));
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::replace(cVARVAL & sv, const cVARVAL & dv)
{
    cVARVAL r(vtBIGSTR);
    cVARVAL s(vtBIGSTR);
    cVARVAL d(vtBIGSTR);

    r = *this;
    s = sv;
    d = dv;

    if (s.values.vBIGSTR.len == 0) return r;
    // Commented out because STRIP will send a 0 length string
    //if (d.values.vBIGSTR.len == 0) return r;

    char sc = *s.values.vBIGSTR.ptr;

    // If vBIGSTR.ptr is null, then the value of dc is irrelevant
    // This line will prevent OS/2 from trapping on a null pointer reference
    char dc = (d.values.vBIGSTR.ptr ? *d.values.vBIGSTR.ptr : ' ');

    for (int i = 0; i < r.values.vBIGSTR.len; ++i)
    {
        if (r.values.vBIGSTR.ptr[i] != sc) continue;
        if (d.values.vBIGSTR.len == 0)
        {
            strcpy(r.values.vBIGSTR.ptr+i,r.values.vBIGSTR.ptr+i+1);
            --i;
            --r.values.vBIGSTR.len;
            continue;
        }
        r.values.vBIGSTR.ptr[i] = dc;
    }

    return r;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::stripstr(cVARVAL & cv)
{
    cVARVAL _empty(vtSTRING,""); return replacestr(cv,_empty);
    //return replace(cv,cVARVAL(vtSTRING,""));
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::replacestr(cVARVAL & sv, cVARVAL & dv)
{
    cVARVAL r(vtBIGSTR);
    cVARVAL s(vtBIGSTR);
    cVARVAL d(vtBIGSTR);
    int i;

    r = *this;
    s = sv;

    if ((s.values.vBIGSTR.len == 0) ||
        (r.values.vBIGSTR.len < s.values.vBIGSTR.len))
        return r;

    d = dv;

    char buf[MAX_BIGSTR_LEN+1], * p = buf;
    int length = 0;

    for (i = 0; (i < (r.values.vBIGSTR.len-s.values.vBIGSTR.len+1)) &&
        (length < MAX_BIGSTR_LEN); ++i)
    {
        if (memcmp(r.values.vBIGSTR.ptr+i,s.values.vBIGSTR.ptr,
            s.values.vBIGSTR.len) != 0)
        {
            *(p++) = r.values.vBIGSTR.ptr[i];
            ++length;
        }
        else if (d.values.vBIGSTR.len <= (MAX_BIGSTR_LEN-length))
        {
            memcpy(p,d.values.vBIGSTR.ptr,d.values.vBIGSTR.len);
            p += d.values.vBIGSTR.len;
            length += d.values.vBIGSTR.len;
            i += s.values.vBIGSTR.len-1;
        }
        else
        {
            memcpy(p,d.values.vBIGSTR.ptr,MAX_BIGSTR_LEN-length);
            p += MAX_BIGSTR_LEN-length;
            length += MAX_BIGSTR_LEN-length;
            i += s.values.vBIGSTR.len-1;
        }
    }

    if (i < r.values.vBIGSTR.len)
    {
        memcpy(p,r.values.vBIGSTR.ptr+i,s.values.vBIGSTR.len-1);
        p += s.values.vBIGSTR.len-1;
        length += s.values.vBIGSTR.len-1;
    }

    r.pokeBuf(buf,length);

    return r;
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::ltrim(cVARVAL & rvalue)
{
    return trimit(*this,rvalue,pplstripleft);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::rtrim(cVARVAL & rvalue)
{
    return trimit(*this,rvalue,pplstripright);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::trim(cVARVAL & rvalue)
{
    return  ltrim(rvalue).rtrim(rvalue);
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
    sprintf(buf,"%2ld%s%02ld%s%02ld %s",hr,Country.TimeSep,
        t.min().values.vINTEGER,Country.TimeSep,
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
    return dayPart(*this,    60L*60L,    60L);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::sec(void)
{
    return dayPart(*this,        60L,     1L);
}

    /*--------------------------------------------------------------------*/

cVARVAL LIBENTRY cVARVAL::valTime(void)
{
    cVARVAL tmp(vtSTRING);
    tmp = *this;
    char * p = nul2mty(tmp);

    int flag = TRUE;

    char * endptr;
    long thisHour = strtol(p,&endptr,10);
    if ((*endptr != *Country.TimeSep) && (*endptr != ':'))
        flag = FALSE;

    long thisMin = strtol(++endptr,&endptr,10);
    if ((*endptr != *Country.TimeSep) && (*endptr != ':') && (*endptr != '\0'))
        flag = FALSE;

    long thisSec = 0;

    if (flag && (*(endptr++) != '\0'))
    {
        thisSec = strtol(endptr,&endptr,10);
        if (*endptr != '\0') flag = FALSE;
    }

    if ((thisHour < 0) || (thisHour > 23)) flag = FALSE;
    if ((thisMin  < 0) || (thisMin  > 59)) flag = FALSE;
    if ((thisSec  < 0) || (thisSec  > 59)) flag = FALSE;

    return cVARVAL(vtBOOLEAN,flag);
}

/******************************************************************************/



#endif

//#ifdef ___COMP___
//
//#pragma argsused
//void S4FUNCTION date4assign( char *date_ptr, long ldate )
//{
//}
//
//#pragma argsused
//long S4FUNCTION date4long( char *date_ptr )
//{
//    return 0L;
//}
//#endif
