#!/system/bin/sh
# At boot the PC can enumerate the MC74's adb gadget before the adb function
# is ready ("config 1 has no interfaces"); the gadget still reports
# CONFIGURED, so its state can't tell. Re-plug the gadget once after boot,
# when adbd is up, so the PC always sees a complete device.
sleep 5
cfg=$(getprop sys.usb.config)
[ -n "$cfg" ] && [ "$cfg" != none ] || exit 0
log -t mc74-usbcheck "re-plugging the USB gadget once after boot ($cfg)"
setprop sys.usb.config none
sleep 2
setprop sys.usb.config "$cfg"
