# FidoNet Technical Standards

Specifications used by QFront v1.0.0.

## Packet & Session Layer

| Spec | Title | Used By |
|------|-------|---------|
| FTS-0001 | Basic FidoNet Technical Standard | PKT_HEADER struct, FTS-1 session |
| FTS-0006 | YooHoo/WaZOO Session Negotiation | wazoo.c — hello packet, CRC-16 |
| FSC-0056 | EMSI/IEMSI Protocol | emsi.c — INQ/DAT/ACK/NAK, CRC-16 |

## Addressing & Routing

| Spec | Title | Used By |
|------|-------|---------|
| FTS-5001 | Nodelist Format | nodelist.c — Zone/Region/Host/Hub/Pvt/Down/Hold |
| FTS-5005 | Binkley-Style Outbound | bso.c — .?lo/.?ut/.bsy/.hld/.try |
| FSC-0053 | FLAGS Extension to BSO | bso.c — flavour flags |
| FSC-0087 | Nodelist Flag Extensions | nodelist.c — CM/MO/IBN/INA flags |

## Echomail & File Distribution

| Spec | Title | Used By |
|------|-------|---------|
| FTS-0004 | EchoMail Specification | Delegated to tosser (hpt/pcbtoss) |
| FTS-5006 | TIC File Format | tic.c — TIC parser, inbound scanner |
| FSC-0057 | Areafix Protocol | Delegated to external (hpt areafix) |

## Transport

| Spec | Title | Used By |
|------|-------|---------|
| FTS-1026 | BinkP Protocol | Reference — binkd handles BinkP |
| FTS-1027 | CRAM Authentication | Reference — binkd handles CRAM |

## File Transfer Protocols

Zmodem and Xmodem are not FidoNet-specific standards.
They are implemented from public specifications:
- Zmodem: Chuck Forsberg, 1986 (public domain)
- Xmodem: Ward Christensen, 1977 (public domain)
- SEAlink: System Enhancement Associates (FidoNet extension)

## Sources

All specifications obtained from the FidoNet Technical Standards
Committee (FTSC) public archive. These are freely distributable
technical standards for interoperable FidoNet software.
