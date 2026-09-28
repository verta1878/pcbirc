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

#include <string.h>
#include <stdio.h>
#include <dir.h>

#include <misc.h>
#include <bug.h>
#include <pcb.h>
#include <pcboard.h>
#include <pcboard.ext>
#include <messages.h>
#include <screen.h>
#include <users.h>
#include <pcb.h>

#include "defines.h"
#include "structs.h"
#include "prototyp.h"
#include "times.h"
#include "byte.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

extern unsigned int   num_records;
extern TRANSLATE      *record_list;
extern pcbdattype     PcbData;

/*****************************************************************************
 Prototypes
 *****************************************************************************/

 static void _NEAR_ LIBENTRY processPreSuffix(char * phone);

/*****************************************************************************/
/* Translate the phone number according to the data in the config file.      */

bool LIBENTRY translate_phone(char *phone)
{

int         i=0,len=0,flen=0;
char        temp[100];
char        temp1[100];
char        *chPtr1=NULL,*chPtr2=NULL;

#ifdef TRAVIS
    checkdatfile("15");
#endif

  memset(temp,0,sizeof(temp));
  memset(temp1,0,sizeof(temp1));

  if(num_records==NULL)
    return(FALSE);

  processPreSuffix(phone);
  for(i=0;i<num_records;i++)
    {
      record_list[i].in[sizeof(record_list[i].in)-1] = '\x0';
      record_list[i].in[sizeof(record_list[i].out)-1] = '\x0';
      stripright(record_list[i].in,' ');
      stripright(record_list[i].out,' ');
      if((chPtr1 = strstr(phone,record_list[i].in))!=NULL)
        {
          len=strlen(record_list[i].in);
          flen=strlen(phone);
          chPtr2 = chPtr1 + len;

          if(chPtr1==phone)
            {
              memcpy(temp,chPtr2,flen-len);
              sprintf(phone,"%s%s",record_list[i].out,temp);
              //processPreSuffix(phone);
              return(TRUE);
            }
          else if(chPtr2==phone+flen)
            {
              memcpy(temp,phone,flen-len);
              sprintf(phone,"%s%s",temp,record_list[i].out);
              //processPreSuffix(phone);
              return(TRUE);
            }
          else
            {
              memcpy(temp,phone,int(chPtr1-phone));
              memcpy(temp1,chPtr2,strlen(chPtr2));
              sprintf(phone,"%s%s%s",temp,record_list[i].out,temp1);
              //processPreSuffix(phone);
              return(TRUE);
            }
        }
    }
  return(FALSE);
}
static void _NEAR_ LIBENTRY processPreSuffix(char * phone)
{
  char tmpBuf[100];
  bool GPDone = FALSE;
  bool GSDone = FALSE;
  bool IPDone = FALSE;
  int  i = 0;


    // Cycle through translation table
    for(i = 0; i<num_records;i++)
    {
      // If a global prefix hasn't been processed yet, process it
      if(!GPDone && strstr("GP",record_list[i].in))
      {
        sprintf(tmpBuf,"%s%s",record_list[i].out,phone);
        strcpy(phone,tmpBuf);
        GPDone = TRUE;
      }
      // If a global suffix hasn't been processed yet, process it
      if(!GSDone && strstr("GS",record_list[i].in))
      {
        strcat(phone,record_list[i].out);
        GSDone = TRUE;
      }
      // If an international prefix hasn't been processed yet, process it
      if(!IPDone && strcmp("IPX",record_list[i].in)==0)
      {
        if(memcmp(record_list[i].out,phone,strlen(record_list[i].out)))
        {
           for(int j=0;j<num_records;j++)
           {
             if(strcmp(record_list[j].in,"IP")==0)
             {
                sprintf(tmpBuf,"%s%s",record_list[j].out,phone);
                strcpy(phone,tmpBuf);
                break;
             }
           }
        }
      }

    }
    #ifdef DEBUG
    if(mc_check_buffers())
      GSDone = TRUE;
    #endif
}

#endif
