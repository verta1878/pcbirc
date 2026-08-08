# License — SIO Clean-Room Reimplementation

## GNU General Public License v3.0

Copyright (c) 2026 SIO Rebuild Contributors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.

## Clean-Room Declaration

This software is a clean-room reimplementation of serial I/O driver
functionality for OS/2. No binary analysis, disassembly, or reverse
engineering of any existing software was performed.

All code was written from:
- Published documentation (SIOREF.TXT, VX00.TXT, VMODEM.TXT, DESIGN.TXT)
- OS/2 Toolkit 4.5 DDK headers (IBM, publicly available)
- FTS-0001 Rev 5 FOSSIL specification (FidoNet, public domain)
- UART datasheets (National Semiconductor, Intel, publicly available)
- OS/2 Physical Device Driver Reference (IBM)
- RFC 854 (Telnet), RFC 1321 (MD5)

## Third-Party Components

- **MD5 implementation** (`vmodem/md5.c`): Derived from the RSA Data
  Security, Inc. reference implementation, which was released into the
  public domain per RFC 1321.

- **OS/2 DDK headers** (`inc/`): From IBM OS/2 Toolkit 4.5, distributed
  freely by IBM.

- **PCI.INC**: PCI device ID database format from SIO2K documentation,
  user-contributed entries.
