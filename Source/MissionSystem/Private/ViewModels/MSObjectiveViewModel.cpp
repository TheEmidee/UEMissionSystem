#include "ViewModels/MSObjectiveViewModel.h"

#include "MSMissionObjective.h"
#include "Templates/SubclassOf.h"

void UMSObjectiveViewModel::Initialize( const TSubclassOf< UMSMissionObjective > & objective_class )
{
    ObjectiveClass = objective_class;

    Name = ObjectiveClass.GetDefaultObject()->GetDescription();
}