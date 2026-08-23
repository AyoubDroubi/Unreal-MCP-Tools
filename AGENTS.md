# AGENTS.md — Unreal MCP Tools

This repository is an Unreal Engine **Editor plugin/tooling repository**, not a game project.

## Mission

Extend Unreal Engine 5.8+'s official MCP/Toolset Registry surface with safe, auditable tools that let coding agents understand and modify Blueprint-heavy projects.

## Non-negotiable rules

- Do not introduce direct `.uasset` binary mutation as the normal write path.
- Prefer documented Unreal Editor APIs.
- Every new MCP tool must be a static `UFUNCTION` on a `UToolsetDefinition` subclass and use `meta=(AICallable)`.
- Read tools must not dirty packages.
- Mutating tools must reject writes outside configured roots.
- Mutating tools must use an Unreal transaction.
- Semantic Blueprint mutations must compile or explicitly explain why they do not.
- Preserve rollback-on-compile-failure behavior.
- Do not auto-save by default.
- Return structured `USTRUCT` results; avoid responses that require scraping log text.
- For graph mutation, require exact graph names, node GUIDs, and pin names rather than fuzzy matching.
- Do not silently create a connection Unreal's graph schema rejects.
- Keep the plugin Editor-only.
- Do not add generated Unreal directories (`Binaries`, `Intermediate`, `Saved`, DDC) to Git.
- UE 5.8 is the baseline unless a compatibility change is deliberate and documented.

## Before changing C++ APIs

Verify the relevant API against the target Unreal version's official headers/docs. Unreal editor APIs move between releases.

## Verification

Run:

```bash
python scripts/verify_repo.py
```

Then build the plugin in an actual UE 5.8+ Editor target. Static checks are not a substitute for UnrealBuildTool/UHT.

## Pull request evidence

For MCP tool changes, include:

- tools added/changed
- read/write classification
- transaction behavior
- compile/save behavior
- manual Unreal Editor smoke test performed
- known version constraints
