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
    ActiveObjectives.RemoveAll( [ & ]( auto objective_vm ) {
        return objective_vm->GetObjectiveClass() == objective;
    } );

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( ActiveObjectives );
}