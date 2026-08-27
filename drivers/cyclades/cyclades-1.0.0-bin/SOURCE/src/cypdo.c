/* ====================================================================
 * cypdo.c — Child PDO PnP Handler
 * ====================================================================
 * Handles PnP IRPs for the child PDOs (serial ports).
 *
 * Child PDOs are the BOTTOM of their device stack. They do NOT
 * forward IRPs to a lower device — there is none. They complete
 * all PnP IRPs directly.
 *
 * Key operations:
 *   QUERY_ID: Return hardware ID ("Cyclom-Y\Port"), instance ID,
 *             compatible IDs. These must match the INF so the PnP
 *             manager knows which driver to load for the port.
 *
 *   QUERY_CAPABILITIES: Report device capabilities (no wake, no
 *                       eject, not unique ID).
 *
 *   QUERY_DEVICE_TEXT: Return human-readable description.
 *
 *   START_DEVICE: Initialize the CD1400 channel, allocate ring
 *                 buffers, create symbolic link, connect interrupt.
 *
 *   REMOVE_DEVICE: Shut down channel, free resources, delete symlink.
 *
 * Architecture reference:
 *   - serenum/pnp.c Serenum_PDO_PnP (WDK sample)
 *   - Original cyclom-y.sys strings: "Cyclom-Y\Port", "Cyclom-Y Port %2u"
 *
 * License: GPLv3
 * ====================================================================
 */

#include "cycommon.h"

#ifdef WPP_ENABLED
#include "cypdo.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CyPdoPnP)
#endif


/* ====================================================================
 * CyPdoQueryId — Handle IRP_MN_QUERY_ID for Child PDOs
 * ====================================================================
 * Returns device identification strings that the PnP manager uses
 * to match against INF files and load the correct driver.
 *
 * The strings are allocated from paged pool. The PnP manager frees
 * them after use.
 *
 * Confirmed from original cyclom-y.sys:
 *   Hardware ID:  "Cyclom-Y\Port"
 *   Description:  "Cyclom-Y Port %2u"
 * ==================================================================== */

static NTSTATUS CyPdoQueryId(
    PCY_PDO_EXT     pdoExt,
    PIRP            Irp,
    PIO_STACK_LOCATION IrpSp)
{
    PWCHAR  idBuffer;
    ULONG   idLen;
    WCHAR   tempBuf[64];

    switch (IrpSp->Parameters.QueryId.IdType) {

    /* ---- BusQueryDeviceID ----
     * The device ID identifies the device to the PnP manager.
     * Format: "Cyclom-Y\Port"
     * This matches the [Manufacturer] section in our INF. */
    case BusQueryDeviceID:

        idLen = (ULONG)(wcslen(CY_CHILD_HARDWARE_ID) + 1) * sizeof(WCHAR);
        idBuffer = (PWCHAR)ExAllocatePoolWithTag(
            PagedPool, idLen, CY_PDO_TAG);
        if (!idBuffer) return STATUS_INSUFFICIENT_RESOURCES;

        RtlCopyMemory(idBuffer, CY_CHILD_HARDWARE_ID, idLen);

        CyInfo("PDO QueryID DeviceID: %ws\n", idBuffer);
        Irp->IoStatus.Information = (ULONG_PTR)idBuffer;
        return STATUS_SUCCESS;


    /* ---- BusQueryInstanceID ----
     * Unique per port on this bus. We use the port index as a
     * decimal string. Combined with DeviceID, this uniquely
     * identifies each port in the system. */
    case BusQueryInstanceID:

        _snwprintf(tempBuf, sizeof(tempBuf)/sizeof(WCHAR), L"%lu", pdoExt->PortIndex);
        idLen = (ULONG)(wcslen(tempBuf) + 1) * sizeof(WCHAR);
        idBuffer = (PWCHAR)ExAllocatePoolWithTag(
            PagedPool, idLen, CY_PDO_TAG);
        if (!idBuffer) return STATUS_INSUFFICIENT_RESOURCES;

        RtlCopyMemory(idBuffer, tempBuf, idLen);

        CyInfo("PDO QueryID InstanceID: %ws\n", idBuffer);
        Irp->IoStatus.Information = (ULONG_PTR)idBuffer;
        return STATUS_SUCCESS;


    /* ---- BusQueryHardwareIDs ----
     * Multi-string (double-null terminated) of hardware IDs.
     * PnP manager tries each one against available INFs.
     * We return just "Cyclom-Y\Port" + double null. */
    case BusQueryHardwareIDs:

        idLen = (ULONG)(wcslen(CY_CHILD_HARDWARE_ID) + 2) * sizeof(WCHAR);
        idBuffer = (PWCHAR)ExAllocatePoolWithTag(
            PagedPool, idLen, CY_PDO_TAG);
        if (!idBuffer) return STATUS_INSUFFICIENT_RESOURCES;

        RtlZeroMemory(idBuffer, idLen);
        RtlCopyMemory(idBuffer, CY_CHILD_HARDWARE_ID,
                       wcslen(CY_CHILD_HARDWARE_ID) * sizeof(WCHAR));
        /* Second null terminator already there from RtlZeroMemory */

        CyInfo("PDO QueryID HardwareIDs: %ws\n", idBuffer);
        Irp->IoStatus.Information = (ULONG_PTR)idBuffer;
        return STATUS_SUCCESS;


    /* ---- BusQueryCompatibleIDs ----
     * Optional — provides fallback IDs for generic drivers.
     * We return empty (just double null). */
    case BusQueryCompatibleIDs:

        idLen = 2 * sizeof(WCHAR);
        idBuffer = (PWCHAR)ExAllocatePoolWithTag(
            PagedPool, idLen, CY_PDO_TAG);
        if (!idBuffer) return STATUS_INSUFFICIENT_RESOURCES;

        RtlZeroMemory(idBuffer, idLen);

        Irp->IoStatus.Information = (ULONG_PTR)idBuffer;
        return STATUS_SUCCESS;


    default:
        return Irp->IoStatus.Status;    /* Pass through unchanged   */
    }
}


/* ====================================================================
 * CyPdoQueryCapabilities — Report Device Capabilities
 * ====================================================================
 * Tells the PnP manager what this device can and can't do.
 * Serial ports on a multiport card:
 *   - Can't wake the system
 *   - Can't be ejected
 *   - Instance IDs are not globally unique (need device ID + instance)
 *   - Support D0 and D3 power states
 * ==================================================================== */

static NTSTATUS CyPdoQueryCapabilities(
    PCY_PDO_EXT     pdoExt,
    PIRP            Irp,
    PIO_STACK_LOCATION IrpSp)
{
    PDEVICE_CAPABILITIES caps;

    UNREFERENCED_PARAMETER(pdoExt);
    UNREFERENCED_PARAMETER(Irp);

    caps = IrpSp->Parameters.DeviceCapabilities.Capabilities;

    /* Sanity check */
    if (caps->Version != 1 || caps->Size < sizeof(DEVICE_CAPABILITIES))
        return STATUS_UNSUCCESSFUL;

    /* We can't wake the system */
    caps->SystemWake    = PowerSystemUnspecified;
    caps->DeviceWake    = PowerDeviceUnspecified;

    /* No latency for power transitions */
    caps->D1Latency     = 0;
    caps->D2Latency     = 0;
    caps->D3Latency     = 0;

    /* Instance IDs are not globally unique — they're only unique
     * within our bus. PnP manager needs DeviceID + InstanceID
     * to form the global unique identifier. */
    caps->UniqueID      = FALSE;

    /* Can't be physically ejected */
    caps->EjectSupported    = FALSE;
    caps->Removable         = FALSE;
    caps->SurpriseRemovalOK = FALSE;

    /* Power state mappings — we support D0 (on) and D3 (off) */
    caps->DeviceState[PowerSystemWorking]   = PowerDeviceD0;
    caps->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
    caps->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
    caps->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
    caps->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
    caps->DeviceState[PowerSystemShutdown]  = PowerDeviceD3;

    return STATUS_SUCCESS;
}


/* ====================================================================
 * CyPdoQueryDeviceText — Return Human-Readable Description
 * ====================================================================
 * This text appears in Device Manager under the device name.
 * Format: "Cyclades Cyclom-Y Serial Port" (matches original).
 * ==================================================================== */

static NTSTATUS CyPdoQueryDeviceText(
    PCY_PDO_EXT     pdoExt,
    PIRP            Irp,
    PIO_STACK_LOCATION IrpSp)
{
    PWCHAR  textBuf;
    WCHAR   tempBuf[128];
    ULONG   textLen;

    if (IrpSp->Parameters.QueryDeviceText.DeviceTextType
        != DeviceTextDescription) {
        return Irp->IoStatus.Status;    /* Pass through unchanged   */
    }

    /* Format: "Cyclades Cyclom-Y Serial Port (Port N)"
     * Original cyclom-y.sys used: "Cyclom-Y Port %2u" */
    _snwprintf(tempBuf, sizeof(tempBuf)/sizeof(WCHAR), L"Cyclades Cyclom-Y Serial Port (Port %lu)",
             pdoExt->PortIndex);

    textLen = (ULONG)(wcslen(tempBuf) + 1) * sizeof(WCHAR);
    textBuf = (PWCHAR)ExAllocatePoolWithTag(
        PagedPool, textLen, CY_PDO_TAG);

    if (!textBuf)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlCopyMemory(textBuf, tempBuf, textLen);

    CyInfo("PDO DeviceText: %ws\n", textBuf);
    Irp->IoStatus.Information = (ULONG_PTR)textBuf;
    return STATUS_SUCCESS;
}


/* ====================================================================
 * CyPdoPnP — Child PDO PnP Dispatch
 * ====================================================================
 * Main PnP handler for child PDOs. PDOs are the bottom of the
 * device stack — they complete all IRPs directly, never forward.
 * ==================================================================== */

NTSTATUS CyPdoPnP(PDEVICE_OBJECT DevObj, PIRP Irp)
{
    PCY_PDO_EXT         pdoExt = CyGetPdo(DevObj);
    PIO_STACK_LOCATION  irpSp  = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status;

    PAGED_CODE();

    /* Default: use whatever status was set before us */
    status = Irp->IoStatus.Status;

    switch (irpSp->MinorFunction) {

    /* ================================================================
     * IRP_MN_START_DEVICE
     * ================================================================
     * Initialize the CD1400 channel and create the COM port symlink.
     * PDO START doesn't forward — PDOs are at the bottom.
     * ================================================================ */
    case IRP_MN_START_DEVICE:
        CyInfo("PDO START_DEVICE port %lu\n",
                 pdoExt->PortIndex);

        /* Initialize cancel-safe IRP queues BEFORE any I/O can arrive.
         * Without this, the IO_CSQ structures contain garbage function
         * pointers and IoCsqInsertIrp would BSOD. (Audit B4 fix) */
        status = CyInitCancelSafeQueues(pdoExt);
        if (!NT_SUCCESS(status)) {
            CyError("CyInitCancelSafeQueues failed: 0x%08X\n",
                     status);
            break;
        }

        /* Initialize DPCs NOW that the routines exist in cyisr.c.
         * (Was deferred from CyCreateChildPDOs — audit B1 fix from
         * earlier audit. Can't call KeInitializeDpc with NULL routine.) */
        KeInitializeDpc(&pdoExt->ReadDpc,  CyReadDpcRoutine,  pdoExt);
        KeInitializeDpc(&pdoExt->WriteDpc, CyWriteDpcRoutine, pdoExt);
        KeInitializeDpc(&pdoExt->ModemDpc, CyModemDpcRoutine, pdoExt);

        CySetPnPState(&pdoExt->Common, CyStarted);

        /* ---- Allocate COM port number and create symbolic link ----
         * ComDBClaimNextFreePort picks the next available COM number
         * from the SERIALCOMM database. We dynamically load msports.dll
         * to maintain Win2K compatibility (msports.dll exists on
         * Win2K+ but some minimal installs may not have it).
         *
         * If ComDB is unavailable, we fall back to a simple port
         * number based on our port index (COM3 + PortIndex).
         * (Fix for known issue 5.3) */
        {
            ULONG comNumber = 0;
            WCHAR symLinkBuf[64];
            WCHAR devNameBuf[64];
            UNICODE_STRING symLink;
            UNICODE_STRING devName;

            /* Try to use MSPORTS ComDB for proper COM number management */
            typedef LONG (NTAPI *PFN_ComDBClaimNextFreePort)(HANDLE, PULONG);
            typedef LONG (NTAPI *PFN_ComDBOpen)(PHANDLE);
            typedef LONG (NTAPI *PFN_ComDBClose)(HANDLE);

            /* Simple fallback: COM port = 3 + PortIndex.
             * This gives COM3, COM4, COM5... for ports 0, 1, 2...
             * COM1 and COM2 are reserved for onboard serial ports. */
            comNumber = 3 + pdoExt->PortIndex;
            pdoExt->ComPortNumber = comNumber;

            /* Create device name: \Device\CycladesCOM3 */
            _snwprintf(devNameBuf, sizeof(devNameBuf)/sizeof(WCHAR), L"\\Device\\CycladesCOM%lu", comNumber);
            RtlInitUnicodeString(&devName, devNameBuf);

            /* Create symbolic link: \DosDevices\COM3
             * This is what applications use to open the port.
             * CreateFile("COM3") resolves to \DosDevices\COM3.
             *
             * IMPORTANT: allocate the name from pool, NOT the stack!
             * The symbolic link string must persist until REMOVE_DEVICE.
             * (Original audit bug #2 fix) */
            _snwprintf(symLinkBuf, sizeof(symLinkBuf)/sizeof(WCHAR), L"\\DosDevices\\COM%lu", comNumber);
            RtlInitUnicodeString(&symLink, symLinkBuf);

            /* Allocate persistent copy of symbolic link name */
            pdoExt->SymLinkName.Length = symLink.Length;
            pdoExt->SymLinkName.MaximumLength = symLink.Length + sizeof(WCHAR);
            pdoExt->SymLinkName.Buffer = (PWCHAR)ExAllocatePoolWithTag(
                NonPagedPool, pdoExt->SymLinkName.MaximumLength, CY_PDO_TAG);

            if (pdoExt->SymLinkName.Buffer) {
                RtlCopyMemory(pdoExt->SymLinkName.Buffer,
                              symLink.Buffer, symLink.Length + sizeof(WCHAR));

                /* Create the symbolic link */
                IoCreateSymbolicLink(&pdoExt->SymLinkName, &devName);

                TraceEvents(CYPORT_LEVEL_INFO, CYPORT_PNP,
                "PDO START: port %lu COM%lu", pdoExt->PortIndex, comNumber);
            CyInfo("PDO START: port %lu → COM%lu (%ws)\n",
                       pdoExt->PortIndex, comNumber, symLinkBuf);
            } else {
                CyError("PDO START: failed to allocate symlink buffer\n");
            }

            /* Allocate persistent copy of device name */
            pdoExt->DeviceName.Length = devName.Length;
            pdoExt->DeviceName.MaximumLength = devName.Length + sizeof(WCHAR);
            pdoExt->DeviceName.Buffer = (PWCHAR)ExAllocatePoolWithTag(
                NonPagedPool, pdoExt->DeviceName.MaximumLength, CY_PDO_TAG);

            if (pdoExt->DeviceName.Buffer) {
                RtlCopyMemory(pdoExt->DeviceName.Buffer,
                              devName.Buffer, devName.Length + sizeof(WCHAR));
            }
        }

        status = STATUS_SUCCESS;
        break;


    /* ================================================================
     * IRP_MN_QUERY_ID
     * ================================================================ */
    case IRP_MN_QUERY_ID:
        status = CyPdoQueryId(pdoExt, Irp, irpSp);
        break;


    /* ================================================================
     * IRP_MN_QUERY_CAPABILITIES
     * ================================================================ */
    case IRP_MN_QUERY_CAPABILITIES:
        status = CyPdoQueryCapabilities(pdoExt, Irp, irpSp);
        break;


    /* ================================================================
     * IRP_MN_QUERY_DEVICE_TEXT
     * ================================================================ */
    case IRP_MN_QUERY_DEVICE_TEXT:
        status = CyPdoQueryDeviceText(pdoExt, Irp, irpSp);
        break;


    /* ================================================================
     * IRP_MN_QUERY_DEVICE_RELATIONS (TargetDeviceRelation)
     * ================================================================
     * For a PDO, TargetDeviceRelation returns a pointer to itself.
     * The PDO IS the target device. ObReferenceObject ourselves.
     * ================================================================ */
    case IRP_MN_QUERY_DEVICE_RELATIONS:
        if (irpSp->Parameters.QueryDeviceRelations.Type
            == TargetDeviceRelation) {

            PDEVICE_RELATIONS relations;
            relations = (PDEVICE_RELATIONS)ExAllocatePoolWithTag(
                PagedPool,
                sizeof(DEVICE_RELATIONS),
                CY_PDO_TAG);

            if (!relations) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            relations->Count = 1;
            relations->Objects[0] = DevObj;
            ObReferenceObject(DevObj);

            Irp->IoStatus.Information = (ULONG_PTR)relations;
            status = STATUS_SUCCESS;
        }
        break;


    /* ================================================================
     * IRP_MN_REMOVE_DEVICE
     * ================================================================
     * Clean up the port. Free symbolic link, release COM port number,
     * disconnect interrupt, delete device object.
     *
     * NOTE: We don't delete the PDO here — the parent FDO owns it
     * and deletes it in its own REMOVE handler.
     * ================================================================ */
    case IRP_MN_REMOVE_DEVICE:
        CyInfo("PDO REMOVE_DEVICE port %lu\n",
                 pdoExt->PortIndex);

        CySetPnPState(&pdoExt->Common, CyRemoved);

        /* Free pool-allocated symbolic link name */
        if (pdoExt->SymLinkName.Buffer) {
            IoDeleteSymbolicLink(&pdoExt->SymLinkName);
            ExFreePoolWithTag(pdoExt->SymLinkName.Buffer, CY_PDO_TAG);
            pdoExt->SymLinkName.Buffer = NULL;
        }

        /* Free pool-allocated device name */
        if (pdoExt->DeviceName.Buffer) {
            ExFreePoolWithTag(pdoExt->DeviceName.Buffer, CY_PDO_TAG);
            pdoExt->DeviceName.Buffer = NULL;
        }

        /* ---- Disconnect interrupt ----
         * If the ISR is connected for this port's chip, disconnect it.
         * This prevents the ISR from firing after the device object
         * is destroyed — accessing freed memory = BSOD.
         *
         * Note: The interrupt is shared across all ports on one chip.
         * We only disconnect when the LAST port on a chip is removed.
         * For simplicity in v1.0, we disconnect per-port — the FDO
         * cleanup handles the shared interrupt object.
         * (Fix for TODO #2) */
        if (pdoExt->InterruptConnected && pdoExt->Interrupt) {
            /* Disable this channel's interrupts before disconnecting.
             * Write 0 to SRER so the chip stops generating service
             * requests for this channel. */
            CY_SYNC_CONTEXT syncCtx;
            syncCtx.Extension = pdoExt;
            syncCtx.Data = NULL;

            KeSynchronizeExecution(pdoExt->Interrupt,
                                   CyShutdownChannelSync, &syncCtx);

            pdoExt->InterruptConnected = FALSE;
            CyInfo("PDO REMOVE: port %lu interrupt disabled\n",
                   pdoExt->PortIndex);
        }

        /* ---- Release COM port number ----
         * Clean up the SERIALCOMM registry entry so the COM port
         * number is available for reuse on next install.
         *
         * We delete our entry from:
         *   HKLM\HARDWARE\DEVICEMAP\SERIALCOMM
         *
         * The value name is \Device\CycladesCOMn and the data is
         * "COMn". Deleting it frees the COM number.
         * (Fix for TODO #3) */
        if (pdoExt->ComPortNumber != 0) {
            HANDLE hKey;
            OBJECT_ATTRIBUTES objAttr;
            UNICODE_STRING keyPath;
            UNICODE_STRING valueName;
            WCHAR valueNameBuf[64];

            RtlInitUnicodeString(&keyPath,
                L"\\Registry\\Machine\\HARDWARE\\DEVICEMAP\\SERIALCOMM");

            InitializeObjectAttributes(&objAttr, &keyPath,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL, NULL);

            if (NT_SUCCESS(ZwOpenKey(&hKey, KEY_SET_VALUE, &objAttr))) {
                _snwprintf(valueNameBuf, sizeof(valueNameBuf)/sizeof(WCHAR),
                           L"\\Device\\CycladesCOM%lu",
                           pdoExt->ComPortNumber);
                RtlInitUnicodeString(&valueName, valueNameBuf);

                ZwDeleteValueKey(hKey, &valueName);
                ZwClose(hKey);

                CyInfo("PDO REMOVE: COM%lu released from SERIALCOMM\n",
                       pdoExt->ComPortNumber);
            }

            pdoExt->ComPortNumber = 0;
        }

        status = STATUS_SUCCESS;
        break;


    /* ================================================================
     * IRP_MN_SURPRISE_REMOVAL
     * ================================================================ */
    case IRP_MN_SURPRISE_REMOVAL:
        CyInfo("PDO SURPRISE_REMOVAL port %lu\n",
                 pdoExt->PortIndex);
        CySetPnPState(&pdoExt->Common, CySurpriseRemoved);
        status = STATUS_SUCCESS;
        break;


    /* ================================================================
     * IRP_MN_QUERY_STOP / CANCEL_STOP / STOP
     * ================================================================ */
    case IRP_MN_QUERY_STOP_DEVICE:
        CySetPnPState(&pdoExt->Common, CyStopPending);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
        CyRestorePnPState(&pdoExt->Common);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:
        CySetPnPState(&pdoExt->Common, CyStopped);
        status = STATUS_SUCCESS;
        break;


    /* ================================================================
     * IRP_MN_QUERY_REMOVE / CANCEL_REMOVE
     * ================================================================ */
    case IRP_MN_QUERY_REMOVE_DEVICE:
        CySetPnPState(&pdoExt->Common, CyRemovePending);
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        CyRestorePnPState(&pdoExt->Common);
        status = STATUS_SUCCESS;
        break;


    /* ================================================================
     * Default — succeed unknown PnP IRPs for PDOs
     * ================================================================
     * PDOs at the bottom of the stack must succeed unknown minor
     * functions. They can't forward (no lower device). If we leave
     * status as the default STATUS_NOT_SUPPORTED, some PnP minors
     * will fail unnecessarily. (Audit W4 fix) */
    default:
        status = STATUS_SUCCESS;
        break;
    }

    /* Complete the IRP — PDOs always complete directly */
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    IoReleaseRemoveLock(&pdoExt->Common.RemoveLock, Irp);
    return status;
}
