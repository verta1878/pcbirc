/* owcompat.h — OpenWatcom DDK compatibility shim
 * Fills gaps in OW v2's DDK headers vs Microsoft WDK.
 * Include AFTER ntddk.h/ntddser.h, BEFORE our headers. */

#ifndef OWCOMPAT_H
#define OWCOMPAT_H

#ifdef __WATCOMC__

/* OW ntddser.h typo workarounds */
#ifdef IOCTL_SERIAL_SET_BUAD_RATE
#define IOCTL_SERIAL_SET_BAUD_RATE  IOCTL_SERIAL_SET_BUAD_RATE
#endif
#ifdef IOCTL_SERIAL_SET_DIR
#define IOCTL_SERIAL_SET_DTR        IOCTL_SERIAL_SET_DIR
#endif

/* OW SERIAL_BAUD_RATE struct has 'BuadRate' (typo) */
#define BaudRate BuadRate

/* IoCsq function prototypes (types in wdm.h, protos missing) */
NTSTATUS NTAPI IoCsqInitialize(PIO_CSQ, PIO_CSQ_INSERT_IRP,
    PIO_CSQ_REMOVE_IRP, PIO_CSQ_PEEK_NEXT_IRP, PIO_CSQ_ACQUIRE_LOCK,
    PIO_CSQ_RELEASE_LOCK, PIO_CSQ_COMPLETE_CANCELED_IRP);
VOID NTAPI IoCsqInsertIrp(PIO_CSQ, PIRP, PIO_CSQ_IRP_CONTEXT);
PIRP NTAPI IoCsqRemoveNextIrp(PIO_CSQ, PVOID);

/* Serial MSR bits */
#ifndef SERIAL_MSR_CTS
#define SERIAL_MSR_CTS  0x10
#define SERIAL_MSR_DSR  0x20
#define SERIAL_MSR_RI   0x40
#define SERIAL_MSR_DCD  0x80
#endif

/* Serial event flags */
#ifndef SERIAL_EV_RXCHAR
#define SERIAL_EV_RXCHAR   0x0001
#define SERIAL_EV_RXFLAG   0x0002
#define SERIAL_EV_TXEMPTY  0x0004
#define SERIAL_EV_CTS      0x0008
#define SERIAL_EV_DSR      0x0010
#define SERIAL_EV_RLSD     0x0020
#define SERIAL_EV_BREAK    0x0040
#define SERIAL_EV_ERR      0x0080
#define SERIAL_EV_RING     0x0100
#endif

/* Purge flags */
#ifndef SERIAL_PURGE_TXABORT
#define SERIAL_PURGE_TXABORT 0x01
#define SERIAL_PURGE_RXABORT 0x02
#define SERIAL_PURGE_TXCLEAR 0x04
#define SERIAL_PURGE_RXCLEAR 0x08
#endif

/* Misc missing constants */
#ifndef ERROR_LOG_MAXIMUM_SIZE
#define ERROR_LOG_MAXIMUM_SIZE 150
#endif
#ifndef BusQueryInstanceID
#define BusQueryInstanceID 3
#endif
#ifndef PAGED_CODE
#define PAGED_CODE()
#endif

#include <stdio.h>  /* _snwprintf */

/* SAL annotations — not in OW */
#ifndef _In_
#define _In_
#define _Out_
#define _Inout_
#define _In_opt_
#define _Out_opt_
#define _In_reads_(n)
#define _Out_writes_(n)
#define _In_reads_bytes_(n)
#define _Out_writes_bytes_(n)
#define _Field_range_(a,b)
#define _Field_size_(n)
#define _IRQL_requires_max_(n)
#define _IRQL_requires_(n)
#define _Must_inspect_result_
#define _Use_decl_annotations_
#define __drv_functionClass(n)
#define __drv_requiresIRQL(n)
#define __drv_maxIRQL(n)
#endif

#endif /* __WATCOMC__ */
#endif /* OWCOMPAT_H */
