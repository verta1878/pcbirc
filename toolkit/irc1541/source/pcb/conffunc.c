/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* CONFFUNC.C — Conference flag bit pack/unpack                              */
/* Clark Development Company, Inc. (C) 1996. All Rights Reserved.            */
/* Watcom C conversion by pcbrevival (GPL v3.0 for our additions)            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>
#ifdef DEBUG
#include <memcheck.h>
#endif

/*============================================================================
    Create a 40-character registration string from 5 packed bytes
    ShortStr (5 bytes, 40 bits) -> LongStr (40 chars: 'X' or ' ')
 ============================================================================*/

void pascal getconf(char *LongStr, char *ShortStr) {
#if defined(__OS2__) || defined(__WATCOMC__)
  int i;
  LongStr[40] = 0;
  for (i = 0; i < 40; i++) {
    int bytepos = i / 8;
    int bitpos  = i % 8;
    LongStr[i] = (ShortStr[bytepos] & (1 << bitpos)) ? 'X' : ' ';
  }
#else
  asm     Push  Ds
  asm     Push  Es
  asm     Lds   Di,ShortStr
  asm     Les   Si,LongStr
  asm     Add   Si,40
  asm     Mov   byte ptr Es:[Si],0
  asm     Dec   Si
  asm     Mov   Cx,40
gcLoop:
  asm     Mov   Bx,Cx
  asm     Push  Cx
  asm     Dec   Bx
  asm     Mov   Cl,Bl
  asm     And   Cl,7
  asm     Shr   Bx,1
  asm     Shr   Bx,1
  asm     Shr   Bx,1
  asm     Mov   Al,[Di+Bx]
  asm     Mov   Dl,1
  asm     Shl   Dl,Cl
  asm     Pop   Cx
  asm     And   Al,Dl
  asm     Jz    gcJ1
  asm     Mov   byte ptr Es:[Si],'X'
  asm     Jmp   short gcJ2
gcJ1:
  asm     Mov   byte ptr Es:[Si],' '
gcJ2:
  asm     Dec   Si
  asm     Loop  gcLoop
  asm     Pop   Es
  asm     Pop   Ds
#endif
}

/*============================================================================
    Put a 40-character registration string into 5 packed bytes
    LongStr (40 chars) -> ShortStr (5 bytes, 40 bits)
 ============================================================================*/

void pascal putconf(char *LongStr, char *ShortStr) {
#if defined(__OS2__) || defined(__WATCOMC__)
  int i;
  for (i = 0; i < 40; i++) {
    int bytepos = i / 8;
    int bitpos  = i % 8;
    if (LongStr[i] == 'X')
      ShortStr[bytepos] |= (unsigned char)(1 << bitpos);
    else
      ShortStr[bytepos] &= (unsigned char)~(1 << bitpos);
  }
#else
  asm     Push  Ds
  asm     Push  Es
  asm     Lds   Di,ShortStr
  asm     Les   Si,LongStr
  asm     Add   Si,39
  asm     Mov   Cx,40
pcLoop:
  asm     Mov   Bx,Cx
  asm     Push  Cx
  asm     Dec   Bx
  asm     Mov   Cl,Bl
  asm     And   Cl,7
  asm     Shr   Bx,1
  asm     Shr   Bx,1
  asm     Shr   Bx,1
  asm     Mov   Al,[Di+Bx]
  asm     Mov   Dl,1
  asm     Shl   Dl,Cl
  asm     Pop   Cx
  asm     Cmp   byte ptr Es:[Si],'X'
  asm     Je    pcJ1
  asm     Not   Dl
  asm     And   Al,Dl
  asm     Jmp   short pcJ2
pcJ1:
  asm     Or    Al,Dl
pcJ2:
  asm     Mov   [Di+Bx],Al
  asm     Dec   Si
  asm     Loop  pcLoop
  asm     Pop   Es
  asm     Pop   Ds
#endif
}
