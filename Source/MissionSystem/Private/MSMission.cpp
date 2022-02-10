#include "MSMission.h"

#include "MSLog.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"
#include "MSMissionObjective.h"

UMSMission::UMSMission()
{
    bIsStarted = false;
    bIsCancelled = false;
}

void UMSMission::Initialize( UMSMissionData * mission_data )
{
    Data = mission_data;

    Objectives.Reserve( mission_data->Objectives.Num() );
    PendingObjectives.Reserve( mission_data->Objectives.Num() );

    for ( const auto & objective_data : mission_data->Objectives )
    {
        if ( !objective_data.bEnabled )
        {
            continue;
        }

        auto * objective = NewObject< UMSMissionObjective >( this, objective_data.Objective );
        check( objective );

        Objectives.Add( objective );

        // Insert in reverse order as objectives to start will be popped out of the list
        PendingObjectives.Insert( objective, 0 );
    }

    StartActionsExecutor.Initialize( *this, mission_data->StartActions, [ this ]() {
        TryStart();
    } );

    EndActionsExecutor.Initialize( *this, mission_data->EndActions, [ this ]() {
        OnMissionEndedDelegate.Broadcast( Data, bIsCancelled );
    } );
}

void UMSMission::Start()
{
    StartActionsExecutor.Execute();
}

void UMSMission::Cancel()
{
    bIsCancelled = true;

    for ( auto * objective : Objectives )
    {
        objective->CancelObjective();
    }

    if ( Data->bExecuteEndActionsWhenCancelled )
    {
        EndActionsExecutor.Execute();
    }
    else
    {
        OnMissionEndedDelegate.Broadcast( Data, bIsCancelled );
    }
}

bool UMSMission::IsComplete() const
{
    for ( auto * objective : Objectives )
    {
        if ( !objective->IsComplete() )
        {
            return false;
        }
    }

    return true;
}

void UMSMission::OnObjectiveCompleted( UMSMissionObjective * mission_objective, const bool was_cancelled )
{
    if ( !bIsStarted )
    {
        return;
    }

    mission_objective->OnMissionObjectiveEnded().RemoveDynamic( this, &UMSMission::OnObjectiveCompleted );
    OnMissionObjectiveCompleteDelegate.Broadcast( mission_objective, was_cancelled );

    if ( bIsCancelled )
    {
        return;
    }

    if ( !was_cancelled )
    {
        ExecuteNextObjective();
    }
}

void UMSMission::TryStart()
{
    if ( bIsStarted )
    {
        return;
    }

    bIsStarted = true;

    ExecuteNextObjective();
}

void UMSMission::TryEnd()
{
    if ( IsComplete() )
    {
        EndActionsExecutor.Execute();
    }
}

void UMSMission::ExecuteNextObjective()
{
    if ( PendingObjectives.Num() > 0 )
    {
        auto * objective = PendingObjectives.Pop();
        objective->OnMissionObjectiveEnded().AddDynamic( this, &UMSMission::OnObjectiveCompleted );

        UE_LOG( LogMissionSystem, Verbose, TEXT( "Execute objective %s" ), *objective->GetClass()->GetName() );

        objective->Execute();
    }
    else
    {
        TryEnd();
    }
}
