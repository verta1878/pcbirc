/* watfix.h — Watcom C++ strictness workarounds for PCBoard source */
#ifdef __WATCOMC__
/* These are Borland C++ permissiveness issues that Watcom flags as errors */
/* E139: enum variable assigned non-enum constant — safe, matches Borland behavior */
/* E473: function argument type mismatch — implicit conversions Borland allowed */
#pragma warning 139 9
#pragma warning 473 9
#pragma warning 665 9
#endif
