<script>
    import { onMount, onDestroy } from 'svelte';
    import { page } from '$app/stores';
    import { browser } from '$app/environment';
    import { goto } from '$app/navigation';
    import enabledStations from "/radiosa/enabled_stations.json";

    const currentStation = $page.params.radioStation;
    let currentStationMetadata = updateMetadata();
    let nowPlaying = { artist: '', title: '' };
    let remainingTime = 0;
    let timeoutId;

    if (browser && !enabledStations.includes(currentStation)) goto("/");

    let playerObj;
    let playerVolume = 1;
    let isPlaying = false;

    const getAudioSrc = (format) => `/api/radio?format=${format}&station=${currentStation}&nocache=${Date.now()}`;

    function togglePlayPause() {
        isPlaying = !isPlaying;
        if (isPlaying) {
            playerObj.src = getAudioSrc('ogg');
            playerObj.play();
        } else {
            playerObj.pause();
        }
    }

    async function updateMetadata() {
        const response = await fetch(`/api/station-metadata?station=${currentStation}`);
        return (await response.json()).currentStationMetadata;
    }

    async function updateNowPlaying() {
        try {
            const response = await fetch(`/api/now-playing?station=${currentStation}`);
            const data = await response.json();
            nowPlaying = data.nowPlaying;
            remainingTime = data.remainingTime;
        } catch (error) {
            console.error('Failed to fetch now playing data:', error);
        } finally {
            scheduleNextUpdate();
        }
    }

    function scheduleNextUpdate() {
        clearTimeout(timeoutId);
        timeoutId = setTimeout(updateNowPlaying, Math.max(1000, (remainingTime || 5) * 1000));
    }

    onMount(() => {
        updateNowPlaying();
    });

    onDestroy(() => clearTimeout(timeoutId));
</script>

<svelte:head>
    {#await currentStationMetadata}
        <title>Radio San Andreas</title>
    {:then metadata}
        <title>{metadata.station_name}</title>
        <link rel="icon" href="/src/visual-assets/logos/{currentStation}.webp" />
    {/await}
</svelte:head>

{#await currentStationMetadata then metadata}
    <div class="radio-info">
        <div style="padding-bottom: 1em;">You are listening to: <br> {metadata.station_name} ({metadata.genre}) <br> Host: {metadata.host}</div>
        <br>
        {#if nowPlaying.title.includes("(ID)") || nowPlaying.title.includes("(DJ)") || nowPlaying.title.includes("(Caller)")}
            <div id="track">BREAK</div>
        {:else}
            <div style="color:greenyellow;">NOW PLAYING</div>
            <div id="artist" style="display: flex; justify-content: space-between;">ARTIST <artist>{nowPlaying.artist}</artist></div><br>
            <div id="track" style="display: flex; justify-content: space-between;">TRACK <song>{nowPlaying.title}</song></div>
        {/if}
    </div>
{/await}

<style lang="scss">
    @import "../../player.scss";
</style>

<img id="background-image" src="src/visual-assets/backgrounds/{currentStation}.jpg" alt={currentStation}>

<audio bind:this={playerObj} bind:volume={playerVolume} on:play={() => isPlaying = true} on:pause={() => isPlaying = false}>
    <source src={getAudioSrc('ogg')} type="audio/ogg">
    <source src={getAudioSrc('mp3')} type="audio/mpeg">
    Audio not supported on this browser
</audio>

<div class="audio-controls">
    <button id="play-button" on:click={togglePlayPause}>
        <img src="/src/visual-assets/buttons/{!isPlaying}.webp" alt={isPlaying ? 'Pause' : 'Play'}>
    </button>
    <br>
    <input id="volume" type="range" min="0" max="1" step="0.01" bind:value={playerVolume}>
</div>