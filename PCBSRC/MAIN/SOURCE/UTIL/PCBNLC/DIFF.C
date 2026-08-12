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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>

#include <dosfunc.h>
#include <pcb.h>
#include <misc.h>
#include <screen.h>
#include <scrnio.h>
#include <messages.h>

#include <defines.h>
#include <structs.h>
#include <prototyp.h>
#include <pcbnlc.h>

#ifdef DEBUG
#include "\memcheck\memcheck.h"
#endif

#define     TMP_NODE            "NODES.TMP"

extern unsigned int             current_zone,
                                current_net,
                                current_node,
                                num_zones,
                                num_nets,
                                num_nodes;

extern unsigned long            begin_offset;

extern DOSFILE                  tempnode,index_file;
extern NODE_REC                 noderec;
extern char                     filename[80];

extern NODELIST                 *nodelist_list;
extern unsigned int             num_lists;

extern pcbdattype PcbData;


extern DOSFILE                  index_file;

/****************************************************************************/
/* Read in a line of characters from the file pointed to by filebuffer.     */
/* It reads until it finds a carriage return, storing the information in    */
/* the buffer passed in.  Function returns a 0 on valid string, or a -1 if  */
/* the end of file was reached.                                             */

int pascal read_list_line(DOSFILE *file,char *buffer,int limit)
{
int     i=0,j=0,len=0,dif=0,length=0,size=0;
char    buf[500];

  j=0;
  memset(buf,0,sizeof(buf));
  size=sizeof(buf);

  while(1)
    {
      length=dosfread(buf,sizeof(buf),file);

      if(length==0 || length==-1)
        return(-1);

      for(i=0;i<size;i++,j++)
        {
          if(j==limit)
            {
              buffer[j]=NULL;
              return(0);
            }

          if(buf[i]==LF)
            {
              j--;
              continue;
            }

          else if(buf[i]==CR)
            {
              buffer[j]=NULL;
              len=strlen(buffer);
              len++;
              dif=length-len;
              dif--;
              dosfseek(file,-dif,SEEK_CUR);
              return(0);
            }
          else
            buffer[j]=buf[i];
        }
    }
//    return(0);
}

/****************************************************************************/
/* Enterance point to the module. Process a NodeDiff file.                  */

bool pascal process_nodediff(void)
{
int           number;
int           highnum;
int           i;
int           count;
char          in_line[500];
char          filename[80];
char          filebuf[80];
char          temp[80];
char          message_buffer[80];
char          buffer[80];
char          *tptr;
char          *oldptr;
char          *newptr;
char          compbar[50];
bool          error;
long          endpos,curpos,progress,when;
float         tmp;
struct find_t file_blk;
DOSFILE       nodelist_file,nodediff_file,output_file;

  count=0;

  print_banner();

  for(i=0;i<num_lists;i++)
    {
      if(nodelist_list[i].diffname[0] == '\x0') continue;
      highnum=0;
      memset(compbar,176,sizeof(compbar));
      compbar[49]=NULL;
      fastprint(START_COL,COMP_LINE,compbar,0x0e);

      error=FALSE;
      sprintf(filebuf,"%s.*",nodelist_list[i].basename);

      if(_dos_findfirst(filebuf,FA_ARCH|FA_NORMAL,&file_blk)==0)
        {
          do
          {
            maxstrcpy(filename,file_blk.name,sizeof(filename));
            newptr=filebuf;
            oldptr=filebuf;
            while((newptr=(char *)strchr(newptr,'\\'))!=NULL)
              {
                oldptr=newptr;
                newptr++;
              }
            oldptr++;
            *oldptr=NULL;

            sprintf(buffer,"%s%s",filebuf,filename);
            maxstrcpy(filename,buffer,sizeof(filename));
            tptr=strchr(filename,'.');
            tptr++;
            if(!(isdigit(*tptr) && isdigit(*(tptr+1)) && isdigit(*(tptr+2))))
              continue;
            number=atoi(tptr);
            if(number>highnum)
              highnum=number;
          }while(_dos_findnext(&file_blk)==0);

          number=highnum;
          tptr=strchr(filename,'.');
          if(tptr!=NULL)
            {
              tptr++;
              *tptr=NULL;
              sprintf(filebuf,"%s%03d",filename,highnum);
            }
          highnum+=7;
          itoa(highnum,buffer,10);
          newptr=filename;
          oldptr=filename;
          while((newptr=(char *)strchr(newptr,'\\'))!=NULL)
            {
              oldptr=newptr;
              newptr++;
            }
          oldptr++;
          *oldptr=NULL;
          maxstrcpy(temp,filename,sizeof(temp));
          sprintf(filename,"%s%s.%03d",temp,nodelist_list[i].diffname,atoi(buffer) % 365);

          if(fileexist(filename)!=255)
            {
              if(dosfopen(filebuf,OPEN_RDWR|OPEN_DENYWRIT,&nodelist_file)==-1)
                {
                  fastprint(START_COL,MESSAGES,"Could not open nodelist file",0x0b);
                  return FALSE;
                }

              if(dosfopen(filename,OPEN_READ|OPEN_DENYRDWR,&nodediff_file)==-1)
                {
                  fastprint(START_COL,MESSAGES,"Could not open Diff file.",0x0b);
                  dosfclose(&nodelist_file);
                  return FALSE;
                }

              endpos=dosfseek(&nodediff_file,0,SEEK_END);
              dosfseek(&nodediff_file,0,SEEK_SET);

              if((when=(endpos/1000))<10)
                when=10;
            }
          else
          {
           char messageBuf[90];

            sprintf(messageBuf,"Cannot locate diff file extension %03d. Diff %s",atoi(buffer) % 365,findstartofname(filename));
            fastprint(START_COL,COMP_LINE+3+i,messageBuf,0x0F);
            continue;
          }

          sprintf(message_buffer,"Processing Nodediff file: %s",filename);
          fastprint(START_COL,STATUS_LINE,message_buffer,0x0d);
          sprintf(filename,"%s.%03d",nodelist_list[i].basename,atoi(buffer) % 365);
          if(dosfopen(filename,OPEN_WRIT|OPEN_CREATE|OPEN_DENYRDWR,&output_file)==-1)
            {
              printf("Could not open output file.\n");
              dosfclose(&nodelist_file);
              dosfclose(&nodediff_file);
              return FALSE;
            }

          while(read_list_line(&nodediff_file,in_line,sizeof(in_line)-1)!=-1 && !error)
            {
              if(*in_line!=';')
                {
                  if(process_command(in_line,&nodelist_file,&nodediff_file,&output_file)==-1)
                    error=TRUE;
                }

              if(count>=when)
                {
                  count=0;
                  curpos=dosfseek(&nodediff_file,0,SEEK_CUR);
                  tmp=(float)curpos/(float)endpos;
                  progress=(long)(tmp*sizeof(compbar));
                  memset(compbar,219,(int)progress);
                  fastprint(START_COL,COMP_LINE,compbar,0x0e);
                }
              count++;
              giveup();
            }
          memset(compbar,219,sizeof(compbar));
          compbar[49]=NULL;
          fastprint(START_COL,COMP_LINE,compbar,0x0e);

          clsbox(START_COL,MESSAGES,LAST_COL,MESSAGES,0x00);
          clsbox(START_COL,STATUS_LINE,LAST_COL,STATUS_LINE,0x00);
          fastprint(START_COL,MESSAGES,"Done!",0x0d);
          dosfclose(&nodelist_file);
          dosfclose(&nodediff_file);
          dosfclose(&output_file);

          // rename old nodelist
          char *t,bakfile[100];

          maxstrcpy(bakfile,filebuf,sizeof(bakfile));
          t = strrchr(bakfile,'.');
          if(t)
          {
            *(t+1) = '!';
            movefile(filebuf,bakfile);
          }

        }
    }

    return TRUE;
}

/*****************************************************************************/
/* Process a nodediff command.                                               */

int pascal process_command(char *buffer,DOSFILE *nodelist_file,DOSFILE *nodediff_file,DOSFILE *output_file)
{
int     num,i;
char    in_buffer[500];

  switch(buffer[0])
    {
      case ';':    break;

      case 'A':   num=atoi(&buffer[1]);
                  for(i=0;i<num;i++)
                    {
                      if(read_list_line(nodediff_file,in_buffer,sizeof(in_buffer)-1)==-1)
                        return(-1);
                      strcat(in_buffer,"\r\n");
                      dosfputs(in_buffer,output_file);
                    }
                  break;

      case 'D':   num=atoi(&buffer[1]);
                  for(i=0;i<num;i++)
                    {
                      if(read_list_line(nodelist_file,in_buffer,sizeof(in_buffer)-1)==-1)
                        return(-1);
                    }
                  break;

      case 'C':   num=atoi(&buffer[1]);
                  for(i=0;i<num;i++)
                    {
                      if(read_list_line(nodelist_file,in_buffer,sizeof(in_buffer)-1)==-1)
                        return(-1);
                      strcat(in_buffer,"\r\n");
                      dosfputs(in_buffer,output_file);
                    }
                  break;

      default:    return(-1);
    }
  return(0);
}
