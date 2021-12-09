#include "MSMission.h"
#include "MSMissionSystem.h"
#include "MSMissionTestTypes.h"

#include <CoreMinimal.h>
#include <Misc/AutomationTest.h>

BEGIN_DEFINE_SPEC( FMSMissionSpec, "MissionSystem.Mission", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask )
END_DEFINE_SPEC( FMSMissionSpec )
void FMSMissionSpec::Define()
{
    Describe( "Initialize", [ this ]() {
        It( "should set the mission data", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            TEST_EQUAL( mission->GetData(), mission_data );
        } );

        It( "should instantiate the objectives of the mission data", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            TEST_EQUAL( mission->GetObjectives().Num(), mission_data->Objectives.Num() );
        } );

        It( "should not instantiate the disabled objectives of the mission data", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass(), false ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            TEST_EQUAL( mission->GetObjectives().Num(), 1 );
        } );

        It( "should instantiate the start actions of the mission data", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );
            mission_data->StartActions.Add( UMSMissionActionMock_ManualFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            TEST_EQUAL( mission->GetStartActions().Num(), mission_data->StartActions.Num() );
        } );
    } );

    Describe( "OnMissionObjectiveComplete", [ this ]() {
        It( "should not be called when Start has not been called", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            auto * objective = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );
            objective->CompleteObjective();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );

        It( "should be called when an objective is completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            auto * objective = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );

            mission->Start();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( Cast< UMSMissionObjective >( objective ), event_receiver->MissionObjective );
        } );
    } );

    Describe( "OnMissionComplete", [ this ]() {
        It( "should not be called when Start has not been called", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            auto * objective = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );
            objective->CompleteObjective();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );

        It( "should not be called when all objectives are not completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            mission->Start();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );

        It( "should be called when all objectives are completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            mission->Start();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
        } );

        It( "should be called when all end actions are completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->EndActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            mission->Start();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
        } );

        It( "should not be called when not all end actions are completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->EndActions.Add( UMSMissionActionMock_ManualFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            mission->Start();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );
    } );

    Describe( "IsComplete", [ this ]() {
        It( "should return false when not all objectives are complete", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            mission->Start();

            TEST_FALSE( mission->IsComplete() );
        } );

        It( "should return true when all objectives are complete", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            mission->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            mission->Start();

            TEST_FALSE( mission->IsComplete() );

            auto * objective2 = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 1 ] );
            objective2->CompleteObjective();

            TEST_TRUE( mission->IsComplete() );
        } );
    } );

    Describe( "Start", [ this ]() {
        It( "executes all start actions", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * action1 = Cast< UMSMissionActionMock_AutoFinishExecute >( mission->GetStartActions()[ 0 ] );
            auto * action2 = Cast< UMSMissionActionMock_AutoFinishExecute >( mission->GetStartActions()[ 1 ] );

            auto * event_receiver1 = NewObject< UMSMissionEventReceiver >();
            action1->OnMissionActionComplete().AddUObject( event_receiver1, &UMSMissionEventReceiver::OnMissionActionDelegateCalled );

            auto * event_receiver2 = NewObject< UMSMissionEventReceiver >();
            action2->OnMissionActionComplete().AddUObject( event_receiver2, &UMSMissionEventReceiver::OnMissionActionDelegateCalled );

            mission->Start();

            TEST_TRUE( action1->bHasBeenExecuted );
            TEST_TRUE( action2->bHasBeenExecuted );
        } );

        It( "executes objectives when all start actions have been completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * objective1 = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );

            mission->Start();

            auto * action1 = Cast< UMSMissionActionMock_AutoFinishExecute >( mission->GetStartActions()[ 0 ] );
            auto * action2 = Cast< UMSMissionActionMock_AutoFinishExecute >( mission->GetStartActions()[ 1 ] );

            TEST_TRUE( action1->bHasBeenExecuted );
            TEST_TRUE( action2->bHasBeenExecuted );
            TEST_TRUE( objective1->bHasBeenExecuted );
        } );

        It( "does not execute objectives when start actions have not all been completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );
            mission_data->StartActions.Add( UMSMissionActionMock_ManualFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * objective1 = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );

            mission->Start();

            TEST_FALSE( objective1->bHasBeenExecuted );
        } );

        It( "does not execute objectives when the mission is already started", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * objective1 = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );

            mission->Start();

            TEST_TRUE( objective1->bHasBeenExecuted );

            objective1->bHasBeenExecuted = false;

            mission->Start();

            TEST_FALSE( objective1->bHasBeenExecuted );
        } );

        It( "execute objectives one after each other", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * objective1 = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );
            auto * objective2 = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 1 ] );

            mission->Start();

            TEST_TRUE( objective1->bHasBeenExecuted );
            TEST_FALSE( objective2->bHasBeenExecuted );

            objective1->CompleteObjective();

            TEST_TRUE( objective2->bHasBeenExecuted );
        } );
    } );

    Describe( "IsStarted", [ this ]() {
        It( "should return false when the mission has not been started", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            TEST_FALSE( mission->IsStarted() );
        } );

        It( "should return true when the mission has been started and all actions have been executed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            mission->Start();
            TEST_TRUE( mission->IsStarted() );
        } );

        It( "should return true when the mission has been started and all latent actions have been executed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->StartActions.Add( UMSMissionActionMock_AutoFinishExecute::StaticClass() );
            mission_data->StartActions.Add( UMSMissionActionMock_ManualFinishExecute::StaticClass() );

            auto * mission = NewObject< UMSMission >();
            mission->Initialize( mission_data );

            auto * action1 = Cast< UMSMissionActionMock_AutoFinishExecute >( mission->GetStartActions()[ 0 ] );
            auto * action3 = Cast< UMSMissionActionMock_ManualFinishExecute >( mission->GetStartActions()[ 1 ] );

            mission->Start();
            TEST_FALSE( mission->IsStarted() );

            action3->FinishExecute();

            TEST_TRUE( mission->IsStarted() );
        } );
    } );
}