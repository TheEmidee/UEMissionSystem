#include "ViewModels/MSViewModel.h"

#include "MSMission.h"
#include "MSMissionData.h"
#include "ViewModels/MSMissionViewModel.h"

void UMSViewModel::RemoveCompletedMission( UMSMissionViewModel * mission_vm )
{
    CompletedMissions.Remove( mission_vm );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedMissions );
}

void UMSViewModel::SetMissionStarted( UMSMission * mission )
{
    if ( GetMissionViewModel( mission ) != nullptr )
    {
        return;
    }

    if ( !mission->GetMissionData()->bHideOnVM )
    {
        auto * mission_vm = NewObject< UMSMissionViewModel >( this );
        mission_vm->Initialize( mission );
        ActiveMissions.Add( mission_vm );
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveMissions );
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( HasActiveMissions );
        mission_vm->OnObjectivesChanged.BindUObject( this, &ThisClass::BroadCastOnMissionsObjectivesChanged );
    }
}

void UMSViewModel::SetMissionEnded( UMSMission * mission )
{
    const auto predicate = [ & ]( auto mission_vm ) {
        return mission_vm->GetMission() == mission;
    };

    if ( auto * mission_vm = ActiveMissions.FindByPredicate( predicate ) )
    {
        CompletedMissions.AddUnique( *mission_vm );

        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedMissions );
    }

    ActiveMissions.RemoveAll( predicate );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveMissions );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( HasActiveMissions );
}

void UMSViewModel::SetMissionObjectiveStarted( const UMSMission * mission, UMSMissionObjective * objective ) const
{
    if ( auto * mission_vm = GetMissionViewModel( mission ) )
    {
        mission_vm->SetObjectiveStarted( objective );
    }
}

void UMSViewModel::RefreshMissionObjectiveProgression( const UMSMission * mission, const UMSMissionObjective * objective ) const
{
    if ( auto * mission_vm = GetMissionViewModel( mission ) )
    {
        mission_vm->RefreshObjectiveProgression( objective );
    }
}

void UMSViewModel::SetMissionObjectiveEnded( const UMSMission * mission, UMSMissionObjective * objective ) const
{
    if ( auto * mission_vm = GetMissionViewModel( mission ) )
    {
        mission_vm->SetObjectiveEnded( objective );
    }
}

bool UMSViewModel::HasActiveMissions() const
{
    return !ActiveMissions.IsEmpty();
}

UMSMissionViewModel * UMSViewModel::GetMissionViewModel( const UMSMission * mission ) const
{
    if ( auto * vm_ptr = ActiveMissions.FindByPredicate( [ & ]( auto mission_vm ) {
             return mission_vm->GetMission() == mission;
         } ) )
    {
        return *vm_ptr;
    }

    return nullptr;
}

void UMSViewModel::BroadCastOnMissionsObjectivesChanged()
{
    OnMissionsObjectivesChanged.ExecuteIfBound();
}