#include "MSMissionSystem.h"

#include "Log/CoreExtLog.h"
#include "MSLog.h"
#include "MSMission.h"

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
    if ( mission_data == nullptr )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with a null mission data" ) );
        return;
    }

    if ( ActiveMissions.Contains( mission_data ) )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with an already active mission (%s)" ), *GetNameSafe( mission_data ) );
        return;
    }

    if ( CompletedMissions.Contains( mission_data ) )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with an already completed mission (%s)" ), *GetNameSafe( mission_data ) );
        return;
    }

    if ( !mission_data->bEnabled )
    {
        StartNextMissions( mission_data );
        return;
    }

    auto * mission = NewObject< UMSMission >( this );
    mission->Initialize( mission_data );

    mission->OnMissionEnded().AddDynamic( this, &UMSMissionSystem::OnMissionEnded );
    mission->OnMissionObjectiveEnded().AddDynamic( this, &UMSMissionSystem::OnMissionObjectiveEnded );

    ActiveMissions.Add( mission_data, mission );

    UE_SLOG( LogMissionSystem, Verbose, TEXT( "Start mission (%s)" ), *GetNameSafe( mission_data ) );
    mission->Start();

    for ( auto & observer : MissionStartObservers )
    {
        observer.Callback.ExecuteIfBound( mission_data );
    }
}

void UMSMissionSystem::StartMissionWithEndDelegate( UMSMissionData * mission_data, FMSMissionSystemMissionEndsDynamicDelegate when_mission_ends )
{
    StartMission( mission_data );
    K2_WhenMissionEnds( mission_data, when_mission_ends );
}

bool UMSMissionSystem::IsMissionComplete( UMSMissionData * mission_data ) const
{
    if ( /*ensureAlways*/ ( mission_data != nullptr ) )
    {
        return CompletedMissions.Contains( mission_data );
    }

    return false;
}

bool UMSMissionSystem::IsMissionActive( UMSMissionData * mission_data ) const
{
    return GetActiveMission( mission_data ) != nullptr;
}

UMSMission * UMSMissionSystem::GetActiveMission( UMSMissionData * mission_data ) const
{
    if ( auto * result = ActiveMissions.Find( mission_data ) )
    {
        return *result;
    }

    return nullptr;
}

void UMSMissionSystem::CancelCurrentMissions() const
{
    TArray< UMSMission * > result;
    ActiveMissions.GenerateValueArray( result );

    for ( auto * mission : result )
    {
        mission->Cancel();
    }
}

void UMSMissionSystem::CompleteCurrentMissions() const
{
    TArray< UMSMission * > result;
    ActiveMissions.GenerateValueArray( result );

    for ( auto * mission : result )
    {
        mission->Complete();
    }
}

void UMSMissionSystem::WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartsDelegate & when_mission_starts )
{
    FMissionStartObserver observer;
    observer.MissionData = mission_data;
    observer.Callback = when_mission_starts;

    MissionStartObservers.Emplace( MoveTemp( observer ) );

    if ( IsMissionActive( mission_data ) )
    {
        when_mission_starts.ExecuteIfBound( mission_data );
    }
}

void UMSMissionSystem::WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndsDelegate & when_mission_ends )
{
    FMissionEndObserver observer;
    observer.MissionData = mission_data;
    observer.Callback = when_mission_ends;

    MissionEndObservers.Emplace( MoveTemp( observer ) );
}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
void UMSMissionSystem::DumpActiveMissions( FOutputDevice & output_device )
{
    output_device.Logf( ELogVerbosity::Verbose, TEXT( "Mission System - Active Missions :" ) );
    for ( const auto & key_pair : ActiveMissions )
    {
        output_device.Logf( ELogVerbosity::Verbose, TEXT( " * Mission : %s" ), *GetNameSafe( key_pair.Key ) );

        key_pair.Value->DumpObjectives( output_device );
    }
}

void UMSMissionSystem::IgnoreObjectivesWithTags( const TArray< FString > & tags )
{
    for ( const auto & tag : tags )
    {
        TagsToIgnoreForObjectives.AddUnique( tag );
    }

    for ( const auto & key_pair : ActiveMissions )
    {
        for ( auto * objective : key_pair.Value->GetObjectives() )
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

    auto * world = Cast< UWorld >( outer );

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

void UMSMissionSystem::StartNextMissions( UMSMissionData * mission_data )
{
    for ( auto * next_mission : mission_data->NextMissions )
    {
        StartMission( next_mission );
    }
}

void UMSMissionSystem::OnMissionEnded( UMSMissionData * mission_data, const bool was_cancelled )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionEnded (%s)" ), *GetNameSafe( mission_data ) );

    OnMissionCompleteDelegate.Broadcast( mission_data, was_cancelled );
    ActiveMissions.Remove( mission_data );

    if ( !was_cancelled )
    {
        CompletedMissions.Add( mission_data );
    }

    for ( auto & observer : MissionEndObservers )
    {
        observer.Callback.ExecuteIfBound( mission_data, was_cancelled );
    }

    if ( !was_cancelled || mission_data->bStartNextMissionsWhenCancelled )
    {
        StartNextMissions( mission_data );
    }
}

void UMSMissionSystem::OnMissionObjectiveEnded( UMSMissionObjective * objective, const bool was_cancelled )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionObjectiveEnded (%s)" ), *objective->GetClass()->GetName() );

    OnMissionObjectiveCompleteDelegate.Broadcast( objective, was_cancelled );
}
