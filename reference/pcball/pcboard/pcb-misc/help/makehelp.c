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
#include "scrnio.h"

enum { NORMAL, DIVIDER, ENDOFFILE };
#define FORWARD  'F'
#define EITHER   'E'
#define BACKWARD 'B'
#define NEITHER  'N'

char           *TitleArray;
HelpTableType  *TableArray;
HelpHeaderType  HelpHeader;
HelpBodyType    HelpBody;

FILE  *TextFile;
FILE  *HelpFile;
int   NumTableEntries;
int   NumTitles;
int   SizeofTableArray;
int   SizeofTitleArray;
char  InText[80];


int pascal gettext(void) {
  if (fgets(InText,80,TextFile) == NULL)
    return(ENDOFFILE);

  if (InText[0] == '-' && InText[1] == '-')
    return(DIVIDER);

  InText[strlen(InText)-1] = 0;
  return(NORMAL);
}

void pascal opentextfile(void) {
  if ((TextFile = fopen("help.txt","rt")) == NULL) {
    puts("Error opening text file");
    exit(0);
  }
}

void pascal openhelpfile(void) {
  if (gettext() != NORMAL)
    exit(0);

  puts(InText);
  unlink(InText);

  if ((HelpFile = fopen(InText,"wb")) == NULL) {
    puts("Error opening help file");
    exit(0);
  }
}

void pascal counttableentries(void) {
  char Temp[3];
  char SaveHeader[80];

  SaveHeader[0] = 0;

  for (NumTableEntries = 0, NumTitles = 0, Temp[2] = 0; gettext() == NORMAL; ) {
    Temp[0] = InText[14];
    Temp[1] = InText[15];
    NumTableEntries += atoi(Temp);

    if (strcmp(SaveHeader,&InText[17]) != 0) {
      NumTitles++;
      strcpy(SaveHeader,&InText[17]);
      if (strlen(SaveHeader) > HELPTITLEWIDTH) {
        printf("Error in length of header: %s",SaveHeader);
        exit(0);
      }
    }
  }


  SizeofTitleArray = NumTitles * (HELPTITLEWIDTH+1);
  SizeofTableArray = NumTableEntries * sizeof(HelpTableType);


  /* NOTE:  we are going to allocate the MAXIMUM memory that will possibly */
  /* be used, however, the titles are actually PACKED into the memory and  */
  /* are not likely to use the whole thing!                                */

  if ((TitleArray = (char *) malloc(SizeofTitleArray)) == NULL) {
    puts("Error allocating title memory!");
    exit(0);
  }

  if ((TableArray = (HelpTableType *) malloc(SizeofTableArray)) == NULL) {
    puts("Error allocating table memory!");
    exit(0);
  }

  printf("NumTitles = %d\n"
         "NumTableEntries = %d\n",
         NumTitles,NumTableEntries);
}

void pascal writetablearray(void) {
  fseek(HelpFile,0,SEEK_SET);
  fwrite(TableArray,SizeofTableArray,1,HelpFile);
}


void pascal fillarrays(void) {
  char          *p;
  HelpTableType *q;
  int           Offset;
  int           Counter;
  int           Len;

  memset(TableArray,0,SizeofTableArray);
  memset(TitleArray,0,SizeofTitleArray);

  fseek(TextFile,0,SEEK_SET); /* go back to the start of the file again */
  gettext();                  /* read file name and just throw it away  */

  Offset = SizeofTableArray;
  p      = TitleArray;
  q      = TableArray;

  if (gettext() == NORMAL) {
    while (TRUE) {
      strcpy(p,&InText[17]);
      for (InText[16] = 0, Counter = atoi(&InText[14]); Counter; Counter--, q++)
        q->OffsetToTitle = Offset;
      if (gettext() != NORMAL)
        break;
      if (strcmp(p,&InText[17]) != 0) {
        Len = strlen(p)+1;
        p += Len;
        Offset += Len;
      }
    }
  }

  Len = (strlen(p) + 1) + (p - TitleArray);
  writetablearray();
  fwrite(TitleArray,Len,1,HelpFile);
}

void pascal processhelpfile(void) {
  int           Status;
  int           Len;
  char          *q;
  HelpTableType *p;

  p = TableArray;

  puts("copying text file information");

  do {
    p->OffsetToHeader = ftell(HelpFile);
    p++;

    if ((Status = gettext()) == NORMAL) {
      q = HelpBody.Text;
      memset(q,0,sizeof(HelpBodyType));
      memset(&HelpHeader,0,sizeof(HelpHeaderType));

      HelpHeader.ForwBack = InText[0];

      InText[3] = 0;
      HelpHeader.PageNum  = atoi(&InText[1]);
      HelpHeader.NumPages = atoi(&InText[4]);

      gettext();                           /* ignore the TITLE line */
      gettext();                           /* get the SUBTITLE line */
      strcpy(q,InText);
      q += strlen(q)+1;
      gettext();                           /* ignore the blank line */

      while ((Status = gettext()) == NORMAL) {
        strcpy(q,InText);
        q += strlen(q)+1;
        HelpHeader.NumLines++;
      }

      Len = (strlen(q) + 1) + (q - HelpBody.Text);
      fwrite(&HelpHeader,sizeof(HelpHeaderType),1,HelpFile);
      fwrite(&HelpBody,Len,1,HelpFile);
    }
  } while (Status != ENDOFFILE);
}


void pascal makeheaderfile(void) {
  FILE *HeaderFile;
  char *p;
  int  HelpNum;

  puts("creating header file");

  if ((HeaderFile = fopen("help.h","wt")) == NULL) {
    puts("Error opening header file");
    exit(0);
  }

  fseek(TextFile,0,SEEK_SET);
  gettext();
  HelpNum = 1;

  while (gettext() == NORMAL) {
    fprintf(HeaderFile,"#define %-14.14s %4d  /* %s */\n",InText,HelpNum,&InText[14]);
    InText[16] = 0;
    HelpNum += atoi(&InText[14]);
  }

  p = (char *) TableArray;

  fprintf(HeaderFile,"#define VERIFYSTR   0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x",
          *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7), *(p+8), *(p+9));

  fclose(HeaderFile);
}

int main(void) {
  opentextfile();
  openhelpfile();
  counttableentries();
  fillarrays();
  processhelpfile();
  writetablearray();
  makeheaderfile();
  fclose(HelpFile);
  fclose(TextFile);
  return(0);
}


