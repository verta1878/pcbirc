/*
 *  stbdsptn.cpp - streambuf::do_sputn out-of-line implementation
 *
 *  Missing source file for PCBoard UUCP link targets (UUIN, UUOUT, UUXFER).
 *  Clark never shipped this source. Reconstructed from BC 3.1 iostream.h
 *  class declaration and standard RTL behavior.
 *
 *  Virtual override: writes n chars through sputc() one at a time.
 *  This is the standard BC 3.1 RTL implementation extracted so it
 *  links as a standalone OBJ instead of pulling the whole iostream lib.
 *
 *  Compile: BCC +<target>.CFG -c -n<objdir> stbdsptn.cpp
 *  Flags:   -ml -K -P -3
 *  Output:  stbdsptn.obj → referenced from LNK files as ..\bc31\stbdsptn.obj
 */

#include <iostream.h>

int _Cdecl streambuf::do_sputn(const char _FAR * s, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        if (sputc(s[i]) == EOF) return i;
    }
    return n;
}
