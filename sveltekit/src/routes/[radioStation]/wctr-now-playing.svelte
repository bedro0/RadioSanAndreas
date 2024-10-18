<script>
    export let nowPlaying = { artist: "" , title: "" , path: ""};
</script>

<div class="now-playing">
    {#if ["(Intro)", "(Mid)", "(Outro)"].some(suffix => nowPlaying.path.includes(suffix))}
        <h3>NOW PLAYING</h3>
        <p><strong>ARTIST:</strong> {nowPlaying.artist}</p>
        <p><strong>TRACK:</strong> {nowPlaying.title}</p>
    {:else}
        <h3>BREAK</h3>
    {/if}
</div>

<style lang="scss">
    @import "../../player.scss";
</style>