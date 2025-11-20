const TIMEOUT_MS = 10000;

export let uniqueListeners = new Map();

export async function insertListener(addressHash){
    if (addressHash in uniqueListeners){
        uniqueListeners[addressHash].timesVisited += 1;
    }
    else {
        uniqueListeners[addressHash] = {
            timesVisited: 1,
        }
    }
    uniqueListeners[addressHash] = {
        ...
        uniqueListeners[addressHash],
        currentlyActive: true,
        lastSeen: Date.now()
    };
    return new Promise((resolve)=>{
        setTimeout(()=>{
            uniqueListeners[addressHash].currentlyActive = false;
            resolve(addressHash);
        }, TIMEOUT_MS);
    });
}