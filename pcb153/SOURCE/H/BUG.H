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


/******************************************************************************
  There is a BUG in Borland C++ 3.1 which ONLY occurs when the -3 switch is
  used to compile in 386 mode.  This bug causes problems with any function
  that returns a long integer value when comparing the value against zero
  to determine if it is positive or negative.  Example:

  long gettimer(int Timer) {
    return(0x0001FFFFL);
  }

  void main(void) {
    puts(gettimer(1) <= 0 ? "negative" : "positive");
  }

  The test in main() will show "negative" even though the value returned is
  positive.  This does NOT occur without the -3 switch.  This can be worked
  around by assigning the value to a variable first and then performing the
  comparison.  For that reason, the following macro has been created which
  is used, instead of gettimer(), to test to see if the timer has expired.
*****************************************************************************/

#ifdef __OS2__
/*#define timerexpired(n) (gettimer(n) <= 0) */
#else
  #ifdef CPU386
    extern long BC386BUG;
    #define timerexpired(n) ((BC386BUG = gettimer(n)) <= 0)
  #else
    #define timerexpired(n) (gettimer(n) <= 0)
  #endif
#endif
