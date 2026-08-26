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


/****************************************************************************
*
*                             PCBoard FIDO
*                           Module DUPECHEK.HPP
*
*                         Written by Stan Paulsen
*                            Copyright (C) 1995
*                            Clark Development
*
*****************************************************************************
*
*       Class Definitions for Fido Duplicate Message Checking on import
*
*****************************************************************************/
#ifndef DUPECHEC_HPP
#define DUPECHEC_HPP
#include "types.hpp"

typedef struct
{
  char   msgid[40];
}MSGID;

class cDUPECHECK
{

  public:
/*                    LIBENTRY      cDUPECHECK        (void){} */
/*                    LIBENTRY      ~cDUPECHECK       (void){} */

  bool              LIBENTRY      checkPath         (const char * msg, uint orgzone);
  bool              LIBENTRY      checkID           (const char * msg,pcbconftype * conf);

  protected:

  private:
};

class cMSGID : public cCONFIGBASE<MSGID>
{
  public:
                    LIBENTRY      cMSGID(const char * idfilename) : cCONFIGBASE<MSGID>(idfilename,TRUE){};
  bool              LIBENTRY      find(MSGID & id);
  void              LIBENTRY      put(MSGID & id);

  protected:

  private:

  uint        maxrecs;
  uint        mrecnum;
  MSGID       marker;

};
#endif
