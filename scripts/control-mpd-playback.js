const MPC = require("mpc-js").MPC;
const client = new MPC();

const Chance = require("chance");
const chance = new Chance();
const weatherCodes = require("./weather.json");

// Local sockets are defined by the name of their respective stations
const channelAlias = process.env.CHANNEL_ALIAS;

// channelMetadata contains metadata about tracks contained in the radio channel program
const channelMetadata = require(`../metadata/${channelAlias}.json`);
// stationData contains name, host, and supposed real life location of the radio station, etc.
const stationData = (require("./all-stations.json"))[channelAlias];

// This variable tracks how many tracks ago a song (or a talk show in case of WCTR) was played.
let songHasNotBeenPlayedFor = 0;
let lastCategory = "";

// get all playable categories from the metadata file
// then get their probability weights using their length.
// Weather and Time of Day don't have length. they will be replaced with chance of 1 part. We want these to be pretty rare so it doesn't really matter.
const categories = Object.keys(channelMetadata);
if (categories.includes("Bridge Announcement")) categories.splice(categories.indexOf("Bridge Announcement"), 1);
const weights = Object.fromEntries(
    categories.map(cat => [cat, channelMetadata[cat]?.length ?? 1])
  );

// To make playback more radio-like, we shuffle tracks in each category on initial run
// (except for weather and ToD ones, they don't need that).
const excludedCategories = ["Weather", "Time of Day"];
categories
  .filter(cat => !excludedCategories.includes(cat))
  .forEach(cat => {
    channelMetadata[cat] = chance.shuffle(channelMetadata[cat]);
  });

async function main(){
    client.connectUnixSocket(`/radiosa/socks/${channelAlias}`);
    console.log(`Connected to MPD socket: ${stationData.channel_name}.`);
    // Enable consume. This allows tracks to be removed from the player queue after they are played. Prevents from accumulating backlog of tracks
    await client.playbackOptions.setConsume(true);
    console.log(`MPD Consume enabled for ${stationData.channel_name}.`);

    while(true){
        selectedTrack = await getNextTrack();
        for (elem of selectedTrack){
            const fullPath = (`/radiosa/music/${elem}`);
            console.log(`Adding ${fullPath} to the queue.`)
            await client.currentPlaylist.add(fullPath);
        }
        const sleepFor = await remainingTime();
        await client.playback.play();

        console.log(`Sleeping for ${sleepFor} seconds.`);
        await new Promise(resolve => setTimeout(resolve, sleepFor * 1000));
    }
}

async function remainingTime(){
    // Returns remaining duration of the queue (not including the first track in the queue)

    let fullQueue = await client.currentPlaylist.playlistInfo();
    let totalPlaytime = 0;
    for (let track of fullQueue.slice(1)){
        totalPlaytime += track.duration;
    }
    return totalPlaytime;

}

/*
* getNextCategory() decides which category of tracks is played next
* Typically, these are "Song", "Caller", "DJ", "ID", "Weather", "Time of Day".
*/

async function getNextTrack(){

    let categorySet = new Set(categories);

    while(true){
        let selectedCategory = getNextCategory(categorySet);
        let selectedTrack;

        switch (selectedCategory){
            case "Weather":
                selectedTrack = [await getNextWeather()];
                break;
            case "Time of Day":
                selectedTrack = [await getTimeTrack()];
                break;
            case "Song":
                const trackObject = getRandomBitTrack(selectedCategory);
                selectedTrack = [
                    chance.pickone(trackObject.intros),
                    trackObject.mid,
                    chance.pickone(trackObject.outros)
                ]
                break;
            case "Show":
                selectedTrack = getRandomBitTrack(selectedCategory);
                break;
            default:
                selectedTrack = [getRandomBitTrack(selectedCategory)];
                break;
        }
        if (selectedTrack[0] === null){
            categorySet.delete(selectedCategory)
            continue;
        }

        if(selectedCategory !== categories[0]) songHasNotBeenPlayedFor++;
        else songHasNotBeenPlayedFor = 0;

        lastCategory = selectedCategory;
        return selectedTrack;
    }
}

function getNextCategory(categorySet){
    let selectedCategory;
    const voicedBYDJ = ["Caller", "DJ", "Weather", "Time of Day"];

    // remove last category from the list of choices to not repeat same kind of track
    // except if previous track was voiced by DJ, then remove all tracks voiced by dj
    const categoriesToRemove = new Set(voicedBYDJ.includes(lastCategory) ? voicedBYDJ : [lastCategory]);
    let categoriesToChooseFrom = Array.from(categorySet.difference(categoriesToRemove));
    let chancesToChooseWith = categoriesToChooseFrom.map(cat => weights[cat]);
    
    if (songHasNotBeenPlayedFor >1) selectedCategory = categories[0];
    else selectedCategory = chance.weighted(categoriesToChooseFrom, chancesToChooseWith);

    return selectedCategory;

}

function getRandomBitTrack(selectedCategory){
    // FROM TOP 2 TRACKS in a given category, returns random one.
    const tracksInCurrentCategory = channelMetadata[selectedCategory];
    const randomBit = +chance.bool();
    const [selectedTrack] = tracksInCurrentCategory.splice(randomBit, 1);
    tracksInCurrentCategory.push(selectedTrack);
    return selectedTrack;
}

async function getPacificTime(){
    // get current time of day in Pacific Timezone
    // fetch details from worldtimeapi
    let currentPacificTime;
    try {
        currentPacificTime = await fetch("https://worldtimeapi.org/api/timezone/America/Los_Angeles").then((response) => response.json());
    }
    catch (error){
        return null;
    }
    /* 
    calculate current fraction of the day by adding the offset (negative number) to the Unix Timestamp (Greenwich Time)
    Then calculate daylight savings time ONLY IF daylight savings is true
    get modulo 86400 for amount of seconds after midnight of current day
    */
    const currentTimeOfDay = (currentPacificTime.unixtime + currentPacificTime.raw_offset + (currentPacificTime.dst && currentPacificTime.dst_offset))%86400;

    if (currentTimeOfDay < 10800 || currentTimeOfDay >= 75600) return "Night";
    if (currentTimeOfDay < 43200) return "Morning";
    if (currentTimeOfDay < 54000) return "Afternoon";
    return "Evening";
}

async function getNextWeather(){
    // get current weather conditions where radio station is located
    // the location is based on an estimation of where radio stations are located in game and what that location corresponds to in real life.
    let currentWeather;
    try {
        currentWeather = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${stationData.latitude}&longitude=${stationData.longitude}&current=weather_code`).then((response) => response.json());
    }
    catch (error){
        return null;
    }
    timeOfDay = await getPacificTime();
    // only return track if the time is morning or afternoon, so radio hosts don't say "sunny" at night or something
    isMorningorAfternoon = (timeOfDay === "Morning" || timeOfDay === "Afternoon");
    if (timeOfDay === null || (weatherCodes[currentWeather.current.weather_code] === undefined) || !isMorningorAfternoon){
        return null;
    }
    else {
        const weatherTrack = chance.pickone(channelMetadata.Weather[weatherCodes[currentWeather.current.weather_code]])
        return weatherTrack;
    }
}

// returns a track based on the time provided by getPacificTime function
async function getTimeTrack(){
    timeOfDay = await getPacificTime();

    if(channelMetadata["Time of Day"][timeOfDay] === undefined){
        return null;
    }
    else{
        return (chance.pickone(channelMetadata["Time of Day"][timeOfDay]));
    }
}

main();