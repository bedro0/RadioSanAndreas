import { TIMEOUT_MS } from "$lib/pollingRate.js" ;
import { json, error } from "@sveltejs/kit";
import { sha256 } from "$lib/sha256.js"

let uniqueListeners = {};
let initialDate;

async function insertListener(userAddressHash, path){
    if (uniqueListeners[userAddressHash]){
        clearTimeout(uniqueListeners[userAddressHash].userTimeout)
    }
    uniqueListeners[userAddressHash] = {
        timesVisited: (uniqueListeners[userAddressHash]?.timesVisited ?? 0) + 1,
        history: [...(uniqueListeners[userAddressHash]?.history ?? []), path],
        currentlyActive: true,
        lastSeen: Date.now()
    };
    uniqueListeners[userAddressHash].userTimeout = setTimeout(() => {
        uniqueListeners[userAddressHash].currentlyActive = false;
    }, TIMEOUT_MS);
}

export async function GET(){
    if (!initialDate){
        initialDate = new Date();
    }
    const responseString = `Total amount of unique IPs since ${initialDate.toISOString()} is ${Object.keys(uniqueListeners).length}`;
    return json(responseString);
}

export async function POST({ request, getClientAddress }){
    if (!initialDate){
        initialDate = new Date();
    }
    const { currentUserURLPath } = await request.json();
    const clientIP = request.headers.get('x-forwarded-for')?.split(',')[0].trim() || getClientAddress();
    const addressHash = await sha256(clientIP);
    insertListener(addressHash, currentUserURLPath);

    const userAddressHashArray = Object.keys(uniqueListeners);
    const currentListenerArray = userAddressHashArray.filter(element => uniqueListeners[element].currentlyActive);
    const listenersSameStation = currentListenerArray.filter(element => {
        const elemHistory = uniqueListeners[element].history;
        return elemHistory[elemHistory.length - 1] === currentUserURLPath;
    });
    const userData = {
        arrays: {
            userAddressHashArray,
            currentListenerArray,
            listenersSameStation
        },
        amounts: {
            totalUsers: userAddressHashArray.length,
            activeUsers: currentListenerArray.length,
            sameStation: listenersSameStation.length
        }
    };
    return json(userData.amounts);
}
