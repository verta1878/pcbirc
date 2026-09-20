/* ============================================================================
 * compat.h — pcbdcom cross-compiler compat macros
 *
 * Handles the differences between DOS 16-bit C compilers:
 *   Borland C++ 3.1     — 'interrupt' keyword, no underscore prefixes
 *   Microsoft C 7.0     — '_interrupt' keyword
 *   OpenWatcom C 1.9+   — '__interrupt __far' keyword combo
 *
 * License: GPLv3 (pcbirc crew)
 * ==========================================================================*/
#ifndef PCBDCOM_COMPAT_H
#define PCBDCOM_COMPAT_H

#if defined(__WATCOMC__)
  /* OpenWatcom */
# define PCBDCOM_INTERRUPT   void __interrupt __far
# define PCBDCOM_FAR         __far
# define PCBDCOM_INT14_ARGS  unsigned _es, unsigned _ds, \
                             unsigned _di, unsigned _si, \
                             unsigned _bp, unsigned _sp, \
                             unsigned _bx, unsigned _dx, \
                             unsigned _cx, unsigned _ax, \
                             unsigned _ip, unsigned _cs, \
                             unsigned _flags
# define PCBDCOM_AX  _ax
# define PCBDCOM_BX  _bx
# define PCBDCOM_CX  _cx
# define PCBDCOM_DX  _dx
# define PCBDCOM_UNUSED_REGS \
    (void)_bp; (void)_si; (void)_di; (void)_es; (void)_ds; \
    (void)_sp; (void)_ip; (void)_cs; (void)_flags
  typedef void (__interrupt __far *pcbdcom_isr_t)();

#elif defined(__BORLANDC__) || defined(__TURBOC__)
  /* Borland C++ 3.1 / Turbo C — 'interrupt' with these arg names.
   * BC arg order: bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags. */
# define PCBDCOM_INTERRUPT   void interrupt
# define PCBDCOM_FAR         far
# define PCBDCOM_INT14_ARGS  unsigned bp, unsigned di, unsigned si, \
                             unsigned ds, unsigned es, unsigned dx, \
                             unsigned cx, unsigned bx, unsigned ax, \
                             unsigned ip, unsigned cs, unsigned flags
# define PCBDCOM_AX  ax
# define PCBDCOM_BX  bx
# define PCBDCOM_CX  cx
# define PCBDCOM_DX  dx
# define PCBDCOM_UNUSED_REGS \
    (void)bp; (void)si; (void)di; (void)es; (void)ds; \
    (void)cx; (void)ip; (void)cs; (void)flags
  typedef void interrupt (*pcbdcom_isr_t)();

#elif defined(_MSC_VER)
  /* Microsoft C 7.0 — '_interrupt' with underscored register args */
# define PCBDCOM_INTERRUPT   void _interrupt _far
# define PCBDCOM_FAR         _far
# define PCBDCOM_INT14_ARGS  unsigned _es, unsigned _ds, \
                             unsigned _di, unsigned _si, \
                             unsigned _bp, unsigned _sp, \
                             unsigned _bx, unsigned _dx, \
                             unsigned _cx, unsigned _ax, \
                             unsigned _ip, unsigned _cs, \
                             unsigned _flags
# define PCBDCOM_AX  _ax
# define PCBDCOM_BX  _bx
# define PCBDCOM_CX  _cx
# define PCBDCOM_DX  _dx
# define PCBDCOM_UNUSED_REGS \
    (void)_bp; (void)_si; (void)_di; (void)_es; (void)_ds; \
    (void)_sp; (void)_ip; (void)_cs; (void)_flags
  typedef void (_interrupt _far *pcbdcom_isr_t)();

#else
# error "Unknown 16-bit DOS C compiler. Add case for it in compat.h."
#endif

#endif  /* PCBDCOM_COMPAT_H */
