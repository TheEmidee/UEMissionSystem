#pragma once

#include "Chaos/AABB.h"
#include "MSMissionObjective.h"

#include <CoreMinimal.h>

#include "MSMission.generated.h"

class UMSMissionAction;
class UMSMissionData;
class UMSMissionObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FMSOnMissionCompleteDelegate, UMSMissionData *, MissionData );

UCLASS()
class MISSIONSYSTEM_API UMSMission final : public UObject
{
    GENERATED_BODY()

public:
    UMSMissionData * GetData() const;
    FMSOnMissionCompleteDelegate & OnMissionComplete();
    FMSOnMissionObjectiveCompleteDelegate & OnMissionObjectiveComplete();
    const TArray< UMSMissionObjective * > & GetObjectives() const;
    const TArray< UMSMissionAction * > & GetStartActions() const;
    bool IsStarted() const;

    void Initialize( UMSMissionData * mission_data );
    void Start();
    void End();

    UFUNCTION( BlueprintPure )
    bool IsComplete() const;

private:
    UFUNCTION()
    void OnObjectiveCompleted( UMSMissionObjective * objective );

    void TryStart();
    void TryEnd();
    void ExecuteNextObjective();

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    UMSMissionData * Data;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionCompleteDelegate OnMissionCompleteDelegate;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveCompleteDelegate OnMissionObjectiveCompleteDelegate;

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
};

FORCEINLINE UMSMissionData * UMSMission::GetData() const
{
    return Data;
}

FORCEINLINE FMSOnMissionCompleteDelegate & UMSMission::OnMissionComplete()
{
    return OnMissionCompleteDelegate;
}

FORCEINLINE FMSOnMissionObjectiveCompleteDelegate & UMSMission::OnMissionObjectiveComplete()
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