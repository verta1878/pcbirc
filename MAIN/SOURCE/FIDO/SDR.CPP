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


#include    <dir.h>
#include    <stdio.h>
#include    <string.h>

#include    <borland.h>
#include    <pcbtools.h>

char * pascal makeUniqueFZipName ( char * name )
{
    // Initialize day name abbreviations
    char * dayAbbr [] = { "SU", "MO", "TU", "WE", "TH", "FR", "SA" };

    // Get the day of week number (0 = SU)
    int dow = getjuliandate() % 7;

    // A temporary pointer for our use
    char * p;

    // Extract path info from name
    char path [ 128 + 1 ];
    strcpy(path,name);
    p = strrchr(path,'\\');
    if (p)
        p[1] = 0;
    else if (path[1] == ':')
        path[2] = 0;
    else
        path[0] = 0;

    // Mask for files to search
    strcat(name,".*");

    // Loop through files
    ffblk curFFB;
    int done = findfirst(name,&curFFB,0);
    while (!done)
    {
        // Position on extension
        p = curFFB.ff_name;
        p += strlen(p)-3;

        // If the days don't match and file size is 0, delete the file
        if (((p[0] != dayAbbr[dow][0]) || (p[1] != dayAbbr[dow][1])) &&
            (curFFB.ff_fsize == 0))
        {
            char buf [ 128 + 1 ];
            strcpy(buf,path);
            strcat(buf,curFFB.ff_name);
            unlink(buf);
        }

        // Next file
        done = findnext(&curFFB);
    }

    // Position on wildcard
    p = name;
    p += strlen(p)-1;

    // Copy zeroth extension
    strcpy(p,dayAbbr[dow]);
    p[2] = '0';
    p[3] = 0;

    // Loop until we find a unique extension
    while (fileexist(name) != 255)
        ++p[2];

    // Return unique name
    return name;
}

#ifdef TEST

int main ( void )
{
    char buf [ 12 ] = "FILENAME";

    makeUniqueFZipName(buf);

    return 0;
}

#endif

