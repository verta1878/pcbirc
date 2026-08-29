#include <stdio.h>
#include <string.h>
#include <conio.h>
#include "X:\H\PCBTOOLS.H"

main(void) {

  if (initdoor ("Get Graph Mode", 0, 0, NOCLS)==-1)
    return 0;

  printf ("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
  printf ("Get Graphics Mode v1.0 (use with PCBoard v15.1+)\n\n");
  printf ("Errorlevel is based on graphics mode: 0=CTTY, 1=ANSI, 2=GRAPH, 3=RIP\n");
  printf ("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");

  if (Control.GraphicsMode && Control.RipMode)
    ErrorLevel = 3;
  else if (Control.GraphicsMode && ! Control.RipMode)
    ErrorLevel = 2;
  else if (UseAnsi)
    ErrorLevel = 1;
  else
    ErrorLevel = 0;

  closedoor(FALSE);
}
