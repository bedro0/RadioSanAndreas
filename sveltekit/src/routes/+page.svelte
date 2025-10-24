<svelte:head>
    <title>GTA:SA Radio</title>
</svelte:head>
<script>
    let menuHover, menuSelect = $state();
    import { goto } from '$app/navigation';
    import { onMount } from 'svelte';
    let { data } = $props();
    const logos = "/assets/logos"
    const enabledStations = Object.keys(data.enabledStations);
    const all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur", "wctr"];
    let activeStates = $state(Array(enabledStations.length).fill(false));

    const setHover = (index, state) => {
        activeStates = activeStates.map((active, i) => (i === index ? state : false));
    }
    onMount(()=>{
        menuHover.volume = 0.125;
        menuSelect.volume = 0.125;
    });
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
        onmouseenter={() => {
            setHover(index, true)
            const hoverSound = menuHover.cloneNode(true);
            hoverSound.volume = menuHover.volume;
            hoverSound.play();
            }
        } 
        onmouseleave={() => {

            setHover(index, false)
            }
        } 
        onclick={
            async () => {
                menuSelect.play();
                await new Promise(resolve => menuSelect.onended=resolve);
                goto(station)
            }
        }
    >
        <img src="{logos}/{station}.webp" alt={station}>

    </button>
    {/each}
    <audio src="/assets/menu/menu_hover.wav" bind:this={menuHover} preload="auto"></audio>
    <audio src="/assets/menu/menu_select.wav" bind:this={menuSelect} preload="auto"></audio>
</div>

<style lang="scss">
    @use "home.scss";
</style>