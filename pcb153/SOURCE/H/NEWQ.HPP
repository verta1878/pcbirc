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

#ifndef   ___NEWQ_HPP___
#define   ___NEWQ_HPP___
#include <dosclass.hpp>
class cNEWQ
{
public:
                cNEWQ              (void);
               ~cNEWQ              (void);


          bool  addEntry           (QUEUE_RECORD & rec);
          bool  removeEntry        (char * file);
          bool  removeEntry        (int slotNum);

          bool  addPoll            (char * node);
          bool  removePoll         (char * node);
          bool  removePoll         (int recNum);

          int   getCount           (void);
          bool  readRecord         (QUEUE_RECORD & rec, int recNum);

          bool  modifyRecord       (int recNum,QUEUE_RECORD rec);

          int   getNextOutbound    (QUEUE_RECORD & rec, int lastRead);

          int   getRecordNumber    (char * file,int field);

          bool  create             (void);

          void  view               (void);
//          bool  isOpen             (void){return (Qfile.handle != 0);}
          int   getOutboundCount   (void);

protected:

private:

static    cDOSFILE Qfile;           // The QUEUE file object
static    int instance;        // How many objects are active
          char    QfileName[50];    // Name of QUEUE file
          int     slotSize;         // Size of queue file record entry (including header)
          int     QSize;
          int     fileOpen;
static    QUEUE_HEADER   h;

          int     findSlot(void);
          bool    seekToSlot(int slotNum);
};
#endif
#endif
