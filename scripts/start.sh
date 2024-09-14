#!/bin/bash

main(){
    set_icecast_config

    # mpd and icecast2 complain if these directories are not present at startup, this is where they store logs and such
    mkdir /.mpd /.icecast /.icecast/logs

    chmod -R a+xrw /.icecast
    chmod -R a+xrw /.mpd
    chmod -R a+r /metadata

    enabled_stations_to_json

    IFS=', ' read -r -a ENABLED_STATIONS_ARRAY <<< "$ENABLED_STATIONS"
    MPD_CONTROL_PORT=6600

    # This script just launches Icecast using the custom config for the sake of separation between MPD and Icecast2
    icecast2 -c /config/icecast.xml &
    echo "Icecast launched successfully"

    for RADIO_STATION in "${ENABLED_STATIONS_ARRAY[@]}"
    do
        mkdir /.mpd/$RADIO_STATION
        chmod -R a+xrw /.mpd/$RADIO_STATION

        touch /config/$RADIO_STATION.conf
        cat /config/sample.conf >> /config/$RADIO_STATION.conf

        ((MPD_CONTROL_PORT++))
        STATION_NAME="$(python3 /scripts/get_station_metadata.py $RADIO_STATION station)"
        STATION_GENRE="$(python3 /scripts/get_station_metadata.py $RADIO_STATION genre)"
        STATION_FOLDER="$(python3 /scripts/get_station_metadata.py $RADIO_STATION folder)"
        sed -i 's|%RADIO_STATION%|'"$RADIO_STATION"'|g' /config/$RADIO_STATION.conf
        sed -i 's|%STATION_NAME%|'"$STATION_NAME"'|g' /config/$RADIO_STATION.conf
        sed -i 's|%STATION_GENRE%|'"$STATION_GENRE"'|g' /config/$RADIO_STATION.conf
        sed -i 's|%ICECAST_PORT%|'"$ICECAST_PORT"'|g' /config/$RADIO_STATION.conf
        sed -i 's|%MPD_CONTROL_PORT%|'"$MPD_CONTROL_PORT"'|g' /config/$RADIO_STATION.conf
        sed -i 's|%ICECAST_SOURCE_PASS%|'"$ICECAST_SOURCE_PASS"'|g' /config/$RADIO_STATION.conf
        
        mpd /config/$RADIO_STATION.conf
        echo "$STATION_NAME launched successfully"
        
        sleep 4
        echo "python3 /scripts/mpd_control.py "/metadata/$STATION_FOLDER.json" @$RADIO_STATION &"

        python3 /scripts/mpd_control.py "/metadata/$STATION_FOLDER.json" $RADIO_STATION &

    done
}

set_icecast_config(){
    sed -i 's|%HOSTNAME%|'"$HOSTNAME"'|g' /config/icecast.xml
    echo Changed hostname to $HOSTNAME
    sed -i 's|%DISPLAY_ADMIN%|'"$DISPLAY_ADMIN"'|g' /config/icecast.xml
    sed -i 's|%ADMIN_ACC%|'"$ADMIN_ACC"'|g' /config/icecast.xml
    sed -i 's|%ICECAST_PORT%|'"$ICECAST_PORT"'|g' /config/icecast.xml

    # I don't feel like writing a sketchy for loop for the following.
    # Maybe if bash was a proper programming language
    if [ "$ICECAST_SOURCE_PASS" == "" ]
    then
        ICECAST_SOURCE_PASS=$(openssl rand -base64 12)
    fi
    sed -i 's|%ICECAST_SOURCE_PASS%|'"$ICECAST_SOURCE_PASS"'|g' /config/icecast.xml


    if [ "$ICECAST_RELAY_PASS" == "" ]
    then
        ICECAST_RELAY_PASS=$(openssl rand -base64 12)
    fi
    sed -i 's|%ICECAST_RELAY_PASS%|'"$ICECAST_RELAY_PASS"'|g' /config/icecast.xml


    if [ "$ICECAST_ADMIN_PASS" == "" ]
    then
        ICECAST_ADMIN_PASS=$(openssl rand -base64 12)
    fi
    sed -i 's|%ICECAST_ADMIN_PASS%|'"$ICECAST_ADMIN_PASS"'|g' /config/icecast.xml
}

start_svelte(){
    cd /frontend/sveltekit-src/src
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