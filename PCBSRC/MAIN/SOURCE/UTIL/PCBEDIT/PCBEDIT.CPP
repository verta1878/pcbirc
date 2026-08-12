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
/*                                PCBEDIT.CPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*       A program to allow easy editing of PCBoard files with @ codes.       */
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

#pragma hdrfile "pcbedit.sym"

#include    <conio.h>
#include    <dos.h>
#include    <stdarg.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <values.h>

#include    <dosfunc.h>
#include    <system.h>

#include    <atcodes.hpp>
#include    <block.hpp>
#include    <compat.hpp>
#include    <disp.hpp>
#include    <fast_cio.hpp>
#include    <file.hpp>
#include    <initrest.hpp>
#include    <keyboard.hpp>
#include    <miscedit.hpp>
#include    <move.hpp>

#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Defined Macros

/******************************************************************************/

// Types

/******************************************************************************/

// Variables

// Top line, screen line, left column, & screen column offsets
int     tlOff = 0, slOff = 0, lcOff = 0, scOff = 0;

// Last top line & last left column offsets
int     ltlOff = -11, llcOff = -11;

// Flag used to disable edit keys
int     disableEditKeys = FALSE;

// List of line pointers, attributes, last @X00 positions and @XFF flags
char *  lineList    [ MAX_LINES ];
int     attrList    [ MAX_LINES ];
int     last00List  [ MAX_LINES ];
char    isFFList    [ MAX_LINES ];
int     lastCLSList [ MAX_LINES ];

int     lastLine = MAX_LINES-1;

// Flag used to determine if text has been changed
int     textChanged = FALSE;

// Buffer to hold current line being edited and pointer to current position
char    curLine[MAX_LINE_SIZE+1+2];
char    tmpLine[MAX_LINE_SIZE+1+2];
char *  curLineChar = curLine;

// The last filename accessed
char    lastFN[PN_SIZE+1];

// Pointer to string used to terminate last line (if not null)
char *  lastLineEnd;

// Insert mode status flag
int     insMode = TRUE;

// Default color attributes for various display parameters
int     statAttr = 0x4F, ribAttr = 0x17, ribHLAttr = 0x2F, helpAttr = 0x07;
int     helpHLAttr = 0x70;

// Status bar type and display status
int     statType = 0;
int     statOn = TRUE;

// Default quick start to true
char    quick = TRUE;

char    buzzFlag = TRUE;
char    nossFlag = FALSE;

char    redispScrn = FALSE;

// Pointers to marked block lines and offsets
int     startBlockLine = -1, endBlockLine = -1;
int     startBlockOff  =  0, endBlockOff  =  0;
int     startBlockLen  =  0, endBlockLen  =  0;
int     blockType = 0;

// Variables used in searches and replacements
int     lastSearchType = 0;
char    lastFindText[256+1] = "";
char    lastReplaceText[256+1] = "";

// Macro set number and pointers
int     macroSet = 0;
char *  macros[15][10];

// Graphics character set number and data
int     charSet = 0;
char *  graphChars[] =
        {
            "\xDA\xBF\xC0\xD9\xC4\xB3\xC3\xB4\xC1\xC2",
            "\xC9\xBB\xC8\xBC\xCD\xBA\xCC\xB9\xCA\xCB",
            "\xD5\xB8\xD4\xBE\xCD\xB3\xC6\xB5\xCF\xD1",
            "\xD6\xB7\xD3\xBD\xC4\xBA\xC7\xB6\xD0\xD2",
            "\xC5\xCE\xD8\xD7\xE8\xE9\x9B\x9C\x99\xEF",
            "\xB0\xB1\xB2\xDB\xDF\xDC\xDD\xDE\xFE\xFA",
            "\x01\x02\x03\x04\x05\x06\xF0\x7F\x0E\x0F",
            "\x18\x19\x1E\x1F\x10\x11\x12\x1D\x14\x15",
            "\xAE\xAF\xF2\xF3\xA9\xAA\xFD\xF6\xAB\xAC",
            "\xE3\xF1\xF4\xF5\xEA\xD9\xE4\xF8\xFB\xFC",
            "\xE0\xE1\xE2\xE5\xE6\xE7\xEB\xEC\xED\xEE",
            "\x80\x87\xA5\xA4\x98\x9F\xF7\xF9\xAD\xA8",
            "\x83\x84\x85\xA0\xA6\x86\x8E\x8F\x91\x92",
            "\x88\x89\x8A\x82\x90\x8C\x8B\x8D\xA1\x9E",
            "\x93\x94\x95\xA2\xA7\x96\x81\x97\xA3\x9A"
        };

int     MAX_ROWS = 25;
int     MAX_COLS = 80;

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Functions

void  buzz(void)
{
    if (!buzzFlag) return;
    doSound(128,500);
}

    /*--------------------------------------------------------------------*/

void  stripSpaces(void)
{
    // If we *are* stripping spaces and length is greater than 0
    if (!nossFlag && (curLine[0] != NUL))
    {
        // Initialize length
        int len = strlen(curLine);

        // Get end of string pointer
        char * t = curLine + len;

        // Move back over trailing spaces
        while ((len > 0) && (*(--t) == ' ')) --len;

        //
        if (*(++t) != NUL)
            *t = NUL,
            redispScrn = TRUE;

        // 0 initialize from the end of the string to the end of the buffer
     // memset(curLine+len,0,MAX_LINE_SIZE+1-len);
    }
}

    /*--------------------------------------------------------------------*/

int  lineLen(char * s)
{
    if (noatFlag) return strlen(s);

    // Initialize variables
    int code, llen = 0;

    // While s is pointing at a string with length > 0
    while (*s != NUL)
    {
        code = doIsATCode(s);
        if (code)                       // If s is an at code adjust for it
        {
            if (code == ATPOS)
                llen = ((llen > (atWidth-1)) ? llen : (atWidth-1));
            else
                llen += atWidth;
            s += atSize;
        }
        else if (*s == TAB)             // Is it a TAB character?
        {
            atWidth = (((llen)/8)+1)*8+1;
            llen = ((llen > (atWidth-1)) ? llen : (atWidth-1));
            ++s;
        }
        else                            // Otherwise adjust for a single char
        {
            ++llen;
            ++s;
        }
    }

    return llen;
}

    /*--------------------------------------------------------------------*/

void  adjColOff(int newOff)
{
    scOff = newOff-lcOff;

    if (scOff < 0)
        lcOff += scOff,
        scOff = 0;
    else if (scOff > (MAX_COLS-1))
        lcOff += scOff-(MAX_COLS-1),
        scOff = MAX_COLS-1;
}

    /*--------------------------------------------------------------------*/

void  udLine(void)
{
    int code;

    // Save old position and re-start at beginning
    char * o = curLineChar;
    curLineChar = curLine;

    // While the current position < old position
    while (curLineChar < o)
    {
        // Look for an at code and adjust accordingly
        switch (*curLineChar)
        {
            case NUL:
                *(curLineChar++) = ' ';
                break;

            default:
                code = doIsATCode(curLineChar);
                curLineChar += (code ? atSize : 1);
                break;
        }
    }

    // Calculate line length from curLine to curLineChar and adjust scOff
    char ch = *curLineChar;
    *curLineChar = NUL;
    adjColOff(lineLen(curLine));
    *curLineChar = ch;
}

    /*--------------------------------------------------------------------*/

void  wrChUDLine(int c)
{
    // Write the character and update the line
    *(curLineChar++) = char(c);
    textChanged = TRUE;
    udLine();
    lineList[tlOff+slOff] = curLine;
    chkAttrs();
    lineList[tlOff+slOff] = NULL;
}

void  procChar(int c)
{
    // If insert mode and space is left
    if (insMode && (strlen(curLine) < MAX_LINE_SIZE))
    {
        // Make room for a character and put it in
        memmove(curLineChar+1,curLineChar,strlen(curLineChar)+1);
        wrChUDLine(c);
    }
    // Else if overwrite mode and we aren't at the end of the buffer
    else if (!insMode && (curLineChar-curLine < MAX_LINE_SIZE))
    {
        // If we are currently on a code remove the code minus one byte
        if (doIsATCode(curLineChar))
        {
            char * t = curLineChar+atSize-1;
            memmove(curLineChar,t,strlen(t)+1);
         // int clLen = strlen(curLine);
         // memset(curLine+clLen,0,MAX_LINE_SIZE-clLen+1);
        }

        // Put the character in
        wrChUDLine(c);
    }
}

    /*--------------------------------------------------------------------*/

void  delChar(void)
{
    // If curLineChar points to a code
    int code = doIsATCode(curLineChar);
    if (code)
    {
        // Remove the code
        strcpy(curLineChar,curLineChar+atSize);
     // int len = strlen(curLine);
     // memset(curLine+len,0,MAX_LINE_SIZE-len+1);
    }
    else
    {
        // Remove the current character
        strcpy(curLineChar,curLineChar+1);
    }
    textChanged = TRUE;
    udLine();
    lineList[tlOff+slOff] = curLine;
    chkAttrs();
    lineList[tlOff+slOff] = NULL;
}

    /*--------------------------------------------------------------------*/

void editErr(char * fmt, ...)
{
    int tmpCursType = cursType;
    setcurstype(NOCURS);

    char    buf [ 80 + 1 ];
    va_list args;

    va_start(args,fmt);
    vsprintf(buf,fmt,args);
    va_end(args);

    buf[60] = NUL;
    strcat(buf," : ENTER TO CONTINUE");
 // while (strlen(buf) < 80) strcat(buf," ");

    setXY(1,(statOn ? MAX_ROWS-1 : MAX_ROWS));
    setAttr(statAttr);
    wrStr(buf);
    cle();

    flushMacro();
    while (getKey() != ENTER) ;

    setcurstype(tmpCursType);
}

    /*--------------------------------------------------------------------*/

void  commitLine(void)
{
    // Remove trailing spaces
    stripSpaces();

    // If curLine has something in it
    if (curLine[0] != NUL)
    {
        // Save curLine to a buffer
        lineList[tlOff+slOff] = new char[strlen(curLine)+1];
        if (lineList[tlOff+slOff] == NULL)
        {
            editErr("Insufficient memory to save line being edited");
            return;
        }
        strcpy(lineList[tlOff+slOff],curLine);
    }
}

    /*--------------------------------------------------------------------*/

void  cacheLine(void)
{
    // Initialize curLine
    memset(curLine,0,MAX_LINE_SIZE+1);

    // If the current line is used
    if (lineList[tlOff+slOff] != NULL)
    {
        // Load it into curLine and delete buffer
        strcpy(curLine,lineList[tlOff+slOff]);
        delete lineList[tlOff+slOff];
        lineList[tlOff+slOff] = NULL;
    }

    char * p = curLine+strlen(curLine);
    while (p < curLineChar) *(p++) = ' ';
}

    /*--------------------------------------------------------------------*/

void  splitLine(void)
{
    if (lineList[MAX_LINES-1] != NULL)
    {
        editErr("Can't split line, all lines in use");
        return;
    }

    // Make sure sufficient memory exists
    char * p = new char[strlen(curLineChar)+1];
    if (p == NULL)
    {
        editErr("Insufficient memory to split line");
        return;
    }

    // Force a screen refresh
    llcOff = -1;

    // Set text changed flag
    textChanged = TRUE;

    // Open a hole for the new line after the current line
    memmove(lineList+tlOff+slOff+1,lineList+tlOff+slOff,
        (MAX_LINES-(tlOff+slOff)-1)*sizeof(char *));
    memmove(attrList+tlOff+slOff+1,attrList+tlOff+slOff,
        (MAX_LINES-(tlOff+slOff)-1)*sizeof(int));
    memmove(last00List+tlOff+slOff+1,last00List+tlOff+slOff,
        (MAX_LINES-(tlOff+slOff)-1)*sizeof(int));
    memmove(isFFList+tlOff+slOff+1,isFFList+tlOff+slOff,
        (MAX_LINES-(tlOff+slOff)-1)*sizeof(char));
    memmove(lastCLSList+tlOff+slOff+1,lastCLSList+tlOff+slOff,
        (MAX_LINES-(tlOff+slOff)-1)*sizeof(int));

    // Create the new line
    lineList[tlOff+slOff+1] = p;
    strcpy(p,curLineChar);
    *curLineChar = NUL;
 // memset(curLineChar,0,strlen(curLineChar));
    commitLine();
    chkAttrs();
    cacheLine();

    // Reposition cursor
    if (curLineChar != curLine)
    {
        curLineChar = curLine;
        lcOff = scOff = 0;
    }
    moveDown();
//    commitLine();
//    chkAttrs();
//    cacheLine();
}

    /*--------------------------------------------------------------------*/

// *** SDR

    /*--------------------------------------------------------------------*/

void  joinLines(void)
{
    if (tlOff+slOff+1 < MAX_LINES)
    {
        // If there is something to append to the current line...
        if (lineList[tlOff+slOff+1] != NULL)
        {
            int clLen = strlen(curLine);
            memset(curLine+clLen,0,MAX_LINE_SIZE+1-clLen);
            char * nl = lineList[tlOff+slOff+1];
            int nlLen = strlen(nl);
            while ((clLen < MAX_LINE_SIZE) && (nlLen > 0))
            {
                if (doIsATCode(nl))
                {
                    strncpy(curLine+clLen,nl,atSize);
                    clLen += atSize;
                    nl += atSize;
                    nlLen -= atSize;
                }
                else
                {
                    *(curLine+(clLen++)) = *(nl++);
                    --nlLen;
                }
            }
            char * p = NULL;
            if (strlen(nl) > 0)
            {
                p = new char[strlen(nl)+1];
                if (p == NULL) return; /* error */
                strcpy(p,nl);
            }
            delete lineList[tlOff+slOff+1];
            lineList[tlOff+slOff+1] = p;
            llcOff = -1;
        }

        // Close the hole from the old line after the current line if necessary
        if (lineList[tlOff+slOff+1] == NULL)
        {
            memmove(lineList+tlOff+slOff+1,lineList+tlOff+slOff+2,
                (MAX_LINES-(tlOff+slOff+2))*sizeof(char *));
            lineList[MAX_LINES-1] = NULL;
            memmove(attrList+tlOff+slOff+1,attrList+tlOff+slOff+2,
                (MAX_LINES-(tlOff+slOff+2))*sizeof(int));
            llcOff = -1;
        }

     // chkAttrs();
    }

    textChanged = TRUE;
}

    /*--------------------------------------------------------------------*/

void  optimizeText(void)
{
    int ttl = tlOff, tsl = slOff;
    textChanged = TRUE;
    llcOff = -1;
    commitLine();
    tlOff = 0;

    int    lastXXLine = -1;
    char * lastXXPtr  = NULL;

    int  attr     = attrList[0];
    char keepAttr = TRUE;
    char found1st = FALSE;

    for (slOff = 0; slOff < MAX_LINES; ++slOff)
    {
        char * p = lineList[slOff];
        int plen = strlen(p);

        while (plen > 0)
        {
            int code = doIsATCode(p);
            if (code == ATXCODE)
            {
                if (!found1st)
                {
                    found1st = TRUE;
                }
                else if ((strncmp(p,"@X00",4) == 0) || (strncmp(p,"@XFF",4) == 0) ||
                    ((slOff == 0) && (p == lineList[slOff])) ||
                    ((plen == 4) && (strchr("\x20\xFF",*(p-1)) != NULL) &&
                    (p > lineList[slOff])))
                {
                    lastXXLine = -1;
                    keepAttr = TRUE;
                    attr = getATX(p,attr,slOff);
                    p += 4;
                    plen -= 4;
                }
                else // it's a color code
                {
                    if ((lastXXLine != -1) && (lastXXPtr+4 == p))
                    {
                        // remove first
                        keepAttr = FALSE;
                        strcpy(lastXXPtr,lastXXPtr+4);
                        if (lastXXLine == slOff) plen -= 4, p -= 4;
                        lastXXLine = slOff;
                        lastXXPtr = p;
                        attr = getATX(p,attr,slOff);
                        p += 4;
                    }
                    else if ((lastXXLine != -1) && ((attr == getATX(p,attr,slOff))))
                    {
                        // remove second
                        strcpy(p,p+4);
                        plen -= 4;
                    }
                    else if (!keepAttr && (lastXXLine != -1) &&
                        ((attr & 0x70) == (getATX(p,attr,slOff) & 0x70)))
                    {
                        // remove first
                        keepAttr = FALSE;
                        strcpy(lastXXPtr,lastXXPtr+4);
                        if (lastXXLine == slOff) plen -= 4, p -= 4;
                        lastXXLine = slOff;
                        lastXXPtr = p;
                        attr = getATX(p,attr,slOff);
                        p += 4;
                    }
                    else
                    {
                        if ((attr & 0x70) == (getATX(p,attr,slOff) & 0x70))
                            keepAttr = FALSE;
                        else
                            keepAttr = TRUE;
                        plen -= 4;
                        lastXXLine = slOff;
                        lastXXPtr = p;
                        attr = getATX(p,attr,slOff);
                        p += 4;
                    }
                }
            }
            else if (code)
            {
                keepAttr = TRUE;
                int vlen = atSize;
                p += vlen;
                plen -= vlen;
            }
            else
            {
                if (strchr("\x20\xFF",*p) == NULL) keepAttr = TRUE;
                ++p;
                --plen;
            }
        }

        chkAttrs();
    }

    tlOff = ttl;
    slOff = tsl;
    cacheLine();
    adjustCLC();
}

    /*--------------------------------------------------------------------*/

int  copyAttr(int abort = FALSE)
{
    static int attr = -1;
    if (abort)
    {
        attr = -1;
        return FALSE;
    }
    if (attr == -1)
    {
        attr = attrList[tlOff+slOff];
        char *p = curLine;
        int plen = strlen(p);
        while ((p < curLineChar) && (plen > 0))
        {
            int code = doIsATCode(p);
            if (code == ATXCODE)
            {
                attr = getATX(p,attr,tlOff+slOff);
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
        return TRUE;
    }
    else
    {
        if (strlen(curLine)+4 <= MAX_LINE_SIZE)
        {
            memmove(curLineChar+4,curLineChar,strlen(curLineChar)+1);
            *(curLineChar++) = '@';
            *(curLineChar++) = 'X';
            *(curLineChar++) = hexDigits[attr >> 4];
            *(curLineChar++) = hexDigits[attr & 0x0F];
            textChanged = TRUE;
            udLine();
            llcOff = -1;
        }
        attr = -1;
        return FALSE;
    }
}

    /*--------------------------------------------------------------------*/

void  stripAttrs(int clOff, int off, int plen)
{
    commitLine();
    chkAttrs();
    slOff = clOff;
    cacheLine();
    char *p = curLine+off;
    while (plen > 0)
    {
        int code = doIsATCode(p);
        if (code == ATXCODE)
        {
            strcpy(p,p+4);
            plen -= 4;
        }
        else if (code)
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

    /*--------------------------------------------------------------------*/

int  validFileName(char *f)
{
    int i;
    for (i = 0; i < strlen(f); ++i)
        if (!isALNUM(f[i]) && (strchr("\\.:!@#$^&()~`-_{}'",f[i]) == NULL))
            break;

    return (i == strlen(f));
}

    /*--------------------------------------------------------------------*/

int  findText(void)
{
    int ttl = tlOff, tsl = slOff;
    int tlc = lcOff, tsc = scOff;
    char * tclc = curLineChar;

    tlOff += slOff;
    slOff = 0;

    int lftLen = strlen(lastFindText);

    if ((curLineChar == NULL) || (strlen(++curLineChar) < lftLen))
    {
        if (tlOff+slOff+1 < MAX_LINES) commitLine();
        ++tlOff;
        if (tlOff+slOff < MAX_LINES) cacheLine();
        curLineChar = curLine;
    }

    while ((tlOff+slOff < MAX_LINES) &&
        (memicmp(curLineChar,lastFindText,lftLen) != 0))
    {
        if (strlen(++curLineChar) < lftLen)
        {
            if (tlOff+slOff+1 < MAX_LINES) commitLine();
            ++tlOff;
            if (tlOff+slOff < MAX_LINES) cacheLine();
            curLineChar = curLine;
        }
        if (memicmp(curLineChar,lastFindText,lftLen) == 0) udLine();
    }
    if (tlOff+slOff >= MAX_LINES) --tlOff;

    if (memicmp(curLineChar,lastFindText,lftLen) != 0)
    {
        commitLine();
        tlOff = ttl;
        slOff = tsl;
        cacheLine();
        curLineChar = tclc;
        lcOff = tlc;
        scOff = tsc;
        udLine();
        return FALSE;
    }

    tlOff -= MAX_ROWS-2-1;
    slOff = MAX_ROWS-2-1;

    if (tlOff < 0)
        slOff += tlOff,
        tlOff = 0;

    return TRUE;
}

    /*--------------------------------------------------------------------*/

int  replaceText(void)
{
    int ch = 0;

    if (strnicmp(curLineChar,lastFindText,strlen(lastFindText)) == 0)
    {
        dispScrn(TRUE);
        setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
        setAttr(statAttr);
        wrStr("Replace this?");
        cle();
        dispCurs();
        while ((ch != 'Y') && (ch != 'N') && (ch != ESC))
            ch = toUPPER(getKey());
        if (ch == 'Y')
        {
            if (strlen(lastFindText) >= strlen(lastReplaceText))
            {
                memcpy(curLineChar,lastReplaceText,strlen(lastReplaceText));
                char *p1 = curLineChar+strlen(lastFindText);
                char *p2 = curLineChar+strlen(lastReplaceText);
                memmove(p2,p1,strlen(p1)+1);
                curLineChar += strlen(lastReplaceText);
            }
            else if (MAX_LINE_SIZE-strlen(curLine) >=
                strlen(lastReplaceText)-strlen(lastFindText))
            {
                char *p1 = curLineChar+strlen(lastFindText);
                char *p2 = p1+(strlen(lastReplaceText)-strlen(lastFindText));
                memmove(p2,p1,strlen(p1)+1);
                memcpy(curLineChar,lastReplaceText,strlen(lastReplaceText));
                curLineChar += strlen(lastReplaceText);
            }
        }
    }

    return ((ch == 'Y') || (ch == 'N'));
}

    /*--------------------------------------------------------------------*/

int  getText(char * s, char * buf)
{
    setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
    setAttr(statAttr);
    wrStr(s);
    cle();

    char cur[256+1];
    strcpy(cur,buf);

    if (getStr(cur,256,F_ALL))
    {
        strcpy(buf,cur);
        return TRUE;
    }

    return FALSE;
}

    /*--------------------------------------------------------------------*/

void  justifyLine(void)
{
    // Select justification type LCR
    int just = 0;
    setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
    setAttr(statAttr);
    wrStr("Justification type (L/C/R):  L");
    cle();

    while (just == 0)
    {
        just = toUPPER(getKey());
        if (just == 27) return;
        if (just == 13) just = 'L';
        if ((just != 'L') && (just != 'C') && (just != 'R')) just = 0;
    }

    textChanged = TRUE;

    strcpy(tmpLine,curLine);
    char * tclc = curLineChar;
    int ttl = tlOff, tsl = slOff, tlc = lcOff, tsc = scOff;

    curLineChar = curLine;
    lcOff = scOff = 0;

    while ((*curLineChar != NUL) && ((lcOff+scOff) == 0))
    {
        if (*curLineChar != ' ')
            moveRight();
        else
            strcpy(curLineChar,curLineChar+1);
    }

    while (*curLineChar != NUL) moveRight();

    int epos = lcOff+scOff;

    while ((curLineChar > curLine) && (epos == lcOff+scOff))
    {
        moveLeft();
        if (*curLineChar == ' ')
        {
            strcpy(curLineChar,curLineChar+1);
            --epos;
        }
    }

    int ll = lineLen(curLine);
    if ((ll == 0) || (ll >= 80))
    {
        strcpy(curLine,tmpLine);
        curLineChar = tclc;
        tlOff = ttl;
        slOff = tsl;
        lcOff = tlc;
        scOff = tsc;
        buzz();
        return;
    }

    curLineChar = curLine;
    lcOff = scOff = 0;
    while ((lcOff+scOff) == 0) moveRight();
    moveLeft();

    switch (just)
    {
        case 'C':
            memmove(curLineChar+((80-ll)/2),curLineChar,strlen(curLineChar)+1);
            memset(curLineChar,' ',((80-ll)/2));
            break;

        case 'R':
            memmove(curLineChar+(79-ll),curLineChar,strlen(curLineChar)+1);
            memset(curLineChar,' ',(79-ll));
            break;
    }

    curLineChar = curLine;
    lcOff = scOff = 0;
}

    /*--------------------------------------------------------------------*/

void  mainLoop(void)
{
    int ch, exitFlag = FALSE, udScrn = TRUE, udStat = FALSE;

    do {

        if (redispScrn)
            udScrn = TRUE,
            redispScrn = FALSE,
            llcOff = ltlOff = -11;

        if (udScrn || (!statOn && udStat)) dispScrn(udStat);
        if (statOn && udStat) dispStat();
        udScrn = udStat = FALSE;

        ch = getKey();

        if ((ch >= F1) && (ch <= F10) && !disableEditKeys)
            procChar(graphChars[charSet][ch-F1]),
            udScrn = TRUE;
        else if ((ch >= SF1) && (ch <= SF10))
            putMacro(macros[macroSet][ch-SF1]),
            udScrn = TRUE;
        else if ((ch >= AF1) && (ch <= AF10))
        {
            if (getkbdstatus() & 0x03)
                macroSet = ch - AF1;
            else
                charSet = ch - AF1;
            udStat = TRUE;
        }
        else if ((ch >= CF1) && (ch <= CF5))
        {
            if (getkbdstatus() & 0x03)
                macroSet = ch - CF1 + 10;
            else
                charSet = ch - CF1 + 10;
            udStat = TRUE;
        }
        else if (((ch == CF7) || (ch == CX)) && (endBlockLine != -1))
        {
            // Cut block
            writeBlock();
            deleteBlock();
            noBlock();
            udScrn = TRUE;
        }
        else if (((ch == CF8) || (ch == CC))&& (endBlockLine != -1))
        {
            // Copy block
            writeBlock();
            noBlock();
            disableEditKeys = FALSE;
            udScrn = TRUE;
        }
        else if (((ch == CF9) || (ch == CV)) && (startBlockLine == -1))
        {
            // Paste block
            readBlock();
            udScrn = TRUE;
        }
        else if ((ch == CF10) && (endBlockLine != -1))
        {
            // Move block
            writeBlock();
            deleteBlock();
            noBlock();
            readBlock();
            udScrn = TRUE;
        }
        else if (ch == AA)
        {
            attrBlock();
            udScrn = TRUE;
        }
        else if ((ch == AB) && (!disableEditKeys || (startBlockLine != -1)))
        {
            markBlock();
            udScrn = TRUE;
        }
        else if ((ch == AC) && !disableEditKeys)
        {
            clearMem();
            udScrn = TRUE;
        }
        else if (ch == AD)
        {
            doDOS();
            udScrn = TRUE;
        }
        else if (ch == AF)
        {
            selectFKeyChar();
            udScrn = TRUE;
        }
        else if (ch == AG)
        {
            selectMacro();
            udScrn = TRUE;
        }
        else if (ch == AH)
        {
            dispHelp();
            llcOff = -1;
            udScrn = TRUE;
        }
        else if ((ch == AI) && ioFlag)
        {
            static int    stackLevel = 0;
            static char * stackNames[4] = { NULL, NULL, NULL, NULL };
            commitLine();
            cacheLine();
            if ((curLine[0] == '%') && validFileName(curLine+1) && (stackLevel < 4))
            {
                if (getFN("save",FALSE))
                {
                    doSaveFile();
                    stackNames[stackLevel] = new char[strlen(lastFN)+1];
                    if (stackNames[stackLevel] != NULL)
                    {
                        strcpy(stackNames[stackLevel],lastFN);
                        ++stackLevel;
                        strcpy(lastFN,curLine+1);
                        strupr(lastFN);
                        doLoadFile();
                    }
                }
            }
            else if (((curLine[0] != '%') || !validFileName(curLine+1)) && (stackLevel > 0))
            {
                if (getFN("save",FALSE))
                {
                    doSaveFile();
                    --stackLevel;
                    strcpy(lastFN,stackNames[stackLevel]);
                    delete stackNames[stackLevel];
                    stackNames[stackLevel] = NULL;
                    doLoadFile();
                }
            }
            udScrn = TRUE;
        }
        else if (ch == AJ)
        {
            justifyLine();
            udScrn = TRUE;
        }
        else if (ch == AK)
        {
            killFile();
            udScrn = TRUE;
        }
        else if (ch == AL)
        {
            loadFile();
            udScrn = TRUE;
        }
        else if ((ch == AM) && !disableEditKeys)
        {
            selectChar();
            udScrn = TRUE;
        }
        else if ((ch == AO) && !noatFlag && !noatxFlag && !disableEditKeys)
        {
            optimizeText();
            udScrn = TRUE;
        }
        else if (ch == AP)
        {
            doSaveFile();

            char buf[256+1];
            sprintf(buf,"PPLC %s",lastFN);
            doDOS(buf,TRUE);

            udScrn = TRUE;
        }
        else if (ch == AR)
        {
            textmode((MAX_ROWS > 25) ? C80 : C4350);
            text_info ti;
            gettextinfo(&ti);
            MAX_ROWS = ti.screenheight;
            if (slOff >= MAX_ROWS-2)
            {
                tlOff += (slOff-(MAX_ROWS-2))+1;
                slOff = MAX_ROWS-2-1;
            }
            llcOff = -1;
            udScrn = TRUE;
        }
        else if (ch == AS)
        {
            saveFile();
            udStat = TRUE;
        }
        else if (ch == AT)
        {
            if (statOn && (++statType == 2))
                statOn = FALSE;
            else if (!statOn)
            {
                statOn = TRUE;
                statType = 0;
            }

            if (statOn && (slOff >= MAX_ROWS-2))
            {
                commitLine();
                tlOff += slOff-(MAX_ROWS-2)-1;
                slOff = MAX_ROWS-2-1;
                cacheLine();
            }
            llcOff = -1;
            udScrn = TRUE;
        }
        else if ((ch == AU) && (startBlockLine == -1))
        {
            disableEditKeys = copyAttr();
            udScrn = TRUE;
        }
        else if ((ch == AV) && !disableEditKeys)
            selectATVar(),
            udScrn = TRUE;
        else if (ch == AW)
            dispWelcome(),
            udScrn = TRUE;
        else if (ch == AX)
        {
            if (confirmExit()) exitFlag = TRUE;
            udStat = TRUE;
        }
        else if ((ch == AY) && !disableEditKeys)
        {
            noatFlag = char(!noatFlag);
            int ttl = tlOff, tsl = slOff, tlc = lcOff, tsc = scOff;
            char * tclc = curLineChar;
            commitLine();
            tlOff = slOff = lcOff = scOff = 0;
            ltlOff = llcOff = -11;
            curLineChar = curLine;
            chkAttrs(TRUE);
            tlOff = ttl, slOff = tsl, lcOff = tlc, scOff = tsc;
            curLineChar = tclc;
            cacheLine();
            udLine();
            udScrn = TRUE;
        }
        else if ((ch == AZ) && !noatFlag && !disableEditKeys)
        {
            noatxFlag = char(!noatxFlag);
            int ttl = tlOff, tsl = slOff, tlc = lcOff, tsc = scOff;
            char * tclc = curLineChar;
            commitLine();
            tlOff = slOff = lcOff = scOff = 0;
            ltlOff = llcOff = -11;
            curLineChar = curLine;
            chkAttrs(TRUE);
            tlOff = ttl, slOff = tsl, lcOff = tlc, scOff = tsc;
            curLineChar = tclc;
            cacheLine();
            udLine();
            udScrn = TRUE;
        }
        else if ((ch == CF) && !disableEditKeys)
        {
            if (getText("Enter text to find:  ",lastFindText))
            {
                lastSearchType = CF;
                if (!findText()) buzz();
            }
            udScrn = TRUE;
        }
        else if (ch == CL)
        {
            if ((lastSearchType == CF) && (strlen(lastFindText) > 0))
            {
                if (!findText()) buzz();
                udScrn = TRUE;
            }
            else if ((lastSearchType == CR) && (strlen(lastFindText) > 0))
            {
                while (TRUE)
                {
                    if (!findText())
                    {
                        buzz();
                        break;
                    }
                    if (!replaceText()) break;
                }
                udScrn = TRUE;
            }
        }
        else if (ch == CP)
        {
            char buf[256+1];
            strcpy(buf,lastFN);

            doSaveFile();
            doClearMem();

            strcpy(lastFN,buf);
            sprintf(buf,"PPLC /NODISPSTAT %s > ERR.$$$",lastFN);
            doDOS(buf,FALSE);

            strcpy(lastFN,"ERR.$$$");
            doLoadFile();

            udScrn = TRUE;
        }
        else if ((ch == CR) && !disableEditKeys)
        {
            if (getText("Search text:  ",lastFindText))
            {
                if (getText("Replacement text:  ",lastReplaceText))
                {
                    lastSearchType = CR;
                    while (TRUE)
                    {
                        if (!findText())
                        {
                            buzz();
                            break;
                        }
                        if (!replaceText()) break;
                    }
                }
            }
            udScrn = TRUE;
        }
//        else if (ch == CS)
//            nossFlag = !nossFlag;
        else if ((ch == CY) && !disableEditKeys)
        {
            memset(curLine,0,MAX_LINE_SIZE+1);
            joinLines();
            adjustCLC();
            udScrn = TRUE;
        }
        else if ((ch == INS) && !disableEditKeys)
            insMode = !insMode,
            udStat = TRUE;
        else if (ch == DEL)
        {
            if ((endBlockLine != -1) && confirm("Do you wish to delete the "
                "marked block?  "))
            {
                deleteBlock();
                udScrn = TRUE;
            }
            else if (endBlockLine != -1)
                udStat = TRUE;
            else if ((strlen(curLineChar) > 0) && (startBlockLine == -1) && !disableEditKeys)
            {
                delChar();
                udScrn = TRUE;
            }
            else if ((tlOff+slOff < MAX_LINES-1) && (startBlockLine == -1) && !disableEditKeys)
            {
                joinLines();
                udScrn = TRUE;
            }
            else
                buzz();
        }
        else if (ch == HOME)
        {
            int tlc = lcOff, tcll = strlen(curLine);

            stripSpaces();

            if (curLineChar != curLine)
                curLineChar = curLine,
                lcOff = 0,
                scOff = 0;

            if ((tlc != lcOff) || (tcll != strlen(curLine)))
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == END)
        {
            char *tclc = curLineChar;
            int tcll = strlen(curLine);
            moveEnd();
            if ((tclc != curLineChar) || (tcll != strlen(curLine)))
                udScrn = TRUE;
        }
        else if (ch == LEFT)
        {
            int tlc = lcOff;
            moveLeft();
            if (tlc != lcOff)
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == RIGHT)
        {
            int tlc = lcOff, tcll = strlen(curLine);
            moveRight();
            if ((tlc != lcOff) || (tcll != strlen(curLine)))
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == CLEFT)
        {
            int lastCHAR = FALSE;

            while ((curLineChar != curLine) || (tlOff+slOff > 0))
            {
                if (curLineChar == curLine)
                {
                    moveUp();
                    moveEnd();
                }
                else
                    moveLeft();

                if (!isALNUM(*curLineChar) && lastCHAR)
                {
                    moveRight();
                    break;
                }

                lastCHAR = (lastCHAR || isALNUM(*curLineChar));
            }
            udScrn = TRUE;
        }
        else if (ch == CRIGHT)
        {
            int lastSPAT = FALSE;

            while ((*curLineChar != NUL) || ((tlOff+slOff) < MAX_LINES))
            {
                if (*curLineChar == NULL)
                {
                    moveDown();
                    moveEnd();
                    curLineChar = curLine;
                    lcOff = scOff = 0;
                }
                else
                    moveRight();

                if ((isALNUM(*curLineChar) &&
                    ((curLineChar == curLine) || lastSPAT)) ||
                    (strlen(curLineChar) == 0))
                    break;

                lastSPAT = (!isALNUM(*curLineChar) || doIsATCode(curLineChar));
            }
            udScrn = TRUE;
        }
        else if (ch == UP)
        {
            int ttl = tlOff;
            moveUp();
            if (ttl != tlOff)
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == DOWN)
        {
            moveDown();
            udScrn = TRUE;
        }
        else if (ch == PGUP)
        {
            int ttl = tlOff;
            commitLine();
            if (tlOff >= MAX_ROWS-(statOn?2:0))
                tlOff -= MAX_ROWS-(statOn?2:0);
            else if (tlOff+slOff >= MAX_ROWS-(statOn?2:0))
                slOff -= MAX_ROWS-(statOn?2:0)-tlOff,
                tlOff = 0;
            else
                tlOff = slOff = 0;
            cacheLine();
            adjustCLC();
            if (ttl != tlOff)
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == PGDN)
        {
            int ttl = tlOff;
            commitLine();
            if (tlOff+slOff+MAX_ROWS-(statOn?2:0) < MAX_LINES)
                tlOff += MAX_ROWS-(statOn?2:0);
            else if (tlOff+MAX_ROWS-(statOn?2:0) < MAX_LINES)
                tlOff = MAX_LINES-slOff-1;
            else
                slOff = MAX_LINES-tlOff-1;
            cacheLine();
            adjustCLC();
            if (ttl != tlOff)
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == CPGUP)
        {
            int ttl = tlOff;
            commitLine();
            tlOff = slOff = 0;
            cacheLine();
            if (ttl != tlOff)
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == CPGDN)
        {
            int ttl = tlOff;
            commitLine();
            slOff = MAX_ROWS-(statOn?2:0)-1;
            tlOff = MAX_LINES-slOff-1;
            cacheLine();
            if (ttl != tlOff)
                udScrn = TRUE;
            else
                udStat = TRUE;
        }
        else if (ch == CHOME)
        {
            commitLine();
            slOff = 0;
            cacheLine();
            udStat = TRUE;
        }
        else if (ch == CEND)
        {
            commitLine();
            slOff = MAX_ROWS-(statOn?2:0)-1;
            cacheLine();
            udStat = TRUE;
        }
        else if ((ch == BS) && !disableEditKeys)
        {
            if (curLineChar != curLine)
                moveLeft(),
                delChar(),
                udScrn = TRUE;
            else if (tlOff+slOff > 0)
                moveUp(),
                moveEnd(),
                joinLines(),
                udScrn = TRUE;
        }
        else if (ch == ENTER)
        {
            if (disableEditKeys)
            {
                disableEditKeys = copyAttr();
                udScrn = TRUE;
            }
            else if (insMode && (lineList[MAX_LINES-1] == NULL) &&
                !disableEditKeys)
            {
                ltlOff = llcOff = -11;
                splitLine();
                udLine();
                udScrn = TRUE;
            }
            else if (!insMode)
            {
                if (curLineChar != curLine)
                    curLineChar = curLine,
                    lcOff = 0,
                    scOff = 0,
                    udScrn = TRUE;
                int ttl = tlOff;
                moveDown();
                if (ttl != tlOff)
                    udScrn = TRUE;
                else
                    udStat = TRUE;
            }
            else
                buzz();
        }
        else if (ch == LF)
        {
            char * tclc = curLineChar;
            int    tlc = lcOff;
            int    tsc = scOff;

            splitLine();
            moveUp();

            curLineChar = tclc;
            lcOff = tlc;
            scOff = tsc;

            udLine();
            udScrn = TRUE;
        }
        else if (ch == TAB)
        {
            if (insMode && !disableEditKeys)
            {
                char *lastPtr;
                int tlc = lcOff, tsc = scOff;

                do {

                    lastPtr = curLineChar;
                    procChar(' ');

                } while ((((lcOff+scOff) & 7) != 0) &&
                    (((lcOff+scOff) == (tlc+tsc)) ||
                    (lastPtr != curLineChar)));
            }
            else
            {
                char *lastPtr;
                int tlc = lcOff, tsc = scOff;

                do {

                    lastPtr = curLineChar;
                    moveRight();

                } while (((((lcOff+scOff) & 7) != 0) ||
                    ((lcOff+scOff) == (tlc+tsc))) &&
                    (lastPtr != curLineChar));
            }

            udScrn = TRUE;
        }
        else if ((ch == STAB) && ((lcOff+scOff) > 0))
        {
            char *lastPtr;
            int tlc = lcOff, tsc = scOff;

            do {

                lastPtr = curLineChar;
                moveLeft();

            } while (((((lcOff+scOff) & 7) != 0) ||
                ((lcOff+scOff) == (tlc+tsc))) &&
                (lastPtr != curLineChar));

            udScrn = TRUE;
        }
        else if (ch == ESC)
        {
            copyAttr(TRUE);
            if (startBlockLine != -1)
            {
                noBlock();
                disableEditKeys = FALSE;
                llcOff = -1;
                udScrn = TRUE;
            }
            else if (disableEditKeys)
            {
                disableEditKeys = FALSE;
                udStat = TRUE;
            }
            else
            {
                procChar(ch);
                udScrn = TRUE;
            }
        }
        else if (strchr("\x07\x0A\x0C",ch) != NULL)
        {
            procChar(ch);
            udScrn = TRUE;
        }
        else if ((ch >= 1) && (ch <= 255) && !disableEditKeys)
        {
            procChar(ch);
            udScrn = TRUE;
        }
        else
            buzz();

    } while (!exitFlag);
}

    /*--------------------------------------------------------------------*/

#include <conio.h>

int main(void)
{
    if (((_osmajor*100)+_osminor) < 310)
    {
        clrscr();
        cprintf("\x07""PCBEdit requires DOS 3.1 or later . . .\r\n");
        return 1;
    }

    if (getenv("INPCBED") != NULL)
    {
        clrscr();
        cprintf("\x07""A copy of PCBEdit is already running . . .\r\n");
        return 1;
    }

#ifdef DEBUG
    mc_startcheck(NULL);
#endif

    initPrg();
    mainLoop();
    restPrg();

#ifdef DEBUG
    mc_endcheck();
#endif

    clrscr();
    cprintf("PCBEdit is a copyrighted program produced for "
            "PCBoard customers use only.\r\n");
    cprintf("If you are interested in purchasing a copy of "
            "PCBoard, please call us at:\r\n\n");
    cprintf("    Clark Development Company, Inc.\r\n");
    cprintf("    P.O. Box 571365\r\n");
    cprintf("    Murray, Utah, 84157-1365\r\n");
    cprintf("    (801) 261-1686, 8:00am-5:00pm (Mountain Time), M-F\r\n");
    cprintf("    (800) 356-1686, 8:00am-5:00pm (Mountain Time), M-F\r\n");

    return 0;
}

/******************************************************************************/

