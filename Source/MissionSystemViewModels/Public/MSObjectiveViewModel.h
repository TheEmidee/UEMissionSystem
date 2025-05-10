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
    const TSubclassOf< UMSMissionObjective > & GetObjective() const;

    void UpdateProgression(int current_progression);
    void Initialize(const TSubclassOf< UMSMissionObjective > & objective, int current_progression);
    void SetCompleted( bool was_cancelled );

    UFUNCTION( BlueprintPure, FieldNotify )
    FText GetDescription() const;

private:
    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    bool bIsCompleted = false;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    bool bWasCancelled = false;

    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess ) )
    TSubclassOf< UMSMissionObjective > Objective;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    int CurrentProgression;

    UPROPERTY( BlueprintReadOnly, FieldNotify, meta = ( AllowPrivateAccess ) )
    int RequiredProgression;
};

FORCEINLINE const TSubclassOf< UMSMissionObjective > & UMSObjectiveViewModel::GetObjective() const
{
    return Objective;
}