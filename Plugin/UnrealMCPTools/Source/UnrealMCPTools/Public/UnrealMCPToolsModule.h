#pragma once

#include "Modules/ModuleManager.h"

class FUnrealMCPToolsModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
