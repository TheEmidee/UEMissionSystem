#include "MSMissionSystem.h"

#include "Log/CoreExtLog.h"
#include "MSLog.h"
#include "MSMission.h"

#include <Engine/World.h>

static FAutoConsoleCommand SkipMissionCommand(
    TEXT( "ms.SkipMission" ),
    TEXT( "A custom command that skips the current mission." ),
    FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateLambda( []( const TArray< FString > & args, UWorld * world, FOutputDevice & output_device ) {
        if ( auto * mission_system = world->GetSubsystem< UMSMissionSystem >() )
        {
            mission_system->SkipCurrentMissions();
        }
    } ) );

UMSMission * UMSMissionSystem::StartMission( UMSMissionData * mission_data )
{
    if ( mission_data == nullptr )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with a null mission data" ) );
        return nullptr;
    }

    if ( ActiveMissions.Contains( mission_data ) )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with an already active mission (%s)" ), *GetNameSafe( mission_data ) );
        return nullptr;
    }

    if ( CompletedMissions.Contains( mission_data ) )
    {
        UE_SLOG( LogMissionSystem, Warning, TEXT( "StartMission called with an already completed mission (%s)" ), *GetNameSafe( mission_data ) );
        return nullptr;
    }

    auto * mission = NewObject< UMSMission >( this );
    mission->Initialize( mission_data );

    mission->OnMissionComplete().AddDynamic( this, &UMSMissionSystem::OnMissionComplete );
    mission->OnMissionObjectiveComplete().AddDynamic( this, &UMSMissionSystem::OnMissionObjectiveComplete );

    ActiveMissions.Add( mission_data, mission );

    UE_SLOG( LogMissionSystem, Verbose, TEXT( "Start mission (%s)" ), *GetNameSafe( mission_data ) );
    mission->Start();

    return mission;
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

void UMSMissionSystem::SkipCurrentMissions() const
{
    TArray< UMSMission * > result;
    ActiveMissions.GenerateValueArray( result );

    for ( auto * mission : result )
    {
        mission->End();
    }
}

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

void UMSMissionSystem::OnMissionComplete( UMSMissionData * mission_data )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionComplete (%s)" ), *GetNameSafe( mission_data ) );

    OnMissionCompleteDelegate.Broadcast( mission_data );
    ActiveMissions.Remove( mission_data );
    CompletedMissions.Add( mission_data );

    for ( auto * next_mission : mission_data->NextMissions )
    {
        StartMission( next_mission );
    }
}

void UMSMissionSystem::OnMissionObjectiveComplete( UMSMissionObjective * objective )
{
    UE_SLOG( LogMissionSystem, Verbose, TEXT( "OnMissionObjectiveComplete (%s)" ), *objective->GetClass()->GetName() );

    OnMissionObjectiveCompleteDelegate.Broadcast( objective );
}
