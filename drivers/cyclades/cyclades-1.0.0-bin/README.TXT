Cyclades Cyclom-Y Driver v1.0.0
================================
ISA + PCI. Win2K through Win11 + DOS.

INSTALL (32-bit Windows):
  1. cd i386\SIGNING && SIGN.BAT
  2. Reboot
  3. i386\cyinstall /install

INSTALL (64-bit Windows):
  1. Build x64 cyport.sys (see x64\README.TXT)
  2. cd x64\SIGNING && SIGN.BAT
  3. Reboot
  4. i386\cyinstall /install (runs via WoW64)

DOS:
  DOS\CYTEST [address]
  DOS\CYFTST [port]

DIRECTORIES:
  i386\     Win32 driver, tools, INFs, signing
  x64\      Win64 driver (WDK build needed)
  DOS\      DOS utilities
  DOC\      Documentation
  SOURCE\   GPLv3 source

EMERGENCY: See EMERGENCY_RECOVERY.txt
