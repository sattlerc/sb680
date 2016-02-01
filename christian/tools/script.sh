#!/bin/bash

# Extract packets with major code '0x02'.
# Creates subdirectory of ./analysis named after <log>.
# Usage: tools/script logs/<log>

INPUT="$1"

mkdir "analysis/$INPUT"
tshark -r "logs/$INPUT" -2R 'usb.data_len == 18 && usb.transfer_type == 0x01 && usb.endpoint_number == 0x81 && usb.capdata[0] == 0x02' -T fields -e usb.capdata >"analysis/$INPUT/cap-data"
cd "analysis/$INPUT"
cat cap-data | sed 's/^02://' | sed 's/:00:00:00:00:00:00:00:00:00:00:00//' >six-bytes
cat six-bytes | sed '$!N; /^\(.*\)\n\1$/!P; D' >six-bytes-unique
cat six-bytes | grep -v c3:00:61:80:00:22 >six-bytes-active
