#!/usr/bin/env python3
"""rebuild_place.py — place fresh-extracted .RED/PCBDISK sources into target/
per INSTALL.DAT.

Called by rebuild.sh and rebuild.bat. Reads INSTALL.DAT and walks its
@DefineDisk sections in order, resolving each @File directive against the
archive that owns that disk's content:

    Disk 1  →  PCBOARD.RED               (4 main EXEs)
    Disk 2  →  PCBDISK.002                (202 files: DOC, EXEs, etc.)
    Disk 3  →  PCBCFGS.RED + PCBDISK.003  (stubs + big EXEs, ~265 files)
    Disk 4  →  PCBOARD2.RED               (OS/2 tree, 10 files — not in
                                           INSTALL.DAT, placed under PCBOS2/)

Source-key collisions between archives are resolved by @Size when the
directive provides one, else by the disk's declared archive priority.

Usage:  python3 rebuild_place.py <work_dir> <target_dir>
"""
import re, os, shutil, sys

# Archive lookup priority per disk. First archive with a size-matching record wins.
DISK_ARCHIVES = {
    1: ['PCBOARD'],
    2: ['PCBDISK.002'],
    3: ['PCBDISK.003', 'PCBCFGS', 'COMMDRV', 'PCBMAIL', 'PPLC'],
    4: ['PCBOARD2'],
}

KEEP = {'README.md', 'MANIFEST.txt', 'AVAILABLE.md',
        'rebuild.sh', 'rebuild.bat', 'rebuild_place.py'}


def load_sources(work_dir):
    """Build {archive_name: {source_key: (path, size)}} from work_dir/ext/*."""
    sources = {}
    ext_root = os.path.join(work_dir, 'ext')
    if not os.path.isdir(ext_root):
        return sources
    for arch in os.listdir(ext_root):
        d = os.path.join(ext_root, arch)
        if not os.path.isdir(d): continue
        sources[arch] = {}
        for f in os.listdir(d):
            p = os.path.join(d, f)
            if os.path.isfile(p):
                sources[arch][f] = (p, os.path.getsize(p))
    return sources


def resolve(src_key, size, disk_num, sources):
    """Pick the right (archive, path) for a given @File directive."""
    for arch in DISK_ARCHIVES.get(disk_num, []):
        entry = sources.get(arch, {}).get(src_key)
        if entry is None: continue
        path, sz = entry
        if size is None or sz == size:
            return arch, path
    # Fallback: any archive with matching size
    if size is not None:
        for arch, files in sources.items():
            entry = files.get(src_key)
            if entry and entry[1] == size:
                return arch, entry[0]
    return None, None


def wipe_target(target_root):
    """Remove all files under target_root except the tracked docs+scripts."""
    for root, dirs, files in os.walk(target_root):
        for f in files:
            rel = os.path.relpath(os.path.join(root, f), target_root)
            if rel not in KEEP:
                os.remove(os.path.join(root, f))
    for root, dirs, files in os.walk(target_root, topdown=False):
        if root == target_root: continue
        try: os.rmdir(root)
        except OSError: pass


def main():
    if len(sys.argv) != 3:
        print(__doc__); sys.exit(1)
    work_dir = sys.argv[1]
    target_root = sys.argv[2].rstrip(os.sep)

    with open(os.path.join(work_dir, 'install.dat'), 'rb') as f:
        data = f.read().decode('cp437')
    pat = re.compile(r'@File\s+(\S+)(?:\s+@Size\s+(\d+))?\s+@Out\s+(\S+?)(?:\r|\s|@)')

    sources = load_sources(work_dir)
    wipe_target(target_root)

    # Walk INSTALL.DAT one disk at a time (@DefineDisk sections)
    sections = re.split(r'@DefineDisk', data)
    placed = missing = 0
    for disk_num, sec in enumerate(sections[1:], 1):
        for m in pat.finditer(sec):
            src, size, dst = m.group(1), m.group(2), m.group(3)
            size = int(size) if size else None

            if '@' in dst and '*.*' not in dst:
                # Handle @OutDrive:@SubDir\NAME → place at root as source name
                if dst.startswith('@OutDrive'):
                    real_dst = src
                else:
                    missing += 1; continue
            elif '*.*' in dst:
                target_dir = dst.replace('\\*.*','').replace('/*.*','')
                real_dst = os.path.join(target_dir, src)
            else:
                real_dst = dst.replace('\\','/')

            arch, path = resolve(src, size, disk_num, sources)
            if path is None:
                missing += 1
                continue

            full = os.path.join(target_root, real_dst)
            os.makedirs(os.path.dirname(full) or target_root, exist_ok=True)
            if os.path.exists(full):
                base, ext = os.path.splitext(full)
                full = f'{base}.{src}{ext}'
            shutil.copy(path, full)
            placed += 1

    # OS/2 tree (Disk 4 has no @File directives in INSTALL.DAT — place manually)
    os2_files = ['PCBOARD2.EXE', 'PCBCP.EXE', 'PCBMONI2.EXE', 'PCBPACK2.EXE',
                 'PCBTITLE.COM', 'USERNET2.EXE', 'BOARD.CMD', 'STARTOS2.CMD',
                 'SAMPLE.OS2', 'PCBCP.HLP']
    os2_dir = os.path.join(target_root, 'PCBOS2')
    os.makedirs(os2_dir, exist_ok=True)
    for f in os2_files:
        entry = sources.get('PCBOARD2', {}).get(f)
        if entry:
            shutil.copy(entry[0], os.path.join(os2_dir, f))
            placed += 1

    print(f'  Placed:  {placed} files')
    if missing:
        print(f'  Missing: {missing} @File directives could not be resolved')


if __name__ == '__main__':
    main()
