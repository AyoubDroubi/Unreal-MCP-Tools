# Unreal MCP Tools

Production-focused **Unreal Engine 5.8+ MCP toolsets** that let Codex and other MCP clients inspect an Unreal project, reason about Blueprints, perform safe graph edits through Unreal Editor APIs, compile the result, and keep every mutation reviewable.

This repository intentionally **does not edit `.uasset` bytes directly**. Unreal Editor remains the authoritative writer/compiler for assets.

## Why this exists

Unreal's official experimental **Model Context Protocol (MCP)** plugin in UE 5.8 exposes Toolset Registry tools to MCP clients. `UnrealMCPTools` extends that official surface with the project/Blueprint capabilities an engineering agent needs for real repository work:

- project and Asset Registry discovery
- Blueprint variables/components/graphs/nodes/pins inspection
- Blueprint complexity/smell analysis
- Blueprint creation and function graph creation
- member variable add/rename/remove
- Blueprint component add/remove
- graph node creation for common gameplay scripting primitives
- pin connection/disconnection and default values
- node movement/deletion
- compile diagnostics
- explicit save
- transaction-backed mutations and rollback-on-compile-failure
- Codex operating rules and installation helpers

## Architecture

```text
Codex / MCP client
        |
        v
UE 5.8 ModelContextProtocol plugin
        |
        v
ToolsetRegistry
        |
        +---------------- Unreal built-in toolsets
        |
        `---------------- UnrealMCPTools
                             |
                             +-- Project / Asset Registry reads
                             +-- Blueprint / EdGraph reads
                             +-- Kismet / SCS mutations
                             +-- Compile validation
                             `-- Transaction / save controls
```

The custom plugin is an **Editor-only** module. It uses supported Unreal editor APIs such as `FBlueprintEditorUtils`, `FKismetEditorUtilities`, `UEdGraphSchema_K2`, `USimpleConstructionScript`, `AssetRegistry`, and `UEditorAssetSubsystem`.

## Safety model

Every semantic mutation follows this contract:

```text
validate target/root
      -> read exact asset/graph
      -> begin FScopedTransaction
      -> mutate through Unreal APIs
      -> mark Blueprint modified
      -> compile
      -> if compile fails: undo latest MCP transaction (default)
      -> save only when explicitly requested or configured
```

Defaults:

- writes are restricted to `/Game`
- `/Engine` is read-only
- semantic Blueprint edits auto-compile
- failed compile triggers rollback
- auto-save is **off**
- graph reads are bounded to avoid flooding an agent with huge node dumps

Change these under **Editor Preferences / Project settings → Plugins → Unreal MCP Tools** as appropriate for your project.

## Tool families

| Family | Representative tools |
|---|---|
| Project | `GetProjectOverview`, `SearchAssets`, `GetAssetDependencies`, `GetAssetReferencers` |
| Blueprint read | `GetBlueprintSummary`, `GetBlueprintGraph`, `AnalyzeBlueprint` |
| Blueprint lifecycle | `CreateBlueprint`, `CreateFunctionGraph`, `RenameGraph`, `CompileBlueprint` |
| Variables | `AddMemberVariable`, `RenameMemberVariable`, `RemoveMemberVariable` |
| Components | `AddComponent`, `RemoveComponent` |
| Graph authoring | `AddCallFunctionNode`, `AddBranchNode`, `AddVariableGetNode`, `AddVariableSetNode`, `AddCustomEventNode` |
| Graph wiring | `ConnectPins`, `BreakPinLinks`, `SetPinDefaultValue`, `MoveNode`, `DeleteNode` |
| Workflow | `CheckEnvironment`, `SaveAsset`, `UndoLastTransaction` |

See [docs/TOOL_CATALOG.md](docs/TOOL_CATALOG.md) for exact behavior and operating constraints.

## Quick start

1. Use **Unreal Engine 5.8 or newer**.
2. Enable Epic's `ModelContextProtocol` and `ToolsetRegistry` plugins.
3. Copy `Plugin/UnrealMCPTools` into `<YourProject>/Plugins/UnrealMCPTools`.
4. Regenerate project files/build the Editor target, then open the project.
5. Refresh MCP tools after the plugin loads.
6. Connect Codex using Unreal MCP's client configuration flow.
7. Start with `CheckEnvironment` and `GetProjectOverview`.
8. Make Codex follow [docs/CODEX_WORKFLOW.md](docs/CODEX_WORKFLOW.md) and the supplied [templates/AGENTS.unreal.md](templates/AGENTS.unreal.md).

Windows helper:

```powershell
./scripts/install.ps1 -ProjectPath "D:\Games\MyProject"
```

macOS/Linux helper:

```bash
./scripts/install.sh /path/to/MyProject
```

## Recommended agent workflow

Never tell an agent “just edit this Blueprint” without context. The reliable sequence is:

```text
GetProjectOverview
  -> SearchAssets
  -> GetBlueprintSummary
  -> GetBlueprintGraph (target graph)
  -> reason using exact GUIDs/pins
  -> one small mutation
  -> automatic compile
  -> re-read affected graph
  -> SaveAsset only after success
  -> run project automation/PIE through available Unreal MCP tools
  -> inspect Git diff/status
```

For large work, repeat this as small verified transactions.

## Source control

Unreal binary assets should use Git LFS in repositories that store them. This repository includes a conservative `.gitattributes` example for `.uasset` and `.umap`.

Do not merge Blueprint binary conflicts manually. Lock or coordinate ownership of heavily edited assets when your source-control system supports it.

## Compatibility

The plugin currently targets **UE 5.8+** because it extends Epic's official `ToolsetRegistry` / MCP integration. It is not a drop-in replacement for community MCP bridges used on older engines.

See [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md).

## Current verification status

The repository contains static checks for plugin layout, generated-header ordering, tool registration markers, docs, and accidental Unreal build artifacts.

A real Unreal Editor build is still the authoritative compatibility test because Unreal headers/toolset APIs are engine-version-specific. Before production use, build the plugin in the exact engine version used by the game and exercise the smoke workflow in [docs/INSTALLATION.md](docs/INSTALLATION.md).

## Design principles

1. **Engine-native writes, never blind binary patching.**
2. **Read before write.**
3. **Exact GUID/pin addressing for graph edits.**
4. **One transaction per logical mutation.**
5. **Compile after semantic Blueprint changes.**
6. **Rollback on compile errors.**
7. **Save deliberately.**
8. **Use built-in Unreal MCP tools where they already solve a problem; extend only the gaps.**
9. **Return structured, agent-friendly results instead of console prose.**
10. **Treat generated assets as code: test them, diff them, and keep changes small.**

## Repository layout

```text
Plugin/UnrealMCPTools/   Unreal Editor plugin copied into a target project
docs/                    architecture, safety, tool catalog, Codex workflow
templates/               reusable AGENTS.md guidance for game repositories
scripts/                 installation and static verification helpers
.github/workflows/       repository checks
```

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md). New mutating tools must have a write-root check, transaction boundary, structured result, and a validation story before they are accepted.
