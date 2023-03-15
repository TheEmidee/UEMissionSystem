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

    for ( const auto * mission_to_cancel : mission_data->MissionsToCancel )
    {
        if ( const auto * active_mission_to_cancel_ptr = ActiveMissions.Find( mission_to_cancel ) )
        {
            ( *active_mission_to_cancel_ptr )->Cancel();
        }
    }

    if ( !mission_data->bEnabled )
    {
        StartNextMissions( mission_data );
        return;
    }

    auto * mission = NewObject< UMSMission >( this );
    mission->Initialize( mission_data );

    mission->OnMissionEnded().AddUObject( this, &UMSMissionSystem::OnMissionEnded );
    mission->OnMissionObjectiveStarted().AddUObject( this, &UMSMissionSystem::OnMissionObjectiveStarted );
    mission->OnMissionObjectiveEnded().AddUObject( this, &UMSMissionSystem::OnMissionObjectiveEnded );

    ActiveMissions.Add( mission_data, mission );

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

bool UMSMissionSystem::IsMissionObjectiveActive( TSubclassOf< UMSMissionObjective > mission_objective_class ) const
{
    if ( !ensureAlwaysMsgf( mission_objective_class != nullptr, TEXT( "Mission objective class must be valid" ) ) )
    {
        return false;
    }

    for ( auto & [ mission_data, active_mission ] : ActiveMissions )
    {
        for ( const auto * objective : active_mission->GetObjectives() )
        {
            if ( objective->GetClass() == mission_objective_class )
            {
                return objective->IsComplete();
            }
        }
    }

    for ( auto * completed_mission : CompletedMissions )
    {
        for ( const auto & objective_data : completed_mission->Objectives )
        {
            if ( objective_data.bEnabled && objective_data.Objective == mission_objective_class )
            {
                return true;
            }
        }
    }

    return false;
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
    FMissionEndObserver observer;
    observer.MissionData = mission_data;
    observer.Callback = when_mission_ends;

    MissionEndObservers.Emplace( MoveTemp( observer ) );
}

void UMSMissionSystem::WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, const FMSMissionSystemMissionObjectiveStartsDelegate & when_mission_objective_starts )
{
    if ( IsMissionObjectiveActive( mission_objective ) )
    {
        when_mission_objective_starts.ExecuteIfBound( mission_objective );
        return;
    }

    FMissionObjectiveStartObserver observer;
    observer.MissionObjective = mission_objective;
    observer.Callback = when_mission_objective_starts;

    MissionObjectiveStartObservers.Emplace( MoveTemp( observer ) );
}

void UMSMissionSystem::WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, const FMSMissionSystemMissionObjectiveEndsDelegate & when_mission_objective_ends )
{
    FMissionObjectiveEndObserver observer;
    observer.MissionObjective = mission_objective;
    observer.Callback = when_mission_objective_ends;

    MissionObjectiveEndObservers.Emplace( MoveTemp( observer ) );
}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
void UMSMissionSystem::DumpActiveMissions( FOutputDevice & output_device )
{
    output_device.Logf( ELogVerbosity::Verbose, TEXT( "Mission System - Active Missions :" ) );
    for ( const auto & key_pair : ActiveMissions )
    {
        key_pair.Value->DumpMission( output_device );
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

    OnMissionEndedEvent.Broadcast( mission_data, was_cancelled );
    ActiveMissions.Remove( mission_data );

    if ( !was_cancelled )
    {
        CompletedMissions.Add( mission_data );
    }

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

void UMSMissionSystem::OnMissionObjectiveStarted( UMSMissionObjective * objective )
{
    for ( auto & observer : MissionObjectiveStartObservers )
    {
        observer.Callback.ExecuteIfBound( objective->GetClass() );
    }
}

void UMSMissionSystem::OnMissionObjectiveEnded( UMSMissionObjective * objective, const bool was_cancelled )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionObjectiveEnded (%s)" ), *objective->GetClass()->GetName() );

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