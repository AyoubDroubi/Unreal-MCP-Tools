#include "UnrealMCPProjectToolset.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Engine/Blueprint.h"
#include "HAL/FileManager.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

namespace UnrealMCPProjectTools
{
    IAssetRegistry& AssetRegistry()
    {
        return FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    }

    FString NormalizePackageName(const FString& Input)
    {
        FString Result = Input;
        Result.TrimStartAndEndInline();
        const int32 DotIndex = Result.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
        if (DotIndex != INDEX_NONE)
        {
            Result = Result.Left(DotIndex);
        }
        return Result;
    }

    FMCPAssetInfo ToAssetInfo(const FAssetData& Asset)
    {
        FMCPAssetInfo Info;
        Info.asset_name = Asset.AssetName.ToString();
        Info.object_path = Asset.GetSoftObjectPath().ToString();
        Info.package_name = Asset.PackageName.ToString();
        Info.class_name = Asset.AssetClassPath.GetAssetName().ToString();
        return Info;
    }
}

using namespace UnrealMCPProjectTools;

FMCPProjectOverview UUnrealMCPProjectToolset::GetProjectOverview()
{
    FMCPProjectOverview Result;
    Result.success = true;
    Result.project_name = FApp::GetProjectName();
    Result.engine_version = FEngineVersion::Current().ToString();
    Result.project_dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
    Result.content_dir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
    Result.project_file = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
    Result.has_source = IFileManager::Get().DirectoryExists(*FPaths::GameSourceDir());

    FARFilter Filter;
    Filter.PackagePaths.Add(FName(TEXT("/Game")));
    Filter.bRecursivePaths = true;

    TArray<FAssetData> Assets;
    AssetRegistry().GetAssets(Filter, Assets);
    Result.asset_count = Assets.Num();

    for (const FAssetData& Asset : Assets)
    {
        if (Asset.AssetClassPath.GetAssetName() == UBlueprint::StaticClass()->GetFName())
        {
            ++Result.blueprint_count;
        }
    }

    return Result;
}

FMCPAssetSearchResult UUnrealMCPProjectToolset::SearchAssets(
    const FString& package_path,
    const FString& name_contains,
    const FString& class_name,
    const int32 max_results)
{
    FMCPAssetSearchResult Result;
    const int32 Limit = FMath::Clamp(max_results, 1, 5000);
    const FString Root = package_path.IsEmpty() ? TEXT("/Game") : NormalizePackageName(package_path);

    FARFilter Filter;
    Filter.PackagePaths.Add(FName(*Root));
    Filter.bRecursivePaths = true;

    TArray<FAssetData> Assets;
    if (!AssetRegistry().GetAssets(Filter, Assets))
    {
        Result.message = FString::Printf(TEXT("Asset Registry could not query package path: %s"), *Root);
        return Result;
    }

    for (const FAssetData& Asset : Assets)
    {
        const FString AssetName = Asset.AssetName.ToString();
        const FString AssetClass = Asset.AssetClassPath.GetAssetName().ToString();

        if (!name_contains.IsEmpty() && !AssetName.Contains(name_contains, ESearchCase::IgnoreCase))
        {
            continue;
        }

        if (!class_name.IsEmpty() && !AssetClass.Equals(class_name, ESearchCase::IgnoreCase))
        {
            continue;
        }

        if (Result.assets.Num() >= Limit)
        {
            Result.truncated = true;
            break;
        }

        Result.assets.Add(ToAssetInfo(Asset));
    }

    Result.success = true;
    Result.message = FString::Printf(
        TEXT("Returned %d asset(s)%s."),
        Result.assets.Num(),
        Result.truncated ? TEXT(" (truncated)") : TEXT(""));
    return Result;
}

FMCPReferenceResult UUnrealMCPProjectToolset::GetAssetDependencies(const FString& package_name)
{
    FMCPReferenceResult Result;
    Result.package_name = NormalizePackageName(package_name);

    if (!Result.package_name.StartsWith(TEXT("/")))
    {
        Result.message = TEXT("package_name must be a long package name such as /Game/Blueprints/BP_Player.");
        return Result;
    }

    TArray<FName> Dependencies;
    AssetRegistry().GetDependencies(FName(*Result.package_name), Dependencies);

    for (const FName Dependency : Dependencies)
    {
        Result.packages.Add(Dependency.ToString());
    }
    Result.packages.Sort();

    Result.success = true;
    Result.message = FString::Printf(TEXT("Found %d package dependency/dependencies."), Result.packages.Num());
    return Result;
}

FMCPReferenceResult UUnrealMCPProjectToolset::GetAssetReferencers(const FString& package_name)
{
    FMCPReferenceResult Result;
    Result.package_name = NormalizePackageName(package_name);

    if (!Result.package_name.StartsWith(TEXT("/")))
    {
        Result.message = TEXT("package_name must be a long package name such as /Game/Blueprints/BP_Player.");
        return Result;
    }

    TArray<FName> Referencers;
    AssetRegistry().GetReferencers(FName(*Result.package_name), Referencers);

    for (const FName Referencer : Referencers)
    {
        Result.packages.Add(Referencer.ToString());
    }
    Result.packages.Sort();

    Result.success = true;
    Result.message = FString::Printf(TEXT("Found %d referencer package(s)."), Result.packages.Num());
    return Result;
}
