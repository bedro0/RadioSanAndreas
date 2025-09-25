<svelte:head>
    <title>GTA:SA Radio</title>
</svelte:head>
<script>
    import { goto } from '$app/navigation';
    let { data } = $props();
    const logos = "/assets/logos"
    const enabledStations = Object.keys(data.enabledStations);
    const all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur", "wctr"];
    let activeStates = $state(Array(enabledStations.length).fill(false));

    const setHover = (index, state) => {
        activeStates = activeStates.map((active, i) => (i === index ? state : false));
    }
</script>
<!--
<div 
style="font-size: clamp(16pt, 1.5vw, 20pt);
margin:32px;
text-shadow: 1px 1px 1px #000;
color:cyan;
font-family:monospace;
text-align:center;" >
Welcome to Grand Theft Auto: San Andreas radio!<br>
Please pick a channel.
</div>
-->
<div id="stations">
    {#each enabledStations as station, index}
    <button 
    class:hover={activeStates[index]}
    onmouseenter={() => setHover(index, true)} 
    onmouseleave={() => setHover(index, false)} 
    onclick={() => goto(station)}
    >
        <img src="{logos}/{station}.webp" alt={station}>

    </button>
    {/each}
</div>

<style lang="scss">
    @use "home.scss";
</style>