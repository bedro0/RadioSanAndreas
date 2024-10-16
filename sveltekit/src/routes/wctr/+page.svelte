<script>
    export let data;
    import { onMount, onDestroy } from "svelte";
    import { browser } from "$app/environment";
    import { goto } from "$app/navigation";

    const enabledStations = JSON.parse(data.enabledStations);
    const currentStation = "wctr";
    let currentStationMetadata = {};
    let nowPlaying = { artist: "", title: "", path: "" };
    let remainingTime = 0;
    let timeoutId
    const isShow = ["(Intro)", "(Mid)", "(Outro)"];
    if (browser && !enabledStations.includes(currentStation)) goto("/");

    let playerObj;
    let playerVolume = 1;
    let isAudio = true;
    let isPaused = true;

    const getAudioSrc = (format) => `/api/radio?format=${format}&station=${currentStation}&nocache=${Date.now()}`;

    async function togglePlayPause(){
        if(isAudio){
            if(playerObj.paused){
                playerObj.play();
            }
            else{
                isAudio=false;
            }
        }
        else{
            isAudio=true;
        }
    }

    async function updateMetadata() {
        try{
            const responseBody = await fetch(`/api/station-metadata?station=${currentStation}`).then(response => response.json());
            currentStationMetadata = responseBody.currentStationMetadata;
        }
        catch(error){
            console.error("Failed to fetch current channel metadata:", error)
        }
    }

    async function updateNowPlaying() {
        try {
            const responseBody = await fetch(`/api/now-playing?station=${currentStation}`).then(response => response.json());
            nowPlaying = responseBody.nowPlaying;
            remainingTime = responseBody.remainingTime;
        } catch (error) {
            console.error("Failed to fetch Now-Playing data:", error);
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
        updateMetadata()
    });

    onDestroy(() => clearTimeout(timeoutId));
</script>

<svelte:head>
    {#await currentStationMetadata}
        <title>Radio San Andreas</title>
    {:then metadata}
        <title>{metadata.station_name}</title>
        <link rel="icon" href="/visual-assets/logos/{currentStation}.webp" />
    {/await}
</svelte:head>

<div id="allfather">
    {#await currentStationMetadata then metadata}
        <div id="channel-id">
            <div class="channel-id-member">You are listening to:</div>
            <div class="channel-id-member" id="station-name">{metadata.station_name}</div>
            <div>
                <img id="station-logo" src="/visual-assets/logos/{currentStation}.webp" alt={currentStation}/>
            </div>
            <div class="channel-id-member" id="station-genre"> <br> Genre: {metadata.genre} </div> 
            <div class="channel-id-member" id="host">Host: {metadata.host}</div>
        </div>

        <div class="currently-playing">
            {#if isShow.some(suffix => nowPlaying.path.includes(suffix))}
            <div id="now-playing">NOW PLAYING</div>
            <div class="display-artist">
                <div id="artist">ARTIST:</div>
                <div id="artist-id">{nowPlaying.artist}</div>
            </div>
            <div class="display-track-title">
                <div id=track-title>TRACK:</div>
                <div id="track-title-id" style="display: flex; justify-items: left;">{nowPlaying.title}</div>
            </div>
            {:else}
            <div id="break">BREAK</div>
            {/if}
        </div>
    {/await}

    <div id="audio-controls">
        <button id="play-button" on:click={togglePlayPause}>
            <img src="/visual-assets/buttons/{!isAudio || isPaused}.webp" alt="{(!isAudio || isPaused) ? "Play" : "Pause"}">
        </button>
        <br>
        <input id="volume" type="range" min="0" max="1" step="0.01" bind:value={playerVolume}>
    </div>
</div>

{#if isAudio}
<audio autoplay bind:this={playerObj} bind:volume={playerVolume} bind:paused={isPaused}>
    <source src={getAudioSrc("ogg")}/>
    <source src={getAudioSrc("mp3")}/>
    Audio is not supported on this browser!
</audio>
{/if}

<style lang="scss">
    @import "../../player.scss";
</style>