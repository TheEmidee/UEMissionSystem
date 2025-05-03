#pragma once

#include <CoreMinimal.h>
#include <MVVMViewModelBase.h>

#include "MSObjectiveViewModel.generated.h"

class UMSMissionObjective;

UCLASS()
class MISSIONSYSTEMVIEWMODELS_API UMSObjectiveViewModel final : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    const UMSMissionObjective * GetObjective() const;

    void RefreshProgression();
    void Initialize( const UMSMissionObjective * objective );

    UFUNCTION( BlueprintPure, FieldNotify )
    FText GetDescription() const;

private:
    UPROPERTY()
    const UMSMissionObjective * Objective;
};

FORCEINLINE const UMSMissionObjective * UMSObjectiveViewModel::GetObjective() const
{
    return Objective;
}