/* OS/2 global variable storage — provides definitions for extern declarations */
/* GPL v3.0 — PWA 15.4 project by hexadecimal */

#include "project.h"
#pragma hdrstop
#include "pcboard.h"
#include "structs.h"

/* These are declared extern in pcboard.ext — provide actual storage */
statustype Status;
TRANSLATE *record_list = 0;

/* Country/locale globals */
char _Collate[256];
char _UpperCase[256];
countrytype _Country;
char *_Scrn_Buf = 0;
