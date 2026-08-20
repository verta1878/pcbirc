/* ====================================================================
 * vmodem.h — Virtual Modem definitions
 * ==================================================================== */

#ifndef VMODEM_H
#define VMODEM_H

/* OS/2 socket compatibility */
/* OS/2 TCP/IP uses soclose() from SO32DLL.
 * On POSIX builds, map to close(). */
#ifdef __OS2__
  /* soclose is provided by SO32DLL.DLL — no macro needed */
#else
  #ifndef soclose
  #define soclose close
  #endif
#endif

#endif /* VMODEM_H */
