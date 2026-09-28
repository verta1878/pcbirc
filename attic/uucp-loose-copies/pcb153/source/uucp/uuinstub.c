/* UUINSTUB.C - stubs for functions referenced by REJECTS.CPP
 * that are normally provided by UUIN's logging and UUOUT dispatch.
 * Written by: hexadecimal, v0.036
 */
#include <stdio.h>
#include <stdarg.h>

void writeUucplog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
}

int uucpQueueOutbound(const char *to, const char *subject, const char *body)
{
    (void)to; (void)subject; (void)body;
    /* Bounce dispatch not implemented — would need UUOUT integration */
    return 0;
}
