/**
 * Copyright 2019,2020,2021,2022 Sony Corporation
 */
#pragma once

#include "Modules/ModuleManager.h"
#include "SRDisplaySystem.h"

class FSRDisplayModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TUniquePtr<srdisplay_module::FSRDisplaySystem> SRDispalySystem;

};
