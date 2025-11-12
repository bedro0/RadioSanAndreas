import Chance from "chance";
const chance = new Chance();
export const getRandomTrackSource = async (station, category) => {
    const channelMetadata = await import("/radiosa/scripts/metadata.mjs").then(obj=>obj[station])
    let trackList = [];
    switch(category){
        case "Song (Intro)":
            trackList = channelMetadata["Song"];
            let randomSongIntrosList = chance.pickone(trackList).intros;
            randomSongIntrosList = randomSongIntrosList.filter(intro=>!intro.includes("(Intro)"));
            return chance.pickone(randomSongIntrosList);
        case "Song (Outro)":
            trackList = channelMetadata["Song"];
            let randomSongOutrosList = chance.pickone(trackList).outros;
            randomSongOutrosList = randomSongOutrosList.filter(outro=>!outro.includes("(Outro)"));
            return chance.pickone(randomSongOutrosList);
        case "Weather":
            for (let weatherType in channelMetadata["Weather"]){
                trackList = trackList.concat(channelMetadata["Weather"][weatherType])
            }
            return chance.pickone(trackList);
        case "Time of Day":
            for (let timeOfDayType in channelMetadata["Time of Day"]){
                trackList = trackList.concat(channelMetadata["Time of Day"][timeOfDayType]);
            }
            return chance.pickone(trackList);
        default:
            return chance.pickone(channelMetadata[category]);

    }
}
export const setTrackSource = async (sourceString, player) => {
    player.src = (`/api/getTrack?track=${encodeURIComponent(sourceString)}`);
    player.play();
}