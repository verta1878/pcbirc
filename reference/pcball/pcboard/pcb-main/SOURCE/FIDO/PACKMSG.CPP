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


#include <structs.h>

DIRECTORIES        dirs;
DOSFILE            dirfile;
char               cfgfilename[66];
char               msgdir[66];
      unsigned int filever = 0;
const unsigned int curver = 3;

sprintf(cfgfilename,"%sPCBFIDO.CFG",PcbData.FidoLoc);

if(fileexist(cfgfilename) != 255 && dosfopen(cfgfilename,OPEN_READ | OPEN_DENYNONE,&dirfile) == 0)
{
  dosfread(&filever,sizeof(filever),&dirfile);
  if(filever == curver)
  {
      dosfread(&dirs,sizeof(dirs),&dirfile);
      maxstrcpy(msgdir,dirs.outgoing_msg);
  }
  //else                 report error
  dosfclose(&dirfile);
}





