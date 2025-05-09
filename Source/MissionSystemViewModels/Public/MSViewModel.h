#pragma once

#include "MSMissionObjective.h"
#include "MSMissionSystemComponent.h"
#include "MSMissionViewModel.h"

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSViewModel.generated.h"

class UMSMissionData;
class UMSMission;
class UMSMissionViewModel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FMSViewModelMissionStartedMulticastDynamicDelegate, UMSMissionViewModel *, Mission );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSViewModelMissionEndedMulticastDynamicDelegate, UMSMissionViewModel *, Mission, bool, bWasCancelled );

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
    void OnMissionStarted( UMSMission * mission );

    UFUNCTION()
    void OnMissionEnded( const UMSMissionData * mission, bool was_cancelled );

    UFUNCTION()
    void OnMissionObjectiveProgressionUpdated( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression, int required_progression );

    UFUNCTION()
    void OnMissionObjectiveStarted(const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression);

    UFUNCTION()
    void OnMissionObjectiveEnded( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, bool was_cancelled );

    UMSMissionViewModel * GetMissionViewModel( const UMSMissionData * mission_data ) const;
    void BroadCastOnMissionsObjectivesChanged();

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > ActiveMissions;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > CompletedMissions;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionStartedMulticastDynamicDelegate OnMissionStartedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionEndedMulticastDynamicDelegate OnMissionEndedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionObjectiveStartedMulticastDynamicDelegate OnMissionObjectiveStartedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionObjectiveProgressionUpdatedMulticastDynamicDelegate OnMissionObjectiveProgressionIsUpdatedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSViewModelMissionObjectiveEndedMulticastDynamicDelegate OnMissionObjectiveEndedDelegate;
};
