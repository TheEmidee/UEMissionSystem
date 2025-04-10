#pragma once

#include "MSObjectiveViewModel.h"

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSMissionViewModel.generated.h"

class UMSMissionObjective;
class UMSMission;
class UMSObjectiveViewModel;

UCLASS()
class MISSIONSYSTEM_API UMSMissionViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void RemoveCompletedObjective( UMSObjectiveViewModel * objective_vm );

    UMSMission * GetMission() const;

    void Initialize( UMSMission * mission );
    void SetObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective );
    void SetObjectiveProgression( const TSubclassOf< UMSMissionObjective > & objective, int current_progression );
    void SetObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective );

private:
    UPROPERTY( BlueprintReadOnly, EditAnywhere, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    FText Name;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > ActiveObjectives;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > CompletedObjectives;

    UPROPERTY( Transient )
    TObjectPtr< UMSMission > Mission;
};

FORCEINLINE UMSMission * UMSMissionViewModel::GetMission() const
{
    return Mission;
}