#include "MSViewModel.h"

#include "MSMission.h"
#include "MSMissionData.h"
#include "MSMissionSystemComponent.h"
#include "MSMissionViewModel.h"

void UMSViewModel::Initialize( UMSMissionSystemComponent * component )
{
    component->OnMissionStarted().AddDynamic( this, &ThisClass::OnMissionStarted );
    component->OnMissionEnded().AddDynamic( this, &ThisClass::OnMissionEnded );
    component->OnMissionObjectiveStarted().AddDynamic( this, &ThisClass::OnMissionObjectiveStarted );
    component->OnMissionObjectiveProgressionUpdated().AddDynamic( this, &ThisClass::OnMissionObjectiveProgressionUpdated );
    component->OnMissionObjectiveEnded().AddDynamic( this, &ThisClass::OnMissionObjectiveEnded );
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

void UMSViewModel::OnMissionStarted( UMSMission * mission )
{
    if ( mission->GetMissionData()->bInvisibleMission )
    {
        return;
    }

    if ( GetMissionViewModel( mission->GetMissionData() ) != nullptr )
    {
        return;
    }

    auto * mission_vm = NewObject< UMSMissionViewModel >( this );
    mission_vm->Initialize( mission );
    ActiveMissions.Add( mission_vm );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveMissions );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( HasActiveMissions );

    OnMissionStartedDelegate.Broadcast( mission_vm );
}

void UMSViewModel::OnMissionEnded( const UMSMissionData * mission, bool was_cancelled )
{
    if ( mission->bInvisibleMission )
    {
        return;
    }

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

void UMSViewModel::OnMissionObjectiveStarted( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression )
{
    if ( mission_data->bInvisibleMission )
    {
        return;
    }

    if ( mission_objective.GetDefaultObject()->IsInvisibleObjective() )
    {
        return;
    }

    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        if ( auto * objective_vm = mission_vm->SetObjectiveStarted( mission_objective, current_progression ) )
        {
            OnMissionObjectiveStartedDelegate.Broadcast( mission_vm, objective_vm, current_progression );
        }
    }
}

void UMSViewModel::OnMissionObjectiveProgressionUpdated( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, int current_progression, int required_progression )
{
    if ( mission_data->bInvisibleMission )
    {
        return;
    }

    if ( mission_objective.GetDefaultObject()->IsInvisibleObjective() )
    {
        return;
    }

    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        if ( auto * objective_vm = mission_vm->UpdateObjectiveProgression( mission_objective, current_progression ) )
        {
            OnMissionObjectiveProgressionIsUpdatedDelegate.Broadcast( mission_vm, objective_vm );
        }
    }
}

void UMSViewModel::OnMissionObjectiveEnded( const UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective, bool was_cancelled )
{
    if ( mission_data->bInvisibleMission )
    {
        return;
    }

    if ( mission_objective.GetDefaultObject()->IsInvisibleObjective() )
    {
        return;
    }

    if ( auto * mission_vm = GetMissionViewModel( mission_data ) )
    {
        if ( auto * objective_vm = mission_vm->SetObjectiveEnded( mission_objective, was_cancelled ) )
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