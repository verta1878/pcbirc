/*
 *  stbsgetn.cpp - streambuf::sgetn out-of-line implementation
 *
 *  Missing source file for PCBoard UUCP link targets (UUIN, UUOUT, UUXFER).
 *  Clark never shipped this source. Reconstructed from BC 3.1 iostream.h
 *  inline definition (_BIG_INLINE_ block, lines 367-374).
 *
 *  Non-virtual fast-path: memcpy from get area when n bytes are
 *  available in the buffer, otherwise falls through to do_sgetn().
 *  When _BIG_INLINE_ is not defined, the compiler needs this
 *  out-of-line version — that is what this file provides.
 *
 *  Compile: BCC +<target>.CFG -c -n<objdir> stbsgetn.cpp
 *  Flags:   -ml -K -P -3
 *  Output:  stbsgetn.obj → referenced from LNK files as ..\bc31\stbsgetn.obj
 */

#include <iostream.h>
#include <mem.h>

int _Cdecl streambuf::sgetn(char _FAR * s, int n)
{
    if (n <= (egptr() - gptr())) {
        memcpy(s, gptr(), n);
        gbump(n);
        return n;
    }
    return do_sgetn(s, n);
}
