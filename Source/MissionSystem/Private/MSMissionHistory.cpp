#include "MSMissionHistory.h"

#include "MSMissionData.h"

namespace
{
    template < typename _ObjectType_ >
    FGuid GetObjectGuid( _ObjectType_ object );

    template <>
    FGuid GetObjectGuid( UMSMissionData * object )
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
    TOptional< EMSState > GetObjectState( _ObjectType_ object, const TMap< FGuid, EMSState > & object_map )
    {
        if ( object == nullptr )
        {
            return {};
        }

        const auto guid = GetObjectGuid( object );

        if ( !ensureAlways( guid.IsValid() ) )
        {
            return {};
        }

        if ( auto * state = object_map.Find( guid ) )
        {
            return *state;
        }

        return {};
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

        if ( auto * state = object_map.Find( id ) )
        {
            if ( *state == EMSState::Active )
            {
                return true;
            }

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

bool FMSMissionHistory::IsMissionActive( UMSMissionData * mission_data ) const
{
    return DoesMissionHasState( mission_data, EMSState::Active );
}

bool FMSMissionHistory::IsMissionCancelled( UMSMissionData * mission_data ) const
{
    return DoesMissionHasState( mission_data, EMSState::Cancelled );
}

bool FMSMissionHistory::IsMissionComplete( UMSMissionData * mission_data ) const
{
    return DoesMissionHasState( mission_data, EMSState::Complete );
}

bool FMSMissionHistory::IsMissionFinished( UMSMissionData * mission_data ) const
{
    const auto state = GetObjectState( mission_data, MissionStates ).Get( EMSState::Active );
    return state > EMSState::Active;
}

bool FMSMissionHistory::AddActiveMission( UMSMissionData * mission_data )
{
    if ( !TryAddObjectToMap( mission_data, MissionStates ) )
    {
        return false;
    }

    check( !ActiveMissionsData.Contains( mission_data ) );
    ActiveMissionsData.Add( mission_data );

    return true;
}

bool FMSMissionHistory::SetMissionComplete( UMSMissionData * mission_data, bool was_cancelled )
{
    if ( !SetObjectComplete( mission_data, MissionStates, was_cancelled ) )
    {
        return false;
    }

    check( ActiveMissionsData.Contains( mission_data ) );
    ActiveMissionsData.Remove( mission_data );

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

bool FMSMissionHistory::IsObjectiveFinished( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const
{
    const auto state = GetObjectState( mission_objective_class, ObjectiveStates ).Get( EMSState::Active );
    return state > EMSState::Active;
}

bool FMSMissionHistory::AddActiveObjective( const TSubclassOf< UMSMissionObjective > & mission_objective_class )
{
    return TryAddObjectToMap( mission_objective_class, ObjectiveStates );
}

bool FMSMissionHistory::SetObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class, bool was_cancelled )
{
    return SetObjectComplete( mission_objective_class, ObjectiveStates, was_cancelled );
}

bool FMSMissionHistory::DoesMissionHasState( UMSMissionData * mission_data, EMSState state ) const
{
    return DoesObjectHasState( mission_data, MissionStates, state );
}

bool FMSMissionHistory::DoesObjectiveHasState( const TSubclassOf< UMSMissionObjective > & mission_objective_class, EMSState state ) const
{
    return DoesObjectHasState( mission_objective_class, ObjectiveStates, state );
}

FArchive & operator<<( FArchive & archive, FMSMissionHistory & mission_history )
{
    /*if ( archive.IsSaving() )
    {
        int num = mission_history.ActiveMissionsData.Num();
        archive << num;

        for ( int i = 0; i < num; ++i )
        {
            archive << mission_history.ActiveMissionsData[ i ];
        }
    }*/

    archive << mission_history.ActiveMissionsData;
    archive << mission_history.MissionStates;
    archive << mission_history.ObjectiveStates;

    return archive;
}