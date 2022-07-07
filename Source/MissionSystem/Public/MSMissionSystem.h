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

UCLASS()
class MISSIONSYSTEM_API UMSMissionSystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    FMSOnMissionEndedDelegate & OnMissionEnded();
    FMSOnMissionObjectiveEndedDelegate & OnMissionObjectiveEnded();

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System" )
    void StartMission( UMSMissionData * mission_data );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission System", meta = ( AutoCreateRefTerm = "when_mission_ends" ) )
    void StartMissionWithEndDelegate( UMSMissionData * mission_data, FMSMissionSystemMissionEndsDynamicDelegate when_mission_ends );

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

    void WhenMissionStartsOrIsActive( UMSMissionData * mission_data, const FMSMissionSystemMissionStartsDelegate & when_mission_starts );
    void WhenMissionEnds( UMSMissionData * mission_data, const FMSMissionSystemMissionEndsDelegate & when_mission_ends );

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

    void StartNextMissions( UMSMissionData * mission_data );

    UFUNCTION()
    void OnMissionEnded( UMSMissionData * mission_data, bool was_cancelled );

    UFUNCTION()
    void OnMissionObjectiveEnded( UMSMissionObjective * objective, bool was_cancelled );

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionEndedDelegate OnMissionCompleteDelegate;

    UPROPERTY( BlueprintAssignable )
    FMSOnMissionObjectiveEndedDelegate OnMissionObjectiveCompleteDelegate;

    UPROPERTY()
    TMap< UMSMissionData *, UMSMission * > ActiveMissions;

    UPROPERTY()
    TArray< UMSMissionData * > CompletedMissions;

    UPROPERTY()
    TArray< FString > TagsToIgnoreForObjectives;

    TArray< FMissionStartObserver > MissionStartObservers;
    TArray< FMissionEndObserver > MissionEndObservers;
};

FORCEINLINE FMSOnMissionEndedDelegate & UMSMissionSystem::OnMissionEnded()
{
    return OnMissionCompleteDelegate;
}

FORCEINLINE FMSOnMissionObjectiveEndedDelegate & UMSMissionSystem::OnMissionObjectiveEnded()
{
    return OnMissionObjectiveCompleteDelegate;
}