#include "MSMission.h"

#include "MSLog.h"
#include "MSMissionAction.h"
#include "MSMissionData.h"
#include "MSMissionObjective.h"
#include "MSMissionSystem.h"

#include <Engine/World.h>
#include <Kismet/KismetStringLibrary.h>

UMSMission::UMSMission()
{
    bIsStarted = false;
    bIsCancelled = false;
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

    Objectives.Reserve( mission_data->Objectives.Num() );
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

        auto * objective = NewObject< UMSMissionObjective >( this, objective_data.Objective );
        check( objective );

        if ( CanExecuteObjective( objective ) )
        {
            Objectives.Add( objective );

            // Insert in reverse order as objectives to start will be popped out of the list
            PendingObjectives.Insert( objective, 0 );
        }
        else
        {
            objective->MarkAsGarbage();
        }
    }

    StartActionsExecutor.Initialize( *this, mission_data->StartActions, [ this ]() {
        TryStart();
    } );

    EndActionsExecutor.Initialize( *this, mission_data->EndActions, [ this ]() {
        ensure( IsComplete() || bIsCancelled );
        Objectives.Empty();
        OnMissionEndedDelegate.Broadcast( Data, bIsCancelled );
    } );
}

void UMSMission::Start()
{
    StartActionsExecutor.Execute();
}

void UMSMission::Complete()
{
    for ( auto * objective : Objectives )
    {
        objective->CompleteObjective();
    }
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
    for ( const auto * objective : Objectives )
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
    const FString status = IsComplete() ? "Completed" : ( bIsCancelled ? "Cancelled" : "OnGoing" );

    output_device.Logf( ELogVerbosity::Verbose,
        TEXT( " * Mission : %s - Status: %s" ),
        *GetNameSafe( GetMissionData() ),
        *status );

    for ( const auto * objective : Objectives )
    {
        const FString objective_status = objective->IsComplete() ? "Completed" : ( objective->IsCancelled() ? "Cancelled" : "OnGoing" );
        output_device.Logf(
            ELogVerbosity::Verbose,
            TEXT( "   - Objective : %s - Status : %s" ),
            *GetNameSafe( objective ),
            *objective_status );
    }
}
#endif

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

        if ( !CanExecuteObjective( objective ) )
        {
            objective->MarkAsGarbage();
            Objectives.Remove( objective );
            ExecuteNextObjective();
            return;
        }

        objective->OnMissionObjectiveEnded().AddDynamic( this, &UMSMission::OnObjectiveCompleted );

        UE_LOG( LogMissionSystem, Verbose, TEXT( "Execute objective %s" ), *objective->GetClass()->GetName() );

        objective->Execute();
    }
    else
    {
        TryEnd();
    }
}

bool UMSMission::CanExecuteObjective( UMSMissionObjective * objective ) const
{
#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    if ( const auto * mission_system = GetWorld()->GetSubsystem< UMSMissionSystem >() )
    {
        return !mission_system->MustObjectiveBeIgnored( objective );
    }
#endif

    return true;
}
