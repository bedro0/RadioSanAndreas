<script>
    export let data;
    import { onMount, onDestroy } from "svelte";
    import { browser } from "$app/environment";
    import { goto } from "$app/navigation";
    import { page } from "$app/stores";
    import WCTRNowPlaying from "./wctr-now-playing.svelte"
    import MusicNowPlaying from "./music-now-playing.svelte"
    const currentStation = $page.params.radioStation;
    const enabledStations = data.enabledStations;
    if ( browser && !(currentStation in enabledStations) ) goto("/");

    const currentStationData = enabledStations[currentStation];
    let nowPlaying = { artist: "" , title: "" , path: ""};
    let remainingTime = 0;
    let timeoutID;

    let playerObj;
    let playerVolume = 1;
    let isAudio = true;
    let isPaused = true;
    const getAudioSrc = (format) => `/api/radio?format=${format}&station=${currentStation}&nocache=${Date.now()}`;

    const togglePlayPause = () => {
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

    const updateNowPlaying = async () => {
        try {
            ({ nowPlaying, remainingTime } = await fetch(`/api/now-playing?station=${currentStation}`).then(response => response.json()));
        }
        catch (error) {
            console.error("Failed to fetch Now-Playing data:", error);
        } 
        finally {
            timeoutID = setTimeout(updateNowPlaying, 1000*(Math.max(1, remainingTime)));
        }
    }

    onMount(() => {
        updateNowPlaying();
    });

    onDestroy(() => {
        clearTimeout(timeoutID)
    })
</script>

<svelte:head>
    <title>{currentStationData.channel_name}</title>
    <link rel="icon" href="/visual-assets/logos/{currentStation}.webp" />
</svelte:head>

<div class="player">
    <div class="channel-info">
        <h2>{currentStationData.channel_name}</h2>
        <img class="logo" src="/visual-assets/logos/{currentStation}.webp" alt={currentStation}>
        <p>Genre: {currentStationData.genre}</p>
        <p>Host: {currentStationData.host}</p>
    </div>
    {#if (currentStation==="wctr")}
    <WCTRNowPlaying nowPlaying = {nowPlaying}/>
    {:else}
    <MusicNowPlaying nowPlaying={nowPlaying}/>
    {/if}
    <div class="controls">
        <div class="buttons">
            <button on:click={() => goto("/")}>
                <img src="/visual-assets/buttons/back.webp" alt="Back">
            </button>
    
            <button on:click={togglePlayPause}>
                <img src="/visual-assets/buttons/{!isAudio || isPaused}.webp" alt="{(!isAudio || isPaused) ? 'Play' : 'Pause'}">
            </button>
        </div>
        <input type="range" min="0" max="1" step="0.01" bind:value={playerVolume}>
    </div>
</div>
{#if isAudio}
    <audio bind:this={playerObj} bind:volume={playerVolume} bind:paused={isPaused} autoplay>
        <source src={getAudioSrc("ogg")}>
        <source src={getAudioSrc("mp3")}>
    </audio>
{/if}

<style lang="scss">
    @import "../../player.scss";
</style>