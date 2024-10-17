const MPC = require("mpc-js").MPC;
const Chance = require("chance");
const chance = new Chance();
const tracksMetadata = require("/radiosa/metadata/wctr.json");

const categories = ["Show", "ID", "Advert"];
const weights = [1, 0.25, 1];
let showHasNotBeenPlayedFor=0;
let lastCategory = "";

for (cat of categories) tracksMetadata[cat]=chance.shuffle(tracksMetadata[cat]);

async function main(){
    const client = new MPC();
    client.connectUnixSocket("/radiosa/socks/wctr");
    console.log("Connected to MPD socket: West Coast Talk Radio.")
    await client.playbackOptions.setConsume(true);
    console.log("MPD Consume enabled for West Coast Talk Radio.")

    while(true){
        selectedCategory = getNextCategory();
        selectedTrack = getNextTrack(selectedCategory);
        if(selectedCategory !== "Show") selectedTrack = [selectedTrack];
        for(elem of selectedTrack){
            const fullPath = (`/radiosa/music/${elem}`);
            console.log(`Adding ${fullPath} to the queue.`)
            await client.currentPlaylist.add(fullPath);
        }
        const sleepFor = await remainingTime(client);
        await client.playback.play();

        console.log(`Sleeping for ${sleepFor} seconds.`);
        await new Promise(resolve => setTimeout(resolve, sleepFor * 1000));
    }
}

async function remainingTime(client){
    // Returns remaining duration of the queue (not including the first track in the queue)

    let fullQueue = await client.currentPlaylist.playlistInfo();
    let totalPlaytime = 0;
    for (let track of fullQueue.slice(1)){
        totalPlaytime += track.duration;
    }
    return totalPlaytime;
}

function getNextCategory(){
    let currentCategories = [... categories];
    let currentWeights = [... weights];
    if(lastCategory){
        lastCategoryIndex = currentCategories.indexOf(lastCategory);
        currentCategories.splice(lastCategoryIndex, 1);
        currentWeights.splice(lastCategoryIndex, 1);
    }
    let selectedCategory;
    if(showHasNotBeenPlayedFor > 1) selectedCategory = "Show";
    else selectedCategory = chance.weighted(currentCategories, currentWeights);

    if(selectedCategory !== "Show") showHasNotBeenPlayedFor++;
    else showHasNotBeenPlayedFor = 0;

    lastCategory = selectedCategory;
    return selectedCategory;
}

function getNextTrack(selectedCategory){
    const tracksInCurrentCategory = tracksMetadata[selectedCategory];
    const randomBit = +(chance.bool());
    const [selectedTrack] = tracksInCurrentCategory.splice(randomBit, 1);
    tracksInCurrentCategory.push(selectedTrack);
    return selectedTrack;
}

main();