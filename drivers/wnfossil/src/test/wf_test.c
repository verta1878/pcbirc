/* wf_test.c — WinFOSSIL Test Suite
 * GPLv3 — FPC264IRC Contributors, 2026. */

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wf_core.h"

static int g_pass = 0, g_fail = 0, g_num = 0;

static void t(const char *name, int cond)
{
    g_num++;
    printf("  T%02d %-44s %s\n", g_num, name, cond ? "PASS" : "FAIL");
    if (cond) g_pass++; else g_fail++;
}

int main(void)
{
    WfPort p;
    WfRingBuf b;
    WfFossilInfo info;
    int ch, r;
    uint16_t st;

    /* Clean registry from previous runs so tests start fresh */
#ifdef _WIN32
    RegDeleteKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\WinFOSSIL\\Port0");
    RegDeleteKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\WinFOSSIL");
#endif

    printf("WinFOSSIL Test Suite v" WF_VERSION_STR "\n");
    printf("=========================================\n");

    /* === RING BUFFER === */
    printf("\n=== RING BUFFER ===\n");
    wf_buf_clear(&b);
    t("Empty count = 0", wf_buf_count(&b) == 0);
    wf_buf_put(&b, 0x41);
    t("Put 1, count = 1", wf_buf_count(&b) == 1);
    ch = wf_buf_get(&b);
    t("Get returns 0x41", ch == 0x41);
    ch = wf_buf_get(&b);
    t("Get empty = -1", ch == -1);
    wf_buf_put(&b, 0x42);
    ch = wf_buf_peek(&b);
    t("Peek = 0x42, count still 1", ch == 0x42 && wf_buf_count(&b) == 1);

    /* === BAUD RATES === */
    printf("\n=== BAUD RATES ===\n");
    t("Decode 0xE3 = 9600", wf_decode_baud(0xE3) == 9600);
    t("Decode 0x83 = 1200", wf_decode_baud(0x83) == 1200);
    t("Decode 0x03 = 19200", wf_decode_baud(0x03) == 19200);
    t("Decode 0x23 = 38400", wf_decode_baud(0x23) == 38400);
    t("Encode 9600 roundtrips", wf_decode_baud(wf_encode_baud(9600)) == 9600);

    /* === PORT LIFECYCLE === */
    printf("\n=== PORT LIFECYCLE ===\n");
    r = wf_init(&p, 0);
    t("Init returns 0x1954", r == WF_SIGNATURE);
    t("Port active", p.active == 1);
    t("Default baud 9600", p.cfg.baud == 9600);
    t("Buffers empty", wf_buf_count(&p.rxbuf) == 0);
    wf_deinit(&p);
    t("Deinit sets inactive", p.active == 0);

    /* === FOSSIL API === */
    printf("\n=== FOSSIL API ===\n");
    wf_init(&p, 0);
    wf_set_params(&p, 0xE3);
    t("Fn00 set_params → 9600", p.cfg.baud == 9600);
    wf_send_wait(&p, 'A');
    t("Fn01 send_wait → txbuf=1", wf_buf_count(&p.txbuf) == 1);
    r = wf_send_nowait(&p, 'B');
    t("Fn0B send_nowait → 1", r == 1);
    t("Fn0C peek empty rx → -1", wf_peek(&p) == -1);
    st = wf_status(&p);
    t("Fn03 status THRE+TSRE", (st & (WF_ST_THRE|WF_ST_TSRE)) == (WF_ST_THRE|WF_ST_TSRE));
    wf_set_dtr(&p, 0);
    t("Fn06 set_dtr stores 0", p.dtr_on == 0);
    wf_set_flow(&p, WF_FLOW_XON);
    t("Fn0F set_flow xon", p.flow_xon == 1);
    wf_etx_handler(&p, 1);
    t("Fn10 etx_handler stores", p.etx_enabled == 1);
    wf_buf_put(&p.rxbuf, 'X');
    wf_purge_rx(&p);
    t("Fn0A purge_rx clears", wf_buf_count(&p.rxbuf) == 0);
    wf_get_info(&p, &info);
    t("Fn1B get_info valid", info.size == sizeof(WfFossilInfo) && info.spec_rev == 5);
    wf_deinit(&p);

    /* === VMODEM === */
    printf("\n=== VMODEM ===\n");
    wf_init(&p, 0);
    wf_vm_init(&p);
    t("VM init command mode", p.vm_state == WF_VM_COMMAND);
    t("VM echo on", p.vm_echo == 1);
    wf_buf_clear(&p.rxbuf);
    strcpy(p.vm_cmd_buf, "AT");
    p.vm_cmd_len = 2;
    wf_vm_parse_cmd(&p);
    t("AT → response in rxbuf", wf_buf_count(&p.rxbuf) > 0);
    wf_buf_clear(&p.rxbuf);
    strcpy(p.vm_cmd_buf, "ATI");
    p.vm_cmd_len = 3;
    wf_vm_parse_cmd(&p);
    t("ATI → ID in rxbuf", wf_buf_count(&p.rxbuf) > 0);
    strcpy(p.vm_cmd_buf, "ATE0");
    p.vm_cmd_len = 4;
    wf_vm_parse_cmd(&p);
    t("ATE0 → echo off", p.vm_echo == 0);
    wf_deinit(&p);

    /* === REGISTRY === */
    printf("\n=== REGISTRY ===\n");
#ifdef _WIN32
    {
        extern int wf_compat_platform(void);
        extern const char *wf_compat_platform_name(void);
        WfPortConfig cfg, cfg2;
        uint32_t val;

        t("Platform detect valid", wf_compat_platform() >= 1);
        t("Platform name set", wf_compat_platform_name()[0] != 0);

        memset(&cfg, 0, sizeof(cfg));
        cfg.enabled = 1; cfg.baud = 57600; cfg.locked = 1;
        cfg.rx_buf_size = 4096; cfg.tx_buf_size = 4096;
        strncpy(cfg.name, "COM1", WF_PORT_NAME_LEN);
        r = wfp_reg_write_port(0, &cfg);
        t("Registry write port", r == 0);

        memset(&cfg2, 0, sizeof(cfg2));
        wfp_reg_read_port(0, &cfg2);
        t("Registry readback match", cfg2.enabled == 1 && cfg2.baud == 57600);

        wfp_reg_write_global("TestVal", 42);
        val = 0;
        wfp_reg_read_global("TestVal", &val);
        t("Registry global r/w", val == 42);
    }
#else
    t("Registry: skipped (not Win32)", 1);
    t("Registry: skipped", 1);
    t("Registry: skipped", 1);
    t("Registry: skipped", 1);
    t("Registry: skipped", 1);
#endif

    /* === COM PORT === */
    printf("\n=== COM PORT ===\n");
    wf_init(&p, 0);
    strncpy(p.cfg.name, "COM99", WF_PORT_NAME_LEN);
    r = wf_open_com(&p);
    t("Open COM99 fails", r != 0);
    st = wf_status(&p);
    t("Status on closed = THRE", (st & WF_ST_THRE) != 0);
    wf_set_dtr(&p, 1);
    wf_set_break(&p, 1);
    t("DTR/break on closed = no crash", 1);
#ifdef _WIN32
    {
        char ports[16][WF_PORT_NAME_LEN];
        int n = wf_enum_ports(ports, 16);
        t("Port enum >= 0", n >= 0);
    }
#else
    t("Port enum: skipped", 1);
#endif
    wf_deinit(&p);

    /* === DLL EXPORTS === */
    printf("\n=== DLL EXPORTS ===\n");
#ifdef _WIN32
    {
        HMODULE hDll = LoadLibraryA("FOSSIL.DLL");
        if (!hDll) hDll = LoadLibraryA("out\\i386\\FOSSIL.DLL");
        t("Load FOSSIL.DLL", hDll != NULL);
        if (hDll) {
            t("commOpenPort found", GetProcAddress(hDll, "commOpenPort") != NULL);
            t("commReadBlock found", GetProcAddress(hDll, "commReadBlock") != NULL);
            t("commVmodemDial found", GetProcAddress(hDll, "commVmodemDial") != NULL);
            t("commGetStatus found", GetProcAddress(hDll, "commGetStatus") != NULL);
            FreeLibrary(hDll);
        } else {
            t("DLL: skipped (not found)", 1);
            t("DLL: skipped", 1);
            t("DLL: skipped", 1);
            t("DLL: skipped", 1);
        }
    }
#else
    t("DLL: skipped (not Win32)", 1);
    t("DLL: skipped", 1);
    t("DLL: skipped", 1);
    t("DLL: skipped", 1);
    t("DLL: skipped", 1);
#endif

    /* === SECURITY === */
    printf("\n=== SECURITY ===\n");
    wf_init(&p, 0);
    p.security.whitelist_count = 0;
    p.security.blacklist_count = 0;
    t("Empty lists allow all", wf_sec_check_ip(&p, "1.2.3.4") == 1);
    strncpy(p.security.tcp_blacklist[0], "10.0.0", 19);
    p.security.blacklist_count = 1;
    t("Blacklist blocks", wf_sec_check_ip(&p, "10.0.0.5") == 0);
    p.security.blacklist_count = 0;
    strncpy(p.security.tcp_whitelist[0], "192.168", 19);
    p.security.whitelist_count = 1;
    t("Whitelist allows match", wf_sec_check_ip(&p, "192.168.1.1") == 1);
    t("Whitelist blocks other", wf_sec_check_ip(&p, "10.0.0.1") == 0);
    wf_deinit(&p);

    /* === PERFORMANCE === */
    printf("\n=== PERFORMANCE ===\n");
    wf_init(&p, 0);
    p.cfg.perf_stats = 1;
    p.perf_rx_bytes = 999;
    wf_perf_reset(&p, 1000);
    t("Reset clears counters", p.perf_rx_bytes == 0);
    p.perf_rx_bytes = 1000;
    p.perf_last_rx = 0;
    p.perf_last_tick = 0;
    wf_perf_update(&p, 1000);
    t("CPS = 1000 bytes/sec", p.perf_cps_rx == 1000);
    wf_deinit(&p);

    /* === RESULTS === */
    printf("\n=========================================\n");
    printf("Results: %d/%d passed", g_pass, g_num);
    if (g_fail > 0) printf(", %d FAILED", g_fail);
    printf("\n");

    return g_fail > 0 ? 1 : 0;
}
