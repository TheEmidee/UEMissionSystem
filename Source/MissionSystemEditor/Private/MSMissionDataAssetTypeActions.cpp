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

FMSMissionObjectiveAssetTypeActions::FMSMissionObjectiveAssetTypeActions( EAssetTypeCategories::Type category ) :
    Category( category )
{
}

FText FMSMissionObjectiveAssetTypeActions::GetName() const
{
    return INVTEXT( "Mission Objective" );
}

FColor FMSMissionObjectiveAssetTypeActions::GetTypeColor() const
{
    return FColor::Cyan;
}

UClass * FMSMissionObjectiveAssetTypeActions::GetSupportedClass() const
{
    return UMSMissionObjective::StaticClass();
}

uint32 FMSMissionObjectiveAssetTypeActions::GetCategories()
{
    return Category;
}