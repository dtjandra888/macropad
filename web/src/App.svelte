<script lang="ts">
    import Macros from "./components/Macros.svelte";
    import Json from "./components/Json.svelte";
    import About from "./components/About.svelte";

    type Page = "macros" | "json" | "about";

    let page: Page = "macros";
</script>

<svelte:head>
    <title>Macropad Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
</svelte:head>

<div class="app">
    <header class="header">
        <div>
            <h1>Macropad</h1>
            <span class="status">Connected</span>
        </div>

        <nav class="toolbar">
            <button
                class:active={page === "macros"}
                onclick={() => (page = "macros")}
            >
                Macros
            </button>

            <button
                class:active={page === "json"}
                onclick={() => (page = "json")}
            >
                JSON
            </button>

            <button
                class:active={page === "about"}
                onclick={() => (page = "about")}
            >
                About
            </button>
        </nav>
    </header>

    <main>
        {#if page === "macros"}
            <Macros />
        {:else if page === "json"}
            <Json />
        {:else if page === "about"}
            <About />
        {/if}
    </main>
</div>
