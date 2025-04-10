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
    const TSubclassOf< UMSMissionObjective > & GetObjectiveClass() const;

    void Initialize( const TSubclassOf< UMSMissionObjective > & objective_class );
    void SetProgression( int progression );

private:
    UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    FText Name;

    UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    int CurrentProgression;

    UPROPERTY( BlueprintReadOnly, FieldNotify, Category = "ViewModel", meta = ( AllowPrivateAccess ) )
    int RequiredProgression;

    TSubclassOf< UMSMissionObjective > ObjectiveClass;
};

FORCEINLINE const TSubclassOf< UMSMissionObjective > & UMSObjectiveViewModel::GetObjectiveClass() const
{
    return ObjectiveClass;
}