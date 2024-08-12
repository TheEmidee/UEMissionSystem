#pragma once

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
    UMSMission * GetMission() const;

    void Initialize( UMSMission * mission );
    void SetObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective );
    void SetObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective );

private:
    UPROPERTY( BlueprintReadOnly, EditAnywhere, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    FText Name;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSObjectiveViewModel > > ActiveObjectives;

    UPROPERTY( Transient )
    TObjectPtr< UMSMission > Mission;
};

FORCEINLINE UMSMission * UMSMissionViewModel::GetMission() const
{
    return Mission;
}