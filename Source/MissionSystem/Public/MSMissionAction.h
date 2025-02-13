#pragma once

#include "GameFramework/PlayerController.h"

#include <CoreMinimal.h>
#include <UObject/NoExportTypes.h>

#include "MSMissionAction.generated.h"

class UMSMissionSystemComponent;
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
    void Execute();

    UFUNCTION( BlueprintCallable )
    void FinishExecute();

    UWorld * GetWorld() const override;

protected:
    /* Executes the actions. You must call FinishExecute to notify the parent objective / mission it can continue execution  */
    UFUNCTION( BlueprintNativeEvent, DisplayName = "Execute" )
    void K2_Execute( APlayerController * player_controller, UMSMissionSystemComponent * mission_system_component );

    FMSOnMissionActionCompleteDelegate OnMissionActionCompleteEvent;
    TWeakObjectPtr< UObject > Outer;
};

FORCEINLINE FMSOnMissionActionCompleteDelegate & UMSMissionAction::OnMissionActionComplete()
{
    return OnMissionActionCompleteEvent;
}