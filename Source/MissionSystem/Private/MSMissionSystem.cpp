#include "MSMissionSystem.h"

#include "Log/CoreExtLog.h"
#include "MSLog.h"
#include "MSMission.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include <Engine/World.h>

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
static FAutoConsoleCommand SkipMissionsCommand(
    TEXT( "MissionSystem.SkipMissions" ),
    TEXT( "Skips the current missions." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        if ( const auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->CancelCurrentMissions();
        }
    } ) );

static FAutoConsoleCommand CompleteMissionsCommand(
    TEXT( "MissionSystem.CompleteMissions" ),
    TEXT( "Completes the current missions." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        if ( const auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->CompleteCurrentMissions();
        }
    } ) );

static FAutoConsoleCommand ListActiveMissionsCommand(
    TEXT( "MissionSystem.ListActiveMissions" ),
    TEXT( "Prints the active missions in the log." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & output_device ) {
        if ( auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->DumpActiveMissions( output_device );
        }
    } ) );

static FAutoConsoleCommand IgnoreObjectivesWithTag(
    TEXT( "MissionSystem.IgnoreObjectivesWithTag" ),
    TEXT( "Don't start objectives that contain this tag." )
        TEXT( "Can be used multiple times." )
            TEXT( "Objectives already started that match the tags will be completed." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & args, const UWorld * world, FOutputDevice & output_device ) {
        if ( auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->IgnoreObjectivesWithTags( args );
        }
    } ) );

static FAutoConsoleCommand ClearIgnoreObjectivesTags(
    TEXT( "MissionSystem.ClearIgnoreObjectivesTag" ),
    TEXT( "Clears the list of tags used to ignore objectives." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        if ( auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->ClearIgnoreObjectivesTags();
        }
    } ) );
#endif

void UMSMissionSystem::StartMission( UMSMissionData * mission_data )
{
    /*Does this make sense to create helper functions like that?
    How will we do when we will load the savegame?
    We will load the history, but how do we start the active missions?
    We will most probably have to keep an array of UMSMissionData somewhere to create the missions and only activate the active objectives*/

    auto * mission = CreateMissionFromData( mission_data );

    if ( mission == nullptr )
    {
        return;
    }

    StartMission( mission );
}

bool UMSMissionSystem::IsMissionComplete( const UMSMissionData * mission_data ) const
{
    return MissionHistory.IsMissionComplete( mission_data );
}

bool UMSMissionSystem::IsMissionActive( const UMSMissionData * mission_data ) const
{
    return MissionHistory.IsMissionActive( mission_data );
}

UMSMission * UMSMissionSystem::GetActiveMission( const UMSMissionData * mission_data ) const
{
    if ( auto * mission = ActiveMissions.FindByPredicate( [ & ]( const auto * active_mission ) {
             return active_mission->GetMissionData() == mission_data;
         } ) )
    {
        return *mission;
    }

    return nullptr;
}

void UMSMissionSystem::CancelCurrentMissions() const
{
    for ( auto * mission : ActiveMissions )
    {
        mission->Cancel();
    }
}

void UMSMissionSystem::CompleteCurrentMissions() const
{
    for ( auto * mission : ActiveMissions )
    {
        mission->Complete();
    }
}

bool UMSMissionSystem::IsMissionObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const
{
    return MissionHistory.IsObjectiveActive( mission_objective_class );
}

void UMSMissionSystem::WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartsDelegate & when_mission_starts )
{
    if ( IsMissionActive( mission_data ) )
    {
        when_mission_starts.ExecuteIfBound( mission_data );
        return;
    }

    FMissionStartObserver observer;
    observer.MissionData = mission_data;
    observer.Callback = when_mission_starts;

    MissionStartObservers.Emplace( MoveTemp( observer ) );
}

void UMSMissionSystem::WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndsDelegate & when_mission_ends )
{
    if ( IsMissionComplete( mission_data ) )
    {
        when_mission_ends.ExecuteIfBound( mission_data, MissionHistory.IsMissionCancelled( mission_data ) );
        return;
    }

    FMissionEndObserver observer;
    observer.MissionData = mission_data;
    observer.Callback = when_mission_ends;

    MissionEndObservers.Emplace( MoveTemp( observer ) );
}

void UMSMissionSystem::WhenMissionObjectiveStartsOrIsActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveStartsDelegate & when_mission_objective_starts )
{
    if ( IsMissionObjectiveActive( mission_objective_class ) )
    {
        when_mission_objective_starts.ExecuteIfBound( mission_objective_class );
        return;
    }

    FMissionObjectiveStartObserver observer;
    observer.MissionObjective = mission_objective_class;
    observer.Callback = when_mission_objective_starts;

    MissionObjectiveStartObservers.Emplace( MoveTemp( observer ) );
}

void UMSMissionSystem::WhenMissionObjectiveEnds( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveEndsDelegate & when_mission_objective_ends )
{
    FMissionObjectiveEndObserver observer;
    observer.MissionObjective = mission_objective_class;
    observer.Callback = when_mission_objective_ends;

    MissionObjectiveEndObservers.Emplace( MoveTemp( observer ) );
}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
void UMSMissionSystem::DumpActiveMissions( FOutputDevice & output_device )
{
    output_device.Logf( ELogVerbosity::Verbose, TEXT( "Mission System - Active Missions :" ) );
    for ( auto * active_mission : ActiveMissions )
    {
        active_mission->DumpMission( output_device );
    }
}

void UMSMissionSystem::IgnoreObjectivesWithTags( const TArray< FString > & tags )
{
    for ( const auto & tag : tags )
    {
        TagsToIgnoreForObjectives.AddUnique( tag );
    }

    for ( auto * active_mission : ActiveMissions )
    {
        for ( auto * objective : active_mission->GetObjectives() )
        {
            if ( objective->IsComplete() )
            {
                continue;
            }

            if ( MustObjectiveBeIgnored( objective ) )
            {
                objective->CompleteObjective();
            }
        }
    }
}

void UMSMissionSystem::ClearIgnoreObjectivesTags()
{
    TagsToIgnoreForObjectives.Reset();
}

bool UMSMissionSystem::MustObjectiveBeIgnored( const UMSMissionObjective * objective ) const
{
    FGameplayTagContainer objective_tag_container;
    objective->GetOwnedGameplayTags( objective_tag_container );

    TArray< FGameplayTag > objective_tags;
    objective_tag_container.GetGameplayTagArray( objective_tags );

    // Look in the tags of the objective if we can find one that contains any of the ignored tags
    // Note that it does not need to match exactly. A substring is enough to return true
    return objective_tags.FindByPredicate( [ this ]( const FGameplayTag & tag ) {
        const auto objective_tag_name = tag.ToString();
        return TagsToIgnoreForObjectives.ContainsByPredicate( [ &objective_tag_name ]( const FString & tag_to_ignore ) {
            return objective_tag_name.Contains( tag_to_ignore );
        } );
    } ) != nullptr;
}
#endif

bool UMSMissionSystem::ShouldCreateSubsystem( UObject * outer ) const
{
    if ( !Super::ShouldCreateSubsystem( outer ) )
    {
        return false;
    }

    const auto * world = Cast< UWorld >( outer );

    if ( world == nullptr )
    {
        return false;
    }

    if ( world->GetNetDriver() == nullptr )
    {
        return true;
    }

    if ( world->GetNetMode() >= ENetMode::NM_Client )
    {
        return false;
    }

    return true;
}

void UMSMissionSystem::Serialize( FArchive & archive )
{
    /*if ( archive.IsLoading() )
    {
        ActiveMissions.Reset();
        CompletedMissions.Reset();
    }

    archive << CompletedMissions;

    if ( archive.IsSaving() )
    {
        auto num_active_missions = ActiveMissions.Num();
        archive << num_active_missions;

        for ( auto & [ data, mission ] : ActiveMissions )
        {
            archive << data;
            mission->SerializeState( archive );
        }
    }
    else
    {
        auto num_active_missions = 0;
        archive << num_active_missions;

        ActiveMissions.Reserve( num_active_missions );

        for ( auto index = 0; index < num_active_missions; ++index )
        {
            UMSMissionData * mission_data;
            archive << mission_data;

            auto * mission = CreateMissionFromData( mission_data );
            mission->SerializeState( archive );

            StartMission( mission );
        }
    }*/
}

void UMSMissionSystem::K2_WhenMissionStartsOrIsActive( UMSMissionData * mission_data, FMSMissionSystemMissionStartsDynamicDelegate when_mission_starts )
{
    const auto active_delegate = FMSMissionSystemMissionStartsDelegate::CreateWeakLambda( when_mission_starts.GetUObject(), [ when_mission_starts ]( const UMSMissionData * mission_data ) {
        when_mission_starts.ExecuteIfBound( mission_data );
    } );

    WhenMissionStartsOrIsActive( mission_data, active_delegate );
}

void UMSMissionSystem::K2_WhenMissionEnds( UMSMissionData * mission_data, FMSMissionSystemMissionEndsDynamicDelegate when_mission_ends )
{
    const auto ended_delegate = FMSMissionSystemMissionEndsDelegate::CreateWeakLambda( when_mission_ends.GetUObject(), [ when_mission_ends ]( const UMSMissionData * mission_data, const bool was_cancelled ) {
        when_mission_ends.ExecuteIfBound( mission_data, was_cancelled );
    } );

    WhenMissionEnds( mission_data, ended_delegate );
}

void UMSMissionSystem::K2_WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveStartsDynamicDelegate when_mission_objective_starts )
{
    const auto active_delegate = FMSMissionSystemMissionObjectiveStartsDelegate::CreateWeakLambda( when_mission_objective_starts.GetUObject(), [ when_mission_objective_starts ]( TSubclassOf< UMSMissionObjective > mission_objective ) {
        when_mission_objective_starts.ExecuteIfBound( mission_objective );
    } );

    WhenMissionObjectiveStartsOrIsActive( mission_objective, active_delegate );
}

void UMSMissionSystem::K2_WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveEndsDynamicDelegate when_mission_objective_ends )
{
    const auto ended_delegate = FMSMissionSystemMissionObjectiveEndsDelegate::CreateWeakLambda( when_mission_objective_ends.GetUObject(), [ when_mission_objective_ends ]( TSubclassOf< UMSMissionObjective > mission_objective, const bool was_cancelled ) {
        when_mission_objective_ends.ExecuteIfBound( mission_objective, was_cancelled );
    } );

    WhenMissionObjectiveEnds( mission_objective, ended_delegate );
}

UMSMission * UMSMissionSystem::CreateMissionFromData( UMSMissionData * mission_data )
{
    if ( mission_data == nullptr )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with a null mission data" ) );
        return nullptr;
    }

    const auto mission_id = mission_data->GetGuid();
    if ( !mission_id.IsValid() )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with a mission data with an invalid ID" ) );
        return nullptr;
    }

    if ( MissionHistory.IsMissionComplete( mission_data ) )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with an already completed mission" ) );
        return nullptr;
    }

    if ( MissionHistory.IsMissionActive( mission_data ) )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with an already active mission" ) );
        return nullptr;
    }

    check( ActiveMissions.FindByPredicate( [ mission_data ]( const auto * mission ) {
        return mission->GetMissionData() == mission_data;
    } ) == nullptr );

    for ( const auto * mission_to_cancel : mission_data->MissionsToCancel )
    {
        if ( const auto * active_mission_to_cancel_ptr = ActiveMissions.FindByPredicate( [ & ]( auto * active_mission ) {
                 return mission_to_cancel == active_mission->GetMissionData();
             } ) )
        {
            ( *active_mission_to_cancel_ptr )->Cancel();
        }
    }

    if ( !mission_data->bEnabled )
    {
        StartNextMissions( mission_data );
        return nullptr;
    }

    if ( !MissionHistory.AddActiveMission( mission_data ) )
    {
        return nullptr;
    }

    auto * mission = NewObject< UMSMission >( this );
    mission->Initialize( mission_data );

    mission->OnMissionEnded().AddUObject( this, &UMSMissionSystem::OnMissionEnded );
    mission->OnMissionObjectiveStarted().AddUObject( this, &UMSMissionSystem::OnMissionObjectiveStarted );
    mission->OnMissionObjectiveEnded().AddUObject( this, &UMSMissionSystem::OnMissionObjectiveEnded );

    ActiveMissions.Add( mission );

    return mission;
}

void UMSMissionSystem::StartMission( UMSMission * mission )
{
    const auto * mission_data = mission->GetMissionData();

    UE_SLOG( LogMissionSystem, Verbose, TEXT( "Start mission (%s)" ), *GetNameSafe( mission_data ) );
    mission->Start();

    for ( auto index = MissionStartObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = MissionStartObservers[ index ];

        if ( observer.MissionData == mission_data )
        {
            observer.Callback.ExecuteIfBound( mission_data );
            MissionStartObservers.RemoveAt( index );
        }
    }
}

void UMSMissionSystem::StartNextMissions( const UMSMissionData * mission_data )
{
    for ( auto * next_mission : mission_data->NextMissions )
    {
        StartMission( next_mission );
    }
}

void UMSMissionSystem::OnMissionEnded( UMSMission * mission, const bool was_cancelled )
{
    const auto * mission_data = mission->GetMissionData();

    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionEnded (%s)" ), *GetNameSafe( mission_data ) );

    if ( !ensureAlways( MissionHistory.SetMissionComplete( mission_data, was_cancelled ) ) )
    {
        return;
    }

    OnMissionEndedEvent.Broadcast( mission, was_cancelled );
    ActiveMissions.Remove( mission );

    for ( auto index = MissionEndObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = MissionEndObservers[ index ];

        if ( observer.MissionData == mission_data )
        {
            observer.Callback.ExecuteIfBound( mission_data, was_cancelled );
            MissionEndObservers.RemoveAt( index );
        }
    }

    if ( !was_cancelled || mission_data->bStartNextMissionsWhenCancelled )
    {
        StartNextMissions( mission_data );
    }
}

void UMSMissionSystem::OnMissionObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective )
{
    if ( !ensureAlways( MissionHistory.AddActiveObjective( objective ) ) )
    {
        return;
    }

    for ( auto & observer : MissionObjectiveStartObservers )
    {
        observer.Callback.ExecuteIfBound( objective->GetClass() );
    }
}

void UMSMissionSystem::OnMissionObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective, const bool was_cancelled )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnObjectiveEnded (%s)" ), *objective->GetClass()->GetName() );

    if ( !ensureAlways( MissionHistory.SetObjectiveComplete( objective, was_cancelled ) ) )
    {
        return;
    }

    OnMissionObjectiveEndedEvent.Broadcast( objective, was_cancelled );

    for ( auto index = MissionObjectiveEndObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = MissionObjectiveEndObservers[ index ];

        if ( observer.MissionObjective == objective->GetClass() )
        {
            observer.Callback.ExecuteIfBound( objective->GetClass(), was_cancelled );
            MissionObjectiveEndObservers.RemoveAt( index );
        }
    }
}