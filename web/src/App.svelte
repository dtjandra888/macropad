<script lang="ts">
    import Macros from "./components/Macros.svelte";
    import Json from "./components/Json.svelte";
    import About from "./components/About.svelte";

    import type { Config } from "./types";

    type Page = "macros" | "json" | "about";

    let page: Page = "macros";

    let config: Config = {
        macros: [],
    };

    let loading = true;
    let error = "";

    async function loadConfig(): Promise<void> {
        try {
            const response = await fetch("/api/config");

            if (!response.ok) {
                throw new Error("Failed to load configuration.");
            }

            config = await response.json();
        } catch (err) {
            error =
                err instanceof Error
                    ? err.message
                    : "Failed to load configuration.";
        } finally {
            loading = false;
        }
    }

    async function saveConfig(): Promise<void> {
        const response = await fetch("/api/config", {
            method: "POST",
            headers: {
                "Content-Type": "application/json",
            },
            body: JSON.stringify(config),
        });

        if (!response.ok) {
            throw new Error("Failed to save configuration.");
        }
    }

    function setPage(newPage: Page): void {
        page = newPage;
    }

    loadConfig();
</script>

<svelte:head>
    <title>Macropad Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
</svelte:head>

<div class="app">
    <header class="header">
        <div>
            <h1>Macropad</h1>

            {#if loading}
                <span class="status">Loading...</span>
            {:else if error}
                <span class="status error">Disconnected</span>
            {:else}
                <span class="status">Connected</span>
            {/if}
        </div>

        <nav class="toolbar">
            <button
                class:active={page === "macros"}
                onclick={() => setPage("macros")}
            >
                Macros
            </button>

            <button
                class:active={page === "json"}
                onclick={() => setPage("json")}
            >
                JSON
            </button>

            <button
                class:active={page === "about"}
                onclick={() => setPage("about")}
            >
                About
            </button>
        </nav>
    </header>

    <main>
        {#if loading}
            <p>Loading configuration...</p>
        {:else if page === "macros"}
            <Macros macros={config.macros} onSave={saveConfig} />
        {:else if page === "json"}
            <Json
                {config}
                onConfigChange={(newConfig) => (config = newConfig)}
                onSave={saveConfig}
            />
        {:else if page === "about"}
            <About />
        {/if}
    </main>
</div>
