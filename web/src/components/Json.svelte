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
            console.log("Invalid: config is not an object");
            return false;
        }

        const config = value as Record<string, unknown>;

        if (!Array.isArray(config.macros)) {
            console.log("Invalid: macros is not an array");
            return false;
        }

        for (const macro of config.macros) {
            if (typeof macro !== "object" || macro === null) {
                console.log("Invalid: macro is not an object");
                return false;
            }

            const macroObject = macro as Record<string, unknown>;

            if (!Array.isArray(macroObject.strokes)) {
                console.log("Invalid: strokes is not an array");
                return false;
            }

            for (const stroke of macroObject.strokes) {
                if (typeof stroke !== "object" || stroke === null) {
                    console.log("Invalid: stroke is not an object");
                    return false;
                }

                const strokeObject = stroke as Record<string, unknown>;

                if (typeof strokeObject.key !== "string") {
                    console.log(
                        "Invalid: key is not a string",
                        strokeObject.key,
                    );
                    return false;
                }

                if (!Array.isArray(strokeObject.modifiers)) {
                    console.log(
                        "Invalid: modifiers is not an array",
                        strokeObject.modifiers,
                    );
                    return false;
                }

                for (const modifier of strokeObject.modifiers) {
                    if (typeof modifier !== "string") {
                        console.log(
                            "Invalid: modifier is not a string",
                            modifier,
                        );
                        return false;
                    }
                }
            }
        }
        return true;
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
        <h2>JSON 1</h2>

        <button class="primary" onclick={save}> Save Changes </button>
    </div>

    {#if error}
        <p class="error">{error}</p>
    {/if}

    <JsonEditor value={jsonText} onChange={updateJson} />
</div>
