#include "MSMissionViewModel.h"

#include "MSMission.h"
#include "MSMissionData.h"
#include "MSObjectiveViewModel.h"

void UMSMissionViewModel::RemoveCompletedObjective( UMSObjectiveViewModel * objective_vm )
{
    CompletedObjectives.Remove( objective_vm );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedObjectives );
}

void UMSMissionViewModel::Initialize( UMSMission * mission )
{
    check( mission != nullptr );

    Mission = mission;
    Name = Mission->GetMissionData()->Name;
}

UMSObjectiveViewModel * UMSMissionViewModel::SetObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective )
{
    auto * objective_vm = NewObject< UMSObjectiveViewModel >( this );
    objective_vm->Initialize( objective );

    ActiveObjectives.Add( objective_vm );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );

    OnMissionObjectiveStartedDelegate.Broadcast( this, objective_vm );

    return objective_vm;
}

UMSObjectiveViewModel * UMSMissionViewModel::UpdateObjectiveProgression( const TSubclassOf< UMSMissionObjective > & objective, int current_progression )
{
    if ( const auto * objective_vm = ActiveObjectives.FindByPredicate( [ & ]( auto view_model ) {
             return view_model->GetObjective() == objective;
         } ) )
    {
        ( *objective_vm )->UpdateProgression( current_progression );

        OnMissionObjectiveProgressionIsUpdatedDelegate.Broadcast( this, *objective_vm );

        return *objective_vm;
    }

    return nullptr;
}

UMSObjectiveViewModel * UMSMissionViewModel::SetObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective, bool was_cancelled )
{
    const auto predicate = [ & ]( auto objective_vm ) {
        return objective_vm->GetObjective() == objective;
    };

    auto * existing_view_model = ActiveObjectives.FindByPredicate( predicate );

    UMSObjectiveViewModel * view_model = nullptr;

    if ( existing_view_model != nullptr )
    {
        view_model = *existing_view_model;
    }
    else
    {
        // When resuming missions from the history, we may broadcast an objective ended directly without having broadcasted that the objective
        // did start at some point
        view_model = NewObject< UMSObjectiveViewModel >( this );
        view_model->Initialize( objective );
    }

    view_model->SetCompleted( was_cancelled );

    CompletedObjectives.AddUnique( view_model );
    ActiveObjectives.RemoveAll( predicate );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedObjectives );

    OnMissionObjectiveEndedDelegate.Broadcast( this, view_model, was_cancelled );

    return view_model;
}