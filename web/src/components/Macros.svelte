<script lang="ts">
  export interface Macro {
    key: string;
    name: string;
    description: string;
  }

  export let macros: Macro[] = [
    {
      key: "1",
      name: "Hello",
      description: 'Types "Hello"',
    },
    {
      key: "2",
      name: "Ctrl + Shift + C",
      description: "Copy",
    },
    {
      key: "3",
      name: "Unassigned",
      description: "No macro assigned",
    },
  ];
</script>

<section>
  <div class="page-heading">
    <div>
      <h2>Macros</h2>
      <p>Configure what each key does.</p>
    </div>

    <button class="primary">
      Save Changes
    </button>
  </div>

  <div class="macro-grid">
    {#each macros as macro}
      <article class="macro-card">
        <div class="key">
          {macro.key}
        </div>

        <div class="macro-info">
          <h3>{macro.name}</h3>
          <p>{macro.description}</p>
        </div>

        <button class="edit-button">
          Edit
        </button>
      </article>
    {/each}
  </div>
</section>
