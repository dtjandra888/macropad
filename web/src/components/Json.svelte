<script lang="ts">
    import type { Config } from "../types";
    import JsonEditor from "./JsonEditor.svelte";

    export let config: Config;
    export let onConfigChange: (config: Config) => void;
    export let onSave: () => Promise<void>;

    let jsonText = JSON.stringify(config, null, 2);
    let error = "";

    function updateJson(value: string): void {
        jsonText = value;
    }

    function isValidConfig(value: unknown): value is Config {
        if (typeof value !== "object" || value === null) {
            return false;
        }

        const config = value as Record<string, unknown>;

        if (!Array.isArray(config.macros)) {
            return false;
        }

        return config.macros.every((macro) => {
            if (typeof macro !== "object" || macro === null) {
                return false;
            }

            const macroObject = macro as Record<string, unknown>;

            if (!Array.isArray(macroObject.strokes)) {
                return false;
            }

            return macroObject.strokes.every((stroke) => {
                if (typeof stroke !== "object" || stroke === null) {
                    return false;
                }

                const strokeObject = stroke as Record<string, unknown>;

                if (typeof strokeObject.key !== "string") {
                    return false;
                }

                if (!Array.isArray(strokeObject.modifiers)) {
                    return false;
                }

                return strokeObject.modifiers.every(
                    (modifier) => typeof modifier === "string",
                );
            });
        });
    }

    async function save(): Promise<void> {
        error = "";

        try {
            const parsed: unknown = JSON.parse(jsonText);

            if (!isValidConfig(parsed)) {
                throw new Error("Invalid configuration format.");
            }

            onConfigChange(parsed);
            await onSave();
        } catch (err) {
            error =
                err instanceof Error
                    ? err.message
                    : "Invalid JSON configuration.";
        }
    }
</script>

<div class="json-page">
    <div class="json-header">
        <h2>JSON 1</h2>

        <button class="primary" onclick={save}> Save Changes </button>
    </div>

    {#if error}
        <p class="error">{error}</p>
    {/if}

    <JsonEditor value={jsonText} onChange={updateJson} />
</div>
