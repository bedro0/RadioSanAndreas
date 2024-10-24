<script>
    export let nowPlaying = { artist: "" , title: "" , path: ""};
</script>

<div class="now-playing">
    {#if ["(Intro)", "(Mid)", "(Outro)"].some(suffix => nowPlaying.path.includes(suffix))}
        <h3>NOW PLAYING</h3>
        <div class="playing-artist"><div id="artist"><span/></div> &emsp; {nowPlaying.artist}</div>
        <div class="playing-track"><div id="title"><span/></div> &emsp; {nowPlaying.title}</div>
    {:else}
        <h3>BREAK</h3>
    {/if}
</div>

<style lang="scss">
    @import "../../player.scss";
</style>