import getEnabledStations from '$lib/get-enabled-stations.js';

export function load() {
    return { enabledStations: getEnabledStations() };
}