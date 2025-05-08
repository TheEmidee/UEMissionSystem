#pragma once

#include "MSObjectiveViewModel.h"

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSMissionViewModel.generated.h"

class UMSMissionObjective;
class UMSMission;
class UMSObjectiveViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FMSViewModelMissionObjectiveStartedMulticastDynamicDelegate, UMSMissionViewModel *, Mission, UMSObjectiveViewModel *, MissionObjective, int, CurrentProgression );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSViewModelMissionObjectiveProgressionUpdatedMulticastDynamicDelegate, UMSMissionViewModel *, Mission, UMSObjectiveViewModel *, MissionObjective );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FMSViewModelMissionObjectiveEndedMulticastDynamicDelegate, UMSMissionViewModel *, Mission, UMSObjectiveViewModel *, MissionObjective, bool, WasCancelled );

UCLASS()
class MISSIONSYSTEMVIEWMODELS_API UMSMissionViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void RemoveCompletedObjective( UMSObjectiveViewModel * objective_vm );

    UMSMission * GetMission() const;

    void Initialize( UMSMission * mission );
    UMSObjectiveViewModel * SetObjectiveStarted(const TSubclassOf< UMSMissionObjective > & objective, int current_progression);
    UMSObjectiveViewModel * UpdateObjectiveProgression( const TSubclassOf< UMSMissionObjective > & objective, int current_progression );
    UMSObjectiveViewModel * SetObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective, bool was_cancelled );

private:
    UPROPERTY( BlueprintReadOnly, EditAnywhere, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    FText Name;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > ActiveObjectives;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > CompletedObjectives;

    UPROPERTY( Transient )
    TObjectPtr< UMSMission > Mission;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionObjectiveStartedMulticastDynamicDelegate OnMissionObjectiveStartedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionObjectiveProgressionUpdatedMulticastDynamicDelegate OnMissionObjectiveProgressionIsUpdatedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionObjectiveEndedMulticastDynamicDelegate OnMissionObjectiveEndedDelegate;
};

FORCEINLINE UMSMission * UMSMissionViewModel::GetMission() const
{
    return Mission;
}