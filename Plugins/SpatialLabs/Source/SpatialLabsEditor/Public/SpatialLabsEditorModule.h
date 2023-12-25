#pragma once

#include "Modules/ModuleInterface.h"
#include "SpatialLabsEditorSettings.h"

class SPATIALLABSEDITOR_API FSpatialLabsEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual USpatialLabsEditorSettings* GetSettings();

    void RegisterSettings();
    void UnregisterSettings() const;

private:

    USpatialLabsEditorSettings* EditorSettings = nullptr;

};
