#include "MSMissionAction.h"

void UMSMissionAction::Execute_Implementation()
{
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

    if ( const auto * outer = GetOuter() )
    {
        return outer->GetWorld();
    }

    return nullptr;
}
