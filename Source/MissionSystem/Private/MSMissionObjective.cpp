#include "MSMissionObjective.h"

#include "MSMissionAction.h"

UMSMissionObjective::UMSMissionObjective()
{
    bExecuteEndActionsWhenCancelled = false;
    bIsComplete = false;
    bIsCancelled = false;
}

void UMSMissionObjective::Execute()
{
    StartActionsExecutor.Initialize( *this, StartActions, [ this ]() {
        K2_Execute();
    } );

    EndActionsExecutor.Initialize( *this, EndActions, [ this ]() {
        OnObjectiveCompleteDelegate.Broadcast( this, bIsCancelled );
    } );

    StartActionsExecutor.Execute();
}

void UMSMissionObjective::CompleteObjective()
{
    if ( !bIsComplete )
    {
        bIsComplete = true;
        K2_OnObjectiveEnded( false );
        EndActionsExecutor.Execute();
    }
}

UWorld * UMSMissionObjective::GetWorld() const
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

void UMSMissionObjective::CancelObjective()
{
    if ( !bIsComplete && !bIsCancelled )
    {
        bIsCancelled = true;

        K2_OnObjectiveEnded( true );

        if ( bExecuteEndActionsWhenCancelled )
        {
            EndActionsExecutor.Execute();
        }
        else
        {
            OnObjectiveCompleteDelegate.Broadcast( this, bIsCancelled );
        }
    }
}

void UMSMissionObjective::K2_Execute_Implementation()
{
}

void UMSMissionObjective::K2_OnObjectiveEnded_Implementation( bool /*was_cancelled*/ )
{
}
