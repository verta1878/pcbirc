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
/*                                ATCODES.CPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Functions for checking, finding and evaluation @Codes and hex digits    */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Included Files

#pragma hdrfile "atcodes.sym"

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    <atcodes.hpp>

#include    <miscedit.hpp>
#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Defined Macros

/******************************************************************************/

// Types

/******************************************************************************/

// Variables

// List of macros in order returned by David's function

// 001 - ALIAS              // V15.1
//       AUTOMORE
//       BEEP
//       BICPS              // V15.0
// 005 - BOARDNAME
//       BPS
//       BYTECREDIT         // V15.1
//       BYTELIMIT
//       BYTERATIO
// 010 - BYTESLEFT
//       CARRIER            // V15.0
//       CITY
//       CLREOL
//       CLS
// 015 - CONFNAME
//       CONFNUM
//       CREDLEFT           // V15.2
//       CREDNOW            // V15.2
//       CREDSTART          // V15.2
// 020 - CREDUSED           // V15.2
//       CURMSGNUM
//       DATAPHONE
//       DAYBYTES
//       DELAY              // V15.0
// 025 - DIRNAME            // V15.1
//       DIRNUM             // V15.1
//       DLBYTES
//       DLFILES
//       ENV                // V15.1
// 030 - EVENT
//       EXPDATE
//       EXPDAYS
//       FBYTES             // V15.1
//       FFILES             // V15.1
// 035 - FILECREDIT         // V15.1
//       FILERATIO
//       FIRSTU
//       FIRST
//       FNUM               // V15.1
// 040 - FREESPACE
//       HOMEPHONE
//       HIGHMSGNUM
//       INCONF
//       KBLEFT
// 045 - KBLIMIT
//       LASTCALLERNODE
//       LASTCALLERSYSTEM
//       LASTDATEON
//       LASTTIMEON
// 050 - LMR
//       LOGDATE            // V15.1
//       LOGTIME            // V15.1
//       LOWMSGNUM
//       MAXBYTES           // V15.1
// 055 - MAXFILES           // V15.1
//       MINLEFT
//       MORE
//       MSGLEFT
//       MSGREAD
// 060 - NOCHAR             // V15.1
//       NODE
//       NUMBLT
//       NUMCALLS
//       NUMCONF            // V15.1
// 065 - NUMDIR
//       NUMTIMESON
//       OFFHOURS
//       OPTEXT
//       PAUSE
// 070 - POFF
//       PON
//       POS                // V15.0
//       PROLTR
//       PRODESC
// 075 - PWXDATE            // V15.1
//       PWXDAYS            // V15.1
//       QOFF
//       QON
//       RATIOBYTES         // V15.1
// 080 - RATIOFILES         // V15.1
//       RBYTES             // V15.0
//       RCPS               // V15.0
//       REALNAME           // V15.1
//       RFILES             // V15.0
// 085 - SBYTES             // V15.0
//       SCPS               // V15.0
//       SECURITY
//       SFILES             // V15.0
//       SYSDATE
// 090 - SYSOPIN
//       SYSOPOUT
//       SYSTIME
//       TIMELIMIT
//       TIMELEFT
// 095 - TIMEUSED
//       TOTALTIME
//       UPBYTES
//       UPFILES
//       USER
// 100 - WAIT
//       WHO
//       XOFF               // V15.2
//       XON                // V15.2
//       YESCHAR            // V15.1
// 105 - @XXX

char *  atVarLst[] =
        {
            "", // ALWAYS LEAVE HERE!
            "@ALIAS@",
            "@AUTOMORE@",
            "@BEEP@",
            "@BICPS@",
            "@BOARDNAME@",
            "@BPS@",
            "@BYTECREDIT@",
            "@BYTELIMIT@",
            "@BYTERATIO@",
            "@BYTESLEFT@",
            "@CARRIER@",
            "@CITY@",
            "@CLREOL@",
            "@CLS@",
            "@CONFNAME@",
            "@CONFNUM@",
            "@CREDLEFT@",
            "@CREDNOW@",
            "@CREDSTART@",
            "@CREDUSED@",
            "@CURMSGNUM@",
            "@DATAPHONE@",
            "@DAYBYTES@",
            "@DELAY@",
            "@DIRNAME@",
            "@DIRNUM@",
            "@DLBYTES@",
            "@DLFILES@",
            "@ENV@",
            "@EVENT@",
            "@EXPDATE@",
            "@EXPDAYS@",
            "@FBYTES@",
            "@FFILES@",
            "@FILECREDIT@",
            "@FILERATIO@",
            "@FIRSTU@",
            "@FIRST@",
            "@FNUM@",
            "@FREESPACE@",
            "@HOMEPHONE@",
            "@HIGHMSGNUM@",
            "@INCONF@",
            "@KBLEFT@",
            "@KBLIMIT@",
            "@LASTCALLERNODE@",
            "@LASTCALLERSYSTEM@",
            "@LASTDATEON@",
            "@LASTTIMEON@",
            "@LMR@",
            "@LOGDATE@",
            "@LOGTIME@",
            "@LOWMSGNUM@",
            "@MAXBYTES@",
            "@MAXFILES@",
            "@MINLEFT@",
            "@MORE@",
            "@MSGLEFT@",
            "@MSGREAD@",
            "@NOCHAR@",
            "@NODE@",
            "@NUMBLT@",
            "@NUMCALLS@",
            "@NUMCONF@",
            "@NUMDIR@",
            "@NUMTIMESON@",
            "@OFFHOURS@",
            "@OPTEXT@",
            "@PAUSE@",
            "@POFF@",
            "@PON@",
            "@POS@",
            "@PROLTR@",
            "@PRODESC@",
            "@PWXDATE@",
            "@PWXDAYS@",
            "@QOFF@",
            "@QON@",
            "@RATIOBYTES@",
            "@RATIOFILES@",
            "@RBYTES@",
            "@RCPS@",
            "@REAL@",
            "@RFILES@",
            "@SBYTES@",
            "@SCPS@",
            "@SECURITY@",
            "@SFILES@",
            "@SYSDATE@",
            "@SYSOPIN@",
            "@SYSOPOUT@",
            "@SYSTIME@",
            "@TIMELIMIT@",
            "@TIMELEFT@",
            "@TIMEUSED@",
            "@TOTALTIME@",
            "@UPBYTES@",
            "@UPFILES@",
            "@USER@",
            "@WAIT@",
            "@WHO@",
            "@XOFF@",
            "@XON@",
            "@YESCHAR@",
            NULL
        };

char *  defAtVarValLst[] =
        {
            "", // ALWAYS LEAVE HERE!
/*001*/     "USER ALIAS",
            "",
            "",
            "3400",
/*005*/     "PCBoard Bulletin Board System",
            "19200",
            "1048576",
            "10,238,976",
            "1:1",
/*010*/     "9,238,976",
            "14400",
            "New York City, NY",
            "",
            "",
/*015*/     "Main Board",
            "0",
            "49,000",
            "1,000",
            "100,000",
/*020*/     "51,000",
            "0",
            "000 000-0000",
            "1,000,000",
            "",
/*025*/     "Demonstration Files",
            "1",
            "1,000,000",
            "0",
            "",
/*030*/     "03:00",
            "00-00-00",
            "30",
            "0",
            "0",
/*035*/     "10",
            "1:1",
            "JOHN",
            "John",
            "1",
/*040*/     "111,111,111",
            "000 000-0000",
            "16,700,000",
            "Main Board",
            "9,999",
/*045*/     "10,000",
            "JOHN DOE",
            "JOHN DOE",
            "00-00-00",
            "00:00",
/*050*/     "8,350,000",
            "11-08-93",
            "15:06",
            "1",
            "Unlimited",
/*055*/     "Unlimited",
            "60",
            "",
            "50",
            "500",
/*060*/     "N",
            "1",
            "10",
            "100",
            "60",
/*065*/     "5",
            "10",
            "23:00-01:00",
            "OPTEXT",
            "",
/*070*/     "",
            "",
            "",
            "X",
            "Xmodem Checksum",
/*075*/     "01-01-94",
            "68",
            "",
            "",
            "Unlimited",
/*080*/     "Unlimited",
            "0",
            "0",
            "REAL NAME",
            "0",
/*085*/     "0",
            "0",
            "20",
            "0",
            "00-00-00",
/*090*/     "13:00",
            "22:00",
            "00:00",
            "196",
            "194",
/*095*/     "1",
            "1",
            "1,111,111",
            "1",
            "JOHN DOE",
/*100*/     "",
            "",
            "",
            "",
            "Y",
            NULL
        };

char * atVarValLst[ATXCODE];

char    replaceableATCode[] =
        {
            0,
            1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
            0, 0, 0, 1,
            0 // ALWAYS LEAVE HERE!
        };

//unsigned char atWidth;
//char          atJust;
int           atWidth;
int           atJust;
int           atSize;
char          atBuf [ 256 + 1 ];

char          noatFlag = FALSE;
char          noatxFlag = FALSE;

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Functions

// #include <stdio.h>

int  getATX(char * s, int c, int o)
{
    // If s is a valid @X code...
    if (isATX(s))
    {
//        printf("\ngetATX - %4.4s %02X %2d",s,c,o);

        if (noatxFlag) return 0x07;

        // Get value of @X code
        int a = (hex2dec(s[2]) << 4) + hex2dec(s[3]);

        if (a == 0x00)                       // If it is a save color code...
            return c;
        else if ((a == 0xFF) && (o != -1)) // If it is a restore color code...
        {
            int last00 = attrList[0];

            for (int i = o-1; i >= 0; --i)
                if (last00List[i] != -1)
                {
                    last00 = last00List[i];
                    break;
                }

            if (last00List[o] != -1)
            {
                a = attrList[o];

                char * p = lineList[o];
                if ((o == tlOff+slOff) && (lineList[o] == NULL)) p = curLine;

                int plen = strlen(p);
                while ((plen > 0) && (p < s))
                {
                    int code = doIsATCode(p);
                    if (code == ATXCODE)
                    {
                        if (memcmp(p,"@X00",4) == 0)
                            last00 = a;
                        else if (memcmp(p,"@XFF",4) == 0)
                            a = last00;
                        else
                            a = getATX(p,a,o);
                        atSize = 4;
                    }
                    if (code)
                    {
                        p += atSize;
                        plen -= atSize;
                    }
                    else
                    {
                        ++p;
                        --plen;
                    }
                }
            }

            a = last00;
        }

        // Return new color
        return a;
    }

    // Return error code
    return -1;
}

    /*--------------------------------------------------------------------*/

#ifndef __OS2__
extern "C" {
#endif

    int  findtoken(char * s);

#ifndef __OS2__
}
#endif

// This function should only be called via doIsAtCode (an inline function)
int  isATCode(char * s)
{
    // Taken care of in doIsATCode
 // if (noatFlag || (*s != '@')) return 0;

    // Taken care of in doIsAtCode
 // if (isATX(s))
 // {
 //  // code = ATXCODE; // Hard coded in the return statement, hence commented
 //     atWidth = atJust = 0;
 //     atSize = 4; // All ATX code are four characters long
 //     *atBuf = NUL;
 //     return ATXCODE;
 // }

    union uSTART
    {
        char * p; // byte order is (LOOFF:HIOFF:LOSEG:HISEG)
        unsigned short int o; // maps to the offset of the character pointer
    };

    uSTART start;
    start.p = s;

    int code = findtoken(s);
    start.o = _BX;
    atWidth = _CL;
    atJust  = _CH;
    atSize  = _DX;

    // Replace the @ that was removed by findtoken
    if (code) *start.p = '@';

    // If the passed and returned pointers don't match, error
    if (s != start.p)
    {
        code = atSize = atWidth = atJust = 0;
        *atBuf = NUL;
    }
    else
    {
        char * ptr;

        if (code == ATENV)
        {
         // @ENV=NAME:SIZEJUST@

            char varName[128+1];
            sprintf(varName,"@%s",start.p+5);

            ptr = strchr(varName+1,'@');
            ptr[1] = NUL;

            ptr = strchr(varName,':');
            if (ptr)
                *ptr = '@';
            else
                ptr = strchr(varName+1,'@');
            ptr[1] = NUL;

            ptr = getenv(strupr(varName));
            if (ptr == NULL) ptr = "";
        }
        else
        {
            ptr = atVarValLst[code];
        }

        char tmpVarVal [ 256 + 1 ];
        if (ptr[0])
            strcpy(tmpVarVal,ptr);
        else
            tmpVarVal[0] = NUL;
        ptr = tmpVarVal;

        int len = (ptr[0] ? strlen(ptr) : 0);

        if (len > 0)
        {
            if (atJust != 'T')
            {
                if (atWidth < len)
                    atWidth = len;
            }
            else
            {
                if (len > atWidth)
                {
                    ptr[atWidth] = NUL;
                    len = atWidth;
                }
                atJust = 'L';
            }
            if (atJust == NUL) atJust = 'L';
        }
        else if (code != ATPOS)
        {
            atWidth = atJust = 0;
        }

        if (code != ATPOS)
        {
            memset(atBuf,' ',atWidth);
            atBuf[atWidth] = NUL;

            int off;

            switch (atJust)
            {
                case 'L': off = 0;                 break;
                case 'C': off = ((atWidth-len)/2); break;
                case 'R': off = atWidth-len;       break;
            }

            memcpy(atBuf+off,ptr,len);
        }
    }

    return code;
}

/******************************************************************************/

