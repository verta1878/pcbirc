# MysticBBS Scope vs PCBBS 15.41

MysticBBS by mkrueger — Rust PCBBS clone. Written from scratch with
PPL decompiler, compiler, runtime, tree-sitter grammar, and LSP.
Goal: "PCBBS clone finished on PCBBS's 100th birthday."

Features to evaluate for PCBBS 15.41:

SHORT PATHS vs LONG FILE NAMES
MysticBBS: Relative paths from BBS root. No drive letters needed.
PCB 15.41: Keep short paths for DOS compatibility. Add long name
support on Linux builds. Relative paths when possible.
DECISION: Both modes. #ifdef DOS = short 8.3, Linux = long.

CP437 vs UTF-8
MysticBBS: CP437 compatibility plus UTF-8 (detected by BOM).
PCB 15.41: CP437 is primary (DOS native). UTF-8 on Linux builds.
Same approach as MysticBBS — detect by BOM.
DECISION: Adopt same BOM detection. CP437 default, UTF-8 optional.

ONE MESSAGE BASE PER CONFERENCE vs SEVERAL NAMED AREAS
MysticBBS: Several named message areas per conference.
PCB 15.41: Clark's original = one base per conference. Adding
named areas would break existing PPE calls.
DECISION: Keep Clark's model for DOS. Named areas as optional
extension (new PCBDAT flag). PPE compatibility preserved.

PRIVATE MESSAGES MIXED vs PERSONAL MAIL INBOX
MysticBBS: Personal mail inbox with @/@W and Y-scan integration.
PCB 15.41: Clark already has personal mail scanning (Y-scan).
A dedicated inbox is a UI enhancement, not structural.
DECISION: Add personal inbox view as new command. Existing Y-scan
stays. No structural change to message base.

PPL 3.50/4.00 ADDITIONS
MysticBBS: Versioned PPL additions, records, BBS objects.
Compiler, decompiler, formatter, LSP, tree-sitter grammar.
PCB 15.41: Clark's PPLC is the reference compiler. We should
NOT diverge from Clark's PPL spec — that breaks all existing PPEs.
DECISION: Keep Clark's PPL exactly as-is for 15.41. Document
MysticBBS's extensions as reference for future. DO NOT adopt
PPL extensions that break existing PPEs.

SECURITY LEVELS vs GROUPS + AGE EXPRESSIONS
MysticBBS: Security level + groups + age expressions per command.
PCB 15.41: Clark's numeric security levels (0-255). Adding groups
would be a major change to PCBBS.DAT and user records.
DECISION: Keep Clark's security levels for 15.41. Groups as
future enhancement (Phase 25+). Age expressions = interesting
but low priority.

WHAT WE SHOULD ADD:

1. UTF-8 BOM detection (simple, no-brainer)
2. Configurable commands (Clark partially has this via CMD.LST)
3. Personal mail inbox command
4. Import tool concept (icbsetup import PCBBS.DAT)

WHAT WE SHOULD NOT ADOPT:

1. PPL extensions — breaks existing PPEs
2. .toml config format — keep Clark's binary formats for DOS
3. Named message areas — breaks PPEs that read message bases
4. Removing numeric security levels — fundamental PCBBS concept
5. "CP437 is dead" — CP437 is very much alive on DOS

TOOLS TO REFERENCE:

* pplc: PPL compiler (Rust) — compare output with Clark's PPLC
* ppld: PPL decompiler — useful for understanding existing PPEs
* ppld --check: checks PPE compatibility — useful for us too
* ppl-language-server: LSP for VS Code PPL editing
* tree-sitter-ppl: grammar for syntax highlighting
* icbsetup: TUI configurator — compare with our pcbis

MysticBBS ROADMAP (from their docs):

* SSH/Websocket support (WIP)
* BinkP mailer (planned)
* dBASE3 statements/functions in PPL (planned)
* Web frontend via MysticTerm WebAssembly
* Self-service password reset via email

CONCLUSION:
Clark's original design philosophy.

PCBBS 15.41 stays true to Clark's architecture:

* DOS first, Linux second
* Binary data formats
* Numeric security levels
* CP437 native
* PPE backward compatibility is non-negotiable

