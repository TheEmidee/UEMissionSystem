#include "MSMissionTestTypes.h"

#include <CoreMinimal.h>
#include <Misc/AutomationTest.h>

BEGIN_DEFINE_SPEC( FMSActionExecutorSpec, "MissionSystem.ActionExecutor", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask )
END_DEFINE_SPEC( FMSActionExecutorSpec )
void FMSActionExecutorSpec::Define()
{
    Describe( "Initialize", [ this ]() {
        It( "should set the instanced actions", [ this ]() {
            FMSActionExecutor executor;
            const TArray< TSubclassOf< UMSMissionAction > > action_classes = {
                UMSMissionActionMock_AutoFinishExecute::StaticClass(),
                UMSMissionActionMock_ManualFinishExecute::StaticClass()
            };

            executor.Initialize( *static_cast< UObject * >( GetTransientPackage() ), action_classes, []() {
            } );

            TEST_EQUAL( executor.GetInstancedActions().Num(), 2 );
        } );
    } );

    Describe( "Execute", [ this ]() {
        It( "should call the callback when it has no actions to run", [ this ]() {
            FMSActionExecutor executor;
            const TArray< TSubclassOf< UMSMissionAction > > action_classes;

            auto callback_called = false;

            executor.Initialize( *static_cast< UObject * >( GetTransientPackage() ), action_classes, [ &callback_called ]() {
                callback_called = true;
            } );

            executor.Execute();

            TEST_TRUE( callback_called );
        } );

        It( "should call the callback", [ this ]() {
            FMSActionExecutor executor;
            const TArray< TSubclassOf< UMSMissionAction > > action_classes = {
                UMSMissionActionMock_AutoFinishExecute::StaticClass(),
                UMSMissionActionMock_ManualFinishExecute::StaticClass()
            };

            auto callback_called = false;

            executor.Initialize( *static_cast< UObject * >( GetTransientPackage() ), action_classes, [ &callback_called ]() {
                callback_called = true;
            } );

            executor.Execute();

            Cast< UMSMissionActionMock_ManualFinishExecute >( executor.GetInstancedActions()[ 1 ] )->FinishExecute();

            TEST_TRUE( callback_called );
        } );

        It( "should call the callback only once", [ this ]() {
            FMSActionExecutor executor;
            const TArray< TSubclassOf< UMSMissionAction > > action_classes = {
                UMSMissionActionMock_AutoFinishExecute::StaticClass()
            };

            auto callback_called = false;
            auto callback_called_count = 0;

            executor.Initialize( *static_cast< UObject * >( GetTransientPackage() ), action_classes, [ &callback_called, &callback_called_count ]() {
                callback_called = true;
                callback_called_count++;
            } );

            executor.Execute();

            TEST_TRUE( callback_called );
            TEST_EQUAL( callback_called_count, 1 );
        } );
    } );
}