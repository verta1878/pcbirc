/* ====================================================================
 * wf_ctl.c — WinFOSSIL Control Utility (WNFOSCTL.EXE)
 * ====================================================================
 * CLI: wnfosctl <port> [LOCK <baud> | UNLOCK | STATUS]
 * Matches original WinFOSSIL Control Utility behavior.
 *
 * Build: gcc -o WNFOSCTL.EXE wf_ctl.c registry_compat.c
 *            -ladvapi32
 *
 * GPLv3 — FPC264IRC Contributors, 2026.
 * ==================================================================== */

#ifdef _WIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wf_core.h"

extern void wf_compat_print_banner(void);
extern int wf_compat_platform(void);

static void usage(void)
{
    printf("WinFOSSIL Control Utility v" WF_VERSION_STR "\n\n");
    printf("Usage:\n");
    printf("   wnfosctl <port> [<command> [options]]\n\n");
    printf("<port> is the port identifier to control, zero based.\n");
    printf("       (e.g. 0 is COM1, 1 is COM2 and so on.\n\n");
    printf("<command> is one of:\n");
    printf("   LOCK <baud>  - locks the port baud rate to <baud>\n");
    printf("   UNLOCK       - unlocks the port baud rate\n");
    printf("   STATUS       - show port status\n");
    printf("   ENABLE       - enable port\n");
    printf("   DISABLE      - disable port\n");
}

int main(int argc, char *argv[])
{
    int port;
    WfPortConfig cfg;
    const char *cmd;

    if (argc < 2) { usage(); return 0; }

    /* Detect platform and show version */
    printf("WinFOSSIL Control Utility v%s\n", WF_VERSION_STR);

    port = atoi(argv[1]);
    if (port < 0 || port >= WF_MAX_PORTS) {
        printf("ERROR: invalid port %d (must be 0-%d)\n", port, WF_MAX_PORTS - 1);
        return 1;
    }

    /* Load current config */
    memset(&cfg, 0, sizeof(cfg));
    cfg.baud = 9600;
    cfg.rx_buf_size = WF_BUF_SIZE;
    cfg.tx_buf_size = WF_BUF_SIZE;
    wfp_reg_read_port(port, &cfg);

    if (argc < 3) {
        /* Just show status */
        printf("Port %d (%s): %s, %lu baud%s\n",
               port, cfg.name[0] ? cfg.name : "not configured",
               cfg.enabled ? "enabled" : "disabled",
               (unsigned long)cfg.baud,
               cfg.locked ? " LOCKED" : "");
        return 0;
    }

    cmd = argv[2];

    if (_stricmp(cmd, "LOCK") == 0) {
        if (argc < 4) {
            printf("ERROR: LOCK requires a baud rate\n");
            return 1;
        }
        cfg.locked = 1;
        cfg.baud = (uint32_t)atoi(argv[3]);
        wfp_reg_write_port(port, &cfg);
        printf("successfully locked port %d at %ld bps.\n",
               port, (long)cfg.baud);
    }
    else if (_stricmp(cmd, "UNLOCK") == 0) {
        cfg.locked = 0;
        wfp_reg_write_port(port, &cfg);
        printf("successfully unlocked port %d.\n", port);
    }
    else if (_stricmp(cmd, "STATUS") == 0) {
        printf("Port %d: %s\n", port, cfg.enabled ? "enabled" : "disabled");
        printf("  Name:     %s\n", cfg.name[0] ? cfg.name : "(default)");
        printf("  Baud:     %lu%s\n", (unsigned long)cfg.baud,
               cfg.locked ? " (LOCKED)" : "");
        printf("  RX buf:   %d\n", cfg.rx_buf_size);
        printf("  TX buf:   %d\n", cfg.tx_buf_size);
    }
    else if (_stricmp(cmd, "ENABLE") == 0) {
        cfg.enabled = 1;
        if (!cfg.name[0])
            snprintf(cfg.name, WF_PORT_NAME_LEN, "COM%d", port + 1);
        wfp_reg_write_port(port, &cfg);
        printf("Port %d enabled.\n", port);
    }
    else if (_stricmp(cmd, "DISABLE") == 0) {
        cfg.enabled = 0;
        wfp_reg_write_port(port, &cfg);
        printf("Port %d disabled.\n", port);
    }
    else {
        printf("ERROR: unknown command '%s'\n", cmd);
        usage();
        return 1;
    }

    return 0;
}

#endif /* _WIN32 */
