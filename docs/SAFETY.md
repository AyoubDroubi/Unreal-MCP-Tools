# Safety and change-control model

## Threat model

An AI tool can make technically valid but undesirable edits, operate on the wrong asset, produce a Blueprint that does not compile, or save too much state. The plugin therefore treats editor mutation as a controlled operation rather than a free-form scripting surface.

## Controls

### Writable roots

Mutating tools reject packages outside `AllowedAssetRoots`. Default: `/Game`.

`/Engine` remains read-only unless `bAllowEngineContent` is explicitly enabled.

For a focused task, narrow the root further, for example:

```text
/Game/Gameplay/Inventory
```

### Exact addressing

Node writes use:

- exact Blueprint path
- exact graph name
- exact node GUID
- exact pin name

An agent should never guess a GUID or pin label. Read the graph immediately before editing it.

### Transactions

Each logical mutation is wrapped in `FScopedTransaction`, allowing Undo and providing a rollback target.

### Compilation

Semantic Blueprint mutations compile by default. A successful editor API call is not considered enough.

### Rollback

If automatic compile fails and rollback is enabled, the latest completed MCP transaction is undone.

This assumes no unrelated editor transaction occurs between the mutation and compile. The plugin intentionally performs compile immediately after the mutation to keep that window narrow.

### Saving

Auto-save is disabled by default. Saving is a separate explicit action so the user/agent can:

1. make a small edit
2. compile
3. inspect/re-read
4. run tests
5. save intentionally

### Large responses

Graph reads are bounded. This protects model context and encourages focused reasoning.

## Agent rules

- Always inspect before mutating.
- Prefer one logical mutation per tool call.
- Never operate on `/Engine` for routine game work.
- Avoid mass edits until the same operation succeeds on one representative asset.
- Compile after any semantic change even if auto-compile is disabled.
- Save only after successful validation.
- Run tests/PIE after gameplay changes using the project's available automation.
- Review source-control changes before commit.
- Do not resolve binary merge conflicts by combining bytes.
- If a Blueprint becomes corrupt or fails to open, stop automated writes and restore from source control.

## Direct binary tooling

Libraries that parse/write `.uasset` can be useful for offline inspection and purpose-built migrations, but they are not part of this plugin's normal write path. Any future direct binary tool should be a separately gated subsystem with explicit version coverage and tests.
