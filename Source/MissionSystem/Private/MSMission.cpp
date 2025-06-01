#include "MSMission.h"

#include "MSLog.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"
#include "MSMissionObjective.h"
#include "MSMissionSystemComponent.h"

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

    const auto & mission_history = GetMissionHistory();

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
            UE_LOG( LogMissionSystem, Verbose, TEXT( "Don't add objective %s because it is already complete." ), *objective_data.Objective->GetClass()->GetName() );
            continue;
        }

        if ( CanExecuteObjective( objective_data.Objective ) )
        {
            // Insert in reverse order as objectives to start will be popped out of the list
            PendingObjectives.Insert( objective_data.Objective, 0 );
        }
    }

    StartActionsExecutor.Initialize( this, mission_data->StartActions, [ this ]() {
        TryStart();
    } );

    EndActionsExecutor.Initialize( this, mission_data->EndActions, [ this ]() {
        ensure( IsComplete() || bIsCancelled );
        ActiveObjectives.Empty();
        OnMissionEndedEvent.Broadcast( this, bIsCancelled );
    } );
}

void UMSMission::Start()
{
    // If the mission has no objectives then we just broadcast the event
    // This can happen by design
    // Or if the game is saved only when objectives are completed. When the game is reloaded, it has saved that the last objective
    // was completed, but not the mission. This mission would start on the next load, but all its objectives would be complete.
    if ( ActiveObjectives.IsEmpty() && PendingObjectives.IsEmpty() )
    {
        OnMissionEndedEvent.Broadcast( this, false );
        return;
    }
    StartActionsExecutor.Execute();
}

void UMSMission::Complete()
{
    auto copy = ActiveObjectives;

    for ( auto objective : copy )
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

#if !UE_BUILD_SHIPPING
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

const FMSMissionHistory & UMSMission::GetMissionHistory() const
{
    auto * component = Cast< UMSMissionSystemComponent >( GetOuter() );
    check( component != nullptr );

    return component->GetMissionHistory();
}

FMSMissionHistory & UMSMission::GetMissionHistory()
{
    auto * component = Cast< UMSMissionSystemComponent >( GetOuter() );
    check( component != nullptr );

    return const_cast< FMSMissionHistory & >( component->GetMissionHistory() );
}

void UMSMission::OnObjectiveCompleted( UMSMissionObjective * mission_objective, const bool was_cancelled )
{
    if ( !bIsStarted )
    {
        return;
    }

    mission_objective->OnObjectiveEnded().RemoveAll( this );
    mission_objective->OnObjectiveProgressionUpdated().RemoveAll( this );
    OnMissionObjectiveCompleteEvent.Broadcast( mission_objective, was_cancelled );

    if ( bIsCancelled )
    {
        return;
    }

    if ( Data->bMustCompleteObjectivesSequentially )
    {
        if ( !was_cancelled )
        {
            ExecuteNextObjective();
        }
    }
    else
    {
        TryEnd();
    }
}

void UMSMission::OnObjectiveProgressionUpdated( UMSMissionObjective * mission_objective )
{
    OnMissionObjectiveProgressionUpdatedEvent.Broadcast( mission_objective );
}

void UMSMission::TryStart()
{
    if ( bIsStarted )
    {
        return;
    }

    bIsStarted = true;

    if ( Data->bMustCompleteObjectivesSequentially )
    {
        ExecuteNextObjective();
    }
    else
    {
        ExecuteAllObjectives();
    }
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

        CreateObjective( objective_class );
    }
    else
    {
        TryEnd();
    }
}

void UMSMission::ExecuteAllObjectives()
{
    if ( PendingObjectives.Num() > 0 )
    {
        while ( !PendingObjectives.IsEmpty() )
        {
            auto objective_class = PendingObjectives.Pop();

            if ( CanExecuteObjective( objective_class ) )
            {
                CreateObjective( objective_class );
            }
        }
    }
    else
    {
        TryEnd();
    }
}

bool UMSMission::CanExecuteObjective( const TSubclassOf< UMSMissionObjective > & objective_class ) const
{
#if !UE_BUILD_SHIPPING
    // :TODO:
#endif

    return true;
}

void UMSMission::CreateObjective( const TSubclassOf< UMSMissionObjective > & objective_class )
{
    auto * objective = NewObject< UMSMissionObjective >( this, objective_class );
    ActiveObjectives.Add( objective );

    objective->OnObjectiveEnded().AddUObject( this, &UMSMission::OnObjectiveCompleted );
    objective->OnObjectiveProgressionUpdated().AddUObject( this, &UMSMission::OnObjectiveProgressionUpdated );

    UE_LOG( LogMissionSystem, Verbose, TEXT( "Execute objective %s" ), *objective->GetClass()->GetName() );

    GetMissionHistory().InitializeObjective( objective );

    objective->Execute();

    if ( !objective->IsComplete() )
    {
        OnMissionObjectiveStartedEvent.Broadcast( objective );
    }
}