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
/*                                 BLOCK.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                        Block manipulation functions                        */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#pragma hdrfile "block.sym"

#include    <string.h>

#include    <dosfunc.h>

#include    <block.hpp>

#include    <atcodes.hpp>
#include    <disp.hpp>
#include    <fast_cio.hpp>
#include    <file.hpp>
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

// Constants

#define blkName "PCBEDIT.BLK"

/******************************************************************************/

// Variables

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

void  noBlock(void)
{
    llcOff = -1;
    startBlockLine = endBlockLine = -1;
    startBlockOff  = endBlockOff  =  0;
    startBlockLen  = endBlockLen  =  0;
    blockType      = 0;
}

    /*--------------------------------------------------------------------*/

void  markBlock(void)
{
    llcOff = -1;

    if (startBlockLine == -1)
    {
        setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
        setAttr(statAttr);
        wrStr("Mark Block, Lines or Characters (B/L/C):  ");
        cle();

        do {

            blockType = toUPPER(getKey());

        } while ((blockType != 'B') && (blockType != 'L') &&
            (blockType != 'C') && (blockType != '\x1B'));

        if (blockType == '\x1B') blockType = NUL;

        if (blockType != NUL)
        {
            startBlockLine = tlOff+slOff;
            if (blockType == 'L')
                startBlockOff = 0;
            else
                startBlockOff = int(curLineChar-curLine);
            char tmp = *curLineChar;
            *curLineChar = NUL;
            startBlockLen = lineLen(curLine);
            *curLineChar = tmp;
            disableEditKeys = TRUE;
        }
    }
    else if (endBlockLine == -1)
    {
        endBlockLine = tlOff+slOff;
        endBlockOff  = int(curLineChar-curLine);
        char tmp = *curLineChar;
        *curLineChar = NUL;
        endBlockLen  = lineLen(curLine);
        *curLineChar = tmp;

        if (endBlockLine < startBlockLine)
        {
            swap(startBlockLine,endBlockLine);
            swap(startBlockOff, endBlockOff);
            swap(startBlockLen, endBlockLen);
        }

        if (blockType == 'L')
        {
            endBlockOff = (((tlOff+slOff) == endBlockLine) ? strlen(curLine) :
                ((lineList[endBlockLine] == NULL) ? 0 :
                    strlen(lineList[endBlockLine])));
        }

        if (((blockType == 'C') && (startBlockLine == endBlockLine) && (endBlockOff < startBlockOff)) ||
            ((blockType == 'B') && (endBlockLen < startBlockLen)))
        {
            swap(startBlockOff,endBlockOff);
            swap(startBlockLen,endBlockLen);
        }
    }
    else
    {
        noBlock();
        disableEditKeys = FALSE;
    }
}

    /*--------------------------------------------------------------------*/

void  deleteBlock(void)
{
    if (endBlockLine == -1) return;

    int ttl = tlOff, tsl = slOff;

    commitLine();

    switch (blockType)
    {
        case 'B':
            tlOff = 0;
            for (slOff = startBlockLine; slOff <= endBlockLine; ++slOff)
            {
                cacheLine();

                int curLen = 0;

                char * sptr = curLine;
                while ((curLen < startBlockLen) && (*sptr != NUL))
                {
                    int code = doIsATCode(sptr);
                    if (code)
                    {
                        if (code == ATPOS)
                        {
                            if (curLen < atWidth) curLen = atWidth;
                        }
                        else
                        {
                            curLen += atWidth;
                        }
                        sptr += atSize;
                    }
                    else if (*sptr == TAB)
                    {
                        curLen = (((curLen)/8)+1)*8+1;
                        ++sptr;
                    }
                    else
                    {
                        ++curLen;
                        ++sptr;
                    }
                }

                char * eptr = sptr;
                while ((curLen <= endBlockLen) && (*eptr != NUL))
                {
                    int code = doIsATCode(eptr);
                    if (code)
                    {
                        if (code == ATPOS)
                        {
                            if (curLen < atWidth) curLen = atWidth;
                        }
                        else
                        {
                            curLen += atWidth;
                        }
                        eptr += atSize;
                    }
                    else if (*eptr == TAB)
                    {
                        curLen = (((curLen)/8)+1)*8+1;
                        ++eptr;
                    }
                    else
                    {
                        ++curLen;
                        ++eptr;
                    }
//                    if (code == ATPOS)
//                    {
//                        if (curLen < atWidth) curLen = atWidth;
//                        eptr += atSize;
//                    }
//                    else if (code)
//                    {
//                        curLen += atWidth;
//                        eptr += atSize;
//                    }
//                    else
//                    {
//                        ++curLen;
//                        ++eptr;
//                    }
                }

                strcpy(sptr,eptr);

                commitLine();
            }
            break;

        case 'C':
            ++startBlockLine;
            --endBlockLine;
            if (startBlockLine >= endBlockLine) break;

        case 'L':
        {
            tlOff = 0;
            slOff = startBlockLine;
            curLineChar = curLine;
            lcOff = scOff = 0;
            cacheLine();
            for (int i = startBlockLine; i <= endBlockLine; ++i)
            {
                memset(curLine,0,MAX_LINE_SIZE+1);
                joinLines();
            }
            commitLine();
            break;
        }
    }

    if (blockType == 'C')
    {
        --startBlockLine;
        ++endBlockLine;

        slOff = startBlockLine;
        cacheLine();

        if (startBlockLine == endBlockLine)
        {
            strcpy(curLine+startBlockOff,curLine+endBlockOff+1);
        }
        else
        {
            strcpy(curLine+startBlockOff,"");
            commitLine();
            slOff = endBlockLine;
            cacheLine();
            strcpy(curLine,curLine+endBlockOff+1);
        }

        commitLine();
    }

    tlOff = 0, slOff = startBlockLine;
    chkAttrs(TRUE);

    tlOff = ttl, slOff = tsl;

    cacheLine();

    textChanged = TRUE;
    llcOff = -1;

    noBlock();
    disableEditKeys = FALSE;
}

    /*--------------------------------------------------------------------*/

void  wrText(int hand, char * ptr, int & errFlag)
{
    int len = strlen(ptr);
    errFlag = ((errFlag)                                           ||
               (writecheck(hand,&len,sizeof(len)) == unsigned(-1)) ||
               (writecheck(hand,ptr,len)          == unsigned(-1)));
}

    /*------------------------------------------------------------------*/

void  writeBlock(void)
{
    if (endBlockLine == -1) return;

    int hand = doscreatecheck(blkName,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);
    if (hand == -1)
    {
        fileErr("File create error",blkName);
        return;
    }

    commitLine();

    int errFlag =
        ((writecheck(hand,"PCBEdit Block File 1.00\r\n\x1A",26) == unsigned(-1)) ||
         (writecheck(hand,&blockType,sizeof(blockType))         == unsigned(-1)));

    if (!errFlag)
    {
        int ttl = tlOff, tsl = slOff;
        switch (blockType)
        {
            case 'B':
                tlOff = 0;
                for (slOff = startBlockLine; slOff <= endBlockLine; ++slOff)
                {
                    cacheLine();

                    memset(tmpLine,0,MAX_LINE_SIZE+1);

                    int curLen = 0;

                    char * sptr = curLine;
                    while ((curLen < startBlockLen) && (*sptr != NUL))
                    {
                        int code = doIsATCode(sptr);
                        if (code)
                        {
                            if (code == ATPOS)
                            {
                                if (curLen < atWidth) curLen = atWidth;
                            }
                            else
                            {
                                curLen += atWidth;
                            }
                            sptr += atSize;
                        }
                        else if (*sptr == TAB)
                        {
                            curLen = (((curLen)/8)+1)*8+1;
                            ++sptr;
                        }
                        else
                        {
                            ++curLen;
                            ++sptr;
                        }
//                        if (code == ATPOS)
//                        {
//                            if (curLen < atWidth) curLen = atWidth;
//                            sptr += atSize;
//                        }
//                        else if (code)
//                        {
//                            curLen += atWidth;
//                            sptr += atSize;
//                        }
//                        else
//                        {
//                            ++curLen;
//                            ++sptr;
//                        }
                    }

                    while ((curLen < startBlockLen) &&
                        ((sptr-curLine) < MAX_LINE_SIZE))
                    {
                        ++curLen;
                        *(sptr++) = ' ';
                    }

                    strcpy(tmpLine,sptr);

                    char * eptr = tmpLine;
                    while ((curLen <= endBlockLen) && (*eptr != NUL))
                    {
                        int code = doIsATCode(eptr);
                        if (code)
                        {
                            if (code == ATPOS)
                            {
                                if (curLen < atWidth) curLen = atWidth;
                            }
                            else
                            {
                                curLen += atWidth;
                            }
                            eptr += atSize;
                        }
                        else if (*eptr == TAB)
                        {
                            curLen = (((curLen)/8)+1)*8+1;
                            ++eptr;
                        }
                        else
                        {
                            ++curLen;
                            ++eptr;
                        }
//                        if (code == ATPOS)
//                        {
//                            if (curLen < atWidth) curLen = atWidth;
//                            eptr += atSize;
//                        }
//                        else if (code)
//                        {
//                            curLen += atWidth;
//                            eptr += atSize;
//                        }
//                        else
//                        {
//                            ++curLen;
//                            ++eptr;
//                        }
                    }

                    while ((curLen <= endBlockLen) &&
                        ((sptr-tmpLine) < MAX_LINE_SIZE))
                    {
                        ++curLen;
                        *(eptr++) = ' ';
                    }

                    *eptr = NUL;

                    wrText(hand,tmpLine,errFlag);

                    commitLine();

                    if (errFlag) break;
                }
                break;

            case 'C':
                tlOff = 0;
                slOff = startBlockLine;
                if (startBlockLine == endBlockLine)
                {
                    cacheLine();
                    strcpy(tmpLine,curLine);
                    commitLine();

                    tmpLine[endBlockOff+1] = NUL;
                    strcpy(tmpLine,tmpLine+startBlockOff);
                    wrText(hand,tmpLine,errFlag);
                }
                else
                {
                    cacheLine();
                    strcpy(tmpLine,curLine);
                    commitLine();

                    strcpy(tmpLine,tmpLine+startBlockOff);
                    wrText(hand,tmpLine,errFlag);

                    for (slOff = startBlockLine+1;
                        (slOff <= (endBlockLine-1)) && !errFlag;
                        ++slOff)
                    {
                        cacheLine();
                        wrText(hand,curLine,errFlag);
                        commitLine();
                    }

                    if (!errFlag)
                    {
                        slOff = endBlockLine;
                        cacheLine();
                        strcpy(tmpLine,curLine);
                        commitLine();

                        tmpLine[endBlockOff+1] = NUL;
                        wrText(hand,tmpLine,errFlag);
                    }
                }
                break;

            case 'L':
                tlOff = 0;
                for (slOff = startBlockLine; slOff <= endBlockLine; ++slOff)
                {
                    cacheLine();
                    wrText(hand,curLine,errFlag);
                    commitLine();
                    if (errFlag) break;
                }
                break;
        }
        tlOff = ttl, slOff = tsl;
    }

    cacheLine();

    dosclose(hand);

    textChanged = TRUE;

    if (errFlag)
        fileErr("File write error",blkName);
    else
        disableEditKeys = FALSE;
}

    /*--------------------------------------------------------------------*/

void  rdText(int hand, char * ptr, int & errFlag)
{
    memset(ptr,0,MAX_LINE_SIZE+1);
    if (errFlag) return;

    int len;
    errFlag = (readcheck(hand,&len,sizeof(len)) != sizeof(len));
    if (errFlag) return;

    errFlag = (readcheck(hand,ptr,len) != len);
}

    /*--------------------------------------------------------------------*/

void  readBlock(void)
{
    if (startBlockLine != -1) return;

    int hand = dosopencheck(blkName,OPEN_READ|OPEN_DENYWRIT);
    if (hand == -1)
    {
        fileErr("File open error",blkName);
        return;
    }

    memset(tmpLine,0,MAX_LINE_SIZE+1);
    if ((readcheck(hand,tmpLine,26) != 26) ||
        (strcmp("PCBEdit Block File 1.00\r\n\x1A",tmpLine) != 0))
    {
        fileErr("Invalid block file",blkName);
        dosclose(hand);
        return;
    }

    blockType = 0;
    if ((readcheck(hand,&blockType,sizeof(blockType)) != sizeof(blockType)) &&
        (blockType != 'B') && (blockType != 'L') && (blockType != 'C'))
    {
        fileErr("Invalid block file",blkName);
        dosclose(hand);
        return;
    }

    commitLine();

    int errFlag = FALSE;

    int ttl = tlOff, tsl = slOff;

    slOff += tlOff;
    tlOff = 0;

    switch (blockType)
    {
        case 'B':
        {
            int ulOff = lcOff+scOff;
            while (!errFlag)
            {
                cacheLine();

                int curLen = 0;

                char * sptr = curLine;
                while ((curLen < ulOff) && (*sptr != NUL))
                {
                    int code = doIsATCode(sptr);
                    if (code)
                    {
                        if (code == ATPOS)
                        {
                            if (curLen < atWidth) curLen = atWidth;
                        }
                        else
                        {
                            curLen += atWidth;
                        }
                        sptr += atSize;
                    }
                    else if (*sptr == TAB)
                    {
                        curLen = (((curLen)/8)+1)*8+1;
                        ++sptr;
                    }
                    else
                    {
                        ++curLen;
                        ++sptr;
                    }
//                    if (code == ATPOS)
//                    {
//                        if (curLen < atWidth) curLen = atWidth;
//                        sptr += atSize;
//                    }
//                    else if (code)
//                    {
//                        curLen += atWidth;
//                        sptr += atSize;
//                    }
//                    else
//                    {
//                        ++curLen;
//                        ++sptr;
//                    }
                }

                while ((curLen < ulOff) && ((sptr-curLine) < MAX_LINE_SIZE))
                {
                    ++curLen;
                    *(sptr++) = ' ';
                }

                rdText(hand,tmpLine,errFlag);

                strncat(tmpLine,sptr,MAX_LINE_SIZE+1-strlen(tmpLine));
                tmpLine[MAX_LINE_SIZE] = NUL;

                *sptr = NUL;
                strncat(curLine,tmpLine,MAX_LINE_SIZE+1-strlen(curLine));
                curLine[MAX_LINE_SIZE] = NUL;

                commitLine();

                if (errFlag) break;

                ++slOff;
            }
            break;
        }

        case 'C':
        {
            int splitFlag = FALSE;
            while (!errFlag)
            {
                rdText(hand,tmpLine,errFlag);
                if (errFlag) break;

                if (splitFlag)
                {
                    if (lineList[MAX_LINES-1] != NULL)
                    {
                        editErr("Can't read entire block, all lines in use");
                        break;
                    }
                    cacheLine();
                    splitLine();
                    commitLine();
                }
                else
                    splitFlag = TRUE;

                cacheLine();

                int tmpLen = strlen(tmpLine);

                strncat(tmpLine,curLineChar,MAX_LINE_SIZE+1-strlen(tmpLine));
                tmpLine[MAX_LINE_SIZE] = NUL;

                *curLineChar = NUL;
                strncat(curLine,tmpLine,MAX_LINE_SIZE+1-strlen(curLine));
                curLine[MAX_LINE_SIZE] = NUL;

                curLineChar += tmpLen;
                if (curLineChar > curLine+MAX_LINE_SIZE)
                    curLineChar = curLine+MAX_LINE_SIZE;

                commitLine();
            }
            break;
        }

        case 'L':
            while (!errFlag)
            {
                rdText(hand,tmpLine,errFlag);
                if (errFlag) break;

                if (lineList[MAX_LINES-1] != NULL)
                {
                    editErr("Can't read entire block, all lines in use");
                    break;
                }

                cacheLine();

                curLineChar = curLine;
                splitLine();

                commitLine();
                --slOff;
                cacheLine();

                strcpy(curLine,tmpLine);

                commitLine();
                ++slOff;
            }
            break;
    }

    blockType = 0;

    tlOff = ttl, slOff = tsl;
    chkAttrs(TRUE);

    cacheLine();

    adjustCLC();

    textChanged = TRUE;
    llcOff = -1;

    dosclose(hand);
}

    /*--------------------------------------------------------------------*/

void  attrBlock(void)
{
    int a;

    if ((startBlockLine == -1) || (endBlockLine != -1))
        a = selectColor(startBlockLine);

    if ((startBlockLine != -1) && (endBlockLine == -1))
    {
        buzz();
    }
    else if ((endBlockLine != -1) && (a != -1))
    {
        commitLine();

        int ttl = tlOff, tsl = slOff;

        bool wr00Flag = FALSE;
        int  wr00Attr = 0;

        int  lastATX = attrList[0];

        tlOff = 0;
        for (slOff = 0; slOff < startBlockLine; ++slOff)
        {
            char * p = lineList[slOff];
            while (*p != NUL)
            {
                if (memcmp(p,"@X00",4) == 0)
                {
                    p += 4;
                    continue;
                }

                int code = doIsATCode(p);
                if (code == ATXCODE)
                    lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));

                p += (code ? atSize : 1);
            }
        }

        switch (blockType)
        {
            case 'B':
                tlOff = 0;
                for (slOff = startBlockLine; slOff <= endBlockLine; ++slOff)
                {
                    cacheLine();

                    wr00Flag = FALSE;

                    int curLen = 0;

                    char * p = curLine;
                    char * sptr = NULL;

                    while (curLen <= endBlockLen)
                    {
                        if (*p == NUL) memcpy(p," ",2);

                        if ((curLen == startBlockLen) && (sptr == NULL))
                            sptr = p;

                        if (memcmp(p,"@X00",4) == 0)
                        {
                            if (curLen >= startBlockLen)
                            {
                                wr00Flag = TRUE;
                                wr00Attr = lastATX;
                            }
                            p += 4;
                            continue;
                        }

                        int code = doIsATCode(p);
                        if (code == ATXCODE)
                            lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));

                        p += (code ? atSize : 1);
                        curLen += (code ? atWidth : 1);
                    }

                    if (sptr != NULL)
                    {
                        p = sptr;
                        curLen = startBlockLen;
                        if (wr00Flag)
                        {
                            memmove(p+8,p,strlen(p)+1);
                            memcpy(p,"@X00@X00",8);
                            p[2] = hexDigits[(wr00Attr&0xF0)>>4];
                            p[3] = hexDigits[(wr00Attr&0x0F)];
                            p += 8;
                        }

                        memmove(p+4,p,strlen(p)+1);
                        memcpy(p,"@X00@X00",4);
						p[2] = hexDigits[(a&0xF0)>>4];
						p[3] = hexDigits[(a&0x0F)];
                        p += 4;

                        while ((*p != NUL) && (curLen <= endBlockLen))
                        {
                            int code = doIsATCode(p);
                            if (code == ATXCODE)
                            {
                                strcpy(p,p+4);
                                continue;
                            }

                            p += (code ? atSize : 1);
                            curLen += (code ? atWidth : 1);
                        }

                        memmove(p+4,p,strlen(p)+1);
                        memcpy(p,"@X00@X00",4);
						p[2] = hexDigits[(lastATX&0xF0)>>4];
						p[3] = hexDigits[(lastATX&0x0F)];
                    }

                    p = curLine;
                    while (*p != NUL)
                    {
                        if (memcmp(p,"@X00",4) == 0)
                        {
                            p += 4;
                            continue;
                        }

                        int code = doIsATCode(p);
                        if (code == ATXCODE)
                            lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));

                        p += (code ? atSize : 1);
                        curLen += (code ? atWidth : 1);
                    }

                    commitLine();
                }
                break;

            case 'L':
            case 'C':
                tlOff = 0;
                slOff = startBlockLine;

                cacheLine();

                if (startBlockLine == endBlockLine)
                {
//                    cacheLine();

                    wr00Flag = FALSE;

                    int curOff = 0;

                    char * p = curLine;
                    char * sptr = curLine+startBlockOff;

                    while (curOff <= endBlockOff)
                    {
                        if (*p == NUL) memcpy(p," ",2);

                        if (memcmp(p,"@X00",4) == 0)
                        {
                            if (curOff >= startBlockOff)
                            {
                                wr00Flag = TRUE;
                                wr00Attr = lastATX;
                            }
                            p += 4;
                            curOff += 4;
                            continue;
                        }

                        int code = doIsATCode(p);
                        if (code == ATXCODE)
                            lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));

                        p += (code ? atSize : 1);
                        curOff += (code ? atSize : 1);
                    }

                    if (sptr != NULL)
                    {
                        p = sptr;
                        curOff = startBlockOff;
                        if (wr00Flag)
                        {
                            memmove(p+8,p,strlen(p)+1);
                            memcpy(p,"@X00@X00",8);
                            p[2] = hexDigits[(wr00Attr&0xF0)>>4];
                            p[3] = hexDigits[(wr00Attr&0x0F)];
                            p += 8;
                            curOff += 8;
                            endBlockOff += 8;
                        }

                        memmove(p+4,p,strlen(p)+1);
                        memcpy(p,"@X00@X00",4);
                        p[2] = hexDigits[(a&0xF0)>>4];
                        p[3] = hexDigits[(a&0x0F)];
                        p += 4;
                        curOff += 4;
                        endBlockOff += 4;

                        while ((*p != NUL) && (curOff <= endBlockOff))
                        {
                            int code = doIsATCode(p);
                            if (code == ATXCODE)
                            {
                                strcpy(p,p+4);
                                endBlockOff -= 4;
                                continue;
                            }

                            p += (code ? atSize : 1);
                            curOff += (code ? atSize : 1);
                        }

                        memmove(p+4,p,strlen(p)+1);
                        memcpy(p,"@X00@X00",4);
                        p[2] = hexDigits[(lastATX&0xF0)>>4];
                        p[3] = hexDigits[(lastATX&0x0F)];
                    }
                }
                else
                {
                    wr00Flag = FALSE;

                    int curOff = 0;

                    char * p = curLine;
                    char * sptr = curLine+startBlockOff;

                    while (*p != NUL)
                    {
                        if (memcmp(p,"@X00",4) == 0)
                        {
                            if (curOff >= startBlockOff)
                            {
                                wr00Flag = TRUE;
                                wr00Attr = lastATX;
                                strcpy(p,p+4);
                            }
                            else
                            {
                                p += 4;
                                curOff += 4;
                            }
                            continue;
                        }

                        int code = doIsATCode(p);
                        if (code == ATXCODE)
                        {
                            lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));
                            if (curOff >= startBlockOff)
                            {
                                strcpy(p,p+4);
                                continue;
                            }
                        }

                        p += (code ? atSize : 1);
                        curOff += (code ? atSize : 1);
                    }

                    commitLine();

                    for (slOff = startBlockLine+1; slOff < endBlockLine;
                        ++slOff)
                    {
                        cacheLine();
                        p = curLine;
                        while (*p != NUL)
                        {
                            if (memcmp(p,"@X00",4) == 0)
                            {
                                wr00Flag = TRUE;
                                wr00Attr = lastATX;
                                strcpy(p,p+4);
                                continue;
                            }

                            int code = doIsATCode(p);
                            if (code == ATXCODE)
                            {
                                lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));
                                strcpy(p,p+4);
                                continue;
                            }

                            p += (code ? atSize : 1);
                            curOff += (code ? atSize : 1);
                        }
                        commitLine();
                    }

                    slOff = endBlockLine;
                    cacheLine();

                    curOff = 0;

                    p = curLine;

                    while (curOff <= endBlockOff)
                    {
                        if (*p == NUL) memcpy(p," ",2);

                        if (memcmp(p,"@X00",4) == 0)
                        {
                            if (curOff >= startBlockOff)
                            {
                                wr00Flag = TRUE;
                                wr00Attr = lastATX;
                            }
                            strcpy(p,p+4);
                            endBlockOff -= 4;
                            continue;
                        }

                        int code = doIsATCode(p);
                        if (code == ATXCODE)
                        {
                            lastATX = (hex2dec(*(p+2))<<4)+hex2dec(*(p+3));
                            strcpy(p,p+4);
                            endBlockOff -= 4;
                            continue;
                        }

                        p += (code ? atSize : 1);
                        curOff += (code ? atSize : 1);
                    }

                    memmove(p+4,p,strlen(p)+1);
                    memcpy(p,"@X00@X00",4);
                    p[2] = hexDigits[(lastATX&0xF0)>>4];
                    p[3] = hexDigits[(lastATX&0x0F)];

                    commitLine();
                    slOff = startBlockLine;
                    cacheLine();

                    p = sptr;

                    if (strlen(curLine) < startBlockOff)
                    {
                        memset(curLine+strlen(curLine),' ',
                            2048-strlen(curLine));
                        *p = NUL;
                    }

                    if (wr00Flag)
                    {
                        memmove(p+8,p,strlen(p)+1);
                        memcpy(p,"@X00@X00",8);
                        p[2] = hexDigits[(wr00Attr&0xF0)>>4];
                        p[3] = hexDigits[(wr00Attr&0x0F)];
                        p += 8;
                    }

                    memmove(p+4,p,strlen(p)+1);
                    memcpy(p,"@X00@X00",4);
                    p[2] = hexDigits[(a&0xF0)>>4];
                    p[3] = hexDigits[(a&0x0F)];
                    p += 4;
                }

                commitLine();

                break;
        }
        tlOff = slOff = 0;
        chkAttrs(TRUE);
        tlOff = ttl, slOff = tsl;
        cacheLine();
        textChanged = TRUE;
        noBlock();
        disableEditKeys = FALSE;
        adjustCLC();
    }
}

/******************************************************************************/

