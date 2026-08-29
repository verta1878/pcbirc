x64 (amd64) — Requires Windows WDK
=====================================
OpenWatcom v2 does not have an x86-64 compiler.
Build cyport.sys with WDK amd64 environment.

  1. Install WDK 7600+
  2. Open "x64 Free Build Environment"
  3. cd SOURCE\build\amd64\free
  4. build -cef
  5. Copy obj\amd64\cyport.sys here

User-mode tools (cyinstall, cyloopback, cystress)
are i386 and run on x64 via WoW64.
