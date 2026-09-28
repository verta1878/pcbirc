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
/*																			  */
/*								   SCOMP.CPP								  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*			  PCBoard Programming Language - Compiler Main Module.			  */
/*																			  */
/*============================================================================*/
/*																			  */
/*								   Written by								  */
/*							   Scott Dale Robison							  */
/*																			  */
/*----------------------------------------------------------------------------*/
/*																			  */
/*			  Copyright (C) 1994 - Clark Development Company, Inc.			  */
/*																			  */
/******************************************************************************/

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#include	<dos.h>

#include	<scrmisc.hpp>

#include	<h2name.h>

#ifdef DEBUGSCR
#include	<memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

/******************************************************************************/

// Types

/******************************************************************************/

// Constants

/******************************************************************************/

// Variables

extern unsigned _stklen = 16384U;

void interrupt (* orig0x24) (...);

extern char warnFlag;
extern char autoUVar;
extern char disArrSubChk;
extern char dispStat;

char name	  [ 256+1 ];

/******************************************************************************/

// Function Prototypes

extern "C" {

	void interrupt CEH			   (...);


}
	void pascal    getcountryspecs (int,int);

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

void pascal procArgs(void)
{
	for (int i = 1; i < _argc; ++i)
	{
		char * arg = _argv[i];
		if ((*arg == '/') || (*arg == '-'))
		{
			++arg;
			if		(memicmp(arg,"DISARR",6) == 0)
			{
				disArrSubChk = TRUE;
			}
			else if (memicmp(arg,"NOUVAR",6) == 0)
			{
				autoUVar	 = FALSE;
			}
			else if (memicmp(arg,"NODISP",6) == 0)
			{
				dispStat = FALSE;
			}
		}
		else
		{
			strupr(strcpy(name,arg));
			char * p = strrchr(name,'\\');
			p = (p ? ++p : name);
			if (strchr(p,'.') == NULL) strcat(p,".PPS");
		}
	}
}

	/*--------------------------------------------------------------------*/

int main(int argc)
{
	int  exitCode = 0;

#ifdef DEBUGSCR
	mc_startcheck(NULL);
#endif

	getcountryspecs(1,0);

	orig0x24 = getvect(0x24);
	setvect(0x24,CEH);

	newline();
	print("PPLC ");
#ifdef PCB_DEMO
	print("DEMO ");
#endif
	println("Version 3.30 - PCBoard Programming Language Compiler");
	print("Copyright (C) 1993-95, Clark Development Company, Inc.");

	if (argc < 2)
	{
		newline();
		newline();
		println("USAGE: PPLC [OPTIONS] SRCNAME[.EXT]");
		newline();
		println(" NOTE: .EXT defaults to .PPS if not specified");
		newline();
		println("Options:");
		newline();
		println("/DISARR - Disable Array Dimension Checking");
		println("/NODISP - Disable Status Display During Compile");
		println("/NOUVAR - Disable Automatic User Variable Generation");
		exitCode = 1;
		goto EXIT;
	}

	procArgs();

	if (strcmp(h2name(_psp,1),"CON        ") != 0) dispStat = FALSE;

	if (fileexist(name) & 0x18)
	{
		newline();
		newline();
		print("Error: ");
		print(name);
		println(" not found on disk, aborting...");
		exitCode = 2;
		goto EXIT;
	}

	if (doScript(name,NULL,0) == 0)
	{
		newline();
		newline();
		print("Source compilation complete...");
	}
	else
	{
		exitCode = 3;
	}

	newline();

EXIT:

	setvect(0x24,orig0x24);

#ifdef DEBUGSCR
	mc_endcheck();
#endif

	return ((exitCode) ? (exitCode+1) : (warnFlag));
}

/******************************************************************************/

