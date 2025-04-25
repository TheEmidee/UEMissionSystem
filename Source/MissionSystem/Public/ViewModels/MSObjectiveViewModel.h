#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSObjectiveViewModel.generated.h"

class UMSMissionObjective;

UCLASS()
class MISSIONSYSTEM_API UMSObjectiveViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    UMSMissionObjective * GetObjective() const;

    void RefreshProgression();
    void Initialize( UMSMissionObjective * objective );

    UFUNCTION( BlueprintPure, FieldNotify )
    FText GetDescription() const;

private:
    UPROPERTY()
    UMSMissionObjective * Objective;
};

FORCEINLINE UMSMissionObjective * UMSObjectiveViewModel::GetObjective() const
{
    return Objective;
}