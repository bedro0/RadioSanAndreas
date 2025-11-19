import { MPC } from "mpc-js";
import Chance from "chance";
import weatherCodes from "./weather.mjs";

const mpdClient = new MPC();
const chance = new Chance();

// Local sockets are defined by the name of their respective stations
const channelAlias = process.env.CHANNEL_ALIAS;
// channelMetadata contains metadata about tracks contained in the radio channel program, as well as name, host, and supposed real life location of the radio station, etc.
const channelMetadata = await import("./metadata.mjs").then(obj => obj[channelAlias]);

// This variable tracks how many tracks ago a song (or a talk show in case of WCTR) was played.
let songHasNotBeenPlayedFor = 0;
let lastCategory;

// get all playable categories from the metadata file then get their probability weights using their length.
// Weather and Time of Day don't have length. they will be replaced with chance of 1 part. We want these to be pretty rare so it doesn't really matter.
// Test: We are squaring the lengths of the arrays to decrease the frequency of certain tracks, like Caller, since they kept repeating
const categories = Object.keys(channelMetadata.Content).filter(cat => !cat.includes("Bridge Announcement"));
const weights = Object.fromEntries(
    [...categories].map(cat => [cat, (channelMetadata.Content[cat]?.length ?? 1)**2])
);

// To make playback more radio-like, we shuffle tracks in each category on initial run
// (except for weather and ToD ones, they don't need that).
categories
    .filter(cat => !["Weather", "Time of Day"].includes(cat))
    .forEach(cat => {
        channelMetadata.Content[cat] = chance.shuffle(channelMetadata.Content[cat]);
    }
);

mpdClient.connectUnixSocket(`/radiosa/socks/${channelAlias}`);
console.log(`Connected to MPD socket: ${channelMetadata.Identity.channel_name}.`);
// Enable consume. This allows tracks to be removed from the player queue after they are played. Prevents from accumulating backlog of tracks
await mpdClient.playbackOptions.setConsume(true);
console.log(`MPD Consume enabled for ${channelMetadata.Identity.channel_name}.`);

while(true){
    let selectedTrack = await getNextTrack();

    for (const elem of selectedTrack){
        const fullPath = (`/radiosa/music/${elem}`);
        await mpdClient.currentPlaylist.add(fullPath);
        console.log(`Added ${fullPath} to the queue.`)
    }

    await mpdClient.playback.play();
    
    const sleepFor = await remainingTime();
    console.log(`${channelMetadata.Identity.channel_name} sleeping for ${sleepFor} seconds.`);
    
    // Wait for that duration before adding more tracks
    await new Promise(resolve => setTimeout(resolve, sleepFor * 1000));
}

async function remainingTime(){
    // Returns remaining duration of the queue (not including the first track in the queue)

    let fullQueue = await mpdClient.currentPlaylist.playlistInfo();
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

        if ( ["Song", "Show"].includes(selectedCategory) ) songHasNotBeenPlayedFor = 0;
        else songHasNotBeenPlayedFor++;

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
    
    if (songHasNotBeenPlayedFor >1){
        [selectedCategory] = [...(new Set(["Song", "Show"]).intersection(categorySet))];
    }
    else selectedCategory = chance.weighted(categoriesToChooseFrom, chancesToChooseWith);

    return selectedCategory;

}

function getRandomBitTrack(selectedCategory){
    // from the pool of FIRST 2 TRACKS in a given category array, returns a random track.
    const tracksInCurrentCategory = channelMetadata.Content[selectedCategory];
    const randomBit = +chance.bool();
    const [selectedTrack] = tracksInCurrentCategory.splice(randomBit, 1);
    tracksInCurrentCategory.push(selectedTrack);
    return selectedTrack;
}

async function getPacificTime(){
    // get current time of day in Pacific Timezone
    // fetch details from worldtimeapi
    const currentPacificTime = await fetch("https://worldtimeapi.org/api/timezone/America/Los_Angeles")
    .then((response) => response.json())
    .catch(error => null);

    if (currentPacificTime === null){
        return null;
    }

    /* 
    calculate current fraction of the day by adding the offset (negative number) to the Unix Timestamp (Greenwich Time)
    Then calculate daylight savings time ONLY IF daylight savings is true
    get modulo 86400 for amount of seconds after midnight of current day
    */
    const currentTimeOfDay = (currentPacificTime.unixtime + currentPacificTime.raw_offset + (currentPacificTime.dst && currentPacificTime.dst_offset))%86400;

    if (currentTimeOfDay >= 75600 || currentTimeOfDay < 10800) return "Night";
    if (currentTimeOfDay < 43200) return "Morning";
    if (currentTimeOfDay < 54000) return "Afternoon";
    return "Evening";
}

async function getNextWeather(){
    // only return track if the time is morning or afternoon, so radio hosts don't say "sunny" at night or something
    // premature return prevents an unnecessary API fetch call.
    const timeOfDay = await getPacificTime();
    if (timeOfDay === null) return null;
    if (!["Morning", "Afternoon"].includes(timeOfDay)) return null;

    // get current weather conditions where radio station is located
    // the location is based on an estimation of where radio stations are located in game and what that location corresponds to in real life.
    const currentWeather = await fetch(`https://api.open-meteo.com/v1/forecast?latitude=${channelMetadata.Identity.latitude}&longitude=${channelMetadata.Identity.longitude}&current=weather_code`)
    .then((response) => response.json())
    .catch((error => null));
    if (currentWeather === null) return null;
    // This snippet may break chance.pickone(), because current weather may be undefined.
    // weatherCodes[4] will return undefined, for example. chance.pickone can only operate on Arrays.
    try{
        const weatherTrack = chance.pickone(channelMetadata.Content.Weather[weatherCodes[currentWeather.current.weather_code]]);
        return weatherTrack;
    }
    catch {
        return null;
    }
}

// returns a track based on the time provided by getPacificTime function
async function getTimeTrack(){
    const timeOfDay = await getPacificTime();

    if(channelMetadata.Content["Time of Day"][timeOfDay] === undefined){
        return null;
    }
    else{
        return (chance.pickone(channelMetadata.Content["Time of Day"][timeOfDay]));
    }
}