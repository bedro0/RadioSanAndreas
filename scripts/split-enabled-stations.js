// Receives enabled stations env variable as a comma separated string.
// Returns a JSON file containing an array of individual channels.
const fs = require("fs");
fs.writeFileSync("../sveltekit/src/enabled-stations.json", enabledStations=(JSON.stringify(process.env.ENABLED_STATIONS.split(", ")) + "\n"));
console.log(`The following stations are enabled: ${process.env.ENABLED_STATIONS}.`);