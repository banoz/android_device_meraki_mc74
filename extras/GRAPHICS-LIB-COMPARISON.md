# MC74 vs I9082 graphics libraries (step 1 of the Lineage 14.1 port)

Sources compared (2026-11-11). Update: the test unit runs build phone-58 (Jun 2016); its libGLES_vc4/vchiq/vcos/vcsm/WFC/mmal are byte-identical to the device2 copies, gralloc/hwcomposer are rebuilds with identical imports. Use `lineage/unit-system/` as the blob source. Its VC firmware is the same May 20 2015 build (73 bytes differ).

Sources compared (2026-11-11):
- MC74: `reviveMC74-device2-backup/system2.img` (capri_me1 4.2.3-meraki, JDQ39, built May 2016), extracted to `lineage/mc74-system/`.
- I9082: `k2wl/android_vendor_samsung_i9082` (CM12 era) and `Osas514/proprietary_vendor_samsung_i9082` (used by the Lineage 14.1 build). Their `libGLES_vc4.so` is byte-identical.
- I9082 Lineage 14.1 device tree: `Osas514/android_device_samsung_i9082` branch cm-14.1; kernel `Osas514/android_kernel_samsung_i9082` android-7.1 (3.0.101; same as `FraTeG/i9082_kernel` nougat); framework patches `Osas514/build_tools` cm-14.1 (`patches/`).

## Result: same design, MC74's own libs are usable with the I9082 recipe

| Check | Result |
|---|---|
| Architecture | Both are the VideoCore IV "RPC" stack: `libGLES_vc4.so` → `libvchiq`/`libvcos`/`libWFC`, `gralloc.capri`, `hwcomposer.capri`, MMAL. Same file set as the I9082 `proprietary-files.txt`. |
| Build dates | MC74 libs 2014–2016 (VC firmware May 2015, `capri_me1`); I9082 libs 2013. |
| Imports from Android libs | `libGLES_vc4`, `gralloc`, `hwcomposer`, `libvchiq`, `libvcsm`, `libWFC`, `libmmal_vc` import **exactly** the same symbols as the I9082 versions, so the I9082 14.1 compat shims (`REFBASE_JB_MR1_COMPAT_SYMBOLS`, `NEEDS_VECTORIMPL_SYMBOLS`, `CAPRI_HWC`, the frameworks_native/base patches) cover them. |
| `/dev/vchiq` ioctl ABI | Identical ioctl numbers and struct sizes in MC74 and I9082 `libvchiq.so`, and they match the 3.0.101 nougat kernel (bulk ioctls are `_IOWR`). Note: the Roku 3.0.82 tree uses `_IOW` for bulk — **do not take vchiq from Roku**. Kernel is VCHIQ v6 (min 3), libs fine. |
| `/dev/vcsm` ioctl ABI | Same 17 ioctls; MC74 lib adds one (`_IOR('I',0x6a,16)`, `vcsm_vc_ptr_from_hdl`) that **no MC74 library calls**. Can add to the kernel later if needed (RPi `MAPPED_VC_ADDR_FROM_HDL`). |
| Device nodes | MC74 graphics use `/dev/vchiq`, `/dev/vcsm`, `/dev/vc-dnfo`, `/dev/vc-lmk`; all exist in the nougat kernel. |
| Mixing | Don't mix sets: MC74 `libmmal` needs `vcos_timer_*_pthreads` that only MC74 `libvcos` has. Use the MC74 set as a whole. |

## Gaps to fill
1. MC74 `libvcos.so` NEEDs `libcorkscrew.so` (gone in Android 7) for `unwind_backtrace`, `get_backtrace_symbols`, `format_backtrace_line`, `free_backtrace_symbols` → ship a stub `libcorkscrew.so` shim.
2. hwcomposer: MC74 build lacks two internal exports present in I9082's (`BrcmComposer2::handle_SET_msg`, `BrcmExternalDisplayControl::query_mode`); internal only, no external users expected.
3. VC firmware stays the MC74's own (`vc-firmware` partition, untouched).

## Board differences that matter
- RAM: MC74 has 512 MB (`mem=510M`), I9082 has 1 GB (`mem=832M@0xA2000000 vc-cma-mem=0/176M@0xCB000000`). Needs `ro.config.low_ram`, smaller VC CMA carve-out, zram.
- No modem, no battery, Ethernet instead of Wi-Fi as primary; RIL, telephony patches can be dropped.

## Host build constraints (this PC)
4 cores, 7 GB RAM, 2 GB swap, 129 GB free; no docker, no sudo from Claude's shell, no python2, no JDK 8.
