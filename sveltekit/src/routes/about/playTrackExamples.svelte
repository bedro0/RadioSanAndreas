<script>
    const {selectedStation} = $props();
    let channelMetadata = $state();
    let audioPlayer=$state();
    let playerVolume=$state();

    const setChannelCategories = async (channelMetadata) => {
        let categories=new Set(Object.keys(channelMetadata));
        if (categories.delete("Song")){
            categories.add("Song (Intro)");
            categories.add("Song (Outro)")
        }
        return categories;
    }
    export const setTrackSource = async (station, category, player) => {
    player.src = (`/api/getTrackByCategory?station=${encodeURIComponent(station)}&category=${category}`);
    player.play();
}
</script>
<style lang="scss">
    @use "$lib/blog.scss";
</style>
<div class="text" style="margin: 0">
    {#await import("/radiosa/scripts/metadata.mjs").then(
        async (obj) => {
            channelMetadata = obj[selectedStation].Content;
            return await setChannelCategories(channelMetadata);
        })}
    <p>Loading the player...</p>
    {:then categories}
        <div style="display: flex;">
            <p>Volume:</p>
            <input style="margin: 0 20px" type="range" min="0" max="1" step="0.01" bind:value={playerVolume}>
            <button style="margin: 0 200px;" onclick={()=>audioPlayer.pause()}>STOP</button>
        </div>
        <h3>Play a random example of ______</h3>
            <div style="display: flex;">
            {#each categories as cat}
                <button 
                onclick={ async ()=>{ setTrackSource(selectedStation, cat, audioPlayer) } } 
                style="margin: 0 0.5%; width: min(50%, 200px)">
                    {cat}
                </button><br>
            {/each}
            </div>
        <audio bind:this={audioPlayer} bind:volume={playerVolume}></audio>
    {/await}
</div>