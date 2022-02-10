#pragma once

#include "Chaos/AABB.h"
#include "MSMissionObjective.h"

#include <CoreMinimal.h>

#include "MSMission.generated.h"

class UMSMissionAction;
class UMSMissionData;
class UMSMissionObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSOnMissionEndedDelegate, UMSMissionData *, MissionData, bool, WasCancelled );

UCLASS()
class MISSIONSYSTEM_API UMSMission final : public UObject
{
    GENERATED_BODY()

public:
    friend class UMSMissionSystem;

    UMSMission();

    FMSOnMissionEndedDelegate & OnMissionEnded();
    FMSOnMissionObjectiveEndedDelegate & OnMissionObjectiveEnded();
    const TArray< UMSMissionObjective * > & GetObjectives() const;
    const TArray< UMSMissionAction * > & GetStartActions() const;
    bool IsStarted() const;

    void Initialize( UMSMissionData * mission_data );
    void Start();
    void Complete();
    void Cancel();

    UFUNCTION( BlueprintPure )
    bool IsComplete() const;

private:
    UFUNCTION()
    void OnObjectiveCompleted( UMSMissionObjective * objective, bool was_cancelled );

    void TryStart();
    void TryEnd();
    void ExecuteNextObjective();

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    UMSMissionData * Data;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionEndedDelegate OnMissionEndedDelegate;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveEndedDelegate OnMissionObjectiveCompleteDelegate;

    UPROPERTY()
    TArray< UMSMissionObjective * > Objectives;

    UPROPERTY()
    TArray< UMSMissionObjective * > PendingObjectives;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsStarted : 1;

    uint8 bIsCancelled : 1;
};

FORCEINLINE FMSOnMissionEndedDelegate & UMSMission::OnMissionEnded()
{
    return OnMissionEndedDelegate;
}

FORCEINLINE FMSOnMissionObjectiveEndedDelegate & UMSMission::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveCompleteDelegate;
}

FORCEINLINE const TArray< UMSMissionObjective * > & UMSMission::GetObjectives() const
{
    return Objectives;
}

FORCEINLINE const TArray< UMSMissionAction * > & UMSMission::GetStartActions() const
{
    return StartActionsExecutor.GetInstancedActions();
}

FORCEINLINE bool UMSMission::IsStarted() const
{
    return bIsStarted;
}