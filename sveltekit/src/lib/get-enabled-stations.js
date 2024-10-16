import fs from "fs";

export default () => JSON.parse(fs.readFileSync("../enabled-stations.json", "utf8"))
