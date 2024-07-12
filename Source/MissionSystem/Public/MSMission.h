#pragma once

#include "MSMissionObjective.h"

#include <CoreMinimal.h>

#include "MSMission.generated.h"

class UMSMissionAction;
class UMSMissionData;
class UMSMissionObjective;
class UMSMission;

DECLARE_EVENT_TwoParams( UMSMission, FMSOnMissionEndedEvent, UMSMission * Mission, bool WasCancelled );
DECLARE_EVENT_OneParam( UMSMission, FMSOnMissionObjectiveStartedEvent, const TSubclassOf< UMSMissionObjective > & MissionObjective );
DECLARE_EVENT_TwoParams( UMSMission, FMSOnMissionObjectiveEndedEvent, const TSubclassOf< UMSMissionObjective > & MissionObjective, bool WasCancelled );

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

    UMSMissionData * GetMissionData() const;

private:
    void OnObjectiveCompleted( UMSMissionObjective * mission_objective, bool was_cancelled );
    void TryStart();
    void TryEnd();

    UFUNCTION()
    void ExecuteNextObjective();

    bool CanExecuteObjective( const TSubclassOf< UMSMissionObjective > & objective_class ) const;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    UMSMissionData * Data;

    FMSOnMissionEndedEvent OnMissionEndedEvent;
    FMSOnMissionObjectiveStartedEvent OnMissionObjectiveStartedEvent;
    FMSOnMissionObjectiveEndedEvent OnMissionObjectiveCompleteEvent;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TArray< TObjectPtr< UMSMissionObjective > > ActiveObjectives;

    UPROPERTY()
    TArray< TSubclassOf< UMSMissionObjective > > PendingObjectives;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsStarted;

    bool bIsCancelled;
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
    return ActiveObjectives;
}

FORCEINLINE const TArray< UMSMissionAction * > & UMSMission::GetStartActions() const
{
    return StartActionsExecutor.GetInstancedActions();
}

FORCEINLINE bool UMSMission::IsStarted() const
{
    return bIsStarted;
}

FORCEINLINE UMSMissionData * UMSMission::GetMissionData() const
{
    return Data;
}