#pragma once

#include "MSMissionTypes.h"

#include <CoreMinimal.h>

#include "MSMissionObjective.generated.h"

class UMSMissionAction;
class UMSMissionObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSOnMissionObjectiveEndedDelegate, UMSMissionObjective *, MissionObjective, bool, WasCancelled );

UCLASS( Abstract, BlueprintType, Blueprintable )
class MISSIONSYSTEM_API UMSMissionObjective : public UObject
{
    GENERATED_BODY()

public:
    UMSMissionObjective();

    FMSOnMissionObjectiveEndedDelegate & OnMissionObjectiveEnded();
    bool IsComplete() const;

    void Execute();

    UFUNCTION( BlueprintCallable )
    void CompleteObjective();

    UWorld * GetWorld() const override;

protected:

    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute();

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnObjectiveEnded" )
    void K2_OnObjectiveEnded( bool was_cancelled );


    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UMSMissionAction > > EndActions;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveEndedDelegate OnObjectiveCompleteDelegate;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsComplete : 1;
};

FORCEINLINE FMSOnMissionObjectiveEndedDelegate & UMSMissionObjective::OnMissionObjectiveEnded()
{
    return OnObjectiveCompleteDelegate;
}

FORCEINLINE bool UMSMissionObjective::IsComplete() const
{
    return bIsComplete;
}