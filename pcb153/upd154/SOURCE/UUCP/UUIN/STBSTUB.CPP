#ifndef __WATCOMC__
/* STBSTUB.CPP - streambuf virtual function stubs for BC31 large model.
 * Replaces the missing stbdsgtn.obj, stbdsptn.obj, stbsgetn.obj, stbsputn.obj
 * from Clark's source distribution.
 * Written by: hexadecimal, v0.036
 */
#include <iostream.h>

/* These virtual functions are called by BC31's iostream for the large memory
 * model. The default implementations just call sputc/sbumpc in a loop.
 */
int streambuf::do_sputn(const char far* s, int n)
{
    for (int i = 0; i < n; i++) {
        if (sputc(s[i]) == EOF) return i;
    }
    return n;
}

int streambuf::do_sgetn(char far* s, int n)
{
    for (int i = 0; i < n; i++) {
        int c = sbumpc();
        if (c == EOF) return i;
        s[i] = (char)c;
    }
    return n;
}
#endif
