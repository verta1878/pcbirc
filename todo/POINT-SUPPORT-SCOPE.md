Point Support Scoping — PCBoard 15.41 + QFront
================================================

Status: Partially implemented. Data structures ready. Functional
testing and sysop-facing configuration not done.

FTSC References (downloaded to docs/ftsc/):
  FTS-5002.002  Pointlist formats (Boss, Point, Poss, Fake Net, FidoUser)
  FTS-5005.003  BSO .PNT subdirectories for point mail
  FTS-5000.005  "Point" keyword in nodelist data lines
  FTS-5001.006  RPK/NPK pointlist keeper flags
  FTS-0001.016  TOPT/FMPT kludges in netmail packets
  FSC-0056.001  EMSI handshake point fields
  FTS-5004.001  DNS point addressing (pP.fF.nN.zZ.fidonet.net)


What Clark Already Built (19,585 lines in PCBSRC/MAIN/SOURCE/FIDO/)
====================================================================

FIDOMSG.CPP (1,097 lines):
  - FMPT kludge generation: "\x01FMPT <point>" written when Orig_Point != 0
  - TOPT kludge generation: "\x01TOPT <point>" written when Dest_Point != 0
  - INTL kludge for zone routing
  - MSGHdr.Orig_Point and MSGHdr.Dest_Point fields
  - 4D address display: zone:net/node.point
  - pointBufFrom[100] and pointBufTo[100] buffers for kludge strings

FIDOFUNC.C (3,507 lines):
  - fido_nodestr_to_int(): parses "zone:net/node.point" into integers
  - FMPT parsing from message body: strstr(msgBuf, "\x01FMPT")
  - TOPT parsing from message body: strstr(msgBuf, "\x01TOPT")
  - BSO .PNT directory creation: "NNNNNNNN.PNT\\PPPPPPPP.DLO"
  - Point-aware address formatting throughout

DATA.CPP (978 lines):
  - addrToNum(): parses point from address strings
  - AKA matching includes point comparison (zone+net+node+point)
  - 4D address sprintf throughout
  - NADDRESS struct has point field

PCBTOSS.CPP (3,951 lines):
  - dest_point and orig_point tracked during toss
  - FMPT/TOPT kludge injection when missing from message
  - Point excluded from PATH lines (line 536: "Don't add points to path")
  - Point excluded from SEEN-BY for echomail (line 530)
  - .PNT directory used for point outbound
  - fido_header_type2.DestPoint checked

RECWAZOO.C (579 lines):
  - RemoteHello.my_point received and displayed in handshake
  - OurHello.my_point set from address[0].point
  - 4D address formatting for point nodes

XMITEMSI.C (1,154 lines):
  - remote_address.this_point used in EMSI handshake
  - RemoteHello.my_point sent/received
  - OurHello.my_point set from address[matched_aka].point
  - All AKAs sent with point in EMSI address list

FCONFIG.C (565 lines):
  - AKA loading includes point field
  - node_list[i].point stored per configured node
  - deny_list[i].this_point for point-specific deny

SEENBY.CPP (301 lines):
  - Points excluded from SEEN-BY processing (point != 0 check)

PASSTHRU.CPP (1,154 lines):
  - fido_nodestr_to_int() used for point parsing in pass-through


What QFront Already Has (10,123 lines)
======================================

  - FTN_ADDR struct: zone, net, node, point, domain fields
  - ftn_parse_addr(): parses "1:234/56.7" into FTN_ADDR
  - ftn_format_addr(): formats with point when non-zero
  - ftn_addr_equal(): compares including point
  - bso_base_name(): builds "NNNNNNNN.pnt/PPPPPPPP" for points
  - YooHoo hello packet: my_point field sent/received
  - Netmail headers: orig_point/dest_point in .PKT creation
  - Nodelist parser: handles "Boss" keyword for pointlists


What's Missing — Per Phase
==========================

Phase 2 (FidoNet Suite Integration):
  - [ ] QFront config: allow Address=1:234/56.1 as point AKA
  - [ ] Routing: point-specific rules (Route 1:234/56.* via 1:234/56)
  - [ ] Session: point-to-boss restriction (points only call their boss)
  - [ ] Session: boss-to-point (boss accepts calls from its points)
  - [ ] EMSI: verify point fields match FSC-0056 spec
  - [ ] Test: netmail FROM point (QFront generates FMPT kludge)
  - [ ] Test: netmail TO point (QFront parses TOPT kludge)
  - [ ] Test: outbound .PNT directory created and scanned correctly

Phase 2 (QNLIST — nodelist compiler):
  - [ ] Parse FTS-5002 Boss format pointlists
  - [ ] Parse FTS-5002 Point format pointlists
  - [ ] QNLIST /COMPILEPOINTS command
  - [ ] Point entries in .NDX binary index
  - [ ] Recognize RPK/NPK nodelist flags (FTS-5001)
  - [ ] Private pointlist support (net-level, region-level)
  - [ ] Test: compile a pointlist, lookup a point by address

Phase 3 (EchoMail and Areafix):
  - [ ] Points excluded from SEEN-BY lines (Clark already does this)
  - [ ] Points excluded from PATH lines (Clark already does this)
  - [ ] Areafix commands from point addresses work correctly
  - [ ] EchoMail delivered to point via boss node
  - [ ] Test: point subscribes to echo via Areafix

Phase 4 (TIC File Echo):
  - [ ] TIC Origin/From/To with point addresses
  - [ ] File distribution to points via boss
  - [ ] Test: TIC file received at point address

Phase 6 (pcbis Internet Services):
  - [ ] Telnet: point address in session info
  - [ ] WFC: point address display for connected points

Phase 7 (BinkP in pcbis):
  - [ ] BinkP handshake: point in AKA list
  - [ ] Point-to-boss BinkP session
  - [ ] Test: BinkP session between boss and point

Phase 9 (NNTP/QWK/UUCP2):
  - [ ] QWK packet headers: point address
  - [ ] NNTP gateway: point address in From/To
  - [ ] Test: QWK packet with point addressing

Phase O1 (pcbolms — OpenOLMS):
  - [ ] QWK offline mail: point address in packet
  - [ ] Blue Wave: point address support
  - [ ] Test: offline mail to/from point

PCBoard Main:
  - [ ] PCBSETUP: point address in AKA configuration (FCONFIG.C
        already loads points — verify PCBSETUP displays them)
  - [ ] PCBFIDO: already has full point support (FIDOMSG.CPP,
        FIDOFUNC.C, PCBTOSS.CPP) — verify it works end-to-end
  - [ ] Message editor: point address in netmail addressing
        (DATA.CPP already formats 4D — verify UI)
  - [ ] User display: point address shown where relevant
  - [ ] Conference routing: mail to points forwarded via boss


Sysop Setup Guide (needed for QFRONT.DOC and PCBoard manual addendum)
=====================================================================

For a sysop to set up a PCBoard point system:

1. Get a point number from your boss node's sysop
   (e.g. you are 1:234/56.1, your boss is 1:234/56)

2. Configure your address in qfront.cfg:
     Address=1:234/56.1

3. Get the pointlist from your net/region pointlist keeper
   and compile it:
     QNLIST /COMPILEPOINTS

4. Configure routing — all mail goes through your boss:
     Route * via 1:234/56

5. Your boss's nodelist entry must have your point listed.
   Your boss's QFront must be configured to accept your calls.

6. Netmail addressing uses full 4D: 1:234/56.1
   FMPT/TOPT kludges are generated automatically by PCBTOSS.

7. EchoMail: subscribe via Areafix to your boss node.
   Your boss forwards echomail to you. You are excluded from
   SEEN-BY and PATH lines per FidoNet convention.

8. BSO outbound structure for your boss:
     outbound/00EA0038.pnt/00000001.fut   (your mail to boss)
   Your boss's outbound for you:
     outbound/00EA0038.pnt/00000001.fut   (boss's mail to you)


Testing Checklist
=================

  [ ] Configure as point 1:234/56.1
  [ ] Send netmail to boss 1:234/56 — FMPT kludge present
  [ ] Receive netmail from boss — TOPT kludge parsed correctly
  [ ] BSO creates .PNT subdirectory automatically
  [ ] BSO scans .PNT subdirectory for pending mail
  [ ] QNLIST compiles pointlist — point found by address lookup
  [ ] EMSI handshake: point field correct in both directions
  [ ] YooHoo handshake: my_point field correct
  [ ] BinkP handshake: point in AKA list
  [ ] Areafix subscribe from point address
  [ ] EchoMail received at point — not in SEEN-BY/PATH
  [ ] QWK packet includes point in addressing
  [ ] PCBoard message editor shows 4D address
  [ ] Full round-trip: point sends netmail to another point via bosses
