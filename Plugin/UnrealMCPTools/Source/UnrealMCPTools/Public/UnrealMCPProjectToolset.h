#pragma once

#include "CoreMinimal.h"
#include "ToolsetRegistry/ToolsetDefinition.h"
#include "UnrealMCPToolsTypes.h"
#include "UnrealMCPProjectToolset.generated.h"

/** Read-only project and Asset Registry tools for discovering a project before making edits. */
UCLASS(BlueprintType, Hidden)
class UNREALMCPTOOLS_API UUnrealMCPProjectToolset final : public UToolsetDefinition
{
    GENERATED_BODY()

public:
    /** Return basic project/engine paths plus Asset Registry counts under /Game. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Project", meta=(AICallable))
    static FMCPProjectOverview GetProjectOverview();

    /**
     * Search Asset Registry metadata without loading every asset.
     * package_path defaults to /Game. name_contains and class_name are optional case-insensitive filters.
     */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Project", meta=(AICallable))
    static FMCPAssetSearchResult SearchAssets(
        const FString& package_path = TEXT("/Game"),
        const FString& name_contains = TEXT(""),
        const FString& class_name = TEXT(""),
        int32 max_results = 200);

    /** Return package dependencies from Asset Registry for a package such as /Game/Blueprints/BP_Player. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Project", meta=(AICallable))
    static FMCPReferenceResult GetAssetDependencies(const FString& package_name);

    /** Return packages that reference the supplied package according to Asset Registry. */
    UFUNCTION(BlueprintCallable, Category="Unreal MCP|Project", meta=(AICallable))
    static FMCPReferenceResult GetAssetReferencers(const FString& package_name);
};
