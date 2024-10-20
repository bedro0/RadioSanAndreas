import { error } from '@sveltejs/kit';
import getEnabledStations from '$lib/get-enabled-stations.js';

export function load({ params }) {
    const enabledStationsObject = getEnabledStations();
    const currentStation = Object.keys(enabledStationsObject).find((station) => station === params.radioStation);

    if(!currentStation) throw error(404, { message: "Not Found" });

    return {
        currentStationObject: enabledStationsObject[currentStation]
    }
}