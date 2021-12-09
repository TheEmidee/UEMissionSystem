#include "MSMissionSystem.h"
#include "MSMissionTestTypes.h"

#include <CoreMinimal.h>
#include <Misc/AutomationTest.h>

BEGIN_DEFINE_SPEC( FMSMissionSystemSpec, "MissionSystem.MissionSystem", EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask )
UMSMissionSystem * MissionSystem;
END_DEFINE_SPEC( FMSMissionSystemSpec )
void FMSMissionSystemSpec::Define()
{
    BeforeEach( [ this ]() {
        MissionSystem = NewObject< UMSMissionSystem >();
    } );

    AfterEach( [ this ]() {
        MissionSystem = nullptr;
    } );

    Describe( "StartMission", [ this ]() {
        It( "should return a null mission pointer when a null mission data is given", [ this ]() {
            TEST_NULL( MissionSystem->StartMission( nullptr ) );
        } );

        It( "should return a valid mission when a valid mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = MissionSystem->StartMission( mission_data );
            TEST_NOT_NULL( mission );
        } );

        It( "should return a null mission pointer when an already started mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            TEST_NOT_NULL( MissionSystem->StartMission( mission_data ) );
            TEST_NULL( MissionSystem->StartMission( mission_data ) );
        } );

        It( "should return a null mission pointer when an already completed mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = MissionSystem->StartMission( mission_data );

            TEST_NOT_NULL( mission );

            auto * objective = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );
            objective->CompleteObjective();

            TEST_NULL( MissionSystem->StartMission( mission_data ) );
        } );

        It( "should start a mission", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission = MissionSystem->StartMission( mission_data );

            TEST_TRUE( mission->IsStarted() );
            TEST_FALSE( mission->IsComplete() );
        } );

        It( "is called with next missions when an active mission is completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            auto * next_mission_data = NewObject< UMSMissionData >();
            next_mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            mission_data->NextMissions.Add( next_mission_data );

            auto * mission = MissionSystem->StartMission( mission_data );
            TEST_TRUE( mission->IsComplete() );
            auto * next_mission = MissionSystem->GetActiveMission( next_mission_data );
            TEST_NOT_NULL( next_mission )
        } );
    } );

    Describe( "GetActiveMission", [ this ]() {
        It( "returns a null mission when the mission data is null", [ this ]() {
            TEST_NULL( MissionSystem->GetActiveMission( nullptr ) )
        } );

        It( "returns a null mission when the mission is not started or completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            TEST_NULL( MissionSystem->GetActiveMission( mission_data ) )
        } );

        It( "returns a mission when the mission is started", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission = MissionSystem->StartMission( mission_data );
            TEST_NOT_NULL( mission )
            TEST_FALSE( mission->IsComplete() )

            auto * mission_from_get_mission = MissionSystem->GetActiveMission( mission_data );
            TEST_NOT_NULL( mission_from_get_mission )
            TEST_EQUAL( mission_from_get_mission, mission )
        } );

        It( "returns a null mission when the mission is completed", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );

            auto * mission = MissionSystem->StartMission( mission_data );
            TEST_NOT_NULL( mission )
            TEST_TRUE( mission->IsComplete() )

            TEST_NULL( MissionSystem->GetActiveMission( mission_data ) )
        } );
    } );

    Describe( "Delegates", [ this ]() {
        It( "should broadcast ObjectiveComplete when a mission objective is complete", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            objective->CompleteObjective();
            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( Cast< UMSMissionObjective >( objective ), event_receiver->MissionObjective );
        } );

        It( "should broadcast MissionComplete when a mission is complete", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            objective->CompleteObjective();
            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->MissionData, mission_data );
        } );
    } );

    Describe( "IsMissionComplete", [ this ]() {
        It( "should return false when a null mission data is given", [ this ]() {
            TEST_FALSE( MissionSystem->IsMissionComplete( nullptr ) );
        } );

        It( "should return false when a not started mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();

            TEST_FALSE( MissionSystem->IsMissionComplete( mission_data ) );
        } );

        It( "should return false when an uncompleted mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            MissionSystem->StartMission( mission_data );

            TEST_FALSE( MissionSystem->IsMissionComplete( mission_data ) );
        } );

        It( "should return true when a completed mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );

            objective->CompleteObjective();

            TEST_TRUE( MissionSystem->IsMissionComplete( mission_data ) );
        } );
    } );

    Describe( "IsMissionActive", [ this ]() {
        It( "should return false when a null mission data is given", [ this ]() {
            TEST_FALSE( MissionSystem->IsMissionActive( nullptr ) );
        } );

        It( "should return false when a not started mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();

            TEST_FALSE( MissionSystem->IsMissionActive( mission_data ) );
        } );

        It( "should return true when an uncompleted mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            MissionSystem->StartMission( mission_data );

            TEST_TRUE( MissionSystem->IsMissionActive( mission_data ) );
        } );

        It( "should return false when a completed mission data is given", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_AutoComplete::StaticClass() ) );
            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_AutoComplete >( mission->GetObjectives()[ 0 ] );

            objective->CompleteObjective();

            TEST_FALSE( MissionSystem->IsMissionActive( mission_data ) );
        } );
    } );

    Describe( "SkipMission", [ this ]() {
        It( "Should complete the current mission when there is only one mission.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            MissionSystem->StartMission( mission_data );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( MissionSystem->IsMissionComplete( mission_data ) );
            TEST_FALSE( MissionSystem->IsMissionActive( mission_data ) );
        } );

        It( "Should broadcast MissionComplete when there is only one mission.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            MissionSystem->StartMission( mission_data );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->MissionData, mission_data );
        } );

        It( "Should broadcast MissionObjectiveComplete when there is only one mission.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( Cast< UMSMissionObjective >( objective ), event_receiver->MissionObjective );
        } );

        It( "Should complete the current missions when there are multiple missions.", [ this ]() {
            auto * mission_data1 = NewObject< UMSMissionData >();
            mission_data1->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission_data2 = NewObject< UMSMissionData >();
            mission_data2->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            MissionSystem->StartMission( mission_data1 );
            MissionSystem->StartMission( mission_data2 );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( MissionSystem->IsMissionComplete( mission_data1 ) );
            TEST_FALSE( MissionSystem->IsMissionActive( mission_data1 ) );
            TEST_TRUE( MissionSystem->IsMissionComplete( mission_data2 ) );
            TEST_FALSE( MissionSystem->IsMissionActive( mission_data2 ) );
        } );

        It( "Should broadcast MissionComplete for each mission when there are multiple missions.", [ this ]() {
            auto * mission_data1 = NewObject< UMSMissionData >();
            mission_data1->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission_data2 = NewObject< UMSMissionData >();
            mission_data2->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            MissionSystem->StartMission( mission_data1 );
            MissionSystem->StartMission( mission_data2 );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->CallCount, 2 );
        } );

        It( "Should broadcast MissionObjectiveComplete for each objective when there is only one mission.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            MissionSystem->StartMission( mission_data );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->CallCount, 2 );
        } );

        It( "Should broadcast MissionObjectiveComplete for each objective when there are multiple missions.", [ this ]() {
            auto * mission_data1 = NewObject< UMSMissionData >();
            mission_data1->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            mission_data1->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission_data2 = NewObject< UMSMissionData >();
            mission_data2->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );
            mission_data2->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            MissionSystem->StartMission( mission_data1 );
            MissionSystem->StartMission( mission_data2 );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->CallCount, 4 );
        } );

        It( "Should not broadcast MissionComplete when the mission was already completed.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );
            objective->CompleteObjective();

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->CallCount, 1 );
        } );

        It( "Should not broadcast MissionObjectiveComplete when the mission was already completed.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );
            objective->CompleteObjective();

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( event_receiver->bDelegateWasCalled );
            TEST_EQUAL( event_receiver->CallCount, 1 );
        } );

        It( "Should not broadcast MissionComplete when there are no missions.", [ this ]() {
            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionCompleteDelegateCalled );

            MissionSystem->SkipCurrentMissions();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );

        It( "Should not broadcast MissionObjectiveComplete when there are no missions.", [ this ]() {
            auto * event_receiver = NewObject< UMSMissionEventReceiver >();
            MissionSystem->OnMissionObjectiveComplete().AddUniqueDynamic( event_receiver, &UMSMissionEventReceiver::OnMissionObjectiveDelegateCalled );

            MissionSystem->SkipCurrentMissions();

            TEST_FALSE( event_receiver->bDelegateWasCalled );
        } );

        It( "Should call OnObjectiveEnded when a single mission gets skipped.", [ this ]() {
            auto * mission_data = NewObject< UMSMissionData >();
            mission_data->Objectives.Add( FMSMissionObjectiveData( UMSMissionObjectiveMock_NoAutoComplete::StaticClass() ) );

            auto * mission = MissionSystem->StartMission( mission_data );

            auto * objective = Cast< UMSMissionObjectiveMock_NoAutoComplete >( mission->GetObjectives()[ 0 ] );

            MissionSystem->SkipCurrentMissions();

            TEST_TRUE( objective->bHasBeenEnded );
        } );
    } );
}