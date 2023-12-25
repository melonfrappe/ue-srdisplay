#pragma once

#include "Modules/ModuleInterface.h"
#include "SpatialLabsCoreSettings.h"

class SPATIALLABSCORE_API FSpatialLabsCoreModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static inline FSpatialLabsCoreModule& Get() {
        if (IsInGameThread())
        {
            return FModuleManager::LoadModuleChecked<FSpatialLabsCoreModule>("SpatialLabsCore");
        }
        else
        {
            return FModuleManager::GetModuleChecked<FSpatialLabsCoreModule>("SpatialLabsCore");
        }
    }

    static inline bool IsAvailable() {
        return FModuleManager::Get().IsModuleLoaded("SpatialLabsCore");
    }

    virtual USpatialLabsCoreSettings* GetCoreSettings();

private:

    void CreateSettings();

    USpatialLabsCoreSettings* CoreSettings = nullptr; 
};
