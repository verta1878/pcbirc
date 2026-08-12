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
/*                                  VAR.HPP                                   */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Data and functions for the cVAR and related classes.            */
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

#ifndef ___VAR_HPP___

#define ___VAR_HPP___

/******************************************************************************/

// Included Files

#include <string.h>
#include <types.hpp>
#include <list.hpp>

/******************************************************************************/

// Defined Macros

#define NUM_TYPES   18

#define vtBOOLEAN    0  /* 1-byte unsigned integer -- 0 or 1 (FALSE or TRUE) */

#define vtUNSIGNED   1  /* 4-byte unsigned integer -- used to be vtCC */

#define vtDATE       2  /* 2-byte unsigned integer -- julian date */
#define vtEDATE      3  /* 2-byte unsigned integer -- julian date */
#define vtINTEGER    4  /* 4-byte signed   integer */
#define vtMONEY      5  /* 4-byte signed   integer -- cents */
#define vtREAL       6  /* 4-byte floating point number -- IEEE floating point */
#define vtSTRING     7  /* 256-byte maximum length pointer based string -- no 0 bytes allowed in the middle of the string */
#define vtTIME       8  /* 4-byte unsigned integer -- seconds elapsed since midnight */

#define vtBYTE       9  /* 1-byte unsigned integer */
#define vtWORD      10  /* 2-byte unsigned integer */
#define vtSBYTE     11  /* 1-byte signed   integer */
#define vtSWORD     12  /* 2-byte signed   integer */
#define vtBIGSTR    13  /* 2048-byte maximum length string -- all bytes allowed, including 0 bytes */
#define vtDREAL     14  /* 8-byte floating point number */
#define vtFUNCTION  15  /* 2 byte unsigned int */
#define vtPROCEDURE 16  /* 2-byte unsigned int */
#define vtDDATE     17  /* 4 byte signed long */

#define MAX_STR_LEN     256
#define MAX_BIGSTR_LEN  2048

#ifdef ___COMP___
//#define bmalloc malloc
//#define bfree   free
//#define bstrdup strdup
#endif

/******************************************************************************/

// Constants

#if defined(___COMP___) || defined(___INFO___)

extern char * typeText [];

#endif

/******************************************************************************/

// Types

typedef  unsigned char BYTE;
#define  MAX_BYTE_SIZE 255

class cVARVAL;

#pragma pack(1)
struct sBIGSTR
{
    sint   size;              // DWT:  changed int to sint
    char * ptr;
    sint   len;               // DWT:  changed int to sint
};
#pragma pack()

    /*--------------------------------------------------------------------*/

#pragma pack(1)
struct sSTDSTRUCT
{
    long   lo;
    long   hi;
};
#pragma pack()

    /*--------------------------------------------------------------------*/

// This structure uses all available bytes in uVARVAL to hold information
// about functions and procedures. It is possible in the future
// that the retId field can contain a bit field containing information
// related to how a parameter is passed to a procedure. By name or value.

#pragma pack(1)
struct funcINFO
{
    BYTE     params;    // Number of parameters
    BYTE  locvarcnt;    // a count of how many local variables in a proc
    uint     offset;    // code offset                       DWT:  changed unsigned int to uint
    sint    firstId;    // Id number of the first parameter  DWT:  changed int to sint
    sint      retId;    // Id for return variable            DWT:  changed int to sint
};
#pragma pack()

    /*--------------------------------------------------------------------*/

#pragma pack(1)
struct procINFO
{
    BYTE     params;    // params and locvarcnt ar limited to 255
    BYTE  locvarcnt;
    uint     offset;    // DWT: changed unsigned int to uint
    sint    firstId;    // DWT: changed int to sint
    uint  passFlags;    // This is a bit field of sizeof(int) to flag pass by reference in parameters DWT: changed unsigned int to uint
};
#pragma pack()


    /*--------------------------------------------------------------------*/

typedef unsigned char   tBOOLEAN;
typedef unsigned long   tUNSIGNED;
typedef uint            tDATE;          // DWT: changed from unsigned int to uint
typedef uint            tEDATE;         // DWT: changed from unsigned int to uint
typedef signed   long   tINTEGER;
typedef signed   long   tMONEY;
typedef float           tREAL;
typedef char          * tSTRING;
typedef signed long     tTIME;
typedef unsigned char   tBYTE;
typedef uint            tWORD;          // DWT: changed from unsigned int to uint
typedef signed   char   tSBYTE;
typedef sint            tSWORD;         // DWT: changed from signed int to sint
typedef sBIGSTR         tBIGSTR;
typedef double          tDREAL;
typedef funcINFO        tFUNCTION;
typedef procINFO        tPROCEDURE;
typedef signed   long   tDDATE;
typedef sSTDSTRUCT      tSTDVARSIZE;

    /*--------------------------------------------------------------------*/

// NOTE:  This structure is read and written to disk.
//        Be *very* careful about changing!!!

#pragma pack(1)
struct sVARINFO
{
    sint            id;    // The variable ID          DWT: changed from int to sint
    unsigned char   dims;  // The variable dimensions
    sint            vs;    // The vector size          DWT: changed from int to sint
    sint            ms;    // The maxtrix size         DWT: changed from int to sint
    sint            cs;    // The cube size            DWT: changed from int to sint
    unsigned char type;    // The variable type
    // SDR: This __BORANDC__ should probably be __BORLANDC__ but I don't know since I didn't do it
    #ifdef __BORANDC__
      struct{                // This structure must not take more than 1 byte total
       int            init   :1; // 1 bit field to tell runtime mod to reinit a var
       int            rest   :7; // other 7 bits open for use.
      }flags;
    #else
      struct{                // This structure must not take more than 1 byte total
       char           init   :1; // 1 bit field to tell runtime mod to reinit a var
       char           rest   :7; // other 7 bits open for use.
      }flags;
    #endif
};
#pragma pack()

    /*--------------------------------------------------------------------*/

#pragma pack(1)
union uVARVAL
{
    tBOOLEAN    vBOOLEAN;
    tUNSIGNED   vUNSIGNED;
    tDATE       vDATE;
    tEDATE      vEDATE;
    tINTEGER    vINTEGER;
    tMONEY      vMONEY;
    tREAL       vREAL;
    tSTRING     vSTRING;
    tTIME       vTIME;
    tBYTE       vBYTE;
    tWORD       vWORD;
    tSBYTE      vSBYTE;
    tSWORD      vSWORD;
    tBIGSTR     vBIGSTR;
    tDREAL      vDREAL;
    tFUNCTION   vFUNCTION;
    tPROCEDURE  vPROCEDURE;
    tDDATE      vDDATE;
    tSTDVARSIZE vSTDVARSIZE;
};
#pragma pack()

    /*--------------------------------------------------------------------*/

#pragma pack(1)
class cVARVAL
{

    public:

     // static bool fastNewDel;
        static int  fastNewDelPtr;
        static void * operator new ( size_t size );
        static void operator delete ( void * ptr );

        // Construtors
        cVARVAL(const cVARVAL & a);             // Copy constructor

        cVARVAL(int _type);                     // Set type and init to 0

        cVARVAL(int _type, tINTEGER _value);    // Set type and assign INTEGER
        cVARVAL(int _type, tDREAL   _value);    // Set type and assign DREAL
        cVARVAL(int _type, tSTRING  _value);    // Set type and assign STRING

        cVARVAL(int _type, int           _value);
        cVARVAL(int _type, unsigned      _value);
        cVARVAL(int _type, unsigned long _value);

        // Destructor
        virtual ~cVARVAL(void);

        // Used by constructors to set default values
        void       LIBENTRY initialize ( void );

        // Used to get the 'size' and 'address' of the variable
        void _FAR_ * LIBENTRY varAddr    ( void );
        int          LIBENTRY varSize    ( void );
        int          LIBENTRY varMaxSize ( void );

        void         LIBENTRY pokeBuf    ( char _FAR_ * buf, int bufLen );

        // Assignment operators
              cVARVAL & operator = (const cVARVAL & rvalue);

        const cVARVAL & operator = (tINTEGER  rvalue);
        const cVARVAL & operator = (tDREAL    rvalue);
        const cVARVAL & operator = (tSTRING   rvalue);
        const cVARVAL & operator = (int       rvalue);
        const cVARVAL & operator = (unsigned  rvalue);

        // Arithmetic operators
        cVARVAL   operator - (void);

        cVARVAL   operator +  (cVARVAL & rvalue);
        cVARVAL   operator -  (cVARVAL & rvalue);
        cVARVAL   operator *  (cVARVAL & rvalue);
        cVARVAL   operator /  (cVARVAL & rvalue);
        cVARVAL   operator %  (cVARVAL & rvalue);

        cVARVAL & operator ++ ( void );
        cVARVAL & operator -- ( void );

                  operator char * ();
                  operator long   ();
        // Functions
        cVARVAL LIBENTRY exp        (cVARVAL & rvalue);

        cVARVAL LIBENTRY abs        (void);

        cVARVAL LIBENTRY len        (void);
        cVARVAL LIBENTRY lower      (void);
        cVARVAL LIBENTRY upper      (void);
        cVARVAL LIBENTRY mixed      (void);
        cVARVAL LIBENTRY mid        (cVARVAL & pos, cVARVAL & strLen);
        cVARVAL LIBENTRY left       (cVARVAL & strLen);
        cVARVAL LIBENTRY right      (cVARVAL & strLen);
        cVARVAL LIBENTRY space      (void);
        cVARVAL LIBENTRY chr        (void);
        cVARVAL LIBENTRY asc        (void);
        cVARVAL LIBENTRY instr      (cVARVAL & rvalue);
        cVARVAL LIBENTRY instrr     (cVARVAL & rvalue);
        cVARVAL LIBENTRY ltrim      (cVARVAL & rvalue);
        cVARVAL LIBENTRY rtrim      (cVARVAL & rvalue);
        cVARVAL LIBENTRY trim       (cVARVAL & rvalue);
        cVARVAL LIBENTRY random     (void);
        cVARVAL LIBENTRY year       (void);
        cVARVAL LIBENTRY month      (void);
        cVARVAL LIBENTRY day        (void);
        cVARVAL LIBENTRY dow        (void);
        cVARVAL LIBENTRY hour       (void);
        cVARVAL LIBENTRY min        (void);
        cVARVAL LIBENTRY sec        (void);
        cVARVAL LIBENTRY timeap     (void);
        cVARVAL LIBENTRY stripatx   (void);
        cVARVAL LIBENTRY replace    (cVARVAL & sv, const cVARVAL & dv);
        cVARVAL LIBENTRY strip      (cVARVAL & cv);
        cVARVAL LIBENTRY replacestr (cVARVAL & sv, cVARVAL & dv);
        cVARVAL LIBENTRY stripstr   (cVARVAL & cv);
        cVARVAL LIBENTRY valDate    (void);
        cVARVAL LIBENTRY valTime    (void);
        cVARVAL LIBENTRY valCC      (void);
        cVARVAL LIBENTRY fmtCC      (void);
        cVARVAL LIBENTRY ccType     (void);

        cVARVAL LIBENTRY bitStat    (cVARVAL & bit);
        cVARVAL LIBENTRY bitSet     (cVARVAL & bit);
        cVARVAL LIBENTRY bitClear   (cVARVAL & bit);

        // Equality operators
        int operator == (cVARVAL & rvalue);
        int operator != (cVARVAL & rvalue) { return !(*this == rvalue); }
        int operator <  (cVARVAL & rvalue);
        int operator <= (cVARVAL & rvalue) { return ((*this <  rvalue) || (*this == rvalue)); }
        int operator >  (cVARVAL & rvalue) { return !(*this <= rvalue); }
        int operator >= (cVARVAL & rvalue) { return !(*this <  rvalue); }

        // Logical operators
        int operator !  (void);
        int operator && (cVARVAL & rvalue);
        int operator || (cVARVAL & rvalue);

        // Data members
        uint    type;         // DWT: changed from int to uint
        uVARVAL values;

};
#pragma pack()

    /*--------------------------------------------------------------------*/

class cVAR
{
    public:

        // Constructor
        cVAR(char * _name, sVARINFO & _info);

        // Destructor
        ~cVAR(void);

        int LIBENTRY redimArray(unsigned char _dims, int _vv, int _mv, int _cv);

        // Equality operators
        int operator == (cVAR & r) const { return stricmp(name,r.name) == 0; }
        int operator != (cVAR & r) const { return stricmp(name,r.name) != 0; }
        int operator <  (cVAR & r) const { return stricmp(name,r.name) <  0; }
        int operator <= (cVAR & r) const { return stricmp(name,r.name) <= 0; }
        int operator >  (cVAR & r) const { return stricmp(name,r.name) >  0; }
        int operator >= (cVAR & r) const { return stricmp(name,r.name) >= 0; }

        // Match functions for tVARLST
//        static int  LIBENTRY varNameMatch   (const cVAR & v, const void * s);
//        static int  LIBENTRY varIDMatch     (const cVAR & v, const void * i);
//        static int  LIBENTRY varStringMatch (const cVAR & v, const void * s);

//        static void LIBENTRY delNullName    (cVAR & v);

        cVARVAL *   LIBENTRY getVal         (int vn = 0, int mn = 0, int cn = 0);

        void        LIBENTRY sort           (cVAR * i);

        // Data members

        char     * name;
        int        elems;
        cVARVAL  * data;
        sVARINFO   info;
#if defined(___INFO___)
        long       refs;
#endif

};

    /*--------------------------------------------------------------------*/

// typedef cILIST<cVAR> tVARLST;

typedef cVAR * cVARPTR;

class cVARLST
{

    public:

        cVARLST                     ( int _size );

        ~cVARLST                    ( void );

        static int    findNameCmp   ( const void * lv, const void * rv );
        static int    findStringCmp ( const void * lv, const void * rv );
        static int    sortIDRevCmp  ( const void * lv, const void * rv );

        int    LIBENTRY add           ( cVAR * data );

        int    LIBENTRY used          ( void );

        cVAR * LIBENTRY findName      ( char * name );
        cVAR * LIBENTRY findString    ( char * str );

        void   LIBENTRY sortIDReverse ( void );

        void   LIBENTRY forEach       ( void LIBENTRY (* func)(cVAR & obj) );

    protected:

    private:

        int       size;
        int       next;
        cVARPTR * list;

        int       firstSysVar;

};

typedef cVARLST tVARLST;

/******************************************************************************/

// Variables

/******************************************************************************/

// Function Prototypes

int     LIBENTRY isMoney       ( char * s );
int     LIBENTRY isBinary      ( char * s );
int     LIBENTRY isOctal       ( char * s );
int     LIBENTRY isDecimal     ( char * s );
int     LIBENTRY isHexadecimal ( char * s );
int     LIBENTRY isUInteger    ( char * s );
int     LIBENTRY isInteger     ( char * s );
int     LIBENTRY isReal        ( char * s );
int     LIBENTRY isString      ( char * s );
int     LIBENTRY isDate        ( char * s );
int     LIBENTRY isTime        ( char * s );

void    LIBENTRY str2time      ( char * s, tTIME & t );

cVARVAL LIBENTRY inkey         ( void );
cVARVAL LIBENTRY kinkey        ( void );
cVARVAL LIBENTRY minkey        ( void );
cVARVAL LIBENTRY tinkey        ( long ticks );

char *  LIBENTRY nul2mty       (const cVARVAL & v );


/******************************************************************************/

// Inline Functions

/******************************************************************************/

#endif

