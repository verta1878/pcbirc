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
#include <pcboard.h>


#include <dbase.hpp>

#include <misc.h>
#include <screen.h>

#include "defines.h"
#include "structs.h"
#include "prototyp.h"

#ifdef DEBUG
#include <memcheck.h>
#endif


extern _FAR_ DIRECTORIES  directory_info;

//extern "C"
bool get_node(unsigned int zone, unsigned int net, unsigned int node, NODE_REC *noderec)
{
	cDBF nodeList;
	char nodelist[100];
	char index[100];

	checkstack();
	sprintf(nodelist,"%sNODELIST.DBF",directory_info.nodelist_path);
	sprintf(index,"%sNODELIST.NDX",directory_info.nodelist_path);

	nodeList.dbfOpen(nodelist,FALSE);
	nodeList.ndxOpen(index);
	if (nodeList.error()) return FALSE;

	char key [ 5+5+5+1 ];

	sprintf(key,"%5u%5u%5u",zone,net,node);
	nodeList.seek(key);

	bool found = !nodeList.error();
	int status;
	status=nodeList.returnStatus();

	if(status==0)
	  found=TRUE;
	else
	  found=FALSE;

	if(found)
	{
		char sconfig  [ 4 + 1 ];
		char scomp	  [ 4 + 1 ];
		char scapable [ 4 + 1 ];

		long i;
		int maxBaud;

		nodeList.get("ZONE",i);
		noderec->zone=(unsigned)i;
		nodeList.get("NET",i);
		noderec->net=(unsigned)i;
		nodeList.get("NODE",i);
		noderec->node=(unsigned)i;
		nodeList.get("BBS_NAME",    noderec->BBS_name);
		nodeList.get("LOCATION",    noderec->location);
		nodeList.get("SYSOP_NAME",  noderec->Sysop_name);
		nodeList.get("BBS_PHONE",   noderec->phone);
		nodeList.get("MAXBAUD", maxBaud);
		nodeList.get("CONFIG",  sconfig);
		nodeList.get("COMP",    scomp);
		nodeList.get("CAPABLE", scapable);

		unsigned config  = unsigned(atol(sconfig));
		unsigned comp	 = unsigned(atol(scomp));
		unsigned capable = unsigned(atol(scapable));

		switch(maxBaud)
		  {
			case	300:	noderec->maximum_baud = BAUD_300;
						  break;

			case   1200:	noderec->maximum_baud = BAUD_1200;
						  break;

			case   2400:	noderec->maximum_baud = BAUD_2400;
						  break;

			case   4800:	noderec->maximum_baud = BAUD_4800;
						  break;

			case   9600:	noderec->maximum_baud = BAUD_9600;
						  break;

			case  14400:	noderec->maximum_baud = BAUD_14400;
						  break;

			case  16800:	noderec->maximum_baud = BAUD_16800;
						  break;
		  }

		memcpy(&noderec->ConfigFlags,&config,sizeof(noderec->ConfigFlags));
		memcpy(&noderec->CompFlags,&comp,sizeof(noderec->CompFlags));
		memcpy(&noderec->Capabilities,&capable,sizeof(noderec->Capabilities));
		stripright(noderec->BBS_name,' ');
		stripright(noderec->location,' ');
		stripright(noderec->Sysop_name,' ');
		stripright(noderec->phone,' ');
	}
  nodeList.dbfClose();
  return found;
}
#endif
