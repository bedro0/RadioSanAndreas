import fs from "fs";

export function load(){

    const enabledStations = fs.readFileSync("/radiosa/sveltekit/src/enabled-stations.json", "utf8");
    return {"enabledStations": enabledStations}
}
