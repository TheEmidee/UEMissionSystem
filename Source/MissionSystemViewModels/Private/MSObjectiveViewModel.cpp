#include "MSObjectiveViewModel.h"

#include "MSMissionObjective.h"

void UMSObjectiveViewModel::Initialize( const TSubclassOf< UMSMissionObjective > & objective )
{
    Objective = objective;
    CurrentProgression = 0;
    RequiredProgression = objective.GetDefaultObject()->GetRequiredProgression();

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( GetDescription );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CurrentProgression );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( RequiredProgression );
}

void UMSObjectiveViewModel::SetCompleted( bool was_cancelled )
{
    bIsCompleted = true;
    bWasCancelled = was_cancelled;

    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( bIsCompleted );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( bWasCancelled );
}

void UMSObjectiveViewModel::UpdateProgression( int current_progression )
{
    CurrentProgression = current_progression;
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( CurrentProgression );
    UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED( GetDescription );
}

FText UMSObjectiveViewModel::GetDescription() const
{
    if ( RequiredProgression > INDEX_NONE )
    {
        return FText::FormatNamed( Objective.GetDefaultObject()->GetDescription(), TEXT( "CurrentProgression" ), CurrentProgression, TEXT( "RequiredProgression" ), RequiredProgression );
    }

    return Objective.GetDefaultObject()->GetDescription();
}