#!bin/bash

# This script just launches Icecast using the custom config for the sake of separation between MPD and Icecast2
icecast2 -c /iceconfig/icecast.xml &
echo "Icecast launched successfully"