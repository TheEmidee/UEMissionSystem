#include "ViewModels/MSMissionViewModel.h"

#include "MSMission.h"
#include "MSMissionData.h"
#include "ViewModels/MSObjectiveViewModel.h"

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

void UMSMissionViewModel::SetObjectiveStarted( UMSMissionObjective * objective )
{
    auto * objective_vm = NewObject< UMSObjectiveViewModel >( this );
    objective_vm->Initialize( objective );

    ActiveObjectives.Add( objective_vm );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );
}

void UMSMissionViewModel::RefreshObjectiveProgression( UMSMissionObjective * objective )
{
    if ( const auto * objective_vm = ActiveObjectives.FindByPredicate( [ & ]( auto view_model ) {
             return view_model->GetObjective() == objective;
         } ) )
    {
        ( *objective_vm )->RefreshProgression();
    }
}

void UMSMissionViewModel::SetObjectiveEnded( UMSMissionObjective * objective )
{
    const auto predicate = [ & ]( auto objective_vm ) {
        return objective_vm->GetObjective() == objective;
    };

    if ( auto * objective_vm = ActiveObjectives.FindByPredicate( predicate ) )
    {
        CompletedObjectives.AddUnique( *objective_vm );

        ActiveObjectives.RemoveAll( predicate );

        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedObjectives );
    }
}