#!/bin/bash

# See launchicecast.sh
/iceconfig/launchicecast.sh

/mpdconfig/radiox.sh &
/mpdconfig/kdst.sh &

# This keeps the docker container up
tail -f /dev/null