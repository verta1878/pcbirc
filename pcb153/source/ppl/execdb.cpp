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
/*                                EXECDB.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*  Data and functions for the cSCRIPT class                                  */
/*                   related to DBASE statements and functions                */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*     Written by                                                             */
/*                                Stan Paulsen                                */
/*                                                                                                                                                        */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Copyright (C) 1994 - Clark Development Company, Inc.                    */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/

// include files

#if defined(DBASE) && defined(___EXEC___)
#ifdef __BORLANDC__
  #include      <dir.h>
  #include      <alloc.h>
#else
  #include      <direct.h>
  #include      <malloc.h>
#endif

#include        <dos.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        "lrand.hpp"
#include        "pcbmisc.hpp"
#include        "scrmisc.hpp"
#include        <assert.h>
#include        "exec.hpp"
#include        "dbase.hpp"

#include        "bug.h"
#ifdef DEBUGSCR
#include        <memcheck.h>
#endif


/******************************************************************************/
// variables global to this module



/******************************************************************************/
// external variables

extern cVARVAL * p1, * p2, * p3, * p4, * rp;
extern bool      v1, v2, v3, v4;
extern bool      f1, f2, f3, f4;

/******************************************************************************/
/*                                                                            */
/*  Function: EXECDBS cSCRIPT member function.                                */
/*                                                                            */
/*  This is code that was ripped out of the EXECUTE function                  */
/*  in SCREXEC module EXECUTE function. This was to reduce the size of SCREXEC*/
/*  This function is called from the main SWITCH statement in EXECUTE.        */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#pragma argsused
void LIBENTRY cSCRIPT::execdbs(int token,int & err)
{

char    * tmp1 = NULL;
cVAR    * a    = NULL;
cVARVAL * r    = NULL;
cVARVAL   s(vtSTRING);
long l1,l2;
int i1;

    switch(token)
    {
            case TOK_DADD:

                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].add();

                break;

            case TOK_DSETALIAS:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                dbfArr[(int)l1].dbfAlias(str1);

                break;

            case TOK_DAPPEND:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].append();

                break;

            case TOK_DBLANK:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].blank();

                break;

            case TOK_DBOTTOM:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].bottom();

                break;

            case TOK_DCLOSE:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].dbfClose();

                break;

            case TOK_DCLOSEALL:

                for(i1=0;i1<MAX_DBF_CHANNELS;i1++)
                     dbfArr[i1].dbfClose();

                break;

              case TOK_DCREATE:


                EVAL(tmpInteger,r,nullErrBreak);
                l1 = tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                                EVAL(tmpInteger,r,nullErrBreak);
                l2 = tmpInteger.values.vINTEGER;

                a = getVarPtr();

                for(i1=strlen(str1);i1!=0;i1--)      // strip any file extensions.
                {
                    if(*(str1+i1) == '\\')
                      break;
                    if(*(str1+i1) == '.')
                    {
                        *(str1+i1) = '\x0';
                        break;
                    }
                }

                dbfArr[(int)l1].dbfCreate(str1,(int)l2,*a);

                break;

            case TOK_DDELETE:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = tmpInteger.values.vINTEGER;


                dbfArr[(int)l1].kill();
                break;

            case TOK_DFBLANK:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                dbfArr[(int)l1].blank(str1);

                break;
            case TOK_DFCOPY:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                strcpy(str1,tmpString.values.vSTRING);
                l1 = (long) tmpInteger.values.vINTEGER;

                                EVAL(tmpInteger,r,nullErrBreak);
                l2 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str2,tmpString.values.vSTRING);

                dbfArr[(int)l1].copy(str1,dbfArr[(int)l2],str2);

                break;

            case TOK_DGET:


                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                r = getVarSubPtr();
                switch(r->type)
                {

                    case vtSTRING:
                        if(r->values.vSTRING != NULL)
                          strcpy(str2,r->values.vSTRING);
                        dbfArr[(int)l1].get(str1,str2);
                        *r = str2;
                        break;
                    case vtBIGSTR:

                        if(r->values.vBIGSTR.ptr != NULL)
                          strcpy(str2,r->values.vBIGSTR.ptr);
                        dbfArr[(int)l1].get(str1,str2);
                        *r = str2;
                        break;

                    case vtBYTE:
                        dbfArr[(int)l1].get(str1,(int &) r->values.vBYTE);
                        break;

                    case vtINTEGER:
                        dbfArr[(int)l1].get(str1,r->values.vINTEGER);
                        break;

                    case vtDREAL:
                        dbfArr[(int)l1].get(str1,r->values.vDREAL);
                        break;

                    case vtDDATE:

                        if(r->values.vSTRING != NULL)
                          strcpy(str2,r->values.vSTRING);
                        dbfArr[(int)l1].get(str1,str2);
                        *r = str2;
                        break;

                    default:
                        cVARVAL tmp(vtDREAL);
                        tmp  = *r;
                        dbfArr[(int)l1].get(str1,tmp.values.vDREAL);
                        *r = tmp;
                        break;
                }

                break;

            case TOK_DGO:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpInteger,r,nullErrBreak);
                l2 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].go(l2);
                break;

            case TOK_DLOCK:         //             TOK_DLOCKF same token


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].lock();

                break;

              case TOK_DLOCKG:


                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                a = getVarPtr();

                                EVAL(tmpInteger,r,nullErrBreak);
                l2 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].lock(*a,(int)l2);
                break;

            case TOK_DLOCKR:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpInteger,r,nullErrBreak);
                l2 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].lock(l2);

                break;

            case TOK_DNCLOSE:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                dbfArr[(int)l1].ndxClose(str1);

                break;

            case TOK_DNCLOSEALL:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].ndxCloseAll();

                break;

            case TOK_DNCREATE:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str2,tmpString.values.vSTRING);

                for(i1=strlen(str1);i1!=0;i1--)      // strip any file extensions.
                {
                    if(*(str1+i1) == '\\')
                      break;
                    if(*(str1+i1) == '.')
                    {
                        *(str1+i1) = '\x0';
                        break;
                    }
                }

                dbfArr[(int)l1].ndxCreate(str1,str2);

                break;

            case TOK_DNEW:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;


                dbfArr[(int)l1].start();

                break;

            case TOK_DNOPEN:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                tmp1 = strchr(str1,'.');
                if(tmp1 != NULL)
                   *tmp1 = '\x0';

                dbfArr[(int)l1].ndxOpen(str1);

                break;

            case TOK_DOPEN:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);

                strcpy(str1,tmpString.values.vSTRING);

                EVAL(tmpInteger,r,nullErrBreak);
                l2 = (long) tmpInteger.values.vINTEGER;

                tmp1 = strchr(str1,'.');
                if(tmp1 != NULL)
                  *tmp1 = '\x0';

                dbfArr[(int)l1].dbfOpen(str1,(int)l2);

                break;

            case TOK_DPACK:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].pack();

                break;

            case TOK_DPUT:

                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);
                strcpy(str1,tmpString.values.vSTRING);

                r = evalArg();
                falseErrBreak(r);

                switch(r->type)
                {
                    case vtSTRING:
                       dbfArr[(int)l1].put(str1,r->values.vSTRING);
                       break;

                    case vtBIGSTR:
                       dbfArr[(int)l1].put(str1,r->values.vBIGSTR.ptr);
                       break;
                    case vtBYTE:
                       dbfArr[(int)l1].put(str1,r->values.vBYTE);
                       break;

                    case vtINTEGER:
                       dbfArr[(int)l1].put(str1,r->values.vINTEGER);
                       break;

                    case vtDREAL:
                       dbfArr[(int)l1].put(str1,r->values.vDREAL);
                       break;

                    case vtDATE:
                       {
                       cVARVAL  dpdd (vtDDATE);
                       cVARVAL  dpstr(vtSTRING);
                       dpdd = *r;
                       dpstr = dpdd;
                       dbfArr[(int)l1].put(str1,dpstr.values.vSTRING);
                       }
                       break;

                    case vtDDATE:
                       {
                       cVARVAL  dps (vtSTRING);
                       dps = *r;
                       dbfArr[(int)l1].put(str1,dps.values.vSTRING);
                       }
                       break;

                    default:
                        cVARVAL tmp(vtSTRING);
                        tmp  = *r;
                        dbfArr[(int)l1].put(str1,tmp.values.vSTRING);
                     //   *r = tmp;
                        break;
                }
                break;

            case TOK_DRECALL:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].resurrect();
                break;

            case TOK_DSEEK:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                r = evalArg();
                switch(r->type)
                {
                    case vtDREAL:
                        dbfArr[(int)l1].seek(r->values.vDREAL);
                        break;

                    case vtSTRING:
                        dbfArr[(int)l1].seek(r->values.vSTRING);
                        break;

                    case vtBIGSTR:
                        dbfArr[(int)l1].seek(r->values.vBIGSTR.ptr);
                        break;

                    default:
                        cVARVAL tmp(vtDREAL);
                        tmp = *r;
                        dbfArr[(int)l1].seek(tmp.values.vDREAL);
                        break;
                }

                break;

            case TOK_DSKIP:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpInteger,r,nullErrBreak);
                l2 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].skip(l2);

                break;

            case TOK_DTAG:


                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                EVAL(tmpString,r,nullErrBreak);

                dbfArr[(int)l1].tag(tmpString.values.vSTRING);

                break;

            case TOK_DTOP:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].top();

                break;

            case TOK_DUNLOCK:

                                EVAL(tmpInteger,r,nullErrBreak);
                l1 = (long) tmpInteger.values.vINTEGER;

                dbfArr[(int)l1].unlock();

                break;
        } // end switch
} // end func




// execdbf  Execute DB functions
/******************************************************************************/
/*                                                                            */
/*  Function: EXECDBF cSCRIPT member function.                                */
/*                                                                            */
/*  This is code that was ripped out of the EVALPOSTFIX function              */
/*  in EVALP   module. This was to reduce the size of SCREXEC                 */
/*  This function is called from the main SWITCH statement in EVALPOSTFIX.    */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
#pragma argsused
void LIBENTRY cSCRIPT::execdbf(cVARVAL * p1,cVARVAL * p2,cVARVAL * p3,cVARVAL * p4,cVARVAL * & rp,int & err)
{

int i1 = 0;
long l1,l2;
cVARVAL  s (vtSTRING);
cVARVAL  st(vtSTRING);

    switch( *((sint _FAR_ *) scriptPointer))  // DWT:  changed int to sint
    {


            case TOK_OP_DERRMSG:

                falseErrBreak(v1);
                l1 = *p1;
                rp = new cVARVAL(vtSTRING,dbfArr[0].errText((int)l1));
                f1 = TRUE;
                break;

            case TOK_OP_DCHKSTAT:
                falseErrBreak(v1);
                l1 = *p1;
                rp = new cVARVAL(vtINTEGER,dbfArr[(int)l1].returnStatus());
                f1 = TRUE;
                break;

            case TOK_OP_DSELECT:
                falseErrBreak(v1);
                for(i1=0;i1<MAX_DBF_CHANNELS;i1++)
                {
                    s = *p1;
                    if((dbfArr[i1].inUse) && stricmp(dbfArr[i1].dbfGetAlias(),s.values.vSTRING)==0)
                      break;
                }
                rp = new cVARVAL(vtINTEGER,i1);
                f1 = TRUE;
                break;

            case TOK_OP_DADD:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].add();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;

                break;

            case TOK_OP_DSETALIAS:

                falseErrBreak(v2)
                s    = *p1;
                l1   = *p2;
                dbfArr[(int)l1].dbfAlias(s.values.vSTRING);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DAPPEND:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].append();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DBLANK:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].blank();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DBOTTOM:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].bottom();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DCLOSE:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].dbfClose();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DCLOSEALL:

                falseErrBreak(v1);
                l1 = *p1;
                for(i1=0;i1<MAX_DBF_CHANNELS;i1++)
                     dbfArr[i1].dbfClose();
                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DDELETE:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)p1->values.vINTEGER].kill();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DFBLANK:

                falseErrBreak(v2);
                s    = *p1;
                l1   = *p2;
                dbfArr[(int)l1].blank(s.values.vSTRING);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DFCOPY:
                falseErrBreak(v4)
                s    = *p1;
                st   = *p3;
                l1   = *p2;
                l2   = *p4;

                dbfArr[(int)l2].copy(st,dbfArr[(int)l1],s);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l2].error()));
                f1 = f2 = f3 = f4 = TRUE;
                break;

            case TOK_OP_DGET:

                falseErrBreak(v2)
                l1   = *p2;
                s    = *p1;
                dbfArr[(int)l1].get(s,str2);

                rp = new cVARVAL(vtSTRING,str2);
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DGO:

                falseErrBreak(v2);
                l1 = *p2;
                l2 = *p1;
                dbfArr[(int)l1].go(l2);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DLOCK:         //             TOK_DLOCKF same token
            case TOK_OP_DLOCKF:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].lock();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DLOCKR:

                falseErrBreak(v2);
                l1 = *p2;
                l2 = *p1;
                dbfArr[(int)l1].lock(l2);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DNCLOSE:

                falseErrBreak(v2);
                s    = *p1;
                l1   = *p2;
                dbfArr[(int)l1].ndxClose(s.values.vSTRING);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DNCLOSEALL:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].ndxCloseAll();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DNEW:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].start();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DNOPEN:

                falseErrBreak(v2);
                s    = *p1;
                l1   = *p2;
                {
                  char * tmp1 = strchr(s.values.vSTRING,'.');
                  if(tmp1 != NULL)
                     *tmp1 = '\x0';
                }

                dbfArr[(int)l1].ndxOpen(str1);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DOPEN:

                falseErrBreak(v3);
                s = *p2;
                l2   = *p1;
                l1   = *p3;
                {
                  char * tmp1 = strchr(s.values.vSTRING,'.');
                  if(tmp1 != NULL)
                    *tmp1 = '\x0';
                }
                dbfArr[(int)l1].dbfOpen(s.values.vSTRING,(int)l2);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = f3 = TRUE;
                break;

            case TOK_OP_DPACK:

                falseErrBreak(v1);
                l1 = *p1;
                dbfArr[(int)l1].pack();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DPUT:

                falseErrBreak(v3);
                l1   = *p3;
                s    = *p2;
                st   = *p1;
                switch(p1->type)
                {
                    case vtSTRING:
                       dbfArr[(int)l1].put(s.values.vSTRING,p1->values.vSTRING);
                       break;

                    case vtBIGSTR:
                       dbfArr[(int)l1].put(s.values.vSTRING,p1->values.vBIGSTR.ptr);
                       break;
                    case vtBYTE:
                       dbfArr[(int)l1].put(s.values.vSTRING,p1->values.vBYTE);
                       break;

                    case vtINTEGER:
                       dbfArr[(int)l1].put(s.values.vSTRING,p1->values.vINTEGER);
                       break;

                    case vtDREAL:
                       dbfArr[(int)l1].put(s.values.vSTRING,p1->values.vDREAL);
                       break;

                    case vtDATE:
                       {
                       cVARVAL  dpdd (vtDDATE);
                       cVARVAL  dpstr(vtSTRING);
                       dpdd = *p1;
                       dpstr = dpdd;
                       dbfArr[(int)l1].put(s.values.vSTRING,dpstr.values.vSTRING);
                       }
                       break;

                    case vtDDATE:
                       dbfArr[(int)l1].put(s.values.vSTRING,st.values.vSTRING);
                       break;

                    default:
                       cVARVAL tmp(vtDREAL);
                       tmp = *p1;
                       dbfArr[(int)l1].put(s.values.vSTRING,tmp.values.vDREAL);
                       break;

                }

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = f3 = TRUE;
                break;

            case TOK_OP_DRECALL:

                falseErrBreak(v1);
                dbfArr[(int)p1->values.vINTEGER].resurrect();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)p1->values.vINTEGER].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DSEEK:

                falseErrBreak(v2);
                switch(p1->type)
                {
                    case vtDREAL:
                        dbfArr[(int)p2->values.vINTEGER].seek(p1->values.vDREAL);
                        break;

                    case vtSTRING:
                        dbfArr[(int)p2->values.vINTEGER].seek(p1->values.vSTRING);
                        break;

                    case vtBIGSTR:
                        dbfArr[(int)p2->values.vINTEGER].seek(p1->values.vBIGSTR.ptr);
                        break;

                    default:
                        cVARVAL tmp(vtDREAL);
                        tmp = *p1;
                        dbfArr[(int)p2->values.vINTEGER].seek(tmp.values.vDREAL);
                        break;
                }
                rp = new cVARVAL(vtBOOLEAN,(int) (dbfArr[(int)p2->values.vINTEGER].returnStatus() == 0));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DSKIP:

                falseErrBreak(v2)
                l1 = *p2;
                l2 = *p1;
                dbfArr[(int)l1].skip(l2);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)l1].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DTAG:

                falseErrBreak(v2)
                dbfArr[(int)p2->values.vINTEGER].tag(p1->values.vSTRING);

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)p2->values.vINTEGER].error()));
                f1 = f2 = TRUE;
                break;

            case TOK_OP_DTOP:

                falseErrBreak(v1);
                dbfArr[(int)p1->values.vINTEGER].top();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)p1->values.vINTEGER].error()));
                f1 = TRUE;
                break;

            case TOK_OP_DUNLOCK:

                falseErrBreak(v1);
                dbfArr[(int)p1->values.vINTEGER].unlock();

                rp = new cVARVAL(vtBOOLEAN,(int) (!dbfArr[(int)p1->values.vINTEGER].error()));
                f1 = TRUE;
                break;
// end of duplicates

                case TOK_OP_DNEXT:
                    {
                    rp = NULL;
                    for(int loop =0;loop<MAX_DBF_CHANNELS;loop++)
                    {
                        if(dbfArr[loop].inUse == FALSE)
                        {
                            rp = new cVARVAL(vtINTEGER,loop);
                            break;
                        }
                    }

                    if(rp == NULL)
                      rp = new cVARVAL(vtINTEGER,-1);
                    }
                    break;

                case TOK_OP_DGETALIAS:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtSTRING,dbfArr[(int)p1->values.vINTEGER].dbfAlias(NULL));

                    f1 = TRUE;
                    break;

                case TOK_OP_DBOF:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtBOOLEAN,dbfArr[(int)p1->values.vINTEGER].bof());

                    f1 = TRUE;
                    break;

                case TOK_OP_DCHANGED:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtBOOLEAN,dbfArr[(int)p1->values.vINTEGER].changed());

                    f1 = TRUE;
                    break;

                case TOK_OP_DDECIMALS:

                    falseErrBreak(v2)
                    rp = new cVARVAL(vtINTEGER,dbfArr[(int)p2->values.vINTEGER].decimals(p1->values.vSTRING));

                    f1 = f2 = TRUE;
                    break;

                case TOK_OP_DDELETED:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtBOOLEAN,dbfArr[(int)p1->values.vINTEGER].killed());

                    f1 = TRUE;
                    break;

                case TOK_OP_DEOF:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtBOOLEAN,dbfArr[(int)p1->values.vINTEGER].eof());

                    f1 = TRUE;
                    break;

                case TOK_OP_DERR:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtBOOLEAN,(int) (dbfArr[(int)p1->values.vINTEGER].error()));

                    f1 = TRUE;
                    break;

                case TOK_OP_DFIELDS:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtINTEGER,dbfArr[(int)p1->values.vINTEGER].fields());

                    f1 = TRUE;
                    break;

                case TOK_OP_DLENGTH:

                    falseErrBreak(v2)
                    rp = new cVARVAL(vtINTEGER,dbfArr[(int)p2->values.vINTEGER].length(p1->values.vSTRING));

                    f1 = f2 = TRUE;
                    break;

                case TOK_OP_DNAME:

                    falseErrBreak(v2)
                    rp = new cVARVAL(vtSTRING,dbfArr[(int)p2->values.vINTEGER].name((int)p1->values.vINTEGER));

                    f1 = f2 = TRUE;
                    break;

                case TOK_OP_DRECCOUNT:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtINTEGER,dbfArr[(int)p1->values.vINTEGER].reccount());

                    f1 = TRUE;
                    break;

                case TOK_OP_DRECNO:

                    falseErrBreak(v1)
                    rp = new cVARVAL(vtINTEGER,dbfArr[(int)p1->values.vINTEGER].recno());

                    f1 = TRUE;
                    break;

                case TOK_OP_DTYPE:
                    char chr[2];
                    falseErrBreak(v2)
                    char ch =dbfArr[(int)p2->values.vINTEGER].type(p1->values.vSTRING);
                    chr[0] = ch;
                    chr[1] = '\x0';
                    rp = new cVARVAL(vtSTRING,chr);

                    f1 = f2 = TRUE;
                    break;
     } // end switch
} // end func
#endif // if defined(DBASE) && defined(___EXEC___)
