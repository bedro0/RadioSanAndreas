// import libraries
const mpd = require("mpd-api");
const Chance = require("chance");

// This script directly controls the playback of the MPD.
// It accepts CLI arguments /path/to/json, mpd-socket, and mpd-control-port

// check if the incoming command was executed properly

if (process.argv.length !== 4) {
    console.log("Usage: node mpd_control.js /path/of/json [mpd socket]");
    process.exit(1);
}

// import the JSON path specified as a CLI argument
const tracksMetadata = require(`${process.argv[2]}`);
// Sockets are defined by the name of their respective stations
const stationAlias = process.argv[3];
const stationMetadata = (require("./station_metadata.json"))[stationAlias]


let categories, weights;
let songHasNotBeenPlayedFor = 0;
let lastCategory = "";

const chance = new Chance();

console.log(`Connected to MPD socket: ${stationAlias}`);

// Some stations contain certain kinds of audio snippets, but not others. This ensures the elements are chosen properly depending on the station
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

async function main(){
    const client = await mpd.connect({ path: `/radiosa/socks/${stationAlias}` });
    // Enable consume. This allows songs to be removed from the queue after they are played. Prevents from accumulating backlog of songs
    await client.api.playback.consume(1);
    console.log(`MPD Consume enabled for ${stationAlias}`);

    while (true){
        output= await getNextCategory();
        for (let elem of output){
            // add tracks to queue one by one
            const fullPath=(`/radiosa/music/${elem}`)
            console.log(`Adding ${fullPath} to the queue`)
            await client.api.queue.add(fullPath)
        }

        const sleepFor = await remainingTime(client);
        console.log("Pressing play.")
        await client.api.playback.play();

        console.log(`Sleeping for ${sleepFor} seconds.`);
        await new Promise(resolve => setTimeout(resolve, sleepFor * 1000));
    }
}

async function remainingTime(client){
    // Returns remaining duration of the queue (not including the first track in the queue)

    let fullQueue = await client.api.queue.info();
    let totalPlaytime = 0;
    for (let track of fullQueue.slice(1)){
        totalPlaytime += track.duration;
    }
    return totalPlaytime;
}

async function getNextCategory(){
    while (true) {
        let selectedCategory;
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

        if(((lastCategory) && (selectedCategory === lastCategory)) || bothAreVoicedbytheDJ()){
            continue;
        }

        let currentTrack = await getNextTrack(selectedCategory);
        if (currentTrack === null){
            continue;
        }
        lastCategory=(selectedCategory);

        /*
        song objects have a different structure
        each song in radio contains at most 3 intros and 3 outros (2 with dj voice lines, 1 without)
        each song object is a json object containing the following keys
        title, intros (list), mid (middle part of the song, 1 element), outros (list)
        intros (and outros) is a list containing a single dict of the following keys
        dj0:path, dj1:path, dj2:path
        */

        if (selectedCategory === "Songs"){
            // get (dict==>intros==>0th==>values), convert to array of all possible intro lines
            // randomly choose a path from the list, assign it to song_intro
            
            const songIntro = chance.pickone(Object.values(currentTrack.intros[0]));
            const songMid = currentTrack.mid[0].mid;
            const songOutro = chance.pickone(Object.values(currentTrack.outros[0]));

            songHasNotBeenPlayedFor = 0;
            return [songIntro, songMid, songOutro];
        }
        else {
            songHasNotBeenPlayedFor++;
            return [currentTrack];
        }
    }
}

async function getNextTrack(selectedCategory){
    if (selectedCategory === "Weather"){
        return await getNextWeather();
    }
    else if (selectedCategory === "Time of Day"){
        return await getTimeTrack(getPacificTime);
    }
    else {
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
    const apiResponse = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${stationMetadata.latitude}&longitude=${stationMetadata.longitude}&current=weather_code`);
    const responseBody = await apiResponse.json()
    timeOfDay = await getPacificTime();
    isMorningorAfternoon = (timeOfDay === "Morning" || timeOfDay === "Afternoon");
    if ((weatherConvert[responseBody.current.weather_code] === undefined) || !isMorningorAfternoon){
        return null;
    }
    else {
        const weatherTrack = chance.pickone(tracksMetadata.Weather[weatherConvert[responseBody.current.weather_code]])
        return weatherTrack;
    }
}

async function getPacificTime(){
    // get current time of day in Pacific Timezone
    // fetch details from worldtimeapi
    const apiResponse = await fetch("https://worldtimeapi.org/api/timezone/America/Los_Angeles");
    const responseBody = await apiResponse.json();
    /* 
    calculate current fraction of the day by adding the offset (negative number) to the Unix Timestamp (Greenwich Time)
    Then calculate daylight savings time ONLY IF daylight savings is true
    get modulo 86400 for amount of seconds after midnight of current day
    */
    const currentTimeOfDay = (responseBody.unixtime + responseBody.raw_offset + (responseBody.dst && responseBody.dst_offset))%86400;
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