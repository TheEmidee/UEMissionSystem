#include "MSMissionObjective.h"

#include "DVEDataValidator.h"
#include "MSMissionAction.h"

UMSMissionObjective::UMSMissionObjective()
{
    bIsComplete = false;
}

void UMSMissionObjective::Execute()
{
    StartActionsExecutor.Initialize( *this, StartActions, [ this ]() {
        K2_Execute();
    } );

    EndActionsExecutor.Initialize( *this, EndActions, [ this ]() {
        OnObjectiveCompleteDelegate.Broadcast( this );
    } );

    StartActionsExecutor.Execute();
}

void UMSMissionObjective::CompleteObjective()
{
    if ( !bIsComplete )
    {
        bIsComplete = true;
        K2_OnObjectiveEnded();
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

#if WITH_EDITOR
EDataValidationResult UMSMissionObjective::IsDataValid( TArray<FText> & validation_errors )
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

void UMSMissionObjective::K2_OnObjectiveEnded_Implementation()
{
}
