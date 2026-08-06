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
* CONVERT.CPP
* Written by:Stan Paulsen
* Copyright Clark Development 1995
* This is a utility to convert the file PCBFIDO.CFG into component parts
* March 1995: Convert from version 2 to version 3
*****************************************************************************/
#include <iostream.h>
#include <string.h>
// #include <conio.h>
#include <stdio.h>

#include <types.hpp>
#include <pcb.h>
// #include <NEWDATA.H> -- included via pcb.h

#include <structs.h>
#include <data.hpp>

#include <convert.hpp>
#include <screen.h>

extern "C" int _Cdecl getch( void );

// Prototypes
void  static near pascal Areas(void);
unsigned int  static near pascal addOrigin(const char * origin);
void  static near pascal ArchiverInfo(void);
void  static near pascal AKAs(void);
void  static near pascal ArchiverNodes(void);
void  static near pascal PhoneTranslation(void);
void  static near pascal Nodelists(void);
void  static near pascal FREQPath(void);
void  static near pascal FREQInfo(void);
void  static near pascal MagicNames(void);
void  static near pascal DenyList(void);
void  static near pascal TMPtoCFG(void);
void  static near pascal FidoQueue(void);

void do_pack(void);

//#define bool unsigned char
//#define FALSE 0
//#define TRUE  1

const unsigned int oldver = 2;
const unsigned int newver = 3;
          unsigned int realver = 0;

extern pcbdattype PcbData;
char basepath[MAXDIR];

FILE *  oldfile;
#ifdef TEST
main(void)
#else
void pascal convert(void)
#endif
{
char  filename[MAXDIR];
char  oldfilename[MAXDIR];
char  buf[2048];
char *nullit=NULL;
char  res;
unsigned int bytesread=0;
FILE *backup;

  cls();
  printf("\n              PCBoard FIDO file conversion utility");
  printf("\n              Copyright Clark Development 1996");

  cout << "Note: No conversion is needed from 15.22 to 15.23." << endl;
  cout << endl<<"Convert 15.21 data to 15.23 format? (Y/N) " << endl;
  cin >> res;
  if(res != 'y' && res != 'Y') return;

  maxstrcpy(filename,PcbData.FidoConfig,sizeof(filename));
  maxstrcpy(basepath,PcbData.FidoConfig,sizeof(basepath));


  nullit = findstartofname(basepath);
  if(nullit) *nullit = '\x0';
  printf("\nBase Path is %s",basepath);
  maxstrcpy(oldfilename,PcbData.FidoConfig,sizeof(oldfilename));
  nullit = strrchr(oldfilename,'.');
  if(nullit) *nullit = '\x0';
  strcat(oldfilename,".old");

  oldfile = fopen(filename,"rb");
  if(!oldfile)
  {
        printf("\nCould not open PCBFIDO.CFG");
        return;
  }
  fread(&realver,sizeof(oldver),1,oldfile);
  if(realver != oldver)
  {
        printf("\nPCBFIDO.CFG is wrong version. Cannot convert with this utility.");
        //getch();

        return;
  }

  printf("\nBacking up PCBFIDO.CFG to PCBFIDO.OLD");
  backup = fopen(oldfilename,"wb");
  fseek(oldfile,0,SEEK_SET);
  while( (bytesread = fread(buf,sizeof(buf),1,oldfile)) != 0)
           fwrite(buf,bytesread==1 ?sizeof(buf):bytesread,1,backup);
  fclose(backup);
  fclose(oldfile);
  //do_pack();

  oldfile = fopen(filename,"rb");
  if(!oldfile)
  {
        printf("\nCould not open PCBFIDO.CFG");
        return;
  }
  rewind(oldfile);
  fseek(oldfile,2,SEEK_SET);
  Areas();
  ArchiverInfo();
  AKAs();
  ArchiverNodes();
  PhoneTranslation();
  Nodelists();
  FREQPath();
  FREQInfo();
  MagicNames();
  DenyList();
  TMPtoCFG();
  FidoQueue();
  printf("\nConversion complete.");
  fclose(oldfile);
  printf("\nFinished. Press any key.");
  getch();
  return;
}

void static near pascal Areas(void)
{
  cDOSFILE      areafile;
  unsigned int numareas = 0,i=0;
  char fname[MAXDIR];
  AREA_STRUCT  area;
  NAREA_STRUCT narea;
  pcbconftype  confrec;

  loadcnames(FALSE);
        printf("\nMoving FIDO Area Tags from PCBFIDO.CFG to ");
        sprintf(fname,"%sAREAS.DAT",PcbData.FidoLoc);
        printf(fname);
        if(areafile.open(fname,OPEN_RDWR|OPEN_CREATE) != 0)
        {
          printf("Could not open AREAS.DAT");
          return;
        }
        areafile.write(&newver,sizeof(newver));
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&area,0,sizeof(area));
                memset(&narea,0,sizeof(narea));
                fread(&area,sizeof(area),1,oldfile);
                getconfrecord(area.PCB_Conference,&confrec);
//                if(confrec.Name[0] == ' ' || confrec.Name[0] == '\x0' || confrec.ConfType != 5)
//                {
//                       printf("\nPacking out conference %d. No FIDO info for this conference.",area.PCB_Conference);
//                       continue;
//                }

                narea.ConfNum = area.PCB_Conference;
                strupr(area.Area_Name);
                maxstrcpy(narea.AreaTag,area.Area_Name,sizeof(narea.AreaTag));
                narea.HighAscii = area.HighAscii;
                narea.LastActivityDate = getjuliandate();
                if(narea.ConfNum == 0 || narea.AreaTag[0] == '\x0')
                  continue;
                areafile.write(&narea,sizeof(narea));
                addOrigin(area.origin);
        }
        areafile.close();
        {
          cAREAS areas;
          areas.sortFile();
        }
        closecnames();
}

unsigned int static near pascal addOrigin(const char * origin)
{
  ORIGIN rec;
  cORIGINS origins;
  unsigned int recno = 0;

  maxstrcpy(rec.origin,(char*)origin,sizeof(rec.origin));

  recno = origins.findRec(rec.origin);

  if(recno == 0) return 0;
  else                  origins.addRec(rec);
  return origins.findRec(rec.origin);
}
void static near pascal ArchiverInfo(void)
{
  ARCHIVERS rec;
  DIRECTORIES dirs;
  EMSI_DATA  emsidata;
  FILE * afile;

  char   fname[MAXDIR];
        sprintf(fname,"%sPCBFIDO.TMP",basepath);
        memset(&rec,0,sizeof(rec));
        memset(&dirs,0,sizeof(dirs));
        memset(&emsidata,0,sizeof(emsidata));


        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("\nCould not open PCBFIDO.TMP");
          return;
        }
        fread(&rec,sizeof(rec),1,oldfile);
        printf("\nMoving FIDO Directory data from PCBFIDO.CFG to PCBFIDO.TMP.");
        fread(&dirs,sizeof(OLDDIRECTORIES),1,oldfile);
        fwrite(&dirs,sizeof(dirs),1,afile);
        printf("\nMoving FIDO EMSI profile data from PCBFIDO.CFG to PCBFIDO.TMP.");
        fread(&emsidata,sizeof(emsidata),1,oldfile);
        fwrite(&emsidata,sizeof(emsidata),1,afile);
        printf("\nMoving FIDO archiver data from PCBFIDO.CFG to PCBFIDO.TMP.");
        fwrite(&rec,sizeof(rec),1,afile);
        fclose(afile);
}
void static near pascal ArchiverNodes(void)
{

  FILE * afile;
  unsigned int numareas = 0,i=0;
  NODE_T  rec;
  NNODE_T nrec;

  char   fname[MAXDIR];
        sprintf(fname,"%sNODEARC.DAT",PcbData.FidoLoc);
        printf("\nMoving FIDO node specific archiver info from PCBFIDO.CFG to NODEARC.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("\n Could not open NODEARC.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                memset(&nrec,0,sizeof(nrec));
                fread(&rec,sizeof(rec),1,oldfile);
                nrec.zone = rec.zone;
                nrec.net = rec.net;
                nrec.node = rec.node;
                nrec.point = rec.point;
                nrec.pkt_type = rec.pkt_type;
                nrec.archiver_index = rec.archiver_index;

                fwrite(&nrec,sizeof(nrec),1,afile);
        }
        fclose(afile);
}


void static near pascal AKAs(void)
{

  FILE * afile;
  unsigned int numareas = 0,i=0;
  THIS_ADDRESS  rec;
  NADDRESS              nrec;

  char   fname[MAXDIR];
        sprintf(fname,"%sAKAS.DAT",PcbData.FidoLoc);
        printf("\nMoving systems AKAs from PCBFIDO.CFG to AKAS.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("Could not open AKAS.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                memset(&nrec,0,sizeof(nrec));
                fread(&rec,sizeof(rec),1,oldfile);
                nrec.zone          = rec.this_zone;
                nrec.net           = rec.this_net;
                nrec.node          = rec.this_node;
                nrec.point         = rec.this_point;
                nrec.primary   = FALSE;
                nrec.inseenby  = TRUE;
                nrec.present   = TRUE;
                nrec.UpOrDownLink = 'U';

                fwrite(&nrec,sizeof(nrec),1,afile);
        }
        fclose(afile);
}


void static near pascal  PhoneTranslation(void)
{

  FILE * afile;
  unsigned int numareas = 0,i=0;
  TRANSLATE  rec;

  char   fname[MAXDIR];
        sprintf(fname,"%sPHONEX.DAT",PcbData.FidoLoc);
        printf("\nMoving phone number translation data from PCBFIDO.CFG to PHONEX.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("Could not open PHONEX.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                fread(&rec,sizeof(rec),1,oldfile);
                fwrite(&rec,sizeof(rec),1,afile);
        }
        fclose(afile);
}

void static near pascal  Nodelists(void)
{

  FILE * afile;
  unsigned int numareas = 0,i=0;
  NODELIST      rec;

  char   fname[MAXDIR];
        sprintf(fname,"%sNODELIST.DAT",PcbData.FidoLoc);
        printf("\nMoving Nodelist data from PCBFIDO.CFG to NODELIST.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("\nCould not open NODELIST.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                fread(&rec,sizeof(rec),1,oldfile);
                fwrite(&rec,sizeof(rec),1,afile);
        }
        fclose(afile);
}
void static near pascal  FREQPath(void)
{

  FILE * afile;
  unsigned int numareas = 0,i=0;
  NFREQ_PATH  nrec;
  FREQ_PATH   rec;

  char   fname[MAXDIR];
        sprintf(fname,"%sFREQPATHS.DAT",PcbData.FidoLoc);
        printf("\nMoving FREQ Path data from PCBFIDO.CFG to NODELIST.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("Could not open FREQPATHS.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                memset(&nrec,0,sizeof(nrec));
                fread(&rec,sizeof(rec),1,oldfile);
                memcpy(&nrec,&rec,sizeof(rec.str));
                fwrite(&nrec,sizeof(nrec),1,afile);
        }
        fclose(afile);
}


void static near pascal  FREQInfo(void)
{

  FILE * afile;
  FREQ_INFO  rec;

  char   fname[MAXDIR];
        sprintf(fname,"%sPCBFIDO.TMP",basepath);
        printf("\nMoving FREQ Limits data from PCBFIDO.CFG to PCBFIDO.TMP.");

        afile = fopen(fname,"a+b");
        if(!afile)
        {
          printf("\nCould not open PCBFIDO.TMP");
          return;
        }
        memset(&rec,0,sizeof(rec));
        fread(&rec,sizeof(rec),1,oldfile);
        fwrite(&rec,sizeof(rec),1,afile);
        fclose(afile);
}


void static near pascal  MagicNames(void)
{
  FILE * afile;
  unsigned int numareas = 0,i=0;
  FREQ_MAGIC   rec;
  NFREQ_MAGIC  nrec;

  char   fname[MAXDIR];
        sprintf(fname,"%sMAGICNAM.DAT",PcbData.FidoLoc);
        printf("\nMoving FREQ Magic Name date from PCBFIDO.CFG to MAGICNAM.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("\nCould not open MAGICNAM.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                memset(&nrec,0,sizeof(nrec));
                fread(&rec,sizeof(rec),1,oldfile);
                maxstrcpy(nrec.MagicName,rec.magic_name,sizeof(nrec.MagicName));
                maxstrcpy(nrec.RealName,rec.real_name,sizeof(nrec.RealName));
                fwrite(&nrec,sizeof(nrec),1,afile);
        }
        fclose(afile);
}

void static near pascal  DenyList(void)
{
  FILE * afile;
  unsigned int numareas = 0,i=0;
  THIS_ADDRESS   rec;
  NADDRESS       nrec;

  char   fname[MAXDIR];
        sprintf(fname,"%sFREQDENY.DAT",PcbData.FidoLoc);
        printf("\nMoving FREQ Deny List data from PCBFIDO.CFG to FREQDENY.DAT.");

        afile = fopen(fname,"w+b");
        if(!afile)
        {
          printf("\nCould not open FREQDENY.DAT");
          return;
        }
        fwrite(&newver,sizeof(newver),1,afile);
        fread(&numareas,sizeof(numareas),1,oldfile);
        for(i=0;i<numareas;i++)
        {
                memset(&rec,0,sizeof(rec));
                memset(&nrec,0,sizeof(nrec));
                fread(&rec,sizeof(rec),1,oldfile);
                nrec.zone = rec.this_zone;
                nrec.net = rec.this_net;
                nrec.node = rec.this_node;
                nrec.point = rec.this_point;
                fwrite(&nrec,sizeof(nrec),1,afile);
        }
        fclose(afile);
}

void static near pascal TMPtoCFG(void)
{

  ARCHIVERS rec;
  DIRECTORIES dirs;
  EMSI_DATA  emsidata;
  FREQ_INFO  frq;
  FILE * afile;
  char   mfname[MAXDIR];

  char   fname[MAXDIR],nfname[MAXDIR];
        sprintf(fname,"%sPCBFIDO.CFG",basepath);
        sprintf(nfname,"%sPCBFIDO.TMP",basepath);
        memset(&rec,0,sizeof(rec));
        memset(&dirs,0,sizeof(dirs));
        memset(&emsidata,0,sizeof(emsidata));

        printf("\nSorting PCBFIDO.CFG");
        fclose(oldfile);
        oldfile = fopen(fname,"wb");
        if(!oldfile)
        {
          printf("\nCould not open PCBFIDO.CFG");
          return;
        }
        fseek(oldfile,SEEK_SET,0);
        fwrite(0,0,1,oldfile);
        fseek(oldfile,SEEK_SET,0);
        fwrite(&newver,sizeof(newver),1,oldfile);
        afile = fopen(nfname,"rb");

        if(!afile)
        {
          printf("\nCould not open PCBFIDO.TMP");
          return;
        }
//        fseek(afile,SEEK_SET,2);
        fread(&dirs,sizeof(dirs),1,afile);
        fread(&emsidata,sizeof(emsidata),1,afile);
        fread(&rec,sizeof(rec),1,afile);
        fread(&frq,sizeof(frq),1,afile);
        fseek(oldfile,2,SEEK_SET);
        fwrite(&dirs,sizeof(dirs),1,oldfile);
        fwrite(&emsidata,sizeof(emsidata),1,oldfile);
        fwrite(&frq,sizeof(frq),1,oldfile);
        fwrite(&rec,sizeof(rec),1,oldfile);
        fclose(afile);
        unlink(nfname);
        sprintf(mfname,"%sPCBFIDO.CFG",PcbData.FidoLoc);
        movefile(fname,mfname);

}
static void near pascal FidoQueue(void)
{

  FILE * afile,*nfile;
  QUEUE_RECORD   qrec;
  char   fname[MAXDIR];
  char   nfname[MAXDIR];
  char   base[MAXDIR];
  char * nullit=NULL;

        maxstrcpy(base,PcbData.FidoQueue,sizeof(base));
        nullit = findstartofname(base);
        if(nullit) *nullit = '\x0';

        sprintf(fname,"%sFIDOQUE.DAT",base);
        sprintf(nfname,"%sFIDOQUE.TMP",PcbData.FidoLoc);
        printf("\nConverting FIDOQUE.DAT");

        afile = fopen(fname,"r+b");
        nfile = fopen(nfname,"wb");
        if(!afile || !nfile)
        {
          printf("\nCould not open QUEUE file");
          return;
        }

        fwrite(&newver,sizeof(newver),1,nfile);
        fread(&qrec,sizeof(qrec),1,afile);
        while(fread(&qrec,sizeof(qrec),1,afile) == 1)
          fwrite(&qrec,sizeof(qrec),1,nfile);
        fclose(afile);
        fclose(nfile);
        movefile(nfname,fname);
}
