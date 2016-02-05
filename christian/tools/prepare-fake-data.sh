#!/bin/bash

# Extract captures data for simulation into C array entries.
# Usage: <script> <input> <output> <bus>.<device>
tshark -r "$1" -2R '(usb.transfer_type == 0x01) && (usb.src matches "'"$3"'.1")' -T fields -e usb.capdata | sed 's/:/, 0x/g' | sed 's/^/{0x/' | sed 's/$/},/' >"$2"
sed -i 's/{0x},//g' "$2"
