#!/bin/bash

# mpd and icecast2 complain if these directories are not present at startup, this is where they store logs and such
mkdir -p /radiosa/socks/nowPlaying /radiosa/.mpd /radiosa/.icecast/logs

chmod -R a+xrw /radiosa/.icecast
chmod -R a+xrw /radiosa/.mpd
chmod -R a+r /radiosa/metadata
chown -R mpd /radiosa/socks  # Assuming MPD runs as the 'mpd' user

main(){
    cd /radiosa/scripts
    ENABLED_STATIONS="$ENABLED_STATIONS" node /radiosa/scripts/split-enabled-stations.js

    HOSTNAME=$HOSTNAME \
    DISPLAY_ADMIN=$DISPLAY_ADMIN \
    ADMIN_ACC=$ADMIN_ACC \
    ICECAST_SOURCE_PASS=$ICECAST_SOURCE_PASS \
    ICECAST_RELAY_PASS=$ICECAST_RELAY_PASS \
    ICECAST_ADMIN_PASS=$ICECAST_ADMIN_PASS \
    node /radiosa/scripts/update-icecast-config.js

    # This launches Icecast using the custom config
    icecast2 -c /radiosa/config/icecast.xml &
    echo "Icecast launched successfully"

    IFS=', ' read -r -a ENABLED_STATIONS_ARRAY <<< "$ENABLED_STATIONS"
    for CHANNEL_ALIAS in "${ENABLED_STATIONS_ARRAY[@]}"
    do
        mkdir /radiosa/.mpd/$CHANNEL_ALIAS
        chmod -R a+xrw /radiosa/.mpd/$CHANNEL_ALIAS

        CHANNEL_ALIAS=$CHANNEL_ALIAS node /radiosa/scripts/update-mpd-config.js

        mpd /radiosa/config/$CHANNEL_ALIAS.conf
        if [[ "$CHANNEL_ALIAS" == "wctr" ]]
        then
            node /radiosa/scripts/talk-radio.js &
        else
            CHANNEL_ALIAS=$CHANNEL_ALIAS node /radiosa/scripts/mpd-control.js &
        fi
    done

    node /radiosa/sveltekit/build/index.js
}

main

tail -f /dev/null