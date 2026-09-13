<script lang="ts">
  import type { Macro } from "./Macros.svelte";

  export let macros: Macro[] = [];

  let jsonText = "";
  let jsonError = "";

  $: if (jsonText === "") {
    jsonText = JSON.stringify(macros, null, 2);
  }

  function applyJson(): void {
    try {
      const parsed: unknown = JSON.parse(jsonText);

      if (!Array.isArray(parsed)) {
        throw new Error("Configuration must be an array.");
      }

      for (const macro of parsed) {
        if (
          typeof macro !== "object" ||
          macro === null ||
          typeof macro.key !== "string" ||
          typeof macro.name !== "string" ||
          typeof macro.description !== "string"
        ) {
          throw new Error(
            "Each macro must contain key, name, and description strings."
          );
        }
      }

      macros = parsed as Macro[];
      jsonError = "";
    } catch (error) {
      if (error instanceof Error) {
        jsonError = error.message;
      } else {
        jsonError = "Invalid JSON.";
      }
    }
  }
</script>

<section>
  <div class="page-heading">
    <div>
      <h2>JSON</h2>
      <p>Edit the current macropad configuration.</p>
    </div>

    <button class="primary" onclick={applyJson}>
      Apply JSON
    </button>
  </div>

  <textarea
    class:error={jsonError !== ""}
    bind:value={jsonText}
    spellcheck="false"
    aria-label="Macropad configuration JSON"
  ></textarea>

  {#if jsonError}
    <p class="json-error">
      {jsonError}
    </p>
  {/if}
</section>
