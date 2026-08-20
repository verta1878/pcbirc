PCBoard 15.4 / 15.41 Features from IcyBoard Analysis
=====================================================

Source: github.com/mkrueger/icy_board (differences.md, feature_parity.md,
new_ppl.md)

FEATURES FOR PCBoard 15.4 (Clark's unfinished work):
====================================================

1. PRIVATE MESSAGES — Personal Mail Inbox
   - Separate mail base for private person-to-person messages
   - Not mixed into conference message bases
   - @ reads current caller's inbox
   - @W writes to a user or alias
   - Y-scan includes "E-Mail" line in personal mail scans
   - Comments to sysop delivered to same mailbox
   - One inbox across all conferences
   - Clark started this but didn't finish it

FEATURES FOR PCBoard 15.41 (our work):
======================================

2. CONFERENCE UPDATES — Multiple Message Areas Per Conference
   - One conference can have several named message areas
   - Each area has its own base, access expression, FTN area tag
   - Scans cover all selected areas in a conference
   - Old PPEs address the default area (backward compatible)
   - Area-aware PPL uses AreaId(conf, area) or 4.00 objects
   - Filebase per conference with richer metadata:
     * Uploader tracking
     * Download counts
     * FILE_ID.DIZ auto-import
     * Long file names
     * Archive inspection without shelling to DOS

3. PPL UPDATES — New Commands and Language Features
   For 15.41, versioned additions that DON'T break old PPEs:

   PPL 3.50 (syntax additions, compile to classic PPE format):
   - Scalar variable initializers: INTEGER n = 1
   - Array initializers: INTEGER values = { 1, 2, 3 }
   - Bracket indexing: values[0] (not ambiguous with function calls)
   - Compound assignment: +=, -=, *=, /=, %=, &=, |=
   - Post-test loops: REPEAT ... UNTIL
   - Infinite loops: LOOP ... ENDLOOP with BREAK/CONTINUE
   - Optional parentheses: IF condition THEN (no parens needed)
   - Typed constants: CONST INTEGER MaxAttempts = 3
   - Nominal integer enums: ENUM Color ... ENDENUM
   - Routine parameters: pass function/procedure as callable

   PPL 4.00 (requires new runtime):
   - Main-program block: BEGIN ... END (EXIT replaces old END)
   - Board objects: CONFERENCE, DIRECTORY, AREA, DOOR, PASSWORD
   - Member calls: ConfInfo(conf), AreaId(conf, area)
   - Message-area identifiers: MSGAREAID
   - Overloaded built-ins: ConfInfo(conf), Len(array, dim)
   - User-defined records: TYPE ... ENDTYPE
   - Named record literals: Point { X = 1, Y = 2 }
   - Web request functions (for future web integration)

   DECLARE is optional at all language versions (compiler collects
   signatures before generating code).
   RETURN expression accepted in classic source too.

4. TELECONFERENCE UPDATES
   - Multi-node chat across machines (Phase 23 pcbnet)
   - TCP teleconference between nodes
   - Chat commands, access levels
   - PCBDraw ANSI art teleconference integration

5. OTHER ICYBOARD FEATURES TO CONSIDER:
   - Hashed passwords (Argon2id/bcrypt) — important for internet
   - Security expressions (groups + age) — extends numeric levels
   - UTF-8 BOM detection — CP437 default, UTF-8 optional
   - Configurable commands — Clark partially has via CMD.LST
   - PPL toolchain: compiler, decompiler, formatter, LSP, tree-sitter
   - BBSLink door integration
   - Multiple drop file formats

COMPATIBILITY RULES:
====================
- Old PPEs MUST work unchanged
- New PPL features are versioned — old source stays old
- Conference default area = backward compatible
- PPLC output format stays compatible with Clark's PPE format
- Binary data formats preserved for DOS
- Security levels preserved (groups ADD, don't replace)
