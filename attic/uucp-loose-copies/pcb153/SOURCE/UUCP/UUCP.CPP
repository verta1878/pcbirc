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



/******************************************************************************/

#include    <_video.h>

#undef _Cdecl
#undef _CType

#include    <conio.h>
#include    <ctype.h>
#include    <dos.h>
#include    <string.h>
//#include    <time.h>

#include    <cnameidx.h>
#include    <validate.h>
#include    <system.h>

#include    <uucp.hpp>

#ifdef ___USE_VMDATA___
#include    <vmdata.h>
#endif

//#include    <memcheck.h>

/******************************************************************************/

// extern "C" {

// int  pascal bgetkey(char Option);  /* same as Borland's bioskey() */

// }

/******************************************************************************/
extern unsigned _stklen = 0x8000;

// char * DatFile = "PCBOARD.DAT";

/******************************************************************************/

//     bool uucpRole;
       char uucpSystem  [   8 + 1 ];
       int  uucpDebug;
       bool uucpQuiet;
//     long uucpRate;
//     int  uucpPort;
//     bool uucpAnon;
//     bool uucpRevCon;
       int  uucpResTime;
       char uucpName    [  16 + 1 ];
       bool uucpOffHook;
       char uucpConfs   [ 128 + 1 ];
       bool uucpForce;
       long uucpFileSize;
       bool uucp7E1;
       bool uucpNoJunk;
       bool uucpLongTo;
       bool uucpDecode;
static bool uucpWait;
       int  uucpBusyTries;
       bool uucpCurDate;
       bool uucpGiveup;
       char uucpGrade;
       int  uucpWaitDays;

/******************************************************************************/


static int near pascal checkOption(char * & p, char * o, int & i,
    int c, char * v [], bool isSwitch = FALSE)
{
    // Assume match not made
    p = NULL;

    // If the match was made on all three bytes (including NUL terminator)
    if (memicmp(v[i],o,3) == 0)
    {
        // Set p to next command line option
        if (isSwitch)
            p = v[i]+2;
        else
            p = v[++i];
    }
    // If only the first two bytes match
    else if (memicmp(v[i],o,2) == 0)
        // Set p to data following switch
        p = v[i]+2;

    // If current index is greater or equal to count of indexes, no data
    if (i >= c) p = NULL;

    // If we found data, return TRUE status
    return (p != NULL);
}

    /*--------------------------------------------------------------------*/

#define ifChkOpt(l,s,is) if (checkOption(p, "-" l, i, _argc, _argv, is)) s

static void near pascal initOptions(void)
{
 // uucpRole      = 1;
    uucpSystem[0] = NUL;
    uucpDebug     = 0;
    uucpQuiet     = FALSE;
 // uucpRate      = 0;
 // uucpPort      = 1;
 // uucpAnon      = FALSE;
 // uucpRevCon    = FALSE;
    uucpResTime   = 30;
    maxstrcpy(uucpName,PcbData.uucpName,sizeof(uucpName));
    uucpOffHook   = FALSE;
    sprintf(uucpConfs,"0-%u",PcbData.NumConf);
    uucpForce     = FALSE;
    uucpFileSize  = 64L;
    uucp7E1       = FALSE;
    uucpNoJunk    = FALSE;
    uucpWait      = FALSE;
    uucpLongTo    = TRUE;
    uucpDecode    = TRUE;
    uucpBusyTries = 0;
    uucpCurDate   = FALSE;
    uucpGiveup    = FALSE;
    uucpGrade     = 0;
    uucpWaitDays  = 7;

    for (int i = 1; i < _argc; ++i)
    {
        char * p;

        // Note: Only the first uncommented in this list should not have else
        //       All others (commented or not first uncommented) should
             ifChkOpt("7",uucp7E1       = TRUE,                        TRUE);  // Force comm port IO to 7E1 instead of 8N1
        else ifChkOpt("a",uucpFileSize  = atol(p),                     FALSE); // set the file size for UUENCODING
     // else ifChkOpt("a",uucpAnon      = TRUE,                        TRUE);  // Allow Anonymous
     // else ifChkOpt("b",uucpRate      = atol(p),                     FALSE); // BPS Port Rate
        else ifChkOpt("b",uucpBusyTries = atoi(p),                     FALSE); // Busy Retry Attempts
        else ifChkOpt("c",maxstrcpy(uucpConfs, p,sizeof(uucpConfs)),   FALSE); // List of conferences to export
     // else ifChkOpt("d",uucpPort      = atoi(p),                     FALSE); // Device (1 = COM1, 2 = COM2)
        else ifChkOpt("d",uucpCurDate   = TRUE,                        TRUE);  // Use Current Date instead of Message Date
        else ifChkOpt("f",uucpForce     = TRUE,                        TRUE);  // Force import of email regardless of username (UUIN) or Force short from name to alias.out if needed (UUOUT)
       #if defined(___UUIN___)
        else ifChkOpt("g",uucpGiveup    = TRUE,                        TRUE);  // Giveup extra time slices
       #else
        else ifChkOpt("g",uucpGrade     = *p,                          FALSE); // Grading of files desired
       #endif
        else ifChkOpt("j",uucpNoJunk    = TRUE,                        TRUE);  // Turns off junk article importing
        else ifChkOpt("l",uucpLongTo    = FALSE,                       TRUE);  // Turns off long to name keeping
        else ifChkOpt("n",maxstrcpy(uucpName,  p,sizeof(uucpName)),    FALSE); // Temporary UUCP Name
        else ifChkOpt("o",uucpOffHook   = TRUE,                        TRUE);  // Take modem off hook on exit (or aliased to uucpJunkOut to create a log of junk newsgroups)
        else ifChkOpt("p",uucpWait      = TRUE,                        TRUE);  // Pause after deinitialization
        else ifChkOpt("q",uucpQuiet     = TRUE,                        TRUE);  // Quiet
     // else ifChkOpt("r",uucpRole      = (atoi(p) ? 1 : 0),           FALSE); // Role (0 = dialin, 1 = dialout)
        else ifChkOpt("s",maxstrcpy(uucpSystem,p,sizeof(uucpSystem)),  FALSE); // System (all, any, sysname)
        else ifChkOpt("t",uucpResTime   = atoi(p),                     FALSE); // Time for Response (def = 30 secs)
        else ifChkOpt("u",uucpDecode    = FALSE,                       TRUE);  // Disable automatic uudecode of encoded messages
       #if defined(___UUIN___)
        else ifChkOpt("w",uucpWaitDays  = atoi(p),                     FALSE); // Number of days to wait for multi-part uuencoded files before auto-importing files
       #else
     // else ifChkOpt("w",uucpRevCon    = TRUE,                        TRUE);  // Allow Reverse Connections
       #endif
        else ifChkOpt("x",uucpDebug     = atoi(p),                     FALSE); // Debug Level (0 - 4 - ?)
    }

    // we need to decrease the value by 30 percent for expansion purposes.
    // this will make a 64k segment 1024 lines in length or a number of
    // 45 * 1024.
    uucpFileSize = (long) (uucpFileSize * .7);
    char *p;

    for (p = uucpConfs; *p; ++p)
        if (*p == ' ')
            *p = ',';

    for (p = uucpConfs; *p; ++p)
        if ((*p == ',') && (*(p+1) == ','))
        {
            strcpy(p,p+1);
            --p;
        }

    stripboth(uucpConfs,',');

    if (uucpBusyTries < 0) uucpBusyTries = 0;
    if (uucpFileSize < 45L) uucpFileSize = 45L;
}

    /*--------------------------------------------------------------------*/

#ifdef ___USE_VMDATA___
// extern "C"
void pascal loadcnames(unsigned char Setup);
#endif

void pascal initialize(void)
{
   #ifdef ___USE_VMDATA___
    VMDataStartUp("CISWAP.TMP", 32, 64, VM_FALSE);
    VMDebugOn(VM_MURPHY|VM_SANITY_CHECK|VM_INFO);
   #endif

    pcbinit();

    if ((PcbData.uucpSeparator[0] == NUL) || (PcbData.uucpSeparator[0] == ' '))
        PcbData.uucpSeparator[0] = '.';

  #ifdef ___USE_CNAMES___
// #ifdef ___USE_VMDATA___
//  loadcnames(1);
// #else
//  opencnames();
// #endif
    opencnames();
  #endif
  #ifdef ___USE_USERS___
    openusersfile();
    openusersinffile();
  #endif

    initOptions();

    _video.displayptr.p = (char far *) Scrn_Addr;

    randomize();
}

    /*--------------------------------------------------------------------*/

void pascal deinitialize(void)
{
  #ifdef ___USE_USERS___
    closeusersinffile();
    closeusersfile();
  #endif
  #ifdef ___USE_CNAMES___
    closecnames();
  #endif

   #ifdef ___USE_VMDATA___
    VMDataShutDown();
   #endif

    if (uucpWait) bgetkey(0);
}

    /*--------------------------------------------------------------------*/

void pascal munge(char * d, char * s, char * n)
{
    // Note: d is the destination, s is the source, n is the name

    if ((s[1] != '.') || ((s[0] != 'X') && (s[0] != 'D')))
    {
        // move past any backslashes
        char * p1 = strrchr(s,'\\');
        if (p1 == NULL) p1 = s;
        // move past any forward slashes
        char * p2 = strrchr(p1,'/');
        if (p2 == NULL) p2 = p1;
        // copy the string
        strcpy(d,p2);
    }
    else
    {
        // String used for bit-mask to hexa-tri-decimal conversions
        char * base36 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        // Skip file type and dot
        char * p;
        switch (*s)
        {
         // case 'D':
         //     p = s + 2 + 5;
         //     break;
         //
         // case 'X':
         //     p = s + 2 + 7;
         //     break;

            default:
                p = s + 2;
                // while not the end of the string and characters match, skip
                while (*n && (toupper(*p) == toupper(*n)))
                {
                    ++p;
                    ++n;
                }
                break;
        }

        // Copy codes to destination string
        strcpy(d+1,(strlen(p) > 7) ? (p+strlen(p)-7) : p);

        // Build a bit-mask from remainder of string
        unsigned bitmask = 0;
        while (*p)
        {
            bitmask <<= 1;
            if (islower(*p)) bitmask |= 1;
            ++p;
        }

        // Insert appropriate munge character (using only last five bits)
        d[0] = base36[bitmask & 0x1F];

        // Add appropriate extension
        addchar(d,'.');
        addchar(d,*s);
    }

    // Force to uppercase for "standardization"
    strupr(d);
}

    /*--------------------------------------------------------------------*/

char * pascal maxstr(char * s, int l)
{
    const int max = 256;
    static char buf [ max + 1 ];
    if (l > max) return s;
    memcpy(buf,s,l);
    buf[l] = NUL;
    return buf;
}

    /*--------------------------------------------------------------------*/

unsigned pascal dstrlen(char * s)
{
    unsigned len = 0;
    while (*s)
    {
        if (*s == '\x09') // if it is a tab
            len = ((len/8)+1)*8;
        else
            ++len;
        ++s;
    }
    return len;
}

    /*--------------------------------------------------------------------*/

void pascal dstrcpy(char * d, char * s, unsigned l)
{
    unsigned len = 0;
    while (*s && (len < l))
    {
        *d = *(s++);
        if (*d == '\x09') // if it is a tab
        {
            unsigned tmp = ((len/8)+1)*8;
            if (tmp > l) break;
            len = tmp;
        }
        else
            ++len;
        ++d;
    }
    d[0] = NUL;

}

    /*--------------------------------------------------------------------*/

int pascal validatefile(char * path, char * result)
{
    int stat = validatepath(NULL,path,result,VALIDATE_FILE);
    if (stat == ERRNOTFOUND) stat = 0;
    return stat;
}

    /*--------------------------------------------------------------------*/

int pascal validatedir(char * path, char * result)
{
    int stat = validatepath(NULL,path,result,VALIDATE_DIR);
    if (stat == ERRNOTFOUND) stat = 0;
    return stat;
}

    /*--------------------------------------------------------------------*/

#ifdef ___UUIN___

bool pascal dosreservedword(char *Name);

bool pascal validfile(char * file)
{
    char * p;

    p = strchr(file,',');
    if (p) *p = NUL;

    if (strlen(file) == 0) return FALSE;
    if (strpbrk(file,
        "\x20\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"
        "\x0C\x0D\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17"
        "\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F\x22\x2A\x2F\x3C"
        "\x3E\x3F\x7C") != NULL)
        return FALSE;

    p = strrchr(file,':');  if (p) strcpy(file,p+1);
    p = strrchr(file,'\\'); if (p) strcpy(file,p+1);
    p = strrchr(file,'/');  if (p) strcpy(file,p+1);

    // Strip leading periods as they are invalid under dos
    stripleft(file,'.');

    // If we stripped the file down to nothing, it's invalid
    if (file[0] == NUL) return FALSE;

    p = strchr(file,'.');
    if (p)
    {
        // If the base name is more than 8 chars long, truncate it
        if ((p-file) > 8) strcpy(file+8,p), p = file+8;

        // If the ".EXT" part is greater than 4 chars long, truncate it
        if (strlen(p) > 4) p[4] = NUL;

        // If there is another period after the first, truncate it
        p = strchr(p+1,'.');
        if (p) *p = NUL;
    }
    else if (strlen(file) > 8)
    {
        file[8] = NUL;
    }

    strupr(file);

    return !dosreservedword(file);
}

#endif

    /*--------------------------------------------------------------------*/

void pascal maxstrcat ( char * Dest, char * Srce, int Max )
{
    int len = strlen(Dest);
    if ((len+1) > Max) len = Max-1;
    maxstrcpy(Dest+len,Srce,Max-len);
}

    /*--------------------------------------------------------------------*/

static int aliasLoaded = 0;

const int listItemLen = 64;

struct sALIASIN
{
    char    source  [ listItemLen + 1 ];
    char    dest    [ listItemLen + 1 ];
};

const int maxAliasItems = 128;

static int      aliasItems = 0;
static sALIASIN aliasList [ maxAliasItems ] ;

static void near pascal loadAliasIn ( void )
{
    if (!aliasLoaded)
    {
        aliasLoaded = 1;

        char aliasName [ 128 + 1 ];
        buildstr(aliasName,PcbData.uucpPath,"ALIAS.IN",NULL);

        cDOSFILE aliasFile;
        if (exist(aliasName) &&
            (aliasFile.open(aliasName,OPEN_READ|OPEN_DENYNONE) == 0))
        {
            char aliasItem [ listItemLen*2+1 ];

            while ((aliasFile.getln(aliasItem,sizeof(aliasItem)) != -1) &&
                (aliasItems < maxAliasItems))
            {
                strlwr(aliasItem);
                char * sptr = aliasItem+strspn(aliasItem," \t");
                char * dptr = strchr(sptr,',');
                if (dptr)
                {
                    char * tptr = dptr;
                    dptr += strspn(dptr,", \t");
                    *tptr = NUL;
                    strcpy(aliasList[aliasItems].source,sptr);
                    strcpy(aliasList[aliasItems].dest,  dptr);
                    stripright(aliasList[aliasItems].source,' ');
                    stripright(aliasList[aliasItems].dest,  ' ');
                    stripright(aliasList[aliasItems].source,'\t');
                    stripright(aliasList[aliasItems].dest,  '\t');
                    ++aliasItems;
                }
            }

            aliasFile.close();
        }
    }
}

    /*--------------------------------------------------------------------*/

void pascal parseName(char * nname, char * sname, unsigned ssize,
    char * lname, unsigned lsize, int lookupAlias)
{
    char * parenPtr = strchr(nname,'(');
    char * lthanPtr = strchr(nname,'<');

    if (nname == NULL)
    {
        maxstrcpy(sname,"ALL",ssize);
        lname[0] = NUL;
    }
    else if ((parenPtr != NULL) && (lthanPtr != NULL))
    {
        maxstrcpy(sname,parenPtr+1,ssize);
        strTrunc(sname,')');
        maxstrcpy(lname,lthanPtr+1,lsize);
        strTrunc(lname,'>');
    }
    else if (parenPtr != NULL) // long name starts, short name in ()
    {
        maxstrcpy(lname,nname,lsize);
        strTrunc(lname,'(');
        maxstrcpy(sname,parenPtr+1,ssize);
        strTrunc(sname,')');
    }
    else if (lthanPtr != NULL) // short name starts, long name in <>
    {
        maxstrcpy(sname,nname,ssize);
        strTrunc(sname,'<');
        maxstrcpy(lname,lthanPtr+1,lsize);
        strTrunc(lname,'>');
    }
    else
    {
        maxstrcpy(lname,nname,lsize);
        sname[0] = NUL;
    }

    stripboth(sname,' ');
    stripboth(sname,'"');
    stripboth(lname,' ');
    stripboth(lname,'"');

    if ((sname[0] == NUL) && (lname[0] != NUL))
    {
        char * p = strrchr(lname,'!');
        if (p == NULL)
            p = lname;
        else
            ++p;

        maxstrcpy(sname,p,ssize);
        strTrunc(sname,'@');
        strTrunc(sname,'%');
    }
    else if ((lname[0] == NUL) && (sname[0] != NUL))
    {
        maxstrcpy(lname,sname,lsize);
    }

    strlwr(sname);
    int snamelen = strlen(sname)+1;

    if (lookupAlias)
    {
        if (!aliasLoaded) loadAliasIn();

        int aliasItem;

        for (aliasItem = 0; aliasItem < aliasItems; ++aliasItem)
        {
            if (memcmp(aliasList[aliasItem].source,sname,snamelen) == 0)
                break;
        }

        if (aliasItem < aliasItems)
        {
            maxstrcpy(sname,aliasList[aliasItem].dest,ssize);
            stripboth(sname,' ');
            stripboth(sname,'"');
        }
    }

    if (isalnum(sname[0]) && (strchr(sname,'@') == NULL))
    {
        sepsToSpaces(sname);
        strupr(sname);
        if (strcmp(sname,lname) == 0) lname[0] = NUL;
    }
}

    /*--------------------------------------------------------------------*/

void pascal sepsToSpaces ( char * s )
{
    while (*s)
    {
        if ((*s == PcbData.uucpSeparator[0]) || (*s == '.') || (*s == '_'))
            if (!((s[0] == '.') && (s[1] == '.')))
                *s = ' ';
        ++s;
    }

 // change(s,PcbData.uucpSeparator[0],' ');
 // change(s,'.',                     ' ');
 // change(s,'_',                     ' ');
}

/******************************************************************************/

