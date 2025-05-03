#include "MSViewModel.h"

#include "MSMission.h"
#include "MSMissionData.h"
#include "MSMissionSystemComponent.h"
#include "MSMissionViewModel.h"

void UMSViewModel::Initialize( UMSMissionSystemComponent * component )
{
    component->OnMissionStarted().AddDynamic( this, &ThisClass::SetMissionStarted );
    component->OnMissionEnded().AddDynamic( this, &ThisClass::SetMissionEnded );
    component->OnMissionObjectiveStarted().AddDynamic( this, &ThisClass::SetMissionObjectiveStarted );
    component->OnMissionObjectiveProgressionUpdated().AddDynamic( this, &ThisClass::RefreshMissionObjectiveProgression );
    component->OnMissionObjectiveEnded().AddDynamic( this, &ThisClass::SetMissionObjectiveEnded );
}

bool UMSViewModel::HasActiveMissions() const
{
    return !ActiveMissions.IsEmpty();
}

UMSViewModel * UMSViewModel::CreateMissionSystemViewModel( UMSMissionSystemComponent * component )
{
    auto * view_model = NewObject< UMSViewModel >();

    view_model->Initialize( component );

    return view_model;
}

void UMSViewModel::RemoveCompletedMission( UMSMissionViewModel * mission_vm )
{
    CompletedMissions.Remove( mission_vm );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedMissions );

    mission_vm->OnObjectiveStatusChanged().RemoveAll( this );
}

void UMSViewModel::SetMissionStarted( UMSMission * mission )
{
    if ( GetMissionViewModel( mission->GetMissionData() ) != nullptr )
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
        mission_vm->OnObjectiveStatusChanged().AddUObject( this, &ThisClass::BroadCastOnMissionsObjectivesChanged );
    }
}

void UMSViewModel::SetMissionEnded( const UMSMissionData * mission, bool /*was_cancelled*/ )
{
    const auto predicate = [ & ]( const TObjectPtr< UMSMissionViewModel > & mission_vm ) {
        return mission_vm->GetMission()->GetMissionData() == mission;
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

void UMSViewModel::SetMissionObjectiveStarted( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > objective )
{
    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        mission_vm->SetObjectiveStarted( objective );
    }
}

void UMSViewModel::RefreshMissionObjectiveProgression( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression, int /*required_progression*/ )
{
    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        mission_vm->UpdateObjectiveProgression( mission_objective, current_progression );
    }
}

void UMSViewModel::SetMissionObjectiveEnded( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > objective, bool was_cancelled )
{
    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        mission_vm->SetObjectiveEnded( objective );
    }
}

UMSMissionViewModel * UMSViewModel::GetMissionViewModel( const UMSMissionData * mission_data ) const
{
    if ( auto * vm_ptr = ActiveMissions.FindByPredicate( [ & ]( auto mission_vm ) {
             return mission_vm->GetMission()->GetMissionData() == mission_data;
         } ) )
    {
        return *vm_ptr;
    }

    return nullptr;
}

void UMSViewModel::BroadCastOnMissionsObjectivesChanged()
{
    OnMissionsObjectivesChangedDelegate.Broadcast();
}