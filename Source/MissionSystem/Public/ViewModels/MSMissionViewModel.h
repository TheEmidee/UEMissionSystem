#pragma once

#include "MSObjectiveViewModel.h"

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSMissionViewModel.generated.h"

class UMSMissionObjective;
class UMSMission;
class UMSObjectiveViewModel;

DECLARE_MULTICAST_DELEGATE( FMSOnObjectiveStatusChangedDelegate )

UCLASS()
class MISSIONSYSTEM_API UMSMissionViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    FMSOnObjectiveStatusChangedDelegate & OnObjectiveStatusChanged();

    UFUNCTION( BlueprintCallable )
    void RemoveCompletedObjective( UMSObjectiveViewModel * objective_vm );

    UMSMission * GetMission() const;

    void Initialize( UMSMission * mission );
    void SetObjectiveStarted( const UMSMissionObjective * objective );
    void RefreshObjectiveProgression( const UMSMissionObjective * objective );
    void SetObjectiveEnded( UMSMissionObjective * objective );

private:
    UPROPERTY( BlueprintReadOnly, EditAnywhere, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    FText Name;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > ActiveObjectives;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > CompletedObjectives;

    UPROPERTY( Transient )
    TObjectPtr< UMSMission > Mission;

    FMSOnObjectiveStatusChangedDelegate OnObjectiveStatusChangedDelegate;
};

FORCEINLINE FMSOnObjectiveStatusChangedDelegate & UMSMissionViewModel::OnObjectiveStatusChanged()
{
    return OnObjectiveStatusChangedDelegate;
}

FORCEINLINE UMSMission * UMSMissionViewModel::GetMission() const
{
    return Mission;
}