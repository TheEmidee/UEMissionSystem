#include "Factories/MSMissionObjectiveFactory.h"

#include "MSMissionObjective.h"

UMSMissionObjectiveFactory::UMSMissionObjectiveFactory( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    bCreateNew = true;

    // true if the associated editor should be opened after creating a new object.
    bEditAfterNew = true;
    SupportedClass = UMSMissionObjective::StaticClass();
}

UObject * UMSMissionObjectiveFactory::FactoryCreateNew(
    UClass * Class,
    UObject * InParent,
    FName Name,
    EObjectFlags Flags,
    UObject * Context,
    FFeedbackContext * Warn )
{
    return NewObject< UMSMissionObjective >( InParent, Class, Name, Flags | RF_Transactional );
}