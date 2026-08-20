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
/*                                                                            */
/*                                SCRCOMP.CPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*      Data and functions for the compile portion of the cSCRIPT class.      */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

#ifdef  ___COMP___

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

// WATCH_MEM is used to conditionally include/exclude code to update the amount
// of available memory in the upper left corner of the screen

//#define WATCH_MEM

#ifdef WATCH_MEM
#include    <conio.h>
#endif

#include    <ctype.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <assert.h>

#include    <scrmisc.hpp>

#ifdef DEBUGSCR
#include    <memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

#define MAX_LAB_NAME_SIZE   32

#define NUM_CONST           108



/******************************************************************************/

// Types

//struct sSYMBOLINFO
//{
//     sSYMBOLINFO(void) : sym(NULL), val(NULL) { }
//
//     sSYMBOLINFO(sSYMBOLINFO & r) : sym(NULL), val(NULL)
//    {
//        if (r.sym) sym = strdup(r.sym);
//        if (r.val) val = strdup(r.val);
//    }
//
//    ~sSYMBOLINFO(void)
//    {
//        if (sym) free(sym);
//        if (val) free(val);
//    }
//
//    int operator == (sSYMBOLINFO & r) { return (strcmp(sym,r.sym) == 0); }
//    int operator != (sSYMBOLINFO & r) { return (strcmp(sym,r.sym) != 0); }
//
//    char * sym;
//    char * val;
//};
//
//cLIST<sSYMBOLINFO> symbolList;

struct sCONSTINFO
{
    tSTRING     symbol;
    tINTEGER    value;
};

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Constants

sCONSTINFO constInfo [NUM_CONST] =
{
    { "\x0C""ACC_CUR_BAL",    0x04     },
    { "\x0C""ACC_MSGREAD",    0x02     },
    { "\x0D""ACC_MSGWRITE",   0x03     },
    { "\x06""ACC_STAT",       0x00     },
    { "\x09""ACC_TIME",       0x01     },
    { "\x0D""ATTACH_LIM_P",   0x03     },
    { "\x0D""ATTACH_LIM_U",   0x02     },
    { "\x05""AUTO",           0x02000L },
    { "\x05""BANK",           0x0A     },
    { "\x05""BELL",           0x00800L },
    { "\x0A""BYTE_BANK",      0x0A     },
    { "\x0A""CHRG_CALL",      0x01     },
    { "\x0A""CHRG_CHAT",      0x04     },
    { "\x0F""CHRG_DOWNBYTES", 0x0B     },
    { "\x0E""CHRG_DOWNFILE",  0x0A     },
    { "\x0C""CHRG_MSGCAP",    0x06     },
    { "\x0F""CHRG_MSGECHOED", 0x08     },
    { "\x10""CHRG_MSGPRIVATE",0x09     },
    { "\x0D""CHRG_MSGREAD",   0x05     },
    { "\x0E""CHRG_MSGWRITE",  0x07     },
    { "\x0E""CHRG_PEAKTIME",  0x03     },
    { "\x0A""CHRG_TIME",      0x02     },
    { "\x09""CMAXMSGS",       0x01     },
    { "\x09""CRC_FILE",       0x01     },
    { "\x08""CRC_STR",        0x00     },
    { "\x0D""CRED_SPECIAL",   0x10     },
    { "\x0D""CRED_UPBYTES",   0x0F     },
    { "\x0C""CRED_UPFILE",    0x0E     },
    { "\x09""CUR_USER",       0        },
    { "\x09""DEB_CALL",       0x02     },
    { "\x09""DEB_CHAT",       0x0B     },
    { "\x0E""DEB_DOWNBYTES",  0x0A     },
    { "\x0D""DEB_DOWNFILE",   0x09     },
    { "\x0B""DEB_MSGCAP",     0x05     },
    { "\x0E""DEB_MSGECHOED",  0x07     },
    { "\x0F""DEB_MSGPRIVATE", 0x08     },
    { "\x0C""DEB_MSGREAD",    0x04     },
    { "\x0D""DEB_MSGWRITE",   0x06     },
    { "\x0C""DEB_SPECIAL",    0x0D    },
    { "\x09""DEB_TIME",       0x03     },
    { "\x08""DEB_TPU",        0x0C     },
    { "\x05""DEFS",           0x00     },
    { "\x09""ECHODOTS",       0x00001L },
    { "\x0A""ERASELINE",      0x00020L },
    { "\x06""FALSE",          0x00     },
    { "\x04""FCL",            0x02     },
    { "\x09""FIELDLEN",       0x00002L },
    { "\x04""FNS",            0x01     },
    { "\x06""F_EXP",          0x02     },
    { "\x05""F_MW",           0x10     },
    { "\x06""F_NET",          0x20     },
    { "\x06""F_REG",          0x01     },
    { "\x06""F_SEL",          0x04     },
    { "\x06""F_SYS",          0x08     },
    { "\x06""GRAPH",          0x01     },
    { "\x06""GUIDE",          0x00004L },
    { "\x0B""HDR_ACTIVE",     0x0E     },
    { "\x0B""HDR_BLOCKS",     0x04     },
    { "\x09""HDR_DATE",       0x05     },
    { "\x09""HDR_ECHO",       0x0F     },
    { "\x09""HDR_FROM",       0x0B     },
    { "\x0B""HDR_MSGNUM",     0x02     },
    { "\x0B""HDR_MSGREF",     0x03     },
    { "\x08""HDR_PWD",        0x0D     },
    { "\x0A""HDR_REPLY",      0x0A     },
    { "\x0D""HDR_RPLYDATE",   0x08     },
    { "\x0D""HDR_RPLYTIME",   0x09     },
    { "\x0B""HDR_STATUS",     0x01     },
    { "\x09""HDR_SUBJ",       0x0C     },
    { "\x09""HDR_TIME",       0x06     },
    { "\x07""HDR_TO",         0x07     },
    { "\x0A""HIGHASCII",      0x01000L },
    { "\x05""LANG",           0x04     },
    { "\x08""LFAFTER",        0x00100L },
    { "\x09""LFBEFORE",       0x00080L },
    { "\x06""LOGIT",          0x08000L },
    { "\x0A""LOGITLEFT",      0x10000L },
    { "\x08""MAXMSGS",        0x00      },
    { "\x03""NC",             0x00     },
    { "\x0B""NEWBALANCE",     0x00     },
    { "\x08""NEWLINE",        0x00040L },
    { "\x08""NOCLEAR",        0x00400L },
    { "\x08""NO_USER",        -1       },
    { "\x05""O_RD",           0x00     },
    { "\x05""O_RW",           0x02     },
    { "\x05""O_WR",           0x01     },
    { "\x0C""PAY_UPBYTES",    0x0D     },
    { "\x0B""PAY_UPFILE",     0x0C     },
    { "\x09""PERSONAL",       0x09     },
    { "\x04""SEC",            0x02     },
    { "\x09""SEC_DROP",       0x11     },
    { "\x09""SEEK_CUR",       0x01     },
    { "\x09""SEEK_END",       0x02     },
    { "\x09""SEEK_SET",       0x00     },
    { "\x08""STACKED",        0x00010L },
    { "\x0A""START_BAL",      0x00     },
    { "\x0E""START_SESSION",  0x01     },
    { "\x0A""STK_LIMIT",      STACK_CEILING + 1024 },
    { "\x05""S_DB",           0x03     },
    { "\x05""S_DN",           0x00     },
    { "\x05""S_DR",           0x01     },
    { "\x05""S_DW",           0x02     },
    { "\x0A""TIME_BANK",      0x0A     },
    { "\x05""TRUE",           0x01     },
    { "\x07""UPCASE",         0x00008L },
    { "\x0A""WARNLEVEL",      0x0E     },
    { "\x09""WORDWRAP",       0x00200L },
    { "\x06""YESNO",          0x04000L }
};

/******************************************************************************/

// Variables

char * nextLine         = NULL;

char   nextLogStatement [ MAX_LINE_SIZE+1 ];
char   commentBuf       [ MAX_LINE_SIZE+1 ];
char   selectExpr       [ MAX_STR_LEN+1 ];

char   warnFlag         = FALSE;
char   autoUVar         = TRUE;
char   disArrSubChk     = FALSE;
char   dispStat         = TRUE;

bool   fatalErr         = FALSE;

char   compileStr       [ MAX_BIGSTR_LEN+1 ];
char   idPrefix         [ MAX_STR_LEN+1 ];
int    caseCounter      = 0;
bool   inFunc           = FALSE;





/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

    /**********************************************************************/
    /*                                                                    */
    /* matchSubStrPasStr                                                  */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Compares a C style sub-string with a Pascal style       */
    /*            string and returns TRUE if they match                   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* ss -> (char *) A pointer to the C style sub-string                 */
    /* ps -> (char *) A pointer to the Pascal style string                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if the strings match, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

bool pascal matchSubStrPasStr(char * ss, char * ps)
{
    return (memicmp(ss,ps+1,*ps) == 0);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* compile -> cSCRIPT member function                                 */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To translate a text source code file to format useable  */
    /*            by the execute function (convert statement keywords to  */
    /*            tokens, convert source infix expressions to a tokenized */
    /*            postfix form, resolve destination labels to offsets,    */
    /*            build a list of variable information, etc.)             */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* f -> (char *) The file name (with optional path) to compile        */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if errors were detected in the compile; FALSE otherwise */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::compile(char * f)
{
    int err = FALSE;

    if (scriptBuffer != NULL)
    {
        if (dosfopen(f,OPEN_READ|OPEN_DENYWRIT,&fs) == 0)
        {
            initialize(1);                           // Initialize pass 1
            err = compPass(1);                       // Compiler pass 1
            if (!err)                                // If pass 1 OK
            {
                dosrewind(&fs);                      // Rewind source file
                initialize(2);                       // Initialize pass 2
                err = compPass(2);                   // Compiler pass 2
            }
            if (err) scriptErr(0,-1,SCR_ERR_GEN,"");
            dosfclose(&fs);
        }
        else
        {
            scriptErr(0,-1,SCR_ERR_GEN,"");
            err = TRUE;
        }
    }
    else
    {
        scriptErr(0,-1,SCR_ERR_MEM,"SCRIPT BUFFER");
        fatalErr = TRUE;
        err = TRUE;
    }

    // We're finished with the labels, so flush the list
    labLst.flush(DD_YES);

//    // We still need the variable information with the exception of the
//    // variable names, so go ahead and delete (and null) the variable
//    // name information
//    varLst.forEach(cVAR::delNullName);

    return err;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* addTokenToBuffer -> cSCRIPT member function                        */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To add a token to the script buffer and update the      */
    /*            pointer and size information for the script buffer      */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* tokval -> (int) The token value to add to the script buffer        */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if not enough room to store the token; FALSE otherwise  */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::addTokenToBuffer(int tokval)
{
    int err = FALSE;

    if (availBytes() < sizeof(tokval))
    {
        scriptSize = SCR_BUF_SIZE;
        err = TRUE;
    }
    else
    {
        * (int far *) scriptPointer = tokval;
        scriptPointer += sizeof(tokval);
        scriptSize += sizeof(tokval);
    }

    return err;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenize -> cSCRIPT member function                                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To convert the arguments after a statement keyword      */
    /*            into tokens that can be used directly by the execute    */
    /*            function (called by tokenXXX functions to provide       */
    /*            generic statement and argument processing in the form   */
    /*            key [arg[,arg[...]]])                                   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /* func  -> (int pascal (*)(int,cSCRIPT&,char*,int,int)) A function   */
    /*          to handle custom processing for each argument according   */
    /*          to the needs of the keyword                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::tokenize(cSCRIPT     & obj,
                             sKEYWORD    & inf,
                             char        * args,
                             int           line,
                             int           pass,
                             int pascal (* func)(int,int,cSCRIPT&,char*,int,int))
{
#ifdef PCB_DEMO
    if (inf.tokenVal < 0)
    {
        newline();
        newline();
        print("ERROR: The ");
        print(inf.token+1);
        print("statement is not supported in the PPLC DEMO ");
        return TRUE;
    }
#endif

    if (obj.addTokenToBuffer(inf.tokenVal)) return TRUE;

    int ac = argCount(args);

    // Calculate keyword display length to strip the trailing space
    int len = (*inf.token)-1;

    if (ac < inf.reqArgs)
    {
        // Insufficient arguments, error
        scriptErr(obj.fs.handle,line,SCR_ERR_ARGNE,
            "%*.*s:%d:%d",len,len,inf.token+1,inf.reqArgs,ac);
        return TRUE;
    }
    else if ((ac > inf.maxArgs) && (pass == 1))
    {
        // Too many arguments, warning
        scriptErr(obj.fs.handle,line,-SCR_WRN_ARGTM,
            "%*.*s:%d:%d",len,len,inf.token+1,inf.maxArgs,ac);
        warnFlag = TRUE;
    }

    if (inf.useVarArgs)
    {
        if (ac > inf.maxArgs) ac = inf.maxArgs;
        if (obj.addTokenToBuffer(ac)) return TRUE;
    }
    else
    {
        ac = inf.reqArgs;
    }

    int err = FALSE;

    char * arg = getArgument(args);     // Get first argument

    // While not an error and args left and arg exists
    for (int i = 0; !err && (i < ac) && (arg != NULL); ++i)
    {
        err = func(i,ac,obj,arg,line,pass); // Do custom processing for arg
        arg = getArgument(NULL);            // Get next argument
    }

    // If all args processed, return error status, otherwise force an error
    return ((i == ac) ? err : TRUE);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* subTokDef -> cSCRIPT subTok* member function template              */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To create a function that can be passed to the tokenize */
    /*            cSCRIPT member function to convert the arguments after  */
    /*            a statement keyword into tokens that can be used        */
    /*            directly by the execute cSCRIPT member function.        */
    /*            The resulting function is passed by token* to tokenize. */
    /*            It is called by tokenize to convert arguments from      */
    /*            source code to a tokenized format.                      */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* name -> The last part of the formal function name (after subTok)   */
    /* pos  -> The position of the argument to treat specially            */
    /* pt   -> The function to call for the specified position (TRUE)     */
    /* pf   -> The function to call for other positions        (FALSE)    */
    /*                                                                    */
    /**********************************************************************/

#define subTokDef(name,pos,pt,pf)                                            \
int pascal cSCRIPT::subTok##name(int n, int t, cSCRIPT & o, char * a,        \
    int l, int p)                                                            \
{                                                                            \
    return ((n == pos) ? o.pt(a,l,p) : o.pf(a,l,p));                         \
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* subTok* -> cSCRIPT member functions (from macro templates)         */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To convert the arguments after a statement keyword into */
    /*            tokens that can be used directly by the execute cSCRIPT */
    /*            member function.  The function is passed by token* to   */
    /*            tokenize.  It is called by tokenize to convert          */
    /*            arguments from source code to a tokenized format.       */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Functions Defined:                                                 */
    /*                                                                    */
    /* subTokARGS    -> converts all arguments from source infix to       */
    /*                  tokenized postfix expressions                     */
    /*                                                                    */
    /* subTokARGV    -> converts all arguments from source variables with */
    /*                  (optional) subscript references to tokenized      */
    /*                  variables with subscript counts and expressions   */
    /*                                                                    */
    /* subTokVAREXPR -> converts arg 0 like ARGV, all others like ARGS    */
    /*                                                                    */
    /* subTokEXPRVAR -> converts arg 1 like ARGV, all others like ARGS    */
    /*                                                                    */
    /* subTokVIDEXPR -> converts arg 0 like ARGV (but without subscripts) */
    /*                  and all others like ARGS                          */
    /*                                                                    */
    /* subTokEEVID   -> converts arg 2 like ARGV (but without subscripts) */
    /*                  and all others like ARGS                          */
    /*                                                                    */
    /* subTokVIDS    -> converts all arguments from source variables      */
    /*                  without subscript references to tokenized         */
    /*                  variables ids without subscript information       */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* n -> (int) The number of the argument (0 based)                    */
    /* t -> (int) The total number of arguments (not used)                */
    /* o -> (cSCRIPT &) A reference to the script object being compiled   */
    /* a -> (char *) The nth argument                                     */
    /* l -> (int) The line number being compiled                          */
    /* p -> (int) The current compiler pass                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
subTokDef(ARGS,   0,in2post, in2post);

#pragma argsused
subTokDef(ARGV,   0,wrVIDSUB,wrVIDSUB);

#pragma argsused
subTokDef(VAREXPR,0,wrVIDSUB,in2post);

#pragma argsused
subTokDef(EXPRVAR,1,wrVIDSUB,in2post);

#pragma argsused
subTokDef(VIDEXPR,0,wrVID,   in2post);

#ifdef DBASE
#pragma argsused
subTokDef(EEVID,  2,wrVID,   in2post);
#endif

#pragma argsused
subTokDef(VIDS,   0,wrVID,   wrVID);

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* subTokREDIM -> cSCRIPT member functions                            */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To convert the arguments after a REDIM keyword into     */
    /*            tokens that can be used directly by the execute cSCRIPT */
    /*            member function.  The function is passed by tokenREDIM  */
    /*            to tokenize.  It will convert like subTokVIDEXPR but    */
    /*            check to see how many subscripts there are and how many */
    /*            arguments are allowable.                                */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* n -> (int) The number of the argument (0 based)                    */
    /* t -> (int) The total number of arguments                           */
    /* o -> (cSCRIPT &) A reference to the script object being compiled   */
    /* a -> (char *) The nth argument                                     */
    /* l -> (int) The line number being compiled                          */
    /* p -> (int) The current compiler pass                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::subTokREDIM(int n, int t, cSCRIPT & o, char * a,
    int l, int p)
{
    // If array subscript checking is disabled, just use subTokVIDEXPR
    if (disArrSubChk)
    {
        return subTokVIDEXPR(n,t,o,a,l,p);
    }
    // Otherwise try to confirm valid parameters were passed in
    else
    {
        // Need to keep this value across function calls
        static unsigned char dims = 0;

        // Process each argument
        switch (n)
        {
            case 0:
            {
                // Get a pointer to the variable
                cVAR * vp = o.getVariable(a,l);

                // If there is an error getting the variable, return error
                if (vp == NULL) return TRUE;

                // Convert total number of arguments to number of dimensions
                dims = t-1;

                // Confirm that there aren't too few or too many arguments
                if      (dims < vp->info.dims)
                {
                    scriptErr(o.fs.handle,l,SCR_ERR_ARGNE,
                        "REDIM:%d:%d",vp->info.dims+1,dims+1);
                    return TRUE;
                }
                else if (dims > vp->info.dims)
                {
                    if (p == 1)
                    {
                        scriptErr(o.fs.handle,l,-SCR_WRN_ARGTM,
                            "REDIM:%d:%d",vp->info.dims+1,dims+1);
                        warnFlag = TRUE;
                    }
                    dims = vp->info.dims;
                }

                // Write out the variable ID
                return o.wrVID(a,l,p,FALSE);
            }

            default:
                // If the cur arg num is valid for the cur array,
                // convert arg from infix to postfix notation
                if (n <= dims) return o.in2post(a,l,p);
        }
    }

    return FALSE;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* subTokSORT -> cSCRIPT member functions                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To convert the arguments after a SORT keyword into      */
    /*            tokens that can be used directly by the execute cSCRIPT */
    /*            member function.  The function is passed by tokenSORT   */
    /*            to tokenize.  It will convert like subTokVIDS but       */
    /*            check to see how many subscripts there are and display  */
    /*            an error message if necessary.                          */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* n -> (int) The number of the argument (0 based)                    */
    /* t -> (int) The total number of arguments                           */
    /* o -> (cSCRIPT &) A reference to the script object being compiled   */
    /* a -> (char *) The nth argument                                     */
    /* l -> (int) The line number being compiled                          */
    /* p -> (int) The current compiler pass                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::subTokSORT(int n, int t, cSCRIPT & o, char * a,
    int l, int p)
{
    stripSpaces(a);

    // Get a pointer to the variable
    cVAR * vp = o.getVariable(a,l);

    // If there was an error getting variable, return error
    if (vp == NULL) return TRUE;

    // Process each argument
    switch (n)
    {
        case 1:
            // Make sure it is of the correct type
            switch (vp->info.type )            // re-int bit
            {
                case vtUNSIGNED:
                case vtINTEGER:
                case vtWORD:
                    break;

                default:
                    scriptErr(o.fs.handle,l,SCR_ERR_SRTTYP,"%s",
                        typeText[vp->info.type ]);      // re-init bit
                    return TRUE;
            }

        case 0:
            // Process according to the number of dimensions
            switch (vp->info.dims)
            {
                case 0:
                    scriptErr(o.fs.handle,l,SCR_ERR_SRTARG,
                        "%d",vp->info.dims);
                    return TRUE;

                case 1:
                    return subTokVIDS(n,t,o,a,l,p);

                default:
                    scriptErr(o.fs.handle,l,SCR_ERR_SRTARG,
                        "%d",vp->info.dims);
                    return TRUE;
            }

        default:
            return TRUE;
    }
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenDef -> cSCRIPT token* member function template                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To create a function that can call the tokenize cSCRIPT */
    /*            member function to convert the arguments after a        */
    /*            statement keyword into tokens that can be used directly */
    /*            by the execute cSCRIPT member function.                 */
    /*            The resulting function is called from the compLine      */
    /*            cSCRIPT member function.                                */
    /*            It's address is stored in the kwArray table so that     */
    /*            each keyword can have a custom tokenize process.        */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* name -> The last part of the formal function name (after token)    */
    /*                                                                    */
    /**********************************************************************/

#define tokenDef(name)                                                       \
int pascal cSCRIPT::token##name(cSCRIPT & obj, sKEYWORD & inf, char * args,  \
    int line, int pass)                                                      \
{                                                                            \
    return tokenize(obj,inf,args,line,pass,subTok##name);                    \
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* token* -> cSCRIPT member functions (from macro templates)          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To setup tokenize to correctly compile keywords with a  */
    /*            specific argument list format (passes subTok* to        */
    /*            tokenize to convert arguments from source code to a     */
    /*            tokenized format).                                      */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Functions Defined:                                                 */
    /*                                                                    */
    /* tokenARGS    -> converts all arguments from source infix to        */
    /*                 tokenized postfix expressions                      */
    /*                                                                    */
    /* tokenARGV    -> converts all arguments from source variables with  */
    /*                 (optional) subscript references to tokenized       */
    /*                 variables with subscript counts and expressions    */
    /*                                                                    */
    /* tokenVAREXPR -> converts arg 0 like ARGV, all others like ARGS     */
    /*                                                                    */
    /* tokenEXPRVAR -> converts arg 1 like ARGV, all others like ARGS     */
    /*                                                                    */
    /* tokenVIDEXPR -> converts arg 0 like ARGV (but without subscripts)  */
    /*                 and all others like ARGS                           */
    /*                                                                    */
    /* tokenEEVID   -> converts arg 2 like ARGV (but without subscripts)  */
    /*                 and all others like ARGS                           */
    /*                                                                    */
    /* tokenREDIM   -> converts arg 0 like ARGV (but without subscripts)  */
    /*                 and all others like ARGS (but performs array       */
    /*                 dimension checking for the number of arguments)    */
    /*                                                                    */
    /* tokenVIDS    -> converts all arguments from source variables       */
    /*                 without subscript references to tokenized          */
    /*                 variables ids without subscript information        */
    /*                                                                    */
    /* tokenSORT    -> converts like VIDS but confirms subscript counts   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

tokenDef(ARGS);
tokenDef(ARGV);
tokenDef(VAREXPR);
tokenDef(EXPRVAR);
tokenDef(VIDEXPR);
#ifdef DBASE
tokenDef(EEVID);
#endif
tokenDef(REDIM);
tokenDef(VIDS);
tokenDef(SORT);

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenLET -> cSCRIPT member function                                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize a LET statement (implicit or explicit) by   */
    /*            writing out a tokenized variable with subscript counts  */
    /*            and expressions (the lvalue) and a postfix expression   */
    /*            (the rvalue).                                           */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::tokenLET(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    if (obj.addTokenToBuffer(inf.tokenVal)) return TRUE;

    // Separate arguments into variable (lvalue) and expression (rvalue)
    char * expr = strchr(args,'=');
    if (expr != NULL)
        *(expr++) = '\0';
    else
        expr = "";


    // Write var ID and sub info for var (lvalue)
    // Convert text infix expr to token postfix expr and write it (rvalue)
    // Return error status for either side
    return (obj.wrVIDSUB(args,line,pass) || obj.in2post(expr,line,pass));
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenGOX -> cSCRIPT member function                                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize a GOTO or GOSUB statement by validating the */
    /*            destination label and writing out the statement token   */
    /*            and the label offset value.                             */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::tokenGOX(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    if (obj.addTokenToBuffer(inf.tokenVal)) return TRUE;

    // If args is not a legal name, return error
    if (!obj.legalName(args))
    {
        scriptErr(obj.fs.handle,line,SCR_ERR_LABBAD,"%s",formattedLine(args));
        return TRUE;
    }

    // Get the offset associated with label args
    int off = obj.getLabel(args,line,pass);

    // Force error if bad offset or return error status from adding token
    return ((off == -1) || obj.addTokenToBuffer(off));
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* isIfDestStatement -> cSCRIPT member function                       */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To determine if a statement is a destination statement  */
    /*            for an IF block and should have a destination label     */
    /*            generated.                                              */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* p -> (char *) A pointer to the statement to check                  */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if it is a destination statement; FALSE otherwise       */
    /*                                                                    */
    /**********************************************************************/

bool pascal cSCRIPT::isIfDestStatement(char * p)
{
    return ((strcmp(p,"ELSEIF ")  == 0) ||
            (strcmp(p,"ELSE IF ") == 0) ||
            (strcmp(p,"ELSE ")    == 0) ||
            (strcmp(p,"ENDIF ")   == 0) ||
            (strcmp(p,"END IF ")  == 0));
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* isIfEndStatement -> cSCRIPT member function                        */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To determine if a statement is an end statement for an  */
    /*            IF block and should have a destination label generated. */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* p -> (char *) A pointer to the statement to check                  */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if it is an end statement; FALSE otherwise              */
    /*                                                                    */
    /**********************************************************************/

bool pascal cSCRIPT::isIfEndStatement(char * p)
{
    return ((strcmp(p,"ENDIF ")   == 0) ||
            (strcmp(p,"END IF ")  == 0));
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* popStartStructInfo -> cSCRIPT member function                      */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To pop information from the start structure information */
    /*            stack and decode it from a long into its component      */
    /*            type and number fields.                                 */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* t -> (char &) A character code for the statement type              */
    /* n -> (int &)  The statement number                                 */
    /*                                                                    */
    /**********************************************************************/

void pascal cSCRIPT::popStartStructInfo(char & t, int & n)
{
    long info = startStruct.pop();        // Get the info
    t = char((info & 0xFFFF0000L) >> 16); // Get the statement type
    n = int(info & 0x0000FFFFL);          // Get the statement number
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* pushStartStructInfo -> cSCRIPT member function                     */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To encode a start structure statement type and number   */
    /*            into a long and push it on the start structure          */
    /*            information stack.                                      */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* t -> (char) A character code for the statement type                */
    /* n -> (int)  The statement number                                   */
    /*                                                                    */
    /**********************************************************************/

void pascal cSCRIPT::pushStartStructInfo(char t, int n)
{
    startStruct.push((long(t)<<16)|(n&0x0000FFFFL));
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* startStructError -> cSCRIPT member function                        */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To determine if there is an error between the actual    */
    /*            desired start structure types and process it.           */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj -> (cSCRIPT &) A reference to the script object being compiled */
    /* at  -> (char) The actual start structure type                      */
    /* dt  -> (char) The desired start structure type to match the end    */
    /* ts  -> (char *) A pointer to the token string length and text      */
    /* l   -> (int) The line number being compiled                        */
    /*                                                                    */
    /**********************************************************************/

bool pascal cSCRIPT::startStructError(cSCRIPT & obj, char at, char dt,
    char * ts, int l)
{
    // Strings for the valid start structure statement keywords
    char * skw [] =
    {
        "FOR",
        "IF",
        "WHILE"
    };

    // If the actual type is not the desired type, error ...
    if (at != dt)
    {
        // Figure out the offset for the actual start structure keyword
        int skwOff;
        switch (at)
        {
            case 'F': skwOff = 0; break;
            case 'I': skwOff = 1; break;
            case 'W': skwOff = 2; break;
        }

        // Display the error
        scriptErr(obj.fs.handle,l,SCR_ERR_BSES,"%s-%*.*s",
            skw[skwOff],(*ts)-1,(*ts)-1,ts+1);

        // Return error
        return TRUE;
    }

    // Return no error
    return FALSE;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenBRANCH -> cSCRIPT member function                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize control structure statements other than     */
    /*            those used to start the control structure.              */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::tokenBRANCH(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    char sysLabel[MAX_LAB_NAME_SIZE+1];

    // Flag used to determine if a CONTINUE label has already been generated
    // for the current statement (needed because a NEXT statement is processed
    // as an increment statement followed by an ENDWHILE statement)
    static bool contLabGened = FALSE;

    char * tokenText = inf.token+1; // Get a pointer to the token text

    // If it is a destination statement and it is not an end statement ...
    if (obj.isIfDestStatement(tokenText) && !obj.isIfEndStatement(tokenText))
    {
        // Make sure there is a block end structure number on the stack
        if (obj.endStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_NOBIF,"");
            return TRUE;
        }

        // Assume offset of 0 for pass 1
        int off = 0;

        // If it is pass 2 then we need to get the destination label
        if (pass == 2)
        {
            int en = obj.endStruct.pop();           // Get the end number
            obj.endStruct.push(en);                 // Push it back
            sprintf(sysLabel,"~END~%d~",en);        // Create system label
            off = obj.getLabel(sysLabel,line,pass); // Get label offset
        }

        // If there is an error in the offset or an error writing the
        // GOTO token or the offset to the script buffer, return error
        if ((off == -1) || obj.addTokenToBuffer(TOK_GOTO) ||
            obj.addTokenToBuffer(off)) return TRUE;
    }

    // If it is a destination statement ...
    if (obj.isIfDestStatement(tokenText))
    {
        // Make sure there is a block control structure number on the stack
        if (obj.cntrlStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_NOBIF,"");
            return TRUE;
        }

        // Get the control structure number (necessary for both passes)
        int cn = obj.cntrlStruct.pop();

        // If it is pass 1 create the destination label
        if (pass == 1)
        {
            sprintf(sysLabel,"~DEST~%d~",cn);
            if (obj.doLabel(sysLabel,line,TRUE)) return TRUE;
        }
    }

    // If it is an end statment ...
    if (obj.isIfEndStatement(tokenText))
    {
        // Make sure there is a block end structure number on the stack
        if (obj.endStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_NOBIF,"");
            return TRUE;
        }

        // Get the statement type that started the structure
        char stype;
        int  snum;
        obj.popStartStructInfo(stype,snum);

        // Process start structure errors as appropriate
        if (startStructError(obj,stype,'I',tokenText-1,line)) return TRUE;

        // Get the end structure number (necessary for both passes)
        int en = obj.endStruct.pop();

        // If it is pass 1 create the end label
        if (pass == 1)
        {
            sprintf(sysLabel,"~END~%d~",en);
            if (obj.doLabel(sysLabel,line,TRUE)) return TRUE;
        }
    }

    // If it is ELSEIF, finish processing like a normal IF
    if ((strcmp(inf.token+1,"ELSE IF ") == 0) ||
        (strcmp(inf.token+1,"ELSEIF ") == 0))
    {
        return tokenIFWHILE(obj,inf,args,line,pass);
    }
    // If it is ELSE, update the destination label count & next line pointer
    else if (strcmp(inf.token+1,"ELSE ") == 0)
    {
        obj.cntrlStruct.push(++obj.destLabelCount);
        if(*args != NULL)
          nextLine = args;
    }
    else if ((strcmp(inf.token+1,"BREAK ") == 0) ||
             (strcmp(inf.token+1,"QUIT ")  == 0))
    {
        // Make sure there is a loop control structure number on the stack
        if (obj.loopStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_INVBC,"%*.*s",
                (*inf.token)-1,(*inf.token)-1,inf.token+1);
            return TRUE;
        }

        // Assume offset of 0 for pass 1
        int off = 0;

        // If it is pass 2 then we need to get the end loop label
        if (pass == 2)
        {
            int en = obj.loopStruct.pop();          // Get the end number
            obj.loopStruct.push(en);                // Push it back
            sprintf(sysLabel,"~ENDLOOP~%d~",en);    // Create system label
            off = obj.getLabel(sysLabel,line,pass); // Get label offset
        }

        // If there is an error in the offset or an error writing the
        // GOTO token or the offset to the script buffer, return error
        return ((off == -1) || obj.addTokenToBuffer(TOK_GOTO) ||
            obj.addTokenToBuffer(off));
    }
    else if ((strcmp(inf.token+1,"CONTINUE ") == 0) ||
             (strcmp(inf.token+1,"LOOP ")     == 0))
    {
        // Make sure there is a loop control structure number on the stack
        if (obj.loopStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_INVBC,"%*.*s",
                (*inf.token)-1,(*inf.token)-1,inf.token+1);
            return TRUE;
        }

        // Assume offset of 0 for pass 1
        int off = 0;

        // If it is pass 2 then we need to get the continue label
        if (pass == 2)
        {
            int cn = obj.loopStruct.pop();          // Get the end number
            obj.loopStruct.push(cn);                // Push it back
            sprintf(sysLabel,"~CONT~%d~",cn);       // Create system label
            off = obj.getLabel(sysLabel,line,pass); // Get label offset
        }

        // If there is an error in the offset or an error writing the
        // GOTO token or the offset to the script buffer, return error
        return ((off == -1) || obj.addTokenToBuffer(TOK_GOTO) ||
            obj.addTokenToBuffer(off));
    }
    else if ((strcmp(inf.token+1,"END WHILE ") == 0) ||
        (strcmp(inf.token+1,"ENDWHILE ") == 0))
    {
        // Make sure there is a start structure number on the stack
        if (obj.startStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_NOBIF,"");
            return TRUE;
        }

        // Get the statement type that started the structure
        char stype;
        int  snum;
        obj.popStartStructInfo(stype,snum);

        // Process start structure errors as appropriate
        if (startStructError(obj,stype,'W',tokenText-1,line)) return TRUE;

        // Get the loop structure number
        int ln = obj.loopStruct.pop();

        // If it is pass 1 and the continue label has not yet been generated
        if (!contLabGened && (pass == 1))
        {
            sprintf(sysLabel,"~CONT~%d~",ln);
            if (obj.doLabel(sysLabel,line,TRUE)) return TRUE;
        }
        // Otherwise the label was generated for a NEXT statement and we
        // shouldn't regenerate it, so just clear the generated status
        else
            contLabGened = FALSE;

        // Assume offset of 0 for pass 1
        int off = 0;

        // If it is pass 2 then we need to get the loop label
        if (pass == 2)
        {
            sprintf(sysLabel,"~LOOP~%d~",ln);
            off = obj.getLabel(sysLabel,line,pass);
        }

        // If there is an error in the offset or an error writing the
        // GOTO token or the offset to the script buffer, flag error
        int err = ((off == -1) || obj.addTokenToBuffer(TOK_GOTO) ||
            obj.addTokenToBuffer(off));

        // If it is pass 1 we need to generate the end loop label
        if (pass == 1)
        {
            sprintf(sysLabel,"~ENDLOOP~%d~",ln);
            if (obj.doLabel(sysLabel,line,TRUE)) return TRUE;
        }

        // Return the previously calculated error status
        return err;
    }
    else if ((strcmp(inf.token+1,"NEXT ") == 0) ||
        (strcmp(inf.token+1,"END FOR ") == 0) ||
        (strcmp(inf.token+1,"ENDFOR ") == 0))
    {
        // Make sure there is a start structure number on the stack
        if (obj.startStruct.isEmpty())
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_NOBIF,"");
            return TRUE;
        }

        // Get the statement type that started the structure
        char stype;
        int  snum;
        obj.popStartStructInfo(stype,snum);

        // Process start structure errors as appropriate
        if (startStructError(obj,stype,'F',tokenText-1,line)) return TRUE;

        // Put back the modified structure info (translates FOR to WHILE)
        obj.pushStartStructInfo('W',snum);

        // If it is pass 1 generate the continue label
        if (pass == 1)
        {
            int cn = obj.loopStruct.pop();          // Get the end number
            obj.loopStruct.push(cn);                // Push it back
            sprintf(sysLabel,"~CONT~%d~",cn);       // Create system label
            if (obj.doLabel(sysLabel,line,TRUE))    // Get label offset
                return TRUE;
        }

        // Set the continue label generated flag so that ENDWHILE won't try to
        // generate it a second time
        contLabGened = TRUE;

        // Get the saved step expression statement
        char * p = obj.forStepExpr.pop();

        // If the pointer is valid, compile the expanded next and return
        if (p != NULL)
        {
            int err = (obj.compLine(pass,p,line) ||
                obj.compLine(pass,"ENDWHILE ",line));
            free(p);
            return err;
        }
        // Otherwise handle the error
        else
        {
            // scriptErr
            return TRUE;
        }
    }

    // If we get to here assume no errors
    return FALSE;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenIFWHILE -> cSCRIPT member function                            */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize logical and block IF and WHILE statements.  */
    /*            Also performs final processing for ELSEIF statements.   */
    /*            NOTE:  This function will only process TOK_IF token     */
    /*                   values due to the need to set WHILE and ELSEIF   */
    /*                   token values to 0.                               */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::tokenIFWHILE(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    char sysLabel[MAX_LAB_NAME_SIZE+1];

    // NOTE: "IF " has length of 3, "WHILE " has length of 6
    //       IF and WHILE may be either logical or block

    const int IF_LEN    = 3;
    const int WHILE_LEN = 6;

    // nestLevel is used to track multiple IF/WHILE statements per line
    static unsigned char nestLevel = 0;
    ++nestLevel;

    // If it is a WHILE statement ...
    if (*inf.token == WHILE_LEN)
    {
        // Save information about the while statement and loop level
        obj.pushStartStructInfo('W',line);
        obj.loopStruct.push(++obj.loopLevel);

        // If it is pass 1, generate the loop label
        if (pass == 1)
        {
            sprintf(sysLabel,"~LOOP~%d~",obj.loopLevel);

            // If there is an error, restore nestLevel and return error
            if (obj.doLabel(sysLabel,line,TRUE))
            {
                --nestLevel;
                return TRUE;
            }
        }
    }

    // If no room for token, return error
    if (obj.addTokenToBuffer(TOK_IF))
    {
        --nestLevel;
        return TRUE;
    }

    // If no open parenthesis (necessary to start the condition), return error
    if (*args != '(')
    {
        scriptErr(obj.fs.handle,line,SCR_ERR_IFCNF,"");
        --nestLevel;
        return TRUE;
    }

    // Confirm and NULL terminate 1 and only 1 expression, otherwise return err
    if (argCount(args+1,TRUE) != 1)
    {
        scriptErr(obj.fs.handle,line,SCR_ERR_IFSNF,"");
        --nestLevel;
        return TRUE;
    }

    // Standardize the statement to execute (when args evaluates TRUE)
    // (Must come before in2post below)
    char * sptr = args+strlen(args)+1;
    int q = 0, p = 0;
    standardizeLine(sptr,q,p);

    // Convert expression from source infix format to tokenized postfix format
    if (obj.in2post(args+1,line,pass))
    {
        --nestLevel;
        return TRUE;
    }

    // If it isn't an IF/WHILE statement or if condition is followed by DO/THEN
    // then we are dealing with a block structure conditional (ie, we are
    // dealing with IF/THEN, ELSEIF/THEN, or WHILE/DO)
    if (((*inf.token != IF_LEN) && (*inf.token != WHILE_LEN)) ||
        matchSubStrPasStr(sptr,"\x03""DO ") ||
        matchSubStrPasStr(sptr,"\x05""THEN "))
    {
        // Replace the expression end token with a NOT to flip the value
        *(int far *)(obj.scriptPointer-sizeof(int)) = TOK_OP_NOT;

        // Add an extra END token to the expression
        obj.addTokenToBuffer(TOK_OP_END);

        // If it is not a WHILE statement
        if (*inf.token != WHILE_LEN)
        {
            // Start a new destionation control structure
            obj.cntrlStruct.push(++obj.destLabelCount);

            // If it is an IF statement save information about the if statement
            // start and end structures
            if (*inf.token == IF_LEN)
            {
                obj.pushStartStructInfo('I',line);
                obj.endStruct.push(++obj.endLabelCount);
            }
        }
    }

    // Assume offset of 0 for pass 1
    int off = 0;

    // If it is pass 2 get the offset of the system label
    if (pass == 2)
    {
        sprintf(sysLabel,"~LABEL~%d~",
            (nestLevel == 1) ? ++obj.sysLabelCount : obj.sysLabelCount);
        off = obj.getLabel(sysLabel,line,pass);
    }

    // If there is an error in the offset or in adding the offset to the buffer
    // return an error
    if ((off == -1) || obj.addTokenToBuffer(off))
    {
        --nestLevel;
        return TRUE;
    }

    int err = FALSE; // Error status flag (assume no error)

    // If it isn't an IF/WHILE statement or if condition is followed by DO/THEN
    // then we are dealing with a block structure conditional (ie, we are
    // dealing with IF/THEN, ELSEIF/THEN, or WHILE/DO)
    if (((*inf.token != IF_LEN) && (*inf.token != WHILE_LEN)) ||
        matchSubStrPasStr(sptr,"\x03""DO ") ||
        matchSubStrPasStr(sptr,"\x05""THEN "))
    {
        // Assume offset of 0 for pass 1
        off = 0;

        // If it is pass 2 get the offset of the destination/end loop label
        if (pass == 2)
        {
            // If it is not a WHILE loop, then get a destination label
            if (*inf.token != WHILE_LEN)
                sprintf(sysLabel,"~DEST~%d~",obj.destLabelCount);
            // Otherwise get an end loop label
            else
                sprintf(sysLabel,"~ENDLOOP~%d~",obj.loopLevel);
            off = obj.getLabel(sysLabel,line,pass);
        }

        // If there is an error in the offset or in adding the offset to the
        // buffer set the error flag
        err = ((off == -1) || obj.addTokenToBuffer(TOK_GOTO) ||
            obj.addTokenToBuffer(off));

        // Skip the DO or THEN if it is present
        if (matchSubStrPasStr(sptr,"\x03""DO "))
            sptr += 3;
        else if (matchSubStrPasStr(sptr,"\x05""THEN "))
            sptr += 5;

        // Save the extra code for later compilation
        nextLine = sptr;
    }
    else
    {
        // Compile the logical statement
        err = obj.compLine(pass,sptr,line);

        // If it is a WHILE loop
        if (*inf.token == WHILE_LEN)
        {
            // Get the loop number
            int ln = obj.loopStruct.pop();

            // Discard the start structure information (not needed for logical)
            obj.startStruct.pop();

            // Assume offset is 0 for pass 1
            off = 0;

            // If it is pass 2 get the loop label
            if (pass == 2)
            {
                sprintf(sysLabel,"~LOOP~%d~",ln);
                off = obj.getLabel(sysLabel,line,pass);
            }

            // If there is an error in the offset or in adding the offset to
            // the buffer set the error flag
            err = (err || ((off == -1) || obj.addTokenToBuffer(TOK_GOTO) ||
                obj.addTokenToBuffer(off)));
        }
    }

    // If it is pass 1 we need to make sure that a system label is generated
    // at the beginning of the next line
    if (pass == 1) obj.genSysLabel = TRUE;

    // Reset nestLevel and return error status
    --nestLevel;
    return err;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenFOR -> cSCRIPT member function                                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize FOR statements.                             */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenFOR(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    // SYNTAX:
    //   FOR VAR = STARTEXPR TO ENDEXPR [STEP INCEXPR]
    //     statement(s)
    //   NEXT
    //
    // EXPANDED FORMAT:
    //   LET VAR = STARTEXPR
    //   WHILE (((INCEXPR < 0) & (VAR >= ENDEXPR)) | ((INCEXPR >= 0) & (VAR <= ENDEXPR))) DO
    //     statement(s)
    //     LET VAR = VAR + INCEXPR
    //   ENDWHILE

    // NOTES:
    //   STARTEXPR will only be evaluated once at loop initialization
    //   ENDEXPR will be evaluated twice per loop iteration (WHILE condition)
    //   INCEXPR will be evaluated three times per loop iteration
    //     (WHILE condition and LET statement

    char lbuf[MAX_LINE_SIZE+1];

    // get index variable into ivar
    char * p = pcbstrchr(args,'=');
    if (p != NULL) *(p++) = '\0';
    char * ivar = pcbstrdup(args);
    args = p;

    // get start expression into startexp
    p = pcbstrstr(args," TO ");
    if (p != NULL) *p = '\0', p += 4;
    char * startexp = pcbstrdup(args);
    args = p;

    // get end expression into endexp
    p = pcbstrstr(args," STEP ");
    if (p != NULL) *p = '\0', p += 6;
    char * endexp = pcbstrdup(args);
    args = p;

    // get step expression into stepexp
    char * stepexp = pcbstrdup(args ? args : "1");

    // Assume no error
    int err = FALSE;

    // If all 4 FOR statement components are not NULL, process
    if (ivar && startexp && endexp && stepexp)
    {
        // validate all 4 arguments

        // Expand and compile initialization
        sprintf(lbuf,"LET %s=%s ",ivar,startexp);
        err = (err || obj.compLine(pass,lbuf,line));

        // Expand and compile conditional
        sprintf(lbuf,"WHILE (((%s<0)&(%s>=%s))|((%s>=0)&(%s<=%s))) DO ",stepexp,ivar,endexp,stepexp,ivar,endexp);
        err = (err || obj.compLine(pass,lbuf,line));

        // Update struct info from WHILE to FOR
        char stype;
        int  snum;
        obj.popStartStructInfo(stype,snum);
        obj.pushStartStructInfo('F',snum);

        // Expand and save increment for use by NEXT
        sprintf(lbuf,"LET %s=%s+%s ",ivar,ivar,stepexp);
        obj.forStepExpr.push(strdup(lbuf));
    }
    // Otherwise it is an error
    else
    {
        scriptErr(obj.fs.handle,line,SCR_ERR_FOR,"");
        err = TRUE;
    }

    // Free up any local allocated memory
    if (ivar     != NULL) free(ivar);
    if (startexp != NULL) free(startexp);
    if (endexp   != NULL) free(endexp);
    if (stepexp  != NULL) free(stepexp);

    // Return error status
    return err;
}



    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenTYPE -> cSCRIPT member function                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To process a type statement declaring a user type       */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenTYPE (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = FALSE;

    return err;
}


    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenSTATIC -> cSCRIPT member function                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Process static local variable declarations              */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenSTATIC (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = FALSE;
    char * tmp1 = NULL, * tmp2 = NULL;
    while(*args == ' ')
      args++;
    err = obj.compLine(pass,args,line);
    if(err)
      return err;
    tmp1 = strchr(args,' ');
    *tmp1 = '\x0';
    tmp1++;
    tmp2 = getArgument(tmp1);
    cVAR * optr;
    while(tmp2 != NULL)
    {
       optr = obj.getVariable(tmp2,line);
       if(optr)
       {
         optr->info.flags.init = TRUE;
         tmp2 = getArgument(NULL);
       }
    }


   return err;
}




    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenDECL -> cSCRIPT member function                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To add function declarations to symbol table            */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenDECL (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = FALSE,procflag = 0,funcflag = 0,ac=0;
    int tmptokval;
    char *tmp1=NULL,*tmp2=NULL, fname[MAX_LAB_NAME_SIZE],str[MAX_STR_LEN+1];
    // determine if a function or procedure is being declared
    tmp1 = strstr(args,"FUNCTION");
    if(tmp1)
      funcflag = 1;
    else{
      tmp1 = strstr(args,"PROCEDURE");
      if(tmp1)
        procflag = 1;
    }

    // if both procedure and function appear or neither appear then ERR
    if(!funcflag && !procflag){
      // Error in declare statement
       scriptErr(obj.fs.handle,line,SCR_ERR_FUNC,"%s","Must include FUNCTION or PROCEDURE");
       return err = TRUE;
    }
    // set args to point to space before next argument
    args = strchr(args,' ');
    stripSpaces(args);
    if(!args){
      //ERR
      scriptErr(obj.fs.handle,line,SCR_ERR_FUNC,"");
      return TRUE;
    }

    // set tmp2 to point to open paren
    tmp2 = strchr(args,'(');

    // if there is an open paren, null terminate it and copy the function/proc
    // name into fname array. Find the closing paren and null terminate it.
    if(tmp2){
      *tmp2 = '\x0';
      strcpy(fname,args);
      *tmp2 = '(';
      args = ++tmp2;
      tmp2 = strchr(args,')');
      if(tmp2)
        *tmp2 = '\x0';
      else{
        scriptErr(obj.fs.handle,line,SCR_ERR_FUNC,"");
        return err = TRUE;
      }
    }
    else{
      //err
      scriptErr(obj.fs.handle,line,SCR_ERR_FUNC,"");
      return err = TRUE;
    }

    //get the numger of arguments to the function/prodecure
    if(*(args+1) == ')')
      ac = 0;
    else
      ac = argCount(args);
    args = tmp1;

    // add variable to symbol table along with type,and number of args expected
    // using doVariable?? pass fucname(numparams) to generate a 1 in the dims
    // field and the number of parameters in the vs field.
    sprintf(str,"%s",fname);
    if(funcflag)
      tmptokval = vtFUNCTION;
    if(procflag)
      tmptokval = vtPROCEDURE;
    err = obj.doVariable(tmptokval,str,line,pass);

    // get the variable and set params to number of args
    cVAR *vptr = obj.getVariable(fname,line);
    vptr->data->values.vFUNCTION.params = ac;


    // Determine type of each parameter and declare
    // a mangled name local variable to hold the parameter.


    // replace closing paren
    *tmp2 = ')';
    if(err)
      scriptErr(obj.fs.handle,line,SCR_ERR_FUNC,"");
    return err;
}


    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenFUNC   -> cSCRIPT member function                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To process function definitions                         */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenFUNC (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = FALSE,i=0;
    char * tmp1 = NULL;
    char * tmp2 = NULL;
    char * tmp3 = NULL;
    char * tmp4 = NULL;
    char   retstr[MAX_STR_LEN+1];

    // Make sure both open and close parens are present
    tmp1 = strchr(args,'(');
    tmp2 = strrchr(args,')');
    if(!tmp1 || !tmp2)
    {
      scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
      return err = TRUE;
    }

    // isolate function name, parameterlist and return type
    *tmp1 = *tmp2 ='\x0';
    stripSpaces(args);

    // Make sure function has been declared
    cVAR * t = obj.getVariable(args,line);
    if(!t)
    {
      scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
      return err = TRUE;
    }

    // increment null termintated pointers to point to paramter list &
    // return type
    tmp1++;
    tmp2+=2;

    // make sure argument count matches the one in the declaration
    if(t->data->values.vFUNCTION.params != obj.argCount(tmp1))
    {
      scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
      return err = TRUE;
    }

    // if infunc flag is 0, set it to one. IF it is greater than 0 then err
    if(inFunc)
    {
       scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
       return err = TRUE;
    }
    else
      inFunc = TRUE;

    // build name mangled prefix for local identifiers
    sprintf(idPrefix,"~F~%s~",args);
    // Add END token to start every functio/procedure so no fall throughs.
    obj.addTokenToBuffer(TOK_END);
    // generate system label in the form ~f~funcname~
    // and process parameter variables

    if(pass == 1)
    {
       err = obj.doLabel(idPrefix,line,TRUE);
       if(err)
       {
         scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
         return err;
       }

      //  Make local varibles to hold copies or point to actual parameter values
      // This uses the type information in the parameter list and compiles a line
      // which delcares a variable of that type with the mangled prefix.
      cVAR * vptr;
      tmp4 = strchr(tmp1,',');
      if(tmp4)
        *tmp4 = '\x0';
      char * arg = tmp1; //getArgument(tmp1);

      // initialize parameter passing bit field
      if(t->info.type  == vtPROCEDURE)
        t->data->values.vPROCEDURE.passFlags = 0;

      if(t->data->values.vFUNCTION.params == 0)
      {
        t->data->values.vPROCEDURE.firstId = obj.curVarID; // -1 for correct array indexing in screxec


      }
      for(i=0;i<t->data->values.vFUNCTION.params;i++)
      {
        // determine if arg is passed by var or value
        err = obj.parseArg(obj,t,&arg,args,i,line);
        if(err)
          return err;
        strcpy(retstr,arg);
        err = obj.compLine(pass,retstr,line);
        if(err)
        {
           scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
           return err;
        }

        //isolate variable name
        if(i == 0)
        {
          tmp3 = strchr(arg,' ');
          if(tmp3)
            *tmp3 = '\x0';
          tmp3++;
          vptr = obj.getVariable(tmp3,line,FALSE);
          *--tmp3 = ' ';
          t->data->values.vPROCEDURE.firstId = vptr->info.id-1; // -1 for correct array indexing in screxec
        }
        tmp1 = ++tmp4;
        tmp4 = strchr(tmp1,',');
        if(tmp4)
          *tmp4 = '\x0';
        arg = tmp1; //getArgument(tmp1);
      }

      //create variable for return value in function.
      if(t->data->type == vtFUNCTION)
      {
        sprintf(retstr,"%s %s",tmp2,args);
        // compile the string.
        err = obj.compLine(pass,retstr,line);
        if(err)
        {
            scriptErr(obj.fs.handle,line,SCR_ERR_DECDEF,args);
            return err;
        }

        // get var id for return var
        cVAR * q = obj.getVariable(args,line);

        // put return var id in t->data->values.vFUNCTION.retId which needs to be added
        if(q)
          t->data->values.vFUNCTION.retId = q->info.id;
        else
          err = TRUE;
      }


    }
    return err;
}




    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* parseArg   -> cSCRIPT member function                              */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To parse arguments to procedures and determine passing  */
    /*            mode.                                                   */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* t     -> (cVAR     &) A reference to parameter variable            */
    /* arg   -> (char **) Parameter being processed                       */
    /* func  -> (char *)  Name of function parameter belongs to           */
    /* pnum  -> (int) The postion of parameter in the list                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::parseArg (cSCRIPT & obj, cVAR * t, char ** arg,
    char * func, int pnum, int line)
{

    int err            = FALSE;
    int varFlag        = FALSE;
    static int maskVal = 1;
    char    *  tmp1    = NULL;

    // re-init maskVal if this is the first call for an argument list
    if(pnum ==0)
      maskVal = 1;

    // start at the first non-space char
    tmp1 = *arg;
    while(!isalnum(*tmp1))
      tmp1++;
    *arg = tmp1;

    // set tmp1 to the first space AFTER the first substring
    tmp1 = strchr(*arg,' ');
    *tmp1 = '\x0';


    if(strcmp(*arg,"VAR") == 0)
      varFlag = TRUE;
    *tmp1 = ' ';
    if(!varFlag && strstr(*arg,"VAR"))
    {
       scriptErr(fs.handle,line,SCR_ERR_DECDEF,"VAR statement out of order");
       return TRUE;
    }
    *tmp1 = '\x0';

    // If not passing by var, return
    if(!varFlag)
    {

      maskVal = maskVal << 1;
      *tmp1 = ' ';
      return FALSE;
    }

    // If trying to pass by var to a function then Error
    if(varFlag && (t->info.type  == vtFUNCTION) )        // re-init bit
    {
      scriptErr(fs.handle,line,SCR_ERR_VARFUNC,"");
      return TRUE;
    }

    // At this point we know that this parameter is going to be passed
    // by VAR. Now we set the ith bit to TRUE in passFlags in procINFO;

    // Can have only 16 VAR params. the value 0x80 would be the value 1 shifted
    // 16 times.
    if(maskVal == 0 )
    {
      scriptErr(fs.handle,line,SCR_ERR_MANYVAR,func);
      err = TRUE;
    }

    // Allow no more than 256 parameters.
    if(pnum > 256 )
    {
      scriptErr(fs.handle,line,SCR_ERR_MANYPS,func);
      err = TRUE;
    }

    t->data->values.vPROCEDURE.passFlags = t->data->values.vPROCEDURE.passFlags | maskVal;
    maskVal = maskVal << 1;

    // set tmp1 past \x0
    tmp1++;

    // set tmp1 to next substring
    while(!isalnum(*tmp1))
      tmp1++;

    //set args to point to tmp1
    *arg = tmp1;

    return err;
}


    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenDBFPROC     -> cSCRIPT member function                        */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To process special DBASE keyword CREATE                 */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenDBFPROC (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = FALSE,i=0;
    char * tmp1;

    obj.addTokenToBuffer(inf.tokenVal);

    switch(inf.tokenVal)
    {

        case TOK_DCREATE:

            tmp1 = obj.getArgument(args);
            for(i=0;i<3;i++)
            {
                err = obj.in2post(tmp1,line,pass);
                tmp1 = obj.getArgument(NULL);
                if(err)
                  break;
            }
            //err = obj.wrVID(tmp1,line,pass,FALSE);
            //tmp1 = obj.getArgument(NULL);
            //if(err)
            //  break;
            err = obj.wrVID(tmp1,line,pass,FALSE);
            break;

        case TOK_DNCREATE:

            tmp1 = obj.getArgument(args);
            for(i=0;i<3;i++)
            {
                err = obj.in2post(tmp1,line,pass);
                tmp1 = obj.getArgument(NULL);
                if(err)
                 break;
            }
            //err = obj.wrVID(tmp1,line,pass,FALSE);
            break;

        case TOK_DGET:

            tmp1 = obj.getArgument(args);
            for(i=0;i<2;i++)
            {
                obj.in2post(tmp1,line,pass);
                tmp1 = obj.getArgument(NULL);
                if(err)
                  break;
            }
            err = obj.wrVIDSUB(tmp1,line,pass,FALSE);
            break;

        case TOK_DLOCKG:

            tmp1 = obj.getArgument(args);
            err = obj.in2post(tmp1,line,pass);
            tmp1 = obj.getArgument(NULL);
            if(err)
               break;
            err = obj.wrVID(tmp1,line,pass,FALSE);
            tmp1 = obj.getArgument(NULL);
            err = obj.in2post(tmp1,line,pass);


            break;

        default:
            err = TRUE;
            break;

    }

    return(err);
}
    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenBEGIN     -> cSCRIPT member function                          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To generate a label for BEGIN of main procedure         */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenBEGIN (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = FALSE;

    // This statement allows BEGIN keyword to be ignored in functions or
    // procedures.
    if(inFunc)
      return err;

    if(pass == 1)
      err = obj.doLabel("~BEGIN~ ",line,TRUE);
    return err;

}


    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenENDFUNC   -> cSCRIPT member function                          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To process end of function definition                   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenENDFUNC (cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int     err = FALSE;
    cVAR * fptr = NULL;
    char s[MAX_STR_LEN];
    char * tmp1 = NULL;
    char * tmp2 =NULL;

       strcpy(s,idPrefix);
       tmp1 = s+3;          // the 3rd postion is the beginning of the function name.
       tmp2 = strchr(tmp1,'~');
       if(tmp2)
       {
         *tmp2 = '\x0';
         fptr = obj.getVariable(tmp1,line,TRUE);
         if(fptr && pass == 1)
           fptr->data->values.vPROCEDURE.locvarcnt -= fptr->data->values.vPROCEDURE.params;
           //else
           // err
       }
       if(fptr->data->type == vtFUNCTION)
         obj.addTokenToBuffer(TOK_FEND);
       else
        obj.addTokenToBuffer(TOK_FPCLR);
       inFunc = FALSE;
       return err;
}


    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenSELECT -> cSCRIPT member function                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize Switch statements                           */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenSELECT(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{

    int    err = FALSE;
    char * tmp = NULL;

    // SYNTAX:
    // SELECT CASE (EXPR)   /* EXPR = SWITCHVAR*/
    // CASE (CONST)
    //   STATEMENT
    //   ...
    //   STATEMENT
    // CASE (CONST)
    //   STATEMENT
    //   ...                                         7
    // DEFAULT
    //   STATEMENT
    //   ...
    // ENDSELECT

    // EXPANDED FORMAT:
    // IF CONST == SWITCHVAR [|| CONST1 ==SELECTVAR ....]   FOR ENUMERATED LISTS
    //   STATEMENT
    //   ...
    // ELSEIF CONST == SELECTVAR [|| CONSTa >= SELECTVAR && CONSTb <= SELECTVAR...] FOR RANGES
    //   STATEMENT
    //   ...
    // ELSE
    //   STATEMENT
    //   ...
    // ENDIF


    // SEE ALSO token::SLCTEND
    //          token::DEFAULT
    //          token::CASE
    //  BELOW


    // Need to pop present value of selectExpr so support nested select case ststements
    // also need to push casecounter.

    ss * s;
    s = new ss;
    s->old = new char[sizeof(selectExpr)+1];
    s->cntr = caseCounter;
    strcpy(s->old,selectExpr);
    obj.selectStack.push(s);

    //obj.caseStack.push(caseCounter);
    caseCounter = 0;

    // need to check for valid expression here
    char * arg = obj.getArgument(args);

    // ensure the CASE keyword exists
    while(isspace(*arg))
      arg++;
    tmp = strchr(arg,' ');
    *tmp = '\x0';

    if(strcmp(arg,"CASE") != 0)
    {
       scriptErr(obj.fs.handle,line,SCR_ERR_SWEXP,"%s","Missing CASE keyword");
       err = TRUE;
    }

    arg = ++tmp;
    arg = obj.getArgument(tmp);

    // if there is nothing in arg flag an error and put 1 in arg to prevent
    // more error reports
    if(!arg)
    {
        strcpy(selectExpr,"1");
        scriptErr(obj.fs.handle,line,SCR_ERR_SWEXP,"%s","Missing EXPRESSION");
        err = TRUE;
    }
    else
      strcpy(selectExpr,arg);
    obj.inCase = TRUE;
    return err;
}


    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenSLCTND -> cSCRIPT member function                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize SwitchEnd statements                        */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenSLCTEND(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    char str[MAX_STR_LEN+1];
    ss *  s = NULL;
    int err = 0;

    if(obj.inCase == FALSE)
      return TRUE;
    caseCounter = 0;

    strcpy(str,"ENDIF ");
    err = obj.compLine(pass,str,line);

    // pop old select expression and copy it into select expression
    // this is to support nested select case statements. ss is a stack of type cISTACK
    s = obj.selectStack.pop();

    caseCounter = s->cntr;
    strcpy(selectExpr,s->old);
    delete s->old;
    delete s;
    if(caseCounter == 0) obj.inCase = FALSE;
    return err;

}
    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenDEFAULT -> cSCRIPT member function                            */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize Switch DEFAULT statements                   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenDEFAULT(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    char str[MAX_STR_LEN+1];
    strcpy(str,"ELSE");

    int err = 0;
    obj.compLine(pass,str,line);
    return err;

}
    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenCASE -> cSCRIPT member function                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize Switch CASE statements                      */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenCASE(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    int err = 0,loopFlag=0;
    char tmp1[99],*p1,*p2;
    char * tmpArgs,*strtArgs;

  strtArgs = tmpArgs = args;               // Set temporary pointers to args
  memset(tmp1,'\x0',strlen(tmp1));

  if(obj.inCase == FALSE)
  {
    scriptErr(obj.fs.handle,line,SCR_ERR_CASE,"%s","No SELECT CASE");
    return TRUE;
  }

  if(strcmp(args,"ELSE ") == 0)
  {
    strcpy(compileStr,"ELSE");
    err = obj.compLine(pass,compileStr,line);// Compile the new line
    return err;
  }

  if(caseCounter == 0)
  {                                        // if case counter is 0 then
    caseCounter++;                         // increment case counter
    strcpy(compileStr,"IF ( ( ");          // start to build IF ... THEN
  }
  else                                     // if it's not the first case
   strcpy(compileStr,"ELSEIF ( ( ");       // start to build ELSEIF ... THEN

    //strcpy(tm
    if(selectExpr == NULL || args == NULL) // Make sure there are enough expressions
    {
      scriptErr(obj.fs.handle,line,SCR_ERR_CASE,"%s","Missing expression");
      strcpy(selectExpr,"1");
      strcpy(args,"1");
      return TRUE;
    }
    tmpArgs = strchr(tmpArgs,',');
    if(tmpArgs)                            // If more than 1 expr, get next expr
      tmpArgs++;
    strcpy(tmp1,obj.getArgument(args));

    do
    {                                      // While there are more expressions
      if(!tmpArgs)
        loopFlag ++;                       // if this is the last expr, set loop flag
      // if the argumant is not a range
      if(!strchr(args,'.'))  // See if we are processing a range
      {                                    // expression in the form 1..10
        strcat(compileStr,selectExpr);       // IF not build string to compile
        strcat(compileStr," == ");           // in the form EXP == EXP
        strcat(compileStr,tmp1);
      }
      // else process the range into if ( (selectexpr >= tmp1
      // && selectexpr <= tmp2) then
      else
      {
        // get first parameter into p1
        p1 = args;
        // get second parameter into p2
        p2 = strchr(p1,'.');

        if(*(p2 + 1) == '.')
        {
          *p2 = '\x0';
          p2 = p2+=2;
          if(atoi(p1) > atoi(p2)){
            scriptErr(obj.fs.handle,line,SCR_ERR_CASE,"%s","Range must be ascending");
            strcpy(p1,"1");
            strcpy(p2,"2");
            return err = TRUE;
          }
        }
        else
        {
          scriptErr(obj.fs.handle,line,SCR_ERR_CASE,"%s","Missing range operator");
          return err = TRUE;
        }
        // build string IF ((selectExpr >= p1) && (selectExpr <= p2))
        strcat(compileStr," (");
        strcat(compileStr,selectExpr);
        strcat(compileStr," >= ");
        strcat(compileStr,p1);
        strcat(compileStr," ) && (");
        strcat(compileStr,selectExpr);
        strcat(compileStr," <= ");
        strcat(compileStr,p2);
        strcat(compileStr," ) ");

      }
      if(tmpArgs)                          // If this isn't the last expression
      {
        args = tmpArgs;
        tmpArgs = strchr(tmpArgs,',');     // see if there is yet another expression
        if(tmpArgs)    // If there is move past the comma
          tmpArgs++;
        strcpy(tmp1,obj.getArgument(args));// Copy current expression into tmp1
        strcat(compileStr," ) || ( ");     // set up for the next expression
      }

    }while(!loopFlag);                     // While there are expressions


    strcat(compileStr," ) ) THEN ");       // Append the THEN statement
    err = obj.compLine(pass,compileStr,line);// Compile the new line

    args = strtArgs;                               // Reset args to original postion
    return err;
}



    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* tokenVARS -> cSCRIPT member function                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To tokenize variable declaration statements.            */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::tokenVARS(cSCRIPT & obj, sKEYWORD & inf, char * args,
    int line, int pass)
{
    return obj.doVariable(-1*inf.tokenVal,args,line,pass);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* wrVID -> cSCRIPT member function                                   */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To output a Variable ID token                           */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* var   -> (char *) A pointer to a variable name and optional        */
    /*          subscripts                                                */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /* issys -> (int) A flag indicating whether or not a variable is a    */
    /*          system variable (constant) (named "~T~#~")                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs writing; FALSE otherwise             */
    /*                                                                    */
    /**********************************************************************/

#pragma argsused
int pascal cSCRIPT::wrVID(char * var, int line, int pass, int issys)
{
    // If var is not system and not legal, return with error
    if (!issys && !legalName(var))
    {
        scriptErr(fs.handle,line,SCR_ERR_VARBAD,"%s",formattedLine(var));
        return TRUE;
    }

    // Find subs in var
    char * subs = strchr(var,'(');
    if (subs)
    {
        // Get past the open parenthesis
        ++subs;

        // Get count of subscripts
        int ac = ((subs != NULL) ? argCount(subs,TRUE) : 0);

        // Return a warning since this function can't write subscripts
        scriptErr(fs.handle,line,-SCR_WRN_ARGTM,"ARRAY:%d:%d",0,ac);
        warnFlag = TRUE;
    }

    // Get ptr to var and return if error
    cVAR * vptr = getVariable(var,line);
    if (vptr == NULL) return TRUE;

    // Write var id, returning error if necessary
    if (addTokenToBuffer(vptr->info.id)) return TRUE;

    // Return error status
    return FALSE;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* wrVIDSUB -> cSCRIPT member function                                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To output a Variable ID token, subscript count, & subs  */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* var   -> (char *) A pointer to a variable name and optional        */
    /*          subscripts                                                */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /* issys -> (int) A flag indicating whether or not a variable is a    */
    /*          system variable (constant) (named "~T~#~")                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs writing; FALSE otherwise             */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::wrVIDSUB(char * var, int line, int pass, int issys)
{
    // If var is not system and not legal, return with error
    if (!issys && !legalName(var))
    {
        scriptErr(fs.handle,line,SCR_ERR_VARBAD,"%s",formattedLine(var));
        return TRUE;
    }

    // Find subs in var
    char * subs = strchr(var,'(');
    if (subs != NULL) ++subs;

    // Get ptr to var and return if error
    cVAR * vptr = getVariable(var,line);
    if (vptr == NULL) return TRUE;
    int defdim = vptr->info.dims;

    // Get count of subscripts
    int ac = ((subs != NULL) ? argCount(subs,TRUE) : 0);

    // If array subscript checking is disabled ...
    if (disArrSubChk)
    {
        // If there are too many (3 is the maximum) generate a warning
        if (ac > 3)
        {
            if (pass == 1)
            {
                scriptErr(fs.handle,line,-SCR_WRN_ARGTM,"ARRAY:%d:%d",3,ac);
                warnFlag = TRUE;
            }
            // Force it back to three subscripts
            ac = 3;
        }
    }
    // Otherwise check the subscripts
    else if (ac != defdim)
    {
        // If not enough then an error
        if (ac < defdim)
        {
            scriptErr(fs.handle,line,SCR_ERR_ARGNE,"ARRAY:%d:%d",defdim,ac);
            return TRUE;
        }
        // If too many then a warning
        else if (ac > defdim)
        {
            if (pass == 1)
            {
                scriptErr(fs.handle,line,-SCR_WRN_ARGTM,"ARRAY:%d:%d",defdim,ac);
                warnFlag = TRUE;
            }
            // Force it back to the default dimensions
            ac = defdim;
        }
    }

    // Write var id and subscript count, returning error if necessary
    if (addTokenToBuffer(vptr->info.id) || addTokenToBuffer(ac)) return TRUE;

    // Process subscripts
    int err = FALSE;

    char * lastNextToken = nextToken;
    nextToken = NULL;

    char * item = getArgument(subs);
    for (int i = 0; !err && (i < ac); ++i)
    {
        err = in2post(item,line,pass);
        item = getArgument(NULL);
    }
    nextToken = lastNextToken;

    // Return error status
    return err;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* getLabel -> cSCRIPT member function                                */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To return the offset in the code buffer associated with */
    /*            a given label                                           */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* name  -> (char *) A pointer to a label name                        */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) 0 for pass 1 or the offset for pass 2 or -1 for error        */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::getLabel(char * name, int line, int pass)
{
    // If pass 1 then don't bother looking for label
    if (pass == 1) return 0;

    char labPrefix[MAX_STR_LEN];
    if(inFunc && *name != '~')
      sprintf(labPrefix,"%s%s%s","~FL~",idPrefix+3,name);
    else
      strcpy(labPrefix,name);
    // Find the label, returning an error
    cLABEL * labptr = labLst.firstThat(cLABEL::labNameMatch,labPrefix);
    if (labptr == NULL)
    {
        if (pass == 2)
        {
            if(*name == '~' && *(name+1) == 'F' && *(name+2) == '~')
            {
              char * tmp;
              tmp = strchr(name+3,'~');
              *tmp = '\x0';
              scriptErr(fs.handle,line,SCR_ERR_NOFNC,"%s",name+3);
            }
            else
             scriptErr(fs.handle,line,SCR_ERR_LABNF,"%s",formattedLine(name));
        }
        return -1;
    }

    // Return the label pointer
    return labptr->ptr;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* getVariable -> cSCRIPT member function                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To return the pointer of a given variable name          */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* name  -> (char *) A pointer to a variable name and optional        */
    /*          subscripts                                                */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) 0 for pass 1 or the offset for pass 2 or -1 for error        */
    /*                                                                    */
    /**********************************************************************/

cVAR * pascal cSCRIPT::getVariable(char * name, int line, int dovarflag)
{
    // Find open parenthesis for subscripts (if present)
    char tmp1[MAX_STR_LEN+1];
    strcpy(tmp1,name);
    stripSpaces(tmp1);
    char * ptr = strchr(tmp1,'(');
    char * ptr1= strchr(name,'(');

    // If open parenthesis found, temporarily null terminate name only
    if (ptr != NULL) *ptr = '\0',*ptr1 = '\x0';

    // Find named variable
    cVAR * varptr;

    // if we're in a function and not trying to create a variable, see if it
    // exists locally, if not see if it exists globally
    if(inFunc && !dovarflag && legalName(name) && *name != '~'){
      sprintf(tmp1,"%s%s",idPrefix,name);
      varptr = varLst.findName(tmp1);
      if(!varptr){
        //strcpy(name,tmp1);
        varptr = varLst.findName(name);
      }
    }
    else
       varptr = varLst.findName(tmp1);


    // Restore open parenthesis
    if (ptr != NULL) *ptr = '(',*ptr1 = '(';

    // If the variable isn't found and we aren't trying to create it, error
    if ((varptr == NULL) && !dovarflag)
    {
        scriptErr(fs.handle,line,SCR_ERR_VARNF,"%s",formattedLine(name));
        return NULL;
    }

    // Otherwise, return pointer
    return varptr;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* cmpOP -> cSCRIPT member function                                   */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To compare two function names and determine if they are */
    /*            the same or less or greater (used for bsearch in anop)  */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* key   -> (const void *) The value we are looking for               */
    /* elem  -> (const void *) The value from opArray to check            */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) < 0 for key < elem, 0 for key == elem, > 0 for key > elem    */
    /*                                                                    */
    /**********************************************************************/

int cSCRIPT::cmpOP(const void * key, const void * elem)
{
    char * token = ((sOPERATOR *)elem)->token;
    return memcmp((char *)key,token+1,*token);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* anop -> cSCRIPT member function                                    */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To check for an operator or function in a string        */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* e     -> (char *) The string to search in                          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an operator/function was found at e, FALSE otherwise */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::anop(char * e)
{
    // If *e is non-0 and strchr returns non-0, it is an operator
    if (*e && strchr("!%&()*+-/<>=^|",*e)) return TRUE;
    return (bsearch(e,opArray,NUM_UNIQUE_OPS,sizeof(sOPERATOR),cmpOP) != NULL);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* cmpSYMS -> cSCRIPT member function                                 */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To compare two symbols and determine if they are the    */
    /*            same or less or greater (used for bsearch in isConst)   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* key   -> (const void *) The value we are looking for               */
    /* elem  -> (const void *) The value from constInfo to check          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) < 0 for key < elem, 0 for key == elem, > 0 for key > elem    */
    /*                                                                    */
    /**********************************************************************/

int cSCRIPT::cmpSYMS(const void * key, const void * elem)
{
    char * symbol = ((sCONSTINFO*)elem)->symbol;
    return memcmp((char *)key,symbol+1,*symbol);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* isConst -> cSCRIPT member function                                 */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To check for a constant symbol in a string              */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* s     -> (char *) The string to search in                          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if a constant symbol was found at s, FALSE otherwise    */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::isConst(char * s)
{
    sCONSTINFO * found = (sCONSTINFO *)
        bsearch(s,constInfo,NUM_CONST,sizeof(sCONSTINFO),cmpSYMS);

    if (found == NULL) return 0;       // If not found, return 0

    return (int)((found-constInfo)+1); // Otherwise return index + 1
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* findTypeAndVal -> cSCRIPT member function                          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Determine the data type and value of a literal in a     */
    /*            string                                                  */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* s     -> (char *) The string to check                              */
    /* type  -> (int &, returned) The type of s                           */
    /* val   -> (uDEFVALS &, returned) The value of s                     */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurred, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::findTypeAndVal(char * s, int & type, uDEFVALS & val)
{
    // Find the end of string for later processing
    char * eos = s+strlen(s)-1;

    // Default integer format type
    // b = bin, o = oct, d = dec, h = hex, 0 = normal format
    // X means that s is a 'non-standard' type that needed special
    // processing and has already been converted; if the value needs
    // to be handled by the standard integer converted, intFmt will be changed
    // to one of b/o/d/h/0
    char intFmt = 'X';

    // Default to type INTEGER, override later if necessary
    type = vtINTEGER;

    if      (isConst(s))       // Is it a predefined PPL constant?
    {
        val.integerval = constInfo[isConst(s)-1].value;
    }
    else if (isATX(s))         // Is it an @X code?
    {
        val.integerval = getATX(s);
    }
    else if (isMoney(s))       // Is it a monetary format?
    {
        cVARVAL m(vtMONEY,s);  // Allow variable value class to auto convert
        val.integerval = m.values.vMONEY;
    }
    else if (isBinary(s))      // Is it binary?
    {
        intFmt = 'b';
    }
    else if (isOctal(s))       // Is it octal?
    {
        intFmt = 'o';
    }
    else if (isDecimal(s))     // Is it decimal?
    {
        intFmt = 'd';
    }
    else if (isHexadecimal(s)) // Is it hexadecimal?
    {
        intFmt = 'h';
    }
    else if (isInteger(s))     // Is it a normal integer?
    {
        intFmt = '0';
    }
    else if (isReal(s))        // Is it a real value?
    {
        type = vtDREAL;        // Assign proper type and convert str to real
        val.realval = strtod(s,NULL);
    }
    else if (isString(s))      // Is it a string value?
    {
        type = vtSTRING;       // Assign type
        *eos = '\0';           // Get rid of closing quote

        val.stringval = s+1;   // Skip open quote

        while (*s != '\0')     // Fold double quote sequences to single quotes
        {                      // (change "" to ")
            if ((*s == '\"') && (*(s+1) == '\"')) strcpy(s,s+1);
            ++s;
        }
    }
    else                       // The type could not be identified
    {
        type = -1;
    }

    // If the type is INTEGER and we didn't already convert it above
    if ((type == vtINTEGER) && (intFmt != 'X'))
    {
        // Get rid of number base character if present
        if (strchr("bodh",intFmt) != NULL) *eos = '\0';

        int base;

        switch (intFmt)
        {
            case 'b': base =  2; break; // binary      = base  2
            case 'o': base =  8; break; // octal       = base  8
         // case 'd': base = 10; break; // decimal     = base 10 (default)
            case 'h': base = 16; break; // hexadecimal = base 16
         // case '0': base = 10; break; // integer     = base 10 (default)
            default : base = 10; break; // default     = base 10
        }

        // NOTE:  PPL constants are *always* positive; if a negative value
        //        is used (such as -1) the - is treated as a unary minus
        //        operator and acts on a postive value.  The following code
        //        is used to determine if the signed long and unsigned long
        //        versions of s are the same.  If they are, the value can
        //        be represented as a signed long integer (good).  If they
        //        aren't the same, we must convert the integer value to
        //        a real value for it to be handled properly.  This code
        //        relies on the C run-time library returning ULONG_MAX
        //        (0xFFFFFFFFUL) from strtoul when an overflow condition
        //        occurs.

        // Convert the value of s and assign it to both a unsigned long and
        // a signed long
        unsigned long ulv;
        val.integerval = (long) ulv = strtoul(s,NULL,base);

        // If the values aren't the same when cast to doubles, the value needs
        // more precision than is available via integer values and must be
        // converted to a double
        if ((double) ulv != (double) val.integerval)
        {
            type = vtDREAL;
            val.realval = strtod(s,NULL);
        }
    }

    // Return error if no value type information exists
    return (type == -1);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* processOp -> cSCRIPT member function                               */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Process operators while converting an infix expression  */
    /*            to a postfix expression                                 */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* s            -> (char *) Pointer to the expression being converted */
    /* token        -> (int) The offset of the operator token in opArray  */
    /* lastTokenVar -> (int &) "Last token was a variable" flag           */
    /* e            -> (char * &) Pointer to current portion of           */
    /*                 expression being converted                         */
    /* line         -> (int) The current line number being compiled       */
    /* pass         -> (int) The current pass number                      */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurred, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::processOp(char * s, int token, int & lastTokenVar,
    char * & e, int line, int pass)
{
    // Assume no errors
    int err = FALSE;

    // Is it close parenthesis?
    if (opArray[token].tokenVal == TOK_OP_CPAR)
    {
        // Pop operators and append to postfix expression in PPE buffer
            // while open parenthesis not found and stack not empty
        while ((opArray[token].tokenVal != TOK_OP_OPAR) &&
            !tokenStack->isEmpty())
        {
            token = tokenStack->pop();

            // If no room for non open paren token return error
            if ((opArray[token].tokenVal != TOK_OP_OPAR) &&
                addTokenToBuffer(opArray[token].tokenVal))
            {
                err = TRUE;
                break;
            }
        }

        // If no error and open parenthesis not found display error
        if (!err && (opArray[token].tokenVal != TOK_OP_OPAR))
        {
            scriptErr(fs.handle,line,SCR_ERR_PAREN2,"%s",formattedLine(s));
            err = TRUE;
        }

        // If no error set "last token was a variable" status to true
        if (!err) lastTokenVar = TRUE;
    }
    // Is it open parenthesis?
    else if (opArray[token].tokenVal == TOK_OP_OPAR)
    {
        // Push current token
        tokenStack->push(token);

        // Last token not a variable
        lastTokenVar = FALSE;
    }
    // Is it a function?
    else if (opArray[token].reqArgs > -1)
    {
        // Make a copy of the expression
        char * t = strdup(e);

        // If no memory for string copy, return error
        if (t == NULL)
        {
            scriptErr(fs.handle,line,SCR_ERR_MEM,"PARSING FUNCTION");
            fatalErr = TRUE;
            err = TRUE;
        }
        else
        {
            // Count how many arguments are available for function
            int args = argCount(t,TRUE);

            // If there isn't a matching close parenthesis, error
            if (args == -1)
            {
                scriptErr(fs.handle,line,SCR_ERR_PAREN1,"%s",formattedLine(s));
                err = TRUE;
            }

            if (!err)
            {
                // Skip past function arguments
                e += strlen(t)+1;

                // If not enough arguments, error
                if (args < opArray[token].reqArgs)
                {
                    int len = strlen(opArray[token].token+1)-1;
                    scriptErr(fs.handle,line,SCR_ERR_ARGNE,"%*.*s:%d:%d",
                        len,len,opArray[token].token+1,opArray[token].reqArgs,args);
                    err = TRUE;
                }
                // If too many arguments
                else if ((args > opArray[token].reqArgs) && (pass == 1))
                {
                    int len = strlen(opArray[token].token+1)-1;
                    scriptErr(fs.handle,line,-SCR_WRN_ARGTM,"%*.*s:%d:%d",
                        len,len,opArray[token].token+1,opArray[token].reqArgs,args);
                    warnFlag = TRUE;
                }
            }

            if (!err)
            {
                // Save and reset argument processor
                char * lastNextToken = nextToken;
                nextToken = NULL;

                // Get first argument from arguments
                char * ptr = getArgument(t);

                // Process all arguments while no errors
                for (int i = 0; (i < args) && !err; ++i)
                {
                    // Convert infix to postfix
                    err = in2post(ptr,line,pass);
                    // Backup over 0x0000 that terminates sub-expression
                    scriptPointer -= sizeof(int);
                    // Decrease PPE buffer size appropriately
                    scriptSize -= sizeof(int);
                    // Get next argument from arguments
                    ptr = getArgument(NULL);
                }

                // Restore previous argument state
                nextToken = lastNextToken;
            }

            // Deallocate expression memory
            free(t);

            // Set error if problem adding token
            if (err || addTokenToBuffer(opArray[token].tokenVal))
                err = TRUE;

            // Last token a variable if no errors
            if (!err) lastTokenVar = TRUE;
        }
    }
    // Else it is a real unary or binary operator
    else
    {
        // Initial continue is TRUE
        int contFlag = TRUE;

        // Loop as long as there are tokens with the same or higher priority to pop
        while (contFlag)
        {
            contFlag = FALSE;
            // Get op token from stack if available
            int tmptok = (tokenStack->isEmpty() ? -1 : tokenStack->top());
            // If token available and higher than cur token
            if ((tmptok != -1) &&
                (opArray[tmptok].tokenPri >= opArray[token].tokenPri))
            {
                // Pop op token from stack
                tmptok = tokenStack->pop();
                // If no room for op token break with error
                if (addTokenToBuffer(opArray[tmptok].tokenVal))
                    return TRUE;
                // Continue if there was a >= priority
                contFlag = TRUE;
            }
        }

        // Push current token
        tokenStack->push(token);

        // If error break
        if (err) return TRUE;

        // Last token not a variable
        lastTokenVar = FALSE;
    }

    return err;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* processArg -> cSCRIPT member function                              */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Process operators while converting an infix expression  */
    /*            to a postfix expression                                 */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* tmpsym       -> (char *) A pointer to the symbol being evaluated   */
    /* legal        -> (int) A flag indicating the legality of tmpsym     */
    /*                 as a variable name                                 */
    /* line         -> (int) The current line number being compiled       */
    /* pass         -> (int) The current pass number                      */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurred, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::processArg(char * tmpsym, int legal, int line, int pass)
{
    uDEFVALS val;  // The value of a constant or literal as defined in tmpsym
    int      type; // The type of the constant or literal as defined in tmpsym

    // If an error in evaluating type and value of tmpsym or tmpsym not legal
    if (findTypeAndVal(tmpsym,type,val) && !legal)
    {
        scriptErr(fs.handle,line,SCR_ERR_IMVC,"");
        return TRUE;
    }

    char constName[MAX_LAB_NAME_SIZE+1];
    cVAR * p = NULL;

    // If the name is legal then get the name
    if (legal)
    {
        char * ptr = strchr(tmpsym,'(');
        if (ptr != NULL) *ptr = '\0';

        strcpy(constName,tmpsym);
        if (ptr != NULL) *ptr = '(';
    }
    // Otherwise it must be a constant or literal
    else
    {
        switch (type)
        {
            case vtINTEGER:
                // Build the name from the value of the constant/literal
                sprintf(constName,"~I~%ld~",val.integerval);
                break;

            case vtDREAL:
                // Build the name from the value of the constant/literal
                sprintf(constName,"~R~%+024.16E~",val.realval);
                break;

            case vtSTRING:
                // Attempt to find a string with the value we're about to add
                p = varLst.findString(val.stringval);
                // If found, use the existing name, otherwise build a new one
                if (p != NULL)
                    strcpy(constName,p->name);
                else
                    sprintf(constName,"~S~%d~",curVarID+1);
                break;
        }
    }


    // Lookup the variable/constant in the list.

      p = getVariable(constName,line,!legal);

      if(p->data->type == vtFUNCTION || p->data->type == vtPROCEDURE)
      {

       scriptErr(fs.handle,line,SCR_ERR_VARNF,"%s",formattedLine(tmpsym));
       return TRUE;
      }

    // If the variable was not found and not legal, create it (or return error)
    if ((p == NULL) && !legal)
    {
        if (doVariable(type,constName,line,pass,TRUE,&val))
            return TRUE;
    }
    // Else if the variable was not found and is legal, error
    else if (p == NULL)
        return TRUE;

    // Write the variable and subscript information to the PPE buffer
    return wrVIDSUB(legal ? tmpsym : constName, line, pass, TRUE);
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* in2post -> cSCRIPT member function                                 */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Convert expressions from source code infix format to    */
    /*            tokenized postfix format                                */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* e    -> (char *) A pointer to the expression to convert            */
    /* line -> (int) The current line number being compiled               */
    /* pass -> (int) The current pass number                              */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurred, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::in2post(char * e, int line, int pass)
{
    // Save the current token stack to support recursive calls
    tokenStkStk.push(tokenStack);

    // Create a new token stack
    tokenStack = new tTS;

    // If not enough memory for new token stack, restore previous and error
    if (tokenStack == NULL)
    {
        scriptErr(fs.handle,line,SCR_ERR_MEM,"CONVERTING EXPRESSION");
        fatalErr = TRUE;
        tokenStack = tokenStkStk.pop();
        return TRUE;
    }

    // Save the current argument tokenization status and initialize
    char * lastNextToken = nextToken;
    nextToken = NULL;

    // Save start of expression string and strip spaces
    char * s = e;
    stripSpaces(e);

    // If e has length 0, disp error and return
    if (*e == '\0')
    {
        scriptErr(fs.handle,line,SCR_ERR_NOEXPR,"");
        if(tokenStack)
        {
          delete tokenStack;
          // Restore the previous token stack
          tokenStack = tokenStkStk.pop();
        }
        return TRUE;
    }

    // Create symbol buffer
    char * tmpsym = new char[strlen(e)+1];

    // If insufficient memory for symbol buffer, disp error and return
    if (tmpsym == NULL)
    {
        scriptErr(fs.handle,line,SCR_ERR_MEM,"CONVERTING EXPRESSION");
        fatalErr = TRUE;
        delete tokenStack;
        tokenStack = tokenStkStk.pop();
        return TRUE;
    }

    // Assume no errors default
    int err = FALSE;

    // At beginning of string, last token was not a variable
    int lastTokenVar = FALSE;

    // While we haven't encountered an error or end of expression
    while (!err && (*e != '\0'))
    {
        // Get first non-operator token
        char * tmpptr = tmpsym;
        int qStat = FALSE;

        // Make sure we start off with an empty string (no token)
        *tmpptr = '\0';

        // While we haven't reached the end of the expression
        while (*e != '\0')
        {
            // If the symbol isn't/can't be a real constant
            // (If the temp symbol is not real or
            //  the last char is not E (for exp notation) or
            //  the cur char is not + or -)
            if (!(isReal(tmpsym) &&
                  isascii(*(tmpptr-1)) && (toupper(*(tmpptr-1)) == 'E') &&
                  ((*e == '-') || (*e == '+'))))
            {
                // If not in quote mode &
                // we are sitting on an operator &
                // we have no symbol or we are on an operator, then break
                if (!qStat && anop(e) && ((tmpptr == tmpsym) ||
                    (strchr("!%&()*+-/<>=^|",*e) != NULL)))
                    break;
            }

            // If we happened upon a quote
            if (*e == '\"')
            {
                // If it is an end of string quote, toggle quote mode
                if (*(e+1) != '\"')
                    qStat = !qStat;
                // If it is an embedded double quote, copy directly
                else
                    *(tmpptr++) = *(e++);
            }

            // Copy character and null terminate the string
            *(tmpptr++) = *(e++);
            *tmpptr = '\0';
        }

        // Ensure the string is null terminated
        *tmpptr = '\0';

        // Copy subscripts if needed
        if ((strlen(tmpsym) > 0) && (*e == '('))
        {
            // Copy open parenthesis and null terminate
            *(tmpptr++) = *(e++);
            *tmpptr = '\0';

            // Initialize parentheses counter and quote status
            int pCount = 1;
            qStat = FALSE;

            // While we haven't reached the end of the expression and
            // we still have mismatched parentheses
            while ((*e != '\0') && (pCount > 0))
            {
                // Toggle quote status if we're on a quote
                if (*e == '\"') qStat = !qStat;

                // If we're not in quote mode
                if (!qStat)
                {
                    // Handle open and close parentheses
                    if (*e == '(')
                        ++pCount;
                    else if (*e == ')')
                        --pCount;
                }

                // Copy character and null terminate string
                *(tmpptr++) = *(e++);
                *tmpptr = '\0';
            }

            // Ensure the string is null terminated
            *tmpptr = '\0';
        }

        // Ensure the string is null terminated
        *tmpptr = '\0';

        int legal, token;


        // If there is anything in tmpsym, we have a var or const
        if (strlen(tmpsym) > 0)
        {


            // If the last token was a variable, error
            if (lastTokenVar)
            {
                scriptErr(fs.handle,line,SCR_ERR_IMOP,"");
                err = TRUE;
                break;
            }

            // We know the last token was a variable/constant
            lastTokenVar = TRUE;

            // Determine legality
            legal = legalName(tmpsym);


            // if it is not a function call

            if(!isCall(*this,tmpsym,line,pass,FALSE,TRUE, &err))
              // Process the argument
              err = processArg(tmpsym,legal,line,pass);
            if (err) break;
        }
        // Otherwise it is an operator/function
        else
        {
            // Calculate where we should start searching for the
            // operator/function in the opArray
            // (part 1 is functions, part 2 is operators)
            token = (isalpha(*e) ? 0 : NUM_UNIQUE_OPS+1);

            // Find the operators token index
            // The token must match the expression or it must be a
            // binary operator with two operands
            // Otherwise check the next entry in opArray
            while (((!matchSubStrPasStr(e,opArray[token].token)) ||
                (!lastTokenVar && opArray[token].isBinary)) &&
                (opArray[token].token != NULL))
                ++token;

            // If we didn't find the token, error
            if (opArray[token].token == NULL)
            {
                scriptErr(fs.handle,line,SCR_ERR_IMVC,"");
                err = TRUE;
                break;
            }

#ifdef PCB_DEMO
            // If the operators token value is greater than 0, unsupported
            if (opArray[token].tokenVal > 0)
            {
                newline();
                newline();
                print("ERROR: The ");
                print(opArray[token].token+1);
                print("\x08 function is not supported in the PPLC DEMO ");
                return TRUE;
            }
#endif

            // Adjust expression pointer past the operator
            e += *opArray[token].token;

            // Process the operator/function
            err = processOp(s,token,lastTokenVar,e,line,pass);
            if (err) break;
        }
    }

    // We don't need the temporary symbol anymore
    delete tmpsym;

    // If we didn't have a variable/constant as the last token in the
    // expression, it's an error
    if (!lastTokenVar)
    {
        scriptErr(fs.handle,line,SCR_ERR_ENDOP,"%s",formattedLine(s));
        err = TRUE;
    }

    // While no error and the token stack isn't empty
    while (!err && !tokenStack->isEmpty())
    {
        // Pop op token from stack
        int tmptok = tokenStack->pop();

        // If we found an unprocessed open parenthesis, error
        if (opArray[tmptok].tokenVal == TOK_OP_OPAR)
        {
            scriptErr(fs.handle,line,SCR_ERR_PAREN2,"%s",formattedLine(s));
            err = TRUE;
            break;
        }
        // If no room for operator token, break with error
        else if (addTokenToBuffer(opArray[tmptok].tokenVal))
        {
            err = TRUE;
            break;
        }
    }

    // Flush any remaining tokens
    tokenStack->flush();

    // Delete the token stack
    delete tokenStack;

    // Restore the previous token stack
    tokenStack = tokenStkStk.pop();

    // If no room for end operator token set error condition
    if (!err && addTokenToBuffer(TOK_OP_END)) err = TRUE;

    // Restore the argument tokenizer to it's original state
    nextToken = lastNextToken;

    // Return error status
    return err;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* doLabel -> cSCRIPT member function                                 */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Add a destination label and it's offset to the list of  */
    /*            labels                                                  */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* t       -> (char *) A pointer to the label to add                  */
    /* curLine -> (int) The current line number being compiled            */
    /* issys   -> (int) A flag indicating if t is a system label          */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurred, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::doLabel(char * t, int curLine, int issys)
{
    char labPrefix[MAX_STR_LEN];
    stripSpaces(t);
    if(inFunc && *t != '~')
      sprintf(labPrefix,"%s%s%s","~FL~",idPrefix+3,t);   // The third position is the beggining of the function name
    else
      strcpy(labPrefix,t);
    // Is it a legal name?
    int legal = (issys ? TRUE : legalName(t));

    // If it's legal, is it already in the list?
    cLABEL * p;
    if (legal) p = labLst.firstThat(cLABEL::labNameMatch,labPrefix);

    // If it's not legal or already in the list, disp error and return
    if (!legal || (p != NULL))
    {
        if (*t == '\0')
            scriptErr(fs.handle,curLine,SCR_ERR_LABMIS,"");
        else if (!legal)
            scriptErr(fs.handle,curLine,SCR_ERR_LABBAD,"%s",formattedLine(t));
        else if (p != NULL)
            scriptErr(fs.handle,curLine,SCR_ERR_LABDEF,"%s",formattedLine(t));
        return TRUE;
    }

    // Attempt to create a label
    cLABEL * l = new cLABEL(labPrefix,scriptSize);

    // If error creating label, disp error and return
    if (l == NULL)
    {
        scriptErr(fs.handle,curLine,SCR_ERR_MEM,"LABEL DEFINITION");
        fatalErr = TRUE;
        return TRUE;
    }

    // Add l to the label list
    if (labLst.add(l) == -1)
    {
        delete l;
        return TRUE;
    }

    // Return OK
    return FALSE;
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* eceSub -> cSCRIPT member function                                  */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Subroutine used by evalConstExpr to evaluate operations */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* invops  -> (char *) A pointer to invalid operators to abort        */
    /* err     -> (int &) A flag to hold error status settings            */
    /*                                                                    */
    /**********************************************************************/

void pascal cSCRIPT::eceSub(char * invops, int & err)
{
    // While there are operators to act on and the last operator is valid
    while (!opStack.isEmpty() &&
        (strchr(invops,*opStack.top()) == NULL))
    {
        // Get the last operator
        char * o = opStack.pop();

        // Get the right and left arguments
        long * r = (argStack.isEmpty() ? NULL : argStack.pop());
        long * l = (argStack.isEmpty() ? NULL : argStack.pop());

        // If either argument or operator is null, error
        if ((o == NULL) || (r == NULL) || (l == NULL))
        {
            err = TRUE;
            break;
        }

        // Evaluate sub-expression
        long result;
        switch (*o)
        {
            case '+':
                result = *l + *r;
                break;

            case '-':
                result = *l - *r;
                break;

            case '*':
                result = *l * *r;
                break;

            case '/':
                result = *l / *r;
                break;

            case '%':
                result = *l % *r;
                break;
        }

        // Store the result on the stack
        argStack.push(new long(result));

        // Delete arguments and operators
        delete l;
        delete r;
        delete o;
    }
}

    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* evalConstExpr -> cSCRIPT member function                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> Evaluates a constant expression used for an array       */
    /*            declaration statement                                   */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* e       -> (char *) A pointer to the expression to evaluate        */
    /* line    -> (int) The current line number being compiled            */
    /* retVal  -> (int &) The value to return                             */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurred, FALSE otherwise                   */
    /*                                                                    */
    /**********************************************************************/

int pascal cSCRIPT::evalConstExpr(char * e, int line, int & retVal)
{
    // Save the start pointer
    char * s = e;

    // Assume no errors
    int err = FALSE;

    // Assume return value of 0
    retVal = 0;

    // While we haven't reached the end of the string and no errors
    while ((*e != '\0') && !err)
    {
        // Do we need to process an operator?
        if (strchr("()+-*/%",*e) != NULL) // operator
        {
            // Switch through operators
            switch (*e)
            {
                case '(': // These have the highest priority and should
                case '*': // simply cause the operator to be pushed for
                case '/': // later processing
                case '%':
                    // pop nothing, push and continue
                    opStack.push(new char(*e));
                    break;

                case '+': // These have the next priority and should cause all
                case '-': // immediate *, /, and % operators to be processed
                    // pop everything to last (+- and operate on it
                    eceSub("(+-",err);
                    opStack.push(new char(*e));
                    break;

                case ')': // End of sub-expression, process back to (
                    // pop everything to last ( and operate on it
                    eceSub("(",err);
                    break;
            }

            // Prepare for next character
            ++e;
        }
        // Do we need to process a number?
        else if (isdigit(*e))
        {
            // Convert string to number and save error position
            char * endPtr;
            long cv = strtol(e,&endPtr,10);

            // Prepare for next character
            e = endPtr;

            // Push value on the stack
            argStack.push(new long(cv));
        }
        // Then it must be an error
        else
        {
            scriptErr(fs.handle,line,SCR_ERR_CEXPCH,"%c",*e);
            err = TRUE;
        }
    }

    // Finish processing the just exited sub-expression
    while (!err && !opStack.isEmpty()) eceSub("(",err);

    // If no error and there is at least an argument on the stack, handle it
    if (!err && !argStack.isEmpty())
    {
        long * rvp = argStack.pop();
        if (*rvp <= 65535U)
            retVal = (int)*rvp;
        else
            err = TRUE;
        delete rvp;
    }

    // If any operators are left, error
    if (!opStack.isEmpty())
    {
        err = TRUE;
        while (!opStack.isEmpty())
        {
            char * op = opStack.pop();
            delete op;
        }
    }

    // If any arguments are left, error
    if (!argStack.isEmpty())
    {
        err = TRUE;
        while (!argStack.isEmpty())
        {
            long * arg = argStack.pop();
            delete arg;
        }
    }

    // Handle error
    if (err) scriptErr(fs.handle,line,SCR_ERR_CEXPR,"%s",formattedLine(s));

    // Return error status
    return err;
}

    /*--------------------------------------------------------------------*/
    /*--------------------------------------------------------------------*/
    /*--------------------------------------------------------------------*/
    /*--------------------------------------------------------------------*/
    /*--------------------------------------------------------------------*/

    /*--------------------------------------------------------------------*/

int pascal cSCRIPT::doVariable(int type, char * args, int curLine, int pass,
    int issys, uDEFVALS * defval)
{
    // Variables already processed during pass 1, so return if pass 2
    if (pass == 2) return FALSE;

    // Get the first token from args
    char * t = getArgument(args);

    char tmp[MAX_STR_LEN+1];
    tmp[0] = '\x0';



    // If no token, disp error and return
    if (t == NULL)
    {
        scriptErr(fs.handle,curLine,SCR_ERR_VARMIS,"");
        return TRUE;
    }

    // All occurances of t have been replaced with tmp so processing t will
    // not affect the rest of the t string


    // copy token into tmp so name mangling will not overwrite what is in t
    assert(t!= NULL&&tmp!=NULL);
    strcpy(tmp,t);
    // While we have tokens
    while (t != NULL)
    {
        // If it is a reserved constant name, error
        if (isConst(tmp))
        {
            scriptErr(fs.handle,curLine,SCR_ERR_CONST,"%s",formattedLine(tmp));
            return TRUE;
        }

        // Are there subscripts?
        int ds[3] = { 0, 0, 0 };

        int sc = 0;

        // find open paren in both t and tmp
        char * s = strchr(tmp,'(');
        char * s1 = strchr(t,'(');
        if (s != NULL)
        {
            // isolate var name in both t and tmp
            *(s++) = '\0';
            *(s1++)= '\x0';

            sc = argCount(s,TRUE);
            if (sc <= -1)
            {
                scriptErr(fs.handle,curLine,SCR_ERR_PAREN1,"%s",formattedLine(s));
                return TRUE;
            }
            else if (sc == 0)
            {
                scriptErr(fs.handle,curLine,SCR_ERR_ARGNE,"ARRAY:1:0");
                return TRUE;
            }
            else if (sc > 3)
            {
                if (pass == 1)
                {
                    scriptErr(fs.handle,curLine,-SCR_WRN_ARGTM,"ARRAY:3:%d",sc);
                    warnFlag = TRUE;
                }
                sc = 3;
            }

            char * lastNextToken = nextToken;
            nextToken = NULL;

            char * t = getArgument(s);
            for (int i = 0; i < sc; ++i)
            {
//                ds[0] = ds[1];
//                ds[1] = ds[2];
//                if (evalConstExpr(t,curLine,ds[2]))
                if (evalConstExpr(t,curLine,ds[3-1-i]))
                    return TRUE;
                t = getArgument(NULL);
            }

            nextToken = lastNextToken;
        }

        // Is it a legal name?
        int legal = (issys ? TRUE : legalName(t));

    // if in a function or procedure then mangle the name
        if(inFunc && legal && !issys ){
           // need to check here to make sure variable is not too large.
           if(strlen(t) > MAX_LAB_NAME_SIZE) t[MAX_LAB_NAME_SIZE] = '\x0';
           sprintf(tmp,"%s%s",idPrefix,t);
        }


        // If it's legal, is it already in the list?
        cVAR *p;
        if (legal) p = getVariable(tmp,curLine,TRUE);

        // If it's not legal or already in the list, disp error and return
        if (!legal || (p != NULL))
        {
            if (*tmp == '\0')
                scriptErr(fs.handle,curLine,SCR_ERR_VARMIS,"");
            else if (!legal)
                scriptErr(fs.handle,curLine,SCR_ERR_VARBAD,"%s",formattedLine(t));
            else if (p != NULL)
                scriptErr(fs.handle,curLine,SCR_ERR_VARDEF,"%s",formattedLine(t));
            return TRUE;
        }

        // Attempt to create a variable


        sVARINFO info = { ++curVarID, sc, ds[2], ds[1], ds[0], type };
        stripSpaces(tmp);
        cVAR * v = new cVAR(tmp,info);
//        cVAR * v = new cVAR(tmp,++curVarID,type,sc,ds[2],ds[1],ds[0]);

        // If error creating variable, disp error and return
        if ((v == NULL) || (v->data == NULL))
        {
            if (v != NULL) delete v;
            scriptErr(fs.handle,curLine,SCR_ERR_MEM,"VARIABLE DECLARATION");
            fatalErr = TRUE;
            return TRUE;
        }

        if (issys)
        {
            switch (type)
            {
                case vtINTEGER:
                    *v->getVal() = defval->integerval;
                    break;

                case vtDREAL:
                    *v->getVal() = defval->realval;
                    break;

                case vtSTRING:
                    *v->getVal() = defval->stringval;
                    break;
            }
        }

        // Add the variable
        if (varLst.add(v) == -1)
        {
            delete v;
            return TRUE;
        }
        // if the var has been created and is a local then the locvarcnt
        // field needs to be incremented.
        else
        {
            if(inFunc)
            {
               // isolate the function name;
               char s[MAX_STR_LEN];
               char *tmp1= NULL, *tmp2=NULL;
               strcpy(s,idPrefix);
               tmp1 = s+3;          // the 3rd postion is the beginning of the function name.
               tmp2 = strchr(tmp1,'~');
               if(tmp2)
               {
                 *tmp2 = '\x0';
                 cVAR * fptr = getVariable(tmp1,curLine,TRUE);
                 if(fptr)
                 {
                   //if(fptr->data->values.vPROCEDURE.locvarcnt != MAX_BYTE_SIZE)
                     fptr->data->values.vPROCEDURE.locvarcnt++;
                   //else
                   // err
                 }
               }
               // If second char is an F and we're in a function or procedure
               // we are garanteed that it is a local variable. Set to 0 so it
               // will be re-initialized.
               if(*(v->name+1) == 'F' && v->info.flags.init != 1)
                 v->info.flags.init = 0;
               else
                 v->info.flags.init = 1;
            }

        }


        // Get next token
        t = getArgument(NULL);
        if(t)
          stripSpaces(t);
        // if in a function the mangle the name.
        if(inFunc && t)
          sprintf(tmp,"%s%s",idPrefix,t);
        if(!inFunc && t)
          strcpy(tmp,t);
    }

    // Return OK
    return FALSE;
}

//    /*--------------------------------------------------------------------*/
//
//char * tlp;
//
//void pascal preprocSub(sSYMBOLINFO & obj)
//{
//    char * p;
////    while ((p = strstr(tlp,obj.sym)) != NULL)
//    if ((p = strstr(tlp,obj.sym)) != NULL)
//    {
//        strcpy(p,p+strlen(obj.sym));
//        memmove(p+strlen(obj.val),p,strlen(p)+1);
//        memcpy(p,obj.val,strlen(obj.val));
//    }
//}
//
    /*--------------------------------------------------------------------*/

#pragma argsused
void pascal cSCRIPT::preprocess(char * s)
{
//    tlp = s;
//    symbolList.forEach(preprocSub);
}

//    /*--------------------------------------------------------------------*/
//
//int pascal symNameMatch(const sSYMBOLINFO & obj, const void * data)
//{
//    return (strcmp(obj.sym,((sSYMBOLINFO*)data)->sym) == 0);
//}
//
//    /*--------------------------------------------------------------------*/
//
//void pascal cSCRIPT::defineSymbol(void)
//{
//    char * s = commentBuf;
//    char * v = strchr(s,'=');
//    if (v) *(v++) = '\0';
//
//    strupr(s);
//
//    if (!legalName(s)) return; // scriptErr
//
//    sSYMBOLINFO si;
//    si.sym = strdup(s);
//    si.val = (v ? strdup(v) : NULL);
//
//    sSYMBOLINFO * sp = symbolList.firstThat(symNameMatch,&si);
//
//    if (sp)
//    {
//        if (sp->val) free(sp->val);
//        sp->val = (v ? strdup(v) : NULL);
//    }
//    else
//        symbolList.add(si);
//}
//
//    /*--------------------------------------------------------------------*/
//
//void pascal cSCRIPT::undefSymbol(void)
//{
//    char * s = commentBuf;
//
//    strupr(s);
//
//    if (!legalName(s)) return; // scriptErr
//
//    sSYMBOLINFO si;
//    si.sym = strdup(s);
//
//    symbolList.detach(si);
//}
//
    /*--------------------------------------------------------------------*/

int pascal cSCRIPT::processComment(int pass, int line)
{
    int err = FALSE;

    if (*commentBuf == '\0') return err;

    stripright(commentBuf,' ');

    if      (matchSubStrPasStr(commentBuf,"\x09""$INCLUDE:") ||
             matchSubStrPasStr(commentBuf,"\x09""#INCLUDE:"))
    {
        DOSFILE tmpFS = fs;
        memset(&fs,0,sizeof(fs));
        if (dosfopen(commentBuf+9,OPEN_READ|OPEN_DENYWRIT,&fs) == 0)
        {
            while (!startStruct.isEmpty())
            {
                err = TRUE;
                scriptErr(tmpFS.handle,int(startStruct.pop()&0x0000FFFFL),
                    SCR_ERR_NOEND,"");
            }

            err = (compPass(pass) || err);
            dosfclose(&fs);
        }
        else
        {
            scriptErr(fs.handle,line,SCR_ERR_MISINC,"%s",strupr(commentBuf));
            err = TRUE;
        }
        fs = tmpFS;
    }
    else if (matchSubStrPasStr(commentBuf,"\x09""$USEFUNCS") ||
           matchSubStrPasStr(commentBuf,"\x09""#USEFUNCS"))
    {
      char cstr[MAX_LINE_SIZE+1];
      strcpy(cstr,"~BEGIN~");
      int os = getLabel(cstr,line,pass);
      if(os > -1  ){
        if(pass == 2 && os <= 0)
          err = TRUE;
        err = addTokenToBuffer(TOK_GOTO);
        err = addTokenToBuffer(os);
      }
      else
        err = TRUE;

    }
//    else if (matchSubStrPasStr(commentBuf,"\x08""$DEFINE:") ||
//             matchSubStrPasStr(commentBuf,"\x08""#DEFINE:"))
//    {
//        strcpy(commentBuf,commentBuf+8);
//        defineSymbol();
//    }
//    else if (matchSubStrPasStr(commentBuf,"\x07""$UNDEF:") ||
//             matchSubStrPasStr(commentBuf,"\x07""#UNDEF:"))
//    {
//        strcpy(commentBuf,commentBuf+7);
//        undefSymbol();
//    }

    *commentBuf = '\0';

    return err;
}

    /*--------------------------------------------------------------------*/

int pascal cSCRIPT::compLine(int pass, char * sBuf, int & curLine)
{
    int err = FALSE;

    int qStat, pCount;

    // Read in multiple lines if "\ " are the last two characters of the line

    char * p;

    do {

        qStat  = FALSE;
        pCount = 0;

        p = standardizeLine(sBuf, qStat, pCount);

        if ((strlen(sBuf) > 1) && (strcmp(sBuf+strlen(sBuf)-2,"\\ ") == 0))
        {
            dosfgets(sBuf+strlen(sBuf)-2,MAX_LINE_SIZE+1-strlen(sBuf)+2,&fs);
            ++curLine;
        }
        else
            break;

    } while (TRUE);

    // Break into multiple logical statements if necessary

    if (p != NULL)
    {
        *(p++) = '\0';
        strcpy(nextLogStatement,p);
        standardizeLine(sBuf, qStat, pCount);
    }

    //preprocess(sBuf);

    // If a problem with quotes or parenthesis, disp error and return
    if (qStat || (pCount != 0))
    {
        if (qStat)
            scriptErr(fs.handle,curLine,SCR_ERR_QUOTE,"%s",formattedLine(sBuf));
        if (pCount > 0)
            scriptErr(fs.handle,curLine,SCR_ERR_PAREN1,"%s",formattedLine(sBuf));
        else if (pCount < 0)
            scriptErr(fs.handle,curLine,SCR_ERR_PAREN2,"%s",formattedLine(sBuf));
        err = TRUE;
    }

    // If we're sitting on a label during pass 1
    if (!err && (*sBuf == ':') && (pass == 1))
    {
        if (doLabel(sBuf+1,curLine) != 0) err = TRUE;
    }
    // Else it better be a keyword
    else if (!err && (*sBuf != ':') && (strlen(sBuf) > 1))
    {
        // Find the keyword
        int token = 0;
        while ((kwArray[token].token != NULL) &&
            !matchSubStrPasStr(sBuf,kwArray[token].token))
            ++token;

        // If no keyword found, attempt a let or check for procedure call
        if ((kwArray[token].token == NULL) ||
            (sBuf[*kwArray[token].token] == '='))
        {
            // If it is a legitimate procedure call
            if(isCall(*this,sBuf,curLine,pass,TRUE,FALSE,&err))
              return err;

            // If there's an error tokenizing the line
            sKEYWORD inf = { "LET ", TOK_LET, 1, 1, FALSE, cSCRIPT::tokenLET };
            if (tokenLET(*this,inf,sBuf,curLine,pass))
            {
                // Display error if script grew too large
                // (tokenize functions handle all other errors)
                if (scriptSize == SCR_BUF_SIZE)
                {
                    scriptErr(0,-1,SCR_ERR_TOOBIG,"");
                    fatalErr = TRUE;
                    ++scriptSize;
                }
                // Return error
                err = TRUE;
            }

        }
        else
        {
            // If there's an error tokenizing the line
            if (kwArray[token].tokenFunc(*this,kwArray[token],
                sBuf+*kwArray[token].token,curLine,pass))
            {
                // Display error if script grew too large
                // (tokenize functions handle all other errors)
                if (scriptSize == SCR_BUF_SIZE)
                {
                    scriptErr(0,-1,SCR_ERR_TOOBIG,"");
                    fatalErr = TRUE;
                    ++scriptSize;
                }
                // Return error
                err = TRUE;
            }
        }
    }

    // Return status from line compile or comment process
    return (err || processComment(pass,curLine));
}

    /*--------------------------------------------------------------------*/

int pascal cSCRIPT::compPass(int pass)
{
    static int level = 0;

    char * sBuf = (char *) malloc(MAX_LINE_SIZE+1);
    if (sBuf == NULL)
    {
        scriptErr(0,-1,SCR_ERR_MEM,"FILE READ BUFFER");
        fatalErr = TRUE;
        return TRUE;
    }

    ++level;

    *nextLogStatement = '\0';
    *commentBuf       = '\0';

    int err = FALSE;

    if ((pass == 1) && (level == 1) && autoUVar)
    {
        // System Variable ID's:
        // ---------------------
        //  1 = U_EXPERT     2 = U_FSE        3 = U_FSEP       4 = U_CLS
        //  5 = U_EXPDATE    6 = U_SEC        7 = U_PAGELEN    8 = U_EXPSEC
        //  9 = U_CITY      10 = U_BDPHONE   11 = U_HVPHONE   12 = U_TRANS
        // 13 = U_CMNT1     14 = U_CMNT2     15 = U_PWD       16 = U_SCROLL
        // 17 = U_LONGHDR   18 = U_DEF79     19 = U_ALIAS     20 = U_VER
        // 21 = U_ADDR      22 = U_NOTES     23 = U_PWDEXP

        err =
            doVariable(vtBOOLEAN,"U_EXPERT,U_FSE,U_FSEP,U_CLS",-1,pass) ||
            doVariable(vtDATE,"U_EXPDATE",-1,pass) ||
            doVariable(vtINTEGER,"U_SEC,U_PAGELEN,U_EXPSEC",-1,pass) ||
            doVariable(vtSTRING,"U_CITY,U_BDPHONE,U_HVPHONE,U_TRANS,"
                "U_CMNT1,U_CMNT2,U_PWD",-1,pass) ||
            doVariable(vtBOOLEAN,"U_SCROLL,U_LONGHDR,U_DEF79",-1,pass) ||
            doVariable(vtSTRING,"U_ALIAS,U_VER,U_ADDR(5),U_NOTES(4)",-1,pass) ||
            doVariable(vtDATE,"U_PWDEXP",-1,pass) ||
            doVariable(vtINTEGER,"U_ACCOUNT(16)",-1,pass) ||

            /* 15.4 additions per WHATSNEW.  Confirmed by PPLC 15.4b data
             * strings at file offset 0x2f055 ("U_SHORTDESC") and 0x2f06a
             * ("U_GENDER,U_BIRTHDATE,U_EMAIL,U_WEB").  These are the
             * per-field aliases that give scripts direct read/write to
             * the Personal PSA members.  U_PERSONAL(idx) from v0.021 is
             * the indexed accessor that works alongside these.
             */
            doVariable(vtBOOLEAN,"U_SHORTDESC",-1,pass) ||
            doVariable(vtSTRING,"U_GENDER,U_BIRTHDATE,U_EMAIL,U_WEB",-1,pass);
    }

    int curLine = 0;

    char whirly[] = "/-\\|";
    int off = 0;

    if (level == 1)
    {
        newline();
        newline();
        print("Pass ");
        print((pass == 1) ? "1" : "2");
        print(" ... ");
    }

    if ((level == 1) && dispStat) print(" ");

    while (!fatalErr && (scriptSize < SCR_BUF_SIZE) &&
        (dosfgets(sBuf,MAX_LINE_SIZE+1,&fs) != -1))
    {
        if (((curLine++ & 0x0F) == 0) && dispStat)
        {
            char buf[3];
            buf[0] = '\x08';
            buf[1] = whirly[off++];
            buf[2] = '\0';
            print(buf);
            off &= 0x03;
        }

#ifdef WATCH_MEM
        int x = wherex();
        int y = wherey();
        gotoxy(1,1);
        cprintf("%d - %lu",curLine,coreleft());
        clreol();
        gotoxy(x,y);
#endif

#ifdef PCB_DEMO
        if (curLine > 10)
        {
            newline();
            newline();
            print("ERROR: Maximum lines (10) reached for the DEMO version of PPLC");
            err = TRUE;
            break;
        }
#endif

        do {

            if (genSysLabel && (pass == 1))
            {
                char sysLabel[MAX_LAB_NAME_SIZE+1];
                sprintf(sysLabel,"~LABEL~%d~",++sysLabelCount);
                genSysLabel = FALSE;
                if (doLabel(sysLabel,curLine,TRUE)) err = TRUE;
            }

            err = (compLine(pass,sBuf,curLine) || err);

            if (nextLine)
            {
                strcpy(sBuf,nextLine);
                addchar(sBuf,' ');
                nextLine = NULL;
            }
            else if (*nextLogStatement)
            {
                strcpy(sBuf,nextLogStatement);
                *nextLogStatement = '\0';
            }
            else
            {
                *sBuf = '\0';
            }

        } while (*sBuf != '\0');
    }

    if ((level == 1) && dispStat) print("\x08 ");

    if (genSysLabel && (pass == 1))
    {
        char sysLabel[MAX_LAB_NAME_SIZE+1];
        sprintf(sysLabel,"~LABEL~%d~",++sysLabelCount);
        err = (doLabel(sysLabel,curLine,TRUE) || err);
        genSysLabel = FALSE;
    }

    if ((level == 1) && !err && (availBytes() >= sizeof(int)))
    {
    uint oldsize = scriptSize;

        *((int far *) scriptPointer) = TOK_END;
        scriptSize += sizeof(int);
        //if (scriptSize < 0) err = TRUE;      /*scriptSize is now unsigned.*/
        if(scriptSize < oldsize) err = TRUE;
    }

    if (!startStruct.isEmpty()) err = TRUE;

    while (!startStruct.isEmpty())
        scriptErr(fs.handle,int(startStruct.pop()&0x0000FFFFL),SCR_ERR_NOEND,"");

    free(sBuf);

    --level;

    return err;
}

    /*--------------------------------------------------------------------*/

int pascal cSCRIPT::legalName(char * s)
{
    stripSpaces(s);
    if (!(isascii(*s) && isalpha(*s))) return FALSE;
    ++s;

    char * t = strchr(s,'(');
    if (t != NULL)
    {
        char * p = t;
        t = strdup(p);
        *p = '\0';
    }

    char * o = s;
    while (*s != '\0')
        if (!(isascii(*s) && isalnum(*s)) && (strchr("_$#@›œú",*s) == NULL))
        {
            if (t) free(t);
            return FALSE;
        }
        else
            ++s;

    if (strlen(--o) > MAX_LAB_NAME_SIZE)  o[MAX_LAB_NAME_SIZE] = '\0';
    if (t != NULL)
    {
        strcat(o,t);
        free(t);
    }

    if (isConst(o)) return FALSE;

    return TRUE;
}

    /*--------------------------------------------------------------------*/

char * pascal cSCRIPT::standardizeLine(char * l, int & q, int & p)
{
    // Save original line pointer
    char * s = l;
    char * sep = NULL;

    while ((l = strchr(l,'\t')) != NULL) *l = ' ';

    l = s;

    stripleft(s,' ');

    // If the first column is an asterisk, it's a comment!
    if (*l == '*')
    {
        *l = '\0';
        strcpy(commentBuf,l+1);
    }

    // While we haven't reached the end of the string
    while (*l != '\0')
    {
        if (*l == '\"')                 // If quote " toggle quote status
            q = !q;
        else if (!q)                    // Otherwise . . .
        {
            switch (*l)
            {
                case '\'':          // If ' or ; 0 terminate string
                case ';':
                    *l = '\0';
                    strcpy(commentBuf,l+1);
                    continue;

                case '[':           // If [ or { convert to (
                case '{':
                    *l = '(';
                    break;

                case ']':           // If ] or } convert to )
                case '}':
                    *l = ')';
                    break;

                case ':':           // If a colon is found in l (but not at the beginning of s)
                    if ((sep == NULL) && (l != s))
                        sep = l;
                    break;

                case ' ':
                    if (l[1] == ' ')
                    {
                        strcpy(l,l+1);
                        continue;
                    }
                    break;

                default:            // Otherwise convert to upper case
                    *l = toupper(*l);
                    break;
            }

            if (*l == '(')
                ++p;
            else if (*l == ')')
                --p;
        }

        ++l;
    }

    stripright(s,' ');     // Strip spaces from right
    addchar(s,' ');        // Make sure there is one extra space in case of one work keywords

    return sep;            // Return pointer to colon separator (or NULL)
}

    /*--------------------------------------------------------------------*/

void pascal cSCRIPT::stripSpaces(char * s)
{
    int qStat = FALSE;
    while (*s != '\0')
    {
        if (*s == '\"')
        {
            qStat = !qStat;
        }
        else if ((*s == ' ') && !qStat)
        {
            strcpy(s,s+1);
            continue;
        }
        ++s;
    }
}

    /*--------------------------------------------------------------------*/

int pascal cSCRIPT::argCount(char * s, int funcstyle)
{
    if (s == NULL) s = "";

    int qStat = FALSE;
    int pCount = 0;
    int cCount;

    if (funcstyle)
        cCount = ((*s == ')') ? 0 : 1);
    else if (strlen(s) > 0)
        cCount = 1;
    else
        cCount = 0;

    while ((*s != '\0') && (!funcstyle || (funcstyle && (pCount >= 0))))
    {
        if (*s == '\"')
            qStat = !qStat;
        else if (!qStat && (*s == '('))
            ++pCount;
        else if (!qStat && (*s == ')'))
            --pCount;
        else if (!qStat && (pCount < 1) && (*s == ','))
            ++cCount;
        ++s;
    }

    if (funcstyle && (pCount != -1))
        return -1;
    else if (funcstyle)
        *(--s) = '\0';

    return cCount;
}

    /*--------------------------------------------------------------------*/

char * pascal cSCRIPT::getArgument(char * s)
{
    if ((s != NULL) && (strlen(s) > 0)) nextToken = s;

    if (nextToken == NULL) return NULL;

    int qStat = FALSE;
    int pCount = 0;

    char *tmp = nextToken;
    while (*nextToken != '\0')
    {
        if (*nextToken == '\"')
        {
            qStat = !qStat;
        }
        else if (!qStat && (*nextToken == '('))
        {
            ++pCount;
        }
        else if (!qStat && (*nextToken == ')'))
        {
            --pCount;
        }
        else if (!qStat && (pCount < 1) && (*nextToken == ','))
        {
            *(nextToken++) = '\0';
            break;
        }
        nextToken++;
    }
    if (strlen(nextToken) == 0) nextToken = NULL;

    return tmp;
}



    /*--------------------------------------------------------------------*/

    /**********************************************************************/
    /*                                                                    */
    /* isCall -> cSCRIPT member function                                  */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Purpose -> To determine if the current variable is a function or   */
    /*            procedure call                                          */
    /*                                                                    */
    /*====================================================================*/
    /*                                                                    */
    /* Parameters:                                                        */
    /*                                                                    */
    /* obj   -> (cSCRIPT &) A reference to the script object being        */
    /*          compiled                                                  */
    /* inf   -> (sKEYWORD &) A reference to information about the keyword */
    /* args  -> (char *) Arguments after the keyword                      */
    /* line  -> (int) The line number being compiled                      */
    /* pass  -> (int) The current compiler pass                           */
    /*                                                                    */
    /*--------------------------------------------------------------------*/
    /*                                                                    */
    /* Returns:                                                           */
    /*                                                                    */
    /* (int) TRUE if an error occurs tokenizing; FALSE otherwise          */
    /*                                                                    */
    /**********************************************************************/


#pragma argsused
int pascal cSCRIPT::isCall(cSCRIPT & obj,char * s, int line,
          int pass, bool procOnly, bool funcOnly, int *err )
{
    int off=0,callYes = FALSE;
    int pf=FALSE,ff=FALSE;                      // procedure flag & function flag
    int ac = 0;
    char * tmp1= NULL, *arg=NULL, *tmp2=NULL;
    char tmps[MAX_LINE_SIZE+1], s1[MAX_LINE_SIZE+1],s2[MAX_LINE_SIZE+1];
    char  *t1, *t2;

    // see if it an implied let statement. This is done by checking if there
    // Are any quotes. If so they are extracted and the remaining string
    // is checked for an = sign. If there is an equal sign it is either
    // an implied let statement or a really screwed up procedure call.
    strcpy(s1,s);
    t1 = strchr(s1,'\"');
    if(!t1)
    {
      t1 = strchr(s1,'=');
        if(t1)
          return FALSE;
    }
    else
    {
        *t1 = '\x0';
        ++t1;
        t2 = strchr(t1,'\"');
        if(!t2){
          *err = TRUE;
          return FALSE;
        }
        else
        {
            *t2 = '\x0';
            ++t2;
        }
        sprintf(s2,"%s%s",s1,t2);
        t1 = strchr(s2,'=');
        if(t1)
          return FALSE;
    }

    strcpy(tmps,s);
    stripSpaces(s);


    if(!isalpha(*s))
      return FALSE;
    // find open paren
    tmp1 = strchr(s,'(');

    //if no open parem then err
    if(tmp1 == NULL)
      return FALSE;
    else
      // isolate function name
      *tmp1 = '\x0';


    //get a pointer to the preveously declared func or proc variable
    cVAR * t = obj.varLst.findName(s);

    // If it dosen't exist then err
    if((t->info.type != vtFUNCTION && (t->info.type  != vtPROCEDURE))){
//      return FALSE;
//      if(t == NULL){
//        scriptErr(obj.fs.handle,line,SCR_ERR_NODEC,s);
//        *err = TRUE;
//      }

      strcpy(s,tmps);
      return FALSE;
    }
    else
      // if it does then this is a function call
      callYes = TRUE;


    // set pf flag to true if it is a procedure
    // set ff flag to true if it is a function
    pf = (t->info.type  == vtPROCEDURE);
    ff = (t->info.type  == vtFUNCTION);

    if(ff && procOnly)
    {
        *tmp1 = '(';
        sprintf(compileStr,"%s %s","EVAL",s);
        *err = obj.compLine(pass,compileStr,line);
        return callYes;
    }
    // copy function name into compileStr
    strcpy(compileStr,s);

    // if it is a valid function or procedure call, build system label
    if(pf && procOnly || ff)
    {
      sprintf(compileStr,"~F~%s~",s);
      *tmp1 = '(';

      // if it's a procedure call, add PCALL to buffer
      if(pf)
        *err = obj.addTokenToBuffer(TOK_PCALL);

      if(*err)
        return callYes;

      // add code offset to vFUNCTION structure for reading in the runtime module
      if(pass == 1)
        off = 0;
      else
        off = obj.getLabel(compileStr,line,pass);
      t->data->values.vFUNCTION.offset = off;

      // write var id to token stream
      *tmp1 = '\x0';

      // Need to trick wrVIDSUB so it thinks it is not in a function when looking
      // up function name. Otherwise it will write return var ID to token stream
      // instead of function ID.
      int f = FALSE;
      if(inFunc)
      {
        inFunc = FALSE;
        f = TRUE;
      }
      *err = obj.wrVIDSUB(s,line,pass);
      if(f)
      {
        inFunc = TRUE;
        f = FALSE;
      }

      *tmp1 = '(';
      if(*err)
        return callYes;

      // make sure there are enough parameters passed in the call
      //if tmp1 == NULL then there are 0 args
      if(*++tmp1 == ')')
        ac = 0;
      else
        ac = argCount(tmp1);


      // if args passed dosen't match expected args then err
      if(ac != t->data->values.vFUNCTION.params){
        scriptErr(obj.fs.handle,line,SCR_ERR_PARANUM,t->name);
        *err = TRUE;
        return callYes ;  /// error
      }
      tmp2 = strrchr(tmp1,')');
      if(tmp2)
        *tmp2 = '\x0';


      // get first argument
      arg = getArgument(tmp1);

      // send parameter list to in2post to process them. and send then to token stream
      // t->data->values.vFUNCTION.params contains how many arguments there should be in a call
      for(int i = 0; i< ac;i++)
      {
         // Check for var passing mode
         // if it is a var parameter and it is not a single variable then err.
         if( (t->info.type  == vtPROCEDURE) &&
             (t->data->values.vPROCEDURE.passFlags & (1 << i) ) &&
             (!obj.getVariable(arg,line,FALSE)) )
             {
               scriptErr(obj.fs.handle,line,SCR_ERR_VARPASS,t->name);
               *err = TRUE;
               return callYes;
             }
         *err = obj.in2post(arg,line,pass);

         if(*err)
           return callYes;
         arg = getArgument(NULL);
      }
    }
    else
    {
      *err = TRUE;
      scriptErr(obj.fs.handle,line,SCR_ERR_BADCALL,"%s",s);
      return callYes;
    }
    // copy untouched original string back in.
    strcpy(s,tmps);
    return callYes ;

}
/******************************************************************************/

#endif

