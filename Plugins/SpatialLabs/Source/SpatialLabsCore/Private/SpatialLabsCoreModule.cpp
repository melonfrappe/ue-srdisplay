#include "SpatialLabsCoreModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "SpatialLabsCore"

void FSpatialLabsCoreModule::StartupModule()
{
    CreateSettings();
}

void FSpatialLabsCoreModule::ShutdownModule()
{

}

USpatialLabsCoreSettings* FSpatialLabsCoreModule::GetCoreSettings()
{
    return CoreSettings;
}

void FSpatialLabsCoreModule::CreateSettings()
{
    CoreSettings = GetMutableDefault<USpatialLabsCoreSettings>();
    CoreSettings->Save();
}

IMPLEMENT_MODULE(FSpatialLabsCoreModule, SpatialLabsCore)

#undef LOCTEXT_NAMESPACE
