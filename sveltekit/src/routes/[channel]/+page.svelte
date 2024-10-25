<script>
    let { data } = $props();
    import { onMount, onDestroy } from "svelte";
    import { goto } from "$app/navigation";
    import { page } from "$app/stores";
    import { browser } from "$app/environment";
    const currentStation = $page.params.channel;
    const currentStationData = data.currentStationObject;

    let nowPlaying = $state({path: ""});
    let remainingTime = $state(0);
    let timeoutID = $state();

    let playerObj = $state();
    let playerVolume = $state(browser && localStorage.getItem("volume") || 0.5);

    $effect(() => {
        if (browser) localStorage.setItem("volume", playerVolume)
        });

    let isAudio = $state(true);
    let isPaused = $state(true);

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
    <div class="now-playing">
        {#if ["Intro", "Mid", "Outro"].some(suffix => nowPlaying.path.includes(suffix))}
            <h3>NOW PLAYING</h3>
            <div class="playing-artist"><div id="artist"> <span></span> </div> {nowPlaying.artist}</div>
            <div class="playing-track"><div id="title"> <span></span> </div> {nowPlaying.title}</div>
        {:else}
            <h3>BREAK</h3>
        {/if}
    </div>

    <div class="controls">
        <div class="buttons">
            <button onclick={() => goto("/")}>
                <img src="/visual-assets/buttons/back.webp" alt="Back">
            </button>
    
            <button onclick={togglePlayPause}>
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