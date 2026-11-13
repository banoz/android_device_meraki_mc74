#!/usr/bin/env python3
"""Catch the u-boot prompt on serial, run one command line, log the console.
usage: uboot-cmd.py <log> <capture-seconds> <u-boot command line>"""
import os, sys, time, glob, subprocess
T = (sorted(glob.glob('/dev/ttyUSB*')) or ['/dev/ttyUSB0'])[0]
subprocess.run(['stty', '-F', T, '115200', 'raw', '-echo', 'cs8', '-cstopb', '-parenb', '-crtscts', '-ixon', '-ixoff', 'clocal'], check=True)
fd = os.open(T, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
log = open(sys.argv[1], 'ab')
def read_for(t):
    b = b''; end = time.time() + t
    while time.time() < end:
        try:
            x = os.read(fd, 4096); b += x; log.write(x); log.flush()
        except BlockingIOError:
            time.sleep(0.02)
    return b
print('waiting for u-boot', flush=True)
buf = b''
while b'u-boot>' not in buf[-300:]:
    os.write(fd, b' '); buf += read_for(0.05)
print('caught prompt', flush=True)
read_for(1); os.write(fd, b'\x03\r'); read_for(1)
os.write(fd, sys.argv[3].encode() + b'\r')
read_for(int(sys.argv[2]))
print('done', flush=True)
