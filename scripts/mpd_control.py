from mpd import MPDClient
import json, random, sys, time

# This script directly controls the playback of the MPD.
# It accepts CLI arguments /path/to/json, mpd-socket, and mpd-control-port

if len(sys.argv) != 3:
    print("Usage: python mpd_control.py /path/of/json [mpd socket] [mpd control port]")
    sys.exit(1)

radio = open(sys.argv[1])
mpd_socket = (sys.argv[2])
# mpd_control_port = int(sys.argv[3])

client=MPDClient()
client.connect("/run/mpd/"+mpd_socket)

# import json file as python dictionary
data=json.load(radio)
all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"]

# some stations do not have certain types of audio tracks
match mpd_socket:
    case "kjah":
        categories = ["Songs", "DJ", "ID"]
        chances = [1, 1, 1]
    case "radiols":
        categories = ["Songs", "DJ", "ID"]
        chances = [1, 1, 1]
    case "sfur":
        categories = ["Songs", "Caller", "DJ"]
        chances = [1, 0.0625, 1]
    case _:
        categories = ["Songs", "Caller", "DJ", "ID"]
        chances = [1, 0.0625, 1, 1]

for cat in categories:
    random.shuffle(data[cat])

# Define tracker variables
song_has_not_been_played_for=0
last_category=""


def main():
    # Enable consume. This allows songs to be removed from the queue after they are played. Prevents from accumulating backlog of songs
    client.consume(1)
    print("MPD Consume enabled for " + mpd_socket)
    
    while True:
        # Get song queue to be played.
        output=queue()
        for elem in output:
            #Add songs to queue one by one
            full_path=("/music/"+elem)
            print("Adding "+full_path+" to the queue.")
            client.add(full_path)
        
        sleep_for=(remaining_time())
        print("Pressing play.")
        client.play()
        
        #Sleep for the duration of the queue (not including the first track in the queue)
        print("Sleeping for "+str(sleep_for)+" seconds.")
        time.sleep(sleep_for)

def remaining_time():

    #Returns remaining duration of the queue (not including the first track in the queue)
    
    full_queue=client.playlistinfo()
    total_playtime=0
    for track in full_queue[1:]:
        total_playtime+=float(track["duration"])
    return total_playtime

def queue():
    global song_has_not_been_played_for
    global last_category
    while True:

        # Make sure a song is played at least every 3rd time
        if (song_has_not_been_played_for>1):
            selected_category="Songs"
        else:                
            # select a random category, (the list indicates the prob. weights for each category) (random.choices returns a list)
            selected_category = random.choices(categories, chances)
            # convert the list to string
            selected_category = selected_category[0]
        if (last_category):
            if(selected_category==last_category):
                continue
        
        current_track=get_next_track(selected_category)
        
        last_category=(selected_category)
        # song objects have a different structure
        # each song in radio contains at most 3 intros and 3 outros (2 with dj voice lines, 1 without)
        # each song object is a dict containing the following keys
        # title, intros (list), mid (middle part of the song, 1 element), outros (list)
        # intros (and outros) is a list containing a single dict of the following keys
        # dj0:path, dj1:path, dj2:path
        if (selected_category=="Songs"):

            # get (dict==>intros==>0th==>values) conv. to list of all possible intro lines
            # randomly choose a path from the list, assign it to song_intro
            song_intro=random.choice(list(current_track["intros"][0].values()))
            song_mid=current_track["mid"][0]["mid"]

            # same as intros but for outro
            song_outro=random.choice(list(current_track["outros"][0].values()))
            # add paths to output list
            song_has_not_been_played_for=0
            return ([song_intro, song_mid, song_outro])
        else:
            song_has_not_been_played_for+=1
            return ([current_track])
        
def get_next_track(category):
    #get the list of tracks with inside the specified category
    tracks_in_category=data[category]

    # the following lines choose 1st or 2nd track in the list randomly.
    # The chosen track is appended to the back of the list
    # This is to prevent tracks from repeating too soon, while preventing the order of the tracks from repeating as well.
    random_bit=random.randint(0,1)
    chosen_track=(tracks_in_category.pop(random_bit))
    tracks_in_category.append(chosen_track)
    return chosen_track

main()