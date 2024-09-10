<svelte:head>
    <title>{title}</title>
</svelte:head>
<style lang="scss">
    @import "../../stations.scss";
</style>
<script>
    import { page } from '$app/stores';
    const station=$page.params.radioStation
    let title=station;
    let playerObj;
    let playerVolume = 1;
    let main_src = `https://icecast.loonartech.net/${station}.ogg?nocache=${new Date().getTime()}`;
    let alt_src = `https://icecast.loonartech.net/${station}.mp3?nocache=${new Date().getTime()}`
</script>
<img id=background-image src="src/visual-assets/backgrounds/{station}.jpg" alt={station}>
<audio autoplay bind:this={playerObj} bind:volume="{playerVolume}">
    <source src="{main_src}" type="audio/ogg"/>
    <source src="{alt_src}" type="audio/mpeg"/>
    "Audio not supported on this browser"
</audio>
<div id=controls>
    <input type="range" id=volume min="0" max="1" step="0.01" bind:value={playerVolume}>
</div>