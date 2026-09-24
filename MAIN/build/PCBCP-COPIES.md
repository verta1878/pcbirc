# PCBCP — four copies, which to keep

Found 2026-09-22 while chasing `\toolkt21`. Nothing has been moved or
deleted; this is the decision, written down first.

## What is there

| Path | Files | What it is |
|---|---:|---|
| `reference/pcball/pcboard/pcb-util/PCBCP` | 40 | Clark's original, as received |
| `pcb1541/pcbcp` | 84 | the original again, + 2 new files, + a nested copy |
| `pcb1541/pcbcp/PCBCP` | 42 | the OpenWatcom port |
| `drivers/PCBCP` | 42 | byte-identical to the port |

`pcb1541/pcbcp`'s own 40 files are identical to the reference copy. It
adds `BUILD_OW.SH` and `README.md`, and then contains the whole ported
tree *nested inside itself* as `PCBCP/`. That nesting is the confusing
part — the same program appears twice at two depths of one folder, one
version un-ported and one ported.

`drivers/PCBCP` and `pcb1541/pcbcp/PCBCP` are byte-identical: `diff -rq`
reports nothing at all.

## What the port actually changed

Not just the `\toolkt21` include. Comparing `pcb1541/pcbcp/SOURCE`
against `pcb1541/pcbcp/PCBCP/SOURCE`:

* **Nine `.C` files differ** — `DLG FILE HELP INIT MAIN PCBCP.DEF PNT
  THRD USER`.
* **Four headers were renamed to lowercase** — `DLG.H`→`dlg.h`,
  `HELP.H`→`help.h`, `MAIN.H`→`main.h`, `XTRN.H`→`xtrn.h`. That matters
  on a case-sensitive filesystem, which is where the OpenWatcom build
  runs.
* **One header is new** — `pcbcp_compat.h`.
* **`G.CMD` was dropped.** It is in the reference copy and in
  `pcb1541/pcbcp`, and not in either ported tree.

The six documented port fixes are in `README.md`; the header renames and
the `G.CMD` drop are not among them.

## Decision

**KEEP** `reference/pcball/pcboard/pcb-util/PCBCP` — untouched. It
arrived inside the pcball archive and that is its provenance. Reference
is for reference.

**KEEP** `pcb1541/pcbcp` as the working tree, with the contents of
`PCBCP/` flattened up one level to replace the un-ported files. **Carry
`G.CMD` forward** — it is only in the un-ported copies and flattening
would otherwise delete it silently.

**RETIRE** `drivers/PCBCP`. Two reasons, either sufficient:

1. It is a byte-for-byte duplicate of the tree that will live at
   `pcb1541/pcbcp`.
2. `drivers/` holds actual drivers — `fossil98vxd` and `wnfossil` (VxD
   assembly), `netfosdl` (Pascal FOSSIL), `SIO`. PCBCP is a Presentation
   Manager control-panel application. It was never a driver.

## Do not do this as one move

The flatten replaces nine `.C` files and renames four headers. Run it as
a `keeper|victim` pass with `CLEANUP DRYRUN` first, and check `G.CMD`
survived before deleting anything. A copy that exists four times is not
an emergency; losing the only un-ported source would be.

## The `\toolkt21` question this came out of

`pcb1541/pcbcp/SOURCE/MAIN.C:24` still carries Clark's original:

    #include <\toolkt21\c\os2h\valapi.h>

and `1522/PCBCP.MAK:111` links `d:\toolkt21\os2lib\validatr.lib`.

`\c\os2h\` for headers plus `\os2lib\` for libraries is the **IBM
Developer's Toolkit for OS/2 2.1** layout, and `\TOOLKT21` was its
default install path. It is not PCBoard's own "Doors Developer's
TOOLKIT" — that is `devtools/TOOLKIT2.ZIP`, a DOS door SDK, and the
name collision is a coincidence.

Neither `valapi.h` nor `validatr.lib` is anywhere in this repo,
including inside the `reference/` archives. But the ported copy does not
need them: `PCBCP/SOURCE/MAIN.C:25` comments the include out —
"OS/2 validation API, not needed for port" — and links `os2386.lib` from
OS/2 Toolkit 4.5 instead.

So `BLDOS2.CMD`'s note that PCBCP "wants the IBM OS/2 Developer's
Toolkit 2.1" describes the un-ported copy only, and is stale for the one
we build. **OS/2 is not blocked on TOOLKT21.** It is blocked on
`b4.lib`, which `PCBOARD2.MAK` links unconditionally.

If TOOLKT21 is ever acquired, it does **not** go under `toolkit/`. That
folder means PCBoard door-toolkit branches — `pwa153`, `delta154`,
`irc1541`, `pplc`, `pwa154` — and filing an IBM OS/2 SDK beside them
recreates exactly the name collision that sent this investigation the
wrong way for an hour. It is a vendor toolchain: archive in `devtools/`,
extracted to `BUILDROOT/TOOLKT21/` alongside `BC31`, `BCOS2`, `MSC70`
and `TC201`, and reached as `\TOOLKT21` on the build drive — which is
the path Clark's makefiles already expect.
