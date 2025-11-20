import { json, error } from "@sveltejs/kit";
import { uniqueListeners, insertListener } from "$lib/userAnalytics.js";
import { sha256 } from "$lib/sha256.js"

export async function POST({ request, getClientAddress }){
    const addressHash = await sha256(getClientAddress());
    insertListener(addressHash);
    return json(uniqueListeners);
}
