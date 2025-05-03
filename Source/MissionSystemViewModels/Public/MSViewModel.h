#pragma once

#include "MSMissionObjective.h"
#include "MSMissionViewModel.h"

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSViewModel.generated.h"

class UMSMission;
class UMSMissionViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FMSOnMissionsObjectivesChangedDelegate );

UCLASS()
class MISSIONSYSTEMVIEWMODELS_API UMSViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void RemoveCompletedMission( UMSMissionViewModel * mission_vm );

    void SetMissionStarted( UMSMission * mission );
    void SetMissionEnded( UMSMission * mission );
    void RefreshMissionObjectiveProgression( const UMSMission * mission, const UMSMissionObjective * objective ) const;
    void SetMissionObjectiveStarted( const UMSMission * mission, UMSMissionObjective * objective ) const;
    void SetMissionObjectiveEnded( const UMSMission * mission, UMSMissionObjective * objective ) const;

    UFUNCTION( BlueprintPure, FieldNotify )
    bool HasActiveMissions() const;

private:
    UMSMissionViewModel * GetMissionViewModel( const UMSMission * mission ) const;
    void BroadCastOnMissionsObjectivesChanged();

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > ActiveMissions;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > CompletedMissions;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess ) )
    FMSOnMissionsObjectivesChangedDelegate OnMissionsObjectivesChangedDelegate;
};
