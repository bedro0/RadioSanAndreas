import { applyAction } from "$app/forms";
import { error } from "@sveltejs/kit";

const all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur", "wctr"];

export async function GET({ url }){
    const station = url.searchParams.get("station");
    const metadataType = url.searchParams.get("metadataType");
    const metadataSubType = url.searchParams.get("metadataSubType");
    let response;

    if(!station || !metadataType){
        throw error(400, "Invalid request, please specify the station and metadataType.")
    }

    if(!all_stations.includes(station) || !["Content", "Identity"].includes(metadataType)){
        throw error(
            400,
            `Invalid request, "station" attribute should be set to one of the following: ${JSON.stringify(all_stations, null, 1)}. "metadataType" attribute should be set to "Content" or "Identity"`
        )
    }

    if (metadataSubType){
        response = await import("/radiosa/scripts/metadata.mjs").then(obj => obj[station][metadataType][metadataSubType]);
    }
    else {
        response = await import("/radiosa/scripts/metadata.mjs").then(obj => obj[station][metadataType]);
    }

    response = JSON.stringify(response);
    return new Response(
        response,
        {
            status: 200,
            headers: {
                "Content-Type": "application/json"
            }
        }
    )
}