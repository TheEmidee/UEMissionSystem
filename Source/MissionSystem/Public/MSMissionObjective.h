#pragma once

#include "MSMissionTypes.h"

#include <CoreMinimal.h>
#include <GameplayTagAssetInterface.h>
#include <GameplayTagContainer.h>

#include "MSMissionObjective.generated.h"

struct FMSMissionHistory;
class UMSMissionAction;
class UMSMissionObjective;
class APlayerController;
class UMSMissionSystemComponent;

DECLARE_EVENT_TwoParams( UMSMissionObjective, FMSOnObjectiveEndedEvent, UMSMissionObjective * MissionObjective, bool WasCancelled );
DECLARE_EVENT_ThreeParams( UMSMissionObjective, FMSOnObjectiveProgressionUpdatedEvent, UMSMissionObjective * MissionObjective, int CurrentProgression, int RequiredProgression );

UCLASS( Abstract, BlueprintType, Blueprintable )
class MISSIONSYSTEM_API UMSMissionObjective : public UObject, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    UMSMissionObjective();

    FMSOnObjectiveEndedEvent & OnObjectiveEnded();
    FMSOnObjectiveProgressionUpdatedEvent & OnObjectiveProgressionUpdated();
    const FText & GetDescription() const;
    int GetRequiredProgression() const;

    const FGuid & GetGuid() const;
    bool IsComplete() const;
    bool IsCancelled() const;
    void Execute();
    void PostLoad() override;
    void PostDuplicate( bool duplicate_for_pie ) override;
    void PostEditImport() override;

    UFUNCTION( BlueprintCallable )
    void CompleteObjective();

    UFUNCTION( BlueprintCallable )
    void IncrementProgression( int steps );

    void CancelObjective();

    UWorld * GetWorld() const override;

    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

protected:
    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute( APlayerController * player_controller, UMSMissionSystemComponent * mission_system_component );

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnObjectiveEnded" )
    void K2_OnObjectiveEnded( bool was_cancelled );

    void GenerateGuidIfNeeded( bool force_generation = false );

    // The actions to execute when this objective is started
    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > StartActions;

    // The actions to execute when this objective is ended
    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > EndActions;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

    // The description of the objective
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Infos", meta = ( AllowPrivateAccess = true ) )
    FText Description;

    // The tags attached to the objective
    UPROPERTY( EditDefaultsOnly, Category = "Tags" )
    FGameplayTagContainer Tags;

    // Set to true to execute the end actions even when this objective is cancelled
    UPROPERTY( EditDefaultsOnly, Category = "Actions" )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    // This flag is set to true when this objective is complete
    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsComplete;

    // This flag is set to true when this objective is cancelled
    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsCancelled;

    // Setting this to a value > 0 makes the objective have progression.
    // You need to call IncrementProgression to report progression
    // CompleteObjective will be called automatically when the progression is over
    // Calling CompleteObjective before will instantly complete the progression
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    int RequiredProgression;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    int CurrentProgression;

    // The id of the objective. It's used by the mission history. It's auto-generated
    UPROPERTY( VisibleAnywhere, AdvancedDisplay )
    FGuid ObjectiveId;

    FMSOnObjectiveEndedEvent OnObjectiveCompleteEvent;
    FMSOnObjectiveProgressionUpdatedEvent OnObjectiveProgressionUpdatedEvent;
};

FORCEINLINE FMSOnObjectiveEndedEvent & UMSMissionObjective::OnObjectiveEnded()
{
    return OnObjectiveCompleteEvent;
}

FORCEINLINE FMSOnObjectiveProgressionUpdatedEvent & UMSMissionObjective::OnObjectiveProgressionUpdated()
{
    return OnObjectiveProgressionUpdatedEvent;
}

FORCEINLINE const FText & UMSMissionObjective::GetDescription() const
{
    return Description;
}

FORCEINLINE int UMSMissionObjective::GetRequiredProgression() const
{
    return RequiredProgression;
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