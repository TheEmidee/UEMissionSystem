#pragma once

#include "MSMission.h"
#include "MSMissionData.h"
#include "MSMissionHistory.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "MSMissionSystem.generated.h"

class UMSMissionData;

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionStartsDynamicDelegate, const UMSMissionData *, MissionData );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionStartsDelegate, const UMSMissionData * MissionData );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FMSMissionSystemMissionStartsMulticastDynamicDelegate, UMSMission *, Mission );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionEndsDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionEndsDelegate, const UMSMissionData * MissionData, bool WasCancelled );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSMissionSystemMissionEndsMulticastDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionObjectiveStartsDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionObjectiveStartsDelegate, TSubclassOf< UMSMissionObjective > MissionObjective );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveStartsMulticastDynamicDelegate, UMSMission *, Mission, TSubclassOf< UMSMissionObjective > , MissionObjective );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndsDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndsDelegate, TSubclassOf< UMSMissionObjective > MissionObjective, bool WasCancelled );

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

    void WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartsDelegate & when_mission_starts );
    void WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndsDelegate & when_mission_ends );

    void WhenMissionObjectiveStartsOrIsActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveStartsDelegate & when_mission_objective_starts );
    void WhenMissionObjectiveEnds( const TSubclassOf< UMSMissionObjective > & mission_objective_class, const FMSMissionSystemMissionObjectiveEndsDelegate & when_mission_objective_ends );

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
    void K2_WhenMissionStartsOrIsActive( UMSMissionData * mission_data, FMSMissionSystemMissionStartsDynamicDelegate when_mission_starts );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Ends", AutoCreateRefTerm = "when_mission_ends" ) )
    void K2_WhenMissionEnds( UMSMissionData * mission_data, FMSMissionSystemMissionEndsDynamicDelegate when_mission_ends );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Starts or Is Active", AutoCreateRefTerm = "when_mission_objective_starts" ) )
    void K2_WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveStartsDynamicDelegate when_mission_objective_starts );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( DisplayName = "When Mission Objective Ends", AutoCreateRefTerm = "when_mission_objective_ends" ) )
    void K2_WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, FMSMissionSystemMissionObjectiveEndsDynamicDelegate when_mission_objective_ends );

private:
    struct FMissionStartObserver
    {
        UMSMissionData * MissionData;
        FMSMissionSystemMissionStartsDelegate Callback;
    };

    struct FMissionEndObserver
    {
        UMSMissionData * MissionData;
        FMSMissionSystemMissionEndsDelegate Callback;
    };

    struct FMissionObjectiveStartObserver
    {
        TSubclassOf< UMSMissionObjective > MissionObjective;
        FMSMissionSystemMissionObjectiveStartsDelegate Callback;
    };

    struct FMissionObjectiveEndObserver
    {
        TSubclassOf< UMSMissionObjective > MissionObjective;
        FMSMissionSystemMissionObjectiveEndsDelegate Callback;
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

    FMSOnMissionEndedEvent OnMissionEndedEvent;
    FMSOnMissionObjectiveEndedEvent OnMissionObjectiveEndedEvent;

    UPROPERTY()
    TArray< UMSMission * > ActiveMissions;

    UPROPERTY()
    TArray< FString > TagsToIgnoreForObjectives;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionStartsMulticastDynamicDelegate OnMissionStartsDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionEndsMulticastDynamicDelegate OnMissionEndsDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FMSMissionSystemMissionObjectiveStartsMulticastDynamicDelegate OnMissionObjectiveStartsDelegate;

    TArray< FMissionStartObserver > MissionStartObservers;
    TArray< FMissionEndObserver > MissionEndObservers;
    TArray< FMissionObjectiveStartObserver > MissionObjectiveStartObservers;
    TArray< FMissionObjectiveEndObserver > MissionObjectiveEndObservers;
    FMSMissionHistory MissionHistory;
};

FORCEINLINE FMSOnMissionEndedEvent & UMSMissionSystem::OnMissionEnded()
{
    return OnMissionEndedEvent;
}

FORCEINLINE FMSOnMissionObjectiveEndedEvent & UMSMissionSystem::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveEndedEvent;
}

FORCEINLINE const FMSMissionHistory & UMSMissionSystem::GetMissionHistory() const
{
    return MissionHistory;
}