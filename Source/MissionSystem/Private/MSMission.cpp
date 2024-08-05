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

    const auto * subsystem = Cast< UMSMissionSystem >( GetOuter() );
    check( subsystem != nullptr );

    const auto & mission_history = subsystem->GetMissionHistory();

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

        if ( mission_history.IsObjectiveFinished( objective_data.Objective ) )
        {
            continue;
        }

        if ( CanExecuteObjective( objective_data.Objective ) )
        {
            // Insert in reverse order as objectives to start will be popped out of the list
            PendingObjectives.Insert( objective_data.Objective, 0 );
        }
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

bool UMSMission::CompleteObjective( const TSubclassOf< UMSMissionObjective > & objective_class )
{
    if ( auto * objective = ActiveObjectives.FindByPredicate( [ & ]( const TObjectPtr< UMSMissionObjective > & active_objective ) {
             return objective_class == active_objective->GetClass();
         } ) )
    {
        ( *objective )->CompleteObjective();
        return true;
    }

    return false;
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