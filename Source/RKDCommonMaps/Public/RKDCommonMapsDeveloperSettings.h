// Copyright (c) 2026 Rakudai. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RKDCommonMapsDeveloperSettings.generated.h"

USTRUCT(BlueprintType)
struct FCommonMapContainer
{
	GENERATED_BODY()
	FCommonMapContainer() = default;
	explicit FCommonMapContainer(const TArray<FSoftObjectPath>& InPaths) : MapURL(InPaths){}
	
	UPROPERTY(EditAnywhere, Category="", meta=(AllowedClasses="/Script/Engine.World"))
	TSet<FSoftObjectPath> MapURL;
};

/**
 * 
 */
UCLASS(config=EditorPerProjectUserSettings, DisplayName="RKD Common Maps")
class RKDCOMMONMAPS_API URKDCommonMapsDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	virtual FName GetCategoryName() const override;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(Config,EditAnywhere, Category="Common Maps|Maps", meta=(ForceInlineRow))
	TMap<FName, FCommonMapContainer> Maps;
#endif
};
