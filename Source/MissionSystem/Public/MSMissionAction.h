#pragma once

#include <CoreMinimal.h>
#include <UObject/NoExportTypes.h>

#include "MSMissionAction.generated.h"

class UMSMissionAction;

DECLARE_MULTICAST_DELEGATE_OneParam( FMSOnMissionActionCompleteDelegate, UMSMissionAction * );

UCLASS( Blueprintable, DefaultToInstanced, EditInlineNew )
class MISSIONSYSTEM_API UMSMissionAction : public UObject
{
    GENERATED_BODY()

public:
    FMSOnMissionActionCompleteDelegate & OnMissionActionComplete();

    void Initialize( UObject * world_context );

    /* Executes the actions. You must call FinishExecute to notify the parent objective / mission it can continue execution  */
    UFUNCTION( BlueprintNativeEvent )
    void Execute();

    UFUNCTION( BlueprintCallable )
    void FinishExecute();

    UWorld * GetWorld() const override;

protected:
    FMSOnMissionActionCompleteDelegate OnMissionActionCompleteEvent;
    TWeakObjectPtr< UObject > Outer;
};

FORCEINLINE FMSOnMissionActionCompleteDelegate & UMSMissionAction::OnMissionActionComplete()
{
    return OnMissionActionCompleteEvent;
}