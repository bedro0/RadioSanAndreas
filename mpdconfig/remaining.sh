#!/bin/bash

# this script accepts port number as arg to correctly fetch mpc queue
# Get the current queue length
queue_length=$(mpc -p "$1" playlist -f "%position%" | wc -l)

# Initialize total playtime variable
total_playtime=0

# Loop through each song in the queue
for ((i=1; i<=$queue_length; i++)); do
    # Get the playtime of each song and add it to the total playtime
    playtime=$(mpc -p "$1" playlist -f "%time%" | sed -n "${i}p" | awk -F: '{print $1*60 + $2}')
    total_playtime=$((total_playtime + playtime))
done

# Print the total playtime in seconds
echo $total_playtime
