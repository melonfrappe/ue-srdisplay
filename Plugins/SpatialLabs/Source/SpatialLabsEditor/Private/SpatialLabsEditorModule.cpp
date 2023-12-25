#include "SpatialLabsEditorModule.h"
#include "ISettingsModule.h"

#define LOCTEXT_NAMESPACE "SpatialLabsEditor"

void FSpatialLabsEditorModule::StartupModule()
{
    RegisterSettings();
}

void FSpatialLabsEditorModule::ShutdownModule()
{
    UnregisterSettings();
}

USpatialLabsEditorSettings* FSpatialLabsEditorModule::GetSettings()
{
    return EditorSettings;
}

void FSpatialLabsEditorModule::RegisterSettings()
{
    EditorSettings = GetMutableDefault<USpatialLabsEditorSettings>();
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings(
            "Project", "Plugins", "SpatialLabs",
            LOCTEXT("RuntimeSettingsName", "SpatialLabs"),
            LOCTEXT("RuntimeSettingsDescription", "Configure SpatialLabs"),
            EditorSettings
        );
    }
}

void FSpatialLabsEditorModule::UnregisterSettings() const
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "SpatialLabs");
    }
}

IMPLEMENT_MODULE(FSpatialLabsEditorModule, SpatialLabsEditor)

#undef LOCTEXT_NAMESPACE
