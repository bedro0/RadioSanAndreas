
<svelte:head>
    <title>{"GTA:SA ("+currentStationData.channel_name+")"}</title>
</svelte:head>

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

    let isSong = $derived(["Intro", "Mid", "Outro"].some(suffix => nowPlaying.path.includes(suffix)))
    $effect(() => {
        if (browser) localStorage.setItem("volume", playerVolume)
        });

    let isPaused = $state(true);
    let sources = $derived({ogg:"", mp3:""});

    const togglePlayPause = () => {
        isPaused = !isPaused
    }

    let allowSetSources = $state(true);

    const syncSources = () => {
        if (!allowSetSources) return;
        let unixstamp = Date.now()
        sources = ({
            ogg: `/api/radio?format=${"ogg"}&station=${currentStation}&nocache=${unixstamp}`,
            mp3: `/api/radio?format=${"mp3"}&station=${currentStation}&nocache=${unixstamp}`
        });
        setSources();
        allowSetSources = false;
        setTimeout(()=>{allowSetSources=true}, 2000)
    }
    const setSources = () => {
        if (playerObj.canPlayType("audio/mpeg")){
            playerObj.setAttribute("src", sources.mp3)
        }
        else if(playerObj.canPlayType("audio/ogg")) {
            playerObj.setAttribute("src", sources.ogg);
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
        if ("mediaSession" in navigator) {
            navigator.mediaSession.metadata = new MediaMetadata({
                title: isSong ? nowPlaying.title : "BREAK",
                artist: isSong ? nowPlaying.artist : "",
                album: currentStationData.channel_name,
                artwork: [{ src: `/assets/logos/${currentStation}.webp` }],
            });
        }
    }

    onMount(() => {
        updateNowPlaying();
        if ("mediaSession" in navigator){
            navigator.mediaSession.setActionHandler("pause", ()=>{isPaused=true});
            navigator.mediaSession.setActionHandler("play", ()=>{isPaused = false});
        }
        syncSources()
        setSources()
    });

    onDestroy(() => {
        clearTimeout(timeoutID)
    })
</script>

<div class="player">
    <div class="channel-info">
        <h2 style="padding-top:0.5ch;">{currentStationData.channel_name}</h2>
        <img class="logo" src="/assets/logos/{currentStation}.webp" alt={currentStation}>
        <p>Genre: {currentStationData.genre}</p>
        <p>Host: {currentStationData.host}</p>
    </div>
    <div class="now-playing" style="margin-bottom:0.5ch;">
        {#if isSong}
            <h3>NOW PLAYING</h3>
            <div class="playing-artist"><div id="artist"> <span></span> </div> &nbsp; {nowPlaying.artist}</div>
            <div class="playing-track"><div id="track"> <span></span> </div> &nbsp; {nowPlaying.title}</div>
        {:else}
            <h3>BREAK</h3>
        {/if}
    </div>

    <div class="controls">
        <div class="buttons">
            <button onclick={togglePlayPause}>
                <img src="/assets/buttons/{isPaused}.webp" alt="{(isPaused) ? 'Play' : 'Pause'}">
            </button>
        </div>
        <input type="range" style="padding-bottom: 1ch;" min="0" max="1" step="0.01" bind:value={playerVolume}>
    </div>
</div>

<audio bind:this={playerObj} bind:volume={playerVolume} bind:paused={isPaused} onplay={syncSources} autoplay></audio>


<style lang="scss">
    @use "player.scss";
</style>