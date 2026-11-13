#!/usr/bin/env python3
"""Run shell commands on the MC74 serial console and log the output.
usage: serial-cmds.py <outfile> <cmd> [<cmd>...]   (each command gets up to 20 s)"""
import os, sys, time, glob, subprocess
T = (sorted(glob.glob('/dev/ttyUSB*')) or ['/dev/ttyUSB0'])[0]
subprocess.run(['stty', '-F', T, '115200', 'raw', '-echo', 'cs8', '-cstopb', '-parenb', '-crtscts', 'clocal'])
fd = os.open(T, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
out = open(sys.argv[1], 'w')
def run(c, t=20):
    mark = '__END%d__' % int(time.time()*1000)
    os.write(fd, (c + '; echo ' + mark + '\r').encode())
    buf = b''; end = time.time() + t
    while time.time() < end and mark.encode() + b'\r' not in buf and mark.encode()+b'\n' not in buf:
        try: buf += os.read(fd, 65536)
        except BlockingIOError: time.sleep(0.05)
    out.write('\n### ' + c + '\n' + buf.decode('latin1').replace('\r', ''))
    out.flush()
os.write(fd, b'\r'); time.sleep(1)
for c in sys.argv[2:]: run(c)
