#pragma once

#include "MSMissionTypes.h"

#include <CoreMinimal.h>
#include <GameplayTagAssetInterface.h>
#include <GameplayTagContainer.h>

#include "MSMissionObjective.generated.h"

class UMSMissionAction;
class UMSMissionObjective;

DECLARE_EVENT_TwoParams( UMSMissionObjective, FMSOnObjectiveEndedEvent, UMSMissionObjective * MissionObjective, bool WasCancelled );

UCLASS( Abstract, BlueprintType, Blueprintable )
class MISSIONSYSTEM_API UMSMissionObjective : public UObject, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    UMSMissionObjective();

    FMSOnObjectiveEndedEvent & OnObjectiveEnded();
    const FText & GetDescription() const;

    const FGuid & GetGuid() const;
    bool IsComplete() const;
    bool IsCancelled() const;
    void Execute();
    void PostLoad() override;
    void PostDuplicate( bool duplicate_for_pie ) override;
    void PostEditImport() override;

    UFUNCTION( BlueprintCallable )
    void CompleteObjective();

    void CancelObjective();

    UWorld * GetWorld() const override;

    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

protected:
    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute();

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnObjectiveEnded" )
    void K2_OnObjectiveEnded( bool was_cancelled );

    void GenerateGuidIfNeeded( bool force_generation = false );

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > EndActions;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Infos", meta = ( AllowPrivateAccess = true ) )
    FText Description;

    UPROPERTY( EditDefaultsOnly, Category = "Tags" )
    FGameplayTagContainer Tags;

    UPROPERTY( EditDefaultsOnly, Category = "Actions" )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsComplete;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsCancelled;

    UPROPERTY( VisibleAnywhere, AdvancedDisplay )
    FGuid ObjectiveId;

    FMSOnObjectiveEndedEvent OnObjectiveCompleteEvent;
};

FORCEINLINE FMSOnObjectiveEndedEvent & UMSMissionObjective::OnObjectiveEnded()
{
    return OnObjectiveCompleteEvent;
}

FORCEINLINE const FText & UMSMissionObjective::GetDescription() const
{
    return Description;
}

FORCEINLINE const FGuid & UMSMissionObjective::GetGuid() const
{
    return ObjectiveId;
}

FORCEINLINE bool UMSMissionObjective::IsComplete() const
{
    return bIsComplete;
}

FORCEINLINE bool UMSMissionObjective::IsCancelled() const
{
    return bIsCancelled;
}