#!/bin/bash
# usage: snap.sh <out.png>  -> grabs what the MC74 panel shows (VC snapshot), rotated upright
S=/tmp/claude-1000/-home-user-Documents-MC74/984d9153-0d9a-4e97-b2fe-f2d6a43dd4b4/scratchpad
adb shell 'A=/proc/vc-fb/0/action; rm -f /data/local/tmp/fb-dump; echo snap-take > $A; echo snap-dump > $A; echo snap-term > $A' >/dev/null
adb pull /data/local/tmp/fb-dump $S/fb-dump >/dev/null && python3 -I -c "
from PIL import Image
d=open('$S/fb-dump','rb').read()
Image.frombytes('RGBA',(800,1280),d[:800*1280*4]).convert('RGB').rotate(-90,expand=True).resize((640,400)).save('$1')"
