<script>
	import favicon from '$lib/assets/favicon.webp';
    import logoSA from "$lib/assets/san_andreas_logo.webp"
	let { children } = $props();
</script>

<svelte:head>
	<link rel="icon" href={favicon} />
</svelte:head>

<nav>
    <div>
        <a href="/"><img src={logoSA} alt="home" style="width: 7em"/></a>

    </div>
    <div id="left">
        <a href="/about">about</a>
        <a href="/blog">blog</a>
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
