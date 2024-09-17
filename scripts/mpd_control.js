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
const radioMetadata = require(`${process.argv[2]}`);

// Sockets are defined by the name of their respective stations
const stationName = process.argv[3];

const allStations = ["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];

// declare initial variables
let categories, weights;
let songHasNotBeenPlayedFor = 0;
let lastCategory = "";

const chance = new Chance();

console.log(`Connected to MPD socket: ${stationName}`);

switch (stationName) {
    case "kjah":
        categories = ["Songs", "DJ", "ID"];
        weights = [1, 1, 1];
        break;
    case "radiols":
        categories = ["Songs", "DJ", "ID"];
        weights = [1, 1, 1];
        break
    case "sfur":
        categories = ["Songs", "Caller", "DJ"];
        weights = [1, 0.0625, 1];
        break
    default:
        categories = ["Songs", "Caller", "DJ", "ID"];
        weights = [1, 0.0625, 1, 1];
}

for (let cat of categories) {
    chance.shuffle(radioMetadata[cat]);
}

async function main(){
    const client = await mpd.connect({ path: `/radiosa/socks/${stationName}` });
    // Enable consume. This allows songs to be removed from the queue after they are played. Prevents from accumulating backlog of songs
    await client.api.playback.consume(1);
    console.log(`MPD Consume enabled for ${stationName}`);

    while (true){
        output= await queue();
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

async function queue(){
    while (true) {
        let selectedCategory;
        if (songHasNotBeenPlayedFor > 1 || lastCategory === "Caller"){
            selectedCategory = "Songs";
        }
        else {
            selectedCategory = chance.weighted(categories, weights);
        }

        if (lastCategory && selectedCategory === lastCategory){
            continue;
        }

        let currentTrack = getNextTrack(selectedCategory);
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
            
            const songIntro = chance.pickset(Object.values(currentTrack.intros[0]));
            const songMid = currentTrack.mid[0].mid;
            const songOutro = chance.pickset(Object.values(currentTrack.outros[0]));

            songHasNotBeenPlayedFor = 0;
            return [songIntro, songMid, songOutro];
        }
        else {
            songHasNotBeenPlayedFor++;
            return [currentTrack];
        }
    }
}

function getNextTrack(selectedCategory){
    const tracksInCategory = radioMetadata[selectedCategory];
    const randomBit = chance.integer({min: 0, max: 1});
    const chosenTrack = tracksInCategory.splice(randomBit, 1)[0];
    tracksInCategory.push(chosenTrack);
    return chosenTrack;
}

main();