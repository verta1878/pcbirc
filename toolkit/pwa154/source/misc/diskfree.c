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
  #include <direct.h>
  #define df_bsec   bytes_per_sector
  #define df_sclus  sectors_per_cluster
  #define df_total  total_clusters
  #define df_avail  avail_clusters
#else
  #include <dir.h>
#endif

#include <dos.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "dosfunc.h"
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/* the OS/2 libraries under Watcom don't have the _dos_getdiskfree() function */
#if defined(__OS2__)
#define  INCL_DOSFILEMGR
#include <os2.h>

typedef struct {
  unsigned total_clusters;
  unsigned avail_clusters;
  unsigned sectors_per_cluster;
  unsigned bytes_per_sector;
} diskfree;

unsigned dosgetdiskfree(unsigned drive, diskfree *dtable) {
  FSALLOCATE info;

  if (DosQueryFSInfo(drive, FSIL_ALLOC, &info, sizeof(info)) != 0) {
    memset(dtable,0,sizeof(dtable));
    return(-1);
  } else {
    dtable->total_clusters = (unsigned)info.cUnit;
    dtable->avail_clusters = (unsigned)info.cUnitAvail;
    dtable->sectors_per_cluster = (unsigned)info.cSectorUnit;
    dtable->bytes_per_sector = (unsigned)info.cbSector;
    return (0);
  }
}
#endif


unsigned long LIBENTRY diskfreespace(char *Path) {
  char   Exist;
  char   DriveToCheck;
  char   *p;
  unsigned long BytesPerCluster;
  char   PathToCheck[60];
  char   SavePath[60];
  #ifdef __OS2__
    diskfree Rec;
  #else
    struct dfree Rec;
  #endif
  #ifdef TEST
    char   Str[20];
  #endif

  strupr(Path);
  if (Path[1] == ':') {
    DriveToCheck = (char) (Path[0] - 'A' + 1);  /* Drive A = 1 */
    strcpy(PathToCheck,Path);
    if (strlen(PathToCheck) > 3)     /* strip trailing backslash *unless* */
      stripright(PathToCheck,'\\');  /* it is for the ROOT directory      */
  } else {
    strcpy(&PathToCheck[2],Path);
    DriveToCheck = dosgetcurdrive();
    PathToCheck[0] = (char) (DriveToCheck + 'A' - 1);
    PathToCheck[1] = ':';
  }

  /* is it a FILE instead of a directory? */
  if ((Exist = fileexist(PathToCheck)) != 255 && (Exist & 16) != 16) {
    /* yes, it's a file, so strip it down to just the path */
    if ((p = strrchr(PathToCheck,'\\')) != NULL || (p = strrchr(PathToCheck,':')) != NULL)
      *p = 0;
  }

/*SavePath[0] = (char) (DriveToCheck + 'A' - 1); */
/*SavePath[1] = ':'; */
/*SavePath[2] = '\\'; */
/*dosgetcurpath(DriveToCheck,&SavePath[3],sizeof(SavePath)-3); */
  dosgetcurpath(DriveToCheck,SavePath,sizeof(SavePath));
  chdir(PathToCheck);

  #ifdef __OS2__
    dosgetdiskfree(DriveToCheck,&Rec);
  #elif defined(__BORLANDC__) || defined(__TURBOC__)
    getdfree(DriveToCheck,&Rec);
  #endif

  chdir(SavePath);

  #ifndef __OS2__
    if (Rec.df_sclus == 0xffff)
      return(0);
  #endif

  #ifdef __OS2__
    BytesPerCluster = (unsigned long) Rec.bytes_per_sector * (unsigned long) Rec.sectors_per_cluster;
  #else
    BytesPerCluster = (unsigned long) Rec.df_bsec * (unsigned long) Rec.df_sclus;
  #endif

#ifdef TEST
  printf("\nDrive Space Statistics for Drive %c:\n\n",DriveToCheck+'A'-1);

  printf("Bytes Per Sector   : %13s\n",comma(Str,Rec.df_bsec));
  printf("Sectors Per Cluster: %13s\n",comma(Str,Rec.df_sclus));
  printf("Bytes Per Cluster  : %13s\n\n",comma(Str,BytesPerCluster));

  printf("Total Clusters     : %13s\n",comma(Str,Rec.df_total));
  printf("Avail Clusters     : %13s\n\n",comma(Str,Rec.df_avail));

  printf("Total Disk Space   : %13s\n",comma(Str,(long) (Rec.df_total * BytesPerCluster)));
  printf("Avail Disk Space   : %13s\n",comma(Str,(long) (Rec.df_avail * BytesPerCluster)));
#endif

  #ifdef __OS2__
    return((unsigned long) (Rec.avail_clusters * BytesPerCluster));
  #else
    return((unsigned long) (Rec.df_avail * BytesPerCluster));
  #endif
}



#ifdef TEST
void main(int argc, char **argv) {
  if (argc != 2)
    return;

  diskfreespace(argv[1]);
}
#endif
