<svelte:head>
    <title>{title}</title>
</svelte:head>

<script lang="ts">
    import {tick} from 'svelte';
    let playerVolume = 1;
    let playerObj;
    let audio_playing = false;
    let title="Grove Street Radio";
    let favicon="gsr.png";
    let all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];
    let activeStates = Array(all_stations.length).fill(false);
    let main_src, alt_src;
    let hover=false;
    let station_playing;


    async function playAudio(station){
        audio_playing = false;
        await tick();
        main_src = `https://icecast.loonartech.net/${station}.ogg?nocache=${new Date().getTime()}`;
        alt_src = `https://icecast.loonartech.net/${station}.mp3?nocache=${new Date().getTime()}`
        audio_playing = true;
        station_playing = station;
    }
    function setHover(index, state) {
		activeStates = activeStates.map((active, i) => i === index ? state : hover);
	}
</script>

<style lang="scss">
    @import "../main.scss";
</style>
{#if !audio_playing}
    <div id="stations">
        {#each all_stations as station, index}
        <script lang="ts">

        </script>
        <button 
        class:hover={activeStates[index]}
        on:mouseenter={() => setHover(index, true)} 
        on:mouseleave={() => setHover(index, false)} 
        on:click={() => playAudio(station)}
        >
            <img src="/src/visual-assets/logos/{station}.png" alt={station}>

        </button>
        {/each}
    </div>
{/if}
<!--
<button id="stop" on:click={() => playerObj.play()}>Play</button>
<button id="stop" on:click={() => playerObj.pause()}>Pause</button>
-->

{#if audio_playing}
    <img id=background-image src="src/visual-assets/backgrounds/{station_playing}.jpg" alt={station_playing}>
    <audio autoplay bind:this={playerObj} bind:volume="{playerVolume}">
        <source src="{main_src}" type="audio/ogg"/>
        <source src="{alt_src}" type="audio/mpeg"/>
        "Audio not supported on this browser"
    </audio>
    <div id=controls>
        <input type="range" id=volume min="0" max="1" step="0.01" bind:value={playerVolume}>
    </div>
{/if}