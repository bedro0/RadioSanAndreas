// import libraries
const Chance = require("chance");

// This script directly controls the playback of MPD.
// It accepts CLI arguments /path/to/json, mpd-socket, and mpd-control-port

// check if the incoming command was executed properly

// import the JSON path specified as a CLI argument
const tracksMetadata = require("../metadata/Radio X.json");
// Local sockets are defined by the name of their respective stations
const stationAlias = "radiox";
const stationMetadata = (require("./station-metadata.json"))[stationAlias]


let categories, weights;
let songHasNotBeenPlayedFor = 0;
let lastCategory = "";

const chance = new Chance();

console.log(`Connected to MPD socket: ${stationAlias}`);

// Some stations contain certain kinds of audio tracks, but not others. 
// This switch ensures the elements are chosen properly depending on the station
switch (stationAlias) {
    case "kjah":
        categories = ["Songs", "DJ", "ID", "Weather", "Time of Day"];
        weights = [1, 1, 1, 0.05, 0.04];
        break;
    case "radiols":
        categories = ["Songs", "DJ", "ID", "Weather", "Time of Day"];
        weights = [1, 1, 1, 0.05, 0.04];
        break
    case "sfur":
        categories = ["Songs", "Caller", "DJ", "Time of Day"];
        weights = [1, 0.0625, 1, 0.04];
        break
    default:
        categories = ["Songs", "Caller", "DJ", "ID", "Weather", "Time of Day"];
        weights = [1, 0.0625, 1, 1, 0.05, 0.04];
}

for (let cat of categories) {
    if(!(["Weather", "Time of Day"].includes(cat))){
        tracksMetadata[cat]=chance.shuffle(tracksMetadata[cat]);
    }
}


/* 
main function calls all other functions, gets theri array output and adds them to the player queue.
then it waits for a variable amount of time before adding another track.
*/
async function main(){
    // Enable consume. This allows tracks to be removed from the player queue after they are played. Prevents from accumulating backlog of tracks
    console.log(`MPD Consume enabled for ${stationAlias}`);

    while (true){
        output= await getNextCategory();
        for (let elem of output){
            // add tracks to queue one by one
            const fullPath=(`/radiosa/music/${elem}`)
            console.log(`Adding ${fullPath} to the queue`)
        }

        const sleepFor = 2;

        console.log(`Sleeping for ${sleepFor} seconds.`);
        await new Promise(resolve => setTimeout(resolve, sleepFor * 1000));
    }
}

/*
* getNextCategory() decides which category of tracks is played next
* Typically, these are "Songs", "Caller", "DJ", "ID", "Weather", "Time of Day".
*/
async function getNextCategory(){
    while (true) {
        let selectedCategory;
        // if 2 non-song tracks have been played, it ensures that next one is a song
        if (songHasNotBeenPlayedFor > 1 || lastCategory === "Caller"){
            selectedCategory = "Songs";
        }
        else {
            selectedCategory = chance.weighted(categories, weights);
        }

        bothAreVoicedbytheDJ = () => {
            voicedbytheDJ = ("Caller", "DJ", "Weather", "Time of Day")
            return (voicedbytheDJ.includes(lastCategory) && voicedbytheDJ.includes(selectedCategory));
        }

        // ensure that 2 of the same kind of tracks are not played back to back
        if(((lastCategory) && (selectedCategory === lastCategory)) || bothAreVoicedbytheDJ()){
            continue;
        }

        let currentTrack = await getNextTrack(selectedCategory);
        if (currentTrack === null){
            continue;
        }
        lastCategory=(selectedCategory);

        /*
        song objects have a different structure than all other tracks
        each song in radio contains at most 3 intros and 3 outros (2 with dj voice lines, 1 without)
        each song object is a json object containing the following keys
        title, intros (list), mid (middle part of the song, 1 element), outros (list)
        intros (and outros) is a list containing a single dict of the following keys
        dj0:path, dj1:path, dj2:path
        */

        if (selectedCategory === "Songs"){
            // get (dict==>intros==>0th==>values), convert to array of all possible intro lines
            // randomly choose a path from the list, assign it to song_intro
            
            const songIntro = chance.pickone(currentTrack.intros);
            const songMid = currentTrack.mid;
            const songOutro = chance.pickone(currentTrack.outros);

            songHasNotBeenPlayedFor = 0;
            return [songIntro, songMid, songOutro];
        }
        else {
            songHasNotBeenPlayedFor++;
            return [currentTrack];
        }
    }
}

// Provides the next track to play depending on the category that was passed
async function getNextTrack(selectedCategory){

    // weather and time of day announcements require real world data, so they are split into different functions.
    if (selectedCategory === "Weather"){
        return await getNextWeather();
    }
    else if (selectedCategory === "Time of Day"){
        return await getTimeTrack(getPacificTime);
    }
    else {
        // After receiving an array of tracks, randomly choose between indexes 0 and 1, then append that track at the back of the array
        // This FiFo like approach ensures that songs are spread apart from each other, but still random to simulate radio-like quality
        const tracksInCategory = tracksMetadata[selectedCategory];
        const randomBit = chance.integer({min: 0, max: 1});
        const chosenTrack = tracksInCategory.splice(randomBit, 1)[0];
        tracksInCategory.push(chosenTrack);
        return chosenTrack;
    }
}

async function getNextWeather(){
    // get current weather conditions about real life counterpart location of the in game radiostation
    // this is completely bonkers
    const weatherConvert = require("./weather.json");
    const currentWeather = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${stationMetadata.latitude}&longitude=${stationMetadata.longitude}&current=weather_code`).then((response) => response.json());
    timeOfDay = await getPacificTime();
    // only return track if the time is morning or afternoon, so radio hosts don't say "sunny" at night or something
    isMorningorAfternoon = (timeOfDay === "Morning" || timeOfDay === "Afternoon");
    if ((weatherConvert[currentWeather.current.weather_code] === undefined) || !isMorningorAfternoon){
        return null;
    }
    else {
        const weatherTrack = chance.pickone(tracksMetadata.Weather[weatherConvert[currentWeather.current.weather_code]])
        return weatherTrack;
    }
}

async function getPacificTime(){
    // get current time of day in Pacific Timezone
    // fetch details from worldtimeapi

    const currentPacificTime = await fetch("https://worldtimeapi.org/api/timezone/America/Los_Angeles").then((response) => response.json());
    /* 
    calculate current fraction of the day by adding the offset (negative number) to the Unix Timestamp (Greenwich Time)
    Then calculate daylight savings time ONLY IF daylight savings is true
    get modulo 86400 for amount of seconds after midnight of current day
    */
    const currentTimeOfDay = (currentPacificTime.unixtime + currentPacificTime.raw_offset + (currentPacificTime.dst && currentPacificTime.dst_offset))%86400;
    if (currentTimeOfDay >= 10800 && currentTimeOfDay < 43200){
        return "Morning";
    }
    else if (currentTimeOfDay >= 43200 && currentTimeOfDay < 54000){
        return "Afternoon";
    }
    else if (currentTimeOfDay >= 54000 && currentTimeOfDay < 75600){
        return "Evening"
    }
    else if (currentTimeOfDay < 10800 || currentTimeOfDay >= 75600){
        return "Night"
    }
}

// returns a track based on the time provided by getPacificTime function
async function getTimeTrack(getPacificTime){
    timeOfDay = await getPacificTime();

    if(timeOfDay === "Afternoon"){
        return null;
    }
    else{
        return (chance.pickone(tracksMetadata["Time of Day"][timeOfDay]));
    }
}
main();