#!/bin/bash
# fpc264irc i386-os2 native (-Tos2) unit rebuild — byte session 2026-10-01
R=/home/claude/fpc264irc
S=$R/src
PPC=$R/bin/ppc386
OUT=/home/claude/os2build/units
mkdir -p $OUT
INC="-Fi$S/rtl/inc -Fi$S/rtl/i386 -Fi$S/rtl/objpas -Fi$S/rtl/objpas/sysutils -Fi$S/rtl/objpas/classes -Fi$S/rtl/os2"
BASE="-Tos2 -Pi386 -s -O2 -Ur -n -FU$OUT -Fu$OUT $INC"
c() { # compile one source, run in its dir
  local f=$1; shift
  ( cd $(dirname $f) && $PPC $BASE "$@" $(basename $f) ) > $OUT/../log/$(basename $f).log 2>&1
  local rc=$?; echo "$rc $(basename $f)"; return $rc
}
mkdir -p log
c $S/rtl/os2/system.pas -Us
