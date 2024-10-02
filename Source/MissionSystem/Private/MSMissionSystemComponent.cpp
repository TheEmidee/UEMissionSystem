#include "MSMissionSystemComponent.h"

#include "Log/CoreExtLog.h"
#include "MSLog.h"
#include "MSMission.h"
#include "MVVMGameSubsystem.h"
#include "ViewModels/MSViewModel.h"

#include <Engine/GameInstance.h>
#include <Engine/World.h>
#include <Serialization/MemoryWriter.h>

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
static FAutoConsoleCommand SkipMissionsCommand(
    TEXT( "MissionSystem.SkipMissions" ),
    TEXT( "Skips the current missions." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        // :TODO:
        /*if ( const auto * mission_system = world->GetSubsystem< UMSMissionSystemComponent >() )
        {
            mission_system->CancelCurrentMissions();
        }*/
    } ) );

static FAutoConsoleCommand CompleteMissionsCommand(
    TEXT( "MissionSystem.CompleteMissions" ),
    TEXT( "Completes the current missions." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        // :TODO:
        /*if ( const auto * mission_system = world->GetSubsystem< UMSMissionSystemComponent >() )
        {
            mission_system->CompleteCurrentMissions();
        }*/
    } ) );

static FAutoConsoleCommand ListActiveMissionsCommand(
    TEXT( "MissionSystem.ListActiveMissions" ),
    TEXT( "Prints the active missions in the log." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & output_device ) {
        // :TODO:
        /*if ( auto * mission_system = world->GetSubsystem< UMSMissionSystemComponent >() )
        {
            mission_system->DumpActiveMissions( output_device );
        }*/
    } ) );

static FAutoConsoleCommand IgnoreObjectivesWithTag(
    TEXT( "MissionSystem.IgnoreObjectivesWithTag" ),
    TEXT( "Don't start objectives that contain this tag." )
        TEXT( "Can be used multiple times." )
            TEXT( "Objectives already started that match the tags will be completed." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & args, const UWorld * world, FOutputDevice & output_device ) {
        // :TODO:
        /*if ( auto * mission_system = world->GetSubsystem< UMSMissionSystemComponent >() )
        {
            mission_system->IgnoreObjectivesWithTags( args );
        }*/
    } ) );

static FAutoConsoleCommand ClearIgnoreObjectivesTags(
    TEXT( "MissionSystem.ClearIgnoreObjectivesTag" ),
    TEXT( "Clears the list of tags used to ignore objectives." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & /*args*/, const UWorld * world, FOutputDevice & /*output_device*/ ) {
        // :TODO:
        /*if ( auto * mission_system = world->GetSubsystem< UMSMissionSystemComponent >() )
        {
            mission_system->ClearIgnoreObjectivesTags();
        }*/
    } ) );
#endif

UMSMissionSystemComponent::UMSMissionSystemComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    bCreateViewModel( false ),
    bRegisterViewModel( true ),
    ViewModelContextName( TEXT( "MSViewModel" ) ),
    bTryResumeMissionFromHistory( true )
{
}

void UMSMissionSystemComponent::BeginPlay()
{
    Super::BeginPlay();

    if ( bTryResumeMissionFromHistory )
    {
        if ( HasDataInHistory() )
        {
            ResumeMissionsFromHistory();
        }
        else if ( FirstMissionToStart != nullptr )
        {
            StartMission( FirstMissionToStart );
        }
    }
}

bool UMSMissionSystemComponent::HasDataInHistory() const
{
    return MissionHistory.HasData();
}

void UMSMissionSystemComponent::StartMission( UMSMissionData * mission_data )
{
    auto * mission = TryCreateMissionFromData( mission_data );

    if ( mission == nullptr )
    {
        return;
    }

    StartMission( mission );
}

bool UMSMissionSystemComponent::IsMissionComplete( UMSMissionData * mission_data ) const
{
    return MissionHistory.IsMissionComplete( mission_data );
}

bool UMSMissionSystemComponent::IsMissionActive( UMSMissionData * mission_data ) const
{
    return MissionHistory.IsMissionActive( mission_data );
}

UMSMission * UMSMissionSystemComponent::GetActiveMission( const UMSMissionData * mission_data ) const
{
    if ( auto * mission = ActiveMissions.FindByPredicate( [ & ]( const auto * active_mission ) {
             return active_mission->GetMissionData() == mission_data;
         } ) )
    {
        return *mission;
    }

    return nullptr;
}

void UMSMissionSystemComponent::CancelCurrentMissions() const
{
    for ( auto * mission : ActiveMissions )
    {
        mission->Cancel();
    }
}

void UMSMissionSystemComponent::CompleteCurrentMissions() const
{
    for ( auto * mission : ActiveMissions )
    {
        mission->Complete();
    }
}

bool UMSMissionSystemComponent::IsMissionObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const
{
    return MissionHistory.IsObjectiveActive( mission_objective_class );
}

void UMSMissionSystemComponent::ResumeMissionsFromHistory()
{
    for ( const auto mission_data : MissionHistory.GetActiveMissionData() )
    {
        // :NOTE: Bypass the checks of TryCreateMissionFromData
        auto * mission = CreateMissionFromData( mission_data );

        if ( mission == nullptr )
        {
            continue;
        }

        StartMission( mission );
    }
}

bool UMSMissionSystemComponent::CompleteObjective( UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective_class )
{
    if ( auto * mission = GetActiveMission( mission_data ) )
    {
        return mission->CompleteObjective( mission_objective_class );
    }

    return false;
}

void UMSMissionSystemComponent::WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartedDelegate & when_mission_starts )
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

void UMSMissionSystemComponent::WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndedDelegate & when_mission_ends )
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

void UMSMissionSystemComponent::WhenMissionObjectiveStartsOrIsActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveStartedDelegate & when_mission_objective_starts )
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

void UMSMissionSystemComponent::WhenMissionObjectiveEnds( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveEndedDelegate & when_mission_objective_ends )
{
    FMissionObjectiveEndObserver observer;
    observer.MissionObjective = mission_objective_class;
    observer.Callback = when_mission_objective_ends;

    MissionObjectiveEndObservers.Emplace( MoveTemp( observer ) );
}

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
void UMSMissionSystemComponent::DumpActiveMissions( FOutputDevice & output_device )
{
    output_device.Logf( ELogVerbosity::Verbose, TEXT( "Mission System - Active Missions :" ) );
    for ( auto * active_mission : ActiveMissions )
    {
        active_mission->DumpMission( output_device );
    }
}

void UMSMissionSystemComponent::IgnoreObjectivesWithTags( const TArray< FString > & tags )
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

void UMSMissionSystemComponent::ClearIgnoreObjectivesTags()
{
    TagsToIgnoreForObjectives.Reset();
}

bool UMSMissionSystemComponent::MustObjectiveBeIgnored( const UMSMissionObjective * objective ) const
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

void UMSMissionSystemComponent::Serialize( FArchive & archive )
{
    Super::Serialize( archive );

    archive << MissionHistory;
}

void UMSMissionSystemComponent::OnRegister()
{
    Super::OnRegister();

    if ( bCreateViewModel )
    {
        ViewModel = NewObject< UMSViewModel >( this );

        if ( bRegisterViewModel )
        {
            if ( auto * system = GetWorld()->GetGameInstance()->GetSubsystem< UMVVMGameSubsystem >() )
            {
                FMVVMViewModelContext context;
                context.ContextClass = UMSViewModel::StaticClass();
                context.ContextName = ViewModelContextName;

                system->GetViewModelCollection()->AddViewModelInstance( context, ViewModel );
            }
        }
    }
}

void UMSMissionSystemComponent::K2_WhenMissionStartsOrIsActive( UMSMissionData * mission_data, FMSMissionSystemMissionStartedDynamicDelegate when_mission_starts )
{
    const auto active_delegate = FMSMissionSystemMissionStartedDelegate::CreateWeakLambda( when_mission_starts.GetUObject(), [ when_mission_starts ]( const UMSMissionData * mission_data ) {
        when_mission_starts.ExecuteIfBound( mission_data );
    } );

    WhenMissionStartsOrIsActive( mission_data, active_delegate );
}

void UMSMissionSystemComponent::K2_WhenMissionEnds( UMSMissionData * mission_data, FMSMissionSystemMissionEndedDynamicDelegate when_mission_ends )
{
    const auto ended_delegate = FMSMissionSystemMissionEndedDelegate::CreateWeakLambda( when_mission_ends.GetUObject(), [ when_mission_ends ]( const UMSMissionData * mission_data, const bool was_cancelled ) {
        when_mission_ends.ExecuteIfBound( mission_data, was_cancelled );
    } );

    WhenMissionEnds( mission_data, ended_delegate );
}

void UMSMissionSystemComponent::K2_WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveStartedDynamicDelegate when_mission_objective_starts )
{
    const auto active_delegate = FMSMissionSystemMissionObjectiveStartedDelegate::CreateWeakLambda( when_mission_objective_starts.GetUObject(), [ when_mission_objective_starts ]( TSubclassOf< UMSMissionObjective > mission_objective ) {
        when_mission_objective_starts.ExecuteIfBound( mission_objective );
    } );

    WhenMissionObjectiveStartsOrIsActive( mission_objective, active_delegate );
}

void UMSMissionSystemComponent::K2_WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveEndedDynamicDelegate when_mission_objective_ends )
{
    const auto ended_delegate = FMSMissionSystemMissionObjectiveEndedDelegate::CreateWeakLambda( when_mission_objective_ends.GetUObject(), [ when_mission_objective_ends ]( TSubclassOf< UMSMissionObjective > mission_objective, const bool was_cancelled ) {
        when_mission_objective_ends.ExecuteIfBound( mission_objective, was_cancelled );
    } );

    WhenMissionObjectiveEnds( mission_objective, ended_delegate );
}

UMSMission * UMSMissionSystemComponent::TryCreateMissionFromData( UMSMissionData * mission_data )
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

    return CreateMissionFromData( mission_data );
}

UMSMission * UMSMissionSystemComponent::CreateMissionFromData( UMSMissionData * mission_data )
{
    auto * mission = NewObject< UMSMission >( this );
    mission->Initialize( mission_data );

    mission->OnMissionEnded().AddUObject( this, &UMSMissionSystemComponent::OnMissionEnded );
    mission->OnMissionObjectiveStarted().AddUObject( this, &UMSMissionSystemComponent::OnMissionObjectiveStarted, mission );
    mission->OnMissionObjectiveEnded().AddUObject( this, &UMSMissionSystemComponent::OnMissionObjectiveEnded, mission );

    ActiveMissions.Add( mission );

    return mission;
}

void UMSMissionSystemComponent::StartMission( UMSMission * mission )
{
    auto * mission_data = mission->GetMissionData();

    UE_SLOG( LogMissionSystem, Verbose, TEXT( "Start mission (%s)" ), *GetNameSafe( mission_data ) );

    // :NOTE: This is intended to broadcast now before actually starting the mission
    // This is to make sure no objectives have been started yet, and that any object listening to the
    // events mission started / objective started receive them in the correct order
    BroadcastOnMissionStarted( mission );

    mission->Start();
}

void UMSMissionSystemComponent::StartNextMissions( const UMSMissionData * mission_data )
{
    for ( auto * next_mission : mission_data->NextMissions )
    {
        StartMission( next_mission );
    }
}

void UMSMissionSystemComponent::OnMissionEnded( UMSMission * mission, const bool was_cancelled )
{
    auto * mission_data = mission->GetMissionData();

    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionEnded (%s)" ), *GetNameSafe( mission_data ) );

    if ( !ensureAlways( MissionHistory.SetMissionComplete( mission_data, was_cancelled ) ) )
    {
        return;
    }

    ActiveMissions.Remove( mission );

    BroadcastOnMissionEnded( mission, was_cancelled );

    if ( ViewModel != nullptr )
    {
        ViewModel->SetMissionEnded( mission );
    }

    if ( !was_cancelled || mission_data->bStartNextMissionsWhenCancelled )
    {
        StartNextMissions( mission_data );
    }
}

void UMSMissionSystemComponent::OnMissionObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective, UMSMission * mission )
{
    if ( !ensureAlways( MissionHistory.AddActiveObjective( objective ) ) )
    {
        return;
    }

    BroadcastOnMissionObjectiveStarted( mission, objective );
}

void UMSMissionSystemComponent::OnMissionObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective, const bool was_cancelled, UMSMission * mission )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnObjectiveEnded (%s)" ), *objective->GetClass()->GetName() );

    if ( !ensureAlways( MissionHistory.SetObjectiveComplete( objective, was_cancelled ) ) )
    {
        return;
    }

    BroadcastOnMissionObjectiveEnded( mission, objective, was_cancelled );
}

void UMSMissionSystemComponent::BroadcastOnMissionStarted( UMSMission * mission )
{
    const auto * mission_data = mission->GetMissionData();

    OnMissionStartedDelegate.Broadcast( mission );

    for ( auto index = MissionStartObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = MissionStartObservers[ index ];

        if ( observer.MissionData == mission_data )
        {
            observer.Callback.ExecuteIfBound( mission_data );
            MissionStartObservers.RemoveAt( index );
        }
    }

    if ( ViewModel != nullptr )
    {
        ViewModel->SetMissionStarted( mission );
    }
}

void UMSMissionSystemComponent::BroadcastOnMissionEnded( UMSMission * mission, bool was_cancelled )
{
    auto * mission_data = mission->GetMissionData();

    OnMissionEndedDelegate.Broadcast( mission_data, was_cancelled );

    for ( auto index = MissionEndObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = MissionEndObservers[ index ];

        if ( observer.MissionData == mission_data )
        {
            observer.Callback.ExecuteIfBound( mission_data, was_cancelled );
            MissionEndObservers.RemoveAt( index );
        }
    }

    if ( ViewModel != nullptr )
    {
        ViewModel->SetMissionEnded( mission );
    }
}

void UMSMissionSystemComponent::BroadcastOnMissionObjectiveStarted( UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective )
{
    OnMissionObjectiveStartedDelegate.Broadcast( mission->GetMissionData(), objective );

    for ( auto & observer : MissionObjectiveStartObservers )
    {
        observer.Callback.ExecuteIfBound( objective->GetClass() );
    }

    if ( ViewModel != nullptr )
    {
        ViewModel->SetMissionObjectiveStarted( mission, objective );
    }
}

void UMSMissionSystemComponent::BroadcastOnMissionObjectiveEnded( UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective, bool was_cancelled )
{
    OnMissionObjectiveEndedDelegate.Broadcast( mission->GetMissionData(), objective, was_cancelled );

    for ( auto index = MissionObjectiveEndObservers.Num() - 1; index >= 0; --index )
    {
        auto & observer = MissionObjectiveEndObservers[ index ];

        if ( observer.MissionObjective == objective->GetClass() )
        {
            observer.Callback.ExecuteIfBound( objective->GetClass(), was_cancelled );
            MissionObjectiveEndObservers.RemoveAt( index );
        }
    }

    if ( ViewModel != nullptr )
    {
        ViewModel->SetMissionObjectiveEnded( mission, objective );
    }
}