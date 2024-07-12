#include "MSMissionObjective.h"

#include "DVEDataValidator.h"
#include "MSMissionAction.h"

UMSMissionObjective::UMSMissionObjective() :
    bExecuteEndActionsWhenCancelled( false ),
    bIsComplete( false ),
    bIsCancelled( false )
{
}

void UMSMissionObjective::Execute()
{
    StartActionsExecutor.Initialize( *this, StartActions, [ this ]() {
        K2_Execute();
    } );

    EndActionsExecutor.Initialize( *this, EndActions, [ this ]() {
        OnObjectiveCompleteEvent.Broadcast( this, bIsCancelled );
    } );

    StartActionsExecutor.Execute();
}

void UMSMissionObjective::PostLoad()
{
    UObject::PostLoad();

    GenerateGuidIfNeeded();
}

void UMSMissionObjective::PostDuplicate( bool duplicate_for_pie )
{
    UObject::PostDuplicate( duplicate_for_pie );

    GenerateGuidIfNeeded( true );
}

void UMSMissionObjective::PostEditImport()
{
    UObject::PostEditImport();

    GenerateGuidIfNeeded( true );
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

#if WITH_EDITOR
EDataValidationResult UMSMissionObjective::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    return FDVEDataValidator( context )
        .NoNullItem( VALIDATOR_GET_PROPERTY( StartActions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( EndActions ) )
        .IsValid( VALIDATOR_GET_PROPERTY( ObjectiveId ) )
        .Result();
}

#endif

void UMSMissionObjective::GenerateGuidIfNeeded( bool force_generation )
{
    if ( !ObjectiveId.IsValid() || force_generation )
    {
        ObjectiveId = FGuid::NewGuid();
        Modify();
    }
}

void UMSMissionObjective::K2_Execute_Implementation()
{
}

void UMSMissionObjective::K2_OnObjectiveEnded_Implementation( bool /*was_cancelled*/ )
{
}
