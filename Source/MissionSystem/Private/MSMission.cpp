#include "MSMission.h"

#include "MSLog.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"
#include "MSMissionObjective.h"

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
        OnMissionCompleteDelegate.Broadcast( Data );
    } );
}

void UMSMission::Start()
{
    StartActionsExecutor.Execute();
}

void UMSMission::End()
{
    for ( auto * objective : Objectives )
    {
        objective->CompleteObjective();
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

void UMSMission::OnObjectiveCompleted( UMSMissionObjective * mission_objective )
{
    if ( !bIsStarted )
    {
        return;
    }

    mission_objective->OnMissionObjectiveComplete().RemoveDynamic( this, &UMSMission::OnObjectiveCompleted );
    OnMissionObjectiveCompleteDelegate.Broadcast( mission_objective );
    ExecuteNextObjective();
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
        objective->OnMissionObjectiveComplete().AddDynamic( this, &UMSMission::OnObjectiveCompleted );

        UE_LOG( LogMissionSystem, Verbose, TEXT( "Execute objective %s" ), *objective->GetClass()->GetName() );

        objective->Execute();
    }
    else
    {
        TryEnd();
    }
}
