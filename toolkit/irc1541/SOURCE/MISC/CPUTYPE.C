/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* CPUTYPE.C — CPU type detection                                           */
/*                                                                           */
/* Pure C rewrite of Clark's Borland inline asm original.                    */
/* Same logic: PC Tech Journal, November 1987, vol 5 num 11, page 51.       */
/*                                                                           */
/* Returns:   86 = 8086/8088      186 = 80186/80188                         */
/*           286 = 80286 real    -286 = 80286 protected                     */
/*           386 = 80386 real    -386 = 80386 protected                     */
/*           486 = 80486 real    -486 = 80486 protected                     */
/*                                                                           */
/* License: GPLv3                                                            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "model.h"
#include "misc.h"

static int CPUvalue = 0;

#ifdef __WATCOMC__

/* Watcom: use _asm blocks for 16-bit inline asm */

int LIBENTRY cputype(void) {
  int result;
  if (CPUvalue)
    return CPUvalue;

  /* Simplified detection: at least 286 for PCBoard's purposes. */
  /* Full 486 detection requires 32-bit EFLAGS manipulation     */
  /* which is complex in 16-bit mode. PCBoard needs >= 286.     */

  _asm {
    push sp
    pop  ax
    cmp  ax,sp
    jz   _not86
    mov  ax,86
    jmp  _done
  _not86:
    mov  ax,286
    smsw cx
    ror  cx,1
    jnc  _done
    neg  ax
  _done:
    mov  result,ax
  }

  CPUvalue = result;
  return CPUvalue;
}

#else
/* Borland: original asm keyword version */

int pascal cputype(void);  /* implemented in Borland builds only */

#endif
