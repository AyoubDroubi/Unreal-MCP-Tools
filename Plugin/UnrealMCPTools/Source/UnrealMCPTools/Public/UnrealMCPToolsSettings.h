#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UnrealMCPToolsSettings.generated.h"

UCLASS(Config=EditorPerProjectUserSettings, DefaultConfig, meta=(DisplayName="Unreal MCP Tools"))
class UNREALMCPTOOLS_API UUnrealMCPToolsSettings final : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UUnrealMCPToolsSettings();

    virtual FName GetCategoryName() const override;

    /** Writable asset roots. Mutating tools reject assets outside these roots. */
    UPROPERTY(Config, EditAnywhere, Category="Safety")
    TArray<FString> AllowedAssetRoots;

    /** Engine content is read-only by default and cannot be mutated unless explicitly enabled. */
    UPROPERTY(Config, EditAnywhere, Category="Safety")
    bool bAllowEngineContent = false;

    /** Compile a Blueprint after each semantic mutation. */
    UPROPERTY(Config, EditAnywhere, Category="Mutation")
    bool bAutoCompileAfterMutation = true;

    /** Save a Blueprint after a successful automatic compile. Off by default so Git diffs remain intentional. */
    UPROPERTY(Config, EditAnywhere, Category="Mutation")
    bool bAutoSaveAfterSuccessfulCompile = false;

    /** Undo the latest MCP transaction when its automatic compile fails. */
    UPROPERTY(Config, EditAnywhere, Category="Mutation")
    bool bRollbackOnCompileFailure = true;

    /** Default maximum number of nodes returned by a graph read. */
    UPROPERTY(Config, EditAnywhere, Category="Read Limits", meta=(ClampMin="10", ClampMax="5000"))
    int32 MaxNodesPerGraphResponse = 250;

    /** Flag graphs larger than this in Blueprint analysis. */
    UPROPERTY(Config, EditAnywhere, Category="Analysis", meta=(ClampMin="25"))
    int32 LargeGraphNodeThreshold = 200;

    /** Flag Blueprints with at least this many dynamic casts. */
    UPROPERTY(Config, EditAnywhere, Category="Analysis", meta=(ClampMin="1"))
    int32 ExcessiveDynamicCastThreshold = 8;
};
