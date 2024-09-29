#!/bin/bash

main(){
    set_icecast_config

    # mpd and icecast2 complain if these directories are not present at startup, this is where they store logs and such
    mkdir /radiosa/.mpd /radiosa/.icecast /radiosa/.icecast/logs

    chmod -R a+xrw /radiosa/.icecast
    chmod -R a+xrw /radiosa/.mpd
    chmod -R a+r /radiosa/metadata
    mkdir -p /radiosa/socks/nowPlaying
    chown -R mpd /radiosa/socks  # Assuming MPD runs as the 'mpd' user


    enabled_stations_to_json

    IFS=', ' read -r -a ENABLED_STATIONS_ARRAY <<< "$ENABLED_STATIONS"

    # This launches Icecast using the custom config
    icecast2 -c /radiosa/config/icecast.xml &
    echo "Icecast launched successfully"

    for RADIO_STATION in "${ENABLED_STATIONS_ARRAY[@]}"
    do
        mkdir /radiosa/.mpd/$RADIO_STATION
        chmod -R a+xrw /radiosa/.mpd/$RADIO_STATION

        touch /radiosa/config/$RADIO_STATION.conf
        cat /radiosa/config/sample.conf >> /radiosa/config/$RADIO_STATION.conf

        STATION_NAME="$(node /radiosa/scripts/get-station-metadata.js $RADIO_STATION station_name)"
        STATION_GENRE="$(node /radiosa/scripts/get-station-metadata.js $RADIO_STATION genre)"
        STATION_FOLDER="$(node /radiosa/scripts/get-station-metadata.js $RADIO_STATION folder)"
        sed -i 's|%RADIO_STATION%|'"$RADIO_STATION"'|g' /radiosa/config/$RADIO_STATION.conf
        sed -i 's|%STATION_NAME%|'"$STATION_NAME"'|g' /radiosa/config/$RADIO_STATION.conf
        sed -i 's|%STATION_GENRE%|'"$STATION_GENRE"'|g' /radiosa/config/$RADIO_STATION.conf
        sed -i 's|%ICECAST_PORT%|'"$ICECAST_PORT"'|g' /radiosa/config/$RADIO_STATION.conf
        sed -i 's|%ICECAST_SOURCE_PASS%|'"$ICECAST_SOURCE_PASS"'|g' /radiosa/config/$RADIO_STATION.conf
        
        mpd /radiosa/config/$RADIO_STATION.conf
        echo "$STATION_NAME launched successfully"
        
        sleep 4

        #echo "node /radiosa/scripts/mpd-control.js "/radiosa/metadata/$STATION_FOLDER.json" $RADIO_STATION &"
        node /radiosa/scripts/mpd-control.js "/radiosa/metadata/$STATION_FOLDER.json" $RADIO_STATION &
    done
}

set_icecast_config(){
    sed -i 's|%HOSTNAME%|'"$HOSTNAME"'|g' /radiosa/config/icecast.xml
    echo Changed hostname to $HOSTNAME
    sed -i 's|%DISPLAY_ADMIN%|'"$DISPLAY_ADMIN"'|g' /radiosa/config/icecast.xml
    sed -i 's|%ADMIN_ACC%|'"$ADMIN_ACC"'|g' /radiosa/config/icecast.xml
    sed -i 's|%ICECAST_PORT%|'"$ICECAST_PORT"'|g' /radiosa/config/icecast.xml

    # I don't feel like writing a sketchy for loop for the following.
    # Maybe if bash was a proper programming language
    if [ "$ICECAST_SOURCE_PASS" == "" ]
    then
        ICECAST_SOURCE_PASS=$(openssl rand -base64 12)
    fi
    sed -i 's|%ICECAST_SOURCE_PASS%|'"$ICECAST_SOURCE_PASS"'|g' /radiosa/config/icecast.xml


    if [ "$ICECAST_RELAY_PASS" == "" ]
    then
        ICECAST_RELAY_PASS=$(openssl rand -base64 12)
    fi
    sed -i 's|%ICECAST_RELAY_PASS%|'"$ICECAST_RELAY_PASS"'|g' /radiosa/config/icecast.xml


    if [ "$ICECAST_ADMIN_PASS" == "" ]
    then
        ICECAST_ADMIN_PASS=$(openssl rand -base64 12)
    fi
    sed -i 's|%ICECAST_ADMIN_PASS%|'"$ICECAST_ADMIN_PASS"'|g' /radiosa/config/icecast.xml
}

start_svelte(){
    cd /radiosa/frontend/sveltekit-src/src
    npm run dev -- --host
}

enabled_stations_to_json(){
    # Read the ENABLED_STATIONS environment variable
    stations=$ENABLED_STATIONS

    # Remove any whitespace around commas
    stations=$(echo $stations | sed 's/\s*,\s*/,/g')

    # Split the string into an array
    IFS=',' read -ra station_array <<< "$stations"

    # Create a JSON array
    json_array=$(printf '"%s",' "${station_array[@]}" | sed 's/,$//')
    json_array="[$json_array]"

    # Save the JSON array to a file
    echo $json_array > enabled_stations.json

    echo "JSON array saved to enabled_stations.json"
}

main
start_svelte