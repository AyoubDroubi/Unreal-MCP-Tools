# Compatibility

## Supported baseline

| Engine | Status | Notes |
|---|---|---|
| UE 5.8 | Primary target | Built around Epic's official experimental ModelContextProtocol + ToolsetRegistry integration |
| Newer than 5.8 | Expected with verification | Editor APIs and experimental plugin contracts may change |
| UE 5.7 and older | Not supported by this branch | Use an older/community MCP bridge or backport deliberately |

## Why 5.8+

The architecture intentionally extends Epic's in-editor MCP transport and Toolset Registry instead of maintaining an independent socket/server protocol.

## Experimental API warning

Epic marks MCP/Toolset Registry functionality as experimental. Treat engine upgrades as compatibility events:

1. compile plugin
2. inspect UHT output
3. verify tool discovery
4. run smoke workflow
5. test Blueprint mutation + rollback + save
6. update this compatibility matrix

## Blueprint project note

A game may be mostly or entirely Blueprint-authored, but installing this plugin still requires an environment capable of compiling an Unreal Editor C++ plugin.

## Project-specific node ecosystems

Gameplay Ability System, Control Rig, Niagara, PCG, Animation Blueprints, Behavior Trees, StateTree, materials, and other graph systems have distinct schemas/types. Do not assume K2 Blueprint graph operations are valid for those graph editors. Use their official toolsets/APIs or add dedicated, tested toolsets.
