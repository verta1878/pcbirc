/*
 * utrayit.h — Cross-platform console tray/minimize for pcbis
 * Converted to C from utrayit.pas by sysop/0 (fpc264irc, GPL v3.0)
 *
 * Windows: system tray icon with left-click restore
 * Unix:    XTWINOPS terminal iconify/de-iconify
 * DOS:     graceful stubs (all functions return 0)
 */

#ifndef H_UTRAYIT
#define H_UTRAYIT

/* Returns 1 if this platform can minimize/restore the console */
int tray_console_supported(void);

/* Returns 1 if this platform has a real notification-area tray */
int tray_supported(void);

/* Minimize/iconify the console window. Returns 1 on success. */
int tray_minimize(void);

/* Restore/de-iconify the console window. Returns 1 on success. */
int tray_restore(void);

/* Hide console and show tray icon (Windows).
   Falls back to tray_minimize on other platforms.
   Returns 1 on success. */
int tray_to_tray(const char *tip);

/* Remove tray icon and show console again. Returns 1 on success. */
int tray_from_tray(void);

/* Clean up — call before exit. Safe to call multiple times. */
void tray_cleanup(void);

#endif /* H_UTRAYIT */
