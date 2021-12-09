#include "MSMissionObjective.h"
#include "MSMissionTestTypes.h"

#include <CoreMinimal.h>
#include <Misc/AutomationTest.h>

BEGIN_DEFINE_SPEC( FMSMissionObjectiveSpec, "MissionSystem.MissionObjective", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask )
END_DEFINE_SPEC( FMSMissionObjectiveSpec )
void FMSMissionObjectiveSpec::Define()
{
    Describe( "Execute", [ this ]() {
        It( "should call K2_Execute", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->Execute();

            TEST_TRUE( objective->bHasBeenExecuted );
        } );

        It( "should call start actions before K2_Execute", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->AddStartAction( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            objective->Execute();

            auto * action = Cast< UMSMissionActionMock_AutoFinishExecute >( objective->GetStartActions()[ 0 ] );
            TEST_TRUE( action->bHasBeenExecuted );
            TEST_TRUE( objective->bHasBeenExecuted );
        } );

        It( "should not call K2_Execute if all start actions have not been completed", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->AddStartAction( UMSMissionActionMock_ManualFinishExecute::StaticClass() );

            objective->Execute();

            TEST_FALSE( objective->bHasBeenExecuted );
        } );
    } );

    Describe( "IsComplete", [ this ]() {
        It( "should return false when not complete", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            TEST_FALSE( objective->IsComplete() );
        } );

        It( "should have IsComplete return true when complete", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->Execute();
            objective->CompleteObjective();
            TEST_TRUE( objective->IsComplete() );
        } );
    } );

    Describe( "CompleteObjective", [ this ]() {
        It( "should execute EndActions", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->AddEndAction( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            objective->Execute();
            objective->CompleteObjective();

            auto * action = Cast< UMSMissionActionMock_AutoFinishExecute >( objective->GetEndActions()[ 0 ] );

            TEST_TRUE( action->bHasBeenExecuted );
        } );

        It( "should call OnMissionObjectiveComplete when no end actions are defined", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();

            objective->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            objective->Execute();
            objective->CompleteObjective();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( Cast< UMSMissionObjective >( objective ), event_receiver->MissionObjective );
        } );

        It( "should call OnMissionObjectiveComplete when end actions have been executed", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->AddEndAction( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();

            objective->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            objective->Execute();
            objective->CompleteObjective();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( Cast< UMSMissionObjective >( objective ), event_receiver->MissionObjective );
        } );

        It( "should not call OnMissionObjectiveComplete when not all end actions have been executed", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();
            objective->AddEndAction( UMSMissionActionMock_ManualFinishExecute::StaticClass() );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();

            objective->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            objective->Execute();
            objective->CompleteObjective();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );

        It( "should not call OnMissionObjectiveComplete multiple times when CompleteObjective is called multiple times", [ this ]() {
            auto * objective = NewObject< UMSMissionObjectiveMock_AutoComplete >();

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            objective->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            objective->Execute();
            objective->CompleteObjective();
            objective->CompleteObjective();

            TEST_EQUAL( event_receiver->CallCount, 1 );
        } );
    } );
}