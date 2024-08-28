#pragma once

#include <CoreMinimal.h>
#include <Factories/Factory.h>

#include "MSMissionDataFactory.generated.h"

UCLASS()
class MISSIONSYSTEMEDITOR_API UMSMissionDataFactory final : public UFactory
{
    GENERATED_BODY()

public:
    explicit UMSMissionDataFactory( const FObjectInitializer & object_initializer );

    UObject * FactoryCreateNew(
        UClass * Class,
        UObject * InParent,
        FName Name,
        EObjectFlags Flags,
        UObject * Context,
        FFeedbackContext * Warn ) override;
};
