<script lang="ts">
  import { onMount } from "svelte";
  import { EditorState } from "@codemirror/state";
  import { EditorView, basicSetup } from "codemirror";
  import { json, jsonParseLinter } from "@codemirror/lang-json";

  export let value: string;
  export let onChange: (value: string) => void;

  let editor: HTMLDivElement;

  onMount(() => {
    const state = EditorState.create({
      doc: value,
      extensions: [
        basicSetup,
        json(),
        jsonParseLinter(),

        EditorView.updateListener.of((update) => {
          if (update.docChanged) {
            onChange(update.state.doc.toString());
          }
        }),
      ],
    });

    const view = new EditorView({
      state,
      parent: editor,
    });

    return () => {
      view.destroy();
    };
  });
</script>

<div class="json-editor" bind:this={editor}></div>
