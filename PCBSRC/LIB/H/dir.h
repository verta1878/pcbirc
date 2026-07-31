/* dir.h — Borland→Watcom compatibility wrapper */
#ifdef __WATCOMC__
#include <direct.h>
#else
#error "This wrapper is for Watcom only"
#endif
