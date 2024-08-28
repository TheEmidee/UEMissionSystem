#pragma once

#include <CoreMinimal.h>
#include <Factories/Factory.h>

#include "MSMissionObjectiveFactory.generated.h"

UCLASS()
class MISSIONSYSTEMEDITOR_API UMSMissionObjectiveFactory final : public UFactory
{
    GENERATED_BODY()

public:
    explicit UMSMissionObjectiveFactory( const FObjectInitializer & object_initializer );

    UObject * FactoryCreateNew(
        UClass * Class,
        UObject * InParent,
        FName Name,
        EObjectFlags Flags,
        UObject * Context,
        FFeedbackContext * Warn ) override;
};
