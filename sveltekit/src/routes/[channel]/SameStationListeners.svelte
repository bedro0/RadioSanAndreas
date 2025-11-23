<script>
    import { getContext } from "svelte";
    let userAnalytics = getContext("userAnalytics")
    let sameStationListeners = $derived(userAnalytics().sameStation)
    let isListenersPlural = $derived("current listener" + (sameStationListeners === 1 ? "" : "s"))
</script>

<div>
    <div style="color:greenyellow">{sameStationListeners}</div>
    <div>{isListenersPlural}</div>
</div>
<style lang="scss">
    div {
        color: cyan;
        display:inline
    }
</style>