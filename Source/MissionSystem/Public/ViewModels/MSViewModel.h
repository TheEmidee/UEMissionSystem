#pragma once

#include "MSMissionObjective.h"

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
    void SetMissionStarted( UMSMission * mission );
    void SetMissionEnded( UMSMission * mission );
    void SetMissionObjectiveStarted( UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective );
    void SetMissionObjectiveEnded( UMSMission * mission, const TSubclassOf<UMSMissionObjective> & objective );

private:
    UMSMissionViewModel * GetMissionViewModel( UMSMission * mission ) const;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UMSMissionViewModel > > ActiveMissions;
};
