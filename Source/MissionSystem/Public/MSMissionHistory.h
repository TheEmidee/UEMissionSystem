#pragma once

#include <CoreMinimal.h>

#include "MSMissionHistory.generated.h"

class UMSMissionObjective;
class UMSMissionData;

enum class EMSState : uint8
{
    Active,
    Cancelled,
    Complete
};

struct FMSObjectiveProgressionData
{
    int CurrentProgression;
    TArray< uint8 > RecordData;

    friend FArchive & operator<<( FArchive & archive, FMSObjectiveProgressionData & progression_data );
};

// This structure holds the completion of the missions and objectives, and must be serialized in the save game
USTRUCT()
struct MISSIONSYSTEM_API FMSMissionHistory
{
    GENERATED_USTRUCT_BODY()

    const TArray< UMSMissionData * > & GetActiveMissionData() const;

    bool HasData() const;

    bool IsMissionActive(const UMSMissionData* mission_data) const;
    bool IsMissionCancelled( UMSMissionData * mission_data ) const;
    bool IsMissionComplete( UMSMissionData * mission_data ) const;
    bool IsMissionFinished( UMSMissionData * mission_data ) const;
    bool AddActiveMission( UMSMissionData * mission_data );
    bool SetMissionComplete( UMSMissionData * mission_data, bool was_cancelled );

    bool IsObjectiveActive( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveCancelled( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveComplete( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool IsObjectiveFinished( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    bool AddActiveObjective( const UMSMissionObjective * mission_objective );
    int GetObjectiveProgression( const TSubclassOf< UMSMissionObjective > & mission_objective_class ) const;
    void UpdateObjectiveProgression( UMSMissionObjective * mission_objective );
    bool SetObjectiveComplete( const UMSMissionObjective * mission_objective, bool was_cancelled );
    void InitializeObjective( UMSMissionObjective * mission_objective ) const;

    friend FArchive & operator<<( FArchive & archive, FMSMissionHistory & mission_history );
    void Clear();

private:
    bool DoesMissionHasState(const UMSMissionData* mission_data, EMSState state) const;
    bool DoesObjectiveHasState( const TSubclassOf< UMSMissionObjective > & mission_objective_class, EMSState state ) const;

    UPROPERTY()
    TArray< UMSMissionData * > ActiveMissionsData;

    TMap< FGuid, EMSState > MissionStates;
    TMap< FGuid, EMSState > ObjectiveStates;
    TMap< FGuid, FMSObjectiveProgressionData > ObjectiveProgressions;
};

FORCEINLINE const TArray< UMSMissionData * > & FMSMissionHistory::GetActiveMissionData() const
{
    return ActiveMissionsData;
}