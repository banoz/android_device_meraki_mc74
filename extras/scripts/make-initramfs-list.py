#!/usr/bin/env python3
"""Generate a gen_init_cpio list from the Lineage root/ dir (all files root-owned, no group/world write)."""
import os, stat
R = '/home/user/Documents/MC74/lineage/src/out/target/product/mc74/root'
out = ['dir /dev 0755 0 0', 'nod /dev/console 0600 0 0 c 5 1']
for dp, dn, fn in os.walk(R):
    rel = os.path.relpath(dp, R)
    for name in sorted(dn) + sorted(fn):
        p = os.path.join(dp, name); r = '/' + os.path.normpath(os.path.join(rel, name))
        if r == '/dev': continue
        if os.path.islink(p): out.append('slink %s %s 0777 0 0' % (r, os.readlink(p))); continue
        m = stat.S_IMODE(os.stat(p).st_mode) & 0o7755
        out.append(('dir %s %o 0 0' % (r, m)) if os.path.isdir(p) else ('file %s %s %o 0 0' % (r, p, m)))
open('/home/user/Documents/MC74/lineage/images/android-initramfs.list', 'w').write('\n'.join(out) + '\n')
