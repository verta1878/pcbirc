/* PCBEXTERNC.H - force extern "C" around PCB library headers for C++ builds.
 * Include this BEFORE any PCB headers in UUCP .CPP files.
 */
#ifndef PCBEXTERNC_H
#define PCBEXTERNC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pcbtools.h>
#include <dosfunc.h>
#include <cnameidx.h>
#include <validate.h>
#include <system.h>

#ifdef __cplusplus
}
#endif

#endif
