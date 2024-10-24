<script>
    export let nowPlaying = { artist: "" , title: "" , path: ""};
</script>

<div class="now-playing">
    {#if ["(ID)", "(DJ)", "(Caller)", "(Story)", "(Atmosphere)"].some(suffix => nowPlaying.path.includes(suffix))}
    <h3>BREAK</h3>
    {:else}
    <h3>NOW PLAYING</h3>
    <p> {nowPlaying.artist} </p>
    <p> {nowPlaying.title} </p>
    {/if}
</div>

<style lang="scss">
    @import "../../player.scss";
</style>