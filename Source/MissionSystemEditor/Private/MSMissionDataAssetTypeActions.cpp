#include "MSMissionDataAssetTypeActions.h"

#include "MSMissionData.h"

FMSMissionDataAssetTypeActions::FMSMissionDataAssetTypeActions( EAssetTypeCategories::Type category ) :
    Category( category )
{
}

FText FMSMissionDataAssetTypeActions::GetName() const
{
    return INVTEXT( "Mission Data" );
}

FColor FMSMissionDataAssetTypeActions::GetTypeColor() const
{
    return FColor::Cyan;
}

UClass * FMSMissionDataAssetTypeActions::GetSupportedClass() const
{
    return UMSMissionData::StaticClass();
}

uint32 FMSMissionDataAssetTypeActions::GetCategories()
{
    return Category;
}