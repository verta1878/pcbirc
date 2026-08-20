/* PCBTOOLS_C.H - C-linkage wrapper for pcbtools.h 
 * Include this instead of pcbtools.h in C++ code that links against
 * C-compiled PCBoard libraries.
 * Written by: hexadecimal, v0.036
 */
#ifndef PCBTOOLS_C_H
#define PCBTOOLS_C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Temporarily suppress C++ overload checking */
#include "pcbtools.h"

#ifdef __cplusplus
}
#endif

#endif
