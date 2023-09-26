#pragma once

#include "MSMission.h"
#include "MSMissionData.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "MSMissionSystem.generated.h"

class UMSMissionData;

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionStartsDynamicDelegate, const UMSMissionData *, MissionData );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionStartsDelegate, const UMSMissionData * MissionData );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionEndsDynamicDelegate, const UMSMissionData *, MissionData, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionEndsDelegate, const UMSMissionData * MissionData, bool WasCancelled );

DECLARE_DYNAMIC_DELEGATE_OneParam( FMSMissionSystemMissionObjectiveStartsDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective );
DECLARE_DELEGATE_OneParam( FMSMissionSystemMissionObjectiveStartsDelegate, TSubclassOf< UMSMissionObjective > MissionObjective );

DECLARE_DYNAMIC_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndsDynamicDelegate, TSubclassOf< UMSMissionObjective >, MissionObjective, bool, WasCancelled );
DECLARE_DELEGATE_TwoParams( FMSMissionSystemMissionObjectiveEndsDelegate, TSubclassOf< UMSMissionObjective > MissionObjective, bool WasCancelled );

UCLASS()
class MISSIONSYSTEM_API UMSMissionSystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    FMSOnMissionEndedEvent & OnMissionEnded();
    FMSOnMissionObjectiveEndedEvent & OnMissionObjectiveEnded();

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void StartMission( UMSMissionData * mission_data );

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionComplete( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionActive( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    UMSMission * GetActiveMission( UMSMissionData * mission_data ) const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void CancelCurrentMissions() const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void CompleteCurrentMissions() const;

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = "Mission System" )
    bool IsMissionObjectiveActive( TSubclassOf< UMSMissionObjective > mission_objective_class ) const;

    void WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartsDelegate & when_mission_starts );
    void WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndsDelegate & when_mission_ends );

    void WhenMissionObjectiveStartsOrIsActive( TSubclassOf< UMSMissionObjective > mission_objective, const FMSMissionSystemMissionObjectiveStartsDelegate & when_mission_objective_starts );
    void WhenMissionObjectiveEnds( TSubclassOf< UMSMissionObjective > mission_objective, const FMSMissionSystemMissionObjectiveEndsDelegate & when_mission_objective_ends );

#if !( UE_BUILD_SHIPPING || UE_BUILD_TEST )
    void DumpActiveMissions( FOutputDevice & output_device );
    void IgnoreObjectivesWithTags( const TArray< FString > & tags );
    void ClearIgnoreObjectivesTags();
    bool MustObjectiveBeIgnored( const UMSMissionObjective * objective ) const;
#endif

    bool ShouldCreateSubsystem( UObject * outer ) const override;

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

    template< typename _ALLOCATOR_TYPE_ >
    void GetActiveMissions( TArray< UMSMission *, _ALLOCATOR_TYPE_ > & missions ) const
    {
        ActiveMissions.GenerateValueArray( missions );
    }

    void StartNextMissions( UMSMissionData * mission_data );
    void OnMissionEnded( UMSMissionData * mission_data, bool was_cancelled );
    void OnMissionObjectiveStarted( UMSMissionObjective * objective );
    void OnMissionObjectiveEnded( UMSMissionObjective * objective, bool was_cancelled );

    FMSOnMissionEndedEvent OnMissionEndedEvent;
    FMSOnMissionObjectiveEndedEvent OnMissionObjectiveEndedEvent;

    UPROPERTY()
    TMap< UMSMissionData *, UMSMission * > ActiveMissions;

    UPROPERTY()
    TMap< UMSMissionData *, bool > CompletedMissions;

    UPROPERTY()
    TArray< FString > TagsToIgnoreForObjectives;

    TArray< FMissionStartObserver > MissionStartObservers;
    TArray< FMissionEndObserver > MissionEndObservers;
    TArray< FMissionObjectiveStartObserver > MissionObjectiveStartObservers;
    TArray< FMissionObjectiveEndObserver > MissionObjectiveEndObservers;
};

FORCEINLINE FMSOnMissionEndedEvent & UMSMissionSystem::OnMissionEnded()
{
    return OnMissionEndedEvent;
}

FORCEINLINE FMSOnMissionObjectiveEndedEvent & UMSMissionSystem::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveEndedEvent;
}