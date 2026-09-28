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
/* VERY IMPORTANT NOTES!!!                                                    */
/* ~~~~~~~~~~~~~~~~~~~~~~~                                                    */
/* 1. This module requires a large stack for VMSort                           */
/* 2. Define ___CI_INIT_VM___ to include initialization for VMDATA access     */
/* 3. Define ___CI_INIT_CN___ to include initialization for CNAMES access     */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

#include    <stdlib.h>
#include    <string.h>

#include    <vmdata.h>

#include    <dosfunc.h>
#include    <newdata.h>
#include    <pcb.h>

#include    <cnameidx.h>

#ifdef DEBUG
#include    <memcheck.h>
#endif

/******************************************************************************/

#define     NUL                 '\0'

#ifdef ___CI_INIT_CN___
#define     RO_CNAMES           FALSE
#endif

/******************************************************************************/

static VM_SHINT cmpidxentries(const void * l, const void * r)
{
    return stricmp((char*)l,(char*)r);
}

    /*--------------------------------------------------------------------*/

#if defined(LIB) && !defined(QUIET_BUILD)

#include    <conio.h>

void pascal fastprint(int x, int y, char * s, int a)
{
    int tx = wherex();
    int ty = wherey();
    gotoxy(x+1,y+1);
    textattr(a);
    cputs(s);
    gotoxy(tx,ty);
}

#endif

    /*--------------------------------------------------------------------*/

// #include    <alloc.h>

void pascal buildcnamesidx(void)
{
    unsigned        i, Confs;
    cnamesidxtype * Rec;
    VMDataSet       CnamesIdxSet;
    pcbconftype     Conf;
    cnamesidxtype   SortBuf [ 256 ];

   #ifdef ___CI_INIT_VM___
    VMDataStartUp("CISWAP.TMP", 16, 16, VM_FALSE);
    VMDebugOn(VM_MURPHY|VM_SANITY_CHECK|VM_INFO);
   #endif

    VMInitRec(&CnamesIdxSet,NULL,0,sizeof(cnamesidxtype));

    Confs = 0;

   #ifdef ___CI_INIT_CN___
    loadcnames(RO_CNAMES);
   #endif

    for (i = 0; i <= PcbData.NumConf; ++i)
    {
        getconfrecord(i,&Conf);
        if (Conf.Name[0] == NUL) continue;

       #ifndef QUIET_BUILD
        fastprint(0,0,"                              "
                      "                              ",0x07);
        fastprint(0,0,Conf.Name,0x07);
       #endif

     // printf("%lu - %d\n",coreleft(),heapcheck());

        Rec = (cnamesidxtype *) VMRecordCreate(&CnamesIdxSet,
            sizeof(cnamesidxtype),NULL,NULL);
        if (Rec == NULL) break;

        memset(Rec,0,sizeof(*Rec));
        strcpy(Rec->Name,Conf.Name);
        Rec->Num = i;
        ++Confs;
    }

   #ifdef ___CI_INIT_CN___
    closecnames();
   #endif

    VMSort(&CnamesIdxSet,sizeof(cnamesidxtype),1,Confs,VM_FALSE,
        cmpidxentries,(VMSortFunc*)qsort,SortBuf,sizeof(SortBuf));

    opencnamesidx(CREATE_IDX);

    if (IdxHand > 0)
    {
        for (i = 1; i <= Confs; ++i)
        {
            Rec = (cnamesidxtype *) VMRecordGetByIndex(&CnamesIdxSet,i,NULL);
           #ifndef QUIET_BUILD
            fastprint(0,0,"                              "
                          "                              ",0x07);
            fastprint(0,0,Rec->Name,0x07);
           #endif
            if (writecheck(IdxHand,Rec,sizeof(*Rec)) == (unsigned)-1) break;
        }
    }

   #ifndef QUIET_BUILD
    fastprint(0,0,"                              "
                  "                              ",0x07);
   #endif

    closecnamesidx();

    VMDone(&CnamesIdxSet);

   #ifdef ___CI_INIT_VM___
    VMDataShutDown();
   #endif
}

/******************************************************************************/

