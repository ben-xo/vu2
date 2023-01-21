#!/bin/bash
set +exuo pipefail

CLEAN=
if [[ "$1" == "--clean" ]]; then
  # use this option if you switch avr-gcc versions with e.g. brew unlink avr-gcc@8 && brew link --force avr-gcc@11
  # note that you'd have to set compiler.path=/opt/homebrew/bin/ in platform.txt (see output of arduino-cli for the location)
  CLEAN="--clean"
fi

arduino-cli compile -v $CLEAN --fqbn arduino:avr:nano .
if [[ "$BOARD" == "" ]]; then
  BOARD=$(arduino-cli board list | grep usbserial | awk '{print $1}' );
  if [[ "$BOARD" == "" ]]; then
    echo "No board found."
  fi
fi
arduino-cli upload -v --fqbn arduino:avr:nano -p "$BOARD" .
