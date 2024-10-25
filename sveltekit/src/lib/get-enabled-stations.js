import fs from "fs";

export default () => JSON.parse(fs.readFileSync("/radiosa/sveltekit/enabled-stations.json", "utf8"))
