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


#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dir.h>
#include <dos.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <assert.h>
#include <malloc.h>
// #include <conio.h>

#include <screen.h>
#include <pcboard.h>
#include <scrnio.h>
#include <bug.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <messages.h>

extern "C" int _Cdecl getch( void );
extern "C" int _Cdecl kbhit( void );

#include <defines.h>
#include <structs.h>
#include <prototyp.h>
#include <pcbnlc.h>
#include <dbase.hpp>



#ifdef DEBUG
#include <memcheck.h>
#endif

#define     TMP_NODE            "NODES.TMP"
#define     TEN_SECONDS         (182)

char    *nodelist_fields[] =
{
  "ZONE,C,5,0",
  "NET,C,5,0",
  "NODE,C,5,0",
  "BBS_NAME,C,30,0",
  "LOCATION,C,20,0",
  "SYSOP_NAME,C,20,0",
  "BBS_PHONE,C,20,0",
  "MAXBAUD,C,6,0",
  "CONFIG,C,5,0",
  "COMP,C,5,0",
  "CAPABLE,C,5,0",
  NULL
};

typedef struct
{
  char  zone[5];
  char  net [5];
  char  node[5];
  char  bbs_name[30];
  char  location[20];
  char  sysop[20];
  char  phone[20];
  char  baud [6];
  char  config[5];
  char  comp[5];
  char  capable[5];
} NODELIST_DBF;

unsigned int            current_zone,       /* current zone being processed */
                        current_net,        /* current net being processed  */
                        current_node,       /* current node we are on       */
                        num_zones,          /* number of zones found        */
                        num_nets,
                        num_nodes;

unsigned long           begin_offset;

DOSFILE                 index_file;
NODE_REC                noderec;
char                    filename[80];
char                    nodelist[100];
char                    index_name[100];
char                    nl_bak[100];
char                    idx_bak[100];
cDBF                    nodelist_db;

extern NODELIST         *nodelist_list;
extern far DIRECTORIES  directory_info;
extern unsigned int     num_lists;
extern struct           ffblk DTA;
extern unsigned         _stklen = 16000;

pcbdattype PcbData;
char         pcboardDatFile[MAXDIR];
char *DatFile="PCBOARD.DAT";

/*****************************************************************************/
/* Copy a file.                                                              */

void TK_CopyFile(char far *source,char far *destination)
{
  copyfile(source,destination,FALSE);
}

/****************************************************************************/

int main(int argc, char **argv)
{
int retval=0;

#ifdef DEBUG
  mc_startcheck(erf_standard);
#endif

  initscrnio();
  num_zones=num_nets=num_nodes=0;

  maxstrcpy(pcboardDatFile,"PCBOARD.DAT",sizeof(pcboardDatFile));
  DatFile = pcboardDatFile;
  if(fileexist(DatFile) == 255) srchpath(DatFile);

  if(fileexist(DatFile)==255)
    {
      printf("\nPCBOARD.DAT not found. Please run this program from a \n");
      printf("directory that contains a PCBOARD.DAT file.\n");
      exit(1);
    }

  readdatfile();
  getmode();

  if(read_fido_config(LIST_RECS)==FALSE)
    {
      free_fido_memory();
      printf("\nCould not read fido configuration file. Aborting.");
      exit(1);
    }

  if(argc==2 && !strcmpi(argv[1],"/diff"))
    {
      if(process_nodediff() == TRUE) retval = 0;
      else                           retval = 1;
      free_fido_memory();
      exit(retval);
    }
  if(argc==2 && !strcmpi(argv[1],"/help"))
    {
      print_help();
      free_fido_memory();
      exit(0);
    }
  if(argc==5 && !strcmpi(argv[1],"/find"))
    {
      if(get_node(atoi(argv[2]),atoi(argv[3]),atoi(argv[4]),&noderec)!=TRUE)
        {
          printf("Node %u:%u/%u not found!!\n",atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
          free_fido_memory();
          exit(1);
        }
      printf("%s\n%s\n%s\n%s\n",noderec.BBS_name,noderec.Sysop_name,noderec.phone,noderec.location);
      free_fido_memory();
      exit(0);
    }
  print_banner();
  if(process_node_list() == TRUE) retval = 0;
  else                            retval = 1;
  free_fido_memory();

#ifdef DEBUG
  mc_endcheck();
#endif

  gotoxy(0,10);
  return(retval);
}

/****************************************************************************/
/* Read in a line of characters from the file pointed to by filebuffer.     */
/* It reads until it finds a carriage return, storing the information in    */
/* the buffer passed in.  Function returns a 0 on valid string, or a -1 if  */
/* the end of file was reached. Strip Underscores.                          */

int pascal read_line(DOSFILE *filebuffer,char *buffer,int max_len)
{
int     i=0,j=0,len=0,dif=0,length=0;
char    buf[4096];

  j=0;
  memset(buf,0,sizeof(buf));

  while(1)
    {
      length=dosfread(buf,sizeof(buf),filebuffer);

      if(length==0 || length==-1)
        return(-1);

      for(i=0;i<sizeof(buf);i++,j++)
        {
          if(j==max_len)
            {
              buffer[j]=NULL;
              return(0);
            }

          switch(buf[i])
            {
              case  CR:
                        buf[i]=NULL;
                        buffer[j]=buf[i];
                        len=strlen(buf);
                        dif=length-len;
                        dif--;
                        dosfseek(filebuffer,-dif,SEEK_CUR);
                        return(0);

              case '_': buf[i]=' ';
                        buffer[j]=buf[i];
                        break;

              case  LF: j--;
                        continue;

              default:
                        buffer[j]=buf[i];
                        break;
            }
        }
    }
//    return(0);
}

/****************************************************************************/
/* Given a particular string of input read in from the nodelist_list, we want to */
/* parse it out and write out a data record to a temporary file pointed to  */
/* by tempfile.                                                             */

static long         cnt1 = 0L;
static long         cnt2 = 0L;

int pascal process_line(char *buffer,DOSFILE *nl_dbf)
{
char                *tptr,workbuffer[200],messagebuff[200];
unsigned int        i,len;
bool                newzone,newnet;
static bool         first=TRUE;
static char         zonestr[30]="1";
SysFlags1           SF1;
SysFlags2           SF2;
SysFlags3           SF3;
static NODELIST_DBF dbf_rec;


  newzone=FALSE;
  newnet=FALSE;
  maxstrcpy(workbuffer,buffer,200);
  memset(dbf_rec.bbs_name,' ',(sizeof(dbf_rec)-(sizeof(dbf_rec.zone)+sizeof(dbf_rec.net)+sizeof(dbf_rec.node))));

  if(*buffer==',')
    i=1;
  else
    i=0;

  if(first)
    {
      first=FALSE;
      memset(&dbf_rec,0x20,sizeof(dbf_rec));
      memcpy(dbf_rec.zone+(sizeof(dbf_rec.zone)-1),"1",1);
      memcpy(dbf_rec.net +(sizeof(dbf_rec.net )-1),"1",1);
      memcpy(dbf_rec.node+(sizeof(dbf_rec.node)-1),"0",1);
    }

  for(tptr=strtok(workbuffer,",");i<8;tptr=strtok(NULL,","),i++)
    {
      if(!tptr) continue;
      switch(i)
        {
          case HEIRARCHY:     if(strcmpi(tptr,"Zone")==0)
                                {
                                  newzone=TRUE;
                                  num_zones++;
                                }
                              if(strcmpi(tptr,"Host")==0 || strcmpi(tptr,"Region")==0)
                                {
                                  newnet=TRUE;
                                  num_nets++;
                                }
                              break;

          case NUMBER:        if(newzone)
                                {
                                  len=strlen(tptr);
                                  maxstrcpy(zonestr,tptr,sizeof(zonestr));
                                  memset(dbf_rec.zone,' ',sizeof(dbf_rec.zone));
                                  memset(dbf_rec.net ,' ',sizeof(dbf_rec.net));
                                  memset(dbf_rec.node,' ',sizeof(dbf_rec.node));
                                  memcpy(dbf_rec.zone+(sizeof(dbf_rec.zone)-len),tptr,len);
                                  memcpy(dbf_rec.net +(sizeof(dbf_rec.net) -len),tptr,len);
                                  memcpy(dbf_rec.node+(sizeof(dbf_rec.node)-1),"0",1);
                                }
                              else if(newnet)
                                {
                                  len=strlen(tptr);
                                  memset(dbf_rec.net ,' ',sizeof(dbf_rec.net));
                                  memset(dbf_rec.node,' ',sizeof(dbf_rec.node));
                                  memcpy(dbf_rec.net +(sizeof(dbf_rec.net) -len),tptr,len);
                                  memcpy(dbf_rec.node+(sizeof(dbf_rec.node)-1),"0",1);

                                  sprintf(messagebuff,"Processing Net: %s:%s       ",zonestr,tptr);
                                  fastprint(START_COL,MESSAGES,messagebuff,0x0b);
                                }
                              else if(!newnet&&!newzone)
                                {
                                  len=strlen(tptr);
                                  memset(dbf_rec.node,' ',sizeof(dbf_rec.node));
                                  memcpy(dbf_rec.node+(sizeof(dbf_rec.node)-len),tptr,len);
                                }

                             break;

          case BBS_NAME:    if(tptr!=NULL)
                              memcpy(dbf_rec.bbs_name,tptr,strlen(tptr));
                            break;

          case LOCATION:    if(tptr!=NULL)
                              memcpy(dbf_rec.location,tptr,strlen(tptr));
                            break;

          case SPEED:       if(tptr!=NULL)
                              memcpy(dbf_rec.baud+(sizeof(dbf_rec.baud)-strlen(tptr)),tptr,strlen(tptr));
                            break;

          case F_SYSOP:     if(tptr!=NULL)
                              memcpy(dbf_rec.sysop,tptr,strlen(tptr));
                            break;

          case PHONE:       if(tptr!=NULL)
                              memcpy(dbf_rec.phone,tptr,strlen(tptr));
                            break;

          case FLAGS:
                            while((tptr=strtok(NULL,","))!=NULL)
                            {
                                    //SysFlags1
                                    if(strcmpi("CM",tptr))
                                      SF1.crash_mailer = TRUE;
                                    else if(strcmpi("NH",tptr))
                                      SF1.no_humans    = TRUE;
                                    else if(strcmpi("LO",tptr))
                                      SF1.listed_only  = TRUE;

                                    //SysFlags2
                                    else if(strcmpi("V21",tptr))
                                      SF2.V21 = TRUE;
                                    else if(strcmpi("V22",tptr))
                                      SF2.V22 = TRUE;
                                    else if(strcmpi("V29",tptr))
                                      SF2.V29 = TRUE;
                                    else if(strcmpi("V32",tptr))
                                      SF2.V32 = TRUE;
                                    else if(strcmpi("V32B",tptr))
                                      SF2.V32B = TRUE;
                                    else if(strcmpi("V33",tptr))
                                      SF2.V33 = TRUE;
                                    else if(strcmpi("V34",tptr))
                                      SF2.V34 = TRUE;
                                    else if(strcmpi("V42",tptr))
                                      SF2.V42 = TRUE;
                                    else if(strcmpi("V42B",tptr))
                                      SF2.V42B = TRUE;
                                    else if(strcmpi("MNP",tptr))
                                      SF2.MNP = TRUE;
                                    else if(strcmpi("H96",tptr))
                                      SF2.H96 = TRUE;
                                    else if(strcmpi("HST",tptr))
                                      SF2.HST = TRUE;
                                    else if(strcmpi("MAX",tptr))
                                      SF2.MAX = TRUE;
                                    else if(strcmpi("PEP",tptr))
                                      SF2.PEP = TRUE;
                                    else if(strcmpi("CSP",tptr))
                                      SF2.CSP = TRUE;
                                    else if(strcmpi("MN",tptr))
                                      SF2.MN = TRUE;

                                    //SysFlags3
                                    else if(strcmpi("XA",tptr))
                                      SF3.XA = TRUE;
                                    else if(strcmpi("XB",tptr))
                                      SF3.XB = TRUE;
                                    else if(strcmpi("XC",tptr))
                                      SF3.XC = TRUE;
                                    else if(strcmpi("XP",tptr))
                                      SF3.XP = TRUE;
                                    else if(strcmpi("XR",tptr))
                                      SF3.XR = TRUE;
                                    else if(strcmpi("XW",tptr))
                                      SF3.XW = TRUE;
                                    else if(strcmpi("XX",tptr))
                                      SF3.XX = TRUE;
                                    else if(strcmpi("G",tptr))
                                      SF3.Gateway = TRUE;

                            }//while

                            memcpy(dbf_rec.config,&SF1,sizeof(SF1));
                            memcpy(dbf_rec.comp,&SF2,sizeof(SF2));
                            memcpy(dbf_rec.capable,&SF3,sizeof(SF3));
                            if(dosfwrite(" ",1,nl_dbf)==-1)
                              return(-1);
                            if(dosfwrite(&dbf_rec,sizeof(dbf_rec),nl_dbf)==-1)
                              return(-1);
                            ++cnt1;
                            break;
        }
    }
  ++cnt2;
  return(0);
}

/****************************************************************************/
/* Print out screen layout                                                  */

void pascal print_banner(void)
{
char        buffer[80];

  cls();
  sprintf(buffer,"PCBoard Node List Compiler - Version %s.%s",VERSION_MAJOR,VERSION_MINOR);
  fastcenter(0,buffer,0x0a);
  fastcenter(1,"Copyright 1995 - Clark Development Company, Inc.",0x0e);
  fastprint(0,STATUS_LINE,"Status :",0x0f);
  fastprint(0,MESSAGES,"Message:",0x0f);
  fastprint(0,COMP_LINE,"% Done :",0x0f);
  gotoxy(0,10);
}

/****************************************************************************/
/* print out the help screen.                                               */

void pascal print_help(void)
{
  cls();
  fastprint(0,0,"PCBoard Node List Compiler",0x0f);
  fastprint(0,1,"Copyright 1994 - Clark Development Company, Inc.",0x0f);
  fastprint(0,3,"PCBNLC",0x0f);
  fastprint(7,3,"[filename]",0x07);
  fastprint(0,5,"[filename]",0x07);
  fastprint(11,5,"is the name of the nodelist_list file to compile.",0x0f);
  gotoxy(0,10);
}

/****************************************************************************/
/* Abort nodelist compile. Restore backups.                                 */

void pascal abort_compile(bool flag)
{
  if(flag==TRUE)
    fastprint(START_COL,STATUS_LINE,"Aborting nodelist compile. Restoring backups.                      ",0x0c);
  else
    fastprint(START_COL,STATUS_LINE,"Error detected writing to disk. Aborting.                          ",0x0c);

  remove(nodelist);
  remove(index_name);

  rename(nl_bak,nodelist);
  rename(idx_bak,index_name);
}

/****************************************************************************/
/* Process Nodelist files.                                                  */

bool pascal process_node_list(void)
{

DOSFILE                                                 thefile,nl_dbf;
char                in_line[4096],messagebuff[200],filebuf[80],*tptr;
char                compbar[50];
unsigned int        number,highnum,num_nodes;
unsigned long       total_nodes;
long                endpos,curpos,progress,when;
float               tmp;
bool                error;
struct find_t       file_block;

  #ifdef _BORLANDC_
  assert(stackavail() >= 1000);
  #endif
  error=FALSE;
  num_nodes=0;
  total_nodes=0;

  sprintf(nodelist,"%sNODELIST.DBF",directory_info.nodelist_path);
  sprintf(index_name,"%sNODELIST.NDX",directory_info.nodelist_path);
  sprintf(nl_bak,"%sNODELIST.DBK",directory_info.nodelist_path);
  sprintf(idx_bak,"%sNODELIST.IBK",directory_info.nodelist_path);

  fastprint(START_COL,STATUS_LINE,"Backing up nodelist database.",0x0c);

  unlink(nl_bak);
  unlink(idx_bak);

  rename(nodelist,nl_bak);
  rename(index_name,idx_bak);

  //create new database.

  nodelist_db.dbfCreate(nodelist,TRUE,nodelist_fields);
  if(nodelist_db.error())
  {

    fastprint(START_COL,STATUS_LINE,"DBase error creating Database.",0x0c);
    return FALSE;
  }

        nodelist_db.dbfClose();

  if(dosfopen(nodelist,OPEN_RDWR|OPEN_DENYRDWR,&nl_dbf)==-1)
  {

    fastprint(START_COL,STATUS_LINE,"Could not open nodelist file.",0x0c);
    fastprint(START_COL,STATUS_LINE,nodelist,0x0c);
    return FALSE;
  }

  dossetbuf(&nl_dbf,16384);
  dosfseek(&nl_dbf,-1,SEEK_END);

  for(number=0;number<num_lists;number++)
    {
      highnum=0;
      memset(compbar,176,sizeof(compbar));
      compbar[49]=NULL;
      fastprint(START_COL,COMP_LINE,compbar,0x0e);

      if(nodelist_list[number].basename[0]!=0x20 && nodelist_list[number].basename[0]!=NULL)
        {
          if((tptr=strrchr(findstartofname(nodelist_list[number].basename),'.'))!=NULL)
            *tptr=NULL;

          sprintf(filebuf,"%-1.80s.*",nodelist_list[number].basename);
          if(_dos_findfirst(filebuf,FA_ARCH,&file_block)==0)
            {
              tptr=strstr(file_block.name,".");
              tptr++;

              if(isdigit(*tptr) && isdigit(*(tptr+1)) && isdigit(*(tptr+2)))
                highnum=atoi(tptr);

              while(_dos_findnext(&file_block)==0)
                {
                  tptr=strstr(file_block.name,".");
                  tptr++;
                  if(isdigit(*tptr) && isdigit(*(tptr+1)) && isdigit(*(tptr+2)))
                    {
                      if(atoi(tptr)>highnum)
                      highnum=atoi(tptr);
                    }
                }
              sprintf(filename,"%s.%03d",nodelist_list[number].basename,highnum);
            }

          if(fileexist(filename)!=255)
            {
              endpos=DTA.ff_fsize;

              if(dosfopen(filename,OPEN_READ|OPEN_DENYNONE,&thefile)==-1)
                {
                  sprintf(messagebuff,"Error opening %s\n",filename);
                  fastprint(START_COL,MESSAGES,messagebuff,0x0c);
                  setcursor(CUR_NORMAL);
                  dosfclose(&nl_dbf);
                  return FALSE;
                }

              if((when=(endpos/200000L))<20)
                when=20;

              sprintf(messagebuff,"Processing nodelist file: %s",filename);
              fastprint(START_COL,STATUS_LINE,messagebuff,0x0d);

              dossetbuf(&thefile,16384);

              while(!read_line(&thefile,in_line,sizeof(in_line)-1) && !error)
                {
                  assert(strlen(in_line) < sizeof(in_line));
                  giveup();

                  if(*in_line!=';' && (in_line[0]==',' || isalpha(in_line[0])))
                    {
                      if(process_line(in_line,&nl_dbf)==-1)
                        {
                          dosfclose(&thefile);
                          dosfclose(&nl_dbf);
                          abort_compile(FALSE);
                          fastprint(START_COL,MESSAGES,"Press any key to continue or wait 10 seconds.                   ",0x0c);
                          settimer(4,TEN_SECONDS);
                          while(!timerexpired(4))
                            {
                              giveup();
                              if(kbhit()!=0)
                                return FALSE;
                            }
                          return FALSE;
                        }
                      num_nodes++;
                      total_nodes++;
                    }

                  if(num_nodes>=when)
                    {
                      if(kbhit()!=0)
                        {
                          if(getch()=='\x1B')
                            {
                              dosfclose(&thefile);
                              dosfclose(&nl_dbf);
                              abort_compile(TRUE);
                              return FALSE;
                            }
                        }
                      num_nodes=0;
                      curpos=dosfseek(&thefile,0,SEEK_CUR);
                      tmp=(float)curpos/(float)endpos;
                      progress=(long)(tmp*sizeof(compbar));
                      if(progress>49)
                        progress=49;
                      memset(compbar,219,(int)progress);
                      compbar[49]=NULL;
                      fastprint(START_COL,COMP_LINE,compbar,0x0e);
                    }
                  giveup();
                }
              memset(compbar,219,sizeof(compbar));
              compbar[49]=NULL;
              fastprint(START_COL,COMP_LINE,compbar,0x0e);

              clsbox(START_COL,MESSAGES,LAST_COL,MESSAGES,0x00);
              clsbox(START_COL,STATUS_LINE,LAST_COL,STATUS_LINE,0x00);
              dosfclose(&thefile);
            }
          else
            {
              sprintf(messagebuff,"%s does not exist!",filename);
              fastprint(START_COL,MESSAGES,messagebuff,0x0c);
            }
        }
    }
  fastprint(START_COL,STATUS_LINE,"Indexing.... Please Wait.",0x0d);
  dosfseek(&nl_dbf,4,SEEK_SET);
//dosfwrite(&total_nodes,sizeof(total_nodes),&nl_dbf);
  // Patch for buggy travis code because he mis-counted records
  dosfwrite(&cnt1,sizeof(cnt1),&nl_dbf);
  dosfclose(&nl_dbf);
        nodelist_db.dbfOpen(nodelist,TRUE);
  nodelist_db.ndxCreate(index_name,"ZONE+NET+NODE");
  if(nodelist_db.error())
  {

    fastprint(START_COL,STATUS_LINE,"Could not Create Index file",0x0c);
    return FALSE;
  }
  nodelist_db.dbfClose();
  unlink(nl_bak);
  unlink(idx_bak);
  clsbox(START_COL,STATUS_LINE,79,STATUS_LINE,0x00);
  fastprint(START_COL,STATUS_LINE,"Done!",0x0e);
  return TRUE;
}


char * pascal findstartofname(char *Path) {
  char *p;

  // find the first character of the filename, skipping over any path
  // information

  if ((p = strrchr(Path,'\\')) != NULL || (p = strrchr(Path,':')) != NULL)
    return(p+1);  // point to first character of filename

  return(Path);   // or point to first character of complete path
}
