#pragma once

#include "MSMissionAction.h"
#include "MSMissionObjective.h"
#include "MSMissionTypes.h"

#include <CoreMinimal.h>

#include "MSMissionTestTypes.generated.h"

#define TEST_TRUE( expression ) \
    EPIC_TEST_BOOLEAN_( TEXT( #expression ), expression, true )

#define TEST_FALSE( expression ) \
    EPIC_TEST_BOOLEAN_( TEXT( #expression ), expression, false )

#define TEST_EQUAL( expression, expected ) \
    EPIC_TEST_BOOLEAN_( TEXT( #expression ), expression, expected )

#define TEST_NULL( expression ) \
    TestNull( TEXT( #expression ), expression );

#define TEST_NOT_NULL( expression ) \
    TestNotNull( TEXT( #expression ), expression );

#define EPIC_TEST_BOOLEAN_( text, expression, expected ) \
    TestEqual( text, expression, expected );

class UMSMissionData;

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionEventReceiver final : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION()
    void OnMissionCompleteDelegateCalled( UMSMissionData * mission )
    {
        MissionData = mission;
        bDelegateWasCalled = true;
        CallCount++;
    }

    UFUNCTION()
    void OnMissionObjectiveDelegateCalled( UMSMissionObjective * objective )
    {
        MissionObjective = objective;
        bDelegateWasCalled = true;
        CallCount++;
    }

    UFUNCTION()
    void OnMissionActionDelegateCalled( UMSMissionAction * action )
    {
        MissionAction = action;
        bDelegateWasCalled = true;
        CallCount++;
    }

    UPROPERTY()
    UMSMissionData * MissionData = nullptr;

    UPROPERTY()
    UMSMissionObjective * MissionObjective = nullptr;

    UPROPERTY()
    UMSMissionAction * MissionAction = nullptr;

    int CallCount = 0;
    bool bDelegateWasCalled = false;
};

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionObjectiveMock : public UMSMissionObjective
{
    GENERATED_BODY()

public:
    void AddStartAction( const TSubclassOf< UMSMissionAction > & action_class )
    {
        StartActions.Add( action_class );
    }

    void AddEndAction( const TSubclassOf< UMSMissionAction > & action_class )
    {
        EndActions.Add( action_class );
    }

    const TArray< UMSMissionAction * > & GetStartActions() const
    {
        return StartActionsExecutor.GetInstancedActions();
    }

    const TArray< UMSMissionAction * > & GetEndActions() const
    {
        return EndActionsExecutor.GetInstancedActions();
    }

    bool bHasBeenExecuted = false;

    bool bHasBeenEnded = false;

protected:
    void K2_OnObjectiveEnded_Implementation() override
    {
        bHasBeenEnded = true;
    };
};

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionObjectiveMock_AutoComplete final : public UMSMissionObjectiveMock
{
    GENERATED_BODY()

    void K2_Execute_Implementation() override
    {
        bHasBeenExecuted = true;
        CompleteObjective();
    }
};

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionObjectiveMock_NoAutoComplete final : public UMSMissionObjectiveMock
{
    GENERATED_BODY()

    void K2_Execute_Implementation() override
    {
        bHasBeenExecuted = true;
    }
};

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionActionMock : public UMSMissionAction
{
    GENERATED_BODY()

public:
    bool bHasBeenExecuted = false;
};

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionActionMock_AutoFinishExecute final : public UMSMissionActionMock
{
    GENERATED_BODY()

public:
    void Execute_Implementation() override
    {
        FinishExecute();
        bHasBeenExecuted = true;
    }
};

UCLASS( HideDropdown )
class MISSIONSYSTEMTESTSUITE_API UMSMissionActionMock_ManualFinishExecute final : public UMSMissionActionMock
{
    GENERATED_BODY()

public:
    void Execute_Implementation() override
    {
        bHasBeenExecuted = true;
    }
};