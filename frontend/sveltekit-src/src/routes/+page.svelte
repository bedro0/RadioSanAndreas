<svelte:head>
    <title>{title}</title>
</svelte:head>

<script lang="ts">
    let showAudio = false;
    let audio: any;
    let title="Grove Street Radio";
    let favicon="gsr.png";
    let all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];
    let main_src, alt_src;
    function playAudio(station){
        main_src = `https://icecast.loonartech.net/${station}.ogg`;
        alt_src = `https://icecast.loonartech.net/${station}.mp3`
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
        <source src="{main_src}" type="audio/ogg"/>
        <source src="{alt_src}" type="audio/mpeg"/>
        "Audio not supported on this browser"
    </audio>
{/if}