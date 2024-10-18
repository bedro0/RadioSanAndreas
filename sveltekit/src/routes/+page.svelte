<svelte:head>
    <title>{title}</title>
    <link rel="icon" href={favicon} type="image/webp"/>
</svelte:head>

<script>
    import { goto } from '$app/navigation';
    export let data;

    const enabledStations = Object.keys(data.enabledStations);
    const title="Radio San Andreas";
    const favicon="/favicon.webp"
    const all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur", "wctr"];
    let activeStates = Array(enabledStations.length).fill(false);
    let hover=false;
    
    
    function setHover(index, state) {
		activeStates = activeStates.map((active, i) => i === index ? state : hover);
	}
</script>

<style lang="scss">
    @import "../main.scss";
</style>
{#await enabledStations then channels}
<div id="stations">
    {#each channels as station, index}
    <button 
    class:hover={activeStates[index]}
    on:mouseenter={() => setHover(index, true)} 
    on:mouseleave={() => setHover(index, false)} 
    on:click={() => goto(station)}
    >
        <img src="/visual-assets/logos/{station}.webp" alt={station}>

    </button>
    {/each}
</div>
{/await}
