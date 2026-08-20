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
/*                                MISCEDIT.CPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                  Miscellaneous functions used by PCBEdit.                  */
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

#pragma hdrfile "miscedit.sym"

#include    <ctype.h>
#include    <dir.h>
#include    <process.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    <dosfunc.h>

#include    <miscedit.hpp>

#include    <atcodes.hpp>
#include    <disp.hpp>
#include    <fast_cio.hpp>
#include    <file.hpp>
#include    <initrest.hpp>
#include    <keyboard.hpp>
#include    <move.hpp>
#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Variables

char hexDigits[] = "0123456789ABCDEF";

char nodosFlag = FALSE;

/******************************************************************************/

// Functions

int pascal confirm(char * s, void pascal (* func)(void), char key)
{
    setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
    setAttr(statAttr);
    wrStr(s);
    cle();

    int ch = toUPPER(getKey());

    if ((func != NULL) && (ch == key))
    {
        func();
        ch = 'Y';
    }

    return (ch == 'Y');
}

    /*--------------------------------------------------------------------*/

int pascal confirmExit(void)
{
    // If text not changed go ahead and exit
    if (!textChanged) return TRUE;

    // Display a prompt on the status line
    return confirm("Text has been modified, do you really wish to exit "
        "(Y/N/S)?  ",saveFile,'S');
}

    /*--------------------------------------------------------------------*/

#define ATTR_X      2

int pascal selectColor(int sbl)
{
    setcurstype(NOCURS);

    llcOff = -1;

    setAttr(helpAttr);
    cls();
    setXY(33,1);
    wrStr("Select Attribute");
    setXY(33,2);
    wrStr("컴컴컴컴컴컴컴컴");
    setXY(ATTR_X,5);
    wrStr("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    setXY(ATTR_X,22);
    wrStr("     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
    setXY(2,24);
    wrStr("\x1B"" & ""\x1A"" to select foreground attribute  --  "
        "\x18"" & ""\x19"" to select background attribute");
    setXY(9,25);
    wrStr("0-F to enter the desired background and/or foreground attribute");

    int row, col;
    for (row = 0; row < 16; ++row)
    {
        setAttr(helpAttr);
        setXY(ATTR_X+2,5+1+row);
        wrChar(hexDigits[row]);
        setXY(ATTR_X+53,5+1+row);
        wrChar(hexDigits[row]);
        for (col = 0; col < 16; ++col)
        {
            setXY(ATTR_X+4+col*3,5+1+row);
            if ((row*16+col) == 0x00)
            {
                setAttr(7);
                wrStr("SAV");
            }
            else if ((row*16+col) == 0xFF)
            {
                setAttr(7);
                wrStr("REC");
            }
            else
            {
                setAttr(row*16+col);
                wrStr("***");
            }
        }
    }
    setAttr(helpAttr);

    int attr = attrList[tlOff+slOff];
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
    row = (attr & 0xF0) >> 4;
    col = (attr & 0x0F);

    int ch;
    int digRow = TRUE;
    do {

        setAttr(helpAttr);

        setXY(ATTR_X+5+col*3,4);
        wrChar(31);
        setXY(ATTR_X+1,5+1+row);
        wrChar(16);
        setXY(ATTR_X+5+col*3,23);
        wrChar(30);
        setXY(ATTR_X+54,5+1+row);
        wrChar(17);

        setXY(67,8);
        fastprintf("@X%02X",row*16+col);

        setAttr(row*16+col);
        setXY(61,12);
        wrStr("旼컴컴컴컴컴컴커");
        setXY(61,13);
        wrStr("� ABCDEFGHIJKL �");
        setXY(61,14);
        wrStr("� abcdefghijkl �");
        setXY(61,15);
        wrStr("�  0123456789  �");
        setXY(61,16);
        wrStr("�   같굇께栢   �");
        setXY(61,17);
        wrStr("� !@#$%^&*()-+ �");
        setXY(61,18);
        wrStr("� 褻栒鴨鉛僥幽 �");
        setXY(61,19);
        wrStr("읕컴컴컴컴컴컴켸");

        ch = getKey();

        setAttr(helpAttr);

        setXY(ATTR_X+5+col*3,4);
        wrChar(' ');
        setXY(ATTR_X+1,5+1+row);
        wrChar(' ');
        setXY(ATTR_X+5+col*3,23);
        wrChar(' ');
        setXY(ATTR_X+54,5+1+row);
        wrChar(' ');

        if (ch == LEFT)
        {
            if (--col < 0) col += 16;
        }
        else if (ch == RIGHT)
        {
            if (++col >= 16) col -= 16;
        }
        else if (ch == UP)
        {
            if (--row < 0) row += 16;
        }
        else if (ch == DOWN)
        {
            if (++row >= 16) row -= 16;
        }
        else if (isXDIGIT(ch))
        {
            ch = toUPPER(ch);
            if (digRow)
                row = hex2dec(char(ch));
            else
                col = hex2dec(char(ch));
            digRow = !digRow;
        }

    } while ((ch != ENTER) && (ch != ESC));

    if ((ch == ENTER) && (strlen(curLine)+4 <= MAX_LINE_SIZE) && (sbl == -1))
    {
        memmove(curLineChar+4,curLineChar,strlen(curLineChar)+1);
        *(curLineChar++) = '@';
        *(curLineChar++) = 'X';
        *(curLineChar++) = hexDigits[row];
        *(curLineChar++) = hexDigits[col];
        textChanged = TRUE;
        udLine();
        lineList[tlOff+slOff] = curLine;
        chkAttrs();
        lineList[tlOff+slOff] = NULL;
    }

    cls();
    dispCurs();

    return ((ch != ESC) ? ((row << 4) + col) : -1);
}

    /*--------------------------------------------------------------------*/

void pascal selectChar(void)
{
    setcurstype(NOCURS);

    llcOff = -1;

    setAttr(helpAttr);
    cls();
    setXY(33,1);
    wrStr("Select Character");
    setXY(33,2);
    wrStr("컴컴컴컴컴컴컴컴");
    setXY(24,24);
    wrStr("\x18"" & ""\x19"" & ""\x1B"" & ""\x1A"" to select character");
    setXY(18,25);
    wrStr("Space Bar to toggle between Low and High ASCII");

    int row, col;
    for (row = 0; row < 16; ++row)
    {
        for (col = 0; col < 8; ++col)
        {
            setXY(col*10+3,6+row);
            int ch = col*16+row;
            fastprintf("%3d %c",ch,
                (strchr("\x07\x08\x09\x0A\x0C\x0D\x1A\x1B",ch) == NULL) ?
                (char) ch : ' ');
        }
    }

    row = col = 0;

    int ch, high = 0;
    do {

        setXY(col*10+1,6+row);
        wrChar(16);

        ch = getKey();

        setXY(col*10+1,6+row);
        wrChar(' ');

        if (ch == LEFT)
        {
            if (--col < 0) col += 8;
        }
        else if (ch == RIGHT)
        {
            if (++col >= 8) col -= 8;
        }
        else if (ch == UP)
        {
            if (--row < 0) row += 16;
        }
        else if (ch == DOWN)
        {
            if (++row >= 16) row -= 16;
        }
        else if (ch == ' ')
        {
            high = !high;
            for (int r = 0; r < 16; ++r)
            {
                for (int c = 0; c < 8; ++c)
                {
                    setXY(c*10+3,6+r);
                    int ch = (c+(high?8:0))*16+r;
                    fastprintf("%3d %c",ch,
                        (strchr("\x07\x08\x09\x0A\x0C\x0D\x1A\x1B",ch) == NULL) ?
                        (char) ch : ' ');
                }
            }
        }

    } while ((ch != ENTER) && (ch != ESC));

    int tmp = (col+(high?8:0))*16+row;
    if (ch == ENTER)
        procChar((strchr("\x07\x08\x09\x0A\x0C\x0D\x1A\x1B",tmp) == NULL) ?
            tmp : ' ');

    cls();
    dispCurs();
}

    /*--------------------------------------------------------------------*/

void pascal saveFKeyChar(char * s)
{
    int hand = doscreatecheck(s,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);
    if (hand != -1)
    {
        for (int i = 0; i < 15; ++i)
            if (writecheck(hand,graphChars[i],10) == unsigned(-1))
            {
                fileErr("File write error",s);
                break;
            }
        dosclose(hand);
    }
    else
    {
        fileErr("File open error",s);
    }
}

    /*--------------------------------------------------------------------*/

void pascal loadFKeyChar(char * s)
{
    int hand = dosopencheck(s,OPEN_READ|OPEN_DENYWRIT);
    if (hand != -1)
    {
        for (int i = 0; i < 15; ++i)
            if (readcheck(hand,graphChars[i],10) != 10)
            {
                fileErr("File read error",s);
                break;
            }
        dosclose(hand);
    }
    else
    {
        fileErr("File open error",s);
    }
}

    /*--------------------------------------------------------------------*/

#define SFKC_XOFF   25

void pascal selectFKeyChar(void)
{
    int i;
    setcurstype(NOCURS);

    int lastCharSet = charSet;
    char lastGraphChars[15][10];
    for (i = 0; i < 15; ++i) memcpy(lastGraphChars[i],graphChars[i],10);

    llcOff = -1;

    setAttr(helpAttr);
    cls();
    setXY(16,1);
    wrStr("Select Character from Function Key Sets Available");
    setXY(16,2);
    wrStr("컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�");
    setXY(11,23);
    wrStr("\x18"" & ""\x19"" to select function key set - "
        "\x1B"" & ""\x1A"" to select character");
    setXY(6,24);
    wrStr("ESC to abort -- ENTER to exit -- SPACE BAR to select -- "
        "^ENTER to edit");
    setXY(20,25);
    wrStr("Alt-L to load a set -- Alt-S to save a set");

    for (i = 0; i < 15; ++i)
    {
        setXY(1+SFKC_XOFF,5+i);
        fastprintf("   %2d - ",i+1);
        for (int j = 0; j < 10; ++j)
            wrChar(graphChars[i][j]),
            wrChar(' ');
    }

    static int off = 0;
    int ch;

    do {

        setAttr(helpAttr);
        setXY(2+SFKC_XOFF,charSet+5);
        wrChar(16);
        setAttr(((helpAttr&0x07)<<4)+((helpAttr&0x70)>>4));
        setXY(9+(off*2)+SFKC_XOFF,charSet+5);
        wrChar(graphChars[charSet][off]);
        setAttr(helpAttr);

        ch = getKey();

        setXY(2+SFKC_XOFF,charSet+5);
        wrChar(' ');
        setXY(9+(off*2)+SFKC_XOFF,charSet+5);
        wrChar(graphChars[charSet][off]);

        if (ch == LEFT)
        {
            if (--off < 0) off += 10;
        }
        else if (ch == RIGHT)
        {
            if (++off >= 10) off -= 10;
        }
        else if (ch == UP)
        {
            if (--charSet < 0) charSet += 15;
        }
        else if (ch == DOWN)
        {
            if (++charSet >= 15) charSet -= 15;
        }
        else if (ch == LF)
        {
            setAttr(helpAttr^0x80);
            setXY(9,21);
            wrStr("Enter the character with which to replace the current "
                "character");

            int nch;
            do {

                nch = getKey();
                if (strchr("\x07\x08\x09\x0A\x0C\x0D\x1A",nch) != NULL)
                    nch = 256;

            } while (nch >= 256);

            if (nch != ESC) graphChars[charSet][off] = (char) nch;

            setAttr(helpAttr);
            setXY(1,21);
            cle();
        }
        else if (ch == AS)
        {
            char tmpFN[PN_SIZE+1];
            strcpy(tmpFN,"");

            setAttr(helpAttr);
            setXY(1,21);
            wrStr("Enter filename to save:  ");
            setcurstype(SOLIDCURS);

            if (getStr(tmpFN,PN_SIZE,F_ALPHA|F_NUM|F_PATH|F_FILE|F_UPPER))
                saveFKeyChar(tmpFN);

            setcurstype(NOCURS);
            setXY(1,21);
            cle();
        }
        else if (ch == AL)
        {
            char tmpFN[PN_SIZE+1];
            strcpy(tmpFN,"");

            setAttr(helpAttr);
            setXY(1,21);
            wrStr("Enter filename to load:  ");
            setcurstype(SOLIDCURS);

            if (getStr(tmpFN,PN_SIZE,F_ALPHA|F_NUM|F_PATH|F_FILE|F_UPPER))
                loadFKeyChar(tmpFN);

            setcurstype(NOCURS);
            setXY(1,21);
            cle();
        }

    } while ((ch != ENTER) && (ch != ESC) && (ch != ' '));

    int tmp = graphChars[charSet][off];
    if (ch == ' ')
        procChar((strchr("\x07\x08\x09\x0A\x0C\x0D\x1A\x1B",ch) == NULL) ?
            tmp : ' ');
    else if (ch == ESC)
    {
        charSet = lastCharSet;
        for (i = 0; i < 15; ++i) memcpy(graphChars[i],lastGraphChars[i],10);
    }

    cls();
    dispCurs();
}

    /*--------------------------------------------------------------------*/

void pascal saveMacros(char * s)
{
    int     j;
    DOSFILE fs;
    if (dosfopen(s,OPEN_WRIT|OPEN_DENYRDWR,&fs) != -1)
    {
        for (int i = 0; i < 15; ++i)
        {
            for (j = 0; j < 10; ++j)
            {
                char * ptr = macros[i][j];
                if (ptr == NULL) ptr = "";
                if ((dosfputs(ptr,&fs) == -1) ||
                    (dosfputs("\r\n",&fs) == -1))
                {
                    fileErr("File write error",s);
                    break;
                }
            }
            if (j != 10) break;
        }
        if (dosfclose(&fs) == -1)
            fileErr("File close error",s);
    }
    else
    {
        fileErr("File open error",s);
    }
}

    /*--------------------------------------------------------------------*/

void pascal loadMacros(char * s)
{
    int j;
    commitLine();
    DOSFILE fs;
    if (dosfopen(s,OPEN_READ|OPEN_DENYWRIT,&fs) != -1)
    {
        for (int i = 0; i < 15; ++i)
        {
            for (j = 0; j < 10; ++j)
            {
                if (dosfgets(curLine,MAX_LINE_SIZE+1+2,&fs) == -1)
                {
                    fileErr("File read error",s);
                    break;
                }
                curLine[256] = NUL;
                if (macros[i][j]) free(macros[i][j]);
                macros[i][j] = ((strlen(curLine) > 0) ?
                    strdup(curLine) : NULL);
            }
            if (j != 10) break;
        }
        if (dosfclose(&fs) == -1)
            fileErr("File close error",s);
    }
    else
    {
        fileErr("File open error",s);
    }
    cacheLine();
}

    /*--------------------------------------------------------------------*/

#define SMAC_XOFF   0

void pascal selectMacro(void)
{
    int i,j;
    setcurstype(NOCURS);

    int lastMacroSet = macroSet;
    char * lastMacros[15][10];
    for (i = 0; i < 15; ++i)
        for (j = 0; j < 10; ++j)
            lastMacros[i][j] =
                strdup((macros[i][j] != NULL) ? macros[i][j] : "");

    llcOff = -1;

    setAttr(helpAttr);
    cls();
    setXY(35,1);
    wrStr("Select Macro");
    setXY(35,2);
    wrStr("컴컴컴컴컴컴");
    setXY(12,23);
    wrStr("Page Up & Page Down to select set -- "
        "\x18"" & ""\x19"" to select macro");
    setXY(6,24);
    wrStr("ESC to abort -- ENTER to exit -- SPACE BAR to select -- "
        "^ENTER to edit");
    setXY(20,25);
    wrStr("Alt-L to load a set -- Alt-S to save a set");

    static int off = 0;
    int ch;

    do {

        setAttr(helpAttr);

        setXY(35,6);
        fastprintf("Macro Set %2d",macroSet+1);

        for (i = 0; i < 10; ++i)
        {
            setXY(1+SMAC_XOFF,8+i);
            fastprintf("   Macro %2d - ",i+1);
            int x = curx();
            fastprintf("%-*.*s",80-x,80-x,macros[macroSet][i] ?
                macros[macroSet][i] : "");
            if (strlen(macros[macroSet][i]) > (80-x))
            {
                setXY(77,8+i);
                wrStr("...");
            }
        }

        setXY(2+SMAC_XOFF,off+8);
        wrChar(16);

        ch = getKey();

        setXY(2+SMAC_XOFF,off+8);
        wrChar(' ');

        if (ch == PGUP)
        {
            if (--macroSet < 0) macroSet += 15;
        }
        else if (ch == PGDN)
        {
            if (++macroSet >= 15) macroSet -= 15;
        }
        else if (ch == UP)
        {
            if (--off < 0) off += 10;
        }
        else if (ch == DOWN)
        {
            if (++off >= 10) off -= 10;
        }
        else if (ch == LF)
        {
            setcurstype(SOLIDCURS);
            setAttr(helpAttr^0x80);
            setXY(14,21);
            wrStr("Enter the text with which to replace the current macro");
            setAttr(helpAttr);
            setXY(15+SMAC_XOFF,off+8);
            char buf[256+1];
            strcpy(buf,macros[macroSet][off] ? macros[macroSet][off] : "");
            if (getStr(buf,256,F_ALL))
            {
                if (macros[macroSet][off]) free(macros[macroSet][off]);
                macros[macroSet][off] = ((strlen(buf) > 0) ?
                    strdup(buf) : NULL);
            }
            setXY(1,21);
            cle();
            setcurstype(NOCURS);
        }
        else if (ch == AS)
        {
            char tmpFN[PN_SIZE+1];
            strcpy(tmpFN,"");

            setAttr(helpAttr);
            setXY(1,21);
            wrStr("Enter filename to save:  ");
            setcurstype(SOLIDCURS);

            if (getStr(tmpFN,PN_SIZE,F_ALPHA|F_NUM|F_PATH|F_FILE|F_UPPER))
                saveMacros(tmpFN);

            setcurstype(NOCURS);
            setXY(1,21);
            cle();
        }
        else if (ch == AL)
        {
            char tmpFN[PN_SIZE+1];
            strcpy(tmpFN,"");

            setAttr(helpAttr);
            setXY(1,21);
            wrStr("Enter filename to load:  ");
            setcurstype(SOLIDCURS);

            if (getStr(tmpFN,PN_SIZE,F_ALPHA|F_NUM|F_PATH|F_FILE|F_UPPER))
                loadMacros(tmpFN);

            setcurstype(NOCURS);
            setXY(1,21);
            cle();
        }

    } while ((ch != ENTER) && (ch != ESC) && (ch != ' '));

    if (ch == ' ')
        putMacro(macros[macroSet][off]);
    else if (ch == ESC)
    {
        macroSet = lastMacroSet;
        for (i = 0; i < 15; ++i)
            for (j = 0; j < 10; ++j)
            {
                if (macros[i][j]) free(macros[i][j]);
                macros[i][j] = ((strlen(lastMacros[i][j]) > 0) ?
                    strdup(lastMacros[i][j]) : NULL);
            }
    }

    for (i = 0; i < 15; ++i)
        for (j = 0; j < 10; ++j)
            if (lastMacros[i][j])
                free(lastMacros[i][j]);

    cls();
    dispCurs();
}

    /*--------------------------------------------------------------------*/

#define SAV_COLS    6
#define SAV_ROWS    20

void pascal selectATVar(void)
{
    setcurstype(NOCURS);

    llcOff = -1;

    setAttr(helpAttr);
    cls();
    setXY(34,1);
    wrStr("Select @ Macro");
    setXY(34,2);
    wrStr("컴컴컴컴컴컴컴");
    setXY(26,25);
    wrStr("\x18"" & ""\x19"" & ""\x1B"" & ""\x1A"" to select macro");

    int row, col;
    for (row = 0; row < SAV_ROWS; ++row)
        for (col = 0; col < SAV_COLS; ++col)
            if (col*SAV_ROWS+row < ATXCODE-1)
            {
                setXY(2+col*(80/SAV_COLS),(25-SAV_ROWS)/2+row+2);
                char buf[80+1], * ptr = atVarLst[col*SAV_ROWS+row+1];
                sprintf(buf,"%*.*s",strlen(ptr)-2,strlen(ptr)-2,ptr+1);
                buf[80/SAV_COLS-2] = NUL;
                wrStr(buf);
            }

    row = col = 0;

    int ch;
    do {

        setXY(1+col*(80/SAV_COLS),(25-SAV_ROWS)/2+row+2);
        wrChar(16);

        ch = getKey();

        setXY(1+col*(80/SAV_COLS),(25-SAV_ROWS)/2+row+2);
        wrChar(' ');

        if (ch == ' ') ch = ENTER;

        if (ch == LEFT)
        {
            if (--col < 0)
            {
                col += SAV_COLS;
                if ((col*SAV_ROWS+row) >= (ATXCODE-1)) --col;
            }
        }
        else if (ch == RIGHT)
        {
            if (++col >= SAV_COLS) col -= SAV_COLS;
            if ((col*SAV_ROWS+row) >= (ATXCODE-1)) col = 0;
        }
        else if (ch == UP)
        {
            if (--row < 0)
            {
                row += SAV_ROWS;
                if ((col*SAV_ROWS+row) >= (ATXCODE-1)) row = ATXCODE-(SAV_ROWS*(SAV_COLS-1))-2;
            }
        }
        else if (ch == DOWN)
        {
            if (++row >= SAV_ROWS) row -= SAV_ROWS;
            if ((col*SAV_ROWS+row) >= (ATXCODE-1)) row = 0;
        }
        else if ((ch == ENTER) &&
            (strcmp(atVarLst[col*SAV_ROWS+row+1],"RESERVED") == 0))
        {
            buzz();
            ch = 0;
        }

    } while ((ch != ENTER) && (ch != ESC));

    char * p = atVarLst[col*SAV_ROWS+row+1];
    int plen;

    char buf[80+1];
    strcpy(buf,p);

    plen = strlen(p)-1;
    buf[plen] = NUL;

    char width[3+1] = "";
    char just = 'L';

    if (ch == ENTER)
    {
        if ((col*SAV_ROWS+row+1) == ATENV)
        {
            char var[16+1];
            *var = NUL;

            setXY(1,MAX_ROWS);
            setAttr(statAttr);
            wrStr("Environment Variable:  ");
            cle();

            getStr(var,16,F_ALPHA|F_NUM|F_UPPER);

            if (strlen(var) == 0)
                *buf = NUL;
            else
            {
                strcat(buf,"=");
                strcat(buf,var);
            }
        }

        if (*buf)
        {
            if (replaceableATCode[col*SAV_ROWS+row+1] ||
                ((col*SAV_ROWS+row+1) == ATENV))
            {
                setXY(1,MAX_ROWS);
                setAttr(statAttr);
                wrStr("Macro width (ENTER for none):  ");
                cle();

                getStr(width,3,F_NUM);

                if (atoi(width) != 0)
                {
                    setXY(1,MAX_ROWS);
                    wrStr("Justification (L/C/R/T):  ");
                    cle();
                    wrChar(just);
                    int tmp;
                    do {
                        tmp = toUPPER(getKey());
                    } while ((tmp != 'L') && (tmp != 'C') && (tmp != 'R') &&
                        (tmp != 'T') && (tmp != ENTER));
                    if (tmp != ENTER) just = char(tmp);
                }
            }
            else if ((col*SAV_ROWS+row+1) == ATPOS)
            {
                setXY(1,MAX_ROWS);
                setAttr(statAttr);
                wrStr("Column to position cursor to (ENTER to abort):  ");
                cle();

                getStr(width,3,F_NUM);
            }
            else if ((col*SAV_ROWS+row+1) == ATDELAY)
            {
                setXY(1,MAX_ROWS);
                setAttr(statAttr);
                wrStr("Delay time (in tenths of a second/ENTER to abort):  ");
                cle();

                getStr(width,3,F_NUM);
            }

            if (atoi(width) > 0)
            {
                sprintf(buf+strlen(buf),":%s%c%c",width,
                    (just == 'L') ? '@'  : just,
                    (just == 'L') ? NUL : '@');
            }
            else if (((col*SAV_ROWS+row+1) != ATPOS) &&
                ((col*SAV_ROWS+row+1) != ATDELAY))
            {
                strcat(buf,"@");
            }
            else
            {
                *buf = NUL;
            }
        }
    }

    plen = strlen(buf);

    if ((ch == ENTER) && (plen > 0) && (strlen(curLine)+plen <= MAX_LINE_SIZE))
    {
        memmove(curLineChar+plen,curLineChar,strlen(curLineChar)+1);
        memcpy(curLineChar,buf,plen);
        curLineChar += plen;
        textChanged = TRUE;
        udLine();
    }

    cls();
    dispCurs();
}

    /*--------------------------------------------------------------------*/

void pascal dispHelp(void)
{
    setcurstype(NOCURS);

    llcOff = -1;

    setAttr(helpAttr);
    cls();

    setXY(35,1);
    wrStr("PCBEdit Help");
    setXY(35,2);
    wrStr("컴컴컴컴컴컴");

    setXY(3,25);
    wrStr("Esc to abort - PgUp & PgDn to move between screens - Any other key for next");

    setWin(1,4,80,23);
    cls();

SCR1:
    wrStr("\r\n\n\nAlt-A - Select Attribute                Alt-N -\r\n");
    wrStr("Alt-B - Mark Text Block                 Alt-O - Optimize @X Codes\r\n");
    wrStr("Alt-C - Clear the Edit Buffer           Alt-P - Compile PPL Source Code\r\n");
    wrStr("Alt-D - DOS Shell                       Alt-Q -\r\n");
    wrStr("Alt-E -                                 Alt-R - Toggle 43/50 Line Mode\r\n");
    wrStr("Alt-F - Function Key Sets Available     Alt-S - Save a File\r\n");
    wrStr("Alt-G - Macro Sets Available            Alt-T - Toggle Status Display\r\n");
    wrStr("Alt-H - Help Screens                    Alt-U - Use Current Attribute\r\n");
    wrStr("Alt-I - Load Included File              Alt-V - Select @ Macro\r\n");
    wrStr("Alt-J - Justify Line                    Alt-W - Welcome/Information Screen\r\n");
    wrStr("Alt-K - Kill a File                     Alt-X - Exit to DOS\r\n");
    wrStr("Alt-L - Load a File                     Alt-Y - Toggle @ code interpretation\r\n");
    wrStr("Alt-M - Select Character                Alt-Z - Toggle @X code interpretation\r\n\n");

    int ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP:
        case ESC: goto EXIT;
    }

SCR2:
    wrStr("\r\n\n\n^A -                                    ^N -\r\n");
    wrStr("^B -                                    ^O -\r\n");
    wrStr("^C - Copy Marked Block                  ^P - Compile PPL Source/Redirect Errors\r\n");
    wrStr("^D -                                    ^Q -\r\n");
    wrStr("^E -                                    ^R - Replace Text\r\n");
    wrStr("^F - Find Text                          ^S -\r\n");
    wrStr("^G -                                    ^T -\r\n");
    wrStr("^H - Backspace                          ^U -\r\n");
    wrStr("^I - Tab                                ^V - Paste Block\r\n");
    wrStr("^J - ^Enter                             ^W -\r\n");
    wrStr("^K -                                    ^X - Cut Marked Block\r\n");
    wrStr("^L - Repeat Last Find/Replace           ^Y - Delete Line\r\n");
    wrStr("^M - Enter                              ^Z -\r\n\n");

    ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP: goto SCR1;
        case ESC: goto EXIT;
    }

SCR3:
    wrStr("      F1 -       F10 -- Select Graphics Character from the Current Set\r\n");
    wrStr("  Alt-F1 -   Alt-F10 -- Select Character Set  1 - 10\r\n");
    wrStr("     ^F1 -      ^F5  -- Select Character Set 11 - 15\r\n\n\n");
    wrStr("Shift-F1 - Shift-F10 -- Select Macro from the Current Set\r\n");
    wrStr("S-Alt-F1 - S-Alt-F10 -- Select Macro Set  1 - 10\r\n");
    wrStr("   S-^F1 -    S-^F5  -- Select Macro Set 11 - 15\r\n\n\n");
    wrStr(" ^F7 - Cut Block -- ^F8 - Copy Block -- ^F9 - Paste Block -- ^F10 - Move Block\r\n\n\n");
    wrStr("                 \x1B & \x1A & \x18 & \x19 - Move Cursor Left/Right/Up/Down\r\n");
    wrStr("PgUp & PgDn     - Up/Down Full Screen   ^PgUp & ^PgDn - Top/Bottom of Document\r\n");
    wrStr("Home & End      - Start/End of Line     ^Home & ^End  - Top/Bottom of Screen\r\n");
    wrStr("Ins             - Toggle INS/OVR Mode   Del           - Delete Character/Block\r\n");
    wrStr("Backspace       - Cursor Left & Delete  Enter         - Next Line/Split Line\r\n");
    wrStr("Tab & Shift-Tab - Next/Last Tab Stop    Esc           - Abort Current Mode\r\n");

    ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP: goto SCR2;
        case ESC: goto EXIT;
    }

SCR4:
    wrStr("/AT     - Enable @ macro processing     /NOAT    - Disable @ macro processing\r\n");
    wrStr("/ATX    - Enable @X codes (if /AT)      /NOATX   - Disable @X codes (if /AT)\r\n");
    wrStr("/BAK    - Save BAK files                /NOBAK   - Discard BAK files\r\n");
    wrStr("/DOS    - Allow DOS shell               /NODOS   - Disallow DOS shell\r\n");
    wrStr("/SNOW   - Enable snow checking          /NOSNOW  - Disable snow checking\r\n");
    wrStr("/COLOR  - Force into color mode         /MONO    - Force into monochrome mode\r\n");
    wrStr("/CATTR  - Use color attributes          /MATTR   - Use monochrome attributes\r\n");
    wrStr("/QUICK  - Quick start (skip welcome)    /NOQUICK - Slow start (display welcome)\r\n");
    wrStr("/IO     - Allow file access             /NOIO    - Disallow file access\r\n");
    wrStr("/BUZZ   - Enable error buzz             /NOBUZZ  - Disable error buzz\r\n");
    wrStr("/SS     - Turn space stripping on       /NOSS    - Turn space stripping off\r\n");
    wrStr("/USEG   - Edit graphics files           /TRYNOG  - Try no graph if necessary\r\n");
    wrStr("/STRIPG - Edit normal files (strip G graphics specification if necessary)\r\n\n");
    wrStr("/STARTATTR:a     - Start attribute      /STATATTR:a       - Status line\r\n");
    wrStr("/RIBATTR:a       - Code ribbon          /RIBHLATTR:a      - Ribbon highlight\r\n");
    wrStr("/HELPATTR:a      - Help screen          /HELPHLATTR:a     - Help highlight\r\n");
    wrStr("/DIALOGATTR:a    - Dialog screen        /DIALOGHLATTR:a   - Dialog highlight\r\n");
    wrStr("/DIALOGOPTATTR:a - Dialog cmd options   /DIALOGFILEATTR:a - Dialog File\r\n");
    wrStr("/DIALOGDIRATTR:a - Dialog drives        /DIALOGDIRATTR:a  - Dialog dirs\r\n");

    ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP: goto SCR3;
        case ESC: goto EXIT;
    }

SCR5:
    wrStr("\r\n/SET:f       - Load a set of characters into the function key table\r\n\n");
    wrStr("/MACRO:f     - Load a set of macro strings into the macro table\r\n\n");
    wrStr("/STATUS:t    - Select the default status bar type desired\r\n\n");
    wrStr("/LLEND:c;... - Specify a different terminating string for the last line\r\n");
    wrStr("               written during a save file operation (128 characters max)\r\n\n");
    wrStr("/ATMAC:v[:s] - Specify a different default value for @ macro v\r\n");
    wrStr("               (NOTE:  s is an optional parameter.  If omitted, the\r\n");
    wrStr("               original default value is used.  Otherwise, the string s\r\n");
    wrStr("               is used.)\r\n\n");
    wrStr("\"REM /...\"   - Remove a switch from processing without physically removing\r\n");
    wrStr("               it from the configuration file, environment variable or\r\n");
    wrStr("               command line.\r\n\n");

    ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP: goto SCR4;
        case ESC: goto EXIT;
    }

SCR6:
    int row, col;
    for (row = 0; row < 16; ++row)
    {
        for (col = 0; col < 8; ++col)
        {
            setXY(col*10+3,row+2);
            int ch = col*16+row;
            fastprintf("%3d %c",ch,
                (strchr("\x07\x08\x09\x0A\x0C\x0D\x1A\x1B",ch) == NULL) ?
                (char) ch : ' ');
        }
    }

    ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP: goto SCR5;
        case ESC: goto EXIT;
    }

SCR7:
    for (row = 0; row < 16; ++row)
    {
        for (col = 0; col < 8; ++col)
        {
            setXY(col*10+3,row+2);
            int ch = (col+8)*16+row;
            fastprintf("%3d %c",ch,
                (strchr("\x07\x08\x09\x0A\x0C\x0D\x1A\x1B",ch) == NULL) ?
                (char) ch : ' ');
        }
    }

    ch = getKey();
    cls();

    switch (ch)
    {
        case PGUP: goto SCR6;
        case ESC: goto EXIT;
    }

EXIT:
    setWin(1,1,80,MAX_ROWS);
    cls();
    dispCurs();
}

    /*--------------------------------------------------------------------*/

void pascal doDOS(char * cmd, int pause)
{
    if (nodosFlag) return;

    int ttl = tlOff, tsl = slOff, tlc = lcOff, tsc = scOff;
    int tmpTxtChanged = textChanged;

    char tmpName[PN_SIZE+1];
    strcpy(tmpName,lastFN);
    strcpy(lastFN,"PCBEDIT.$$$");
    if (exist(lastFN)) unlink(lastFN);
    doSaveFile();
    doClearMem();

    setAttr(origAttr);
    cls();
    int tmpCursType = cursType;
    setcurstype(NORMALCURS);

    char startDir[256+1];
    getcwd(startDir,256+1);

    static char newPrompt[256+1];
    strcpy(newPrompt,"PROMPT=Type 'EXIT' to return to PCBEdit$_");
    strcat(newPrompt,getenv("PROMPT") ? getenv("PROMPT") : "$P$G");
    newPrompt[127] = NUL;
    putenv(newPrompt);
    putenv("INPCBED=Y");

    if (cmd != NULL)
    {
        wrStr("Shelling to:  ");
        wrStr(cmd);
        wrStr("\r\n\n");
    }

    spawnl(P_WAIT, getenv("COMSPEC"), getenv("COMSPEC"),
        cmd ? "/C" : cmd, cmd, NULL);

    if ((cmd != NULL) && pause)
    {
        wrStr("\r\n\nPress a key to continue...");
        getKey();
    }

    setcurstype(tmpCursType);

    setdisk(*startDir-'A');
    chdir(startDir);
    dispScrn(TRUE);
    strcpy(lastFN,"PCBEDIT.$$$");
    doLoadFile();
    unlink(lastFN);
    strcpy(lastFN,tmpName);

    commitLine();
    tlOff = ttl, slOff = tsl, lcOff = tlc, scOff = tsc;
    textChanged = tmpTxtChanged;
    cacheLine();
    adjustCLC();
}

/******************************************************************************/

