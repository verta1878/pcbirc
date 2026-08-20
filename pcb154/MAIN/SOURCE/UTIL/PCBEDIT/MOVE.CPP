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
/*                                  MOVE.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*        Functions to allow cursor movement within an @ code document        */
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

#pragma hdrfile "move.sym"

#include    <string.h>

#include    <move.hpp>

#include    <atcodes.hpp>
#include    <keyboard.hpp>
#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Functions

void  moveLeft(void)
{
    if (curLineChar == curLine) return;

    char * tmpCLC = curLineChar;
    curLineChar = curLine;

    int tlc = lcOff;
    int tsc = scOff;
    lcOff = scOff = 0;

    char * lstCLC = curLineChar;
    int llc = 0;
    int lsc = 0;

    while (TRUE)
    {
        moveRight();
        if (curLineChar == tmpCLC) break;
        lstCLC = curLineChar;
        llc = lcOff;
        lsc = scOff;
    }

    curLineChar = lstCLC;

    lcOff = tlc;
    scOff = tsc;

    adjColOff(llc+lsc);
}

    /*--------------------------------------------------------------------*/

void  moveRight(void)
{
    if ((strlen(curLineChar) == 0) && (strlen(curLine) < MAX_LINE_SIZE))
    {
        procChar(' ');
        return;
    }

    int code = doIsATCode(curLineChar);
    if (code)
    {
        curLineChar += atSize;
        if (code == ATPOS)
        {
            if ((lcOff+scOff) < --atWidth)
                atWidth -= lcOff+scOff;
            else
                atWidth = 0;
        }
    }
    else if (*curLineChar == TAB)
    {
        ++curLineChar;
        atWidth = (((lcOff+scOff)/8)+1)*8+1-(lcOff+scOff+1);
    }
    else
    {
        ++curLineChar;
        atWidth = 1;
    }

    adjColOff(lcOff+scOff+atWidth);
}

    /*--------------------------------------------------------------------*/

void  adjustCLC(void)
{
    // Initialize variables
    curLineChar = curLine;
    int clSize = strlen(curLine);
    int clcSize = clSize;
    int clLen = lineLen(curLine);
    int clcLen = clLen;

    while ((clcSize > 0) && (clLen-clcLen < lcOff+scOff))
    {
        int code = doIsATCode(curLineChar);
        if (code == ATPOS)
        {
            if ((clLen-clcLen) < --atWidth)
                atWidth -= (clLen-clcLen);
            else
                atWidth = 0;
        }
        if (code)
        {
            curLineChar += atSize;
            clcSize -= atSize;
            clcLen -= atWidth;
        }
        else if (*curLineChar == TAB)
        {
            atWidth = (((clLen-clcLen)/8)+1)*8+1-(clLen-clcLen+1);
            ++curLineChar;
            --clcSize;
            clcLen -= atWidth;
        }
        else
        {
            ++curLineChar;
            --clcSize;
            --clcLen;
        }
    }

    if ((clLen-clcLen < lcOff+scOff) && (clSize < MAX_LINE_SIZE))
    {
        memset(curLineChar,' ',
            ((MAX_LINE_SIZE-clSize) < ((lcOff+scOff)-(clLen-clcLen))) ?
            (MAX_LINE_SIZE-clSize) : ((lcOff+scOff)-(clLen-clcLen)));
        curLineChar = curLine+strlen(curLine);
        clLen = lineLen(curLine);
        clcLen = 0;
    }

    if (clLen-clcLen != lcOff+scOff) // was <
    {
        ltlOff = -11;
        adjColOff(clLen-clcLen);
    }

    int tlc = lcOff, tsc = scOff;
    while ((lcOff+scOff == tlc+tsc) && (strlen(curLineChar) > 0))
        moveRight();
    if (lcOff+scOff > tlc+tsc) moveLeft();

    if (lcOff+scOff == tlc+tsc) lcOff = tlc, scOff = tsc;

    if (clLen-clcLen != lcOff+scOff) adjColOff(clLen-clcLen);
}

    /*--------------------------------------------------------------------*/

void  moveUp(void)
{
    if (tlOff+slOff == 0) return;

    commitLine();

    if (--slOff < 0) --tlOff, ++slOff;

    curLineChar = curLine;
    cacheLine();

    adjustCLC();
}

    /*--------------------------------------------------------------------*/

void  moveDown(void)
{
    if (tlOff+slOff == MAX_LINES-1) return;

    commitLine();

    if (++slOff > MAX_ROWS-(statOn?2:0)-1) ++tlOff, --slOff;

    curLineChar = curLine;
    cacheLine();

    adjustCLC();
}

    /*--------------------------------------------------------------------*/

void  moveEnd(void)
{
    stripSpaces();
    curLineChar = curLine+strlen(curLine);
    adjColOff(lineLen(curLine));
}

/******************************************************************************/

