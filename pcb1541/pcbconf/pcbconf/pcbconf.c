/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* pcbconf - Import FidoNet echo/file-list into PCBoard config               */
/*                                                                           */
/* Bulk-loads a .NA (active) or .NO (inactive) FidoNet list into the         */
/* PCBoard conference / file-area database, starting at a given conference   */
/* number. Saves a sysop the tedium of setting up feeds one at a time in     */
/* PCBoard's config UI.                                                      */
/*                                                                           */
/* Targets: pwa153, pwa154 (delta154), 1541. Same source, same switches.     */
/* Features are added forward from 1541. Older targets stay lean.            */
/*                                                                           */
/* Copyright (C) 2026  pcbirc crew.  All Rights Reserved.                    */
/*                                                                           */
/* This program is free software: you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation, either version 3 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details:                              */
/* <http://www.gnu.org/licenses/gpl-3.0.html>.                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dosfunc.h"           /* pwa153/H - dosopencheck, dosclose, etc. */
#include "misc.h"              /* pwa153/H - maxstrcpy, findfour, etc.    */
#include "pcb.h"               /* pwa153/H - PcbData, conference records  */

enum {OFF = 0, ON = 1};


/* --------------------------------------------------------------------- */
/* Command-line state.                                                   */
/* --------------------------------------------------------------------- */

long StartConf = -1;            /* /CONF= starting conference number     */
bool NaFlag    = FALSE;         /* /NA  active list                      */
bool NoFlag    = FALSE;         /* /NO  inactive list                    */
bool DescFlag  = FALSE;         /* /DESC use description as conf name    */
char PathBuf[128];              /* /FILE= path                           */


/* --------------------------------------------------------------------- */
/* instruct() -- print usage.                                            */
/* --------------------------------------------------------------------- */

void pascal instruct(void) {
static char *Str =
      "\r\n"
      "Usage:  PCBCONF /FILE=<path> /CONF=<n> /NA|/NO [/DESC]\r\n"
      "\r\n"
      "where:  /FILE=path  = full path to a .NA or .NO echo/file list\r\n\t"
              "/CONF=n     = starting conference number (imports walk up)\r\n\t"
              "/NA         = input is a .NA (Active) list\r\n\t"
              "/NO         = input is a .NO (iNactive) list\r\n\t"
              "/DESC       = use the description field for the conference name\r\n\t"
              "              (default is the echo tag)\r\n"
      "\r\n"
      "Exactly one of /NA or /NO must be given.\r\n"
      "\r\n";

  printf("%s",Str);
}


/* --------------------------------------------------------------------- */
/* parsecmdline() -- returns 1 if args valid and ready to run, else 0.   */
/* Slash-form switches only. Case-insensitive via in-place strupr.       */
/* --------------------------------------------------------------------- */

int pascal parsecmdline(int argc, char **argv) {
  int   i;
  char *arg;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] != '/')
      continue;

    strupr(argv[i]);
    arg = argv[i];

    if (strcmp(arg,"/NA") == 0)
      NaFlag = TRUE;
    else if (strcmp(arg,"/NO") == 0)
      NoFlag = TRUE;
    else if (strcmp(arg,"/DESC") == 0)
      DescFlag = TRUE;
    else if (strncmp(arg,"/CONF=",6) == 0)
      StartConf = atol(&arg[6]);
    else if (strncmp(arg,"/FILE=",6) == 0)
      maxstrcpy(PathBuf,&arg[6],sizeof(PathBuf));
    else
      return(0);
  }

  if (NaFlag && NoFlag)    return(0);       /* mutually exclusive       */
  if (!NaFlag && !NoFlag)  return(0);       /* exactly one required     */
  if (PathBuf[0] == 0)     return(0);       /* /FILE= required          */
  if (StartConf < 0)       return(0);       /* /CONF= required          */

  return(1);
}


/* --------------------------------------------------------------------- */
/* readline() -- read one CRLF/LF-terminated line from a text file.      */
/* Strips line terminators. Returns 0 at EOF.                            */
/* --------------------------------------------------------------------- */

int pascal readline(FILE *fp, char *buf, int bufsize) {
  int len;

  if (!fgets(buf,bufsize,fp))
    return(0);

  len = strlen(buf);
  while (len > 0 && (buf[len-1] == '\r' || buf[len-1] == '\n'))
    buf[--len] = 0;

  return(1);
}


/* --------------------------------------------------------------------- */
/* parseline() -- split a .NA/.NO line into <tag> <description>.         */
/* Returns 1 on a real entry, 0 on blank or comment line.                */
/* --------------------------------------------------------------------- */

int pascal parseline(char *line, char *tag, int tagsize,
                     char *desc, int descsize) {
  int i, j;

  while (*line == ' ' || *line == '\t') line++;

  if (*line == 0 || *line == ';' || *line == '#')
    return(0);

  i = 0;
  while (*line && *line != ' ' && *line != '\t' && i < tagsize-1)
    tag[i++] = *line++;
  tag[i] = 0;

  if (i == 0)
    return(0);

  while (*line == ' ' || *line == '\t') line++;

  j = 0;
  while (*line && j < descsize-1)
    desc[j++] = *line++;
  desc[j] = 0;

  return(1);
}


/* --------------------------------------------------------------------- */
/* runimport() -- walk the .NA/.NO file, write one conference per line.  */
/* --------------------------------------------------------------------- */

int pascal runimport(void) {
  FILE *fp;
  char  line[512];
  char  tag[64];
  char  desc[128];
  char  confname[64];
  long  confnum;
  int   imported;

  fp = fopen(PathBuf,"rt");
  if (!fp) {
    printf("pcbconf: cannot open %s\n",PathBuf);
    return(1);
  }

  confnum  = StartConf;
  imported = 0;

  while (readline(fp,line,sizeof(line))) {
    if (!parseline(line,tag,sizeof(tag),desc,sizeof(desc)))
      continue;

    if (DescFlag && desc[0])
      maxstrcpy(confname,desc,sizeof(confname));
    else
      maxstrcpy(confname,tag,sizeof(confname));

    /* ================================================================ */
    /* TODO: wire to pwa153 toolkit conference-DB write.                */
    /*                                                                  */
    /*   1. openconfig()   -- CNAMES with proper share modes            */
    /*                        (dosopencheck / OPEN_WRIT | OPEN_DENYWRIT)*/
    /*   2. seekconf(confnum) -- position to record                     */
    /*   3. writeconf(confname, tag, desc, NaFlag)                      */
    /*                        -- name, echo tag, description, active fl */
    /*   4. closeconfig()                                               */
    /*                                                                  */
    /* Header defs: toolkit/pwa153/H/PCBDATA.H (conftype), NEWDATA.H.   */
    /* Reference: MUTIL Import_FIDONET.NA loop shape                    */
    /*   (mysticbbsirc/mystic/mutil_importna.pas).                      */
    /* ================================================================ */

    printf("  [%ld] %-20s %s\n",confnum,tag,confname);
    imported++;
    confnum++;
  }

  fclose(fp);
  printf("pcbconf: %d entries processed.\n",imported);
  return(0);
}


/* --------------------------------------------------------------------- */
/* main()                                                                */
/* --------------------------------------------------------------------- */

int main(int argc, char **argv) {
  if (parsecmdline(argc,argv) == 0) {
    instruct();
    return(1);
  }

  return(runimport());
}
