// Modifies variable patterns inside icecast.xml to match environment variables

import fs from "fs";
import Chance from "chance";
const chance = new Chance();

const patternsToReplace = ["HOSTNAME", "DISPLAY_ADMIN", "ADMIN_USER"]
const passwordsToReplace = {
    "ICECAST_SOURCE_PASS":process.env.ICECAST_SOURCE_PASS,
    "ICECAST_RELAY_PASS":process.env.ICECAST_RELAY_PASS,
    "ICECAST_ADMIN_PASS":process.env.ICECAST_ADMIN_PASS
}

let icecastConfigFile = fs.readFileSync( "../config/icecast.xml", "utf-8");
for(let pattern of patternsToReplace){
    icecastConfigFile=icecastConfigFile.replaceAll(`%${pattern}%`, process.env[pattern]);
    console.log(`Changed %${pattern}% to ${process.env[pattern]} in Icecast config.`);
}
for(let passwordPattern in passwordsToReplace){
    if(passwordsToReplace[passwordPattern] === ""){
        passwordsToReplace[passwordPattern] = chance.string({ length:16, casing: 'upper', alpha: true, numeric: true });
    }
    icecastConfigFile=icecastConfigFile.replaceAll(`%${passwordPattern}%`, passwordsToReplace[passwordPattern]);
    console.log(`Changed %${passwordPattern}% to ${passwordsToReplace[passwordPattern]} in Icecast config.`);
}
fs.writeFileSync("./pass.json", JSON.stringify(passwordsToReplace));
fs.writeFileSync("../config/icecast.xml", icecastConfigFile);
console.log("Icecast config updated successfully.");