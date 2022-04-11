#include "MSMissionSystem.h"

#include "Log/CoreExtLog.h"
#include "MSLog.h"
#include "MSMission.h"

#include <Engine/World.h>

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
static FAutoConsoleCommand SkipMissionCommand(
    TEXT( "MissionSystem.SkipMission" ),
    TEXT( "Skips the current missions." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        if ( const auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->CancelCurrentMissions();
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

bool UMSMissionSystem::MustObjectiveBeIgnored( UMSMissionObjective * objective ) const
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
