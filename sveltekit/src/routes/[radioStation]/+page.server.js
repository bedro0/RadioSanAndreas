import { error } from '@sveltejs/kit';
import getEnabledStations from '$lib/get-enabled-stations.js';

export function load({ params }) {
    const enabledStations = getEnabledStations();
    const enabledStationsArray = Object.keys(enabledStations);
    const station = enabledStationsArray.find((station) => station === params.radioStation);

    if(!station) throw error(404);

    return {enabledStations};
}