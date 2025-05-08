#pragma once

#include "MSMission.h"
#include "MSMissionData.h"
#include "MSMissionHistory.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "MSMissionSystemComponent.generated.h"

class UMSMissionData;

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionStartedDynamicDelegate, const UMSMissionData *, MissionData );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionStartedDelegate, const UMSMissionData * MissionData );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FMSMissionSystemMissionStartedMulticastDynamicDelegate, UMSMission *, Mission );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionEndedDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionEndedDelegate, const UMSMissionData * MissionData, bool WasCancelled );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSMissionSystemMissionEndedMulticastDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveStartedDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective, int, CurrentProgression );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveStartedDelegate, TSubclassOf< UMSMissionObjective > MissionObjective, int CurrentProgression );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FMSMissionSystemMissionObjectiveStartedMulticastDynamicDelegate, const UMSMissionData *, MissionData, TSubclassOf< UMSMissionObjective >, MissionObjective, int, CurrentProgression );

DECLARE_DYNAMIC_DELEGATE_ThreeParams( FMSMissionSystemMissionObjectiveProgressionUpdatedDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective, int, CurrentProgression, int, RequiredProgression );
DECLARE_DELEGATE_ThreeParams( FMSMissionSystemMissionObjectiveProgressionUpdatedDelegate, TSubclassOf< UMSMissionObjective > MissionObjective, int CurrentProgression, int RequiredProgression );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FMSMissionSystemMissionObjectiveProgressionUpdatedMulticastDynamicDelegate, const UMSMissionData *, MissionData, TSubclassOf< UMSMissionObjective >, MissionObjective, int, CurrentProgression, int, RequiredProgression );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndedDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndedDelegate, TSubclassOf< UMSMissionObjective > MissionObjective, bool WasCancelled );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FMSMissionSystemMissionObjectiveEndedMulticastDynamicDelegate, const UMSMissionData *, MissionData, TSubclassOf< UMSMissionObjective >, MissionObjective, bool, WasCancelled );

/* This component manages the missions and their objectives for a player
 The best actor to put this component on would be the player controller
 */

UCLASS( Blueprintable, BlueprintType )
class MISSIONSYSTEM_API UMSMissionSystemComponent final : public UActorComponent
{
    GENERATED_BODY()

public:
    explicit UMSMissionSystemComponent( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    const FMSMissionHistory & GetMissionHistory() const;
    FMSMissionSystemMissionStartedMulticastDynamicDelegate & OnMissionStarted();
    FMSMissionSystemMissionEndedMulticastDynamicDelegate & OnMissionEnded();
    FMSMissionSystemMissionObjectiveStartedMulticastDynamicDelegate & OnMissionObjectiveStarted();
    FMSMissionSystemMissionObjectiveProgressionUpdatedMulticastDynamicDelegate & OnMissionObjectiveProgressionUpdated();
    FMSMissionSystemMissionObjectiveEndedMulticastDynamicDelegate & OnMissionObjectiveEnded();

    UFUNCTION( BlueprintCallable, BlueprintPure = false, meta = ( ExpandBoolAsExecs = "ReturnValue" ) )
    bool HasDataInHistory() const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void StartMission( UMSMissionData * mission_data );

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionComplete( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionActive( const UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    UMSMission * GetActiveMission( const UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void CancelCurrentMissions() const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void CompleteCurrentMissions() const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void ResumeMissionsFromHistory();

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    bool CompleteObjective( UMSMissionData * mission_data, TSubclassOf< UMSMissionObjective > mission_objective_class );

    void WhenMissionStartsOrIsActive( const UMSMissionData * mission_data, const FMSMissionSystemMissionStartedDelegate & when_mission_starts );
    void WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndedDelegate & when_mission_ends );

    void WhenMissionObjectiveStartsOrIsActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveStartedDelegate & when_mission_objective_starts );
    void WhenMissionObjectiveProgressionIsUpdated( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveProgressionUpdatedDelegate & when_mission_objective_progression_is_updated, bool execute_callback_now );
    void WhenMissionObjectiveEnds( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveEndedDelegate & when_mission_objective_ends );

#if !UE_BUILD_SHIPPING
    void DumpActiveMissions( FOutputDevice & output_device );
    void IgnoreObjectivesWithTags( const TArray< FString > & tags );
    void ClearIgnoreObjectivesTags();
    bool MustObjectiveBeIgnored( const UMSMissionObjective * objective ) const;
#endif

    void Serialize( FArchive & archive ) override;
    void ClearMissionHistory();

    UFUNCTION( BlueprintCallable )
    void TryResumeMissionFromHistory();

protected:
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Starts or Is Active", AutoCreateRefTerm = "when_mission_starts" ) )
    void K2_WhenMissionStartsOrIsActive( const UMSMissionData * mission_data, FMSMissionSystemMissionStartedDynamicDelegate when_mission_starts );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Ends", AutoCreateRefTerm = "when_mission_ends" ) )
    void K2_WhenMissionEnds( UMSMissionData * mission_data, FMSMissionSystemMissionEndedDynamicDelegate when_mission_ends );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Starts or Is Active", AutoCreateRefTerm = "when_mission_objective_starts" ) )
    void K2_WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveStartedDynamicDelegate when_mission_objective_starts );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Progression is Updated", AutoCreateRefTerm = "when_mission_objective_progression_is_updated" ) )
    void K2_WhenMissionObjectiveProgressionIsUpdated( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveProgressionUpdatedDynamicDelegate when_mission_objective_progression_is_updated, bool execute_callback_now );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Ends", AutoCreateRefTerm = "when_mission_objective_ends" ) )
    void K2_WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveEndedDynamicDelegate when_mission_objective_ends );

private:
    struct FMissionStartObserver
    {
        const UMSMissionData * MissionData = nullptr;
        FMSMissionSystemMissionStartedDelegate Callback;
    };

    struct FMissionEndObserver
    {
        const UMSMissionData * MissionData = nullptr;
        FMSMissionSystemMissionEndedDelegate Callback;
    };

    struct FMissionObjectiveStartObserver
    {
        TSubclassOf< UMSMissionObjective > MissionObjective;
        FMSMissionSystemMissionObjectiveStartedDelegate Callback;
    };

    struct FMissionObjectiveProgressionObserver
    {
        TSubclassOf< UMSMissionObjective > MissionObjective;
        FMSMissionSystemMissionObjectiveProgressionUpdatedDelegate Callback;
    };

    struct FMissionObjectiveEndObserver
    {
        TSubclassOf< UMSMissionObjective > MissionObjective;
        FMSMissionSystemMissionObjectiveEndedDelegate Callback;
    };

    UMSMission * TryCreateMissionFromData( UMSMissionData * mission_data );
    UMSMission * CreateMissionFromData( UMSMissionData * mission_data );
    void StartMission( UMSMission * mission );
    void StartNextMissions( const UMSMissionData * mission_data );
    void OnMissionEnded( UMSMission * mission, bool was_cancelled );
    void OnMissionObjectiveStarted( UMSMissionObjective * objective, UMSMission * mission );
    void OnMissionObjectiveProgressionUpdated( UMSMissionObjective * objective, UMSMission * mission );
    void OnMissionObjectiveEnded( UMSMissionObjective * objective, bool was_cancelled, UMSMission * mission );
    void BroadcastOnMissionStarted( UMSMission * mission );
    void BroadcastOnMissionEnded( UMSMission * mission, bool was_cancelled );
    void BroadcastOnMissionObjectiveStarted( UMSMission * mission, UMSMissionObjective * objective );
    void BroadcastOnMissionObjectiveProgressionUpdated( const UMSMission * mission, const UMSMissionObjective * objective );
    void BroadcastOnMissionObjectiveEnded( const UMSMission * mission, UMSMissionObjective * objective, bool was_cancelled );

    UPROPERTY()
    TArray< UMSMission * > ActiveMissions;

    UPROPERTY()
    TArray< FString > TagsToIgnoreForObjectives;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionStartedMulticastDynamicDelegate OnMissionStartedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionEndedMulticastDynamicDelegate OnMissionEndedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionObjectiveStartedMulticastDynamicDelegate OnMissionObjectiveStartedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionObjectiveProgressionUpdatedMulticastDynamicDelegate OnMissionObjectiveProgressionIsUpdatedDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionObjectiveEndedMulticastDynamicDelegate OnMissionObjectiveEndedDelegate;

    UPROPERTY( EditDefaultsOnly )
    uint8 bTryResumeMissionFromHistory : 1;

    UPROPERTY( EditDefaultsOnly, meta = ( EditCondition = "bTryResumeMissionFromHistory" ) )
    TObjectPtr< UMSMissionData > FirstMissionToStart;

    TArray< FMissionStartObserver > MissionStartObservers;
    TArray< FMissionEndObserver > MissionEndObservers;
    TArray< FMissionObjectiveStartObserver > MissionObjectiveStartObservers;
    TArray< FMissionObjectiveProgressionObserver > MissionObjectiveProgressionObservers;
    TArray< FMissionObjectiveEndObserver > MissionObjectiveEndObservers;
    FMSMissionHistory MissionHistory;
};

FORCEINLINE const FMSMissionHistory & UMSMissionSystemComponent::GetMissionHistory() const
{
    return MissionHistory;
}

FORCEINLINE FMSMissionSystemMissionStartedMulticastDynamicDelegate & UMSMissionSystemComponent::OnMissionStarted()
{
    return OnMissionStartedDelegate;
}

FORCEINLINE FMSMissionSystemMissionEndedMulticastDynamicDelegate & UMSMissionSystemComponent::OnMissionEnded()
{
    return OnMissionEndedDelegate;
}

FORCEINLINE FMSMissionSystemMissionObjectiveStartedMulticastDynamicDelegate & UMSMissionSystemComponent::OnMissionObjectiveStarted()
{
    return OnMissionObjectiveStartedDelegate;
}

FORCEINLINE FMSMissionSystemMissionObjectiveProgressionUpdatedMulticastDynamicDelegate & UMSMissionSystemComponent::OnMissionObjectiveProgressionUpdated()
{
    return OnMissionObjectiveProgressionIsUpdatedDelegate;
}

FORCEINLINE FMSMissionSystemMissionObjectiveEndedMulticastDynamicDelegate & UMSMissionSystemComponent::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveEndedDelegate;
}