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
/*                                  DISP.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                    Functions used to update the display                    */
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

#pragma hdrfile "disp.sym"

#include    <alloc.h>
#include    <string.h>

#include    <disp.hpp>

#include    <atcodes.hpp>
#include    <fast_cio.hpp>
#include    <keyboard.hpp>
#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Defined Macros

#define doAdjustAttr(a,l,c,x) ((blockType == 0) ? a : adjustAttr(a,l,c,x))

/******************************************************************************/

extern int startAttr;

/******************************************************************************/

// Inline Functions

inline int min ( int l, int r )
{
    return ((l < r) ? l : r);
}

/******************************************************************************/

// Functions

    /**********************************************************************/
    /*                                                                    */
    /* adjustAttr Function Description                                    */
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                                    */
    /* Takes an attribute, line number, column offset, and a set of mask  */
    /* values.  If the line and column fall within a marked region, the   */
    /* mask values are applied to the attribute and the modified          */
    /* attribute is returned.  Otherwise, the attribute is returned       */
    /* unmodified.                                                        */
    /*                                                                    */
    /*   Parameter Descriptions                                           */
    /*   ~~~~~~~~~~~~~~~~~~~~~~                                           */
    /*   int returns = The attribute to use                               */
    /*   int attr    = Attribute to modify and/or return                  */
    /*   int num     = The current line number                            */
    /*   int off     = The current column offset                          */
    /*   int xormask = The mask of bits to toggle                         */
    /*                                                                    */
    /**********************************************************************/

int  adjustAttr(int attr, int num, int off, int xormask)
{
    if (blockType != 0)
    {
        int adjAttr = (attr ^ xormask);

        switch (blockType)
        {
            case 'C':
                if ((num > startBlockLine) &&
                    ((num < endBlockLine) || (endBlockLine == -1)))
                    return adjAttr;
                else if ((num == startBlockLine) && (startBlockLine == endBlockLine))
                {
                    if ((off >= startBlockOff) && (off <= endBlockOff))
                        return adjAttr;
                }
                else if ((num == startBlockLine) && (off >= startBlockOff))
                    return adjAttr;
                else if ((num == endBlockLine) && (off <= endBlockOff))
                    return adjAttr;
                break;

            case 'L':
                if ((num >= startBlockLine) &&
                    ((num <= endBlockLine) || (endBlockLine == -1)))
                    return adjAttr;
                break;

            case 'B':
                if ( (num >= startBlockLine) &&
                    ((num <= endBlockLine) || (endBlockLine == -1)) &&
                     (off >= startBlockLen) &&
                    ((off <= endBlockLen) || (endBlockLine == -1))
                   )
                    return adjAttr;
                break;
        }
    }

    return attr;
}

    /*------------------------------------------------------------------*/

void  dispLine(char * line, int off, int attr, int num)
{
    // Initialize the last attribute used with an @CLREOL@
    int tmpnum = num;
    int lastCLREOL;
    do {
        lastCLREOL = lastCLSList[tmpnum--];
    } while ((lastCLREOL == -1) && (tmpnum >= 0));
    if (lastCLREOL == -1) lastCLREOL = startAttr;

    // Skip past all until off
    int i = 0, code;
    char * p = ((line == NULL) ? "" : line);
    int aaOff = 0;

    while ((i < off) && (*p != NUL))
    {
        code = doIsATCode(p);
        if (code)
        {
            if (code == ATXCODE)
            {
                attr = getATX(p,attr,num);
                p += 4;
                if (blockType != 'B') aaOff += 4;
            }
            else if (code == ATPOS)
            {
                if (i < --atWidth)
                    atWidth -= i;
                else
                    atWidth = 0;
                memset(atBuf,' ',atWidth);
                atBuf[atWidth] = NUL;
            }
            if (code != ATXCODE)
            {
                i += atWidth;
                p += atSize;
                if (blockType != 'B') aaOff += atSize;
            }
            if (code == ATCLREOL) lastCLREOL = attr;
        }
        else if (*p == TAB)
        {
            atWidth = (((i)/8)+1)*8+1;
            if (i < --atWidth)
                atWidth -= i;
            else
                atWidth = 0;
            memset(atBuf,' ',atWidth);
            atBuf[atWidth] = NUL;
            i += atWidth;
            ++p;
            if (blockType != 'B') ++aaOff;
        }
        else
        {
            ++i;
            ++p;
            ++aaOff;
        }
    }

    // Create and initialize buf to empty string
    char buf[(80*2)+1];
 // memset(buf,0,(80*2)+1);

    int    blen = 0;
    char * bptr = buf;

    // If we went past off adjust back to off
    if (i > off)
    {
        strcpy(atBuf,atBuf+(atWidth-(i-off)));
        atBuf[80] = NUL;

        char * tmp = atBuf;

        while (*tmp != NUL)
        {
            *(bptr++) = *(tmp++);
            *(bptr++) = char(doAdjustAttr(attr,num,aaOff,0x77));
            ++blen;
            if (blockType == 'B') ++aaOff;
        }

        i = off;
    }
    else if ((blockType == 'B') && (aaOff < off))
    {
        aaOff = off;
    }

    // Find and print no more than 80 columns
    while ((blen < MAX_COLS) && (*p != NUL))
    {
        switch (*p)
        {
            case TAB:
                atWidth = (((i+blen)/8)+1)*8+1-(i+blen+1);
                if (atWidth > 0)
                {
                    int colsLeft = MAX_COLS-blen;
                 // int ch2prn = ((colsLeft > atWidth) ? atWidth : colsLeft);
                    int ch2prn = min(colsLeft,atWidth);
                    for (int i = 0; i < ch2prn; ++i)
                    {
                        *(bptr++) = ' ';
                        *(bptr++) = char(doAdjustAttr(attr,num,aaOff,0x77));
                        ++blen;
                        if (blockType == 'B') ++aaOff;
                    }
                }
                ++p;
                if (blockType != 'B') ++aaOff;
                break;

            case '@':
                if ((code = doIsATCode(p)) != 0)
                {
                    switch (code)
                    {
                        case ATXCODE:
                            attr = getATX(p,attr,num);
                            p += 4;
                            if (blockType != 'B') aaOff += 4;
                            break;

                        case ATPOS:
                            if (i+blen < --atWidth)
                                atWidth -= i+blen;
                            else
                                atWidth = 0;
                            break;
                    }
                    if (code != ATXCODE)
                    {
                        if (atWidth > 0)
                        {
                            int colsLeft = MAX_COLS-blen;
                            int ch2prn = ((colsLeft > atWidth) ?
                                atWidth : colsLeft);
                            for (int i = 0; i < ch2prn; ++i)
                            {
                                *(bptr++) = ((code != ATPOS) ? atBuf[i] : ' ');
                                *(bptr++) = char(doAdjustAttr(attr,num,aaOff,0x77));
                                ++blen;
                                if (blockType == 'B') ++aaOff;
                            }
                        }
                        p += atSize;
                        if (blockType != 'B') aaOff += atSize;
                        if (code == ATCLREOL) lastCLREOL = attr;
                    }
                    break;
                }
                // else fall through to default

            default:
                *(bptr++) = *(p++);
                *(bptr++) = char(doAdjustAttr(attr,num,aaOff++,0x77));
                ++blen;
                break;
        }
    }

    // If less than 80 columns printed, echo color to EOL
    while (blen < MAX_COLS)
    {
        *(bptr++) = ' ';
        *(bptr++) = char(doAdjustAttr(lastCLREOL,num,aaOff++,0x77));
        ++blen;
    }

    putline(cury(),buf);
}

    /*------------------------------------------------------------------*/

void  dispCurs(void)
{
    // Position in the correct location and turn it on based on insMode
    setXY(scOff+1,slOff+1);
    setcurstype(insMode ? SOLIDCURS : NORMALCURS);
}

    /*------------------------------------------------------------------*/

#ifdef __OS2__
#define maxPathNameSize 22
#define ROW_FMT_WIDTH   "5"
#else
#define maxPathNameSize 17
#define ROW_FMT_WIDTH   "4"
#endif

void  dispStat(void)
{
    if (statOn)
    {
        // Turn off cursor
        setcurstype(NOCURS);

        // Write status line
        setAttr(statAttr);
        setXY(1,MAX_ROWS-1);
        switch (statType)
        {
            case 1:
                fastprintf("Row %-" ROW_FMT_WIDTH "d Col %-4d  %3s  Set %-2d 1=%c 2=%c 3=%c 4=%c 5=%c"
                    " 6=%c 7=%c 8=%c 9=%c 10=%c"
                   #ifndef __OS2__
                    "    %3luK"
                   #endif
                    ,
                    tlOff+slOff+1, lcOff+scOff+1, insMode ? "INS" : "OVR",
                    charSet+1, graphChars[charSet][0], graphChars[charSet][1],
                    graphChars[charSet][2], graphChars[charSet][3],
                    graphChars[charSet][4], graphChars[charSet][5],
                    graphChars[charSet][6], graphChars[charSet][7],
                    graphChars[charSet][8], graphChars[charSet][9]
                   #ifndef __OS2__
                    ,coreleft()/1024
                   #endif
                    );
                break;

            default:
            {
                char *modeStr = "";
                if (disableEditKeys && (startBlockLine != -1))
                    modeStr = "BLK";
                else if (disableEditKeys)
                    modeStr = "ATR";
                else if (insMode)
                    modeStr = "INS";
                else
                    modeStr = "OVR";
                statType = 0;
                fastprintf("Row %-" ROW_FMT_WIDTH "d Col %-4d  %3s  Set %2d:%s  Alt-H=HELP"
                    "  %c%s%-*.*s"
                   #ifndef __OS2__
                    "  %3luK"
                   #endif
                    ,
                    tlOff+slOff+1, lcOff+scOff+1, modeStr,
                    charSet+1, graphChars[charSet],
                    (textChanged ? '*' : ' '),
                    (strlen(lastFN) > maxPathNameSize) ? "..." : "",
                    (strlen(lastFN) > maxPathNameSize) ? (maxPathNameSize-3) : maxPathNameSize,
                    (strlen(lastFN) > maxPathNameSize) ? (maxPathNameSize-3) : maxPathNameSize,
                    (strlen(lastFN) > maxPathNameSize) ? lastFN+strlen(lastFN)-(maxPathNameSize-3) : lastFN
                   #ifndef __OS2__
                    ,coreleft()/1024
                   #endif
                    );
                cle();
                break;
            }
        }
     // cle();

        // Write code ribbon
        char buf[80];
        memset(buf,' ',80);
        buf[79] = NUL;

        int i = (((i = (int)(curLineChar-curLine)) > 39) ? 39 : i);
        char *tptr = buf + 40 - i;
        memcpy(tptr, curLineChar-i, i + ((strlen(curLineChar) <= 39) ? strlen(curLineChar) : 39));

        i = doIsATCode(buf+40);
        i = (i ? atSize : 1);

        setXY(1,MAX_ROWS);

        setAttr(ribAttr);
        fastprintf("%-40.40s",buf);

        char tmp = *curLineChar;
        *curLineChar = NUL;
        int len2here = lineLen(curLine);
        *curLineChar = tmp;

        setAttr(doAdjustAttr(ribHLAttr,tlOff+slOff,
            (blockType != 'B') ? (int)(curLineChar-curLine) : len2here,
            0x80));
        fastprintf("%-*.*s",i,i,buf+40);

        setAttr(ribAttr);
        fastprintf("%s",buf+40+i);

        cle();
    }

    // Redisplay cursor
    dispCurs();
}

    /*------------------------------------------------------------------*/

void  chkAttrs(int forceAll)
{
    // Initialize current line offset and declare continue flag
    int clOff = tlOff+slOff, contFlag;

    // While we continue finding color changes
    do {

        // Assume we won't continue past this line
        contFlag = FALSE;

        // Assign ptr, size, and attr to the current line
        last00List[clOff] = -1;
        isFFList[clOff] = FALSE;
        lastCLSList[clOff] = -1;

        int attr = attrList[clOff];

        char * ptr = lineList[clOff];
        if (ptr)
        {
            // While there is something left of ptr to process
            while ((ptr = strchr(ptr,'@')) != NULL)
            {
                // Get @Code ID for current ptr
                int code = doIsATCode(ptr);

                // Adjust ptr and size
                if (code)
                {
                    // If it's an @X code get the new attr
                    if (code == ATXCODE)
                    {
                        attr = getATX(ptr,attr,clOff);
                        int tmp = (hex2dec(*(ptr+2)) << 4) + hex2dec(*(ptr+3));
                        if (tmp == 0x00)
                            last00List[clOff] = attr;
                        else if (tmp == 0xFF)
                            isFFList[clOff] = TRUE;
                        atSize = 4;
                    }
                    else if (code == ATCLS)
                    {
                        lastCLSList[clOff] = attr;
                    }

                    ptr += atSize;
                }
                else
                {
                    ++ptr;
                }
            }
        }

        // Echo the attr change if necessary
        if ((++clOff < MAX_LINES) &&
            (forceAll || (attrList[clOff] != attr)))
        {
            attrList[clOff] = attr;
            llcOff = -1;
            contFlag = TRUE;
        }

    } while (contFlag);

    // If there is an @X00 on this line, check for future @XFF's
    if (!forceAll && (last00List[tlOff+slOff] != -1))
    {
        int ttl, tsl;
        ttl = tlOff; tsl = slOff;

        for (tlOff = 0, slOff = ttl+tsl+1; slOff < MAX_LINES; ++slOff)
        {
            if (isFFList[slOff]) chkAttrs();
            if (last00List[slOff] != -1) break;
        }

        tlOff = ttl; slOff = tsl;
    }
}

    /*------------------------------------------------------------------*/

void  dispScrn(int udStat)
{
    // Turn off cursor
    setcurstype(NOCURS);

    // Check for @X00 and @XFF codes
//    lineList[tlOff+slOff] = curLine;
//    chkAttrs();
//    lineList[tlOff+slOff] = NULL;

    if (!statOn && udStat)
    {
        lineList[tlOff+slOff] = curLine;
        setXY(1,MAX_ROWS);
        dispLine(lineList[ltlOff+MAX_ROWS-1],lcOff,
            attrList[ltlOff+MAX_ROWS-1],ltlOff+MAX_ROWS-1);
        lineList[tlOff+slOff] = NULL;
    }

    // If the screen has scrolled in any direction...
//    if (keyHit())
//    {
//        if ((tlOff != ltlOff) || (lcOff != llcOff))
//            ltlOff = -11,
//            llcOff = -11;
//    }
//    else if ((tlOff != ltlOff) || (lcOff != llcOff))
    if ((tlOff != ltlOff) || (lcOff != llcOff))
    {
        if (tlOff == ltlOff-1)
        {
            movetxt(1,MAX_ROWS-(statOn?2:0)-1,2);
        }
        else if (tlOff == ltlOff+1)
        {
            movetxt(2,MAX_ROWS-(statOn?2:0),1);
        }
        else
        {
            for (int i = 0; i < MAX_ROWS-(statOn?2:0); ++i)
            {
                if (i == slOff) continue;
                setXY(1,i+1);
                dispLine((tlOff+i >= MAX_LINES) ? "" : lineList[tlOff+i],
                    lcOff,
                    attrList[(tlOff+i >= MAX_LINES) ? MAX_LINES-1 : tlOff+i],
                    tlOff+i);
            }
        }
        ltlOff = tlOff;
        llcOff = lcOff;
    }

    setXY(1,slOff+1);
    dispLine(curLine,lcOff,attrList[tlOff+slOff],tlOff+slOff);

    // Display status lines
    dispStat();
}

/******************************************************************************/

