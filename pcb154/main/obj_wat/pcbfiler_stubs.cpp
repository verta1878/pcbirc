/* pcbfiler_stubs.cpp — stubs for PCBFILER linking */
#include <cnameidx.h>
#include <screen.h>

#define VMDATA
#include "unique.hpp"

/* Conference management */
int LIBENTRY getconfbyrec(unsigned rec, cnamesidxtype *idx) {
    (void)rec; (void)idx;
    return -1;
}
unsigned LIBENTRY findconfbyname(char *name, cnamesidxtype *idx) {
    (void)name; (void)idx;
    return 0;
}

/* Font */
fonttype LIBENTRY getfont(void) { return (fonttype)0; }

/* uniquebase::foundinlist */
bool uniquebase::foundinlist(char *Str) {
    (void)Str;
    return 0;
}

/* Archive viewer — stub */
extern "C" int ARCV(char far *filename) {
    (void)filename;
    return 0;
}
