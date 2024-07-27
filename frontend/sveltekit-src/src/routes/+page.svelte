<svelte:head>
    <title>{title}</title>
</svelte:head>

<script lang="ts">
    let title="Grove Street Radio";
    let favicon="gsr.png";
    let all_stations=["bouncefm", "csr", "kdst", "kjah", "krose", "mastersounds", "playbackfm", "radiols", "radiox", "sfur"];
</script>

<style lang="scss">
    @import "../main.scss";
</style>

<div class="radio-buttons">
    {#each all_stations as station}
    <button>{station}</button>
    {/each}
</div>