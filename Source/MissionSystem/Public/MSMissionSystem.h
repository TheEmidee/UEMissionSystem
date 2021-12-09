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
    FMSOnMissionCompleteDelegate & OnMissionComplete();
    FMSOnMissionObjectiveCompleteDelegate & OnMissionObjectiveComplete();

    UFUNCTION( BlueprintCallable )
    UMSMission * StartMission( UMSMissionData * mission_data );

    UFUNCTION( BlueprintPure )
    bool IsMissionComplete( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure )
    bool IsMissionActive( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure )
    UMSMission * GetActiveMission( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintCallable )
    void SkipCurrentMissions() const;

    bool ShouldCreateSubsystem( UObject * outer ) const override;

private:
    UFUNCTION()
    void OnMissionComplete( UMSMissionData * mission_data );

    UFUNCTION()
    void OnMissionObjectiveComplete( UMSMissionObjective * objective );

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionCompleteDelegate OnMissionCompleteDelegate;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveCompleteDelegate OnMissionObjectiveCompleteDelegate;

    UPROPERTY()
    TMap< UMSMissionData *, UMSMission * > ActiveMissions;

    UPROPERTY()
    TArray< UMSMissionData * > CompletedMissions;
};

FORCEINLINE FMSOnMissionCompleteDelegate & UMSMissionSystem::OnMissionComplete()
{
    return OnMissionCompleteDelegate;
}

FORCEINLINE FMSOnMissionObjectiveCompleteDelegate & UMSMissionSystem::OnMissionObjectiveComplete()
{
    return OnMissionObjectiveCompleteDelegate;
}