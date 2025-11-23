<script>
	import favicon from '$lib/assets/favicon.webp';
    import logoSA from "$lib/assets/san_andreas_logo.webp"
    import ActiveUsers from "./ActiveUsers.svelte";
    import { page } from "$app/stores"
    import { onMount, setContext } from "svelte";
    import { POLLING_RATE_MS } from "$lib/pollingRate.js"
	let { children } = $props();

    let userAnalytics = $state({})
    let activeUsers = $derived(userAnalytics.activeUsers);
    const returnUserAnalytics = () => userAnalytics;
    setContext("userAnalytics", returnUserAnalytics)
    async function updateUserAnalytics({ urlPath }){
        userAnalytics = await fetch("/api/analytics", {
            method: "POST",
            body: JSON.stringify({ currentUserURLPath: urlPath }),
            headers: {
            "Content-Type": "application/json"
            }
        }).then(respone => respone.json());
    }

    async function analyticsLoop(){
        while (true){
            await updateUserAnalytics({ urlPath: $page.url.pathname});
            await new Promise((resolve)=>{
                setTimeout(resolve, POLLING_RATE_MS)
            })
        }
    }

    onMount(() => {
        analyticsLoop();
    })
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

<nav>
    <div>
        <a href="/"><img src={logoSA} alt="home" style="width: 7em"/></a>
    </div>
    <ActiveUsers {activeUsers}/>
    <div id="left">
        <a href="/about">about</a>
        <a href="https://github.com/bedro0/RadioSanAndreas">github</a>
    </div>
</nav>

<style>
    nav{
        display: flex;
        justify-content: space-between;
        align-items: center;
        padding: 1% 1%;
        font-size: clamp(11pt, 1.5vw, 20pt);
        margin:0 5%;
        font-family:monospace;
        a{
            text-decoration:none;
            color:cyan;
        }
    }
    #left {
        white-space: nowrap;
        display: inline;
        a{
            padding: 0 8px;
        }
    }
</style>

{@render children?.()}