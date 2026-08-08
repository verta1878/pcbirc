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


/******************************************************************************
                                REPORT.CPP
                          Written by: Stan Paulsen

                 Copyright (C) 1995 Clark Development, inc.

     This module contains functions related to generating a FIDO log report

 ******************************************************************************/

#include <iostream.h>
#include <dir.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include <ctype.h>

#include <bug.h>
#include <pcb.h>
#include <misc.h>

#include <screen.h>
#include <scrnio.h>
#include <screen.h>
//#include <scrnio.ext>
#include <help.h>
#include <event.h>

//#include <data.hpp>
#include <defines.h>
#include <ptsetup.hpp>
#include <passthru.hpp>
#include <fidonet.hpp>

static void near pascal compileLogs();
static void near pascal sortLog();
static unsigned long near pascal getBlockDate(const char * line);
static unsigned int  near pascal getMaxNodes(void);
int    rcmp(const void * a, const void * b);

char fidolog[MAXDIR];

typedef struct
{
  unsigned long code;
  unsigned long offset;
}SORTREC;

void pascal generateLog(void)
{
  compileLogs();
  sortLog();
}

static void near pascal compileLogs()
{
cDOSFILE      nfile,ffile;
char          nodelog[MAXDIR],basename[15],name[MAXDIR];
char          line[100],header[100],logBuf[100];;
char     *    nameptr=NULL;
unsigned int  nodenum=0,totnodes=0;
unsigned long          datetime=0;

  cls();

  cout << "Generating report. Please wait..." << endl;
  totnodes = getMaxNodes();

  maxstrcpy(nodelog,PcbData.ClrFile,sizeof(nodelog));
  nameptr = findstartofname(nodelog);
  if(nameptr)
  {
     maxstrcpy(basename,nameptr,sizeof(basename));
     *nameptr = '\x0';
  }
  if(memcmp(basename,"CALLER",6) == 0 && totnodes >=100)
    strcpy(basename,"CLR");

  sprintf(fidolog,"%sFIDO.LOG",nodelog);
  if(fileexist(fidolog) != 255) unlink(fidolog);
  if( ffile.open(fidolog,OPEN_RDWR | OPEN_CREATE | OPEN_DENYNONE) != 0)
  {
    printf("\nCould not open temporary log file.");
    return;
  }


  for(nodenum = 1;nodenum<=totnodes;nodenum++)
  {
    sprintf(name,"%s%s%d",nodelog,basename,nodenum);
    if(fileexist(name) == 255)  continue;
    sprintf(logBuf,"Searching caller log %s.",findstartofname(name));
    cout << logBuf << endl;

    if(nfile.open(name,OPEN_RDWR | OPEN_DENYNONE) != 0)
    {
      printf("\nCould not open caller log ");
      printf(name);
      continue;
    }
    while(nfile.getln(line,sizeof(line)) == 0)
    {
      // Read export block
      if(strstr(line,"Beginning FIDO Message Export") )
      {
        datetime = getBlockDate(line);
        sprintf(logBuf,"[NODE %d] [%ld]",nodenum,datetime);
        ffile.putln(logBuf);
        while(TRUE)
        {
          ffile.putln(line);
          if(nfile.getln(line,sizeof(line)) != 0) break;
          if(strstr(line,"Ending FIDO Message Export") )
          {
            ffile.putln(line);
            ffile.putln("END LOG BLOCK");
            break;
          }
        }
      }

      // Read import block
      if(strstr(line,"Beginning FIDO mail import") )
      {
        datetime = getBlockDate(line);
        sprintf(logBuf,"[NODE %d] [%ld]",nodenum,datetime);
        ffile.putln(logBuf);
        while(TRUE)
        {
          ffile.putln(line);
          if(nfile.getln(line,sizeof(line)) != 0) break;;
          if(strstr(line,"Ending message import") )
          {
            ffile.putln(line);
            ffile.putln("END LOG BLOCK");
            break;
          }
        }
      }

      // Read import block
      if(strstr(line,"Beginning mail run") )
      {
        datetime = getBlockDate(line);
        sprintf(logBuf,"[NODE %d] [%ld]",nodenum,datetime);
        ffile.putln(logBuf);
        while(TRUE)
        {
          ffile.putln(line);
          if(nfile.getln(line,sizeof(line)) != 0) break;
          if(strstr(line,"Ending mail run") )
          {
            ffile.putln(line);
            ffile.putln("END LOG BLOCK");
            break;
          }
        }
      }
      if(strstr(line,"Beginning incoming mail transfer") )
      {
        datetime = getBlockDate(line);
        sprintf(logBuf,"[NODE %d] [%ld]",nodenum,datetime);
        ffile.putln(logBuf);
        while(TRUE)
        {
          ffile.putln(line);
          if(nfile.getln(line,sizeof(line)) != 0) break;
          if(strstr(line,"Ending mail run") || strstr(line,"Handshake Error"))
          {
            ffile.putln(line);
            ffile.putln("END LOG BLOCK");
            break;
          }
        }
      }

      if(strstr(line,"Beginning NETMAIL export.") )
      {
        datetime = getBlockDate(line);
        sprintf(logBuf,"[NODE %d] [%ld]",nodenum,datetime);
        ffile.putln(logBuf);
        while(TRUE)
        {
          ffile.putln(line);
          if(nfile.getln(line,sizeof(line)) != 0) break;
          if(strstr(line,"Ending NETMAIL export."))
          {
            ffile.putln(line);
            ffile.putln("END LOG BLOCK");
            break;
          }
        }
      }
      if(strstr(line,"Adding POLL to") || strstr(line,"Removing POLL to"))
      {
        datetime = getBlockDate(line);
        sprintf(logBuf,"[NODE %d] [%ld]",nodenum,datetime);
        ffile.putln(logBuf);
        ffile.putln(line);
        ffile.putln("END LOG BLOCK");
      }
    }// While
    nfile.close();
  }// for
  ffile.putln("END LOG BLOCK");
  ffile.close();

}


static unsigned long near pascal getBlockDate(const char * line)
{
char * ptr;
char   buf[20];
unsigned int mo,da,yr,hr,min;


  ptr = (char *) line;
  mo = atoi(ptr);

  while(*ptr != '-' && *ptr != '\x0') ptr++;
  da = atoi(++ptr);

  while(*ptr != '-' && *ptr != '\x0') ptr++;
  yr = atoi(++ptr);

  while(*ptr != '(' && *ptr != '\x0') ptr++;
  hr = atoi(++ptr);

  while(*ptr != ':' && *ptr != '\x0') ptr++;
  min = atoi(++ptr);

  sprintf(buf,"%02d%02d%02d%02d%02d",mo,da,yr,hr,min);
  return atol(buf);
}

static void near pascal sortLog()
{
cDOSFILE     file,tmp;
char         tmpname[MAXDIR],line[100];
char      *  ptr=NULL,ch;
SORTREC *    recs;
const unsigned int numrecs=5000;
      unsigned int i=0,j=0;

  recs = (SORTREC *) malloc(sizeof(SORTREC) * numrecs);
  if(!recs) return;
  memset(recs,0,sizeof(SORTREC)*numrecs);
  ptr = findstartofname(fidolog);
  ch = *ptr;
  if(ptr) *ptr = '\x0';
  maxstrcpy(tmpname,fidolog,sizeof(tmpname));
  *ptr = ch;
  strcat(tmpname,"FIDOLOG");
  if(file.open(fidolog,OPEN_RDWR | OPEN_DENYNONE) != 0)
  {
    printf("\n Could not open log file: %s",fidolog);
    free(recs);
    return;
  }
  while(file.getln(line,sizeof(line)) == 0 && i < numrecs)
  {
    if(line[0] == '[' && line[1] == 'N' && line[2] == 'O' && strstr(line,"[NODE"))
    {
      ptr = strchr(line,']');
      if(!ptr) continue;
      while(*ptr != '[' && *ptr != '\x0') ptr++;
      if(*ptr == '\x0') continue;
      ++ptr;
      recs[i].code   = atol(ptr);
      recs[i++].offset = file.seek(0,SEEK_CUR) - strlen(line)-2;
    }
  }
  if(fileexist(tmpname) != 255) unlink(tmpname);
  tmp.open(tmpname,OPEN_RDWR | OPEN_DENYNONE);
  qsort(recs,i+1,sizeof(SORTREC),rcmp);
  tmp.putln("******************************");
  file.rewind();
  for(j=0;j<i;j++)
  {
    file.seek(recs[j].offset,SEEK_SET);
    while(TRUE)
    {
      file.getln(line,sizeof(line));
      if(strstr(line,"END LOG BLOCK")) break;
      //if(strstr(line,"[NODE"))
      //{
      //   ptr = strchr(line,']');
      //   if(ptr) *(++ptr) = '\x0';
      //}
      tmp.putln(line);
    }
    tmp.putln("******************************");
  }
  tmp.close();
  file.close();
  file.unlink();
  cout << "\nFido Activity Log Created in: "<< tmpname << endl;

}

int rcmp(const void * a, const void * b)
{
  if( ((SORTREC *)a)->code > ((SORTREC *)b)->code) return 1;
  if( ((SORTREC *)a)->code == ((SORTREC *)b)->code) return 0;
  return -1;
}

static unsigned int near pascal getMaxNodes(void)
{
cDOSFILE      unet;
unsigned int  maxnodes=0;


       if(unet.open(PcbData.NetFile,OPEN_READ | OPEN_DENYNONE) != 0) return 0;
       unet.seek(2,SEEK_SET);
       unet.read(&maxnodes,sizeof(maxnodes));
       return maxnodes;
}
