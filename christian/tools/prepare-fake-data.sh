#!/bin/bash

# Extract captures data for simulation into C array entries.
# Usage: <script> <input> <output> <bus>.<device>
tshark -r "$1" -2R '(usb.transfer_type == 0x01) && (usb.endpoint_number == 0x82) && (usb.src matches "'"$3"'.2")' -T fields -e usb.capdata | sed 's/:/, 0x/g' | sed 's/^/{0x/' | sed 's/$/},/' >"$2"
