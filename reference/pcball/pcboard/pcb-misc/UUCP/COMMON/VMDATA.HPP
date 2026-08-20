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


#include    <stddef.h>

#include    <vmdata.h>

class cVMDATA
{

    public:

        cVMDATA ( char * swapFile = "$VMDATA$.$$$",
            unsigned minReqKB = 16, unsigned minLeaveKB = 16,
            VMBool ctrlCHandler = VM_FALSE )
        {
            if (++instances == 1)
                /*VMDataStartUp(swapFile,minReqKB,minLeaveKB,ctrlCHandler)*/;
        }

        virtual ~cVMDATA ( void ) { if (instances-- == 1) /*VMDataShutDown()*/; }

        void pascal initRecSet ( unsigned recSize, void * recBuf = NULL,
            unsigned recBufLen = 0 )
        {
            VMInitRec(&dataSet,recBuf,recBufLen,recSize);
        }

        void pascal initRecVarIdxSet ( VMSizeFunc * sizeFunc,
            unsigned tables, void * idxBuf, unsigned idxBufLen,
            void * recBuf = NULL, unsigned recBufLen = 0 )
        {
            VMInitRecVarIdx(&dataSet,recBuf,recBufLen,sizeFunc,
                tables,idxBuf,idxBufLen);
        }

        void pascal doneSet ( void )
        {
         // VMRecordBufferDisable(&dataSet,VM_TRUE);
         // VMRecordBufferFree(&dataSet);
         // VMIndexBufferDisable(&dataSet);
         // VMIndexBufferFree(&dataSet);
         // VMRelease(&dataSet);
            VMDone(&dataSet);
        }

     // void * pascal createRec ( unsigned size, long & retPos, long & retIdx )
     // {
     //     return VMRecordCreate(&dataSet,size,&retPos,&retIdx);
     // }

     // void * pascal createRec ( unsigned size, long & retPos )
     // {
     //     return VMRecordCreate(&dataSet,size,&retPos,NULL);
     // }

        void * pascal createRec ( unsigned size )
        {
            return VMRecordCreate(&dataSet,size,NULL,NULL);
        }

        void pascal changedRec ( void )
        {
            VMRecordChanged(&dataSet);
        }

        void pascal unchangedRec ( void )
        {
            VMRecordUnChanged(&dataSet);
        }

        long pascal recCount ( void )
        {
            return VMRecordCount(&dataSet);
        }

        void pascal sort ( unsigned size, long first, long count, VMBool cross,
            VMCompareFunc * cmpFunc, VMSortFunc * sortFunc,
            void * workBuf, unsigned workBufLen )
        {
            VMSort(&dataSet,size,first,count,cross,cmpFunc,sortFunc,
                workBuf,workBufLen);
        }

     // void * pascal recIdxGet ( long idx, long & pos )
     // {
     //     return VMRecordGetByIndex(&dataSet,idx,&pos);
     // }

        void * pascal recIdxGet ( long idx )
        {
            return VMRecordGetByIndex(&dataSet,idx,NULL);
        }

    protected:

        VMDataSet dataSet;

    private:

        static int instances;

};

