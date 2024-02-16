#!/bin/bash

# Updates MPD queue
# This script accepts json file as arg1 ($1) and MPC port as arg2 ($2)
# Save the script output in a BASH array
mapfile -t OUTPUT < <(python3 /mpdconfig/queue.py "$1")
# Iterate through the array elements one by one
for PART in "${OUTPUT[@]}"
do
    echo $PART
    # Adds the specific audio file to MPD queue
    mpc -p "$2" add "$PART"
done

# sending play command does nothing if music is already playing
mpc -p "$2" play
