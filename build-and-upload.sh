#!/bin/bash
set +ex
arduino-cli compile -v --fqbn arduino:avr:nano .
BOARD=$(arduino-cli board list | grep usbserial | awk '{print $1}' );
if [[ "$BOARD" == "" ]]; then
  echo "No board found."
fi
arduino-cli upload -v --fqbn arduino:avr:nano -p "$BOARD" .
