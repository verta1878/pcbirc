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
#include "dosclass.hpp"
#include "structs.h"
#include "data.hpp"

#define ADD1FAIL    1
#define CLEARCOUNT  2

class cNEWQ : public cCONFIGBASE<QUEUE_RECORD>
{
public:
                cNEWQ              (void) : cCONFIGBASE<QUEUE_RECORD>("FIDOQUE.DAT",FALSE){}
               ~cNEWQ              (void);


          bool  addEntry           (QUEUE_RECORD & rec);
          bool  removeEntry        (char * file);
          bool  removeEntry        (int slotNum);

          bool  addPoll            (char * node,bool readStatus);
          bool  removePoll         (char * node);
          bool  removePoll         (int recNum);

          bool  addFREQ            (char *node, char *filename);
          bool  removeFREQ         (char *file);
          bool  removeFREQ         (int recNum);

          bool  readRecord         (QUEUE_RECORD & rec, int recNum);
          int   getPollRecord      (char * node);
          int   getFREQRecord      (char * node);
 unsigned int   findRec            (const char * str);

          bool  modifyRecord       (int recNum,QUEUE_RECORD rec);

          int   getNextOutbound    (QUEUE_RECORD & rec, int lastRead);
          int   getNextRecord      (QUEUE_RECORD & rec, int lastRead);

          int   getRecordNumber    (char * file,int field);
          uint  getCount           (void){return totRecs();}

          void  view               (void);
          void  scanQueue          (void);
          uint  getOutboundCount   (void);
          void  forEachMatch       (unsigned zone,unsigned net,unsigned node,int action);
          void  forEachMatch       (char * addr,int action);
          bool  checkForFiles      (const char * filename);

          bool  isDupe             (QUEUE_RECORD & rec);

static    bool  connect;

protected:

private:

          uint     QSize;
};

/*
class cSEND : public cCONFIGBASE<SEND>
{
  public:
                 cFREQDENY    (void) : cCONFIGBASE<NADDRESS>("SEND.DAT",FALSE){};
    unsigned int findRec      (const char * send);

  protected:
  private:
};
*/
#endif
#endif
