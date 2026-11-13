#!/bin/bash
# runs inside the container
set -e
cd /src
export USE_CCACHE=0 LC_ALL=C
export ANDROID_JACK_VM_ARGS="-Dfile.encoding=UTF-8 -XX:+TieredCompilation -Xmx6g"
export JACK_SERVER_VM_ARGUMENTS="$ANDROID_JACK_VM_ARGS"
source build/envsetup.sh
lunch lineage_mc74-userdebug
make -j${JOBS:-3} ${TARGET:-bacon}
