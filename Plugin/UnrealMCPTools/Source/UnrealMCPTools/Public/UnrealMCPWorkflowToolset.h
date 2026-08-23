#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/ToolsetDefinition.h"
#include "UnrealMCPToolsTypes.h"
#include "UnrealMCPWorkflowToolset.generated.h"

/** Small operational tools used by Codex to verify the bridge, save intentionally, and recover from a bad edit. */
UCLASS(BlueprintType, Hidden)
class UNREALMCPTOOLS_API UUnrealMCPWorkflowToolset final : public UToolsetDefinition
{
    GENERATED_BODY()

public:
    /** Describe plugin safety settings and prove that the custom toolset is loaded. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Workflow", meta=(AICallable))
    static FMCPToolResult CheckEnvironment();

    /** Save one loaded asset under an allowed writable root. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Workflow", meta=(AICallable))
    static FMCPToolResult SaveAsset(const FString& asset_path);

    /** Undo the latest Unreal Editor transaction. Use only when the latest edit is known to be the edit being reverted. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Workflow", meta=(AICallable))
    static FMCPToolResult UndoLastTransaction();
};
