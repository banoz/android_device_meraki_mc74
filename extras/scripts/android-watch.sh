#!/bin/bash
# usage: android-watch.sh <kernel-name> <seconds> -> RAM-boot, stream kmsg + logcat to <name>-kmsg.txt / <name>-live-logcat.txt
cd /home/user/Documents/MC74/lineage
timeout 500 ./run-test.sh $1 60 >/dev/null
timeout 240 bash -c 'until adb devices | grep -qw device; do sleep 2; done' || { echo "no adb"; exit 1; }
adb shell 'echo 0 > /sys/module/mc74_stubs/parameters/rebootafter'
(adb shell cat /proc/kmsg > $1-kmsg.txt 2>&1 &)
(adb logcat -b all -v time > $1-live-logcat.txt 2>&1 &)
for i in $(seq 1 $(( ${2:-600} / 10 ))); do
  adb get-state >/dev/null 2>&1 || { echo "$(date +%T) adb gone"; break; }
  [ $((i % 6)) = 0 ] && echo "$(date +%T) boot_completed=$(adb shell getprop sys.boot_completed | tr -d '\r') ss=$(adb shell pidof system_server | tr -d '\r')"
  sleep 10
done
