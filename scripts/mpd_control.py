from mpd import MPDClient
import json, random, sys, time

# This script directly controls the playback of the MPD.
# It accepts CLI arguments /path/to/json, mpd-socket, and mpd-control-port

if len(sys.argv) != 4:
    print("Usage: python mpd_control.py /path/of/json [mpd socket] [mpd control port]")
    sys.exit(1)

radio = open(sys.argv[1])
mpd_socket = (sys.argv[2])
mpd_control_port = int(sys.argv[3])

client=MPDClient()
client.connect("@"+mpd_socket, mpd_control_port)

# import json file as python dictionary
data=json.load(radio)
all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"]
categories = ["Songs", "Caller", "DJ", "ID"]

def main():
    # Enable consume. This allows songs to be removed from the queue after they are played. Prevents from accumulating backlog of songs
    client.consume(1)
    print("MPD Consume enabled for " + mpd_socket)
    
    while True:
        # Get song queue to be played.
        output=queuev1()
        for elem in output:
            #Add songs to queue one by one
            full_path=("/music/"+elem)
            print("Adding "+full_path+" to the queue.")
            client.add(full_path)
        
        sleep_for=int(remaining_time()-30)
        print("Pressing play.")
        client.play()
        
        #Sleep for the duration of the queue minus 30 seconds
        print("Sleeping for "+str(sleep_for)+" seconds.")
        time.sleep(sleep_for)

def remaining_time():

    #Returns remaining time for the entire queue by adding up durations of each track
    
    full_queue=client.playlistinfo()
    total_playtime=0
    for track in full_queue:
        total_playtime+=float(track["duration"])
    return total_playtime

def queuev1():

    # create empty lists too keep track of audio tracks and categories
    output=[]
    cats_used=[]

    # iterate through the keys inside the dictionary and shuffle the lists for each category
    for cat in categories:
        random.shuffle(data[cat])
    while True:

        # this line ensures that when there are no remaining songs in the dictionary, fetch is ended
        # makes sure bunch of transitional voice lines are not queued back to back
        # please refer to get_random_track() to see why this line is used

        if ("Songs" not in categories):
            break
        # select a random category, (the list indicates the prob. weights for each category) (random.choices returns a list)
        selected_category = random.choices(categories, [1, 0.0625, 1, 1])

        # convert the list to string
        selected_category = selected_category[0]
        # selected_category=random.choice(categories)

        # ensure current category and the previous one are not both not-songs
        # ensures that two transitions don't play back to back

        if (cats_used):
            if (selected_category != "Songs")and(cats_used[-1] != "Songs"):
                continue
        current_track=get_random_track(selected_category)

        # please refer to get_random_track() to see why this is used
        if current_track==None:
            break
        
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
            output.extend([song_intro, song_mid, song_outro])
        else:
            output.append(current_track)
        cats_used.append(selected_category)

    # print each element at a time to output in bash mapfile command
    return output

def get_random_track(category):
    # set a variable to a list inside the dictionary (note that the data is shuffled inside them)
    random_category=data[category]

    # if the list is exhausted remove the category from the list, so it's never picked again (within current iteration)
    if len(random_category)<1:
        categories.remove(category)

        # return none to indicate nothing was chosen
        # this will start the while loop over in main with no consequences
        return None
    
    # pop an element out of the list (ensures that duplicate audio tracks are not played)
    # this is the reason for checks ensuring lists are not emptys
    return (random_category.pop())

main()