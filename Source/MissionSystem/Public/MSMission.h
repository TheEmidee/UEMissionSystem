#pragma once

#include "Chaos/AABB.h"
#include "MSMissionObjective.h"

#include <CoreMinimal.h>

#include "MSMission.generated.h"

class UMSMissionAction;
class UMSMissionData;
class UMSMissionObjective;

DECLARE_EVENT_TwoParams( UMSMission, FMSOnMissionEndedEvent, UMSMissionData * MissionData, bool WasCancelled );

UCLASS()
class MISSIONSYSTEM_API UMSMission final : public UObject
{
    GENERATED_BODY()

public:
    friend class UMSMissionSystem;

    UMSMission();

    UWorld * GetWorld() const override;

    FMSOnMissionEndedEvent & OnMissionEnded();
    FMSOnMissionObjectiveStartedEvent & OnMissionObjectiveStarted();
    FMSOnMissionObjectiveEndedEvent & OnMissionObjectiveEnded();
    const TArray< UMSMissionObjective * > & GetObjectives() const;
    const TArray< UMSMissionAction * > & GetStartActions() const;
    bool IsStarted() const;

    void Initialize( UMSMissionData * mission_data );
    void Start();
    void Complete();
    void Cancel();

    UFUNCTION( BlueprintPure )
    bool IsComplete() const;

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    void DumpMission( FOutputDevice & output_device );
#endif

    const UMSMissionData * GetMissionData() const;

private:
    void OnObjectiveStarted( UMSMissionObjective * mission_objective );
    void OnObjectiveCompleted( UMSMissionObjective * objective, bool was_cancelled );
    void TryStart();
    void TryEnd();

    UFUNCTION()
    void ExecuteNextObjective();

    bool CanExecuteObjective( UMSMissionObjective * objective ) const;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    UMSMissionData * Data;

    FMSOnMissionEndedEvent OnMissionEndedEvent;
    FMSOnMissionObjectiveStartedEvent OnMissionObjectiveStartedEvent;
    FMSOnMissionObjectiveEndedEvent OnMissionObjectiveCompleteEvent;

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

FORCEINLINE FMSOnMissionEndedEvent & UMSMission::OnMissionEnded()
{
    return OnMissionEndedEvent;
}

FORCEINLINE FMSOnMissionObjectiveStartedEvent & UMSMission::OnMissionObjectiveStarted()
{
    return OnMissionObjectiveStartedEvent;
}

FORCEINLINE FMSOnMissionObjectiveEndedEvent & UMSMission::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveCompleteEvent;
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

FORCEINLINE const UMSMissionData * UMSMission::GetMissionData() const
{
    return Data;
}