#include "UnrealMCPWorkflowToolset.h"

#include "Editor.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UnrealMCPToolsSettings.h"

namespace UnrealMCPWorkflowTools
{
    FString NormalizePackagePath(const FString& Path)
    {
        FString Result = Path;
        Result.TrimStartAndEndInline();
        const int32 DotIndex = Result.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
        if (DotIndex != INDEX_NONE)
        {
            Result = Result.Left(DotIndex);
        }
        return Result;
    }

    FString NormalizeObjectPath(const FString& Path)
    {
        const FString PackagePath = NormalizePackagePath(Path);
        FString AssetName;
        if (!PackagePath.Split(TEXT("/"), nullptr, &AssetName, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
        {
            return Path;
        }
        return FString::Printf(TEXT("%s.%s"), *PackagePath, *AssetName);
    }

    bool IsWriteAllowed(const FString& Path)
    {
        const FString PackagePath = NormalizePackagePath(Path);
        const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();

        if (PackagePath.StartsWith(TEXT("/Engine")) && !Settings->bAllowEngineContent)
        {
            return false;
        }

        for (const FString& Root : Settings->AllowedAssetRoots)
        {
            if (!Root.IsEmpty() && PackagePath.StartsWith(Root))
            {
                return true;
            }
        }
        return false;
    }
}

using namespace UnrealMCPWorkflowTools;

FMCPToolResult UUnrealMCPWorkflowToolset::CheckEnvironment()
{
    const UUnrealMCPToolsSettings* Settings = GetDefault<UUnrealMCPToolsSettings>();

    FMCPToolResult Result;
    Result.success = GEditor != nullptr;
    Result.message = Result.success
        ? TEXT("Unreal MCP Tools is loaded in an editor process.")
        : TEXT("Unreal MCP Tools loaded without a valid GEditor.");

    Result.warnings.Add(FString::Printf(TEXT("Auto compile: %s"), Settings->bAutoCompileAfterMutation ? TEXT("enabled") : TEXT("disabled")));
    Result.warnings.Add(FString::Printf(TEXT("Auto save: %s"), Settings->bAutoSaveAfterSuccessfulCompile ? TEXT("enabled") : TEXT("disabled")));
    Result.warnings.Add(FString::Printf(TEXT("Rollback on compile failure: %s"), Settings->bRollbackOnCompileFailure ? TEXT("enabled") : TEXT("disabled")));
    Result.warnings.Add(FString::Printf(TEXT("Engine writes: %s"), Settings->bAllowEngineContent ? TEXT("enabled") : TEXT("disabled")));

    FString Roots = FString::Join(Settings->AllowedAssetRoots, TEXT(", "));
    Result.warnings.Add(FString::Printf(TEXT("Allowed roots: %s"), *Roots));
    return Result;
}

FMCPToolResult UUnrealMCPWorkflowToolset::SaveAsset(const FString& asset_path)
{
    FMCPToolResult Result;
    Result.asset_path = asset_path;

    if (!IsWriteAllowed(asset_path))
    {
        Result.message = TEXT("Save rejected by Unreal MCP Tools writable-root policy.");
        return Result;
    }

    UObject* Asset = LoadObject<UObject>(nullptr, *NormalizeObjectPath(asset_path));
    if (!Asset)
    {
        Result.message = TEXT("Asset not found.");
        return Result;
    }

    UEditorAssetSubsystem* AssetSubsystem = GEditor ? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>() : nullptr;
    if (!AssetSubsystem)
    {
        Result.message = TEXT("EditorAssetSubsystem is unavailable.");
        return Result;
    }

    Result.success = AssetSubsystem->SaveLoadedAsset(Asset, false);
    Result.asset_path = Asset->GetPathName();
    Result.message = Result.success ? TEXT("Asset saved.") : TEXT("Asset save failed.");
    return Result;
}

FMCPToolResult UUnrealMCPWorkflowToolset::UndoLastTransaction()
{
    FMCPToolResult Result;
    if (!GEditor)
    {
        Result.message = TEXT("GEditor is unavailable.");
        return Result;
    }

    Result.success = GEditor->UndoTransaction(true);
    Result.message = Result.success
        ? TEXT("Latest editor transaction undone.")
        : TEXT("No editor transaction was undone.");
    return Result;
}
