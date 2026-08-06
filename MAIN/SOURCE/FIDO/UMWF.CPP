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


#include    <malloc.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>

#ifdef LIB
#include    "pcbtools.h"
#undef bool
#undef true
#undef false
#else
#include    "misc.h"
#include    "pcboard.h"
#include    "pcboard.ext"
#include    "messages.h"
#include    "users.h"
#endif

#include    "umwf.hpp"

#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

#pragma pack(1)
struct UMWF_RECORD
{
    char     name [ 25 ];
    uint     conf;
};
#pragma pack()

const int maxUMWFListSize = 32767U / sizeof(UMWF_RECORD); // 65535U / sizeof(UMWF_RECORD);

// static UMWF_RECORD _FAR_ umwfList [ maxUMWFListSize ];

static UMWF_RECORD * umwfList = NULL;

static int umwfSize = 0;

/******************************************************************************/

void LIBENTRY initUMWF(void)
{
    if (umwfList == NULL)
    {
        umwfList = new UMWF_RECORD [ maxUMWFListSize ];
        umwfSize = 0;
    }
}

    /*--------------------------------------------------------------------*/

void LIBENTRY resetUMWF(void)
{
    if (umwfList != NULL)
    {
        delete [] umwfList;
        umwfList = NULL;
        umwfSize = 0;
    }
}

    /*--------------------------------------------------------------------*/

static int cmpUMWF(const void * l, const void * r)
{
    UMWF_RECORD * lr = (UMWF_RECORD *) l;
    UMWF_RECORD * rr = (UMWF_RECORD *) r;

    int cmp = memcmp(lr->name,rr->name,sizeof(lr->name));

    if (cmp == 0) cmp = lr->conf - rr->conf;

    return cmp;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY doUMWF(void)
{
    if (umwfList == NULL) return;

    if (umwfSize == 0) return;

    qsort(umwfList,umwfSize,sizeof(UMWF_RECORD),cmpUMWF);

    char umwfName [ 25 + 1 ];
    memset(umwfName,0,sizeof(umwfName));

    unsigned umwfConf = 65535U;
                        // This is another PC-Lint avoidance initialization.
                        // And it is unsigned because we probably don't need
                        // to worry about having 64K of conferences and can
                        // use that as our sentinal value (instead of having
                        // to use a long and using -1).

    long umwfUser = -1; // This initialization isn't absolutely necessary
                        // because I initialize umwfName above to all NULs
                        // (which is an invalid user name), but it doesn't
                        // hurt and it keeps PC-Lint from complaining.
                        // How's *that* David?  ;)

    for (int i = 0; i < umwfSize; ++i)
    {
        if (memcmp(umwfList[i].name,umwfName,25) != 0)
        {
            memcpy(umwfName,umwfList[i].name,25);
            umwfUser = finduser(umwfName);
            umwfConf = 65535U;
           #ifndef EXCLUDE_DEBUG
            if (DebugLevel >= 5)
            {
                char tmpbuf [ 62 + 1 ];
                sprintf(tmpbuf,"%5ld - %s",umwfUser,umwfName);
                writedebugrecord(tmpbuf);
            }
           #endif
        }

        if (umwfUser == -1) continue;

        if (umwfConf == umwfList[i].conf) continue;

        umwfConf = umwfList[i].conf;
        putmessagewaiting(umwfConf,umwfUser);
    }

    umwfSize = 0;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY addUMWF(char * name, unsigned conf)
{
    if (umwfList == NULL) return;

    if (umwfSize == maxUMWFListSize) doUMWF();

    char buf [ 25 + 1 ];
    sprintf(buf,"%-25.25s",name);
    strupr(buf);

    if (strcmp(buf,"ALL                      ") == 0) return;

    UMWF_RECORD rec;

    memcpy(rec.name,buf,25);
    rec.conf = conf;

    memcpy(&umwfList[umwfSize++],&rec,sizeof(UMWF_RECORD));
}

/******************************************************************************/

