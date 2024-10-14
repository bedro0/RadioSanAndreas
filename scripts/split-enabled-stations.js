const fs = require("fs");
fs.writeFile("../sveltekit/src/enabled-stations.json", enabledStations=(JSON.stringify(process.env.ENABLED_STATIONS.split(", ")) + "\n"), error => {
    if (error) {
        console.error(error);
    }
    else {
        console.log("The following stations are enabled:", process.env.ENABLED_STATIONS);
    }
});