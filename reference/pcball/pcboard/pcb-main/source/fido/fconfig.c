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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mem.h>
#include <malloc.h>

#include <misc.h>
#include <dosfunc.h>
#include <newdata.h>
#include <pcboard.h>
//#include <messages.h>
//#include <pcb.h>

#include "fconfig.h"
#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include <tossmisc.h>
#include <data.hpp>

#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef PCBSETUP
#define bmalloc malloc
#define bfree free
#endif

#ifdef PCBSETUP
#define writeFidolog(a,b)
#endif

NODELIST					*nodelist_list=NULL;
AREA_STRUCT 				*area_list	  =NULL;
NODE_T						*node_list	  =NULL;
NADDRESS					*address	  =NULL;
#ifdef PCBSETUP
TRANSLATE					*record_list  =NULL;
#endif
NFREQ_PATH					*reqlist	  =NULL;
NFREQ_MAGIC 				*magic_list   =NULL;
THIS_ADDRESS				*deny_list	  =NULL;
FREQ_INFO		  _FAR_ 	  freq_info;
ARCHIVERS		  _FAR_ 	  archiver_info;
DIRECTORIES 	  _FAR_ 	  directory_info;
EMSI_DATA		  _FAR_ 	  emsi_data;
unsigned int				num_lists,num_areas,node_count,num_akas,num_records,num_reqs,num_magic,num_deny;
unsigned int				ConfigFileVer;

//static int				  i;

extern char ALLFILE[];

#ifdef	PCBSETUP
static unsigned int 		old_num_lists,old_num_areas,old_node_count,old_num_akas,old_num_records,old_num_reqs,old_num_magic,old_num_deny;
#endif

/*****************************************************************************/
/* Read Config File.														 */

bool LIBENTRY read_fido_config(int flags)
{
char   logBuf[65];

   maxstrcpy(logBuf,"Out of memory reading config file on ",sizeof(logBuf));
  // read in the fixed length configuration data
  // Brackets are to limit the scope of c
  {
	if(flags&AREA_RECS)
	{
	 cORIGINS o;
	 cNODELISTS n;
	 cXLATEPHONE x;
	 cAREAS 	 a;
//	   cSEND	   s;
	}
	cCONFIG c;
	c.getDirs(directory_info);
	c.getEMSI(emsi_data);
	c.getFREQInfo(freq_info);
	c.getArchInfo(archiver_info);
  }

  // Read in system AKAs
  if(flags&ADDR_RECS)
  {
	cAKAS akas;
	NADDRESS rec;
	unsigned int numrecs = akas.totRecs();
	unsigned int i=0;

	 memset(&rec,0,sizeof(rec));

	 // If there are no records, then bail
	if(numrecs != 0)
	{

	 if((address=(NADDRESS *)bmalloc(numrecs*sizeof(NADDRESS)))==NULL)
	 {
	   free_fido_memory();
	   strcat(logBuf," AKAs");
	   writeFidolog(logBuf,BLOCK);
	   return(FALSE);
	 }
	 memset(address,0,numrecs*sizeof(NADDRESS));
	 for(i=0;i<numrecs;i++)
	 {
		rec = akas.getRec(i+1);
		memcpy(&address[i],&rec,sizeof(address[i]));
	 }
	 num_akas = numrecs;
	}
	else
	{
	  address=(NADDRESS *)bmalloc(sizeof(NADDRESS));
	  memset(address,0,sizeof(NADDRESS));
	}

  }

  // Read in node specific archiver info
  if(flags&NODE_RECS)
  {
	cNODEARC nodes;
	NNODE_T   rec;
	unsigned int numrecs = nodes.totRecs();
	unsigned int i = 0;
	if(numrecs != 0)
	{
	memset(&rec,0,sizeof(rec));


	if((node_list=(NODE_T *)bmalloc(numrecs*sizeof(NODE_T)))==NULL)
	{
	  free_fido_memory();
	  strcat(logBuf," NODEARC");
	  writeFidolog(logBuf,BLOCK);
	  return(FALSE);
	}
	 memset(node_list,'Y',numrecs*sizeof(NODE_T));
	for(i=0;i<numrecs;i++)
	{
		// The structure returned by getRec is missing the nodestr field so it
		// is skipped in the memcpy. It was removed because it was redundant
		rec = nodes.getRec(i+1);
		memcpy(&node_list[i].zone,&rec,(sizeof(node_list[i]) - sizeof(node_list[i].nodestr)));
		if(rec.point == 0)
		   sprintf(node_list[i].nodestr,"%u:%u/%u",node_list[i].zone,node_list[i].net,node_list[i].node);
		else
		   sprintf(node_list[i].nodestr,"%u:%u/%u.%u",node_list[i].zone,node_list[i].net,node_list[i].node,node_list[i].point);

	}
	node_count = numrecs;
	}
  }
#ifdef PCBSETUP
  // read phone translatetion records
  if(flags&XLAT_RECS)
  {
	cXLATEPHONE xlat;
	TRANSLATE  rec;
	unsigned int numrecs = xlat.totRecs();
	unsigned int i		 = 0;

	if(numrecs != 0)
	{
	  if((record_list=(TRANSLATE *)bmalloc(numrecs*sizeof(TRANSLATE)))==NULL)
	  {
		free_fido_memory();
		strcat(logBuf," PHONEX");
		writeFidolog(logBuf,BLOCK);
		return(FALSE);
	  }
	  for(i=0;i<numrecs;i++)
	  {
		rec = xlat.getRec(i+1);
		memcpy(&record_list[i],&rec,sizeof(TRANSLATE));
	  }
	  num_records = numrecs;
	}
  }
#endif // PCBSETUP

  if(flags&LIST_RECS)
  {
	cNODELISTS lists;
	NODELIST   rec;
	unsigned int numrecs = lists.totRecs();
	unsigned int i		 =0;

	if(numrecs != 0)
	{
	  if((nodelist_list=(NODELIST *)bmalloc(numrecs*sizeof(NODELIST)))==NULL)
	  {
		free_fido_memory();
		strcat(logBuf," NODELIST");
		writeFidolog(logBuf,BLOCK);
		return(FALSE);
	  }

	  for(i=0;i<numrecs;i++)
	  {
		rec = lists.getRec(i+1);
		memcpy(&nodelist_list[i],&rec,sizeof(NODELIST));
	  }
	  num_lists = numrecs;
	}
  }


  // Read in FREQ paths
  if(flags&FREQ_RECS)
  {

   cFREQPATHS paths;
   NFREQ_PATH  rec;
   unsigned int numrecs = paths.totRecs();
   unsigned int i		= 0;

   if(numrecs != 0)
   {

	if((reqlist=(NFREQ_PATH *)bmalloc(numrecs*sizeof(NFREQ_PATH)))==NULL)
	{
	  free_fido_memory();
	   strcat(logBuf," FREQPATH");
	   writeFidolog(logBuf,BLOCK);
	  return(FALSE);
	}
	memset(reqlist,'X',numrecs*sizeof(NFREQ_PATH));
	for(i=0;i<numrecs;i++)
	{
		rec = paths.getRec(i+1);
		memcpy(&reqlist[i],&rec,sizeof(reqlist[i]));
	}
	num_reqs = numrecs;
   }
  }

  // read in magic names
  if(flags&MAGIC_RECS)
  {
	cMAGICNAMES magic;
	NFREQ_MAGIC  rec;
	unsigned int numrecs = magic.totRecs();
	unsigned int i = 0;

	if(numrecs != 0)
	{
	if((magic_list=(NFREQ_MAGIC *)bmalloc(numrecs*sizeof(NFREQ_MAGIC)))==NULL)
	{
	  free_fido_memory();
	  strcat(logBuf," FREQMAGIC");
	  writeFidolog(logBuf,BLOCK);
	  return(FALSE);
	}
	memset(magic_list,'W',numrecs*sizeof(NFREQ_MAGIC));
	for(i=0;i<numrecs;i++)
	{
		rec = magic.getRec(i+1);
		memcpy(&magic_list[i],&rec,sizeof(magic_list[i]));
	}
	num_magic = numrecs;
	}
  }

  // read in FREQ deny list
  if(flags&DENY_RECS)
  {

   cFREQDENY deny;
   NADDRESS  rec;
   unsigned int numrecs = deny.totRecs();
   unsigned int i = 0;

   if(numrecs != 0)
   {
	if((deny_list=(THIS_ADDRESS *)bmalloc(numrecs*sizeof(THIS_ADDRESS)))==NULL)
	{
	  free_fido_memory();
	  strcat(logBuf," FREQDENY");
	  writeFidolog(logBuf,BLOCK);
	  return(FALSE);
	}
	memset(deny_list,'V',numrecs*sizeof(THIS_ADDRESS));
	for(i=0;i<numrecs;i++)
	{
		// The structure returned by getRec is missing the nodestr field so it
		// is skipped in the memcpy. It was removed because it was redundant
		rec = deny.getRec(i+1);
		memcpy(&deny_list[i].this_zone,&rec,( sizeof(deny_list[i]) - sizeof(deny_list[i].nodestr) ) );
		sprintf(deny_list[i].nodestr,"%u:%u/%u",deny_list[i].this_zone,deny_list[i].this_net,deny_list[i].this_node);
		if(deny_list[i].this_point != 0)
		{
		  char pnt[10];
			 sprintf(pnt,"u",deny_list[i].this_point);
			 strcat(deny_list[i].nodestr,pnt);
		}
	 }
	 num_deny = numrecs;
	}
  }


#ifdef	PCBSETUP
  old_num_deny=num_deny;
  old_num_magic=num_magic;
  old_num_reqs=num_reqs;
  old_num_lists=num_lists;
  old_num_records=num_records;
  old_num_records=num_records;
  old_node_count=node_count;
  old_num_akas=num_akas;
  old_num_areas=num_areas;
#endif

  return(TRUE);
}

/*****************************************************************************/
/* Copy a file. 															 */

void CopyFile(char _FAR_ *source,char _FAR_ *destination)
{
		int 	InFile;
		int 	OutFile;
		int 	len;
  char	String[200];
  #ifdef __OS2__
	os2errtype Os2Error;
  #endif

  char *block = (char *)calloc (16384, sizeof (char));

		if ((InFile = dosopen (source, OPEN_READ | OPEN_DENYNONE POS2ERROR))==-1) {
				sprintf (String, "(15) Unable to open source filename %s.", source);
				free (block);
				return;
		}
		if ((OutFile = doscreate (destination, OPEN_WRIT | OPEN_DENYNONE, OPEN_NORMAL POS2ERROR)) == -1) {
				sprintf (String, "(16) Unable to create destination file %s.", destination);
				dosclose (InFile);
				free (block);
				return;
		}

		while (1) {
				len=dosread (InFile, block, 16384 POS2ERROR);					// Read 16K blocks at a time
				if (len > 0)
						if (doswrite (OutFile, block, len POS2ERROR)!=len)
								break;
				if (len <=0)
						break;
		}

		dosclose (OutFile);
		dosclose (InFile);
		free (block);
}

#ifdef PCBSETUP

/*****************************************************************************/
/* Write Config File.														 */

bool LIBENTRY write_fido_config(void)
{
  // Write out fixed length records in order they appear in the file
  // Brackets are needed to limit the scope of C
  {
	cCONFIG c;
	c.putDirs(directory_info);
	c.putEMSI(emsi_data);
	c.putFREQInfo(freq_info);
	c.putArchInfo(archiver_info);
  }


  // Write out all area tags
  if(area_list!=NULL)
  {

  }

  // Write out all system AKAs
  if(address!=NULL)
  {
   cAKAS		 akas;
   NADDRESS 	 rec;
   unsigned int  i=0;

	akas.removeFile();

	for(i=0;i<num_akas;i++)
	{
	   memset(&rec,0,sizeof(rec));
	   memcpy(&rec,&address[i],sizeof(rec));
	   if(rec.zone == 0 || rec.net == 0) continue;
	   akas.addRec(rec);
	}
  }

  // Write out all node specific archiver info
  if(node_list!=NULL)
  {
	cNODEARC	 arcs;
	NNODE_T 	 rec;
	unsigned int i =0;

	arcs.removeFile();
	for(i=0;i<node_count;i++)
	{
		memset(&rec,0,sizeof(rec));
		memcpy(&rec,&node_list[i].zone,sizeof(rec));
		if(rec.zone == 0 || rec.net == 0) continue;
		arcs.addRec(rec);
	}
  }

  // Write out all phone translation info
  if(record_list!=NULL)
  {
	cXLATEPHONE   N;
	unsigned int i=0;

	N.removeFile();

	for(i=0;i<num_records;i++)
	  N.addRec(record_list[i]);
  }

  // Write out all nodelist info
  if(nodelist_list!=NULL)
  {
	cNODELISTS	  N;
	unsigned int  i=0;

	  N.removeFile();
	  for(i=0;i<num_lists;i++)
		 N.addRec(nodelist_list[i]);
  }


  // Write out all FREQ path records
  if(reqlist!=NULL)
  {
   cFREQPATHS	 F;
   unsigned int i=0;

	F.removeFile();
	for(i=0;i<num_reqs;i++)
	  F.addRec(reqlist[i]);

  }

  // Write out magic names info
  if(magic_list!=NULL)
  {
   cMAGICNAMES	M;
   unsigned int i=0;
   NFREQ_MAGIC	rec;

	M.removeFile();
	for(i=0;i<num_magic;i++)
	{
	  memcpy(&rec,&magic_list[i],sizeof(rec));
	  M.addRec(rec);
	}

  }

  // Write out FREQ deny list
  if(deny_list!=NULL)
  {
   cFREQDENY	D;
   NADDRESS 	rec;
   unsigned int i=0;

	D.removeFile();
	for(i=0;i<num_deny;i++)
	{
		memset(&rec,0,sizeof(rec));
		memcpy(&rec,&deny_list[i].this_zone,sizeof(rec));
		if(rec.zone == 0 || rec.net == 0) continue;
		D.addRec(rec);
	}
  }
  return(TRUE);
}
#endif

/*****************************************************************************/
/* Free memory from bmalloc 												 */

void LIBENTRY free_fido_memory(void)
{
  if(nodelist_list!=NULL)
	{
	  bfree(nodelist_list);
	  nodelist_list=NULL;
	}

  if(node_list!=NULL)
	{
	  bfree(node_list);
	  node_list=NULL;
	}

  if(area_list!=NULL)
	{
	  bfree(area_list);
	  area_list=NULL;
	}

  if(address!=NULL)
	{
	  bfree(address);
	  address=NULL;
	}

#ifdef PCBSETUP
  if(record_list!=NULL)
	{
	  bfree(record_list);
	  record_list=NULL;
	}
#endif

  if(reqlist!=NULL)
	{
	  bfree(reqlist);
	  reqlist=NULL;
	}

  if(magic_list!=NULL)
	{
	  bfree(magic_list);
	  magic_list=NULL;
	}

  if(deny_list!=NULL)
	{
	  bfree(deny_list);
	  deny_list=NULL;
	}
}

#endif
