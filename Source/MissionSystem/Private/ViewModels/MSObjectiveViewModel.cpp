#include "ViewModels/MSObjectiveViewModel.h"

#include "MSMissionObjective.h"

void UMSObjectiveViewModel::Initialize(const UMSMissionObjective* objective)
{
    Objective = objective;

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( GetDescription );
}

void UMSObjectiveViewModel::RefreshProgression()
{
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( GetDescription );
}

FText UMSObjectiveViewModel::GetDescription() const
{
    if ( Objective->GetRequiredProgression() > INDEX_NONE )
    {
        return FText::FormatNamed( Objective->GetDescription(), TEXT( "CurrentProgression" ), Objective->GetCurrentProgression(), TEXT( "RequiredProgression" ), Objective->GetRequiredProgression() );
    }

    return Objective->GetDescription();
}