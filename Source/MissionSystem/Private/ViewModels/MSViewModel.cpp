#include "ViewModels/MSViewModel.h"

#include "ViewModels/MSMissionViewModel.h"

void UMSViewModel::SetMissionStarted( UMSMission * mission )
{
    if ( GetMissionViewModel( mission ) != nullptr )
    {
        return;
    }

    auto * mission_vm = NewObject< UMSMissionViewModel >( this );
    mission_vm->Initialize( mission );
    ActiveMissions.Add( mission_vm );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveMissions );
}

void UMSViewModel::SetMissionEnded( UMSMission * mission )
{
    ActiveMissions.RemoveAll( [ & ]( auto mission_vm ) {
        return mission_vm->GetMission() == mission;
    } );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveMissions );
}

void UMSViewModel::SetMissionObjectiveStarted( UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective )
{
    if ( auto * mission_vm = GetMissionViewModel( mission ) )
    {
        mission_vm->SetObjectiveStarted( objective );
    }
}

void UMSViewModel::SetMissionObjectiveEnded( UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective )
{
    if ( auto * mission_vm = GetMissionViewModel( mission ) )
    {
        mission_vm->SetObjectiveEnded( objective );
    }
}

UMSMissionViewModel * UMSViewModel::GetMissionViewModel( UMSMission * mission ) const
{
    if ( auto * vm_ptr = ActiveMissions.FindByPredicate( [ & ]( auto mission_vm ) {
             return mission_vm->GetMission() == mission;
         } ) )
    {
        return *vm_ptr;
    }

    return nullptr;
}