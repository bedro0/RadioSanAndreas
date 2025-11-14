import { error } from "@sveltejs/kit";
import fs from "fs";
export async function GET(event){
    let uniqueListenersFile = fs.readFileSync("/radiosa/uniqueListeners.json");
    let uniqueListenersArray = JSON.parse(uniqueListenersFile);
    let uniqueListenersSet = new Set(uniqueListenersArray);

    const ipHash = await stringToSHA256(event.getClientAddress());
    uniqueListenersSet.add(ipHash);
    uniqueListenersArray = Array.from(uniqueListenersSet);
    uniqueListenersFile = JSON.stringify(uniqueListenersArray);
    fs.writeFileSync("/radiosa/uniqueListeners.json", uniqueListenersFile)
    return new Response(null, {
        status:200,
        headers: {
            "Content-Type": "application/json"
        }
    });
}

async function stringToSHA256(address){
    const textEncoder = new TextEncoder();
    const data = textEncoder.encode(address);
    const hashBuffer = await crypto.subtle.digest('SHA-256', data);
    const hashArray = Array.from(new Uint8Array(hashBuffer));
    const hashHex = hashArray.map(b => b.toString(16).padStart(2, '0')).join('');
    return hashHex;
}