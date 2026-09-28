/*
 *  stbsputn.cpp - streambuf::sputn out-of-line implementation
 *
 *  Missing source file for PCBoard UUCP link targets (UUIN, UUOUT, UUXFER).
 *  Clark never shipped this source. Reconstructed from BC 3.1 iostream.h
 *  inline definition (_BIG_INLINE_ block, lines 359-365).
 *
 *  Non-virtual fast-path: memcpy into put area when n bytes of space
 *  are available in the buffer, otherwise falls through to do_sputn().
 *  When _BIG_INLINE_ is not defined, the compiler needs this
 *  out-of-line version — that is what this file provides.
 *
 *  Compile: BCC +<target>.CFG -c -n<objdir> stbsputn.cpp
 *  Flags:   -ml -K -P -3
 *  Output:  stbsputn.obj → referenced from LNK files as ..\bc31\stbsputn.obj
 */

#include <iostream.h>
#include <mem.h>

int _Cdecl streambuf::sputn(const char _FAR * s, int n)
{
    if (n <= (epptr() - pptr())) {
        memcpy(pptr(), s, n);
        pbump(n);
        return n;
    }
    return do_sputn(s, n);
}
