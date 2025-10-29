#!/bin/bash

# mpd and icecast2 complain if these directories are not present at startup, this is where they store logs and such
mkdir -p /radiosa/socks/nowPlaying /radiosa/.mpd /radiosa/.icecast/logs

chmod -R a+xrw /radiosa/.icecast
chmod -R a+xrw /radiosa/.mpd
chown -R mpd /radiosa/socks  # Assuming MPD runs as the 'mpd' user

export ENABLED_STATIONS=$ENABLED_STATIONS
export HOSTNAME=$HOSTNAME
export DISPLAY_ADMIN=$DISPLAY_ADMIN
export ADMIN_ACC=$ADMIN_ACC
export ICECAST_ADMIN_PASS=$ICECAST_ADMIN_PASS
export ICECAST_RELAY_PASS=$ICECAST_RELAY_PASS
export ICECAST_SOURCE_PASS=$ICECAST_SOURCE_PASS

cd /radiosa/scripts

node /radiosa/scripts/write-enabled-stations-array.mjs
node /radiosa/scripts/update-icecast-config.mjs

# This launches Icecast using the custom config
icecast2 -c /radiosa/config/icecast.xml &
echo "Icecast launched successfully"
echo The following stations are enabled: $ENABLED_STATIONS.

IFS=', ' read -r -a ENABLED_STATIONS_ARRAY <<< "$ENABLED_STATIONS"
for CHANNEL_ALIAS in "${ENABLED_STATIONS_ARRAY[@]}"
do
    mkdir /radiosa/.mpd/$CHANNEL_ALIAS
    chmod -R a+xrw /radiosa/.mpd/$CHANNEL_ALIAS

    CHANNEL_ALIAS=$CHANNEL_ALIAS node /radiosa/scripts/update-mpd-config.mjs

    mpd /radiosa/config/$CHANNEL_ALIAS.conf
    
    CHANNEL_ALIAS=$CHANNEL_ALIAS node /radiosa/scripts/control-mpd-playback.mjs &
done

if [ "$DEV_MODE_ENABLED" = "false" ];
then
node /radiosa/sveltekit/build/index.js;
else
cd /radiosa/sveltekit
npm run dev -- --host;
fi

tail -f /dev/null