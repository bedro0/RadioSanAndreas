mpd /mpdconfig/radiox.conf
echo "Radio X launched successfully"
# wait for mpd to initialize
sleep 4
# this "consumes" the queue, meaning songs that were already played will be removed
# prevents the queue from accumulating a backlog
mpc consume -p 6601 && echo "MPD Consume enabled"

while true
do

# see upque.sh (update queue) for more info
# this argument passes the location of json metadata file and port number
# mpc (cli client for interacting with mpd uses network ports for controls)
# to have two separate streams, two instances of mpd must be run
# default port is 6600, but I am not using default to avoid any issues
/mpdconfig/upque.sh "/music/Radio X.json" "6601"

# see remaining.sh for more info
radiox_remaining_seconds=$(/mpdconfig/remaining.sh "6601")
radiox_sleep_for=$((radiox_remaining_seconds - 120))
echo "Radio X sleeping for $radiox_sleep_for seconds"

# sleep for the amount of time it takes to play the entire queue 
# (-2 minutes to make sure another queue will be added to keep playback indefinitely)
# 2 minute window does not accumulate on each playback since new queue duration is calculated each time

sleep $radiox_sleep_for
done