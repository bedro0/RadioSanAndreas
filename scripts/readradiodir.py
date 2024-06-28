import json, os, sys, mutagen, re, random
radio={"Songs": [], "Caller":[], "DJ": [], "ID": [], "Weather": [], "Time of Day": []}

if len(sys.argv) != 3:
    print("Usage: python readradiodir.py /path/to/read /path/to/write")
    sys.exit(1)

# set the path of read directory to a variable
readFrom = sys.argv[1]
# set the path of json to write to
writeTo = sys.argv[2]

def main():
    
    # assign all filenames in a list variable
    fileslist=os.listdir(readFrom)

    for filename in fileslist:

        # get .ogg tags from the audio file
        tracktags=mutagen.File(os.path.join(readFrom, filename))

        # passes title of the audio file instead of filename to getCategory. It makes regex simpler later
        # assign returned list to categories
        categories=getCategory(tracktags["title"][0])
        if (not categories):
            continue

        fullpath=os.path.join(os.path.basename(os.path.normpath(readFrom)), filename)
        addtoDict (*categories, fullpath, tracktags["title"][0])
    dumptoJSON()

# returns category of the audio file, and boolean whether audio file is a part of a song
def getCategory(filename):
    # files that are not song parts have category in parentheses at the beginnging, this regex checks for that
    non_song = re.compile(r'^\(+(.*?)\)+')
    # files that are song parts, have the part type at the end in parentheses, this regex checks for that
    song_part = re.compile(r'\(+(.*?)\)+$')

    # match regex
    non_song_match=non_song.findall(filename)
    song_part_match=song_part.findall(filename)
    
    # This if Checks whether non_song_match list variable has any elements in it
    # returns true if audio is not a song
    if non_song_match:
        
        #Checks if the audio file is any of these type
        if non_song_match[0] in ["Caller", "DJ", "ID"]:
            
            # return the type of audio, and bool to indicate whether it's a part of song
            return [non_song_match[0], False]
        elif non_song_match[0] == "Atmosphere":
            
            #classifying the difference between voicelines announcing ToD and weather
            #will help to create more dynamic playback later
            if any(timeofday in filename for timeofday in ["Morning", "Evening", "Night"]):
                return ["Time of Day", False]
            else: return ["Weather", False]
    elif song_part_match:
        return [song_part_match[0], True]
 
# adds the audio track object in the dictionary
def addtoDict(category, isSong, fullpath, title):
    # if file is not a song, add path to a list of respective category
    if not isSong:
        radio[category].append(fullpath)
    else:
        # keep anything before (space-open parenthesis) symbol, get rid of the rest to get song title
        retitle = re.compile(r'^(.*?)\s*\(')
        song_title=retitle.findall(title)[0]
        
        # depending on what kind of audio file it is, pass respective args to songsAdd
        match category:
            case "Intro DJ #1":
                songsAdd(song_title, "intros", "dj1", fullpath)

            case "Intro DJ #2":
                songsAdd(song_title, "intros", "dj2", fullpath)

            case "Intro":
                songsAdd(song_title, "intros", "dj0", fullpath)

            case "Mid":
                songsAdd(song_title, "mid", "mid", fullpath)

            case "Outro DJ #1":
                songsAdd(song_title, "outros", "dj1", fullpath)

            case "Outro DJ #2":
                songsAdd(song_title, "outros", "dj2", fullpath)

            case "Outro":
                songsAdd(song_title, "outros", "dj0", fullpath)

def songsAdd(title, part, dj, path):
    for songobj in radio["Songs"]:
        # check if any other part of current song was already processed
        if title in songobj.values():
            # returns "intros" (or "outros") list if it already exists. If not, creates empty list inside dict
            intro_or_outro=songobj.setdefault(part, [])
            if intro_or_outro:
                # if empty list is not returned, update dict inside list
                intro_or_outro[0].update({dj: path})
            else:
                # if empty list is returned, create dict inside the list
                intro_or_outro.append({dj: path})
            return
    # if song has not been processed at all, create an entry with respective title, and song part
    radio["Songs"].append({"title": title, part:[{dj: path}]})

# dumps python dict to json entry and then file
# json file dir can be specified as commandline argument
def dumptoJSON():
    filename=os.path.basename(os.path.normpath(readFrom))
    filePath=os.path.join(writeTo, filename)
    with open((filePath+".json"), "w") as fileVar:
        json.dump(radio, fileVar, indent=4)

main()