#include "MSMissionHistory.h"

#include "MSMissionData.h"

bool FMSMissionHistory::IsMissionActive( const UMSMissionData * mission_data ) const
{
    return DoesMissionHasState( mission_data, EMSState::Active );
}

bool FMSMissionHistory::IsMissionCancelled( const UMSMissionData * mission_data ) const
{
    return DoesMissionHasState( mission_data, EMSState::Cancelled );
}

bool FMSMissionHistory::IsMissionComplete( const UMSMissionData * mission_data ) const
{
    return DoesMissionHasState( mission_data, EMSState::Complete );
}

bool FMSMissionHistory::AddActiveMission( const UMSMissionData * mission_data )
{
    if ( !ensureAlways( mission_data != nullptr ) )
    {
        return false;
    }

    const auto id = mission_data->GetMissionId();

    if ( !ensureAlways( id.IsValid() ) )
    {
        return false;
    }

    if ( auto * found_id = MissionStates.Find( id ) )
    {
        return false;
    }

    MissionStates.Add( id, EMSState::Active );
    return true;
}

bool FMSMissionHistory::IsObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const
{
    return DoesObjectiveHasState( mission_objective_class, EMSState::Active );
}

bool FMSMissionHistory::IsObjectiveCancelled( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const
{
    return DoesObjectiveHasState( mission_objective_class, EMSState::Cancelled );
}

bool FMSMissionHistory::IsObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const
{
    return DoesObjectiveHasState( mission_objective_class, EMSState::Complete );
}

bool FMSMissionHistory::DoesMissionHasState( const UMSMissionData * mission_data, EMSState state ) const
{
    if ( mission_data == nullptr )
    {
        return false;
    }

    if ( !ensureAlways( mission_data->GetMissionId().IsValid() ) )
    {
        return false;
    }

    if ( auto * mission_state = MissionStates.Find( mission_data->GetMissionId() ) )
    {
        return *mission_state == state;
    }

    return false;
}

bool FMSMissionHistory::DoesObjectiveHasState( const TSubclassOf< UMSMissionObjective > & mission_objective_class, EMSState state ) const
{
    if ( mission_objective_class == nullptr )
    {
        return false;
    }

    auto * cdo = mission_objective_class.GetDefaultObject();
    const auto objective_id = cdo->GetObjectiveId();

    if ( !ensureAlways( objective_id.IsValid() ) )
    {
        return false;
    }

    if ( auto * objective_state = ObjectiveStates.Find( objective_id ) )
    {
        return *objective_state == state;
    }

    return false;
}