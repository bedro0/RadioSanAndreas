import { json, error } from "@sveltejs/kit";
import fs from "fs";
import Chance from "chance";
const all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur", "wctr"];
const chance = new Chance();

export async function GET({ url }){
    const category = url.searchParams.get("category");
    const station = url.searchParams.get("station");

    if (!all_stations.includes(station)){
        throw error(400, "Please ensure the station alias is correct and exists in the following list: bouncefm, csr, kdst, kjah, krose, mastersounds, playbackfm, radiols, radiox, sfur, wctr.")
    }
    
    const channelMetadata = await import("/radiosa/scripts/metadata.mjs").then(obj => obj[station]["Content"]);
    let categories=new Set(Object.keys(channelMetadata));
        if (categories.delete("Song")){
            categories.add("Song (Intro)");
            categories.add("Song (Outro)")
        }
    if (!categories.has(category)){
        throw error(400, "Please make sure the category exists in the specified station.")
    }
    const randomTrack = await getRandomTrackFromCategory(channelMetadata, category);
    const responseFile = fs.readFileSync(`/radiosa/music/${randomTrack}`);
    return new Response(responseFile, {
        headers: {
            "Content-Type": "audio/ogg"
        }
    });
}

async function getRandomTrackFromCategory(channelMetadata, category){
    let trackList = [];
    switch(category){
        case "Song (Intro)":
            trackList = channelMetadata["Song"];
            let djIntros = [];
            for (let track of trackList){
                djIntros = djIntros.concat(track.intros.filter(intro=>intro.includes("Intro DJ #")))
            }
            return chance.pickone(djIntros);
        case "Song (Outro)":
            trackList = channelMetadata["Song"];
            let djOutros = [];
            for (let track of trackList){
                djOutros = djOutros.concat(track.outros.filter(outro=>outro.includes("Outro DJ #")))
            }
            return chance.pickone(djOutros);
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