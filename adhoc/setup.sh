#!/bin/bash
OUTPUT=$(./print)
if (($? != 0)); then
    exit 1
fi
DEVICE=$(echo -en "$OUTPUT" | head -n1 | tr -d "\n")
NAME=$(echo -en "$OUTPUT" | tail -n+2 | head | tr -d "\n")
echo "device: $DEVICE"
echo "name: $NAME"
xinput disable "$NAME"
sudo chown $(whoami) "$DEVICE"
