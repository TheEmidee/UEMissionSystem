#pragma once

#include <CoreMinimal.h>

#include "MSMissionTypes.generated.h"

class UMSMissionAction;

USTRUCT()
struct MISSIONSYSTEM_API FMSActionExecutor
{
    GENERATED_BODY()

public:
    const TArray< UMSMissionAction * > & GetInstancedActions() const;

    void Initialize( UObject & action_owner, const TArray< UMSMissionAction * > & action_classes, TFunction< void() > callback );
    void Execute();

private:
    void OnActionExecuted( UMSMissionAction * action );
    void TryExecuteCallback();

    UPROPERTY()
    TArray< UMSMissionAction * > InstancedActions;

    UPROPERTY()
    TArray< UMSMissionAction * > PendingActions;

    TFunction< void() > Callback;
};

FORCEINLINE const TArray< UMSMissionAction * > & FMSActionExecutor::GetInstancedActions() const
{
    return InstancedActions;
}