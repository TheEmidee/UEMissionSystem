#include "MSMissionAction.h"

void UMSMissionAction::Execute_Implementation()
{
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
