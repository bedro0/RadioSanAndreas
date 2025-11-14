import fs from "fs";
const icecastPasswords = await import("./pass.json", { with: { type: "json" } }).then(obj=>obj.default);
const channelAlias=process.env.CHANNEL_ALIAS;
const channelMetadata = await import("./metadata.mjs").then(obj => obj[channelAlias].Identity);
const replacePatternWith = {
    "CHANNEL_NAME": channelMetadata.channel_name,
    "CHANNEL_ALIAS": channelAlias,
    "ICECAST_SOURCE_PASS": icecastPasswords.ICECAST_SOURCE_PASS,
    "STATION_GENRE": channelMetadata.genre
}

let sampleConfig = fs.readFileSync("../config/sample.conf", "utf-8");
for(let key in replacePatternWith){
    sampleConfig = sampleConfig.replaceAll(`%${key}%`, replacePatternWith[key]);
    console.log(`Replaced all instances of %${key}% with ${replacePatternWith[key]} in the config file for ${replacePatternWith.CHANNEL_NAME}.`)
}
fs.writeFileSync(`../config/${channelAlias}.conf`, sampleConfig);
console.log(`MPD config successfully created for ${replacePatternWith.CHANNEL_NAME}`);