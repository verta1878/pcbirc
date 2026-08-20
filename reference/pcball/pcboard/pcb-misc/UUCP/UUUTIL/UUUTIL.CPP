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

#include    <conio.h>
#include    <ctype.h>
#include    <dos.h>

#include    <iostream.h>
#include    <iomanip.h>

//#include    <misc.h>
#include    <cnameidx.h>

#include    <fidocfg.hpp>

#include    <uucp.hpp>

/******************************************************************************/

unsigned sConf = 1;
unsigned tConf = 0;
// int      useAREATAG = 0;

char mPath [ 128 + 1 ];

/******************************************************************************/

    /**********************************************************************/
    /*                                                                    */
    /* Class Hierarchy                                                    */
    /* ~~~~~~~~~~~~~~~                                                    */
    /* cDOSFILE                  ; DWT's DOSFILE Streams in Class Format  */
    /*     cTEXTDATA             ; Line Oriented Text Data                */
    /*         cFIELD_TD         ; Field Oriented Text Data               */
    /*             cDOS_FTD      ; DOS Field Text Data                    */
    /*                 cDEF_DFTD ; Default DOS Field Text Data            */
    /*                 cFID_DFTD ; FIDO DOS Field Text Data               */
    /*             cUNIX_FTD     ; UNIX Field Text Data                   */
    /*                 cACT_UFTD ; Active UNIX Field Text Data            */
    /*                 cMOD_UFTD ; Moderators UNIX Field Text Data        */
    /*                                                                    */
    /**********************************************************************/

const int MAX_FIELDS    = 10;
const int MAX_LINE_SIZE = 256;

class cTEXTDATA : public cDOSFILE
{

    public:

        cTEXTDATA ( char * name, int flags );

        virtual int  pascal readrec  ( void ) = 0;
        virtual void pascal unfmt    ( void ) = 0;
        virtual void pascal fmt      ( void ) = 0;
        virtual int  pascal writerec ( void ) = 0;

        int pascal get ( void ) { int r = readrec(); unfmt(); return r; }
        int pascal put ( void ) {                      fmt(); return writerec(); }

        char   field [ MAX_FIELDS ] [ MAX_LINE_SIZE + 1 ];

    protected:

        char   line  [ MAX_LINE_SIZE + 1 ];

    private:

};

cTEXTDATA::cTEXTDATA(char * name, int flags)
{
    open(name,flags);
    for (int i = 0; i < MAX_FIELDS; ++i) field[i][0] = NUL;
    line[0] = NUL;
}

class cFIELD_TD : public cTEXTDATA
{

    public:

        cFIELD_TD ( char * name, int flags ) :
            cTEXTDATA(name,flags)
        {
        }

 //     int  pascal readrec  ( void ) = 0;
 //     void pascal unfmt    ( void ) = 0;
 //     void pascal fmt      ( void ) = 0;
 //     int  pascal writerec ( void ) = 0;

        void pascal def_unfmt ( char tok );
        void pascal def_fmt   ( char tok );

    protected:

    private:

};

void pascal cFIELD_TD::def_unfmt(char tok)
{
    char _tok [ 2 ] = { tok, NUL };
    char * p = strtok(line,_tok);
    for (int i = 0; i < MAX_FIELDS; ++i)
    {
        if (p)
            strcpy(field[i],p);
        else
            field[i][0] = NUL;
        p = strtok(NULL,_tok);
    }
}

void pascal cFIELD_TD::def_fmt(char tok)
{
    line[0] = NUL;
    for (int i = 0; i < MAX_FIELDS; ++i)
    {
        strcat(line,field[i]);
        addchar(line,tok);
    }
    stripright(line,tok);
}

class cDOS_FTD : public cFIELD_TD
{

    public:

        cDOS_FTD ( char * name, int flags ) :
            cFIELD_TD(name,flags)
        {
        }

        int  pascal readrec  ( void ) { return getln(line,sizeof(line)); }
 //     void pascal unfmt    ( void ) = 0;
 //     void pascal fmt      ( void ) = 0;
        int  pascal writerec ( void ) { return putln(line); }

    protected:

    private:

};

class cUNIX_FTD : public cFIELD_TD
{

    public:

        cUNIX_FTD ( char * name, int flags ) :
            cFIELD_TD(name,flags)
        {
        }

        int  pascal readrec  ( void ) { return getuln(line,sizeof(line)); }
 //     void pascal unfmt    ( void ) = 0;
 //     void pascal fmt      ( void ) = 0;
        int  pascal writerec ( void ) { return putuln(line); }

    protected:

    private:

};

class cDEF_DFTD : public cDOS_FTD
{

    public:

        cDEF_DFTD ( char * name, int flags ) :
            cDOS_FTD(name,flags)
        {
        }

        void pascal unfmt ( void ) { def_unfmt(','); }
        void pascal fmt   ( void ) { def_fmt  (','); }

    protected:

    private:

};

class cFID_DFTD : public cDOS_FTD
{

    public:

        cFID_DFTD ( char * name, int flags ) :
            cDOS_FTD(name,flags)
        {
        }

        void pascal unfmt ( void );

        void pascal fmt   ( void )
        {
            sprintf(line,"%-20.20s%s",field[0],field[1]);
        }

    protected:

    private:

};

void pascal cFID_DFTD::unfmt ( void )
{
    stripleft(line,' ');

    memcpy(field[0],line,20);
    field[0][20] = NUL;
    char * ptr = strchr(field[0],' ');
    if (ptr) *ptr = NUL;

    ptr = strchr(line,' ');
    strcpy(field[1], ptr ? ptr : "");
    stripboth(field[1],' ');

    for (int i = 2; i < MAX_FIELDS; ++i) field[i][0] = NUL;
}

class cACT_UFTD : public cUNIX_FTD
{

    public:

        cACT_UFTD ( char * name, int flags ) :
            cUNIX_FTD(name,flags)
        {
        }

        void pascal unfmt ( void ) { def_unfmt(' '); }
        void pascal fmt   ( void ) { def_fmt  (' '); }

    protected:

    private:

};

class cMOD_UFTD : public cUNIX_FTD
{

    public:

        cMOD_UFTD ( char * name, int flags ) :
            cUNIX_FTD(name,flags)
        {
        }

        void pascal unfmt ( void ) { def_unfmt(':'); }
        void pascal fmt   ( void ) { def_fmt  (':'); }

    protected:

    private:

};

/******************************************************************************/

static int allHit = FALSE;

static bool near pascal confirm(char * msg)
{
    allHit = FALSE;

    char a;

    cout << endl << msg << "?  ";
    a = getche();

    if (toupper(a) == 'A')
    {
        allHit = TRUE;
        a = 'Y';
    }

    return (toupper(a) == 'Y');
}

static void near pascal mangleNameToPath(char * name, char * path)
{
    strcpy(path,PcbData.uucpPath);

    char tmp [ 128 ]; // Need this much space for later
    strcpy(tmp,name);

    char * tok = strtok(tmp," .,;");
    while (tok != NULL)
    {
        addchar(path,*tok);
        addchar(path,'\\');
        tok = strtok(NULL," .,;");
    }

    path[strlen(path)-2] = NUL;

    tok = strrchr(name,'.');
    strcpy(tmp,tok ? tok : "");
    tok = tmp;
    while (*tok != NUL)
    {
        if (isascii(*tok) && isalnum(*tok))
            ++tok;
        else
            strcpy(tok,tok+1);
    }

    if (strlen(tmp) == 0)
        strcpy(tmp,"MSGS");
    else
        tmp[5] = NUL;

    strcat(path,tmp);

    int seq = 0;

    do {

        sprintf(tmp,"%s%03d",path,++seq);

    } while (fileexist(tmp) != 255);

    strupr(strcpy(path,tmp));

    if (validatefile(path,tmp) != 0) return;

    int h = doscreatecheck(path,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);

    if (h > 0)
    {
        memset(tmp,0x20,sizeof(tmp));
        memset(tmp,0x00,12);
        writecheck(h,tmp,sizeof(tmp));
        dosclose(h);
    }
}

#ifdef FIDO

static void near pascal mangleFNameToPath(char * name, char * path)
{
    strcpy(path,mPath);

    char tmp [ 128 ]; // Need this much space for later
    strcpy(tmp,name);

    char * p;

    do {

        p = strpbrk(tmp,"/. ");
        if (p) strcpy(p,p+1);

    } while (p);

    if (strlen(tmp) == 0)
        strcpy(tmp,"MSGS");
    else
        tmp[5] = NUL;

    char subdir [ 1 + 1 + 1 ];
    subdir[0] = tmp[0];
    subdir[1] = '\\';
    subdir[2] = NUL;

    strcat(path,subdir);
    strcat(path,tmp);

    int seq = 0;

    do {

        sprintf(tmp,"%s%03d",path,++seq);

    } while (fileexist(tmp) != 255);

    strupr(strcpy(path,tmp));

    if (validatefile(path,tmp) != 0) return;

    int h = doscreatecheck(path,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL);

    if (h > 0)
    {
        memset(tmp,0x20,sizeof(tmp));
        memset(tmp,0x00,12);
        writecheck(h,tmp,sizeof(tmp));
        dosclose(h);
    }
}

#endif

static void near pascal cmdIMPORT(char * fname)
{
    cDEF_DFTD f(fname,OPEN_READ|OPEN_DENYWRIT);

    char     name [ 60 + 1 ];
    unsigned conf = sConf;

    pcbconftype rec;

    opencnamesidx(READ_IDX);

    int overWriteAll = FALSE;

    while (f.get() == 0)
    {
        maxstrcpy(name,f.field[0],sizeof(name));
        if (strlen(f.field[1]) > 0) conf = unsigned(atol(f.field[1]));
        if (conf == 0) conf = 1;

        cout << endl << "Checking conference " << conf << " ...";

        if (getconfbyname(name) >= 0)
        {
            cout << endl << "Conference named " << name << " already exists!";
            if (confirm("Skip newsgroup (Y/N)")) continue;
        }

        for (getconfrecord(conf,&rec);
             rec.Name[0] != NUL;
             getconfrecord(conf,&rec))
        {
            if (overWriteAll)
            {
                rec.Name[0] = NUL;
                break;
            }
            cout << endl << "Conference in use by (" << rec.Name << ")!";
            if      (confirm("Overwrite conference (Y/N/A)")) // overwrite
            {
                if (allHit) overWriteAll = TRUE;
                rec.Name[0] = NUL;
                break;
            }
            else if (confirm("Find next available (Y/N)"))  // next
                ++conf;
            else                                      // skip
                break;
        }

        if (rec.Name[0] == NUL)
        {
            cout << endl << "Inserting (" << name << ") as conference " <<
                conf << " ...";

            getconfrecord(tConf,&rec);

            memset(rec.Name,0,sizeof(rec.Name));
            strcpy(rec.Name,name);

         // rec.PublicConf       = TRUE;
         // rec.AutoRejoin       = FALSE;
         // rec.ViewMembers      = FALSE;
         // bool PrivUplds;
            rec.PrivMsgs         = FALSE;
            rec.EchoMail         = TRUE;
         // rec.ReqSecLevel      = 0;
         // rec.AddSec           = 0;
         // rec.AddTime          = 0;
         // rec.MsgBlocks        = 1;

            mangleNameToPath(rec.Name,rec.MsgFile);
         // char UserMenu[32];
         // char SysopMenu[32];
         // char NewsFile[32];
         // char PubUpldSort;
         // char UpldDir[29];
         // char PubUpldLoc[26];
         // char PrvUpldSort;
         // char PrivDir[29];
         // char PrvUpldLoc[26];
         // char DrsMenu[29];
         // char DrsFile[33];
         // char BltMenu[29];
         // char BltNameLoc[33];
         // char ScrMenu[29];
         // char ScrNameLoc[33];
         // char DirMenu[29];
         // char DirNameLoc[33];
         // char PthNameLoc[33];

         // rec.ForceEcho        = TRUE;
         // rec.ReadOnly         = FALSE;
         // rec.NoPrivateMsgs    = FALSE;
         // rec.RetReceiptLevel  = 255;
         // rec.RecordOrigin     = FALSE;
            rec.PromptForRouting = TRUE;
         // rec.AllowAliases     = FALSE;
         // rec.ShowIntroOnRA    = FALSE;
         // rec.ReqLevelToEnter  = 0;
         // rec.Password[0]      = NUL;
         // char Intro[32];
         // char AttachLoc[32];
         // rec.RegFlags[0]      = NUL;
         // char AttachLevel;
         // char CarbonLimit;
         // char CmdLst[32];
         // rec.OldIndex         = FALSE;
            rec.LongToNames      = TRUE;
         // char CarbonLevel;
            rec.ConfType         = UN_PUB_NEWS;
            rec.ExportPtr        = 0;

            putconfrecord(conf,&rec);
        }

        cout << endl;

        ++conf;
    }

    closecnamesidx();

    buildcnamesidx();
}

static void near pascal cmdSETNAMES(char * fname)
{
    cDEF_DFTD f(fname,OPEN_READ|OPEN_DENYWRIT);

    char     name [ 60 + 1 ];
    unsigned conf = sConf;

    pcbconftype rec;

    opencnamesidx(READ_IDX);

    while (f.get() == 0)
    {
        maxstrcpy(name,f.field[0],sizeof(name));
        if (strlen(f.field[1]) > 0) conf = unsigned(atol(f.field[1]));
        if (conf == 0) conf = 1;

        cout << endl << "Setting (" << name << ") as conference " << conf <<
            " ...";

        getconfrecord(conf,&rec);

        memset(rec.Name,0,sizeof(rec.Name));
        strcpy(rec.Name,name);

        rec.ConfType  = UN_PUB_NEWS;
        rec.ExportPtr = 0x7FFFFFFFL;

        putconfrecord(conf,&rec);

        cout << endl;

        ++conf;
    }

    closecnamesidx();

    buildcnamesidx();
}

static void near pascal cmdEXPORT(char * fname)
{
    cDEF_DFTD f(fname,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE);

    pcbconftype rec;

    for (unsigned i = 0; i <= PcbData.NumConf; ++i)
    {
        getconfrecord(i,&rec);

        if ((rec.ConfType != UN_MOD_NEWS) && (rec.ConfType != UN_PUB_NEWS))
            continue;

        cout << endl << "Exporting newsgroup " << rec.Name << " ...";

        strcpy(f.field[0],rec.Name);
        sprintf(f.field[1],"%u",i);
        f.put();
    }
}

static void near pascal cmdVALIDATE(char * fname)
{
    cDEF_DFTD f(fname,OPEN_READ|OPEN_DENYWRIT);
}

static void near pascal cmdPURGE(char * fname)
{
    cDEF_DFTD f(fname,OPEN_READ|OPEN_DENYWRIT);
}

static void near pascal cmdACTIVE(char * fname)
{
    cACT_UFTD f(fname,OPEN_READ|OPEN_DENYWRIT);

    char        name [ 60 + 1 ];
    pcbconftype rec;
    long        conf;

    opencnamesidx(READ_IDX);

    while (f.get() == 0)
    {
        maxstrcpy(name,f.field[0],sizeof(name));

        cout << endl << "Checking newsgroup " << name << " ...";

        conf = getconfbyname(name);

        if (conf == -1) continue;

        getconfrecord(unsigned(conf),&rec);

        if ((rec.ConfType != IN_JUNK) && (rec.ConfType != UN_MOD_NEWS) &&
            (rec.ConfType != UN_PUB_NEWS))
            continue;

        // Check out the newsgroup type and set conference appropriately
        switch (tolower(f.field[3][0]))
        {
            case 'm': // moderated
                rec.ConfType = UN_MOD_NEWS;
                break;

            case 'y': // public
                rec.ConfType = UN_PUB_NEWS;
                break;

            case 'x': // unknown
            case 'n': // not used
            default:
                rec.ConfType = IN_JUNK;
                break;
        }

        putconfrecord(unsigned(conf),&rec);
    }

    closecnamesidx();
}

static void near pascal cmdMODERATORS(char * fname)
{
    cMOD_UFTD f(fname,OPEN_READ|OPEN_DENYWRIT);
    cDEF_DFTD o(PcbData.uucpModFile,OPEN_WRIT|OPEN_DENYRDWR);

    char        name [ 60 + 1 ];
    pcbconftype rec;
    long        conf;

    opencnamesidx(READ_IDX);

    while (f.get() == 0)
    {
        maxstrcpy(name,f.field[0],sizeof(name));

        cout << endl << "Checking newsgroup " << name << " ...";

        conf = getconfbyname(name);

        if (conf == -1) continue;

        getconfrecord(unsigned(conf),&rec);

        if ((rec.ConfType != IN_JUNK) && (rec.ConfType != UN_MOD_NEWS) &&
            (rec.ConfType != UN_PUB_NEWS))
            continue;

        rec.ConfType = UN_MOD_NEWS;

        sprintf(o.field[0],"%5ld",conf);
        maxstrcpy(o.field[1],f.field[1],sizeof(o.field[1]));
        o.put();

        putconfrecord(unsigned(conf),&rec);
    }

    closecnamesidx();

    cout << endl << "Remember to edit & save the moderator list from "
        "PCBSetup!";
}

#ifdef FIDO

static int fidoSetNames = 0;

static void near pascal cmdFIMPORT(char * fname)
{
    cFIDOCFG fidoCfg;

    cFID_DFTD f(fname,OPEN_READ|OPEN_DENYWRIT);

    char     name [ 60 + 1 ];
    unsigned conf = sConf;

    pcbconftype rec;

    opencnamesidx(READ_IDX);

    int overWriteAll = FALSE;

    while (f.get() == 0)
    {
        maxstrcpy(name,f.field[useAREATAG ? 0 : 1],sizeof(name));
     // if (strlen(f.field[2]) > 0) conf = unsigned(atol(f.field[1]));
     // if (conf == 0) conf = 1;

        cout << endl << "Checking conference " << conf << " ...";

        if (getconfbyname(name) >= 0)
        {
            cout << endl << "Conference named " << name << " already exists!";
            if (confirm("Skip fido area (Y/N)")) continue;
        }

        for (getconfrecord(conf,&rec);
             rec.Name[0] != NUL;
             getconfrecord(conf,&rec))
        {
            if (overWriteAll)
            {
                rec.Name[0] = NUL;
                break;
            }
            cout << endl << "Conference in use by (" << rec.Name << ")!";
            if      (confirm("Overwrite conference (Y/N/A)")) // overwrite
            {
                if (allHit) overWriteAll = TRUE;
                rec.Name[0] = NUL;
                break;
            }
            else if (confirm("Find next available (Y/N)"))  // next
                ++conf;
            else                                      // skip
                break;
        }

        if (rec.Name[0] == NUL)
        {
            cout << endl << "Inserting (" << f.field[0] <<
                ") as conference " << conf << " ...";

            getconfrecord(tConf,&rec);

            memset(rec.Name,0,sizeof(rec.Name));
            strcpy(rec.Name,name);

         // rec.PublicConf       = TRUE;
         // rec.AutoRejoin       = FALSE;
         // rec.ViewMembers      = FALSE;
         // bool PrivUplds;
            rec.PrivMsgs         = FALSE;
            rec.EchoMail         = TRUE;
         // rec.ReqSecLevel      = 0;
         // rec.AddSec           = 0;
         // rec.AddTime          = 0;
         // rec.MsgBlocks        = 1;

            if (!fidoSetNames)
                mangleFNameToPath(f.field[0],rec.MsgFile);

         // char UserMenu[32];
         // char SysopMenu[32];
         // char NewsFile[32];
         // char PubUpldSort;
         // char UpldDir[29];
         // char PubUpldLoc[26];
         // char PrvUpldSort;
         // char PrivDir[29];
         // char PrvUpldLoc[26];
         // char DrsMenu[29];
         // char DrsFile[33];
         // char BltMenu[29];
         // char BltNameLoc[33];
         // char ScrMenu[29];
         // char ScrNameLoc[33];
         // char DirMenu[29];
         // char DirNameLoc[33];
         // char PthNameLoc[33];

         // rec.ForceEcho        = TRUE;
         // rec.ReadOnly         = FALSE;
         // rec.NoPrivateMsgs    = FALSE;
         // rec.RetReceiptLevel  = 255;
         // rec.RecordOrigin     = FALSE;
            rec.PromptForRouting = FALSE;
         // rec.AllowAliases     = FALSE;
         // rec.ShowIntroOnRA    = FALSE;
         // rec.ReqLevelToEnter  = 0;
         // rec.Password[0]      = NUL;
         // char Intro[32];
         // char AttachLoc[32];
         // rec.RegFlags[0]      = NUL;
         // char AttachLevel;
         // char CarbonLimit;
         // char CmdLst[32];
         // rec.OldIndex         = FALSE;
            rec.LongToNames      = FALSE;
         // char CarbonLevel;
            rec.ConfType         = FIDO_TYPE;
            rec.ExportPtr        = 0;

            putconfrecord(conf,&rec);

            fidoCfg.addNewArea(conf,f.field[0],rec.MsgFile);
        }

        cout << endl;

        ++conf;
    }

    closecnamesidx();

    buildcnamesidx();
}

static void near pascal cmdFSETNAMES(char * fname)
{
    fidoSetNames = 1;
    cmdFIMPORT(fname);
    fidoSetNames = 0;
}

#endif // FIDO

#define ifChkCmdLaunch(c)                        \
    if (memicmp(p,"/"#c":",sizeof(#c)+2-1) == 0) \
        cmd##c(p+sizeof(#c)+2-1)

static void near pascal mainLoop(void)
{
    for (int i = 1; i < _argc; ++i)
    {
        char * p = _argv[i];
             ifChkCmdLaunch(IMPORT);
        else ifChkCmdLaunch(SETNAMES);
        else ifChkCmdLaunch(EXPORT);
        else ifChkCmdLaunch(VALIDATE);
        else ifChkCmdLaunch(PURGE);
        else ifChkCmdLaunch(ACTIVE);
        else ifChkCmdLaunch(MODERATORS);
       #ifdef FIDO
        else ifChkCmdLaunch(FSETNAMES);
        else ifChkCmdLaunch(FIMPORT);
       #endif
    }
}

/******************************************************************************/

#include    <vmdata.h>

int main(void)
{
    #ifdef MEMCHECK
      mc_startcheck(erf_standard);
    #endif

    cout << endl << "UUUtil Version 1.30 -- UUCP Utility Program" << endl;

    if (_argc == 1)
    {
        cout << endl << "Usage:  UUUTIL options" << endl;
        cout << endl << "Where valid options are:" << endl;
     // cout << endl << "   /FILE:d:\\path\\pcboard.dat";
        cout << endl << "   /START:confnum";
        cout << endl << "   /TEMPLATE:confnum";
        cout << endl << "   /INDEX";
        cout << endl << "   /MSGS:d:\\path\\";
     // cout << endl << "   /USEAREA";
        cout << endl << "   /SETNAMES:d:\\path\\localng.lst";
        cout << endl << "   /IMPORT:d:\\path\\localng.lst";
        cout << endl << "   /EXPORT:d:\\path\\localng.lst";
     // cout << endl << "   /VALIDATE:d:\\path\\localng.lst";
     // cout << endl << "   /PURGE:d:\\path\\localng.lst";
        cout << endl << "   /ACTIVE:d:\\path\\active";
        cout << endl << "   /MODERATORS:d:\\path\\moderators";
        cout << endl << "   /FSETNAMES:d:\\path\\fidonet.na";
        cout << endl << "   /FIMPORT:d:\\path\\fidonet.na";
        cout << endl;

        return 1;
    }

    initialize();

    if (PcbData.UsrFile[1] == ':')
    {
        strcpy(mPath,"C:\\PCB\\FIDO\\");
        mPath[0] = PcbData.UsrFile[0];
    }
    else
        strcpy(mPath,"\\PCB\\FIDO\\");
 /*
    pcbinit();

   #ifdef ___USE_VMDATA___
    VMDataStartUp("CISWAP.TMP", 16, 16, VM_FALSE);
    VMDebugOn(VM_MURPHY|VM_SANITY_CHECK|VM_INFO);
   #endif
 */
    // Read command line parameters
    for (int i = 0; i < _argc; ++i)
    {
        char * p = _argv[i];
             if (memicmp(p,"/START:",    7) == 0) sConf = unsigned(atol(p+ 7));
        else if (memicmp(p,"/TEMPLATE:",10) == 0) tConf = unsigned(atol(p+10));
        else if (memicmp(p,"/INDEX",     6) == 0) buildcnamesidx();
        else if (memicmp(p,"/MSGS:",     6) == 0) strcpy(mPath,p+6);
     // else if (memicmp(p,"/USEAREA",   8) == 0) useAREATAG = 1;
    }
 /*
   #ifdef ___USE_VMDATA___
    VMDataShutDown();
   #endif
 */
    mainLoop();

    deinitialize();

    #ifdef MEMCHECK
      mc_endcheck();
    #endif

    return 0;
}

/******************************************************************************/

