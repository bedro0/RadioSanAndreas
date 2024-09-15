<script>
    import { browser } from '$app/environment';
    import { goto } from '$app/navigation';
    import { page } from '$app/stores';
    import station_metadata_json from "/src/station_metadata.json";
    import enabledStations from "/radiosa/enabled_stations.json"

    const currentStation = $page.params.radioStation;
    const stationMetadata = station_metadata_json[currentStation];

    $: if (browser && !enabledStations.includes(currentStation)) {
        goto("/");
    }

    let playerObj;
    let playerVolume = 1;
    let isPlaying = false;

    $: main_src = `/api/radio?format=ogg&station=${currentStation}&nocache=${Date.now()}`;
    $: alt_src = `/api/radio?format=mp3&station=${currentStation}&nocache=${Date.now()}`;
    $: favicon = `/src/visual-assets/logos/${currentStation}.webp`;

    function togglePlayPause() {
        isPlaying = !isPlaying;
        if (isPlaying) {
            playerObj.play();
        } else {
            playerObj.pause();
        }
    }
</script>

<svelte:head>
    {#if enabledStations.includes(currentStation)}
        <title>{stationMetadata.station_name}</title>
        <link rel="icon" href={favicon} />
    {/if}
</svelte:head>

{#if stationMetadata}
    <style lang="scss">
        @import "../../player.scss";
    </style>

    <img id="background-image" src="src/visual-assets/backgrounds/{currentStation}.jpg" alt={currentStation}>

    <audio autoplay bind:this={playerObj} bind:volume={playerVolume} on:play={() => isPlaying = true} on:pause={() => isPlaying = false}>
        <source src={main_src} type="audio/ogg">
        <source src={alt_src} type="audio/mpeg">
        Audio not supported on this browser
    </audio>

    <button id="playback-control" on:click={togglePlayPause}>
        <img src="/src/visual-assets/buttons/{!isPlaying}.webp" alt={isPlaying ? 'Pause' : 'Play'}>
    </button>

    <div id="audio-slider">
        <input type="range" id="volume" min="0" max="1" step="0.01" bind:value={playerVolume}>
    </div>
{/if}