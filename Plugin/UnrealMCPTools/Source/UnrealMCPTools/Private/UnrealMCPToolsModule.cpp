#include "UnrealMCPToolsModule.h"

#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogUnrealMCPTools, Log, All);

void FUnrealMCPToolsModule::StartupModule()
{
    UE_LOG(LogUnrealMCPTools, Log, TEXT("Unreal MCP Tools loaded."));
}

void FUnrealMCPToolsModule::ShutdownModule()
{
    UE_LOG(LogUnrealMCPTools, Log, TEXT("Unreal MCP Tools unloaded."));
}

IMPLEMENT_MODULE(FUnrealMCPToolsModule, UnrealMCPTools)
