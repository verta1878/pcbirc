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


void cFIDOUSER(void)
{
char        path[100];
IndexType   index;
IndexType2  index2;
char        searchfld[26];
DOSFILE     ndxfile;
int         searchlen;
int         num=0;

  memset(users,0,sizeof(users));
  maxstrcpy(path,PcbData.NdxLoc,sizeof(path)-9);
  strcat(path,"PCBNDX.Z");
  if(dosfopen(path,OPEN_READ|OPEN_DENYNONE,&ndxfile)==-1)
    return(-1L);

  strcpy(searchfld,"~FIDO~");
  searchlen=strlen(searchfld);

  while(1)
    {
      if(BigNdx)
        {

          if(dosfread(&index2,sizeof(index2),&ndxfile)!=sizeof(index2))
            break;
          index2.UserName[sizeof(index2.UserName)-1]=NULL;
          stripright(index2.UserName,' ');

          if(strncmp(index2.UserName,searchfld,searchlen)==0)
            {
              maxstrcpy(users[num].name,index2.UserName,sizeof(users[num].name));
              users[num].num=index2.UserRec;
              num++;
            }
        }
      else
        {
          if(dosfread(&index,sizeof(index),&ndxfile)!=sizeof(index))
            break;
          index.UserName[sizeof(index.UserName)-1]=NULL;
          stripright(index.UserName,' ');

          if(strncmp(index.UserName,searchfld,searchlen)==0)
            {
              maxstrcpy(users[num].name,index.UserName,sizeof(users[num].name));
              users[num].num=index.UserRec;
              num++;
            }
        }
    }
  dosfclose(&ndxfile);
}

long cFIDOUSER::find_fido_rec(char * addr,bool cont,bool reset,long *new_offset)
{
static long offset=0;
char        searchfld[26];
int         searchlen;

  if(reset==TRUE)  /* Just reset the offset pointer and return. */
    {
      offset=0;
      return(0L);
    }

  /* if this is not a continuation then start at the beginning. */
  if(cont==FALSE)
    offset=0;

  if(new_offset!=NULL)
    offset=*new_offset;

  if(addr!=NULL)
    sprintf(searchfld,"~FIDO~%s",addr);
  else
    strcpy(searchfld,"~FIDO~");

  searchlen=strlen(searchfld);

  while(users[offset].name[0]!=0)
    {
      if(strncmp(users[i].name,addr,searchlen)==0)
        {
          if(new_offset!=NULL)
            *new_offset=offset;
          return(users[offset].num);
        }
    }
  return(-1L);
}
