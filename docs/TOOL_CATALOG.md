# Tool catalog

All custom tools are static `AICallable` functions discovered by UE 5.8's Toolset Registry.

## Project toolset — read-only

### `GetProjectOverview`

Returns project name, engine version, important paths, Asset Registry counts, and whether a `Source` directory exists.

### `SearchAssets`

Bounded Asset Registry query under a package path with optional name/class filters. Does not load every asset.

### `GetAssetDependencies`

Returns package dependencies reported by Asset Registry.

### `GetAssetReferencers`

Returns packages that reference a package.

## Blueprint toolset — reads

### `GetBlueprintSummary`

Returns:

- parent class
- Blueprint status
- variables
- SCS components
- graph inventory
- bounded node/pin details

### `GetBlueprintGraph`

Reads one graph using an optional node limit. This is the canonical precursor to graph mutations.

### `AnalyzeBlueprint`

Current checks include:

- graph node counts / large graph flagging
- dynamic-cast count
- global actor lookup calls (`GetAllActors...`)
- graphs containing latent functions

This is intentionally a signal generator, not a claim that every finding is a bug.

## Blueprint lifecycle — writes

### `CreateBlueprint`

Creates a Blueprint under an allowed root with a loaded parent class path such as:

```text
/Script/Engine.Actor
```

Compiles the new Blueprint. Saving remains explicit.

### `CreateFunctionGraph`

Creates a user function graph and compiles.

### `RenameGraph`

Renames through Blueprint editor utilities and compiles.

### `CompileBlueprint`

Compiles with structured error/warning counts and messages. Optional save-on-success.

## Variables — writes

### `AddMemberVariable`

Supported scalar element types:

- bool
- byte
- int
- int64
- float
- double
- string
- name
- text
- object
- class
- struct

Containers:

- scalar
- array
- set

`object`, `class`, and `struct` require an `object_type_path`.

Map variables are deliberately not exposed in 0.1.0 because a correct agent-facing API needs both key and value type schemas.

### `RenameMemberVariable`

Only renames variables declared by the target Blueprint.

### `RemoveMemberVariable`

Only removes variables declared by the target Blueprint.

## Components — writes

### `AddComponent`

Adds an `UActorComponent` subclass through the Blueprint's `USimpleConstructionScript`. Optional parent component is addressed by SCS variable name.

### `RemoveComponent`

Removes one SCS component and promotes children using Unreal's SCS API.

## Graph authoring — writes

### `AddCallFunctionNode`

Requires:

- exact graph
- owner UClass path
- exact UFunction name
- coordinates

Only Blueprint-callable/pure functions are accepted.

### `AddBranchNode`

Creates a K2 Branch.

### `AddVariableGetNode` / `AddVariableSetNode`

Use the K2 schema to spawn variable nodes from the Blueprint skeleton class.

### `AddCustomEventNode`

Creates a named K2 custom event.

## Graph wiring — writes

### `ConnectPins`

Requires exact node GUIDs and pin names. The graph schema must accept the connection.

### `BreakPinLinks`

Breaks all links on one exact pin.

### `SetPinDefaultValue`

Only accepts input pins and validates the current pin default after setting.

### `MoveNode`

Layout-only operation. Transaction-backed but does not compile.

### `DeleteNode`

Only deletes nodes Unreal marks as user-deletable.

## Workflow toolset

### `CheckEnvironment`

Verifies the custom plugin is alive and reports core safety settings.

### `SaveAsset`

Explicitly saves one loaded asset under an allowed root.

### `UndoLastTransaction`

Undo escape hatch. Only use when the latest editor transaction is known to be the intended one.

## Intentionally delegated to Unreal/native tools

Do not recreate official tooling without a reason. Depending on engine/toolset availability, broader MCP/editor surfaces should handle:

- level/actor transforms
- generic object property edits
- materials/material instances
- UMG/widget work
- automation tests
- live coding
- specialized graph systems

Add a custom tool when Blueprint-specific precision/safety is missing, not merely to grow the tool count.
