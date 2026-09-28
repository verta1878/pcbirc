# Synchronet / SyncTerm Protocol Source

Source files from Synchronet BBS / SyncTerm terminal for reference
and potential integration into pcbrevival.

## License

All files are Copyright Rob Swindell and contributors.
Licensed under GNU General Public License v2 or later.
Source: https://gitlab.synchro.net/main/sbbs

## Files

- sexyz.c — Synchronet External X/Y/ZMODEM protocol driver (GPL v2+)
- zmodem.c/h — Zmodem protocol implementation
- xmodem.c/h — Xmodem/Ymodem protocol implementation  
- ftpsrvr.cpp/h — Synchronet FTP server (GPL v2+)
- wren_bind_sftp.c — SyncTerm SFTP client bindings

## Credits

- Rob Swindell (digital man) — SEXYZ, FTP server, Synchronet
- Stephen Hurd (Deuce) — SyncTerm, Unix stdio mode
- Jacques Mattheij — original zmtx/zmrx Zmodem (MIT relicensed)

## Usage in pcbrevival

The SEXYZ protocol driver can be used as PCBoard's external protocol
handler, replacing the DOS-era DSZ/CEXYZ. The FTP server code provides
reference for implementing `ftpserve.exe` (the companion FTP handler
invoked by PCBSF.BAT/PCBST.BAT/PCBRF.BAT).
