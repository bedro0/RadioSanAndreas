import fs from "fs";

export default () => JSON.parse(fs.readFileSync("/radiosa/sveltekit/src/enabled-stations.json", "utf8"))
