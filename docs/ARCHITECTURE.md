# Architecture

## Position in the Unreal MCP stack

`UnrealMCPTools` is not a second MCP transport. UE 5.8 already ships an experimental Model Context Protocol plugin that discovers tools from the Toolset Registry. This repository adds toolsets to that registry.

```text
MCP client (Codex)
  -> Epic ModelContextProtocol
     -> ToolsetRegistry
        -> Epic toolsets
        -> UnrealMCPProjectToolset
        -> UnrealMCPBlueprintToolset
        -> UnrealMCPWorkflowToolset
```

That choice keeps transport, discovery, authentication/configuration concerns with Epic's integration and keeps this repository focused on safe Unreal project operations.

## Modules

### Project toolset

Uses `AssetRegistry` for cheap read-only discovery:

- project overview
- bounded asset search
- package dependencies
- package referencers

It intentionally avoids loading every asset.

### Blueprint toolset

Uses:

- `UBlueprint`
- `FBlueprintEditorUtils`
- `FKismetEditorUtilities`
- `UEdGraph` / `UEdGraphNode` / `UEdGraphPin`
- `UEdGraphSchema_K2`
- K2 node types
- `USimpleConstructionScript`

Read paths emit stable graph/node/pin identifiers. Write paths consume those identifiers.

### Workflow toolset

Contains deliberately small operational escape hatches:

- environment/capability check
- explicit asset save
- undo latest transaction

It does not duplicate source-control or test functionality when the engine's normal tooling can perform it.

## Mutation pipeline

For a normal semantic Blueprint edit:

1. Normalize package/object path.
2. Enforce `AllowedAssetRoots`.
3. Load the exact Blueprint.
4. Resolve exact graph/node/pin.
5. Open `FScopedTransaction`.
6. call `Modify()` on affected objects.
7. use the relevant Unreal API.
8. mark Blueprint modified or structurally modified.
9. close transaction.
10. compile.
11. if errors and rollback is enabled, undo that completed transaction.
12. optionally auto-save only when configured.

Layout-only node movement skips compile because it does not change generated behavior.

## Why no direct UAsset writes

A Blueprint package includes more than visually obvious node data: generated/skeleton classes, graph state, serialized references, compiler-dependent data, package state, and version-specific serialization.

Direct binary mutation can be useful for inspection/modding/migrations, but it is the wrong default for an agent editing a live game repository. Let Unreal own serialization and compilation.

## Large graph strategy

The plugin does not dump an unbounded Blueprint into model context. `GetBlueprintSummary` and `GetBlueprintGraph` cap returned nodes using settings. An agent should:

1. inspect summary
2. choose the relevant graph
3. read that graph
4. edit a small region
5. re-read and verify

Future work may add graph slicing/query tools, but bounded responses are already safer than raw full-project dumps.

## Extension rule

Before adding a new custom tool, check whether an official Unreal MCP toolset already exposes the operation. Prefer composing existing official tools. Add custom tools where agent-grade Blueprint/project semantics are missing or where additional safety guarantees are needed.
