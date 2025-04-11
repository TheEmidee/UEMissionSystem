#include "ViewModels/MSObjectiveViewModel.h"

#include "MSMissionObjective.h"

void UMSObjectiveViewModel::Initialize( const TSubclassOf< UMSMissionObjective > & objective_class )
{
    ObjectiveClass = objective_class;

    const auto * cdo = ObjectiveClass.GetDefaultObject();
    Name = cdo->GetDescription();
    RequiredProgression = cdo->GetRequiredProgression();
}

void UMSObjectiveViewModel::SetProgression( int progression )
{
    CurrentProgression = progression;
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CurrentProgression );
}