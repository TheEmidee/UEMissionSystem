#include "MSMissionObjective.h"

#include "DVEDataValidator.h"
#include "MSMissionAction.h"

UMSMissionObjective::UMSMissionObjective()
{
    bExecuteEndActionsWhenCancelled = false;
    bIsComplete = false;
    bIsCancelled = false;
}

void UMSMissionObjective::Execute()
{
    OnObjectiveStartedEvent.Broadcast( this );

    StartActionsExecutor.Initialize( *this, StartActions, [ this ]() {
        K2_Execute();
    } );

    EndActionsExecutor.Initialize( *this, EndActions, [ this ]() {
        OnObjectiveCompleteEvent.Broadcast( this, bIsCancelled );
    } );

    StartActionsExecutor.Execute();
}

void UMSMissionObjective::CompleteObjective()
{
    if ( !bIsComplete )
    {
        bIsComplete = true;
        K2_OnObjectiveEnded( false );
        EndActionsExecutor.Execute();
    }
}

UWorld * UMSMissionObjective::GetWorld() const
{
    if ( IsTemplate() )
    {
        return nullptr;
    }

    if ( const auto * outer = GetOuter() )
    {
        return outer->GetWorld();
    }

    return nullptr;
}

void UMSMissionObjective::GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const
{
    tag_container.AppendTags( Tags );
}

void UMSMissionObjective::CancelObjective()
{
    if ( !bIsComplete && !bIsCancelled )
    {
        bIsCancelled = true;

        K2_OnObjectiveEnded( true );

        if ( bExecuteEndActionsWhenCancelled )
        {
            EndActionsExecutor.Execute();
        }
        else
        {
            OnObjectiveCompleteEvent.Broadcast( this, bIsCancelled );
        }
    }
}
#if WITH_EDITOR
EDataValidationResult UMSMissionObjective::IsDataValid( TArray< FText > & validation_errors )
{
    Super::IsDataValid( validation_errors );

    return FDVEDataValidator( validation_errors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( StartActions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( EndActions ) )
        .Result();
}
#endif

void UMSMissionObjective::K2_Execute_Implementation()
{
}

void UMSMissionObjective::K2_OnObjectiveEnded_Implementation( bool /*was_cancelled*/ )
{
}
