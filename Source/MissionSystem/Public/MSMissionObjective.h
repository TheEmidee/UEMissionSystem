#pragma once

#include "MSMissionTypes.h"

#include <CoreMinimal.h>

#include "MSMissionObjective.generated.h"

class UMSMissionAction;
class UMSMissionObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FMSOnMissionObjectiveCompleteDelegate, UMSMissionObjective *, MissionObjective );

UCLASS( Abstract, BlueprintType, Blueprintable )
class MISSIONSYSTEM_API UMSMissionObjective : public UObject
{
    GENERATED_BODY()

public:
    UMSMissionObjective();

    FMSOnMissionObjectiveCompleteDelegate & OnMissionObjectiveComplete();
    bool IsComplete() const;

    void Execute();

    UFUNCTION( BlueprintCallable )
    void CompleteObjective();

    UWorld * GetWorld() const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

protected:
    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute();

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnObjectiveEnded" )
    void K2_OnObjectiveEnded();

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UMSMissionAction > > EndActions;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

private:
    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveCompleteDelegate OnObjectiveCompleteDelegate;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsComplete : 1;
};

FORCEINLINE FMSOnMissionObjectiveCompleteDelegate & UMSMissionObjective::OnMissionObjectiveComplete()
{
    return OnObjectiveCompleteDelegate;
}

FORCEINLINE bool UMSMissionObjective::IsComplete() const
{
    return bIsComplete;
}