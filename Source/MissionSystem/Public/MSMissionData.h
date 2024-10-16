#pragma once

#include "MSMissionAction.h"
#include "MSMissionObjective.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "MSMissionData.generated.h"

USTRUCT( BlueprintType )
struct MISSIONSYSTEM_API FMSMissionObjectiveData
{
    GENERATED_USTRUCT_BODY()

    FMSMissionObjectiveData();
    FMSMissionObjectiveData( const TSubclassOf< UMSMissionObjective > & objective, bool enabled = true );

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UMSMissionObjective > Objective;

    UPROPERTY( EditDefaultsOnly )
    uint8 bEnabled : 1;
};

UCLASS( BlueprintType )
class MISSIONSYSTEM_API UMSMissionData final : public UDataAsset
{
    GENERATED_BODY()

public:
    UMSMissionData();

    const FGuid & GetGuid() const;
    void PostLoad() override;
    void PostDuplicate( bool duplicate_for_pie ) override;
    void PostEditImport() override;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FText Name;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly )
    FText Description;

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > EndActions;

    UPROPERTY( EditDefaultsOnly, Category = "ActiveObjectives" )
    TArray< FMSMissionObjectiveData > Objectives;

    UPROPERTY( EditDefaultsOnly, Category = "Other missions" )
    TArray< UMSMissionData * > NextMissions;

    UPROPERTY( EditDefaultsOnly, Category = "Other missions" )
    TArray< UMSMissionData * > MissionsToCancel;

    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bEnabled : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Options" )
    uint8 bStartNextMissionsWhenCancelled : 1;

    UPROPERTY( VisibleAnywhere, AdvancedDisplay )
    FGuid MissionId;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    void GenerateGuidIfNeeded( bool force_generation = false );
};

FORCEINLINE const FGuid & UMSMissionData::GetGuid() const
{
    return MissionId;
}