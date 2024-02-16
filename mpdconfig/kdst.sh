mpd /mpdconfig/kdst.conf
echo "K-DST launched successfully"
mpc consume -p 6602 && echo "MPD Consume enabled"

while true
do
/mpdconfig/upque.sh "/music/K-DST.json" "6602"
kdst_remaining_seconds=$(/mpdconfig/remaining.sh "6602")
kdst_sleep_for=$((kdst_remaining_seconds - 120))
echo "K-DST sleeping for $kdst_sleep_for seconds"
sleep $kdst_sleep_for
done