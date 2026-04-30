// Copyright (c) 2026 Rakudai. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FRKDCommonMapsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterContextMenu();
	void AddMapToCategory(FName CategoryName);
	void RemoveMapFromCategory(FName CategoryName);
	void BuildAddCategorySubmenu(FMenuBuilder& MenuBuilder);
	void BuildRemoveCategorySubmenu(FMenuBuilder& MenuBuilder);

	FDelegateHandle ToolMenusHandle;
};