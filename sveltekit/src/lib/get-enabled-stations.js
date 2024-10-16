import fs from "fs";

export default () => JSON.parse(fs.readFileSync("src/enabled-stations.json", "utf8"))
