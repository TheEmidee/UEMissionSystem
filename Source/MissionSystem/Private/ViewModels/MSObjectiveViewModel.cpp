#include "ViewModels/MSObjectiveViewModel.h"

#include "MSMissionObjective.h"

void UMSObjectiveViewModel::Initialize( const TSubclassOf< UMSMissionObjective > & objective_class )
{
    ObjectiveClass = objective_class;

    Name = ObjectiveClass.GetDefaultObject()->GetDescription();
}