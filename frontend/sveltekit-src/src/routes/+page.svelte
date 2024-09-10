<svelte:head>
    <title>{title}</title>
</svelte:head>

<script>
    import { goto } from '$app/navigation'
    import {tick} from 'svelte';
    let title="Radio San Andreas";
    let all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];
    let activeStates = Array(all_stations.length).fill(false);
    let hover=false;
    
    function setHover(index, state) {
		activeStates = activeStates.map((active, i) => i === index ? state : hover);
	}
</script>

<style lang="scss">
    @import "../main.scss";
</style>

<div id="stations">
    {#each all_stations as station, index}
    <button 
    class:hover={activeStates[index]}
    on:mouseenter={() => setHover(index, true)} 
    on:mouseleave={() => setHover(index, false)} 
    on:click={() => goto(station)}
    >
        <img src="/src/visual-assets/logos/{station}.webp" alt={station}>

    </button>
    {/each}
</div>
