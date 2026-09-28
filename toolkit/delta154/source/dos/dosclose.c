/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#ifdef __OS2__
#define INCL_DOSFILEMGR
#include <os2.h>
#elif defined(__WATCOMC__)
#include <i86.h>
#else
#include "model.h"
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static void (LIBENTRY *dosclosecallback)(char *FileName,int Handle,int ExtendedError);

void LIBENTRY dosclose(int handle) {
  #ifdef __OS2__
    APIRET ExtendedError;
  #endif

  if (handle > 0 && handle <= MAXHANDLES && OpenFileNames[handle][0] != 0) {
    #ifdef __OS2__
      ExtendedError = DosClose(handle);
    #elif defined(__WATCOMC__)
      {
        union REGS r;
        r.h.ah = 0x3E;
        r.w.bx = handle;
        int386(0x21, &r, &r);
        if (r.w.cflag)
          getextendederror();
      }
    #else
      ExtendedError = 0;
      asm mov ah,3Eh
      asm mov bx,handle
      int21();
      asm jnc end
      getextendederror();
      end:;
   #endif

    if (dosclosecallback != NULL)
      dosclosecallback(OpenFileNames[handle],handle,ExtendedError);

    OpenFileNames[handle][0] = 0;
  }
}


void LIBENTRY closedosopenfiles(void) {
  int X;

  for (X = MAXHANDLES; X > 0; X--)
    dosclose(X);
}


void LIBENTRY setdosclosecallback(void (LIBENTRY *cb)(char *FileName,int Handle,int ExtendedError)) {
  dosclosecallback = cb;
}
