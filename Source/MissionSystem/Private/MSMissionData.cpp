#include "MSMissionData.h"

#include "DVEDataValidator.h"

FMSMissionObjectiveData::FMSMissionObjectiveData()
{
    bEnabled = true;
}

FMSMissionObjectiveData::FMSMissionObjectiveData( const TSubclassOf< UMSMissionObjective > & objective, const bool enabled /*= true*/ ) :
    Objective( objective ),
    bEnabled( enabled )
{
}

UMSMissionData::UMSMissionData() :
    bEnabled( true ),
    bExecuteEndActionsWhenCancelled( true ),
    bStartNextMissionsWhenCancelled( false )
{
}

void UMSMissionData::PostLoad()
{
    Super::PostLoad();

    RegenerateGuidIfNeeded();
}

void UMSMissionData::PostDuplicate( bool duplicate_for_pie )
{
    Super::PostDuplicate( duplicate_for_pie );

    RegenerateGuidIfNeeded();
}

void UMSMissionData::PostEditImport()
{
    Super::PostEditImport();

    RegenerateGuidIfNeeded();
}

#if WITH_EDITOR
EDataValidationResult UMSMissionData::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    return FDVEDataValidator( context )
        .NoNullItem( VALIDATOR_GET_PROPERTY( StartActions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( EndActions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( NextMissions ) )
        .CustomValidation< TArray< FMSMissionObjectiveData > >( Objectives, []( FDataValidationContext & context, const TArray< FMSMissionObjectiveData > & objectives ) {
            for ( const auto & objective_data : objectives )
            {
                if ( objective_data.Objective == nullptr )
                {
                    context.AddError( FText::FromString( TEXT( "Objectives contains an invalid objective" ) ) );
                }
            }
        } )
        .Result();
}

#endif

void UMSMissionData::RegenerateGuidIfNeeded()
{
    if ( !MissionId.IsValid() )
    {
        MissionId = FGuid::NewGuid();
        Modify();
    }
}