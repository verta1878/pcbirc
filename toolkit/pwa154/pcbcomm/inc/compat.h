/* ============================================================================
 * compat.h — pcbcomm cross-compiler compat macros
 *
 * Handles the differences between DOS 16-bit C compilers:
 *   Borland C++ 3.1     — 'interrupt' keyword, no underscore prefixes
 *   Microsoft C 7.0     — '_interrupt' keyword
 *   OpenWatcom C 1.9+   — '__interrupt __far' keyword combo
 *
 * License: GPLv3 (pcbirc crew)
 * ==========================================================================*/
#ifndef PCBCOMM_COMPAT_H
#define PCBCOMM_COMPAT_H

#if defined(__WATCOMC__)
  /* OpenWatcom */
# define PCBCOMM_INTERRUPT   void __interrupt __far
# define PCBCOMM_FAR         __far
# define PCBCOMM_INT14_ARGS  unsigned _es, unsigned _ds, \
                             unsigned _di, unsigned _si, \
                             unsigned _bp, unsigned _sp, \
                             unsigned _bx, unsigned _dx, \
                             unsigned _cx, unsigned _ax, \
                             unsigned _ip, unsigned _cs, \
                             unsigned _flags
# define PCBCOMM_AX  _ax
# define PCBCOMM_BX  _bx
# define PCBCOMM_CX  _cx
# define PCBCOMM_DX  _dx
# define PCBCOMM_UNUSED_REGS \
    (void)_bp; (void)_si; (void)_di; (void)_es; (void)_ds; \
    (void)_sp; (void)_ip; (void)_cs; (void)_flags
  typedef void (__interrupt __far *pcbcomm_isr_t)();

#elif defined(__BORLANDC__) || defined(__TURBOC__)
  /* Borland C++ 3.1 / Turbo C — 'interrupt' with these arg names.
   * BC arg order: bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags. */
# define PCBCOMM_INTERRUPT   void interrupt
# define PCBCOMM_FAR         far
# define PCBCOMM_INT14_ARGS  unsigned bp, unsigned di, unsigned si, \
                             unsigned ds, unsigned es, unsigned dx, \
                             unsigned cx, unsigned bx, unsigned ax, \
                             unsigned ip, unsigned cs, unsigned flags
# define PCBCOMM_AX  ax
# define PCBCOMM_BX  bx
# define PCBCOMM_CX  cx
# define PCBCOMM_DX  dx
# define PCBCOMM_UNUSED_REGS \
    (void)bp; (void)si; (void)di; (void)es; (void)ds; \
    (void)cx; (void)ip; (void)cs; (void)flags
  typedef void interrupt (*pcbcomm_isr_t)();

#elif defined(_MSC_VER)
  /* Microsoft C 7.0 — '_interrupt' with underscored register args */
# define PCBCOMM_INTERRUPT   void _interrupt _far
# define PCBCOMM_FAR         _far
# define PCBCOMM_INT14_ARGS  unsigned _es, unsigned _ds, \
                             unsigned _di, unsigned _si, \
                             unsigned _bp, unsigned _sp, \
                             unsigned _bx, unsigned _dx, \
                             unsigned _cx, unsigned _ax, \
                             unsigned _ip, unsigned _cs, \
                             unsigned _flags
# define PCBCOMM_AX  _ax
# define PCBCOMM_BX  _bx
# define PCBCOMM_CX  _cx
# define PCBCOMM_DX  _dx
# define PCBCOMM_UNUSED_REGS \
    (void)_bp; (void)_si; (void)_di; (void)_es; (void)_ds; \
    (void)_sp; (void)_ip; (void)_cs; (void)_flags
  typedef void (_interrupt _far *pcbcomm_isr_t)();

#else
# error "Unknown 16-bit DOS C compiler. Add case for it in compat.h."
#endif

#endif  /* PCBCOMM_COMPAT_H */
