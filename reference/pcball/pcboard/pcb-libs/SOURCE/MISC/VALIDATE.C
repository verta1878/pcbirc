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


#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <dos.h>
  #include <direct.h>
#else
  #include <dir.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dosfunc.h>
#include "validate.h"
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/*** flowchart for validatepath() function *********************************

Entry Point<-----------------------------------------------------------+
    |                                                                  ^
    v                                                                  |
Setup Work Areas                                                       |
    |                                                                  |
Expand Path to include drive letter                                    |
and FULL path name                                                     |
    |                                                                  |
    v                                                                  |
Check for existence of file<----------------------------------------+  |
    |                                                               ^  |
    v                                                               |  |
Exist?-->Yes, Supposed to be Path?-->Yes, Is it?-->Yes, return OK   |  |
    |            |                       |                          |  |
    |No          |No, a File             |No, return ERRNOTDIR      |  |
    |            v                                                  |  |
    |        Is it a File? - Yes, return OK                         |  |
    |            |                                                  |  |
    |            |No, return ERRNOTFILE                             |  |
    |                                                               |  |
    v                                                               |  |
Supposed to be Path?-->Yes, MKDIR -->Successful?->Yes, return OK    |  |
    |                                   |                           |  |
    |No                                 |No                         |  |
    |                                   v                           |  |
    |                                   +------------------>+       |  |
    v                                                       |       |  |
Have we backed up the Path yet?-->Yes, return ERRNOTFOUND   |       |  |
    |                                                       |       |  |
    |No                                                     |       |  |
    |                                                       v       |  |
    |<------------------------------------------------------+       ^  |
    v                                                               |  |
Can Path be backed up?-->Yes, backup path, set to DIR (*)---------|->+
    |                    and call self recursively                  |
    |                         |                                     ^
    |No                       v                                     |
    v                    Upon return from recursive call... was an  |
return ERRNOTFOUND       Error returned?-->Yes, return same error   |
                              |                                     |
                              |No                                   |
                              v                                     |
                         Go back and see if file/path NOW exists--->+



****************************************************************************/


#ifdef OLDFUNCTION_USE_DRIVEOK_INSTEAD
static int _NEAR_ LIBENTRY validatedrive(char Drive) {
  char CurDrive;
  int  RetVal;

  CurDrive = (char) getdisk();
  setdisk(Drive-'A');
  RetVal = (getdisk() == Drive-'A' ? FALSE : TRUE);  /* TRUE if ERROR */
  setdisk(CurDrive);
  return(RetVal);
}
#endif



int LIBENTRY validatepath(FILE *Out, char *Path, char *ResultPath, char Choice) {
  int  Code;
  unsigned CurDrive;
  char Temp[80];
  char WorkPath[80];
  char *p;
  bool BackedUpAlready;
  char NewChoice;

  if (Path[0] == 0)
    return(0);

  BackedUpAlready = FALSE;

  strupr(Path);
  strcpy(WorkPath,Path);

  #ifdef TEST
    fprintf(Out,"\nentry point:  [%s]\n",Path);
  #endif

  if (WorkPath[0] != '\\' && !(WorkPath[1] == ':' && WorkPath[2] == '\\')) {
    CurDrive = dosgetcurdrive();

    if (WorkPath[1] == ':' && CurDrive+'A' != WorkPath[0])
      dossetcurdrive(WorkPath[0] - 'A' + 1);

    getcwd(Temp,66);
    #ifdef TEST
      fprintf(Out,"Current Path: [%s]\n",Temp);
    #endif

    dossetcurdrive(CurDrive);

    p = &Temp[strlen(Temp)-1];
    if (*p != '\\') {
      p++;
      *p = '\\';
    }
    p++;
    if (WorkPath[1] == ':')
      strcpy(p,&WorkPath[2]);
    else
      strcpy(p,WorkPath);
    strcpy(WorkPath,Temp);
    #ifdef TEST
      fprintf(Out,"Full Path: [%s]\n",WorkPath);
    #endif
  }

  /* watch for a ".\" which basically means the CURRENT directory and, if it */
  /* is "\.\" then change it to just "\" since the ".\" is redundant         */
  substitute(WorkPath,"\\.\\","\\",sizeof(WorkPath));

  p = &WorkPath[strlen(WorkPath)-1];
  if (*p == '\\') {
    if (p == &WorkPath[2]) {
      if (driveok(WorkPath[0]-'A')) {
        #ifdef TEST
          fprintf(Out,"Path is a ROOT directory - returning OK\n");
        #endif
        return(0);
      } else {
        #ifdef TEST
          fprintf(Out,"Disk drive is INVALID - returning ERRDISKINVALID\n");
        #endif
        strcpy(ResultPath,WorkPath);
        return(ERRDISKINVALID);
      }
    }
    *p = 0;
  }

  strcpy(ResultPath,WorkPath);

Retry:
  Code = fileexist(WorkPath);
  #ifdef TEST
    fprintf(Out,"fileexist() return code: %d\n",Code);
  #endif

  if (Code != 255) {
    switch (Choice) {
      case CHECK_DIR    :
      case VALIDATE_DIR : if ((Code & 16) == 0) {
                            #ifdef TEST
                              if (Out != NULL)
                                fprintf(Out,"ERROR: tried to make DIRECTORY \"%s\"-already exists as a FILE\n",WorkPath);
                            #endif
                            return(ERRNOTDIR); /* should be a DIR, was file instead */
                          } else {
                            #ifdef TEST
                              fprintf(Out,"returning OK - VALIDATE_DIR, Code == 16\n");
                            #endif
                            return(0);
                          }
      case CHECK_FILE   :
      case VALIDATE_FILE: if ((Code & 16) != 0) {
                            #ifdef TEST
                              if (Out != NULL)
                                fprintf(Out,"ERROR: expected \"%s\" to be a FILE-found a DIRECTORY\n",WorkPath);
                            #endif
                            return(ERRNOTFILE); /* should be a FILE, was DIR instead */
                          } else {
                            #ifdef TEST
                              fprintf(Out,"returning OK - VALIDATE_FILE, Code != 16\n");
                            #endif
                            return(0);
                          }
    }
  } else {
    switch (Choice) {
      case VALIDATE_DIR : if (mkdir(WorkPath) == -1) {
                            #ifdef TEST
                              if (Out != NULL)
                                fprintf(Out,"can't create %s, error code = %d (%s)\n",WorkPath,errno,sys_errlist[errno]);
                            #endif
                            if (validatesemantics(WorkPath) == -1) {
                              #ifdef TEST
                                fprintf(Out,"invalid path specified\n");
                              #endif
                              return(ERRPATHINVALID);
                            }
                          } else {
                            #ifdef TEST
                            if (Out != NULL)
                              fprintf(Out,"created: %s\n",WorkPath);
                            #endif
                            return(0);
                          }
                          break;
      case CHECK_DIR    : return(ERRDIRNOTFOUND);
      case VALIDATE_FILE:
      case CHECK_FILE   : if (BackedUpAlready) {
                            #ifdef TEST
                              fprintf(Out,"returning ERRNOTFOUND in middle of function\n");
                            #endif
                            return(ERRNOTFOUND);
                          } break;
    }
    strcpy(Temp,WorkPath);

    p = strrchr(Temp,'\\');
    if (*(p-1) != ':') {                    /* backup if not LAST PATH */
      #ifdef TEST
        fprintf(Out,"backing up from: [%s]\n",Temp);
      #endif
      *p = 0;
      #ifdef TEST
        fprintf(Out,"backing up to: [%s]\n",Temp);
      #endif
      BackedUpAlready = TRUE;
      switch (Choice) {
        case VALIDATE_FILE : NewChoice = VALIDATE_DIR; break;
        case CHECK_FILE    : NewChoice = CHECK_DIR;    break;
        default            : NewChoice = Choice;       break;
      }
      Code = validatepath(Out,Temp,ResultPath,NewChoice);
      if (Code) {
        #ifdef TEST
          fprintf(Out,"returning [%d] from recursive call\n",Code);
        #endif
        return(Code);
      }
      goto Retry;
    } else {
      if (driveok(Temp[0]-'A')) {
        #ifdef TEST
          fprintf(Out,"Path is a ROOT directory\n");
        #endif
        switch (Choice) {
          case VALIDATE_DIR :
          case CHECK_DIR    : return(0);
          case VALIDATE_FILE:
          case CHECK_FILE   : return(ERRNOTFOUND);
        }
      } else {
        #ifdef TEST
          fprintf(Out,"Disk drive is INVALID - returning ERRDISKINVALID\n");
        #endif
        return(ERRDISKINVALID);
      }
    }
  }
  #ifdef TEST
    fprintf(Out,"returning ERRNOTFOUND at end of function\n");
  #endif
  return(ERRNOTFOUND);
}


#ifdef TEST
void main(void) {

  char Str[80];
  char Ans[10];
  char Ans2[10];
  char Type;
  char Result[80];
  int  Code;

  printf("Enter a PATH/FILENAME to test: ");
  gets(Str);

  printf("Is this a FILE (Y/N)? ");
  gets(Ans);
  strupr(Ans);

  printf("Create missing directories (Y/N)? ");
  gets(Ans2);
  strupr(Ans2);

  switch (Ans2[0]) {
    case 'Y': Type = (Ans[0] == 'Y' ? VALIDATE_FILE : VALIDATE_DIR); break;
    case 'N': Type = (Ans[0] == 'Y' ? CHECK_FILE : CHECK_DIR);       break;
  }

  Code = validatepath(stderr,Str,Result,Type);

  printf("\n\n"
         "validatepath() passed: [%s]\n"
         "return value was     : %d\n"
         "return string was    : [%s]\n",
         Str,Code,Result);
}
#endif
