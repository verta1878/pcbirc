/*-----------------------------------------------------------------------*
 * filename - thread.c
 *
 * function(s)
 *      _beginthread - create and start a thread
 *      _endthread   - terminate a thread
 *-----------------------------------------------------------------------*/

/*
 *      C/C++ Run Time Library - Version 1.5
 *
 *      Copyright (c) 1991, 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#include <os2bc.h>

#include <_io.h>
#include <_thread.h>
#include <errno.h>

ULONG COMMITSTACK = 1;   // stack is committed by default

extern void _setexc(PEXCEPTIONREGISTRATIONRECORD p);    /* in startup.c */
extern void _unsetexc(PEXCEPTIONREGISTRATIONRECORD p);  /* in startup.c */

/*---------------------------------------------------------------------*

Name            new_thread - thread startup function

Usage           static void new_thread(void *arglist);

Prototype in    local

Description     This function is the first code that is run by a
                new thread created with _beginthread.  The function
                first checks that a thread data area has been
                allocated, and terminates if not.  It then retrieves
                the thread start address from the thread data area,
                and calls that function, passing it arglist as
                its single parameter.

Return value    None.

*---------------------------------------------------------------------*/

static void new_thread(void *arglist)
{
    THREAD_DATA *t;
    EXCEPTIONREGISTRATIONRECORD hand;

    if ((t = _thread_data()) == NULL)
        DosExit(EXIT_THREAD,0);

    /* Set up the RTL exception handler.  Save a pointer to
     * the original registration record in the thread data, so that
     * _endthread() can restore it.  We can't just unregister this handler
     * because there may be active C++ registration records also.
     */
    _asm mov eax, fs:[0];
    t->thread_excep = (void *)_EAX;

    _setexc(&hand);

    /* Call the thread.  Remove the exception handler in case
     * the thread simply returned instead of calling _endthread().
     * Note that if the function returns then all C++ handlers have
     * been removed.
     */
    (*(t->thread_func))(arglist);
    _unsetexc(&hand);
}

/*---------------------------------------------------------------------*

Name            _beginthread - create and start a thread

Usage           int _beginthread(void (*start_address)(void *),
                    unsigned stack_size, void *arglist);

Prototype in    process.h

Description     This function creates a thread.  The thread starts
                execution at start_address.  The size of its stack
                is stack_size; the stack is allocated by they system
                after the stack size is rounded up to the next multiple
                of 4096.  The thread is a passed arglist as its only parameter;
                it may be NULL, but must be present.  The thread
                terminates by simply returning, or by calling _endthread.

Return value    If successful, the thread ID number.
                If unsuccessful, -1 is returned, and errno is set as
                follows:
                    EAGAIN  Too many threads
                    ENOMEM  Not enough memory

*---------------------------------------------------------------------*/

int _RTLENTRY _EXPFUNC _beginthread(void (_USERENTRY *start_address)(void *),
                                unsigned stack_size, void *arglist)
{
    TID tid;
    APIRET apiret;
    THREAD_DATA *t;
    ULONG tflags = (COMMITSTACK<<1)|1;

    /* Create the thread in suspended state.
     */
    if ((apiret = DosCreateThread(&tid, (PFNTHREAD)new_thread,
                (ULONG)arglist, tflags, (ULONG)stack_size)) != 0)
        return (__IOerror((int)apiret));

    /* Make sure that a thread data area can be allocated for thread.
     */
    if ((t = _thread_data_tid((int)tid)) == NULL)
    {
        DosResumeThread(tid);
        errno = ENOMEM;
        return (-1);
    }

    /* Save the starting address of the thread code in the thread data
     * area, so that new_thread() can retrieve it.
     */
    t->thread_func = start_address;
    DosResumeThread(tid);
    return ((int)tid);
}

/*---------------------------------------------------------------------*

Name            _endthread - terminate current thread

Usage           void _endthread(void);

Prototype in

Description     This function terminates the current thread.  The thread
                must have been created by an earlier call to _beginthread.

Return value    None.

*---------------------------------------------------------------------*/

void _RTLENTRY _EXPFUNC _endthread(void)
{
    _EAX = (unsigned) _thread_data()->thread_excep;
    _asm mov fs:[0], eax;

    DosExit(EXIT_THREAD,0);
}
