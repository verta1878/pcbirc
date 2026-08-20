/* ====================================================================
 * pcbdraw.c — PCBoard ANSI Art Viewer/Editor (main)
 * ====================================================================
 * Standalone tool: view ANSI art, run teleconference server/client.
 *
 * Usage:
 *   pcbdraw view <file.ans>          View ANSI file (terminal dump)
 *   pcbdraw sauce <file.ans>         Show SAUCE metadata
 *   pcbdraw server [port]            Run teleconference server
 *   pcbdraw client <host> [port]     Connect as client
 *
 * All output tagged [PCBDRAW] for pcbis_ui integration.
 *
 * Copyright (C) 2026 pcbrevival contributors (GPLv3)
 * ==================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __NT__
 #include <winsock2.h>
 #include <conio.h>
#elif defined(__OS2__)
 #define INCL_DOS
 #define INCL_KBD
 #include <os2.h>
#else
 #include <unistd.h>
#endif

#include "pcbdraw.h"

/* ---- View: dump ANSI file to terminal ---- */

static int do_view(const char *filename)
{
    PDCanvas *canvas;
    AnsiParser parser;
    SauceRecord sauce;
    int x, y, width = 80;
    PDCanvasElement e;
    unsigned char last_attr = 7;
    const char *ext;

    /* Check SAUCE for width */
    if (sauce_load(filename, &sauce)) {
        if (sauce_get_width(&sauce) > 0)
            width = sauce_get_width(&sauce);
    }

    canvas = canvas_create(width, 500);
    if (!canvas) {
        printf("[PCBDRAW] ERROR: cannot allocate canvas\n");
        return 1;
    }

    /* Detect format by extension */
    ext = strrchr(filename, '.');
    if (!ext) ext = ".ans";

    if (stricmp(ext, ".bin") == 0) {
        binary_load_file(filename, canvas, width);
    } else if (stricmp(ext, ".msg") == 0) {
        pcboard_load_file(filename, canvas);
    } else {
        /* Default: ANSI */
        ansi_init(&parser);
        ansi_load_file(&parser, filename, canvas);
        canvas_trim_height(canvas, ansi_get_final_y(&parser));
    }

    /* Dump to terminal with ANSI colors */
    for (y = 0; y < canvas->height; y++) {
        for (x = 0; x < canvas->width; x++) {
            e = canvas_get(canvas, x, y);
            if (ATTR_BYTE(e.attr) != last_attr) {
                printf("\033[0");
                if (ATTR_BOLD(e.attr)) printf(";1");
                if (ATTR_BLINK(e.attr)) printf(";5");
                printf(";3%d;4%d", ATTR_FG_ONLY(e.attr), ATTR_BG_ONLY(e.attr));
                printf("m");
                last_attr = ATTR_BYTE(e.attr);
            }
            if (e.ch.ch >= 32 && e.ch.ch < 127)
                putchar(e.ch.ch);
            else if (e.ch.ch == 0)
                putchar(' ');
            else
                putchar('.');
        }
        putchar('\n');
    }
    printf("\033[0m");

    canvas_free(canvas);
    return 0;
}

/* ---- SAUCE: show metadata ---- */

static int do_sauce(const char *filename)
{
    SauceRecord sauce;

    if (!sauce_load(filename, &sauce)) {
        printf("[PCBDRAW] No SAUCE record in %s\n", filename);
        return 1;
    }

    printf("[PCBDRAW] SAUCE Record:\n");
    printf("  Title:    %.35s\n", sauce.title);
    printf("  Author:   %.20s\n", sauce.author);
    printf("  Group:    %.20s\n", sauce.group);
    printf("  Date:     %.8s\n", sauce.date);
    printf("  DataType: %d\n", sauce.datatype);
    printf("  FileType: %d\n", sauce.filetype);
    printf("  Width:    %d\n", sauce.tinfo1);
    printf("  Height:   %d\n", sauce.tinfo2);
    printf("  Flags:    0x%02X%s\n", sauce.tflags,
           sauce_get_ice(&sauce) ? " (ICE colors)" : "");
    printf("  FileSize: %ld\n", sauce.filesize);

    return 0;
}

/* ---- Server mode ---- */

static int do_server(unsigned short port)
{
    PDCanvas *canvas;
    PDServer srv;

    canvas = canvas_create(80, 25);
    if (!canvas) return 1;

    if (pd_server_start(&srv, canvas, port) < 0) {
        printf("[PCBDRAW] Failed to start server on port %u\n", port);
        canvas_free(canvas);
        return 1;
    }

    printf("[PCBDRAW] Server running on port %u. Press Ctrl-C to stop.\n", port);

    while (srv.running) {
        pd_server_poll(&srv);
    }

    pd_server_stop(&srv);
    canvas_free(canvas);
    return 0;
}

/* ---- Client mode ---- */

static int do_client(const char *host, unsigned short port)
{
    PDCanvas *canvas;
    PDClient cli;

    canvas = canvas_create(80, 25);
    if (!canvas) return 1;

    if (pd_client_connect(&cli, canvas, host, port, "PCBUser", "") < 0) {
        printf("[PCBDRAW] Failed to connect to %s:%u\n", host, port);
        canvas_free(canvas);
        return 1;
    }

    printf("[PCBDRAW] Connected. Type to chat, Ctrl-C to quit.\n");

    while (cli.connected) {
        pd_client_poll(&cli);
#ifdef __NT__
        if (_kbhit()) {
            /* TODO: read input, send chat */
        }
        Sleep(10);
#elif defined(__OS2__)
        DosSleep(10);
#else
        usleep(10000);
#endif
    }

    pd_client_disconnect(&cli);
    canvas_free(canvas);
    return 0;
}

/* ---- Usage ---- */

static void usage(void)
{
    printf("[PCBDRAW] PCBoard ANSI Art Viewer/Editor v1.0\n");
    printf("[PCBDRAW] C port of PabloDraw (cwensley/MIT, sysop/0 FPC port)\n");
    printf("[PCBDRAW] Copyright (C) 2026 pcbrevival contributors (GPLv3)\n");
    printf("[PCBDRAW]\n");
    printf("[PCBDRAW] Usage:\n");
    printf("[PCBDRAW]   pcbdraw view <file>           View ANSI/BIN/PCB file\n");
    printf("[PCBDRAW]   pcbdraw sauce <file>          Show SAUCE metadata\n");
    printf("[PCBDRAW]   pcbdraw server [port]         Run teleconference server\n");
    printf("[PCBDRAW]   pcbdraw client <host> [port]  Connect as client\n");
    printf("[PCBDRAW]\n");
    printf("[PCBDRAW] Supported: .ans .ansi .bin .msg .diz .ice .cia\n");
}

/* ---- Main ---- */

int main(int argc, char *argv[])
{
    int rc = 0;

#ifdef __NT__
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    if (argc < 2) {
        usage();
        return 1;
    }

    if (stricmp(argv[1], "view") == 0) {
        if (argc < 3) { printf("[PCBDRAW] view requires a filename\n"); return 1; }
        rc = do_view(argv[2]);
    }
    else if (stricmp(argv[1], "sauce") == 0) {
        if (argc < 3) { printf("[PCBDRAW] sauce requires a filename\n"); return 1; }
        rc = do_sauce(argv[2]);
    }
    else if (stricmp(argv[1], "server") == 0) {
        unsigned short port = PD_NET_PORT;
        if (argc >= 3) port = (unsigned short)atoi(argv[2]);
        rc = do_server(port);
    }
    else if (stricmp(argv[1], "client") == 0) {
        unsigned short port = PD_NET_PORT;
        if (argc < 3) { printf("[PCBDRAW] client requires a hostname\n"); return 1; }
        if (argc >= 4) port = (unsigned short)atoi(argv[3]);
        rc = do_client(argv[2], port);
    }
    else {
        printf("[PCBDRAW] Unknown command: %s\n", argv[1]);
        usage();
        rc = 1;
    }

#ifdef __NT__
    WSACleanup();
#endif

    return rc;
}
