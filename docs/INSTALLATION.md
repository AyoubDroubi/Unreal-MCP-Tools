# Installation and smoke test

## Requirements

- Unreal Engine 5.8+ source or launcher installation with C++ toolchain available
- a C++ Unreal project, or a Blueprint project converted to support an Editor plugin build
- Epic `ModelContextProtocol` plugin enabled
- Epic `ToolsetRegistry` plugin enabled
- an MCP client such as Codex

## Install

Copy:

```text
Plugin/UnrealMCPTools
```

to:

```text
<YourProject>/Plugins/UnrealMCPTools
```

Or use:

```powershell
./scripts/install.ps1 -ProjectPath "D:\Games\MyGame"
```

```bash
./scripts/install.sh /path/to/MyGame
```

The result should be:

```text
MyGame/
  MyGame.uproject
  Plugins/
    UnrealMCPTools/
      UnrealMCPTools.uplugin
      Source/
```

## Enable dependencies

In the Unreal Editor plugin browser enable:

- Model Context Protocol
- Toolset Registry
- Unreal MCP Tools

Restart/rebuild when requested.

## Build

Regenerate project files if necessary and build your project's **Editor** target. The plugin is Editor-only and is not intended to ship in the packaged game.

If UHT or UnrealBuildTool reports an API mismatch, verify the exact engine version before changing code. The baseline is UE 5.8.

## Refresh tools

After the module loads, refresh MCP tool discovery using the Unreal MCP refresh mechanism (for example the console command exposed by the official MCP plugin). Then reconnect/refresh the MCP client if necessary.

## Codex

Use Unreal's MCP client-configuration flow for Codex. Keep transport/client configuration outside this repository; this plugin only contributes Toolset Registry tools.

Copy `templates/AGENTS.unreal.md` into the game repository's agent instructions (or merge the relevant section into the project's existing AGENTS.md).

## Mandatory smoke test

Use a disposable Blueprint under `/Game/MCPTests`.

1. `CheckEnvironment`
2. `GetProjectOverview`
3. `SearchAssets("/Game")`
4. `CreateBlueprint("/Game/MCPTests/BP_MCPTest", "/Script/Engine.Actor")`
5. `GetBlueprintSummary`
6. `AddMemberVariable(..., "Health", "float", ..., "100.0")`
7. `CreateFunctionGraph(..., "ApplyTest")`
8. `GetBlueprintGraph` for the new graph
9. add a Branch node
10. re-read the graph and capture its GUIDs/pins
11. compile
12. save explicitly
13. close/reopen the editor
14. verify the asset loads and compiles
15. undo/revert/delete the disposable asset

Also exercise a deliberately invalid mutation and verify compilation failure does not leave the Blueprint in a worse state.

## Production validation

Before allowing an agent to edit real assets:

- create a source-control branch
- verify Git LFS policy for `.uasset` / `.umap`
- run existing project tests
- keep editor auto-save disabled
- set `AllowedAssetRoots` narrowly if the task only touches one area
- test a representative Blueprint of each major class family
