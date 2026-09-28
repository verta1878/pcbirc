/* FMEMCPY.C — Replaces MEMMOVE.ASM. Far memory copy. */
#include <string.h>

#pragma aux FMEMCPY "*"

void FMEMCPY(void *dst, void *src, int len) {
    memcpy(dst, src, len);
}
