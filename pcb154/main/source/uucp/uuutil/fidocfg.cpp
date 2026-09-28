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


#ifdef FIDO

#include    <stdio.h>
#include    <string.h>

#include <pcbtools.h>
#include    <fidocfg.hpp>
extern pcbdattype PcbData;

cFIDOCFG::cFIDOCFG(void) :
    version(0),
    numAreas(0)
{
    // Open the source file
    srcName = PcbData.FidoConfig;
    src.open(srcName,OPEN_RDWR|OPEN_DENYNONE);

    // Create the destination file
    strcpy(destName,srcName);           // Get a copy of the source name

    char * p1 = strrchr(destName,'\\'); // Find the last dir separator
    p1 = (p1 ? (p1+1) : destName);      // Move past it
    char * p2 = strchr(p1,'.');         // Find the extension
    if (!p2) p2 = p1+strlen(p1);        // If none, move to the end of string
    strcpy(p2,".$$$");                  // Replace the extension for temp file

    dest.create(destName,OPEN_RDWR|OPEN_DENYRDWR,OPEN_NORMAL);

    // Read the source version and area count
    if (src.isOpen())
    {
        src.read(&version,sizeof(version));
        src.read(&numAreas,sizeof(numAreas));
    }
    else
    {
        version = 2;
        numAreas = 0;
    }

    // Write them out to the new destination file
    dest.write(&version,sizeof(version));
    dest.write(&numAreas,sizeof(numAreas));
}

cFIDOCFG::~cFIDOCFG(void)
{
    // Allocate buffer for copying
    char buf [ 1024 ];
    int bufSize = sizeof(buf);

    // Copy the entire file from source to dest
    if (src.isOpen())
    {
        int br;
        while ((br = src.read(buf,bufSize)) > 0) dest.write(buf,br);
    }

    // Write updated number of areas
    dest.seek(sizeof(version),SEEK_SET);
    dest.write(&numAreas,sizeof(numAreas));

    // Close the files
    src.close();
    dest.close();

    // Kill the old file and rename the new file
    unlink(srcName);
    rename(destName,srcName);
}

void pascal cFIDOCFG::addNewArea(AREA_STRUCT & newArea)
{
    // Increment the number of areas
    ++numAreas;

    // Write the new area information
    dest.write(&newArea,sizeof(newArea));
}

void pascal cFIDOCFG::addNewArea(int conf, char * name, char * msgs)
{
    // Initialize and write the new area structure
    AREA_STRUCT newArea;

    memset(&newArea,0,sizeof(newArea));
    maxstrcpy(newArea.Area_Name,name,sizeof(newArea.Area_Name));
 // maxstrcpy(newArea.Messages, msgs,sizeof(newArea.Messages));
    newArea.PCB_Conference = conf;

    addNewArea(newArea);
}

#endif

