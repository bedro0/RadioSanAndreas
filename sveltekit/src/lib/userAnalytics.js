import { promises as fsp } from "fs";
export async function saveClientHash(hashedAddress){
    const dbfile = "/radiosa/uniqueListeners.json"
    let uniqueListeners = await fsp.readFile(dbfile, "utf-8").then(file => JSON.parse(file));
    
    if (hashedAddress in uniqueListeners){
        uniqueListeners[hashedAddress].timesVisited =+ 1;
    }
    else {
        uniqueListeners[hashedAddress] = {
            timesVisited: 1
        }
    }
    await fsp.writeFile(dbfile, JSON.stringify(uniqueListeners, null, 4));

    return uniqueListeners;
}