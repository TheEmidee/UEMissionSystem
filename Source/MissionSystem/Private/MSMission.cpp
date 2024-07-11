#include "MSMission.h"

#include "MSLog.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"
#include "MSMissionObjective.h"
#include "MSMissionSystem.h"

#include <Engine/World.h>

UMSMission::UMSMission() :
    Data( nullptr ),
    bIsStarted( false ),
    bIsCancelled( false )
{
}

UWorld * UMSMission::GetWorld() const
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

void UMSMission::Initialize( UMSMissionData * mission_data )
{
    Data = mission_data;

    ActiveObjectives.Reserve( mission_data->Objectives.Num() );
    PendingObjectives.Reserve( mission_data->Objectives.Num() );

    for ( const auto & objective_data : mission_data->Objectives )
    {
        if ( !objective_data.bEnabled )
        {
            continue;
        }

        if ( !ensureAlwaysMsgf( IsValid( objective_data.Objective ), TEXT( "%s has an invalid Mission Objective!" ), *mission_data->GetName() ) )
        {
            continue;
        }

        /*auto * objective = NewObject< UMSMissionObjective >( this, objective_data.Objective );
        check( objective );*/

        if ( CanExecuteObjective( objective_data.Objective ) )
        {
            // ActiveObjectives.Add( objective );

            // Insert in reverse order as objectives to start will be popped out of the list
            PendingObjectives.Insert( objective_data.Objective, 0 );
        }
        /*else
        {
            objective->MarkAsGarbage();
        }*/
    }

    StartActionsExecutor.Initialize( *this, mission_data->StartActions, [ this ]() {
        TryStart();
    } );

    EndActionsExecutor.Initialize( *this, mission_data->EndActions, [ this ]() {
        ensure( IsComplete() || bIsCancelled );
        ActiveObjectives.Empty();
        OnMissionEndedEvent.Broadcast( this, bIsCancelled );
    } );
}

void UMSMission::Start()
{
    StartActionsExecutor.Execute();
}

void UMSMission::Complete()
{
    for ( auto objective : ActiveObjectives )
    {
        objective->CompleteObjective();
    }
}

void UMSMission::Cancel()
{
    bIsCancelled = true;

    for ( auto objective : ActiveObjectives )
    {
        objective->CancelObjective();
    }

    if ( Data->bExecuteEndActionsWhenCancelled )
    {
        EndActionsExecutor.Execute();
    }
    else
    {
        OnMissionEndedEvent.Broadcast( this, bIsCancelled );
    }
}

bool UMSMission::IsComplete() const
{
    for ( auto objective : ActiveObjectives )
    {
        if ( !objective->IsComplete() )
        {
            return false;
        }
    }

    return true;
}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
void UMSMission::DumpMission( FOutputDevice & output_device )
{
    const auto get_status = []( bool is_complete, bool is_cancelled ) {
        return FString( is_complete ? TEXT( "Completed" ) : ( is_cancelled ? TEXT( "Cancelled" ) : TEXT( "OnGoing" ) ) );
    };

    output_device.Logf( ELogVerbosity::Verbose,
        TEXT( " * Mission : %s - Status: %s" ),
        *GetNameSafe( GetMissionData() ),
        *get_status( IsComplete(), bIsCancelled ) );

    for ( auto objective : ActiveObjectives )
    {
        output_device.Logf(
            ELogVerbosity::Verbose,
            TEXT( "   - Objective : %s - Status : %s" ),
            *GetNameSafe( objective ),
            *get_status( objective->IsComplete(), objective->IsCancelled() ) );
    }
}
#endif

void UMSMission::SerializeState( FArchive & archive )
{
    archive << bIsStarted;
    archive << bIsCancelled;

    if ( archive.IsSaving() )
    {
        auto num_objectives = ActiveObjectives.Num();
        archive << num_objectives;

        for ( auto objective : ActiveObjectives )
        {
            objective->SerializeState( archive );
        }

        auto num_pending_objectives = PendingObjectives.Num();
        archive << num_pending_objectives;

        /*for ( auto objective : PendingObjectives )
        {
            objective->SerializeState( archive );
        }*/
    }
}

void UMSMission::OnObjectiveCompleted( UMSMissionObjective * mission_objective, const bool was_cancelled )
{
    if ( !bIsStarted )
    {
        return;
    }

    mission_objective->OnObjectiveEnded().RemoveAll( this );
    OnMissionObjectiveCompleteEvent.Broadcast( mission_objective->GetClass(), was_cancelled );

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
        auto objective_class = PendingObjectives.Pop();

        if ( !CanExecuteObjective( objective_class ) )
        {
            /*objective->MarkAsGarbage();
            ActiveObjectives.Remove( objective );*/
            ExecuteNextObjective();
            return;
        }

        auto * objective = NewObject< UMSMissionObjective >( this, objective_class );
        ActiveObjectives.Add( objective );

        objective->OnObjectiveEnded().AddUObject( this, &UMSMission::OnObjectiveCompleted );

        UE_LOG( LogMissionSystem, Verbose, TEXT( "Execute objective %s" ), *objective->GetClass()->GetName() );

        objective->Execute();
        OnMissionObjectiveStartedEvent.Broadcast( objective->GetClass() );
    }
    else
    {
        TryEnd();
    }
}

bool UMSMission::CanExecuteObjective( const TSubclassOf< UMSMissionObjective > & objective_class ) const
{
#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    if ( const auto * mission_system = GetWorld()->GetSubsystem< UMSMissionSystem >() )
    {
        // return !mission_system->MustObjectiveBeIgnored( objective_class );
    }
#endif

    return true;
}