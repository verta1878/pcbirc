/* Borland conio function declarations for Watcom */
#ifdef __WATCOMC__
extern "C" {
    void clreol(void);
    void gotoxy(int x, int y);
    char wherex(void);
    char wherey(void);
    int puttext(int l, int t, int r, int b, void *buf);
    int movetext(int l, int t, int r, int b, int dl, int dt);
    void _setcursortype(int type);
}
#endif
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
/*                                FAST_CIO.CPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                       Functions for fast console I/O                       */
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

#pragma hdrfile "file_cio.sym"

#include    <conio.h>
#include    <stdarg.h>
#include    <stdio.h>
#include    <string.h>

#include    <fast_cio.hpp>

#include    <keyboard.hpp>
#include    <miscedit.hpp>
#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Variables

int     NOCURS     = _NOCURSOR;
int     SOLIDCURS  = _SOLIDCURSOR;
int     NORMALCURS = _NORMALCURSOR;

#ifndef __OS2__

int     curAttr;

char    *curScrPtr;

int     winL = 1, winT = 1;

unsigned long   baseScrnAddr = 0xB8000000L;
int             snowWatch = FALSE;

#endif

int     cursType = NOCURS;

/******************************************************************************/

// Functions

int  getStr(char * buf, int size, int flags)
{
    int tmpCursType = cursType;
    setcurstype(SOLIDCURS);

    flags &= F_VALID;

    if (flags == 0) flags = F_ALL;

    // Initialize a local insert mode for this function
    int insMode = TRUE;

    // Save the column where the input buffer starts
    int x = curx();

    // Output the last filename and make sure the line after it is clear
    char * bufPtr = buf;
    if (strlen(bufPtr) > (80-x)) bufPtr = buf+strlen(buf)-(80-x);
    fastprintf("%-*.*s",80-x,80-x,bufPtr);
    cle();

    // Initialize temp pointer to the end of the last filename
    char * p = buf+strlen(buf);
    setXY(x+(int)(p-bufPtr),cury());

    // Keyboard loop
    int ch;
    while (((ch = getKey()) != ENTER) && (ch != ESC))
    {
        // Turn off cursor
        setcurstype(NOCURS);

        // Process the keypress
        if (ch == BS)
        {
            if (p > buf)
                strcpy(p-1,p),
                --p;
            if (p < bufPtr) --bufPtr;
        }
        else if (ch == DEL)
        {
            if (strlen(p) > 0) strcpy(p,p+1);
        }
        else if (ch == HOME)
        {
            bufPtr = p = buf;
        }
        else if (ch == END)
        {
            bufPtr = buf;
            if (strlen(bufPtr) > (80-x)) bufPtr = buf+strlen(buf)-(80-x);
            p = buf+strlen(buf);
        }
        else if (ch == CEND)
        {
            memset(p,0,strlen(p));
        }
        else if (ch == LEFT)
        {
            if (p > buf) --p;
            if (p < bufPtr) --bufPtr;
        }
        else if (ch == RIGHT)
        {
            if (strlen(p) > 0) ++p;
            if (p > bufPtr+(80-x)) ++bufPtr;
        }
        else if (ch == INS)
            insMode = !insMode;
        else if ( (ch < 256)                                                            &&
                 ((flags & (F_ALL))                                                     ||
                 ((flags & (F_ALPHA))       && isALPHA(ch))                             ||
                 ((flags & (F_NUM))         && isDIGIT(ch))                             ||
                 ((flags & (F_FILE|F_PATH)) && (strchr(".!@#$^&()~`-_{}'",ch) != NULL)) ||
                 ((flags & (F_PATH))        && (strchr("\\:",ch) != NULL))              ||
                 ((flags & (F_WILD))        && (strchr("*?",ch) != NULL))))
        {
            if ((flags & F_LOWER) && (ch < 256))
                ch = toLOWER(ch);
            else if ((flags & F_UPPER) && (ch < 256))
                ch = toUPPER(ch);
            if (insMode && (strlen(buf) < size))
            {
                memmove(p+1,p,strlen(p)+1);
                *(p++) = char(ch);
            }
            else if (!insMode && (p-buf < size))
            {
                if (strlen(p) == 0) *(p+1) = 0;
                *(p++) = char(ch);
            }
            if (p > bufPtr+(80-x)) ++bufPtr;
        }
        else
            buzz();

        // Return to the beginning of the input buffer and redisplay
        setXY(x,cury());
        fastprintf("%-*.*s",80-x,80-x,bufPtr);

        // Return to the current cursor position
        setXY(x+(int)(p-bufPtr),cury());

        // Turn the cursor back on
        setcurstype(insMode ? SOLIDCURS : NORMALCURS);
    }

    setcurstype(tmpCursType);

    // if normal exit return TRUE else return FALSE
    return (ch == ENTER);
}

    /*------------------------------------------------------------------*/

void  setAttr(int attr)
{
   #ifndef __OS2__
    if (snowWatch)
   #endif
        textattr(attr);
   #ifndef __OS2__
    else
        curAttr = attr;
   #endif
}

    /*------------------------------------------------------------------*/

void  cls(void)
{
    textattr(0x07);
    clrscr();
   #ifndef __OS2__
    textattr(curAttr);
   #endif
    setXY(1,1);
}

    /*------------------------------------------------------------------*/

void  cle(void)
{
   #ifndef __OS2__
    if (!snowWatch)
        textattr(curAttr);
   #endif
    clreol();
}

    /*------------------------------------------------------------------*/

int  wrChar(int ch)
{
   #ifndef __OS2__
    if (snowWatch)
   #endif
        return putch(ch);
   #ifndef __OS2__
    else
    {
        int x = wherex()+1, y = wherey();
        while (x > MAX_COLS)
            x -= MAX_COLS,
            ++y;

        switch (ch)
        {
            case ENTER:
                x = 1;
                curScrPtr = (char *) (baseScrnAddr + (y-1)*160);
                break;

            case LF:
                if (++y > MAX_ROWS) movetxt(2,MAX_ROWS,1), --y;
                curScrPtr = (char *) (baseScrnAddr + (y-1)*160 + (x-1)*2);
                break;

            default:
                *(curScrPtr++) = char(ch);
                *(curScrPtr++) = char(curAttr);
                break;
        }

        gotoxy(x,y);

        return ch;
    }
   #endif
}

    /*------------------------------------------------------------------*/

int  wrStr(char *s)
{
   #ifndef __OS2__
    if (snowWatch)
   #endif
        return cputs(s);
   #ifndef __OS2__
    else
    {
        int x = wherex(), y = wherey();

        int i;
        while (*s)
        {
            i = *(s++);
            switch (i)
            {
                case ENTER:
                    x = 1;
                    setXY(x,y);
                    break;

                case LF:
                    if (++y > MAX_ROWS) movetxt(2,MAX_ROWS,1), --y;
                    setXY(x,y);
                    break;

                default:
                    *(curScrPtr++) = char(i);
                    *(curScrPtr++) = char(curAttr);
                    if (++x > MAX_COLS)
                        x -= MAX_COLS,
                        ++y;
                    break;
            }
        }

        gotoxy(x,y);

        return i;
    }
   #endif
}

    /*------------------------------------------------------------------*/

int fastprintf(const char *f, ...)
{
    int retVal;

    va_list va;
    va_start(va,f);

    char buf[256+1];
    retVal = vsprintf(buf,f,va);
    wrStr(buf);

    va_end(va);

    return retVal;
}

    /*------------------------------------------------------------------*/

void  setXY(int x, int y)
{
//  gotoxy((winL-1)+x,(winT-1)+y);
    gotoxy(x,y);
   #ifndef __OS2__
    curScrPtr = ((char *) baseScrnAddr)+(((winT-1)+y-1)*160)+
        (((winL-1)+x-1)*2);
   #endif
}

    /*------------------------------------------------------------------*/

void  setcurstype(int type)
{
    cursType = type;
    _setcursortype(type);
}

    /*------------------------------------------------------------------*/

void  putline(int line, char * buf)
{
   #ifndef __OS2__
    if (snowWatch)
   #endif
        puttext(1,line,80,line,buf);
   #ifndef __OS2__
    else
       #ifdef _BORLANDC_
         #pragma option -Oi
       #endif
       memcpy((void*)(baseScrnAddr+((line-1)*160)),buf,160);
       #ifdef _BORLANDC_
         #pragma option -O-i
       #endif
   #endif
}

    /*------------------------------------------------------------------*/

int  movetxt(int t, int b, int dt)
{
   #ifndef __OS2__
//  if (snowWatch)
   #endif
        return movetext(1,t,80,b,1,dt);
   #ifndef __OS2__
//  else
//  {
//      memmove((void*)(baseScrnAddr+((dt-1)*160)),
//          (void*)(baseScrnAddr+((t-1)*160)),((b-t+1)*160));
//      return 1;
//  }
   #endif
}

    /*------------------------------------------------------------------*/

int  puttxt(void * c)
{
   #ifndef __OS2__
    if (snowWatch)
   #endif
        return puttext(1,1,80,25,c);
   #ifndef __OS2__
    else
    {
        memcpy((void*)baseScrnAddr,c,4000);
        return 1;
    }
   #endif
}

    /*------------------------------------------------------------------*/

void  setWin(int l, int t, int r, int b)
{
    window(l,t,r,b);
   #ifndef __OS2__
    winL = l;
    winT = t;
   #endif
    setXY(1,1);
}

    /*------------------------------------------------------------------*/

int  curx(void)
{
    return wherex();
}

    /*------------------------------------------------------------------*/

int  cury(void)
{
    return wherey();
}

/******************************************************************************/

