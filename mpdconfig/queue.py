import json, random, sys

# this script outputs paths to San Andreas audio files, given the JSON metadata of those files
# refer to readradiodir.py (read radio directory script) for more info

if len(sys.argv) != 2:
    print("Usage: python queue.py /path/of/json")
    sys.exit(1)

radio = open(sys.argv[1])
# import json file as python dictionary
data=json.load(radio)
categories = ["Songs", "Caller", "DJ", "ID"]

def main():

    # create empty lists too keep track of audio tracks and categories
    output=[]
    cats_used=[]

    # iterate through the keys inside the dictionary and shuffle the lists for each category
    for cat in categories:
        random.shuffle(data[cat])
    while True:

        # this line ensures that when there are no remaining songs in the dictionary, fetch is ended
        # makes sure bunch of transitional voice lines are not queued back to back
        # please refer to getRandomTrack() to see why this line is used

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
        currentTrack=getRandomTrack(selected_category)

        # please refer to getRandomTrack() to see why this is used
        if currentTrack==None:
            break
        
        # song objects have a different structure
        # each song in radio contains at most 3 intros and 3 outros (2 with dj voice lines, 1 without)
        # each song object is a dict containing the following keys
        # title, intros (list), mid (middle part of the song, 1 element), outros (list)
        # intros (and outros) is a list containing a single dict of the following keys
        # dj0:path, dj1:path, dj2:path
        if (selected_category=="Songs"):

            # get (dict==>intros==>0th==>values) conv. to list of all possible intro lines
            # randomly choose a path from the list, assign it to songIntro
            songIntro=random.choice(list(currentTrack["intros"][0].values()))
            songMid=currentTrack["mid"][0]["mid"]

            # same as intros but for outro
            songOutro=random.choice(list(currentTrack["outros"][0].values()))
            # add paths to output list
            output.extend([songIntro, songMid, songOutro])
        else:
            output.append(currentTrack)
        cats_used.append(selected_category)

    # print each element at a time to output in bash mapfile command
    for elem in output:
        print (elem)

def getRandomTrack(category):
    # set a variable to a list inside the dictionary (note that the data is shuffled inside them)
    randomCategory=data[category]

    # if the list is exhausted remove the category from the list, so it's never picked again (within current iteration)
    if len(randomCategory)<1:
        categories.remove(category)

        # return none to indicate nothing was chosen
        # this will start the while loop over in main with no consequences
        return None
    
    # pop an element out of the list (ensures that duplicate audio tracks are not played)
    # this is the reason for checks ensuring lists are not emptys
    return (randomCategory.pop())

main()