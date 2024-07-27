<svelte:head>
    <title>{title}</title>
</svelte:head>

<script lang="ts">
    import {tick} from 'svelte';
    let audio_playing = false;
    let title="Grove Street Radio";
    let favicon="gsr.png";
    let all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];
    let activeStates = Array(all_stations.length).fill(false);
    let main_src, alt_src;
    let hover=false;


    async function playAudio(station){
        audio_playing = false;
        await tick();
        main_src = `https://icecast.loonartech.net/${station}.ogg?nocache=${new Date().getTime()}`;
        alt_src = `https://icecast.loonartech.net/${station}.mp3?nocache=${new Date().getTime()}`
        audio_playing = true;
    }
    function setHover(index, state) {
		activeStates = activeStates.map((active, i) => i === index ? state : hover);
	}
</script>

<style lang="scss">
    @import "../main.scss";
</style>

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
        <img src="/src/visual-assets/{station}.png" alt={station}>

    </button>
    {/each}
</div>

<button on:click={() => audio_playing=false}>Pause</button>

{#if audio_playing}
    <audio autoplay>
        <source bind:this="{main_src}" type="audio/ogg"/>
        <source src="{alt_src}" type="audio/mpeg"/>
        "Audio not supported on this browser"
    </audio>
{/if}