#include "MSMissionHistory.h"

#include "MSMissionData.h"

namespace
{
    template < typename _ObjectType_ >
    FGuid GetObjectGuid( _ObjectType_ object )
    {
        return FGuid();
    }

    template <>
    FGuid GetObjectGuid( const UMSMissionData * object )
    {
        return object->GetGuid();
    }

    template <>
    FGuid GetObjectGuid( TSubclassOf< UMSMissionObjective > object )
    {
        auto * cdo = object.GetDefaultObject();
        return cdo->GetGuid();
    }

    template < typename _ObjectType_ >
    bool DoesObjectHasState( _ObjectType_ object, const TMap< FGuid, EMSState > & object_map, EMSState required_state )
    {
        if ( object == nullptr )
        {
            return false;
        }

        const auto guid = GetObjectGuid( object );

        if ( !ensureAlways( guid.IsValid() ) )
        {
            return false;
        }

        if ( auto * state = object_map.Find( guid ) )
        {
            return *state == required_state;
        }

        return false;
    }

    template < typename _ObjectType_ >
    bool TryAddObjectToMap( _ObjectType_ object, TMap< FGuid, EMSState > & object_map )
    {
        if ( !ensureAlways( object != nullptr ) )
        {
            return false;
        }

        const auto id = GetObjectGuid( object );

        if ( !ensureAlways( id.IsValid() ) )
        {
            return false;
        }

        if ( object_map.Contains( id ) )
        {
            return false;
        }

        object_map.Add( id, EMSState::Active );

        return true;
    }

    template < typename _ObjectType_ >
    bool SetObjectComplete( _ObjectType_ object, TMap< FGuid, EMSState > & object_map, const bool was_cancelled )
    {
        if ( !ensureAlways( object != nullptr ) )
        {
            return false;
        }

        const auto id = GetObjectGuid( object );

        if ( !ensureAlways( id.IsValid() ) )
        {
            return false;
        }

        if ( auto * found_id = object_map.Find( id ) )
        {
            *found_id = was_cancelled ? EMSState::Cancelled : EMSState::Complete;
            return true;
        }

        return false;
    }
}

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
    return TryAddObjectToMap( mission_data, MissionStates );
}

bool FMSMissionHistory::SetMissionComplete( const UMSMissionData * mission_data, bool was_cancelled )
{
    return SetObjectComplete( mission_data, MissionStates, was_cancelled );
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

bool FMSMissionHistory::AddActiveObjective( const TSubclassOf< UMSMissionObjective > & mission_objective_class )
{
    return TryAddObjectToMap( mission_objective_class, ObjectiveStates );
}

bool FMSMissionHistory::SetObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class, bool was_cancelled )
{
    return SetObjectComplete( mission_objective_class, ObjectiveStates, was_cancelled );
}

bool FMSMissionHistory::DoesMissionHasState( const UMSMissionData * mission_data, EMSState state ) const
{
    return DoesObjectHasState( mission_data, MissionStates, state );
}

bool FMSMissionHistory::DoesObjectiveHasState( const TSubclassOf< UMSMissionObjective > & mission_objective_class, EMSState state ) const
{
    return DoesObjectHasState( mission_objective_class, ObjectiveStates, state );
}