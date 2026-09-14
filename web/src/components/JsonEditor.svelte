<script lang="ts">
  import { onMount } from "svelte";
  import { EditorState } from "@codemirror/state";
  import { EditorView } from "@codemirror/view";
  import { basicSetup } from "codemirror";
  import { json } from "@codemirror/lang-json";

  export let value: string;
  export let onChange: (value: string) => void;

  let editor: HTMLDivElement;

  onMount(() => {
    const state = EditorState.create({
      doc: value,
      extensions: [
        basicSetup,
        json(),
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

<div bind:this={editor}></div>
