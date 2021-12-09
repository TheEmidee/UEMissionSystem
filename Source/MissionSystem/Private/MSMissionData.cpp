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

#if WITH_EDITOR
EDataValidationResult UMSMissionData::IsDataValid( TArray< FText > & validation_errors )
{
    Super::IsDataValid( validation_errors );

    return FDVEDataValidator( validation_errors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( StartActions ) )
        .NoNullItem( VALIDATOR_GET_PROPERTY( NextMissions ) )
        .CustomValidation< TArray< FMSMissionObjectiveData > >( Objectives, []( TArray< FText > & errors, TArray< FMSMissionObjectiveData > objectives ) {
            for ( const auto & objective_data : objectives )
            {
                if ( objective_data.Objective == nullptr )
                {
                    errors.Add( FText::FromString( TEXT( "Objectives contains an invalid objective" ) ) );
                }
            }
        } )
        .Result();
}
#endif