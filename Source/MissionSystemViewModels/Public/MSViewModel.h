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
    void Initialize( UMSMissionSystemComponent * component );

    UFUNCTION( BlueprintCallable )
    void RemoveCompletedMission( UMSMissionViewModel * mission_vm );

    UFUNCTION( BlueprintPure, FieldNotify )
    bool HasActiveMissions() const;

    UFUNCTION( BlueprintCallable )
    static UMSViewModel * CreateMissionSystemViewModel( UMSMissionSystemComponent * component );

private:
    UFUNCTION()
    void SetMissionStarted( UMSMission * mission );

    UFUNCTION()
    void SetMissionEnded( const UMSMissionData * mission, bool was_cancelled );

    UFUNCTION()
    void RefreshMissionObjectiveProgression( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression, int required_progression );

    UFUNCTION()
    void SetMissionObjectiveStarted( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > objective );

    UFUNCTION()
    void SetMissionObjectiveEnded( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > objective, bool was_cancelled );

    UMSMissionViewModel * GetMissionViewModel( const UMSMissionData * mission_data ) const;
    void BroadCastOnMissionsObjectivesChanged();

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > ActiveMissions;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > CompletedMissions;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess ) )
    FMSOnMissionsObjectivesChangedDelegate OnMissionsObjectivesChangedDelegate;
};
