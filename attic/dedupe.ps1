# ============================================================
#  attic\dedupe.ps1                            2026-09-23
#
#  Pass 3 of CLEANUP.BAT.  Not meant to be run directly:
#      CLEANUP DEDUPE      do it
#      CLEANUP DRYRUN      say what it would do
#
#  Reads attic\DEDUPE.LST, one  keeper|victim  per line, both
#  folders relative to the repo root.
#
#  Deletes the victim folder ONLY if:
#    * every file under it has a byte-identical twin under the
#      keeper, compared by SHA-256; and
#    * it holds no file the keeper lacks.
#
#  All or none.  One mismatch and the folder is kept whole, with
#  the offending files named.  A file the keeper has and the
#  victim does not is fine - the keeper is allowed to be ahead.
#
#  del has no Recycle Bin.  That is why this hashes rather than
#  trusting name, size or date.
# ============================================================

param([string]$Repo, [string]$Dry = "0")

$list = Join-Path $Repo 'attic\DEDUPE.LST'
if (-not (Test-Path -LiteralPath $list)) {
  Write-Host '  attic\DEDUPE.LST not found.'
  exit 0
}

function Get-Tree($root) {
  $h = @{}
  if (-not (Test-Path -LiteralPath $root)) { return $null }
  Get-ChildItem -LiteralPath $root -Recurse -File | ForEach-Object {
    $rel = $_.FullName.Substring($root.Length).TrimStart('\')
    $h[$rel] = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash
  }
  return $h
}

$gone = 0; $held = 0; $absent = 0

foreach ($line in Get-Content -LiteralPath $list) {
  if ($line -match '^\s*;' -or $line -notmatch '\S') { continue }
  $parts = $line -split '\|', 2
  if ($parts.Count -lt 2) { continue }
  $keepRel = $parts[0].Trim()
  $victRel = $parts[1].Trim()
  if (-not $victRel) { continue }

  $keep = Join-Path $Repo $keepRel
  $vict = Join-Path $Repo $victRel

  if (-not (Test-Path -LiteralPath $vict)) {
    Write-Host ('  gone     ' + $victRel)
    $absent++
    continue
  }
  if (-not (Test-Path -LiteralPath $keep)) {
    Write-Host ('  HELD     ' + $victRel)
    Write-Host ('           keeper missing: ' + $keepRel)
    $held++
    continue
  }

  $k = Get-Tree $keep
  $v = Get-Tree $vict

  $onlyVictim = @($v.Keys | Where-Object { -not $k.ContainsKey($_) })
  $differing  = @($v.Keys | Where-Object { $k.ContainsKey($_) -and $k[$_] -ne $v[$_] })

  if ($onlyVictim.Count -or $differing.Count) {
    Write-Host ('  HELD     ' + $victRel + '   [' + $v.Count + ' files]')
    foreach ($f in $onlyVictim | Select-Object -First 10) {
      Write-Host ('           only in victim : ' + $f)
    }
    foreach ($f in $differing | Select-Object -First 10) {
      Write-Host ('           differs        : ' + $f)
    }
    if (($onlyVictim.Count + $differing.Count) -gt 20) {
      Write-Host ('           ... and ' + (($onlyVictim.Count + $differing.Count) - 20) + ' more')
    }
    Write-Host '           nothing deleted - it is all or none'
    $held++
    continue
  }

  if ($Dry -eq '1') {
    Write-Host ('  WOULD    ' + $victRel + '   [' + $v.Count + ' files, all matched]')
    Write-Host ('           keeper: ' + $keepRel)
    $gone++
    continue
  }

  Remove-Item -LiteralPath $vict -Recurse -Force -ErrorAction SilentlyContinue
  if (Test-Path -LiteralPath $vict) {
    Write-Host ('  FAILED   ' + $victRel + '   [delete failed - file in use?]')
    $held++
  } else {
    Write-Host ('  deleted  ' + $victRel + '   [' + $v.Count + ' files, all matched]')
    Write-Host ('           kept:   ' + $keepRel)
    $gone++
  }
}

Write-Host ''
if ($Dry -eq '1') {
  Write-Host ('  would delete   : ' + $gone)
} else {
  Write-Host ('  deleted        : ' + $gone)
}
Write-Host ('  held back      : ' + $held)
Write-Host ('  already gone   : ' + $absent)
