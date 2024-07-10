#pragma once

#include "MSMissionTypes.h"

#include <CoreMinimal.h>
#include <GameplayTagAssetInterface.h>
#include <GameplayTagContainer.h>

#include "MSMissionObjective.generated.h"

class UMSMissionAction;
class UMSMissionObjective;

DECLARE_EVENT_OneParam( UMSMissionObjective, FMSOnMissionObjectiveStartedEvent, UMSMissionObjective * MissionObjective );
DECLARE_EVENT_TwoParams( UMSMissionObjective, FMSOnMissionObjectiveEndedEvent, UMSMissionObjective * MissionObjective, bool WasCancelled );

UCLASS( Abstract, BlueprintType, Blueprintable )
class MISSIONSYSTEM_API UMSMissionObjective : public UObject, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    UMSMissionObjective();

    friend class UMSMission;

    FMSOnMissionObjectiveStartedEvent & OnMissionObjectiveStarted();
    FMSOnMissionObjectiveEndedEvent & OnMissionObjectiveEnded();

    const FGuid & GetObjectiveId() const;
    bool IsComplete() const;
    bool IsCancelled() const;
    void Execute();
    void PostLoad() override;
    void PostDuplicate( bool duplicate_for_pie ) override;
    void PostEditImport() override;

    UFUNCTION( BlueprintCallable )
    void CompleteObjective();

    UWorld * GetWorld() const override;

    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

    void SerializeState( FArchive & archive );

protected:
    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute();

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnObjectiveEnded" )
    void K2_OnObjectiveEnded( bool was_cancelled );

    void CancelObjective();
    void RegenerateGuidIfNeeded();

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > StartActions;

    UPROPERTY( EditDefaultsOnly, Instanced, Category = "Actions" )
    TArray< TObjectPtr< UMSMissionAction > > EndActions;

    UPROPERTY()
    FMSActionExecutor StartActionsExecutor;

    UPROPERTY()
    FMSActionExecutor EndActionsExecutor;

    UPROPERTY( EditAnywhere, Category = "Tags" )
    FGameplayTagContainer Tags;

    UPROPERTY( EditAnywhere, Category = "Actions" )
    uint8 bExecuteEndActionsWhenCancelled : 1;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsComplete;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    bool bIsCancelled;

    UPROPERTY( VisibleAnywhere, AdvancedDisplay )
    FGuid ObjectiveId;

    FMSOnMissionObjectiveStartedEvent OnObjectiveStartedEvent;
    FMSOnMissionObjectiveEndedEvent OnObjectiveCompleteEvent;
};

FORCEINLINE FMSOnMissionObjectiveStartedEvent & UMSMissionObjective::OnMissionObjectiveStarted()
{
    return OnObjectiveStartedEvent;
}

FORCEINLINE FMSOnMissionObjectiveEndedEvent & UMSMissionObjective::OnMissionObjectiveEnded()
{
    return OnObjectiveCompleteEvent;
}

FORCEINLINE const FGuid & UMSMissionObjective::GetObjectiveId() const
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