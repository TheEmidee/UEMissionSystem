#include "Factories/MSMissionDataFactory.h"

#include "MSMissionData.h"

UMSMissionDataFactory::UMSMissionDataFactory( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    bCreateNew = true;

    // true if the associated editor should be opened after creating a new object.
    bEditAfterNew = true;
    SupportedClass = UMSMissionData::StaticClass();
}

UObject * UMSMissionDataFactory::FactoryCreateNew(
    UClass * Class,
    UObject * InParent,
    FName Name,
    EObjectFlags Flags,
    UObject * Context,
    FFeedbackContext * Warn )
{
    return NewObject< UMSMissionData >( InParent, Class, Name, Flags | RF_Transactional );
}