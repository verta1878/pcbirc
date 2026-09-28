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


#include <pcbtools.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif


/***************************************************************************
 ***************************************************************************/
int pascal zmGetByte(int Timeout) {
  int Byte;

  zmFlushSpooler();

  // Timeout value is expressed in seconds so multiply by 18.2
  settimer(TIMEOUT_COUNTER, (((long) Timeout * 182L) / 10L));

  settimer(2,0);  // expire the CD/KBD timer right away

  do {
    if ((Byte = comminkey()) != -1)
      return(Byte);

    if (gettimer(2) <= 0) {  // if CD/KBD timer expired, then check them now
      if (Asy.LostCarrier || ! cdstillup()) {   // is CD is down, then get out now
        Asy.LostCarrier = TRUE;
        zmAbort = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
//      return(TIMEOUT);   was timeout, try CAN instead
        return(CAN);
      }

      if (kbdinkey() == 0x1B) {  // if ESC is pressed, then get out now
        zmAbort = TRUE;
        return(CAN);
      }
      settimer(2,18); // frequency to check for carrier or keyboard is 1 second
    }

    giveup();
  } while (gettimer(TIMEOUT_COUNTER) > 0);

  if (Asy.LostCarrier || ! cdstillup()) {
    Asy.LostCarrier = TRUE;
    zmAbort = TRUE;
    if (PcbData.Packet)
      turnoffdtr();
    return(CAN);
  }

  return TIMEOUT;
}
