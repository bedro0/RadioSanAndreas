const fs = require("fs");
const icecastPasswords = require("./pass.json")
const channelMetadata = require("./all-stations.json");
const channelAlias=process.env.CHANNEL_ALIAS;
const replacePatternWith = {
    "CHANNEL_NAME": channelMetadata[channelAlias].channel_name,
    "CHANNEL_ALIAS": channelAlias,
    "ICECAST_SOURCE_PASS": icecastPasswords.ICECAST_SOURCE_PASS,
    "STATION_GENRE": channelMetadata[channelAlias].genre
}

let sampleConfig = fs.readFileSync("../config/sample.conf", "utf-8");
for(key in replacePatternWith){
    sampleConfig = sampleConfig.replaceAll(`%${key}%`, replacePatternWith[key]);
    console.log(`Replaced all instances of %${key}% with ${replacePatternWith[key]} in the config file for ${replacePatternWith.CHANNEL_NAME}.`)
}
fs.writeFileSync(`../config/${channelAlias}.conf`, sampleConfig);
console.log(`MPD config successfully created for ${replacePatternWith.CHANNEL_NAME}`);