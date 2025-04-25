#pragma once

#include "MSMissionObjective.h"
#include "MSMissionViewModel.h"

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSViewModel.generated.h"

class UMSMission;
class UMSMissionViewModel;

UCLASS()
class MISSIONSYSTEM_API UMSViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void RemoveCompletedMission( UMSMissionViewModel * mission_vm );

    void SetMissionStarted( UMSMission * mission );
    void SetMissionEnded( UMSMission * mission );
    void RefreshMissionObjectiveProgression( UMSMission * mission, UMSMissionObjective * objective ) const;
    void SetMissionObjectiveStarted( UMSMission * mission, UMSMissionObjective * objective ) const;
    void SetMissionObjectiveEnded( UMSMission * mission, UMSMissionObjective * objective ) const;

    UFUNCTION( BlueprintPure, FieldNotify )
    bool HasActiveMissions() const;

private:
    UMSMissionViewModel * GetMissionViewModel( UMSMission * mission ) const;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > ActiveMissions;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > CompletedMissions;
};
