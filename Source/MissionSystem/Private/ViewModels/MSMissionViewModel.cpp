#include "ViewModels/MSMissionViewModel.h"

#include "MSMission.h"
#include "MSMissionData.h"
#include "ViewModels/MSObjectiveViewModel.h"

void UMSMissionViewModel::Initialize( UMSMission * mission )
{
    check( mission != nullptr );

    Mission = mission;
    Name = Mission->GetMissionData()->Name;
}

void UMSMissionViewModel::SetObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective )
{
    auto * objective_vm = NewObject< UMSObjectiveViewModel >( this );
    objective_vm->Initialize( objective );

    ActiveObjectives.Add( objective_vm );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );
}

void UMSMissionViewModel::SetObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective )
{
    const auto predicate = [ & ]( auto objective_vm ) {
        return objective_vm->GetObjectiveClass() == objective;
    };

    auto * objective_vm = ActiveObjectives.FindByPredicate( predicate );
    CompletedObjectives.AddUnique( *objective_vm );

    ActiveObjectives.RemoveAll( predicate );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CompletedObjectives );
}