import json, sys

if len(sys.argv) != 3:
    print("Usage: python get_station_metadata.py [Station Alias] [Requested Metadata]")
    sys.exit(1)

station_alias=sys.argv[1]
requestedMetadata=sys.argv[2]
#metadataFile=json.load(config/station_metadata.json)
metadataFile={
    
    "bouncefm":{
        "station":"Bounce FM", 
        "folder":"Bounce FM", 
        "genre":"Funk, Disco, Soul, R&B", 
        "location":"East Los Santos", 
        "location_real":"East Los Angeles, California",
        "host":"The Funktipus"
    },
    "csr":{
        "station":"CSR 103.9",
        "folder":"CSR",
        "genre":"New Jack Swing, '90s Soul, '90s Pop",
        "location":"San Fierro",
        "location_real":"San Francisco, California",
        "host":"Philip 'PM' Michaels"
    },
    "kdst":{
        "station":"K-DST",
        "folder":"K-DST",
        "genre":"Classic Rock",
        "location":"Los Santos",
        "location_real":"Los Angeles, California",
        "host":"Tommy \"The Nightmare\" Smith"
    },
    "kjah":{
        "station":"K-JAH West",
        "folder":"K-JAH",
        "genre":"Reggae, Dub, Dancehall",
        "location":"Las Venturas",
        "location_real":"Paradise, Nevada",
        "host":"Marshall Peters, Johnny Lawton"
    },
    "krose":{
        "station":"K-Rose",
        "folder":"KRose",
        "genre":"Country",
        "location":"Bone County",
        "location_real":"Lincoln County, Nevada",
        "host":"Mary-Beth Maybell"
    },
    "mastersounds":{
        "station":"Master Sounds 98.3",
        "folder":"Master Sounds",
        "genre":"Groove, Classic Funk, Classic Soul",
        "location":"Blackfield, Las Venturas",
        "location_real":"Paradise, Nevada",
        "host":"Johnny \"The Love Giant\" Parkinson"
    },
    "playbackfm":{
        "station":"Playback FM",
        "folder":"Playback FM",
        "genre":"Classic East Coast Hip-Hop",
        "location":"San Fierro",
        "location_real":"San Francisco, California",
        "host":"Forth Right MC"
    },
    "radiols":{
        "station":"",
        "folder":"Radio Los Santos",
        "genre":"'90s Hip-Hop",
        "location":"Downtown Lost Santos",
        "location_real":"Downtown Los Angeles, California",
        "host":"Julio G"
    },
    "radiox":{
        "station":"Radio:X",
        "folder":"Radio X",
        "genre":"Alternative Rock, Grunge",
        "location":"Las Venturas",
        "location_real":"Las Vegas, Nevada",
        "host":"Sage"
    },
    "sfur":{
        "station":"San Fierro Underground Radio",
        "folder":"SFUR",
        "genre":"House",
        "location":"San Fierro",
        "location_real":"San Francisco, California",
        "host":"Hans Oberlander"
    },
    "wctr":{
        "station":"West Coast Talk Radio",
        "folder":"WCTR",
        "genre":"Talk Radio Station",
        "location":"Missionary Hill, San Fierro",
        "location_real":"Sutro Tower, San Francisco, California",
        "host":"Ammu-Nation"
    }
}

def main():
    print(metadataFile[station_alias][requestedMetadata])

main()