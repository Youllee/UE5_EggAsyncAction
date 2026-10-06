#pragma once

#include "Modules/ModuleManager.h"

class FEggAsyncModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
