<script lang="ts">
    import type { Macro } from "../types";

    export let macros: Macro[] = [];
    export let onSave: () => Promise<void>;
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
                    {:else}
                        <p>{macro.strokes.length} keystroke(s)</p>
                    {/if}
                </div>

                <button class="edit-button"> Edit </button>
            </article>
        {/each}
    </div>
</section>
