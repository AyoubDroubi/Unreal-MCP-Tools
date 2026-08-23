# Unreal project agent rules

## Unreal MCP

This project uses Unreal MCP Tools with Epic's UE 5.8+ ModelContextProtocol / Toolset Registry integration.

When editing Blueprints:

1. call `CheckEnvironment`
2. discover assets with `GetProjectOverview` / `SearchAssets`
3. call `GetBlueprintSummary`
4. read the exact target graph with `GetBlueprintGraph`
5. use current GUIDs and exact pin names
6. perform one small semantic mutation
7. treat compilation failure as task failure
8. re-read the affected graph
9. run relevant automation/PIE tests
10. call `SaveAsset` only after validation

Do not guess asset paths, graph names, node GUIDs, or pin names.

## Safety

- Do not modify `/Engine`.
- Do not widen Unreal MCP writable roots unless the task requires it.
- Do not directly patch `.uasset` or `.umap` bytes.
- Do not auto-save broad sets of assets.
- Do not continue automated Blueprint writes after persistent compile errors.
- Do not attempt binary conflict merges.
- Keep changes small enough to undo/review.

## Architecture

Before adding logic to a large Character/Controller/GameMode Blueprint, check whether the responsibility belongs in:

- Actor Component
- Subsystem
- Data Asset
- Blueprint Interface
- Event Dispatcher
- dedicated gameplay Actor/Object

Avoid Event Tick for work that can be event/timer/state-change driven. Avoid repeated `GetAllActors...` calls in gameplay loops.

For multiplayer, explicitly reason about authority, ownership, RPC direction, replicated state, and PlayerState/GameState lifetime.

## Validation

A successful tool call is not enough. The final task evidence should include:

- Blueprint compile result
- relevant automation/PIE result when available
- affected asset list
- source-control diff/status review
