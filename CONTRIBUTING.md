# Contributing

## Tool design checklist

A new **read** tool should:

- use Asset Registry when loading an asset is unnecessary
- avoid dirtying packages
- bound potentially large responses
- return stable identifiers that later write tools can consume

A new **write** tool must:

- validate its target path against the configured writable roots
- load the target through Unreal Editor APIs
- create an `FScopedTransaction`
- call `Modify()` on affected transactional objects
- mutate through Unreal-supported APIs
- mark the Blueprint/package appropriately
- compile after semantic Blueprint changes
- return compiler errors/warnings in a structured result
- preserve undo/rollback behavior
- avoid saving unless the API contract says it saves

## Development

1. Copy `Plugin/UnrealMCPTools` into a UE 5.8+ C++ project's `Plugins` folder.
2. Enable `ModelContextProtocol` and `ToolsetRegistry`.
3. Build the Editor target.
4. Open the project and refresh MCP tools.
5. Call `CheckEnvironment`.
6. Exercise reads against a disposable Blueprint.
7. Exercise each mutation, compile, undo, and save.
8. Run project automation tests and verify the asset opens cleanly after restarting the editor.

## Code style

- Unreal naming/style conventions.
- Tool parameters use explicit names that are clear to an LLM.
- No ambiguous fuzzy mutation APIs.
- Comments on AICallable UFUNCTIONs are part of the tool's public contract.
- Keep implementation helpers private to the `.cpp` unless reuse is demonstrated.

## Compatibility

Do not lower or widen the supported engine range only because a source file compiles. Verify Toolset Registry discovery and MCP calls in each declared engine version.
