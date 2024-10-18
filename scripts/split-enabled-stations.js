// Receives enabled stations env variable as a comma separated string.
// Returns a JSON file containing an array of individual channels.
const fs = require("fs");
const allStations = JSON.parse(fs.readFileSync("./all-stations.json", "utf-8"));
const enabledStationsArray = process.env.ENABLED_STATIONS.split(", ")
enabledStations = {};
enabledStationsArray.forEach((channel) => (enabledStations[channel] = allStations[channel]))

fs.writeFileSync("../sveltekit/src/enabled-stations.json", JSON.stringify(enabledStations, null, 4));
console.log(`The following stations are enabled: ${process.env.ENABLED_STATIONS}.`);