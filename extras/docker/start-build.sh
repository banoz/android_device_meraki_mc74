#!/bin/bash
# usage: start-build.sh [targets] [jobs] -> builds in container mc74build, log docker/android-build.log
docker rm -f mc74build >/dev/null 2>&1
docker run -d --name mc74build -v /home/user/Documents/MC74/lineage/src:/src -v /home/user/Documents/MC74/lineage/docker:/docker \
  -e TARGET="${1:-bootimage systemimage}" -e JOBS="${2:-3}" mc74-lineage-build bash -c "/docker/build.sh > /docker/android-build.log 2>&1"
