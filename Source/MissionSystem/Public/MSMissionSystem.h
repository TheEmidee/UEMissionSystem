#pragma once

#include "MSMission.h"
#include "MSMissionData.h"
#include "MSMissionHistory.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "MSMissionSystem.generated.h"

class UMSMissionData;

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionStartedDynamicDelegate, const UMSMissionData *, MissionData );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionStartedDelegate, const UMSMissionData * MissionData );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FMSMissionSystemMissionStartedMulticastDynamicDelegate, UMSMission *, Mission );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionEndedDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionEndedDelegate, const UMSMissionData * MissionData, bool WasCancelled );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSMissionSystemMissionEndedMulticastDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionObjectiveStartedDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionObjectiveStartedDelegate, TSubclassOf< UMSMissionObjective > MissionObjective );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveStartedMulticastDynamicDelegate, const UMSMissionData *, MissionData, TSubclassOf< UMSMissionObjective >, MissionObjective );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndedDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndedDelegate, TSubclassOf< UMSMissionObjective > MissionObjective, bool WasCancelled );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FMSMissionSystemMissionObjectiveEndedMulticastDynamicDelegate, const UMSMissionData *, MissionData, TSubclassOf< UMSMissionObjective >, MissionObjective, bool, WasCancelled );

UCLASS()
class MISSIONSYSTEM_API UMSMissionSystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    FMSOnMissionEndedEvent & OnMissionEnded();
    FMSOnMissionObjectiveEndedEvent & OnMissionObjectiveEnded();
    const FMSMissionHistory & GetMissionHistory() const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void StartMission( UMSMissionData * mission_data );

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionComplete( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionActive( UMSMissionData * mission_data ) const;

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

    void WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartedDelegate & when_mission_starts );
    void WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndedDelegate & when_mission_ends );

    void WhenMissionObjectiveStartsOrIsActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveStartedDelegate & when_mission_objective_starts );
    void WhenMissionObjectiveEnds( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveEndedDelegate & when_mission_objective_ends );

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    void DumpActiveMissions( FOutputDevice & output_device );
    void IgnoreObjectivesWithTags( const TArray< FString > & tags );
    void ClearIgnoreObjectivesTags();
    bool MustObjectiveBeIgnored( const UMSMissionObjective * objective ) const;
#endif

    bool ShouldCreateSubsystem( UObject * outer ) const override;
    void Serialize( FArchive & archive ) override;

protected:
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Starts or Is Active", AutoCreateRefTerm = "when_mission_starts" ) )
    void K2_WhenMissionStartsOrIsActive( UMSMissionData * mission_data, FMSMissionSystemMissionStartedDynamicDelegate when_mission_starts );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Ends", AutoCreateRefTerm = "when_mission_ends" ) )
    void K2_WhenMissionEnds( UMSMissionData * mission_data, FMSMissionSystemMissionEndedDynamicDelegate when_mission_ends );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Starts or Is Active", AutoCreateRefTerm = "when_mission_objective_starts" ) )
    void K2_WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveStartedDynamicDelegate when_mission_objective_starts );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Ends", AutoCreateRefTerm = "when_mission_objective_ends" ) )
    void K2_WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveEndedDynamicDelegate when_mission_objective_ends );

private:
    struct FMissionStartObserver
    {
        UMSMissionData * MissionData;
        FMSMissionSystemMissionStartedDelegate Callback;
    };

    struct FMissionEndObserver
    {
        UMSMissionData * MissionData;
        FMSMissionSystemMissionEndedDelegate Callback;
    };

    struct FMissionObjectiveStartObserver
    {
        TSubclassOf< UMSMissionObjective > MissionObjective;
        FMSMissionSystemMissionObjectiveStartedDelegate Callback;
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
    void OnMissionObjectiveStarted( const TSubclassOf< UMSMissionObjective > & objective, UMSMission * mission );
    void OnMissionObjectiveEnded( const TSubclassOf< UMSMissionObjective > & objective, bool was_cancelled, UMSMission * mission );
    void BroadcastOnMissionStarts(UMSMission* mission);
    void BroadcastOnMissionEnds( UMSMission * mission, bool was_cancelled );
    void BroadcastOnMissionObjectiveStarts( UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective );
    void BroadcastOnMissionObjectiveEnds(UMSMission * mission, const TSubclassOf< UMSMissionObjective > & objective, bool was_cancelled);

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
    FMSMissionSystemMissionObjectiveEndedMulticastDynamicDelegate OnMissionObjectiveEndedDelegate;

    TArray< FMissionStartObserver > MissionStartObservers;
    TArray< FMissionEndObserver > MissionEndObservers;
    TArray< FMissionObjectiveStartObserver > MissionObjectiveStartObservers;
    TArray< FMissionObjectiveEndObserver > MissionObjectiveEndObservers;
    FMSMissionHistory MissionHistory;
};

FORCEINLINE const FMSMissionHistory & UMSMissionSystem::GetMissionHistory() const
{
    return MissionHistory;
}