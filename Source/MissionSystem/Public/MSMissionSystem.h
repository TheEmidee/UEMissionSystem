#pragma once

#include "MSMission.h"
#include "MSMissionData.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "MSMissionSystem.generated.h"

class UMSMissionData;

UCLASS()
class MISSIONSYSTEM_API UMSMissionSystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    FMSOnMissionEndedDelegate & OnMissionEnded();
    FMSOnMissionObjectiveEndedDelegate & OnMissionObjectiveEnded();

    UFUNCTION( BlueprintCallable )
    UMSMission * StartMission( UMSMissionData * mission_data );

    UFUNCTION( BlueprintPure )
    bool IsMissionComplete( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure )
    bool IsMissionActive( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure )
    UMSMission * GetActiveMission( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintCallable )
    void CancelCurrentMissions() const;

    bool ShouldCreateSubsystem( UObject * outer ) const override;

private:
    UFUNCTION()
    void OnMissionEnded( UMSMissionData * mission_data, bool was_cancelled );

    UFUNCTION()
    void OnMissionObjectiveEnded( UMSMissionObjective * objective, bool was_cancelled );

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionEndedDelegate OnMissionCompleteDelegate;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveEndedDelegate OnMissionObjectiveCompleteDelegate;

    UPROPERTY()
    TMap< UMSMissionData *, UMSMission * > ActiveMissions;

    UPROPERTY()
    TArray< UMSMissionData * > CompletedMissions;
};

FORCEINLINE FMSOnMissionEndedDelegate & UMSMissionSystem::OnMissionEnded()
{
    return OnMissionCompleteDelegate;
}

FORCEINLINE FMSOnMissionObjectiveEndedDelegate & UMSMissionSystem::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveCompleteDelegate;
}