<script>
    let { activeUsers } = $props();
    let isUsersPlural = $derived("active user" + (activeUsers === 1 ? "" : "s"));
</script>
{#if activeUsers}
    <div>
        <div style="color:greenyellow">{activeUsers}</div>
        <div>{isUsersPlural}</div>
    </div>
{/if}
<style lang="scss">
    div {
        color: cyan;
        display:inline
    }
</style>