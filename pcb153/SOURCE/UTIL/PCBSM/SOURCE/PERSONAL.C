/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* PERSONAL.C — 15.4 Personal PSA editor for PCBSM.                          */
/* Written by: hexadecimal, v0.020, corrected v0.032.                        */
/*                                                                           */
/* Edits Gender / Birthdate / Email / Web fields of the Personal PSA record. */
/* Menu-entry registered from INIT.C.  Guards on PersonalSupport — if the    */
/* PSA isn't installed, message says so and returns.                         */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <pcbfiles.h>
#include <pcbfiles.ext>
#include <help.h>


static void near pascal initpersonalfields(FldType *P) {
  addquest(P, 0, vCHAR, CHGPSN+0, ALLCHAR, 3, 10,  1,
           "Gender         ",
           &Personal.Gender,    CLEAR, NULL);
  addquest(P, 1, vSTR,  CHGPSN+1, ALLNUM,  3, 12,  6,
           "Birthdate      ",
           &Personal.Birthdate, CLEAR, NULL);
  addquest(P, 2, vSTR,  CHGPSN+2, ALLCHAR, 3, 14, 60,
           "Email Address  ",
           &Personal.Email,     CLEAR, NULL);
  addquest(P, 3, vSTR,  CHGPSN+3, ALLCHAR, 3, 16, 60,
           "Web Address    ",
           &Personal.Web,       CLEAR, NULL);
}


void pascal adjustpersonal(void) {
  FldType Fields[4];

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1, "Edit Personal Info (Gender/Email/Web)");

  if (!PersonalSupport) {
    fastprintmove(3, 12,
                  "Personal Info PSA is not installed on this system.",
                  Colors[DISPLAY]);
    fastprintmove(3, 14,
                  "Install it first via menu 8 (Add PSA Support).",
                  Colors[DISPLAY]);
    fastprintmove(19, 22, " Press any key to continue ", Colors[DESC]);
    { char Ch; Ch = inkey(&Ch, CLOCK); }
    return;
  }

  initpersonalfields(Fields);

  fastprintmove(3, 5, "Edit the Personal PSA record for the current user.",
                Colors[DISPLAY]);
  fastprintmove(3, 7, "Gender is stored as space (unset), 'M', 'F' or other.",
                Colors[DISPLAY]);

  readscrn(Fields, 3, 0, "", "", 1, CLEAR);
  freescrn(Fields, 4);
}
