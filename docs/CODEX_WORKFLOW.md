# Codex workflow for Blueprint-heavy Unreal projects

This is the operating sequence expected from Codex once the plugin is installed in a game repository.

## 1. Establish context

Before edits:

1. read the game's `AGENTS.md`
2. inspect `.uproject`, `Config`, `Source`, plugins, and project conventions
3. call `CheckEnvironment`
4. call `GetProjectOverview`
5. use `SearchAssets` instead of guessing asset paths

For a Blueprint task:

```text
GetBlueprintSummary
-> AnalyzeBlueprint when architecture/performance matters
-> GetBlueprintGraph on the exact target graph
```

## 2. Form a minimal edit

Prefer the smallest change that satisfies the task.

Bad:

```text
rewrite BP_Player
```

Good:

```text
read BP_Player/EventGraph
identify exact branch and pins
add one function graph or component
wire the required nodes
compile
re-read
```

## 3. Address graph elements exactly

Before a graph mutation, obtain current:

- graph name
- node GUID
- pin names
- pin directions/types
- existing links

Do not reuse stale GUIDs after destructive graph operations without re-reading.

## 4. Mutate and verify immediately

Semantic writes auto-compile by default. After a successful mutation:

1. inspect returned warnings
2. re-read the affected graph/summary
3. ensure intended node/pin structure exists
4. run the relevant Unreal automation/PIE checks
5. save only when satisfied

If compilation fails, treat the operation as failed even if Unreal created the requested node.

## 5. Use the right surface

Use **this plugin** for:

- Blueprint structure
- exact Blueprint graph reads/writes
- Asset Registry discovery
- Blueprint compile diagnostics
- safe save/undo

Use **official Unreal MCP toolsets**, when available, for broader editor operations such as scene actors, object/property operations, materials, widgets, automation tests, and other engine-supported surfaces.

Use **source code editing** for C++/config/text files.

Do not force all Unreal work through one mechanism.

## 6. Gameplay feature checklist

For a feature such as inventory, combat, interaction, health, quests, AI, or save/load:

- identify ownership/lifetime
- decide Actor vs Component vs Subsystem vs Data Asset responsibilities
- inspect existing interfaces/dispatchers/tags before adding coupling
- avoid unnecessary Event Tick
- avoid repeated global actor scans
- consider server authority/replication for multiplayer projects
- keep UI presentation separate from gameplay state
- make data-driven definitions when content count will grow
- verify save/load and multiplayer behavior where applicable

## 7. Refactor checklist

When asked to clean a Blueprint:

- run `AnalyzeBlueprint`
- inspect large graphs
- inspect dynamic-cast concentration
- inspect global actor lookup calls
- look for duplicated logic
- split responsibilities into functions/components only when it improves ownership/cohesion
- preserve behavior with tests before large restructuring
- refactor incrementally and compile after each step

## 8. Source control

Before committing:

- save only intended assets
- inspect changed files
- ensure no generated Unreal folders were added
- confirm LFS handles `.uasset`/`.umap`
- run project tests
- include a human-readable summary of Blueprint assets and logical changes

## 9. Stop conditions

Stop automated mutation and report clearly if:

- target asset/graph cannot be resolved exactly
- engine version is unsupported
- graph schema rejects the intended connection
- Blueprint compile errors persist after rollback
- an asset fails to load after editor restart
- the task requires a domain-specific node/tool not safely exposed yet
- source-control state indicates unresolved binary conflicts
