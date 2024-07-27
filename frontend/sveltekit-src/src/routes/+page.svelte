<svelte:head>
    <title>{title}</title>
</svelte:head>

<script lang="ts">
    import {tick} from 'svelte';
    let showAudio = false;
    let audio: any;
    let title="Grove Street Radio";
    let favicon="gsr.png";
    let all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];
    let main_src, alt_src;
    async function playAudio(station){
        showAudio = false;
        await tick();
        main_src = `https://icecast.loonartech.net/${station}.ogg?nocache=${new Date().getTime()}`;
        alt_src = `https://icecast.loonartech.net/${station}.mp3?nocache=${new Date().getTime()}`
        showAudio = true;
    }
</script>

<style lang="scss">
    @import "../main.scss";
</style>

<div class="radio-buttons">
    {#each all_stations as station}
    <script lang="ts">

    </script>
    <button on:click={() => playAudio(station)}>
        <img src="/src/visual-assets/{station}.png" alt={station}>

    </button>
    {/each}
</div>

{#if showAudio}
    <audio autoplay bind:this={audio}>
        <source bind:this="{main_src}" type="audio/ogg"/>
        <source src="{alt_src}" type="audio/mpeg"/>
        "Audio not supported on this browser"
    </audio>
{/if}