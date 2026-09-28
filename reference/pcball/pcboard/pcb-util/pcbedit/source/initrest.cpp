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
/*                                INITREST.CPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*        Functions used during program initialization and restoration        */
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

#pragma hdrfile "initrest.sym"

#include    <conio.h>
#include    <dos.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    <dosfunc.h>

#include    <initrest.hpp>

#include    <atcodes.hpp>
#include    <compat.hpp>
#include    <fast_cio.hpp>
#include    <file.hpp>
#include    <keyboard.hpp>
#include    <miscedit.hpp>
#include    <pcbedit.hpp>

#include    "wel_scrn.h"

#pragma hdrstop

/******************************************************************************/

// Defined Macros

#define IN              TRUE
#define OUT             FALSE

/******************************************************************************/

// Types

/******************************************************************************/

// Variables

int             origAttr, startAttr = 0x07;

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Functions

void pascal procArg(char * a)
{
    if (stricmp("/SNOW",a) == 0)
       #ifndef __OS2__
        snowWatch = TRUE;
       #else
        ; // not used by OS/2
       #endif
    else if (stricmp("/NOSNOW",a) == 0)
       #ifndef __OS2__
        snowWatch = FALSE;
       #else
        ; // not used by OS/2
       #endif
    else if (stricmp("/QUICK",a) == 0)
        quick = TRUE;
    else if (stricmp("/NOQUICK",a) == 0)
        quick = FALSE;
    else if (stricmp("/AT",a) == 0)
        noatFlag = FALSE;
    else if (stricmp("/NOAT",a) == 0)
        noatFlag = TRUE;
    else if (stricmp("/ATX",a) == 0)
        noatxFlag = FALSE;
    else if (stricmp("/NOATX",a) == 0)
        noatxFlag = TRUE;
    else if (stricmp("/DOS",a) == 0)
        nodosFlag = FALSE;
    else if (stricmp("/NODOS",a) == 0)
        nodosFlag = TRUE;
    else if (stricmp("/BAK",a) == 0)
        bakFlag = TRUE;
    else if (stricmp("/NOBAK",a) == 0)
        bakFlag = FALSE;
    else if (stricmp("/IO",a) == 0)
        ioFlag = TRUE;
    else if (stricmp("/NOIO",a) == 0)
        ioFlag = FALSE;
    else if (stricmp("/BUZZ",a) == 0)
        buzzFlag = TRUE;
    else if (stricmp("/NOBUZZ",a) == 0)
        buzzFlag = FALSE;
    else if (stricmp("/SS",a) == 0)
        nossFlag = FALSE;
    else if (stricmp("/NOSS",a) == 0)
        nossFlag = TRUE;
    else if (stricmp("/USEG",a) == 0)
        gFlag = 0;
    else if (stricmp("/TRYNOG",a) == 0)
        gFlag = 1;
    else if (stricmp("/STRIPG",a) == 0)
        gFlag = 2;
    else if (memicmp("/ATMAC:",a,7) == 0)
    {
        char *t = a+7;
        while ((*t != ':') && (*t != NUL))
            *t = char(toUPPER(*t)),
            ++t;
        int code = doIsATCode(a+7);
        if (replaceableATCode[code])
        {
            char * newStr = strchr(a+7,':');
            delete atVarValLst[code];
            atVarValLst[code] = strdup((newStr++ != NULL) ? newStr :
                defAtVarValLst[code]);
        }
    }
    else if (memicmp("/STATUS:",a,8) == 0)
    {
        switch (atoi(a+8))
        {
            case 0:
                statOn = TRUE;
                statType = 0;
                break;
            case 1:
                statOn = TRUE;
                statType = 1;
                break;
            case 2:
                statOn = FALSE;
                statType = 2;
                break;
        }
    }
    else if (stricmp("/COLOR",a) == 0)
    {
        textmode(C80);
       #ifndef __OS2__
        baseScrnAddr = 0xB8000000L;
       #endif
        statAttr = 0x4F;
        ribAttr = 0x17;
        ribHLAttr = 0x2F;
        helpAttr = 0x07;
        helpHLAttr = 0x70;
        dialogAttr = 0x07;
        dialogHLAttr = 0x70;
        dialogFileAttr = 0x0F;
        dialogDrvAttr = 0x0C;
        dialogDirAttr = 0x09;
        dialogOptAttr  = 0x0A;
    }
    else if (stricmp("/MONO",a) == 0)
    {
        textmode(MONO);
       #ifndef __OS2__
        baseScrnAddr = 0xB0000000L;
       #endif
        statAttr = 0x09;
        ribAttr = 0x07;
        ribHLAttr = 0x0F;
        helpAttr = 0x07;
        helpHLAttr = 0x70;
        dialogAttr = 0x07;
        dialogHLAttr = 0x70;
        dialogFileAttr = 0x0F;
        dialogDrvAttr = 0x07;
        dialogDirAttr = 0x07;
        dialogOptAttr  = 0x07;
    }
    else if (stricmp("/CATTR",a) == 0)
    {
        statAttr = 0x4F;
        ribAttr = 0x17;
        ribHLAttr = 0x2F;
        helpAttr = 0x07;
        helpHLAttr = 0x70;
        dialogAttr = 0x07;
        dialogHLAttr = 0x70;
        dialogFileAttr = 0x0F;
        dialogDrvAttr = 0x0C;
        dialogDirAttr = 0x09;
        dialogOptAttr  = 0x0A;
    }
    else if (stricmp("/MATTR",a) == 0)
    {
        statAttr = 0x09;
        ribAttr = 0x07;
        ribHLAttr = 0x0F;
        helpAttr = 0x07;
        helpHLAttr = 0x70;
        dialogAttr = 0x07;
        dialogHLAttr = 0x70;
        dialogFileAttr = 0x0F;
        dialogDrvAttr = 0x07;
        dialogDirAttr = 0x07;
        dialogOptAttr  = 0x07;
    }
    else if (memicmp("/STARTATTR:",a,11) == 0)
        startAttr = atoi(a+11);
    else if (memicmp("/STATATTR:",a,10) == 0)
        statAttr = atoi(a+10);
    else if (memicmp("/RIBATTR:",a,9) == 0)
        ribAttr = atoi(a+9);
    else if (memicmp("/RIBHLATTR:",a,11) == 0)
        ribHLAttr = atoi(a+11);
    else if (memicmp("/HELPATTR:",a,10) == 0)
        helpAttr = atoi(a+10);
    else if (memicmp("/HELPHLATTR:",a,12) == 0)
        helpHLAttr = atoi(a+10);
    else if (memicmp("/DIALOGATTR:",a,12) == 0)
        dialogAttr = atoi(a+10);
    else if (memicmp("/DIALOGHLATTR:",a,14) == 0)
        dialogHLAttr = atoi(a+10);
    else if (memicmp("/DIALOGFILEATTR:",a,16) == 0)
        dialogFileAttr = atoi(a+10);
    else if (memicmp("/DIALOGDRVATTR:",a,15) == 0)
        dialogDrvAttr = atoi(a+10);
    else if (memicmp("/DIALOGDIRATTR:",a,15) == 0)
        dialogDirAttr = atoi(a+10);
    else if (memicmp("/DIALOGOPTATTR:",a,15) == 0)
        dialogOptAttr = atoi(a+10);
    else if (memicmp("/MACRO:",a,7) == 0)
    {
        char * p = a+7;
        loadMacros(p);
    }
    else if (memicmp("/SET:",a,5) == 0)
    {
        char * p = a+5;
        loadFKeyChar(p);
    }
    else if (memicmp("/LLEND:",a,7) == 0)
    {
        char *p = a+7, buf[128+1];
        memset(buf,0,128+1);
        int i = 0;
        while ((*p != 0) && (i <= 128))
        {
            int ch;
            sscanf(p,"%d;",&ch);
            p = strchr(p,';');
            if (p++ == NULL) p = "";
            buf[i++] = (char) ch;
        }
        if (lastLineEnd != NULL) delete lastLineEnd;
        lastLineEnd = new char[strlen(buf)+1];
        strcpy(lastLineEnd,buf);
    }
    else if (memicmp("REM /",a,5) != 0)
    {
        strcpy(lastFN,a);
        strupr(lastFN);
        char * p = lastFN;
        while (*p)
        {
            if (!isALNUM(*p) && (strchr("\\.:!@#$^&()~`-_{}'",*p) == NULL))
                strcpy(p,p+1);
            else
                ++p;
        }
    }
}

    /*--------------------------------------------------------------------*/

char * pascal getparm(char *s)
{
    static char *tmps = NULL;

    if (s == NULL) s = tmps;

    if (s == NULL) return s;

    char *rv = s;

    if (*rv == '\"') ++rv;

    tmps = strchr(rv,(rv != s) ? '\"' : ' ');

    if (tmps != NULL)
    {
        *tmps = NUL;
        ++tmps;
    }

    return rv;
}

    /*--------------------------------------------------------------------*/

void pascal procStr(char * s)
{
    // Make a local copy of the string
    char * args = new char[strlen(s)+1];
    if (args == NULL) return;
    strcpy(args,s);

    // Grab the tokens from the string copy and process them
    char *arg = getparm(args);
    while (arg != NULL)
    {
        procArg(arg);
        arg = getparm(NULL);
    }

    // Deallocation the memory used for string copy
    delete args;
}

    /*--------------------------------------------------------------------*/

void pascal procCfgFile(char * s)
{
    char tmpBuf[128+1+2];

    DOSFILE fs;
    if (dosfopen(s,OPEN_READ|OPEN_DENYWRIT,&fs) == -1) return;

    while (dosfgets(tmpBuf,128+1+2,&fs) != -1) procStr(tmpBuf);

    if (dosfclose(&fs) == -1) fileErr("File close error",s);
}

    /*--------------------------------------------------------------------*/

void pascal procCfgFiles(void)
{
    if (_argv[0] != NULL)
    {
        char cfgFile [ PN_SIZE + 1 ];
        strcpy(cfgFile,_argv[0]);

        char * tmpPtr = strrchr(cfgFile,'\\');
        if (tmpPtr)
            ++tmpPtr;
        else
            tmpPtr = cfgFile;

        strcpy(tmpPtr,"PCBEDIT.CFG");
        procCfgFile(cfgFile);
    }

    procCfgFile("PCBEDIT.CFG");
}

    /*--------------------------------------------------------------------*/

void pascal fadeScreen(int *fgAttr, int *bgAttr, int delLen, int initIndex,
    int abortIndex, int stepIndex)
{
    // For each attribute pair...
    for (int i = initIndex; i != abortIndex; i += stepIndex)
    {
        // Change each attribute in screen information
        for (unsigned char *p = WEL_SCRN+1; p < WEL_SCRN+WEL_SCRN_LENGTH;
            p += 2)
        {
            if (*p == fgAttr[i-stepIndex])
            {
                *p = char(fgAttr[i]);
                continue;
            }
            if (*p == bgAttr[i-stepIndex]) *p = char(bgAttr[i]);
        }

        // Display the screen and wait
        puttxt(WEL_SCRN);
        doDelay(delLen);
    }
}

    /*--------------------------------------------------------------------*/

void pascal welcomeScreen(int in)
{
    // Variables used to fade screen in and out
//  int numAttrs = 8;
//  int fgAttrs[] = { 8, 8, 1, 9, 3, 7, 11, 15 };
//  int bgAttrs[] = { 0, 0, 8, 1, 9, 3,  7, 11 };

//  int numAttrs = 6;
//  int fgAttrs[] = { 8, 8, 1, 9, 7, 15 };
//  int bgAttrs[] = { 0, 0, 8, 1, 9,  7 };

    int fgColor[] = { 8, 1, 9, 7, 15 };
    int bgColor[] = { 0, 0, 1, 9,  7 };
   #ifndef __OS2__
    int fgMono[]  = { 8, 8, 7, 15 };
    int bgMono[]  = { 0, 0, 9,  7 };
   #endif

    int numAttrs, *fgAttrs, *bgAttrs;

   #ifndef __OS2__
    if (baseScrnAddr == 0xB0000000L)
    {
        numAttrs = 4;
        fgAttrs = fgMono;
        bgAttrs = bgMono;
    }
    else
   #endif
    {
        numAttrs = 5;
        fgAttrs = fgColor;
        bgAttrs = bgColor;
    }

    int welScrnDelay = 100;

    if (in)                             // Fade screen in routine
    {
        cls();
        doDelay(welScrnDelay);
        fadeScreen(fgAttrs,bgAttrs,welScrnDelay,1,numAttrs,1);
    }
    else                                // Fade screen out routine
    {
        fadeScreen(fgAttrs,bgAttrs,welScrnDelay,(numAttrs-1)-1,0,-1);
        cls();
    }

    setAttr(fgAttrs[numAttrs-1]);
}

    /*--------------------------------------------------------------------*/

void pascal dispWelcome(void)
{
    llcOff = -1;

    setcurstype(NOCURS);

    setAttr(origAttr);
    cls();

    // Fade in the welcome screen
    welcomeScreen(IN);

    // Auto center version & alpha/beta release date
    char * betaStamp =
       #ifdef __MSDOS__
        "DOS Version"
       #else
        "OS/2 Version"
       #endif
       #if defined(__IS_ALPHA__) || defined(__IS_BETA__)
        " -- "
        #if defined(__IS_ALPHA__)
         "Alpha"
        #elif defined(__IS_BETA__)
         "Beta"
        #endif
        " Release -- Compiled " __DATE__ " @ " __TIME__
       #endif
        ;
    setXY((80-strlen(betaStamp))/2+1,11);
    wrStr(betaStamp);

    // Display prompt and wait for a keypress
    setXY(27,25);
    wrStr("Press any key to continue...");
    getKey();

    // Fade out the welcome screen
    welcomeScreen(OUT);
}

    /*--------------------------------------------------------------------*/

#ifndef __OS2__
void interrupt (* orig0x24) (...);
extern "C" void interrupt CEH (...);
#endif

void pascal initPrg(void)
{
   int i;

   #ifndef __OS2__
    orig0x24 = getvect(0x24);
    setvect(0x24,CEH);
   #endif

    // Save original attribute
    struct text_info ti;
    gettextinfo(&ti);
    origAttr = ti.attribute;

    // If monochrome mode, set baseScrnAddr appropriately
   #ifndef __OS2__
    if (ti.currmode == MONO) baseScrnAddr = 0xB0000000L;
   #endif

    // Get the number of rows on the screen
    MAX_ROWS = ti.screenheight;
//  MAX_COLS = ti.screenwidth;

    // Turn off cursor
    setcurstype(NOCURS);

    // Initialize lists
    memset(lineList,0,sizeof(char *)*MAX_LINES);
    memset(last00List,-1,sizeof(int)*MAX_LINES);
    memset(isFFList,0,sizeof(char)*MAX_LINES);
    memset(lastCLSList,-1,sizeof(int)*MAX_LINES);
    memset(macros,0,sizeof(macros));

    // Other initializations
    memset(curLine,0,MAX_LINE_SIZE+1);
    strcpy(lastFN,"");
    lastLineEnd = NULL;

    for (i = 0; i < ATXCODE; ++i)
        atVarValLst[i] = strdup(defAtVarValLst[i]);

    // Read in and process cfg file(s), env var and command line parms
    procCfgFiles();
    if (getenv("PCBEDIT") != NULL) procStr(getenv("PCBEDIT"));
    for (i = 1; i < _argc; ++i) procArg(_argv[i]);

   #ifndef __OS2__
    curAttr = 0x07;
    curScrPtr = (char *) baseScrnAddr;
   #endif

    // Set each line initial attribute to startAttr
    int * alPtr = attrList;
    for (i = 0; i < MAX_LINES; ++i) *(alPtr++) = startAttr;

    // Load the start file if necessary
    if (strlen(lastFN) > 0) doLoadFile();

    if (!quick) dispWelcome();

    cls();
}

    /*--------------------------------------------------------------------*/

void pascal restPrg(void)
{
    int i;

    // Deallocate memory and null out all pointers in lineList
    for (i = 0; i < MAX_LINES; ++i)
        if (lineList[i] != NULL)
        {
            delete lineList[i];
            lineList[i] = NULL;
        }

    // Deallocate memory and null out all pointers in atVarValLst
    for (i = 0; i < ATXCODE; ++i)
        if (atVarValLst[i])
        {
            free(atVarValLst[i]);
            atVarValLst[i] = NULL;
        }

    // Deallocate memory and null out all pointers in macros
    for (i = 0; i < 15; ++i)
        for (int j = 0; j < 10; ++j)
            if (macros[i][j])
            {
                free(macros[i][j]);
                macros[i][j] = NULL;
            }

    // Deallocate memory used used by the lastLineEnd pointer
    if (lastLineEnd != NULL) delete lastLineEnd;

    if (exist("PCBEDIT.BLK")) unlink("PCBEDIT.BLK");
    if (exist("PCBEDIT.$$$")) unlink("PCBEDIT.$$$");

    // Restore original attribute and clear the screen
    setAttr(origAttr);
    cls();
    setcurstype(NORMALCURS);

    // deinstall ceh
   #ifndef __OS2__
    setvect(0x24,orig0x24);
   #endif
}

/******************************************************************************/

