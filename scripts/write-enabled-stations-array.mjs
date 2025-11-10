// Receives enabled stations env variable as a comma separated string.
// Export an object with that only contains the keys to the enabled stations.
import fs from "fs";

const allStations = await import("./metadata.mjs");
const enabledStationsArray = process.env.ENABLED_STATIONS.split(", ");
const enabledStations = {};
enabledStationsArray.forEach((channel) => (enabledStations[channel] = allStations[channel]));

fs.writeFileSync("/radiosa/sveltekit/enabled-stations.json", JSON.stringify(enabledStations, null, 4));