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
*								DATA.CPP * *				  Pcboard's
FIDO configuration class *						   Author: Stan Paulsen *
Copyright Clark Development 1995 * *			 Member functions related
to configuration data * Classes included * cAREAS, cORIGINS, cNODEARC,
cXLATEPHONE, cAKAS, cNODELISTS, cFREQPATHS, * cFREQDENY, cMAGICNAMES,
cCONFIG
*****************************************************************************/
#ifdef FIDO
/****************************************************************************
Include Files
*****************************************************************************/
// Borland Include Files
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <ctype.h>

#ifdef PCBSETUP
#include <vmdata.h>
#define writeFidolog(a,b)
#endif

// PCBoard specific headers

#include "data.hpp"
#include "prototyp.h"
#include "tossmisc.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

/*
static void LIBENTRY addrToNum(const char * addr, uint & zone,
								uint & net, uint & node,uint & point);
*/

int idxcmp(const void * r1, const void * r2);
short areacmp(const void * a, const void * b);


/*****************************************************************************
* Member functions for the cAREAS class. This class maintains data associated
* with the FIDO conference information (area tags orgin line and akas for that
* conf.
******************************************************************************/



/*****************************************************************************
* Member insertSorted: Inserts a record in sorted order
*
*
******************************************************************************/
void cAREAS::insertSorted(NAREA_STRUCT & rec)
{
cDOSFILE	  tmp;
NAREA_STRUCT  tmprec,mtrec;
uint		  i=0,totalrecs=0,ver;

  tmp.setName(f.getName());
  tmp.setExt(".TMP");
  memset(&mtrec,0,sizeof(mtrec));
  totalrecs = totRecs();
  if(tmp.open(OPEN_RDWR | OPEN_DENYWRIT) != 0) return;
  ver = getVersion();
  tmp.write(&ver,sizeof(ver));
  if(totalrecs == 0)
  {
	f.seek(2,SEEK_SET);
	f.write(&rec,sizeof(rec));
  }
  else
  {
	for(i=1;i<=totalrecs;i++)
	{
	  tmprec = getRec(i);
	  if(memcmp(tmprec.AreaTag,rec.AreaTag,sizeof(tmprec.AreaTag)) > 0 )
		tmp.write(&rec,sizeof(rec));

	  if(memcmp(&mtrec,&tmprec,sizeof(mtrec)) != 0)
		tmp.write(&tmprec,sizeof(tmprec));
	}
  }
  f.close();
  f.unlink();
  tmp.close();
  tmp.rename(f.getName());
  if(f.open(OPEN_RDWR | OPEN_DENYNONE) != 0) Err(TRUE);
}

unsigned int  cAREAS::findRec(const char * area)
{
  NAREA_STRUCT	rec;
  unsigned int currec =0;
  unsigned int hirec  =totRecs();
  unsigned int lorec  =1;
		   int cmp	  =0;

  while(lorec <= hirec)
  {
	currec = (hirec+lorec)/2U;
	rec = getRec(currec);
	cmp = strcmpi(rec.AreaTag,area);

	if(cmp < 0) 	   lorec  = currec+1;
	else if(cmp == 0)  return currec;
	else			   hirec  = currec-1;
  }

  if(cmp == 0) return currec;

  return 0;
}

void cAREAS::updateLastActiveDate(const char * areaname)
{

NAREA_STRUCT area;
unsigned int areanum = findRec(areaname);

	  area = getRec(areanum);
	  area.LastActivityDate = getjuliandate();
	  modifyRec(area,areanum);
}

#ifdef PCBSETUP
bool cAREAS::sortFile(void)
{
  NAREA_STRUCT * rec;
  unsigned int numrecs=0,i=0;
  VMDataSet  recs;
  //NAREA_STRUCT buffer[64];
  NAREA_STRUCT buffer[64];

	numrecs=totRecs();

	VMInitRec(&recs,NULL,0,sizeof(NAREA_STRUCT));
	for(i=1;i<=numrecs;i++)
	{
		rec =(NAREA_STRUCT *) VMRecordCreate(&recs,sizeof(NAREA_STRUCT),NULL,NULL);
		*rec = getRec(i);
	}

	VMSort(&recs,sizeof(NAREA_STRUCT),1L,(long)numrecs,VM_FALSE,(VMCompareFunc*)areacmp,(VMSortFunc*)qsort,&buffer,sizeof(buffer));
	removeFile();
	open();


	for(i=1;i<=numrecs;i++)
	{
		rec = (NAREA_STRUCT *)VMRecordGetByIndex(&recs,i,NULL);
		if(rec->AreaTag[0] != '\x0') addRec(*rec);
	}

	VMDone(&recs);

	updateIndex();
	return TRUE;
}



void cAREAS::updateAkaIndex(void)
{
cAKAS		 akas;
NADDRESS	 aka;
cAREAS		 areas;
NAREA_STRUCT area;
unsigned int totrecs = akas.totRecs();
unsigned int i,j;
unsigned int lo=0,hi=0;
char	  *  bptr=NULL;
char		 wgs[15];
char		 node[25];

	  memset(wgs,0,sizeof(wgs));
	  fastcenter(1,"Processing AKA conference ranges. Please wait...",0x0C);
	  // For each fido aka
	  for(i=1;i<=totrecs;i++)
	  {

		aka  = akas.getRec(i);
		bptr = aka.range;

		if(aka.point == 0) sprintf(node,"%u:%u/%u",aka.zone,aka.net,aka.node);
		else			   sprintf(node,"%u:%u/%u.%u",aka.zone,aka.net,aka.node,aka.point);

		fastcenter(2,"                                            ",0x0B);
		fastcenter(2,node,0x0B);

		// While I'm not at the end of the range field for this aka record
		// Fange field looks like  100-200 34 60 500-600
		while(*bptr != '\x0')
		{
		  // Skip any spaces
		  while(!isdigit(*bptr) && *bptr != '\x0') bptr++;

		  // Get the lo number
		  lo = atoi(bptr);

		  // Skip past numbers and go to next delimiter
		  while(*bptr != '-' && *bptr != ' ' && *bptr != '\x0') bptr++;

		  // if it is a range, get the hi number of the range
		  if(*bptr == '-')
		  {
			hi = atoi(++bptr);
			while(isdigit(*bptr) && *bptr != '\x0') bptr++;
		  }
		  // if it was just a single number, set hinum = lonum
		  else if(*bptr == ' ' || *bptr == '\x0') hi = lo;

		  // Now get area records in this range and set their aka index to this aka
		  for(j=lo;j<=hi;j++)
		  {

			sprintf(wgs,"%u",j);
			fastprint(65,1,wgs,0x0B);
			area = getAreaRec(j);
			if(area.ConfNum == 0 || area.AkaIndex == i)
			  continue;
			area.AkaIndex = i;
			modifyRec(area,findRec(area.AreaTag));
		  }
		}
	  }
}

void cAREAS::updateOriginsIndex(void)
{
cORIGINS	 origins;
ORIGIN		 origin;
cAREAS		 areas;
NAREA_STRUCT area;
unsigned int totrecs = origins.totRecs();
unsigned int i,j;
unsigned int lo=0,hi=0;
char	  *  bptr=NULL;
char		 wgs[15];

	  memset(wgs,0,sizeof(wgs));


	  fastcenter(1,"Processing AKA conference ranges. Please wait...",0x0C);
	  // For each fido origin
	  for(i=1;i<=totrecs;i++)
	  {
		origin	= origins.getRec(i);
		bptr = origin.ranges;

		fastcenter(2,"                                                                      ",0x0B);
		fastcenter(2,origin.origin,0x0B);
		// While I'm not at the end of the range field for this origin record
		// Fange field looks like  100-200 34 60 500-600
		while(*bptr != '\x0')
		{
		  // Skip any spaces
		  while(!isdigit(*bptr) && *bptr != '\x0') bptr++;

		  // Get the lo number
		  lo = atoi(bptr);

		  // Skip past numbers and go to next delimiter
		  while(*bptr != '-' && *bptr != ' ' && *bptr != '\x0') bptr++;

		  // if it is a range, get the hi number of the range
		  if(*bptr == '-')
		  {
			hi = atoi(++bptr);
			while(isdigit(*bptr) && *bptr != '\x0') bptr++;
		  }
		  // if it was just a single number, set hinum = lonum
		  else if(*bptr == ' ' || *bptr == '\x0') hi = lo;

		  // Now get area records in this range and set their origin index to this origin
		  for(j=lo;j<=hi;j++)
		  {
			sprintf(wgs,"%u",j);
			fastprint(65,1,wgs,0x0B);
			area = getAreaRec(j);
			if(area.ConfNum == 0 || area.OriginIndex == i)
			  continue;
			area.OriginIndex = i;
			modifyRec(area,findRec(area.AreaTag));
		  }
		}
	  }
}
#endif

short areacmp(const void * a, const void * b)
{
	return memcmp(((NAREA_STRUCT*)a)->AreaTag,((NAREA_STRUCT*)b)->AreaTag,sizeof(((NAREA_STRUCT*)a)->AreaTag));
}


bool cAREAS::updateIndex(void)
{
	AINDEX		 irec;
	AINDEX	   * ircs;
	NAREA_STRUCT arec;
	unsigned int i=1;
	unsigned int trecs=0;

	trecs = totRecs();

	#ifndef __OS2__
	long memleft = coreleft();
	if(memleft < trecs*sizeof(AINDEX) )
	{
	  irecsinmem = FALSE;
	  return	   FALSE;
	}
	else
	#endif
	{
		ircs	   = new AINDEX[trecs];
		irecsinmem = FALSE;
		if(!ircs)
		{
		  return FALSE;
		}
	}

	memset(ircs,0,trecs*sizeof(AINDEX));


	index.removeFile();

	for(i=1;i<=trecs;i++)
	{
		arec	   = getRec(i);
		irec.conf  = arec.ConfNum;
		irec.recno = i;
		if(irec.recno != 0) memcpy((void *)&ircs[i-1],(const void *)&irec,sizeof(AINDEX));
	}

	qsort(ircs,size_t(trecs),sizeof(AINDEX),idxcmp);
	for(i=0;i<trecs;i++)
	  index.addRec(ircs[i]);

	delete(ircs);
	return TRUE;
}

int idxcmp(const void * r1, const void * r2)
{
	return (((AINDEX*)r1)->conf-((AINDEX*)r2)->conf);
}

NAREA_STRUCT &	cAREAS::getAreaRec(unsigned int conf)
{

  static _FAR_ NAREA_STRUCT rec;
  AINDEX	   irec;



  // get index record for this conference number
  if(index.totRecs() != totRecs()) updateIndex();
  irec = index.findIRec(conf);

  if(irec.recno != 0)
  {
	rec = getRec(irec.recno);
	return rec;
  }

  memset(&rec,0,sizeof(rec));
  return rec;

}

char * cAREAS::getOrigin(unsigned int conf)
{
  NAREA_STRUCT rec;
  AINDEX	   irec;
  static _FAR_ char  origin[70];
  cORIGINS	   org;
  ORIGIN	   o;

  // get index record for this conference number
  irec = index.getRec(conf);

  // Get area record associated with this conference number
  rec = getRec(irec.recno);


  if(rec.OriginIndex !=0)
  {
	 o = org.getRec(rec.OriginIndex);
	 maxstrcpy(origin,o.origin,sizeof(origin));
  }
  else
	 memset(origin,0,sizeof(origin));

  return origin;
}

char * cAREAS::getAKA(unsigned int conf)
{
  NAREA_STRUCT rec;
  AINDEX	   irec;
  static _FAR_ char  aka[25];
  cAKAS 	   akas;

  // get index record for this conference number
  irec = index.getRec(conf);

  // Get area record associated with this conference number
  rec = getRec(irec.recno);


  if(rec.AkaIndex !=0)
  {
	NADDRESS arec;
	 arec = akas.getRec(rec.AkaIndex);
	 if(arec.point == 0) sprintf(aka,"%u:%u/%u",arec.zone,arec.net,arec.node);
	 else				sprintf(aka,"%u:%u/%u.%u",arec.zone,arec.net,arec.node,arec.point);
  }
  else
	 memset(aka,0,sizeof(aka));

  return aka;
}


/*****************************************************************************
* Member functions for the cAREAINDEX class. This class maintains data associated
* with the FIDO area tag index file
*
******************************************************************************/

AINDEX & cAREAINDEX::findIRec(unsigned int conf)
{
uint	 trecs=totRecs();
uint	 i=0;
static _FAR_  AINDEX   irec;


	memset(&irec,0,sizeof(irec));
	for(i=1;i<=trecs;i++)
	{
	  irec = getRec(i);
	  if(irec.conf == conf) return irec;
	}
	memset(&irec,0,sizeof(irec));
	return irec;
}

/*****************************************************************************
* Member functions for the cORIGINS class. This class maintains data associated
* with the FIDO origin lines for conferences
*
******************************************************************************/

unsigned int cORIGINS::findRec(const char * orig)
{
  unsigned int i=0,trecs=0;
  ORIGIN  O;

  trecs = totRecs();

  for(i=0;i<trecs;i++)
  {
	O = getRec(i);
	if(strcmp(O.origin,orig) == 0) return i;
  }
  return 0;
}

/*****************************************************************************
* Member functions for the cAKAS class. This class maintains data associated
* with FIDO AKAs
*
******************************************************************************/
unsigned int cAKAS::findRec(const char * aka)
{
  NADDRESS		 rec;
  unsigned int	 i=0;
  unsigned int	 trecs=0;
  uint	 zone=0,net=0,node=0,point=0;

  addrToNum((char *)aka,zone,net,node,point);
  trecs = totRecs();

  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(rec.zone == zone && rec.net == net && rec.node == node && rec.point == point)
	  return(i);   // didn't used to return i here DWT
  }
	return 0;
}

#ifndef PCBFU
/*****************************************************************************
* Member functions for the cXLATPHONE class. This class maintains data associated
* with the FIDO phone number translation table
*
******************************************************************************/
unsigned int cXLATEPHONE::findRec(const char * in)
{
  TRANSLATE 	 rec;
  unsigned int	 i=0;
  unsigned int	 trecs=0;

  trecs = totRecs();

  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(strcmp(rec.in,in) == 0) return i;
  }
	return 0;
}


bool cXLATEPHONE::findAndReplace(char * phonenum)
{

//bool LIBENTRY translate_phone(char *phone)
//{

int 		len=0,flen=0;
char		temp[100];
char		temp1[100];
char		*chPtr1=NULL,*chPtr2=NULL;
TRANSLATE	rec;
uint		i=0,trecs=0;


  memset(temp,0,sizeof(temp));
  memset(temp1,0,sizeof(temp1));

  trecs = totRecs();

  processPreSuffix(phonenum);
  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if((chPtr1 = strstr(phonenum,rec.in))!=NULL)
	{
	  len=strlen(rec.in);
	  flen=strlen(phonenum);
	  chPtr2 = chPtr1 + len;

	  if(chPtr1==phonenum)
	  {
		memcpy(temp,chPtr2,flen-len);
		sprintf(phonenum,"%s%s",rec.out,temp);
		return(TRUE);
	  }
	  else if(chPtr2==phonenum+flen)
	  {
		memcpy(temp,phonenum,flen-len);
		sprintf(phonenum,"%s%s",temp,rec.out);
		return(TRUE);
	  }
	  else
	  {
		memcpy(temp,phonenum,int(chPtr1-phonenum));
		memcpy(temp1,chPtr2,strlen(chPtr2));
		sprintf(phonenum,"%s%s%s",temp,rec.out,temp1);
		return(TRUE);
	  }
	}
  }
  return(FALSE);
}

void cXLATEPHONE::processPreSuffix(char * phone)
{
  char tmpBuf[100];
  bool GPDone = FALSE;
  bool GSDone = FALSE;
  bool IPDone = FALSE;
  uint i=0,j=0,trecs=0;
  TRANSLATE rec,recj;

  trecs = totRecs();

	// Cycle through translation table
	for(i=1;i<=trecs;i++)
	{
	  rec = getRec(i);
	  // If a global prefix hasn't been processed yet, process it
	  if(!GPDone && strstr("GP",rec.in))
	  {
		sprintf(tmpBuf,"%s%s",rec.out,phone);
		strcpy(phone,tmpBuf);
		GPDone = TRUE;
	  }
	  // If a global suffix hasn't been processed yet, process it
	  if(!GSDone && strstr("GS",rec.in))
	  {
		strcat(phone,rec.out);
		GSDone = TRUE;
	  }
	  // If an international prefix hasn't been processed yet, process it
	  if(!IPDone && strcmp("IPX",rec.in)==0)
	  {
		if(memcmp(rec.out,phone,strlen(rec.out)))
		{
		   for(j=1;j<=trecs;j++)
		   {
			 recj = getRec(j);
			 if(strcmp(recj.in,"IP")==0)
			 {
				sprintf(tmpBuf,"%s%s",recj.out,phone);
				strcpy(phone,tmpBuf);
				break;
			 }
		   }
		}
	  }

	}
}



/*****************************************************************************
* Member functions for the cNODEARC class. This class maintains data associated
* with the FIDO node specific archiver information. eg which archiver to use when
* archiving mail for a certian node
******************************************************************************/
unsigned int cNODEARC::findRec(const char * addr)
{

  NNODE_T		 rec;
  unsigned int	 i=0;
  unsigned int	 trecs=0;
  uint	 zone=0,net=0,node=0,point=0;

  addrToNum((char *)addr,zone,net,node,point);
  trecs = totRecs();

  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(rec.zone == zone && rec.net == net && rec.node == node && rec.point == point)
	  return(i);   // didn't used to return i here  DWT
  }
	return 0;
}

/*****************************************************************************
* Member functions for the cNODELISTS class. This class maintains data associated
* with FIDO nodelists
*
******************************************************************************/
unsigned int cNODELISTS::findRec(const char * nodelist)
{
  NODELIST		 rec;
  unsigned int	 i=0;
  unsigned int	 trecs=0;

  trecs = totRecs();

  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(strcmp(rec.basename,nodelist) == 0) return i;
  }
	return 0;
}


/*****************************************************************************
* Member functions for the cFREQPATHS class. This class maintains data associated
* with FIDO file requests paths
*
******************************************************************************/
unsigned int cFREQPATHS::findRec(const char * path)
{
  NFREQ_PATH	 rec;
  unsigned int	 i=0;
  unsigned int	 trecs=0;

  trecs = totRecs();

  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(strcmp(rec.Path,path) == 0) return i;
  }
	return 0;
}

/*****************************************************************************
* Member functions for the cMAGICNAMES class. This class maintains data associated
* with FIDO magic names list
*
******************************************************************************/
unsigned int cMAGICNAMES::findRec(const char * magicname)
{
  unsigned	 int i = 0;
  unsigned	 trecs=0;
  NFREQ_MAGIC rec;

  trecs = totRecs();
  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(strcmp(rec.MagicName,magicname) == 0) return i;
  }
  return 0;
}

bool cMAGICNAMES::checkPassword(unsigned int recno, const char * password)
{
	NFREQ_MAGIC  rec;

	rec = getRec(recno);
	if(strcmp(rec.Password,password) == 0) return TRUE;
	else return FALSE;
}


#endif //ifndef PCBFU
/*****************************************************************************
* Member functions for the cFREQDENY class. This class maintains data associated
* with FIDO file request deny list
*
******************************************************************************/
unsigned int cFREQDENY::findRec(const char * addr)
{
  NADDRESS			rec;
  unsigned int		i=0;
  unsigned trecs   =0;
  uint		 zone=0,net=0,node=0,point=0;

  addrToNum((char *)addr,zone,net,node,point);

  trecs = totRecs();

  for(i=1;i<=trecs;i++)
  {
	rec = getRec(i);
	if(rec.zone == zone && rec.net == net && rec.node == node && rec.point == point)
	  return i;
  }
	return 0;
}

/*****************************************************************************
* Member functions for the cSEND class. This class maintains data associated
* with FIDO SEND verbs.
*
******************************************************************************/

#ifndef PCBSETUP
unsigned int cSEND::findRec(const char * entry)
{
FSEND			   rec;
unsigned int	  trecs = totRecs();
unsigned int	  i;

  for(i=1;i<=trecs;i++)
  {
	 rec = getRec(i);
	 if(strcmp(rec.send,entry) == 0) return i;
  }
  return 0;
}

void cSEND::process(void)
{
cDOSFILE		  sts;
FSEND			  rec;
char			  line[80];
char	  * 	  bp = NULL;
unsigned int	  trecs = totRecs();
unsigned int	  recnum=0;
unsigned int	  i;

  // Open STS file
  if(fileexist("PCBFIDO.STS") == 255 || sts.open("PCBFIDO.STS",OPEN_READ | OPEN_DENYNONE) != 0) return;
  while( sts.getln(line,sizeof(line))  != 0)
  {
	 if(memcmp(line,"SEND",4) == 0)
	 {
	   bp = line+5;

	   // Skip any commas or spaces
	   while(*bp == ',' || *bp == ' ') ++bp;
	   stripright(bp,' ');

	   recnum = findRec(bp);

	   if(recnum != 0)
	   {
		 rec = getRec(recnum);
		 rec.hit = TRUE;
		 modifyRec(rec,recnum);
	   }
	 }
  }
  sts.close();

  // Now look for any records that were not hit and remove them
  // If it was hit, clear the hit flag.
  for(i=1;i<=trecs;i++)
  {
	 rec = getRec(i);
	 if(!rec.hit) removeRec(i);
	 else
	 {
	   rec.hit = FALSE;
	   modifyRec(rec,i);
	 }
  }
  if(totRecs() == 0) removeFile();

}
#endif //ifndef pcbsetup


/*****************************************************************************
* Member functions for the cCONFIG class. This class maintains data associated
* with FIDO configuration data that is fixed length.
*
******************************************************************************/

cCONFIG::cCONFIG(void)
{
	// generic initializations
	openedbyme = FALSE;
	err 	   = FALSE;
	filever    = 0;
	curver	   = 3;
	diroffset  =2;
	emsioffset =diroffset  + sizeof(DIRECTORIES);
	freqoffset =emsioffset + sizeof(EMSI_DATA);
	archoffset =freqoffset + sizeof(FREQ_INFO);

	// Init datafile name
	maxstrcpy(datafile,PcbData.FidoLoc,sizeof(datafile));
	char * t = findstartofname(datafile);
	if(t) *t = '\x0';
	strcat(datafile,"PCBFIDO.CFG");

	if(fileexist(datafile) == 255) create();
	// Now attempt to open the file if it is not already open
	if(!f.isOpen() && fileexist(datafile) != 255 && f.open(datafile,OPEN_RDWR |OPEN_DENYNONE) == 0)
	{
	  if(!f.isOpen())
		err = TRUE;
	  else
		openedbyme = TRUE;
	}
	else
	  err = TRUE;

	// Check the datfile's version number and compare against current version number

	if(f.isOpen() && f.read(&filever,sizeof(filever)) != sizeof(filever)) err = TRUE;

	if( !err && filever != curver)
	{
		 #ifndef PCBSETUP
		 writeFidolog("Wrong config file version. Run conversion utility.",BLOCK);
		 #endif
		 err = TRUE;
	}
}


bool cCONFIG::getDirs(DIRECTORIES & dirs)
{
	if(err) return FALSE;
	f.seek(diroffset,SEEK_SET);
	if(f.isOpen() && f.read(&dirs,sizeof(DIRECTORIES)) != sizeof(DIRECTORIES)) err = TRUE;
	return TRUE;
}
bool cCONFIG::getEMSI(EMSI_DATA & edata)
{
	if(err) return FALSE;
	f.seek(emsioffset,SEEK_SET);
	return(f.read(&edata,sizeof(edata)) == sizeof(edata));
}
bool cCONFIG::getFREQInfo(FREQ_INFO & finfo)
{
	if(err) return FALSE;
	f.seek(freqoffset,SEEK_SET);
	return(f.read(&finfo,sizeof(finfo)) == sizeof(finfo));
}
bool cCONFIG::getArchInfo(ARCHIVERS & archinfo)
{
	if(err) return FALSE;
	f.seek(archoffset,SEEK_SET);
	return(f.read(&archinfo,sizeof(archinfo)) == sizeof(archinfo));
}
bool cCONFIG::putDirs(DIRECTORIES & dirs)
{
	f.seek(diroffset,SEEK_SET);
	return(f.write(&dirs,sizeof(dirs)) == sizeof(dirs));
}
bool cCONFIG::putEMSI(EMSI_DATA & edata)
{
	f.seek(emsioffset,SEEK_SET);
	return(f.write(&edata,sizeof(edata)) == sizeof(edata));
}
bool cCONFIG::putFREQInfo(FREQ_INFO & finfo)
{
	f.seek(freqoffset,SEEK_SET);
	return(f.write(&finfo,sizeof(finfo)) == sizeof(finfo));
}
bool cCONFIG::putArchInfo(ARCHIVERS & archinfo)
{
	f.seek(archoffset,SEEK_SET);
	return(f.write(&archinfo,sizeof(archinfo)) == sizeof(archinfo));
}

void cCONFIG::create(void)
{

	if(fileexist(datafile) != 255) return;

	if(f.isOpen()) f.close();
	if(f.open(datafile,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE) != 0) return;
	f.write(&curver,sizeof(curver));
	f.close();

}

#ifdef PCBSETUP
void LIBENTRY addrToNum(const char * addr, uint & zone,
						uint & net, uint & node,uint & point)
{
char	 tBuf[25];
char * bPtr = NULL;

	// Initializations
	zone = net = node = point = 0;
	maxstrcpy(tBuf,(char *)addr,sizeof(tBuf));

	// grab ZONE
	bPtr = strrchr(tBuf,'~');
	if(bPtr == NULL) return;
	zone = atoi(++bPtr);

	// grab NET
	bPtr = strchr(tBuf,':');
	if(bPtr == NULL) return;
	net = atoi(++bPtr);

	// Grab ZONE
	bPtr = strchr(tBuf,'/');
	if(bPtr == NULL) return;
	node = atoi(++bPtr);

	// grab POINT
	bPtr = strchr(tBuf,'.');
	if(bPtr == NULL) return;
	point = atoi(++bPtr);
}

#endif // pcbsetup
#endif
