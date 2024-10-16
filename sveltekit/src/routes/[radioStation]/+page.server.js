import fs from "fs";

export function load(){

    const enabledStations = fs.readFileSync("src/enabled-stations.json", "utf8");
    return {"enabledStations": enabledStations}
}
