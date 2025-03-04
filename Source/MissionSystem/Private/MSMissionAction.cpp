#include "MSMissionAction.h"

#include "MSMissionSystemComponent.h"

#include <GameFramework/PlayerController.h>

void UMSMissionAction::Execute()
{
    APlayerController * pc = nullptr;
    UMSMissionSystemComponent * component;

    auto * object = Outer.Get();
    do
    {
        component = Cast< UMSMissionSystemComponent >( object );
        if ( component != nullptr )
        {
            pc = Cast< APlayerController >( component->GetOwner() );
            break;
        }

        object = object->GetOuter();
    } while ( object != nullptr );

    K2_Execute( pc, component );
}

void UMSMissionAction::Initialize( UObject * world_context )
{
    Outer = world_context;
}

void UMSMissionAction::FinishExecute()
{
    OnMissionActionCompleteEvent.Broadcast( this );
}

UWorld * UMSMissionAction::GetWorld() const
{
    if ( IsTemplate() )
    {
        return nullptr;
    }

    if ( Outer.IsValid() )
    {
        return Outer->GetWorld();
    }

    return nullptr;
}

void UMSMissionAction::K2_Execute_Implementation( APlayerController * player_controller, UMSMissionSystemComponent * mission_system_component )
{
}
