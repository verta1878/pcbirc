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
/*                                  FILE.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*           Functions for file I/O and clearing of the text buffer           */
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

#pragma hdrfile "file.sym"

#include    <direct.h>
#include    <dos.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#include    <dosfunc.h>

#include    <file.hpp>

#include    <disp.hpp>
#include    <fast_cio.hpp>
#include    <keyboard.hpp>
#include    <miscedit.hpp>
#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Defined Macros

/******************************************************************************/

// Types

/******************************************************************************/

// Variables

int  dialogAttr    = 0x07, dialogHLAttr  = 0x70, dialogFileAttr = 0x0F,
     dialogDrvAttr = 0x0C, dialogDirAttr = 0x09, dialogOptAttr  = 0x0A;

char bakFlag = TRUE;
char ioFlag = TRUE;

char gFlag = 0;

extern int startAttr;

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Functions

int pascal getFN(char * p, int f)
{
    setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
    setAttr(statAttr);
    wrStr("Enter filename to ");
    wrStr(p);
    wrStr(":  ");

    char curFN[PN_SIZE+1];
    strcpy(curFN,lastFN);

    if (getStr(curFN,PN_SIZE,F_ALPHA|F_NUM|F_PATH|F_FILE|F_UPPER))
    {
        if (strlen(curFN) == 0)
        {
            return FALSE;
        }
        else if (!f || (f && exist(curFN)))
        {
            strcpy(lastFN,curFN);
            return TRUE;
        }
        else
        {
            fileErr("File not found",curFN);
        }
    }

    return FALSE;
}

    /*--------------------------------------------------------------------*/

int fnstrcmp(const void * l, const void * r)
{
    return strcmp(*(char **)l,*(char **)r);
}

    /*--------------------------------------------------------------------*/

#define MAX_LIST (6+256)

int pascal fileSelDialog(char * h)
{
    char *list[MAX_LIST];
    int   elems = 0;
    int   i;

    for (i = 0; i < MAX_LIST; ++i)
    {
        list[i] = new char[14+1];
        if (list[i] == NULL)
        {
            editErr("Insufficient memory for file list");
            for (int j = 0; j < i; ++j) delete list[i];
            return getFN(h,TRUE);
        }
    }

    llcOff = -1;

    int oldStat = statOn;
    statOn = FALSE;

    setcurstype(NOCURS);

    setAttr(dialogAttr);
    cls();
    setXY(31,1);
    wrChar(toUPPER(*h));
    wrStr(h+1);
    wrStr(" File Selection");

    char curDir[PN_SIZE+1];
    strcpy(curDir,lastFN);

    char * ptr = curDir;
    if (ptr[1] == ':') ptr += 2;
    while (strchr(ptr,'\\') != NULL) ptr = strchr(ptr,'\\')+1;
    if (strlen(ptr) > 12) ptr[12] = NUL;

    char curFile[12+1];
    strcpy(curFile,ptr);
    *ptr = NUL;

    setAttr(dialogAttr);
    setXY(17,24);
        wrStr("\x1B"" & ""\x1A"" & ""\x18"" & ""\x19"" to highlight "
        "file/dir/drive/option");

    char curMask[12+1];
    strcpy(curMask,"*.*");

READDIR:
    if (strlen(curDir) == 0)
    {
        getcwd(curDir,PN_SIZE+1);
        if (strlen(curDir) > 3) strcat(curDir,"\\");
    }

    if (curDir[strlen(curDir)-1] != '\\') strcat(curDir,"\\");

    elems = 0;

    strcpy(list[elems++],"{PROMPT}");
    strcpy(list[elems++],"{RELATED}");
    strcpy(list[elems++],"{BASE}");
    strcpy(list[elems++],"{DIRECTORY}");
    strcpy(list[elems++],"{MASK}");
    strcpy(list[elems++],"{FILE}");

    char curPN[PN_SIZE+1];
    strcpy(curPN,curDir);
    strcat(curPN,curMask);

    ffblk curFFB;
    int done = findfirst(curPN,&curFFB,0);
    while (!done && (elems < MAX_LIST))
    {
        strcpy(list[elems++],curFFB.ff_name);
        done = findnext(&curFFB);
    }

    qsort(&list[6],elems-6,sizeof(char *),fnstrcmp);

    if (elems < MAX_LIST) strcpy(list[elems++],"..\\");

    strcpy(curPN,curDir);
    strcat(curPN,"*.*");

    done = findfirst(curPN,&curFFB,FA_DIREC);
    while (!done && (elems < MAX_LIST))
    {
        if ((curFFB.ff_attrib & FA_DIREC) && (curFFB.ff_name[0] != '.'))
        {
            strcpy(list[elems],curFFB.ff_name);
            strcat(list[elems++],"\\");
        }
        done = findnext(&curFFB);
    }

    int tmpDrv = getdisk();
    for (i = 'A'; (i <= 'Z') && (elems < MAX_LIST); ++i)
    {
        setdisk(i-'A');
        if (getdisk() != i-'A') continue;
        list[elems][0] = char(i);
        list[elems][1] = ':';
        list[elems++][2] = NUL;
    }
    setdisk(tmpDrv);

REDISPLAY:
    setAttr(dialogAttr);
    setXY(1,25);
    wrStr("     Enter to select file/dir/drive/option    "
        "Esc to exit without selecting     ");

    setXY(1,3);
    wrStr("Current Directory is ");
    setAttr(dialogHLAttr);
    fastprintf("%-59.59s",(strlen(curDir) > 0) ? curDir : ".\\");

    setAttr(dialogAttr);
    setXY(1,4);
    wrStr("     Current Mask is ");
    setAttr(dialogHLAttr);
    fastprintf("%-12.12s",curMask);

    setAttr(dialogAttr);
    setXY(1,5);
    wrStr("     Current File is ");
    setAttr(dialogHLAttr);
    fastprintf("%-12.12s",curFile);

    int tlOpt = 0;
    int elem  = 0;

    int exitFlag = FALSE;
    int rv = FALSE;
    int bigChange = TRUE;
    do {

        bigChange = bigChange                  ||
                    (elem == tlOpt)            ||
                    (elem == (tlOpt+(5*16)-1));

        int start = (bigChange ? tlOpt : elem-1);
        int end   = (bigChange ? tlOpt+(5*16)-1 : elem+1);

        bigChange = FALSE;

        for (int i = start; i <= end; ++i)
        {
            char buf[16+1];
            int  tmp;

            if (i < elems)
                ptr = list[i],
                tmp = dialogFileAttr;
            else
                ptr = "",
                tmp = dialogAttr;

            memset(buf,' ',16);
            buf[16] = NUL;
            memcpy(buf+(16-strlen(ptr))/2,ptr,strlen(ptr));

            if (*ptr == '{')
                tmp = dialogOptAttr;
            else if (*(ptr+1) == ':')
                tmp = dialogDrvAttr;
            else if ((tmp != dialogAttr) && (*(ptr+strlen(ptr)-1) == '\\'))
                tmp = dialogDirAttr;

            setAttr((i == elem) ? dialogHLAttr : tmp);
            setXY((i%5)*16+1,(i-tlOpt)/5+7);
            wrStr(buf);
        }

//        get & process keypress
//            LEFT RIGHT UP DOWN HOME END PGUP PGDN ESC ENTER SPACE

        int key = getKey();

        switch (key)
        {
            case LEFT:
                if (--elem < 0) elem = 0;
                if (elem < tlOpt) tlOpt -= 5, bigChange = TRUE;
                break;

            case RIGHT:
                if (++elem >= elems) elem = elems-1;
                if (elem >= tlOpt+(5*16)) tlOpt += 5, bigChange = TRUE;
                break;

            case UP:
                bigChange = TRUE;
                elem -= 5;
                if (elem < 0) elem = 0;
                if (elem < tlOpt) tlOpt -= 5;
                break;

            case DOWN:
                bigChange = TRUE;
                elem += 5;
                if (elem >= elems) elem = elems-1;
                if (elem >= tlOpt+(5*16)) tlOpt += 5;
                break;

            case PGUP:
                bigChange = TRUE;
                elem -= (5*16);
                if (elem < 0) elem = 0;
                if (elem < tlOpt) tlOpt = (elem/5)*5;
                break;

            case PGDN:
                bigChange = TRUE;
                elem += (5*16);
                if (elem >= elems) elem = elems-1;
                if (elem >= tlOpt+(5*16)) tlOpt = (elem/5)*5-(5*(16-1));
                break;

            case ESC:
                exitFlag = TRUE;
                break;

            case ENTER:
            {
                switch (elem)
                {
                    case 0:
                        if (getFN(h,TRUE)) rv = exitFlag = TRUE;
                        if (!exitFlag) goto REDISPLAY;
                        break;

                    case 2:
                        if (strchr(curFile,'.') != NULL)
                            *strchr(curFile,'.') = NUL;

                        ptr = curFile+strlen(curFile);
                        if (strlen(curFile) > 0)
                        {
                            if (*(--ptr) == 'G')
                                *ptr = NUL;
                            else
                                ++ptr;
                        }

                        while ((strlen(curFile) > 0) && isDIGIT(*(--ptr)))
                            *ptr = NUL;

                    case 1:
                        strcpy(curMask,curFile);
                        if (strlen(curMask) < 12) strcat(curMask,"*");
                        if (strchr(curMask,'.') == NULL)
                        {
                            if (strlen(curMask) < 12) strcat(curMask,".");
                            if (strlen(curMask) < 12) strcat(curMask,"*");
                        }
                        goto READDIR;

                    case 3:
                    {
                        char tmpDir[PN_SIZE+1];
                        strcpy(tmpDir,curDir);
                        setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
                        setAttr(statAttr);
                        wrStr("Enter path:  ");
                        if (getStr(tmpDir,PN_SIZE,F_ALPHA|F_NUM|F_PATH|F_FILE|
                            F_UPPER))
                        {
                            strcpy(curDir,tmpDir);
                            goto READDIR;
                        }
                        goto REDISPLAY;
                    }

                    case 4:
                    {
                        char tmpMask[12+1];
                        strcpy(tmpMask,curMask);
                        setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
                        setAttr(statAttr);
                        wrStr("Enter mask:  ");
                        if (getStr(tmpMask,12,F_ALPHA|F_NUM|F_FILE|F_UPPER|
                            F_WILD))
                        {
                            strcpy(curMask,tmpMask);
                            goto READDIR;
                        }
                        goto REDISPLAY;
                    }

                    case 5:
                    {
                        char tmpFile[12+1];
                        strcpy(tmpFile,curFile);
                        setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
                        setAttr(statAttr);
                        wrStr("Enter file name:  ");
                        if (getStr(tmpFile,12,F_ALPHA|F_NUM|F_FILE|F_UPPER|
                            F_WILD))
                        {
                            strcpy(curFile,tmpFile);
                            goto READDIR;
                        }
                        goto REDISPLAY;
                    }
                }
                if (elem < 6) break;

                if (list[elem][1] == ':')
                {
//                    strcpy(curFile,"");
                    if (_getdcwd((*list[elem])-'A'+1,curDir,PN_SIZE+1) != NULL)
                    {
                        if (strlen(curDir) > 3) strcat(curDir,"\\");
                        goto READDIR;
                    }
                    else
                    {
                        editErr("Error changing default drive");
                        goto REDISPLAY;
                    }
                }

                ptr = list[elem];
                ptr += strlen(ptr)-1;
                if (*ptr == '\\')
                {
//                    strcpy(curFile,"");
                    if (strcmp(list[elem],"..\\") == 0)
                    {
                        if (strlen(curDir) > 3)
                            *(curDir+strlen(curDir)-1) = NUL;
                        ptr = curDir;
                        while (strchr(ptr,'\\') != NULL)
                            ptr = strchr(ptr,'\\')+1;
                        *ptr = NUL;
                    }
                    else
                    {
                        strcat(curDir,list[elem]);
                    }
                    goto READDIR;
                }

                // is it a file
                strcpy(lastFN,curDir);
                strcat(lastFN,list[elem]);
                rv = exitFlag = TRUE;
                break;
            }
        }

    } while (!exitFlag);

    for (i = 0; i < MAX_LIST; ++i) delete list[i];

    statOn = oldStat;

    return rv;
}

    /*--------------------------------------------------------------------*/

void pascal doClearMem(void)
{
    int i;
    // Deallocate used memory and set line pointers to NULL
    for (i = 0; i < MAX_LINES; ++i)
        if (lineList[i] != NULL)
        {
            delete lineList[i];
            lineList[i] = NULL;
        }

    // Initialize lists
    memset(lineList,0,sizeof(char *)*MAX_LINES);
    memset(last00List,-1,sizeof(int)*MAX_LINES);
    memset(isFFList,0,sizeof(char)*MAX_LINES);
    memset(lastCLSList,-1,sizeof(int)*MAX_LINES);

    // Set each line initial attribute to start attribute
    int * alPtr = attrList;
    for (i = 0; i < MAX_LINES; ++i) *(alPtr++) = startAttr;

    // Re-initialize necessary variables
    memset(curLine,0,MAX_LINE_SIZE+1);
    curLineChar = curLine;
    tlOff = lcOff = slOff = scOff = 0;
    ltlOff = llcOff = -11;
    disableEditKeys = FALSE;
    startBlockLine = endBlockLine = -1;
    startBlockOff  = endBlockOff  = 0;
    textChanged = FALSE;
}

    /*--------------------------------------------------------------------*/

void pascal clearMem(void)
{
    // Display a prompt on the status line
    if (confirm("Do you really wish to clear out the current file?  "))
    {
        strcpy(lastFN,"");
        doClearMem();
    }
}

    /*--------------------------------------------------------------------*/

void pascal fileErr(char * s, char * f)
{
    // Error display info
    int    ld  = FALSE;
    char * ptr = f;
    if (strlen(ptr) > 40)
    {
        ld  = TRUE;
        ptr = ptr+strlen(ptr)-37;
    }

    editErr("%s (%s%s)",s,ld ? "..." : "",ptr);
}

    /*--------------------------------------------------------------------*/

//void pascal replace(char * s, char f, char r)
//{
//    while (*s != 0)
//    {
//        if (*s == f) *s = r;
//        if (*s == 0) break;
//        ++s;
//    }
//}

    /*--------------------------------------------------------------------*/

void pascal doLoadFile(void)
{
    // Strip trailing . if it exists
    char * lfnEnd = lastFN+strlen(lastFN)-1;
    if (*lfnEnd == '.') *lfnEnd = NUL;

    // If no filename, exit
    if (strlen(lastFN) == 0) return;

    if (((gFlag == 1) && !exist(lastFN)) || (gFlag == 2))
    {
        char * p = lastFN;
        while (strchr(p,'\\') != NULL) p = strchr(p,'\\')+1;
        if (strchr(p,'.') != NULL)
            p = strchr(p,'.');
        else
            p += strlen(p);
        if (*(--p) == 'G') strcpy(p,p+1);
    }

    // If no filename, exit
    if (strlen(lastFN) == 0) return;

    // Clear memory in preparation for loading the file
    doClearMem();

    // If the file doesn't exist, return with an error
    if (!exist(lastFN))
    {
        fileErr("File not found",lastFN);
        return;
    }

    // Open the file in read text mode
    DOSFILE fs;
    if (dosfopen(lastFN,OPEN_READ|OPEN_DENYWRIT,&fs) == -1)
    {
        fileErr("File open error",lastFN);
        return;
    }

    // Start reading in the lines while more lines and available space
    int i   = 0;
    int lls = FALSE;
    while (i < MAX_LINES)
    {
        int rv = dosfgets(curLine,sizeof(curLine),&fs);
        if (rv == -1) break;
        if ((rv == 1) && (strlen(curLine) > MAX_LINE_SIZE)) lls = TRUE;
        curLine[MAX_LINE_SIZE] = NUL;
     // replace(curLine,'\x0A',NUL);
     // replace(curLine,'\x0D',NUL);
     // replace(curLine,'\x1A',NUL);
        if (strlen(curLine) > 0)
        {
            tlOff = i;
            commitLine();
        }
        if (++i == MAX_LINES) editErr("Maximum lines reached in editor");
     // memset(curLine,0,MAX_LINE_SIZE+1);
    }

    // Reset top line offset back to 0
    tlOff = 0;

    chkAttrs(TRUE);

    // Close the file
    if ((dosfclose(&fs) == -1) && (i != -1))
        fileErr("File close error",lastFN);

    // Cache the line
    cacheLine();

    if (lls) fileErr("Long lines split",lastFN);
}

    /*--------------------------------------------------------------------*/

void pascal loadFile(void)
{
    if (!ioFlag)
    {
        buzz();
        return;
    }

    // If text changed confirm desire to load
    if (!textChanged || (textChanged && confirm("Text has been modified, "
        "do you really wish to load another file (Y/N/S)?  ",saveFile,'S')))
    {
//        setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
//        setAttr(statAttr);
//        wrStr("Enter filename to load:  ");
//        if (getFN()) doLoadFile();
        if (fileSelDialog("load")) doLoadFile();
    }
}

    /*--------------------------------------------------------------------*/

void pascal doSaveFile(void)
{
    int i;

    // If no filename, exit
    if (strlen(lastFN) == 0) return;

    // Strip trailing . if it exists
    char * lfnEnd = lastFN+strlen(lastFN)-1;
    if (*lfnEnd == '.') *lfnEnd = NUL;

    // If the last filename exists then...
    if (exist(lastFN))
    {
        if (bakFlag)
        {
            char bakName[PN_SIZE+1];

            strcpy(bakName,lastFN);
            char * ptr = bakName;

            while (strchr(ptr,'\\') != NULL) ptr = strchr(ptr,'\\')+1;
            if (strchr(ptr,'.') != NULL) *(strchr(ptr,'.')) = NUL;

            strcat(bakName,".BAK");

            if (exist(bakName) && (unlink(bakName) != 0))
            {
                fileErr("Delete file error",bakName);
                return;
            }

            if (rename(lastFN,bakName) != 0)
            {
                fileErr("Rename file error",lastFN);
                return;
            }
        }
        else
        {
            if (unlink(lastFN) != 0)
            {
                fileErr("Delete file error",lastFN);
                return;
            }
        }
    }

    // Open the file in write text mode
    DOSFILE fs;
    if (dosfopen(lastFN,OPEN_WRIT|OPEN_DENYRDWR,&fs) == -1)
    {
        fileErr("File open error",lastFN);
        return;
    }

    // Commit the text in curLine to the lineList
    commitLine();

    while ((lineList[lastLine] == NULL) && (lineList > 0)) --lastLine;

    // Write each line except for the last line
    for (i = 0; i <= lastLine; ++i)
    {
        char * ptr = lineList[i];
        if (ptr == NULL) ptr = "";
        if ((!ioFlag && (*ptr == '%') && (dosfputs("@BEEP@",&fs) == -1)) ||
            (dosfputs(ptr,&fs) == -1) ||
            ((i < lastLine) && (dosfputs("\r\n",&fs) == -1)) ||
            ((i == lastLine) && (dosfputs(lastLineEnd ? lastLineEnd : "\r\n\x1A",&fs) == -1)))
        {
            fileErr("File write error",lastFN);
            break;
        }
    }

    // Close the file
    if ((dosfclose(&fs) == -1) && (i != -1))
        fileErr("File close error",lastFN);

    lastLine = MAX_LINES-1;

    // Cache the line
    cacheLine();

    // Make sure textChanged flag is FALSE
    textChanged = FALSE;
}

    /*--------------------------------------------------------------------*/

void pascal saveFile(void)
{
    // Prompt the user and save a file if appropriate
//    setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
//    setAttr(statAttr);
//    wrStr("Enter filename to save:  ");
    if (!ioFlag || getFN("save",FALSE)) doSaveFile();
}

    /*--------------------------------------------------------------------*/

void pascal killFile(void)
{
    if (!ioFlag)
    {
        buzz();
        return;
    }


    // Prompt the user and save a file if appropriate
    char tmpFN[PN_SIZE+1];
    strcpy(tmpFN,lastFN);
    strcpy(lastFN,"");
//    setXY(1,statOn ? MAX_ROWS-1 : MAX_ROWS);
//    setAttr(statAttr);
//    wrStr("Enter filename to kill:  ");
//    if (getFN())
    if (fileSelDialog("kill"))
    {
        if ((strlen(lastFN) > 0) && (lastFN[strlen(lastFN)-1] == '.'))
            lastFN[strlen(lastFN)-1] = NUL;
        if (exist(lastFN))
            unlink(lastFN);
    }
    strcpy(lastFN,tmpFN);
}

/******************************************************************************/

