<script lang="ts">
    import type { Macro } from "../types";

    export let macros: Macro[] = [];
    export let onSave: () => Promise<void>;

    // e.g. "HID_MOD_LSHIFT + HID_W"; HID_NONE is left out for modifier-only holds
    function holdLabel(macro: Macro): string {
        const { key, modifiers } = macro.strokes[0];
        const parts = key === "HID_NONE" ? modifiers : [...modifiers, key];
        return parts.join(" + ");
    }
</script>

<section>
    <div class="page-heading">
        <div>
            <h2>Macros</h2>
            <p>Configure what each key does.</p>
        </div>

        <button class="primary" onclick={onSave}> Save Changes </button>
    </div>

    <div class="macro-grid">
        {#each macros as macro, index}
            <article class="macro-card">
                <div class="key">
                    {index}
                </div>

                <div class="macro-info">
                    <h3>Key {index}</h3>

                    {#if macro.strokes.length === 0}
                        <p>No macro assigned</p>
                    {:else if macro.mode === "hold"}
                        <p>Hold: {holdLabel(macro)}</p>
                    {:else}
                        <p>{macro.strokes.length} keystroke(s)</p>
                    {/if}
                </div>

                <button class="edit-button"> Edit </button>
            </article>
        {/each}
    </div>
</section>
