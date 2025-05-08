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

        OnMissionStartedDelegate.Broadcast( mission_vm );
    }
}

void UMSViewModel::SetMissionEnded( const UMSMissionData * mission, bool was_cancelled )
{
    const auto predicate = [ & ]( const TObjectPtr< UMSMissionViewModel > & mission_vm ) {
        return mission_vm->GetMission()->GetMissionData() == mission;
    };

    auto * mission_vm = ActiveMissions.FindByPredicate( predicate );

    if ( mission_vm != nullptr )
    {
        CompletedMissions.AddUnique( *mission_vm );

        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedMissions );
    }

    ActiveMissions.RemoveAll( predicate );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveMissions );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( HasActiveMissions );

    if ( mission_vm != nullptr )
    {
        OnMissionEndedDelegate.Broadcast( *mission_vm, was_cancelled );
    }
}

void UMSViewModel::SetMissionObjectiveStarted( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > objective )
{
    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        if ( auto * objective_vm = mission_vm->SetObjectiveStarted( objective ) )
        {
            OnMissionObjectiveStartedDelegate.Broadcast( mission_vm, objective_vm );
        }
    }
}

void UMSViewModel::RefreshMissionObjectiveProgression( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression, int required_progression )
{
    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        if ( auto * objective_vm = mission_vm->UpdateObjectiveProgression( mission_objective, current_progression ) )
        {
            OnMissionObjectiveProgressionIsUpdatedDelegate.Broadcast( mission_vm, objective_vm );
        }
    }
}

void UMSViewModel::SetMissionObjectiveEnded( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > objective, bool was_cancelled )
{
    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        if ( auto * objective_vm = mission_vm->SetObjectiveEnded( objective, was_cancelled ) )
        {
            OnMissionEndedDelegate.Broadcast( mission_vm, was_cancelled );
            OnMissionObjectiveEndedDelegate.Broadcast( mission_vm, objective_vm, was_cancelled );
        }
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