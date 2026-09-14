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

  async function save(): Promise<void> {
    error = "";

    try {
      const parsed: Config = JSON.parse(jsonText);

      if (
        typeof parsed !== "object" ||
        parsed === null ||
        typeof parsed.version !== "number" ||
        !Array.isArray(parsed.macros)
      ) {
        throw new Error("Invalid configuration format.");
      }

      onConfigChange(parsed);
      await onSave();
    } catch (err) {
      error = err instanceof Error
        ? err.message
        : "Invalid JSON configuration.";
    }
  }
</script>

<div class="json-page">
  <div class="json-header">
    <h2>JSON</h2>

    <button class="primary" onclick={save}>
      Save Changes
    </button>
  </div>

  {#if error}
    <p class="error">{error}</p>
  {/if}

  <JsonEditor
    value={jsonText}
    onChange={updateJson}
  />
</div>
