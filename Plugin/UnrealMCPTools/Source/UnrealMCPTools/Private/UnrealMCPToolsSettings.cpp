#include "UnrealMCPToolsSettings.h"

UUnrealMCPToolsSettings::UUnrealMCPToolsSettings()
{
    AllowedAssetRoots = { TEXT("/Game") };
}

FName UUnrealMCPToolsSettings::GetCategoryName() const
{
    return TEXT("Plugins");
}
