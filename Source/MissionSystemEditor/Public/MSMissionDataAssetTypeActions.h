#pragma once

#include "AssetTypeActions_Base.h"

class FMSMissionDataAssetTypeActions final : public FAssetTypeActions_Base
{
public:
    FMSMissionDataAssetTypeActions( EAssetTypeCategories::Type category );

    FText GetName() const override;
    FColor GetTypeColor() const override;
    UClass * GetSupportedClass() const override;
    uint32 GetCategories() override;

private:
    EAssetTypeCategories::Type Category;
};

class FMSMissionObjectiveAssetTypeActions final : public FAssetTypeActions_Base
{
public:
    FMSMissionObjectiveAssetTypeActions( EAssetTypeCategories::Type category );

    FText GetName() const override;
    FColor GetTypeColor() const override;
    UClass * GetSupportedClass() const override;
    uint32 GetCategories() override;

private:
    EAssetTypeCategories::Type Category;
};
