#pragma once

#include "MSMissionTypes.h"

#include <CoreMinimal.h>
#include <GameplayTagAssetInterface.h>
#include <GameplayTagContainer.h>

#include "MSMissionObjective.generated.h"

class UMSMissionAction;
class UMSMissionObjective;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSOnMissionObjectiveEndedDelegate, UMSMissionObjective *, MissionObjective, bool, WasCancelled );

UCLASS( Abstract, BlueprintType, Blueprintable )
class MISSIONSYSTEM_API UMSMissionObjective : public UObject, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    UMSMissionObjective();

    friend class UMSMission;

    FMSOnMissionObjectiveEndedDelegate & OnMissionObjectiveEnded();
    bool IsComplete() const;
    bool IsCancelled() const;

    void Execute();

    UFUNCTION( BlueprintCallable )
    void CompleteObjective();

    UWorld * GetWorld() const override;

    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

protected:
    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute();

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnObjectiveEnded" )
    void K2_OnObjectiveEnded( bool was_cancelled );

    void CancelObjective();

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

    UPROPERTY( EditAnywhere )
    FGameplayTagContainer Tags;

    UPROPERTY( EditAnywhere )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsComplete : 1;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsCancelled : 1;
};

FORCEINLINE FMSOnMissionObjectiveEndedDelegate & UMSMissionObjective::OnMissionObjectiveEnded()
{
    return OnObjectiveCompleteDelegate;
}

FORCEINLINE bool UMSMissionObjective::IsComplete() const
{
    return bIsComplete;
}

FORCEINLINE bool UMSMissionObjective::IsCancelled() const
{
    return bIsCancelled;
}