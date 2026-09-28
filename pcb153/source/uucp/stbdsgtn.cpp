/*
 *  stbdsgtn.cpp - streambuf::do_sgetn out-of-line implementation
 *
 *  Missing source file for PCBoard UUCP link targets (UUIN, UUOUT, UUXFER).
 *  Clark never shipped this source. Reconstructed from BC 3.1 iostream.h
 *  class declaration and standard RTL behavior.
 *
 *  Virtual override: reads n chars through sbumpc() one at a time.
 *  This is the standard BC 3.1 RTL implementation extracted so it
 *  links as a standalone OBJ instead of pulling the whole iostream lib.
 *
 *  Compile: BCC +<target>.CFG -c -n<objdir> stbdsgtn.cpp
 *  Flags:   -ml -K -P -3
 *  Output:  stbdsgtn.obj → referenced from LNK files as ..\bc31\stbdsgtn.obj
 */

#include <iostream.h>

int _Cdecl streambuf::do_sgetn(char _FAR * s, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        int c = sbumpc();
        if (c == EOF) return i;
        s[i] = (char)c;
    }
    return n;
}
