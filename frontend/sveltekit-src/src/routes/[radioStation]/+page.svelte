<script>
    import { browser } from '$app/environment'
    import { goto } from '$app/navigation'
    import { page } from '$app/stores';
    import station_metadata_json from "/src/station_metadata.json";

    const station=$page.params.radioStation;
    let playerObj;
    let playerVolume = 1;
    if (station_metadata_json[station] === undefined && browser){
        goto("/")
    }
    let main_src = `https://icecast.loonartech.net/${station}.ogg?nocache=${new Date().getTime()}`;
    let alt_src = `https://icecast.loonartech.net/${station}.mp3?nocache=${new Date().getTime()}`;
    let favicon = `/src/visual-assets/logos/${station}.webp`;
    let bool_currently_playing = true;
    function play_pause_audio(){
        bool_currently_playing = !bool_currently_playing;
    }

</script>

<svelte:head>
    {#if (station_metadata_json[station] !== undefined)}
    <title>{station_metadata_json[station].station_name}</title>
    <link rel="icon" href={favicon}/>
    {/if}
</svelte:head>

{#if (station_metadata_json[station] !== undefined)}
<style lang="scss">
    @import "../../player.scss";
</style>

<img id=background-image src="src/visual-assets/backgrounds/{station}.jpg" alt={station}>

{#if bool_currently_playing}
<audio autoplay bind:this={playerObj} bind:volume="{playerVolume}">
    <source src="{main_src}" type="audio/ogg"/>
    <source src="{alt_src}" type="audio/mpeg"/>
    "Audio not supported on this browser"
</audio>
{/if}
<button id=playback-control  on:click={() => play_pause_audio()}>
    <img src="/src/visual-assets/buttons/{!bool_currently_playing}.webp" alt={station}>
</button>

<div id=audio-slider>
    <input type="range" id=volume min="0" max="1" step="0.01" bind:value={playerVolume}>
</div>
{/if}