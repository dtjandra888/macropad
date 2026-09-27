<script lang="ts">
    import type { Config } from "../types";
    import JsonEditor from "./JsonEditor.svelte";
    import { isValidConfig } from "../lib/config";

    export let config: Config;
    export let onConfigChange: (config: Config) => void;
    export let onSave: () => Promise<void>;

    let jsonText = JSON.stringify(config, null, 2);
    let error = "";

    function updateJson(value: string): void {
        jsonText = value;
    }

    async function save(): Promise<void> {
        error = "";

        try {
            const parsed: unknown = JSON.parse(jsonText);

            console.log("JSON being saved:", parsed);
            console.log("Valid config:", isValidConfig(parsed));

            if (!isValidConfig(parsed)) {
                throw new Error("Invalid configuration format.");
            }

            onConfigChange(parsed);
            await onSave();
        } catch (err) {
            console.error("Save error:", err);

            error =
                err instanceof Error
                    ? err.message
                    : "Invalid JSON configuration.";
        }
    }
</script>

<div class="json-page">
    <div class="json-header">
        <h2>JSON</h2>

        <button class="primary" onclick={save}> Save Changes </button>
    </div>

    {#if error}
        <p class="error">{error}</p>
    {/if}

    <JsonEditor value={jsonText} onChange={updateJson} />
</div>
